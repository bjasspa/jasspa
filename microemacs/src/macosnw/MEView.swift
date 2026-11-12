// MEView.swift
// JASSPA MicroEmacs - native macOS frontend
//
// Owns the NSView that ME renders into.  Responsibilities:
//   - Measure the monospace font and tell macosterm.c the cell metrics
//   - Register MENativeCallbacks (setCursor, bell, title, font)
//   - Implement meNativeViewFlushDirty (@_cdecl): copies dirty rows from
//     frameCur->store into the cells[] render buffer, then calls setNeedsDisplay
//   - Translate NSEvent keyboard and mouse events into ME key codes
//   - Forward events to macosterm.c via meNativePushKey / meNativePushMouseButton

import Cocoa
import CoreText

// ---------------------------------------------------------------------------
// MARK: - ME modifier bits  (from estruct.h via the meNativeMods C global)
// ---------------------------------------------------------------------------
// Modifier bits and special-key codes are read from meNativeMods and meSKeys
// (C globals) so they always match the current build, an ME key is 16 bits in
// an 8-bit build and 32 bits in a Unicode build.

private enum MEMod {
    static let shift:   UInt32 = meNativeMods.shift
    static let control: UInt32 = meNativeMods.control
    static let alt:     UInt32 = meNativeMods.alt
}

// NSEvent.keyCode values for function/nav keys (hardware scan codes, layout-independent)
private let kVKReturn:       UInt16 = 0x24
private let kVKTab:          UInt16 = 0x30
private let kVKDelete:       UInt16 = 0x33   // Backspace
private let kVKEscape:       UInt16 = 0x35
private let kVKForwardDelete:UInt16 = 0x75
private let kVKHome:         UInt16 = 0x73
private let kVKEnd:          UInt16 = 0x77
private let kVKPageUp:       UInt16 = 0x74
private let kVKPageDown:     UInt16 = 0x79
private let kVKLeftArrow:    UInt16 = 0x7B
private let kVKRightArrow:   UInt16 = 0x7C
private let kVKDownArrow:    UInt16 = 0x7D
private let kVKUpArrow:      UInt16 = 0x7E
private let kVKF1:           UInt16 = 0x7A
private let kVKF2:           UInt16 = 0x78
private let kVKF3:           UInt16 = 0x63
private let kVKF4:           UInt16 = 0x76
private let kVKF5:           UInt16 = 0x60
private let kVKF6:           UInt16 = 0x61
private let kVKF7:           UInt16 = 0x62
private let kVKF8:           UInt16 = 0x64
private let kVKF9:           UInt16 = 0x65
private let kVKF10:          UInt16 = 0x6D
private let kVKF11:          UInt16 = 0x67
private let kVKF12:          UInt16 = 0x6F
// Numpad keys
private let kVKKP0:          UInt16 = 0x52
private let kVKKP1:          UInt16 = 0x53
private let kVKKP2:          UInt16 = 0x54
private let kVKKP3:          UInt16 = 0x55
private let kVKKP4:          UInt16 = 0x56
private let kVKKP5:          UInt16 = 0x57
private let kVKKP6:          UInt16 = 0x58
private let kVKKP7:          UInt16 = 0x59
private let kVKKP8:          UInt16 = 0x5B
private let kVKKP9:          UInt16 = 0x5C
private let kVKKPDecimal:    UInt16 = 0x41
private let kVKKPEnter:      UInt16 = 0x4C

// ---------------------------------------------------------------------------
// MARK: - Drawing cell
// ---------------------------------------------------------------------------

/// One cell in the backing store: colour indices + attributes + character.
/// ch is a code point (an 8-bit build's code page char), below 0x20 an ME
/// special char, ME_FRAMECHAR_CONT the second cell of a wide char.
private struct Cell {
    var ch:     UInt32 = 0x20
    var fg:     UInt8  = 0
    var bg:     UInt8  = 1
    var attrs:  UInt8  = 0
}

/// How a char is drawn. font is 0 for the cell font (the attrs variant), n > 0
/// for MEView.fallbackFonts[n-1] and -1 when no font has the char (drawn as the
/// unavailable char box). A wide or fallback glyph is drawn on its own, scaled by
/// scale and offset by dx, dy from its cell's origin & baseline so it fits its
/// cells.
private struct GlyphRef {
    var glyph:  CGGlyph = 0
    var font:   Int32   = 0
    var cells:  Int     = 1
    var scale:  CGFloat = 1
    var dx:     CGFloat = 0
    var dy:     CGFloat = 0
}

// ---------------------------------------------------------------------------
// MARK: - MEView
// ---------------------------------------------------------------------------

final class MEView: NSView {

    // ---- Font and cell geometry - shared across all windows ----------------
    private static var font: NSFont = NSFont.monospacedSystemFont(ofSize: 14, weight: .regular)
    private static var boldFont: NSFont = NSFont.monospacedSystemFont(ofSize: 14, weight: .regular)
    private static var italicFont: NSFont = NSFont.monospacedSystemFont(ofSize: 14, weight: .regular)
    private static var boldItalicFont: NSFont = NSFont.monospacedSystemFont(ofSize: 14, weight: .regular)
    // ---- Glyph lookup - shared across all windows, reset on a font change ----
    // Every char is drawn with its own glyph at its cell and never shaped with
    // its neighbours, so a char is drawn the same whatever is drawn with it and
    // moving the cursor cannot change it. A char the font lacks is drawn with
    // CoreText's fallback font for it, fitted to its cells, or as the unavailable
    // char box if no font has it (or ME_GLYPH_FALLBACK=0 is set).
    private static var asciiGlyphs: [[CGGlyph]] = Array(repeating: Array(repeating: 0, count: 128), count: 4)
    private static var glyphCache:  [[UInt32: GlyphRef]] = Array(repeating: [:], count: 4)
    private static var fallbackFonts: [CTFont] = []
    private static var fallbackIndex: [String: Int32] = [:]
    private static let useFallback = ProcessInfo.processInfo.environment["ME_GLYPH_FALLBACK"] != "0"
    // An 8-bit build's code page (charToUnicode) for chars 0x80 to 0xff
    private static var codePage: [UInt32] = (0..<128).map { UInt32($0 + 128) }
    
    private var changeFontN: Int32 = 0
    private var fontPanelSemaphore: DispatchSemaphore? = nil

    static var cellW: Int = 8
    static var cellH: Int = 16
    private(set) var cols:   Int = 80
    private(set) var rows:   Int = 24

    // ---- Cell backing store -----------------------------------------------
    // Populated on the main thread by cFlushDirty (called from TTflush via
    // DispatchQueue.main.sync while the ME engine thread is blocked).
    // Read on the main thread by draw().  No lock needed.
    private var cells: [Cell] = []   // rows * cols

    // ---- Cursor -----------------------------------------------------------
    private var cursorCol:  Int = 0
    private var cursorRow:  Int = 0
    private var cursorVisible: Bool = true
    private var cursorOn:   Bool = true   // blink phase - driven by ME's TTshowCur/TThideCur
    var hasFocus: Bool = true             // window key state - controls filled vs outline cursor

    // ---- Scratch buffers reused each drawRow render call (main thread only) ------
    private var scratchGlyphs:    [CGGlyph] = Array(repeating: 0, count: 512)
    private var scratchPositions: [CGPoint] = Array(repeating: .zero, count: 512)

    // ---- Initialisation ---------------------------------------------------

    override init(frame: NSRect) {
        super.init(frame: frame)
        wantsLayer = true;
        layer?.backgroundColor = NSColor.black.cgColor;
        registerForDraggedTypes([.fileURL])
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        wantsLayer = true;
        layer?.backgroundColor = NSColor.black.cgColor;
        registerForDraggedTypes([.fileURL])
    }

    /// Initialise a secondary view's cell buffer; font/glyph/cell metrics are shared statics.
    func copyMetrics(from source: MEView, cols: Int, rows: Int) {
        self.cols = cols;
        self.rows = rows;
        var blank = Cell();
        blank.bg = bgColorIdx;
        cells = Array(repeating: blank, count: cols * rows);
    }

    func setSize(cols: Int, rows: Int)
    {
        let sz = cols * rows;
        if (sz > (self.cols * self.rows))
        {
            var blank = Cell();
            blank.bg = bgColorIdx;
            cells = Array(repeating: blank, count: sz);
        }
        self.cols = cols;
        self.rows = rows;
        meNativeReturnViewFrameSize(Unmanaged.passUnretained(self).toOpaque(), Int32(cols), Int32(rows))
        needsDisplay = true;
    }

    func setFont(n: Int32, font: NSFont)
    {
        // Use CoreText for sub-pixel accurate metrics
        let ctFont = font as CTFont;
        let w = max(1,Int(CTFontGetAdvancesForGlyphs(ctFont,.horizontal,[CTFontGetGlyphWithName(ctFont,"W" as CFString)],nil,1) + 0.5));
        let h = max(1,Int(CTFontGetDescent(ctFont) + CTFontGetAscent(ctFont) + CTFontGetLeading(ctFont) + 0.5));
        // Tell C side
        meNativeReturnFontSize(Int32(w),Int32(h),Int32(font.ascender),Int32(CTFontGetSymbolicTraits(ctFont).contains(.monoSpaceTrait) ? 1:0));

        if((n & 2) == 0)
        {
            MEView.font = font;
            MEView.boldFont = NSFontManager.shared.convert(font, toHaveTrait: .boldFontMask);
            MEView.italicFont = NSFontManager.shared.convert(font, toHaveTrait: .italicFontMask);
            MEView.boldItalicFont = NSFontManager.shared.convert(MEView.boldFont, toHaveTrait: .italicFontMask);
            MEView.cellW = w;
            MEView.cellH = h;
            MEView.resetGlyphs()
            MEView.updateCharset()
        }
    }

    /// Called when the code page changes, only an 8-bit build's display uses it.
    static func updateCharset()
    {
        if meNativeUnicode == 0, let cp = charToUnicode {
            for ii in 0..<128 {
                codePage[ii] = UInt32(cp[ii])
            }
            for wc in gWindowControllers {
                wc.meView?.needsDisplay = true
            }
        }
    }

    private static func resetGlyphs()
    {
        var chars = (0..<128).map { UniChar($0) }
        for vv in 0..<4 {
            var g = [CGGlyph](repeating: 0, count: 128)
            CTFontGetGlyphsForCharacters(ctFontVariant(vv), &chars, &g, 128)
            asciiGlyphs[vv] = g
            glyphCache[vv] = [:]
        }
        fallbackFonts = []
        fallbackIndex = [:]
    }

    private static func ctFontVariant(_ vv: Int) -> CTFont
    {
        switch (vv & 0x03)
        {
        case 0:  return MEView.font as CTFont;
        case 1:  return MEView.italicFont as CTFont;
        case 2:  return MEView.boldFont as CTFont;
        default: return MEView.boldItalicFont as CTFont;
        }
    }

    private func ctFontVariant(attrs: UInt8) -> CTFont
    {
        return MEView.ctFontVariant(Int(attrs & 0x03))
    }

    private static func glyphFont(_ gr: GlyphRef, attrs: UInt8) -> CTFont
    {
        return (gr.font > 0) ? fallbackFonts[Int(gr.font) - 1] : ctFontVariant(Int(attrs & 0x03))
    }

    /// The code point of a cell's char, 0 if it has none (an 8-bit build's
    /// undefined code page char)
    private static func codePoint(_ ch: UInt32) -> UInt32
    {
        return ((ch < 0x80) || (meNativeUnicode != 0)) ? ch : codePage[Int(ch & 0x7f)]
    }

    /// How code point cp (0x20 or more) is drawn in cells cells with font variant attrs
    private static func glyphRef(_ cp: UInt32, attrs: UInt8, cells: Int) -> GlyphRef
    {
        let vv = Int(attrs & 0x03)
        if (cp < 0x80) && (cells == 1) {
            return GlyphRef(glyph: asciiGlyphs[vv][Int(cp)])
        }
        if let gr = glyphCache[vv][cp], gr.cells == cells {
            return gr
        }
        var gr = GlyphRef(glyph: 0, font: -1, cells: cells)
        if let us = Unicode.Scalar(cp) {
            let str = String(Character(us))
            var u16 = Array(str.utf16)
            var gs = [CGGlyph](repeating: 0, count: u16.count)
            let base = ctFontVariant(vv)
            if CTFontGetGlyphsForCharacters(base, &u16, &gs, u16.count) && (gs[0] != 0) {
                gr.glyph = gs[0]
                gr.font = 0
            } else if useFallback {
                let fb = CTFontCreateForString(base, str as CFString, CFRange(location: 0, length: u16.count))
                let name = CTFontCopyPostScriptName(fb) as String
                // LastResort draws a placeholder for the char's block, the box is used instead
                if (name != "LastResort") && !CFEqual(fb, base) &&
                   CTFontGetGlyphsForCharacters(fb, &u16, &gs, u16.count) && (gs[0] != 0) {
                    if let ii = fallbackIndex[name] {
                        gr.font = ii
                    } else {
                        fallbackFonts.append(fb)
                        gr.font = Int32(fallbackFonts.count)
                        fallbackIndex[name] = gr.font
                    }
                    gr.glyph = gs[0]
                }
            }
        }
        if (gr.font > 0) || ((gr.font == 0) && (cells > 1)) {
            fitGlyph(&gr, glyphFont(gr, attrs: attrs))
        }
        glyphCache[vv][cp] = gr
        return gr
    }

    /// Sets the scale and offset that fit gr's glyph inside its cells: centred,
    /// scaled down (keeping its shape) if its ink is wider than its cells or
    /// taller than a cell, and moved up or down if it reaches out of the cell.
    private static func fitGlyph(_ gr: inout GlyphRef, _ ft: CTFont)
    {
        var glyph = gr.glyph
        var adv = CGSize.zero
        CTFontGetAdvancesForGlyphs(ft, .horizontal, &glyph, &adv, 1)
        let bb = CTFontGetBoundingRectsForGlyphs(ft, .horizontal, &glyph, nil, 1)
        let ww = CGFloat(gr.cells * cellW)
        let hh = CGFloat(cellH)
        let desc = -font.descender
        let asc = hh - desc
        let left = bb.isNull ? 0 : min(0, bb.minX)
        let right = bb.isNull ? adv.width : max(adv.width, bb.maxX)
        var ss: CGFloat = 1
        if right - left > ww {
            ss = ww / (right - left)
        }
        if !bb.isNull && (bb.height > hh) && (hh / bb.height < ss) {
            ss = hh / bb.height
        }
        gr.scale = ss
        gr.dx = (ww - (right - left) * ss) / 2 - left * ss
        gr.dy = 0
        if !bb.isNull {
            if bb.maxY * ss > asc {
                gr.dy = asc - bb.maxY * ss
            } else if bb.minY * ss < -desc {
                gr.dy = -desc - bb.minY * ss
            }
        }
    }

    /// Draws a wide, fallback or unavailable char at x (its first cell) on the
    /// baseline in colour, clipped to its cells
    private func drawSingleGlyph(_ gr: GlyphRef, attrs: UInt8, x: CGFloat, cellY: CGFloat, baseline: CGFloat,
                                 color: CGColor, in ctx: CGContext)
    {
        let rect = CGRect(x: x, y: cellY, width: CGFloat(gr.cells * MEView.cellW), height: CGFloat(MEView.cellH))
        if gr.font < 0 {
            drawUnavailable(rect, color: color, in: ctx)
            return
        }
        ctx.saveGState()
        ctx.setFillColor(color)
        ctx.clip(to: rect)
        ctx.translateBy(x: x + gr.dx, y: baseline + gr.dy)
        ctx.scaleBy(x: gr.scale, y: gr.scale)
        var glyph = gr.glyph
        var pos = CGPoint.zero
        CTFontDrawGlyphs(MEView.glyphFont(gr, attrs: attrs), &glyph, &pos, 1, ctx)
        ctx.restoreGState()
    }

    /// The box with a diagonal drawn for a char no font has (as the Windows and X11
    /// builds)
    private func drawUnavailable(_ rect: CGRect, color: CGColor, in ctx: CGContext)
    {
        let x0 = rect.minX + 1.5, x1 = rect.maxX - 1.5
        let y0 = rect.minY + 1.5, y1 = rect.maxY - 1.5
        ctx.saveGState()
        ctx.setStrokeColor(color)
        ctx.setLineWidth(1.0)
        ctx.stroke(CGRect(x: x0, y: y0, width: x1 - x0, height: y1 - y0))
        ctx.move(to: CGPoint(x: x0, y: y0))
        ctx.addLine(to: CGPoint(x: x1, y: y1))
        ctx.strokePath()
        ctx.restoreGState()
    }

    // MARK: - Geometry helpers

    var pixelWidth:  CGFloat { CGFloat(cols * MEView.cellW) }
    var pixelHeight: CGFloat { CGFloat(rows * MEView.cellH) }

    private func cellRect(col: Int, row: Int, rows: Int) -> NSRect {
        // NSView is bottom-left origin; ME is top-left.
        let y = CGFloat(rows - 1 - row) * CGFloat(MEView.cellH)
        return NSRect(x: CGFloat(col * MEView.cellW), y: y,
                      width: CGFloat(MEView.cellW), height: CGFloat(MEView.cellH))
    }

    // Convenience overload using self.rows (safe for cursor/focus callers on main thread)
    private func cellRect(col: Int, row: Int) -> NSRect {
        cellRect(col: col, row: row, rows: rows)
    }

    // The cursor's cells to redraw, two in case it is on a wide char
    private func cursorRect(col: Int, row: Int) -> NSRect {
        var rr = cellRect(col: col, row: row, rows: rows)
        rr.size.width *= 2
        return rr
    }

    // MARK: - Drawing

    override var isOpaque: Bool { true }
    override var acceptsFirstResponder: Bool { true }

    override func draw(_ dirtyRect: NSRect) {
        guard let ctx = NSGraphicsContext.current?.cgContext else { return }

        // cells is only written on the main thread (cFlushDirty / resize),
        // so no lock is needed here.
        let snapCols  = cols
        let snapRows  = rows
        let snapCells = cells

        let pixH = CGFloat(snapRows * MEView.cellH)
        let topRow    = max(0,       Int(floor((pixH - dirtyRect.maxY) / CGFloat(MEView.cellH))))
        let bottomRow = min(snapRows, Int(ceil( (pixH - dirtyRect.minY) / CGFloat(MEView.cellH))))

        for row in topRow..<bottomRow {
            drawRow(row, cols: snapCols, rows: snapRows, cells: snapCells, in: ctx)
        }

        drawCursor(cols: snapCols, rows: snapRows, cells: snapCells, in: ctx)
        drawMarkedText(cols: snapCols, rows: snapRows, cells: snapCells, in: ctx)
    }

    /// Whether an input method's char takes two cells (East Asian wide ranges)
    private static func isWide(_ cp: UInt32) -> Bool {
        switch cp {
        case 0x1100...0x115f, 0x2e80...0x303e, 0x3041...0x33ff, 0x3400...0x4dbf, 0x4e00...0x9fff,
             0xa000...0xa4cf, 0xac00...0xd7a3, 0xf900...0xfaff, 0xfe30...0xfe4f, 0xff00...0xff60,
             0xffe0...0xffe6, 0x1f300...0x1f64f, 0x1f900...0x1f9ff, 0x20000...0x3fffd:
            return true
        default:
            return false
        }
    }

    /// Draws an input method's uncommitted text over the cells from the cursor, underlined
    private func drawMarkedText(cols: Int, rows: Int, cells: [Cell], in ctx: CGContext) {
        guard !markedText.isEmpty, cursorRow >= 0, cursorRow < rows, cursorCol >= 0, cursorCol < cols else { return }
        let idx = cursorRow * cols + cursorCol
        let cell = (idx < cells.count) ? cells[idx] : Cell()
        let fg = nsColor(index: Int(cell.fg)).cgColor
        let bg = nsColor(index: Int(cell.bg)).cgColor
        let cellY = CGFloat((rows - 1 - cursorRow) * MEView.cellH)
        let baseline = cellY - CGFloat(MEView.font.descender)
        var col = cursorCol
        for us in markedText.unicodeScalars {
            let nc = MEView.isWide(us.value) ? 2 : 1
            guard (us.value >= 0x20) && (col + nc <= cols) else { break }
            let rect = CGRect(x: CGFloat(col * MEView.cellW), y: cellY, width: CGFloat(nc * MEView.cellW), height: CGFloat(MEView.cellH))
            ctx.setFillColor(bg)
            ctx.fill(rect)
            let gr = MEView.glyphRef(us.value, attrs: 0, cells: nc)
            if (gr.font == 0) && (nc == 1) {
                var glyph = gr.glyph
                var pos = CGPoint(x: rect.minX, y: baseline)
                ctx.setFillColor(fg)
                CTFontDrawGlyphs(ctFontVariant(attrs: 0), &glyph, &pos, 1, ctx)
            } else {
                drawSingleGlyph(gr, attrs: 0, x: rect.minX, cellY: cellY, baseline: baseline, color: fg, in: ctx)
            }
            ctx.setFillColor(fg)
            ctx.fill(CGRect(x: rect.minX, y: cellY + 1, width: rect.width, height: 1))
            col += nc
        }
    }

    private func drawRow(_ row: Int, cols: Int, rows: Int, cells: [Cell], in ctx: CGContext) {
        let rowBase  = row * cols
        let cellY    = CGFloat((rows - 1 - row) * MEView.cellH)  // NSView y of cell bottom
        let baseline = cellY - CGFloat(MEView.font.descender)    // glyph origin y

        // ---- Pass 1: backgrounds -------------------------------------------
        // Merge consecutive cells sharing the same bg colour into one wider
        // fill, replacing N individual cell-sized fills with far fewer calls.
        var col = 0
        while col < cols {
            let bgIdx = Int(cells[rowBase + col].bg)
            var end   = col + 1
            while end < cols && Int(cells[rowBase + end].bg) == bgIdx { end += 1 }
            ctx.setFillColor(nsColor(index: bgIdx).cgColor)
            ctx.fill(CGRect(x: CGFloat(col * MEView.cellW), y: cellY,
                            width: CGFloat((end - col) * MEView.cellW), height: CGFloat(MEView.cellH)))
            col = end
        }

        // ---- Pass 2: text glyphs -------------------------------------------
        // Batch consecutive cells with the same fg colour and font variant into
        // a single CTFontDrawGlyphs call.  Special chars and spaces do NOT break
        // the run, they simply contribute no glyph (their cell is already filled
        // by pass 1 and will be drawn over in pass 3). A wide, fallback or
        // unavailable char is drawn on its own, fitted to its cells.
        col = 0
        while col < cols {
            let cell0  = cells[rowBase + col];
            let attrs0 = cell0.attrs & 0x03;   // bold + italic bits
            let fg0    = cell0.fg;
            let fgColor = nsColor(index: Int(fg0)).cgColor

            var runCount = 0;
            var c = col;
            while((c < cols) && (runCount < scratchGlyphs.count))
            {
                let cell = cells[rowBase + c];
                if(((cell.attrs & 0x03) != attrs0) || (cell.fg != fg0)) {
                    break;
                }
                if (cell.ch > 0x20) && (cell.ch != ME_FRAMECHAR_CONT)
                {
                    let cp = MEView.codePoint(cell.ch)
                    let wide = (c + 1 < cols) && (cells[rowBase + c + 1].ch == ME_FRAMECHAR_CONT)
                    if cp >= 0x20 {
                        let gr = MEView.glyphRef(cp, attrs: attrs0, cells: wide ? 2 : 1)
                        if (gr.font == 0) && !wide {
                            if gr.glyph != 0 {
                                scratchGlyphs[runCount]    = gr.glyph
                                scratchPositions[runCount] = CGPoint(x: CGFloat(c * MEView.cellW), y: baseline)
                                runCount += 1
                            }
                        } else {
                            drawSingleGlyph(gr, attrs: attrs0, x: CGFloat(c * MEView.cellW), cellY: cellY,
                                            baseline: baseline, color: fgColor, in: ctx)
                        }
                    }
                }
                c += 1
            }

            if runCount > 0 {
                ctx.setFillColor(fgColor)
                CTFontDrawGlyphs(ctFontVariant(attrs: attrs0),
                                 scratchGlyphs, scratchPositions, runCount, ctx)
            }
            col = c
        }

        // ---- Pass 3: specials and underlines --------------------------------
        for col in 0..<cols {
            let cell = cells[rowBase + col];
            if(cell.ch < 0x20) {
                drawSpecial(UInt8(cell.ch),
                            rect: cellRect(col: col, row: row, rows: rows),
                            fg: Int(cell.fg), in: ctx)
            }
            if((cell.attrs & 0x10) != 0)
            {
                let ulY = cellY + 1
                ctx.setFillColor(nsColor(index: Int(cell.fg)).cgColor)
                ctx.fill(CGRect(x: CGFloat(col * MEView.cellW), y: ulY,
                                width: CGFloat(MEView.cellW), height: 1))
            }
        }
    }

    /// Draw ME internal special glyphs (box drawing, arrows, etc.)
    private func drawSpecial(_ code: UInt8, rect: NSRect, fg: Int, in ctx: CGContext) {
        let fgColor = nsColor(index: fg);
        ctx.setStrokeColor(fgColor.cgColor);
        ctx.setFillColor(fgColor.cgColor);
        ctx.setLineWidth(1.0);

        let mx = rect.midX, my = rect.midY;
        let x0 = rect.minX, y0 = rect.minY;
        let x1 = rect.maxX, y1 = rect.maxY;
        let w = rect.width;

        // ME special character codes (from meFrameXTermDrawSpecialChar in unixterm.c)
        switch code {
        case 0x01:          /* unicode tag - 3 byte encode [u] */
            ctx.stroke(rect.insetBy(dx: 0.5, dy: 0.5));
            ctx.move(to: CGPoint(x: x0+2, y: my+2));
            ctx.addLine(to: CGPoint(x: x0+2, y: y0+3));
            ctx.move(to: CGPoint(x: x0+3, y: y0+2));
            ctx.addLine(to: CGPoint(x: x1-3, y: y0+2));
            ctx.move(to: CGPoint(x: x1-2, y: y0+3));
            ctx.addLine(to: CGPoint(x: x1-2, y: my+2));
            ctx.strokePath();
        
        case 0x02:          /* unicode tag - 5 byte encode [U] */
            ctx.stroke(rect.insetBy(dx: 0.5, dy: 0.5));
            ctx.move(to: CGPoint(x: x0+2, y: y1-3));
            ctx.addLine(to: CGPoint(x: x0+2, y: y0+3));
            ctx.move(to: CGPoint(x: x0+3, y: y0+2));
            ctx.addLine(to: CGPoint(x: x1-3, y: y0+2));
            ctx.move(to: CGPoint(x: x1-2, y: y0+3));
            ctx.addLine(to: CGPoint(x: x1-2, y: y1-3));
            ctx.strokePath();
        
        case 0x07:          /* meCHAR_UNDEF, undefined char - can be generated during X11 clipboard get or charset change <+> */
            ctx.move(to: CGPoint(x: mx, y: y0));
            ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.addLine(to: CGPoint(x: x0, y: my));
            ctx.addLine(to: CGPoint(x: mx, y: y0));
            ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.strokePath();
            ctx.move(to: CGPoint(x: x0, y: my));
            ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.strokePath();
            
        case 0x08:          /* Visible space - mid height dot '.' */
            let r: CGFloat = w * 0.15;
            ctx.fillEllipse(in: CGRect(x: mx - r, y: my - r, width: r*2, height: r*2))

        case 0x09:          /* Tab character -> */
            let sz = w * 0.375;
            ctx.move(to: CGPoint(x: x0, y: my));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.strokePath();
            let path = CGMutablePath();
            path.move(to:    CGPoint(x: x1, y: my));
            path.addLine(to: CGPoint(x: mx, y: my-sz));
            path.addLine(to: CGPoint(x: mx, y: my+sz));
            path.closeSubpath();
            ctx.addPath(path);
            ctx.fillPath();
            
        case 0x0a:          /* CR character / <-| */
            let sz = w * 0.375;
            ctx.move(to: CGPoint(x: x1-1, y: y1));
            ctx.addLine(to: CGPoint(x: x1-1, y: my));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.strokePath();
            let path = CGMutablePath();
            path.move(to:    CGPoint(x: x0, y: my));
            path.addLine(to: CGPoint(x: mx, y: my+sz));
            path.addLine(to: CGPoint(x: mx, y: my-sz));
            path.closeSubpath();
            ctx.addPath(path);
            ctx.fillPath();

        case 0x0b:          /* Line Drawing / Bottom right _| */
            ctx.move(to: CGPoint(x: mx, y: y1));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.addLine(to: CGPoint(x: x0, y: my));
            ctx.strokePath();

        case 0x0c:          /* Line Drawing / Top right */
            ctx.move(to: CGPoint(x: mx, y: y0));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.addLine(to: CGPoint(x: x0, y: my));
            ctx.strokePath();

        case 0x0d:          /* Line Drawing / Top left */
            ctx.move(to: CGPoint(x: mx, y: y0));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.strokePath();
        
        case 0x0e:          /* Line Drawing / Bottom left |_ */
            ctx.move(to: CGPoint(x: mx, y: y1));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.strokePath();
            
        case 0x0f:          /* Line Drawing / Centre cross + */
            ctx.move(to: CGPoint(x: x0, y: my)); ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.move(to: CGPoint(x: mx, y: y0)); ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.strokePath();

        case 0x10:          /* Cursor Arrows / Right */
            let path = CGMutablePath();
            path.move(to:    CGPoint(x: x1 - 2, y: my));
            path.addLine(to: CGPoint(x: x0 + 2, y: y0 + 2));
            path.addLine(to: CGPoint(x: x0 + 2, y: y1 - 2));
            path.closeSubpath();
            ctx.addPath(path);
            ctx.fillPath();
        
        case 0x11:          /* Cursor Arrows / Left */
            let path = CGMutablePath();
            path.move(to:    CGPoint(x: x0 + 2, y: my));
            path.addLine(to: CGPoint(x: x1 - 2, y: y0 + 2));
            path.addLine(to: CGPoint(x: x1 - 2, y: y1 - 2));
            path.closeSubpath();
            ctx.addPath(path);
            ctx.fillPath();

        case 0x12:          /* Line Drawing / Horizontal line - */
            ctx.move(to: CGPoint(x: x0, y: my));
            ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.strokePath();
                  
        case 0x13:          /* Unticked/empty checkbox ([ ]) */
            let sz = mx-x0;
            ctx.move(to: CGPoint(x: x0, y: my-sz));
            ctx.addLine(to: CGPoint(x: x1, y: my-sz));
            ctx.addLine(to: CGPoint(x: x1, y: my+sz));
            ctx.addLine(to: CGPoint(x: x0, y: my+sz));
            ctx.addLine(to: CGPoint(x: x0, y: my-sz));
            ctx.strokePath();
            
        case 0x14:          /* Ticked checkbox ([X]) */
            let sz = mx-x0;
            if(sz > 3)
            {
                ctx.move(to: CGPoint(x: x0, y: my-sz));
                ctx.addLine(to: CGPoint(x: x1, y: my-sz));
                ctx.addLine(to: CGPoint(x: x1, y: my+sz));
                ctx.addLine(to: CGPoint(x: x0, y: my+sz));
                ctx.addLine(to: CGPoint(x: x0, y: my-sz));
                ctx.strokePath();
                ctx.fill(CGRect(x: x0+2, y: my-sz+2,width: x1-x0-4, height: sz+sz-4));
            }
            else
            {
                ctx.fill(CGRect(x: x0, y: my-sz,width: x1-x0, height: sz+sz));
            }

        case 0x15:          /* Line Drawing / Left Tee |- */
            ctx.move(to: CGPoint(x: mx, y: y0)); ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.move(to: CGPoint(x: mx, y: my)); ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.strokePath();
            
        case 0x16:          /* Line Drawing / Right Tee -| */
            ctx.move(to: CGPoint(x: mx, y: y0)); ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.move(to: CGPoint(x: mx, y: my)); ctx.addLine(to: CGPoint(x: x0, y: my));
            ctx.strokePath();
            
        case 0x17:          /* Line Drawing / Bottom Tee _|_ */
            ctx.move(to: CGPoint(x: x0, y: my)); ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.move(to: CGPoint(x: mx, y: my)); ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.strokePath();
            
        case 0x18:          /* Line Drawing / Top Tee -|- */
            ctx.move(to: CGPoint(x: x0, y: my)); ctx.addLine(to: CGPoint(x: x1, y: my));
            ctx.move(to: CGPoint(x: mx, y: my)); ctx.addLine(to: CGPoint(x: mx, y: y0));
            ctx.strokePath();
            
        case 0x19:          /* Line Drawing / Vertical Line | */
            ctx.move(to: CGPoint(x: mx, y: y0)); ctx.addLine(to: CGPoint(x: mx, y: y1));
            ctx.strokePath();

        case 0x1a:          /* Line Drawing / Bottom right _| with resize */
            ctx.move(to: CGPoint(x: mx, y: y1));
            ctx.addLine(to: CGPoint(x: mx, y: my));
            ctx.addLine(to: CGPoint(x: x0, y: my));
            ctx.move(to: CGPoint(x: x0, y: y0));
            ctx.addLine(to: CGPoint(x: x1, y: y0+x1-x0));
            ctx.move(to: CGPoint(x: x0+2, y: y0));
            ctx.addLine(to: CGPoint(x: x1, y: y0+x1-x0-2));
            ctx.move(to: CGPoint(x: x0+4, y: y0));
            ctx.addLine(to: CGPoint(x: x1, y: y0+x1-x0-4));
            ctx.strokePath();
        
        case 0x1b:          /* Scroll box - vertical */
            ctx.setAlpha(0.3);
            ctx.fill(rect);
            ctx.setAlpha(1.0);
            
            /* 0x1c is difficult to use as its the meCHAR_LEADER char - only use if value not used in variables such as $window-chars */ 

        case 0x1d:          /* Scroll box - horizontal */
            ctx.setAlpha(0.3);
            ctx.fill(rect);
            ctx.setAlpha(1.0);

        case 0x1e:          /* Cursor Arrows / Up */
            let path = CGMutablePath();
            path.move(to:    CGPoint(x: mx, y: y1 - 2));
            path.addLine(to: CGPoint(x: x0, y: y0 + 2));
            path.addLine(to: CGPoint(x: x1, y: y0 + 2));
            path.closeSubpath();
            ctx.addPath(path);
            ctx.fillPath();
            
        case 0x1f:          /* Cursor Arrows / Down */
            let path = CGMutablePath();
            path.move(to:    CGPoint(x: mx, y: y0 + 2));
            path.addLine(to: CGPoint(x: x0, y: y1 - 2));
            path.addLine(to: CGPoint(x: x1, y: y1 - 2));
            path.closeSubpath();
            ctx.addPath(path);
            ctx.fillPath();
            
        default:
            break;
        }
    }

    private func drawCursor(cols: Int, rows: Int, cells: [Cell], in ctx: CGContext) {
        guard cursorVisible && cursorOn else { return }
        guard cursorCol >= 0 && cursorCol < cols &&
              cursorRow >= 0 && cursorRow < rows else { return }

        // a wide char's cursor covers both its cells
        let idx  = cursorRow * cols + cursorCol;
        guard idx < cells.count else { return }
        let cell = cells[idx];
        let wide = (cell.ch != ME_FRAMECHAR_CONT) && (cursorCol + 1 < cols) && (cells[idx + 1].ch == ME_FRAMECHAR_CONT)
        var rect = cellRect(col: cursorCol, row: cursorRow, rows: rows);
        if wide {
            rect.size.width *= 2
        }
        let cursorNSColor = nsColor(index: Int(meNativeGetCursorColor(Unmanaged.passUnretained(self).toOpaque())));

        if(hasFocus)
        {
            // Focused: filled block, character redrawn in cell background colour
            ctx.setFillColor(cursorNSColor.cgColor);
            ctx.fill(rect);
            let cp = MEView.codePoint(cell.ch)
            if (cell.ch > 0x20) && (cell.ch != ME_FRAMECHAR_CONT) && (cp >= 0x20)
            {
                let gr = MEView.glyphRef(cp, attrs: cell.attrs, cells: wide ? 2 : 1)
                let cellY    = CGFloat((rows - 1 - cursorRow) * MEView.cellH);
                let baseline = cellY - CGFloat(MEView.font.descender);
                let bgColor  = nsColor(index: Int(cell.bg)).cgColor
                if (gr.font == 0) && !wide {
                    if gr.glyph != 0 {
                        var glyph    = gr.glyph;
                        var position = CGPoint(x: rect.minX, y: baseline);
                        ctx.setFillColor(bgColor);
                        CTFontDrawGlyphs(ctFontVariant(attrs: cell.attrs),&glyph,&position,1,ctx);
                    }
                } else {
                    drawSingleGlyph(gr, attrs: cell.attrs, x: rect.minX, cellY: cellY, baseline: baseline,
                                    color: bgColor, in: ctx)
                }
            }
        }
        else
        {
            // Unfocused: 1-pixel outline rectangle in cursor colour, no fill
            ctx.setStrokeColor(cursorNSColor.cgColor);
            ctx.setLineWidth(1.0);
            ctx.stroke(rect.insetBy(dx: 0.5, dy: 0.5));
        }
    }

    // MARK: - Cursor blink  (driven by ME's TThandleBlink / CURSOR_TIMER_ID)
    // Swift no longer owns the blink schedule; TTshowCur -> cSetCursor sets
    // cursorOn=true, TThideCur -> cHideCursor sets cursorOn=false.

    func startCursorBlink() {
        hasFocus = true;
        setNeedsDisplay(cursorRect(col: cursorCol, row: cursorRow));
        meNativeViewFocusGained(Unmanaged.passUnretained(self).toOpaque());
    }

    func stopCursorBlink() {
        hasFocus = false;
        setNeedsDisplay(cursorRect(col: cursorCol, row: cursorRow));
        meNativeViewFocusLost(Unmanaged.passUnretained(self).toOpaque());
    }

    // MARK: - Flush: copy dirty rows from the C frame store into cells[]

    // Called on the main thread (via DispatchQueue.main.sync from meNativeViewFlushDirty).
    // The ME engine thread is blocked for the duration, so frameCur->store is stable.
    func cFlushDirty(rowMin: Int, rowMax: Int,
                     colStart: UnsafePointer<Int32>,
                     colEnd:   UnsafePointer<Int32>)
    {
        let frameCols = Int(meNativeGetFrameCols())
        guard frameCols > 0, frameCols <= cols, rowMax < rows else { return }

        // styleTable entry layout: fg=bits[7:0], bg=bits[15:8], attrs=bits[23:16]
        guard let stPtr = meNativeGetStyleTable() else { return }

        var dirtyMinY = CGFloat(rows * MEView.cellH)
        var dirtyMaxY = CGFloat(0)

        for row in rowMin...rowMax {
            let cs = Int(colStart[row])
            let ce = Int(colEnd[row])
            guard cs < ce else { continue }

            guard let textPtr   = meNativeGetRowChars(Int32(row)),
                  let schemePtr = meNativeGetRowScheme(Int32(row)) else { continue }

            let rowBase = row * cols
            let safeEnd = min(ce, cols)
            guard safeEnd > cs, rowBase + safeEnd <= cells.count else { continue }

            for col in cs..<safeEnd {
                let ch = textPtr[col]
                let sc = schemePtr[col]
                let st = stPtr[Int(sc) & 0x0fff]
                // meSCHEME_NOFONT (0x1000) suppresses font styling (bold/italic/underline)
                // for trailing spaces and other positions where ME strips font attributes.
                cells[rowBase + col] = Cell(
                    ch:     ch,
                    fg:     UInt8(st & 0xff),
                    bg:     UInt8((st >> 8) & 0xff),
                    attrs:  (sc & 0x1000) != 0 ? 0 : UInt8((st >> 16) & 0xff));
            }

            let cellMinY = CGFloat((rows - 1 - row) * MEView.cellH)
            let cellMaxY = cellMinY + CGFloat(MEView.cellH)
            if cellMinY < dirtyMinY { dirtyMinY = cellMinY }
            if cellMaxY > dirtyMaxY { dirtyMaxY = cellMaxY }
        }

        if dirtyMaxY > dirtyMinY {
            setNeedsDisplay(NSRect(x: 0, y: dirtyMinY,
                                   width: pixelWidth,
                                   height: dirtyMaxY - dirtyMinY))
        }
    }

    func cSetCursor(_ col: Int, _ row: Int) {
        // Called from ME engine thread - dispatch UI work to main thread
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.setNeedsDisplay(self.cursorRect(col: self.cursorCol, row: self.cursorRow))
            self.cursorCol = col
            self.cursorRow = row
            self.cursorOn  = true   // TTshowCur - cursor visible
            self.setNeedsDisplay(self.cursorRect(col: self.cursorCol, row: self.cursorRow))
        }
    }

    func cHideCursor() {
        // Called from ME engine thread - dispatch UI work to main thread
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.cursorOn = false   // TThideCur - blink-off phase
            self.setNeedsDisplay(self.cursorRect(col: self.cursorCol, row: self.cursorRow))
        }
    }

    // MARK: - Font change

    /// Called from the ME engine thread (via gCbs.changeFont).
    /// - If (n & 9) == 9: shows NSFontPanel and blocks until the user closes it.
    /// - If name is non-empty: applies that PostScript font synchronously.
    func cChangeFont(n: Int32, name: String, size: Float) {
        self.changeFontN = n;
        if (n & 9) == 9 {
            // Dialog mode: show NSFontPanel and block the ME engine thread until
            // the user closes the panel.
            let sema = DispatchSemaphore(value: 0);
            fontPanelSemaphore = sema;
            DispatchQueue.main.async { [weak self] in
                guard let self = self else { sema.signal(); return }
                let fm = NSFontManager.shared;
                fm.target  = self;
                fm.action  = #selector(MEView.fontPanelFontChanged(_:));
                NotificationCenter.default.addObserver(
                    self,
                    selector: #selector(MEView.fontPanelWillClose(_:)),
                    name: NSWindow.willCloseNotification,
                    object: NSFontPanel.shared);
                // The font panel must exist before it can highlight a selection,
                // so order it front first, then set the current font as selected.
                fm.orderFrontFontPanel(nil);
                fm.setSelectedFont(MEView.font, isMultiple: false);
            }
            sema.wait();
            fontPanelSemaphore = nil;
            return
        }
        // Named-font mode: apply the specified font synchronously.
        guard !name.isEmpty else { return }
        let block: () -> Void = { [weak self] in
            guard let self = self else { return }
            let pt = CGFloat(size);
            let newFont = NSFont(name: name, size: pt) ?? NSFont.monospacedSystemFont(ofSize: pt, weight: .regular);
            self.applyFont(newFont);
        }
        if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
    }

    /// Signalled by NSWindow.willCloseNotification for the shared font panel.
    @objc private func fontPanelWillClose(_ notification: Notification) {
        NotificationCenter.default.removeObserver(
            self, name: NSWindow.willCloseNotification, object: notification.object);
        fontPanelSemaphore?.signal();
    }

    /// Called by NSFontPanel when the user changes the font selection.
    @objc func fontPanelFontChanged(_ sender: Any?)
    {
        let fm = NSFontManager.shared;
        let newFont = fm.convert(MEView.font);
        let psName = newFont.fontName;
        let ptSize = Int32(newFont.pointSize);
        psName.withCString { meNativeReturnFontName(UnsafeMutablePointer(mutating: $0), ptSize) };
        applyFont(newFont);
    }

    /// Apply a resolved NSFont: update shared metrics once, then reset and resize all windows.
    private func applyFont(_ newFont: NSFont)
    {
        setFont(n: self.changeFontN, font: newFont);
        if((self.changeFontN & 2) == 0) {
            let bc = nsColor(index: Int(bgColorIdx)).cgColor;
            for wc in gWindowControllers {
                if let view = wc.meView {
                    view.layer?.backgroundColor = bc;
                    wc.fontDidChange(in: view);
                }
            }
        }
    }

    /// Called from the ME engine thread when $frame-width or $frame-depth is set.
    /// Resizes the cells buffer and the NSWindow to match the new dimensions.
    func cSetWindowSize(_ cols: Int, _ rows: Int)
    {
        let block: () -> Void = { [weak self] in
            guard let self = self else { return }
            let sz = cols * rows
            if sz > self.cols * self.rows {
                var blank = Cell();
                blank.bg = bgColorIdx;
                self.cells = Array(repeating: blank, count: sz);
            }
            self.cols = cols;
            self.rows = rows;
            if let wc = self.window?.windowController as? MEWindowController {
                wc.sizeWindow(to: self);
            }
            self.needsDisplay = true;
        }
        if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
    }

    func cSetTitle(_ title: String) {
        DispatchQueue.main.async { [weak self] in
                  self?.window?.title = title
        }
    }


    // MARK: - Keyboard events

    override func keyDown(with event: NSEvent) {
        // While an input method is composing every key goes to it
        if hasMarkedText() {
            interpretKeyEvents([event])
            return
        }
        let keyCode   = event.keyCode
        let modifiers = event.modifierFlags

        let isCtrl  = modifiers.contains(.control)
        let isAlt   = modifiers.contains(.option)
        let isShift = modifiers.contains(.shift)

        // ME_SHIFT / ME_CONTROL / ME_ALT for special keys only.
        // For printable chars shift is encoded in the character value itself.
        var specialMod: UInt32 = 0
        if isShift { specialMod |= MEMod.shift }
        if isCtrl  { specialMod |= MEMod.control }
        if isAlt   { specialMod |= MEMod.alt }

        // ---- Function / navigation keys (layout-independent) -------------
        // SKEY values come from meSKeys (C global) - correct for this build.
        let specialKey: UInt32?
        switch keyCode {
        case kVKUpArrow:        specialKey = meSKeys.up
        case kVKDownArrow:      specialKey = meSKeys.down
        case kVKLeftArrow:      specialKey = meSKeys.left
        case kVKRightArrow:     specialKey = meSKeys.right
        case kVKHome:           specialKey = meSKeys.home
        case kVKEnd:            specialKey = meSKeys.end_
        case kVKPageUp:         specialKey = meSKeys.pageUp
        case kVKPageDown:       specialKey = meSKeys.pageDown
        case kVKDelete:         specialKey = meSKeys.backspace
        case kVKForwardDelete:  specialKey = meSKeys.delete_
        case kVKEscape:         specialKey = meSKeys.esc
        case kVKReturn, kVKKPEnter: specialKey = meSKeys.ret
        case kVKTab:            specialKey = meSKeys.tab
        case kVKF1:             specialKey = meSKeys.f1
        case kVKF2:             specialKey = meSKeys.f2
        case kVKF3:             specialKey = meSKeys.f3
        case kVKF4:             specialKey = meSKeys.f4
        case kVKF5:             specialKey = meSKeys.f5
        case kVKF6:             specialKey = meSKeys.f6
        case kVKF7:             specialKey = meSKeys.f7
        case kVKF8:             specialKey = meSKeys.f8
        case kVKF9:             specialKey = meSKeys.f9
        case kVKF10:            specialKey = meSKeys.f10
        case kVKF11:            specialKey = meSKeys.f11
        case kVKF12:            specialKey = meSKeys.f12
        // Keypad navigation (NumLock off)
        case kVKKP1:            specialKey = meSKeys.kpEnd
        case kVKKP2:            specialKey = meSKeys.down
        case kVKKP3:            specialKey = meSKeys.kpPageDown
        case kVKKP4:            specialKey = meSKeys.left
        case kVKKP5:            specialKey = meSKeys.kpBegin
        case kVKKP6:            specialKey = meSKeys.right
        case kVKKP7:            specialKey = meSKeys.kpHome
        case kVKKP8:            specialKey = meSKeys.up
        case kVKKP9:            specialKey = meSKeys.kpPageUp
        case kVKKP0:            specialKey = meSKeys.insert
        case kVKKPDecimal:      specialKey = meSKeys.kpDelete
        default:                specialKey = nil
        }

        if let sk = specialKey {
            meNativePushKey(sk | specialMod)
            return
        }

        // ---- Regular character keys --------------------------------------
        // Plain and shifted keys go through the text input system so dead keys,
        // input methods and the character viewer work, the chars arrive in
        // insertText. For Ctrl/Alt use charactersIgnoringModifiers to get the base
        // letter; Cmd keys not taken by a menu are typed as before.
        if !isCtrl && !isAlt && !modifiers.contains(.command) {
            interpretKeyEvents([event])
            return
        }
        guard let chars = (isCtrl || isAlt)
                ? event.charactersIgnoringModifiers
                : event.characters,
              let scalar = chars.unicodeScalars.first
        else { return }

        let code = scalar.value

        if code >= 0x20 && code <= 0x7e {
            let keyVal: UInt32
            if isCtrl && code >= 0x61 && code <= 0x7a {
                // Ctrl+a-z -> C0 codes 0x01-0x1a, Alt may combine
                keyVal = (code ^ 0x60) | (specialMod & MEMod.alt)
            } else if isCtrl && code >= 0x40 && code <= 0x5f {
                // Ctrl+@, A-Z, [, \, ], ^, _ -> C0 codes 0x00-0x1f
                keyVal = (code ^ 0x40) | (specialMod & MEMod.alt)
            } else if isCtrl {
                // Ctrl+punctuation/digits: keep ME_CONTROL, strip ME_SHIFT
                keyVal = code | MEMod.control | (specialMod & MEMod.alt)
            } else {
                // Plain or Alt+char: shift already in character, never add ME_SHIFT
                keyVal = code | (specialMod & MEMod.alt)
            }
            meNativePushKey(keyVal)
            return
        }

        if code < 0x20 {
            // C0 from OS (defensive - shouldn't happen with Ctrl using ignoringModifiers)
            meNativePushKey(code | (specialMod & MEMod.alt))
            return
        }

        // A non-ASCII char (e.g. Alt with a national key), the C side converts it
        meNativePushChar(code, specialMod & (MEMod.control | MEMod.alt))
    }

    // MARK: - Text input (NSTextInputClient)
    // Typed text arrives in insertText. An input method's uncommitted (marked)
    // text is drawn at the cursor until it is committed or cancelled.

    private var markedText: String = ""

    override func doCommand(by selector: Selector) {
        // Keys are handled in keyDown, nothing to do (and no beep)
    }

    // MARK: - Mouse events

    override func mouseDown(with event: NSEvent)       { handleMouse(event, button: 1, pick: 1) }
    override func mouseUp(with event: NSEvent)         { handleMouse(event, button: 1, pick: 0) }
    override func rightMouseDown(with event: NSEvent)  { handleMouse(event, button: 3, pick: 1) }
    override func rightMouseUp(with event: NSEvent)    { handleMouse(event, button: 3, pick: 0) }
    override func otherMouseDown(with event: NSEvent)  { handleMouse(event, button: 2, pick: 1) }
    override func otherMouseUp(with event: NSEvent)    { handleMouse(event, button: 2, pick: 0) }

    override func mouseMoved(with event: NSEvent)     { handleMouseMove(event) }
    override func mouseDragged(with event: NSEvent)   { handleMouseMove(event) }
    override func rightMouseDragged(with event: NSEvent) { handleMouseMove(event) }
    override func otherMouseDragged(with event: NSEvent) { handleMouseMove(event) }

    override func scrollWheel(with event: NSEvent) {
        let pt   = convert(event.locationInWindow, from: nil)
        let col  = Int(pt.x) / MEView.cellW
        let row  = rows - 1 - Int(pt.y) / MEView.cellH
        guard col >= 0, col < cols, row >= 0, row < rows else { return }
        let mods = meModifiers(event)
        let up   = event.scrollingDeltaY > 0
        meNativeScrollWheel(up ? 1 : 0, Int32(col), Int32(row), mods)
    }

    private func handleMouse(_ event: NSEvent, button: Int32, pick: Int32) {
        let (col, row, dX, dY) = cellPos(event)
        guard col >= 0, col < cols, row >= 0, row < rows else { return }
        let mods = meModifiers(event)
        meNativePushMouseButton(button, pick, Int32(col), Int32(row),
                                Int32(dX), Int32(dY), mods)
        // Grab focus
        window?.makeFirstResponder(self)
    }

    private func handleMouseMove(_ event: NSEvent) {
        let (col, row, dX, dY) = cellPos(event)
        guard col >= 0, col < cols, row >= 0, row < rows else { return }
        let mods = meModifiers(event)
        meNativePushMouseMove(Int32(col), Int32(row), Int32(dX), Int32(dY), mods)
    }

    /// Returns (col, row, fractional-dX-0..255, fractional-dY-0..255)
    private func cellPos(_ event: NSEvent) -> (Int, Int, Int, Int) {
        let pt    = convert(event.locationInWindow, from: nil)
        let px    = Int(pt.x)
        let py    = Int(pixelHeight) - Int(pt.y)   // flip to top-left origin
        let col   = px / MEView.cellW
        let row   = py / MEView.cellH
        let dX    = (px - col * MEView.cellW) * 255 / max(1, MEView.cellW)
        let dY    = (py - row * MEView.cellH) * 255 / max(1, MEView.cellH)
        return (col, row, dX, dY)
    }

    private func meModifiers(_ event: NSEvent) -> UInt32 {
        var m: UInt32 = 0
        let f = event.modifierFlags
        if f.contains(.shift)   { m |= MEMod.shift }
        if f.contains(.control) { m |= MEMod.control }
        if f.contains(.option)  { m |= MEMod.alt }
        return m
    }

    // MARK: - Track mouse for mouseMoved events
    override func updateTrackingAreas() {
        super.updateTrackingAreas()
        trackingAreas.forEach { removeTrackingArea($0) }
        let ta = NSTrackingArea(rect: bounds,
                                options: [.mouseMoved, .activeInKeyWindow, .inVisibleRect],
                                owner: self, userInfo: nil)
        addTrackingArea(ta)
    }

    // MARK: - Become first responder on click
    override func becomeFirstResponder() -> Bool { true }
    override func resignFirstResponder() -> Bool { true }

    // MARK: - Drag and drop of files from the Finder

    private static let dropReadOptions: [NSPasteboard.ReadingOptionKey: Any] =
        [.urlReadingFileURLsOnly: true]

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
        sender.draggingPasteboard.canReadObject(forClasses: [NSURL.self],
                                                options: MEView.dropReadOptions) ? .copy : []
    }

    // Files are loaded into the frame & window under the drop point, as with
    // the X-Windows and MS-Windows versions.
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        guard let urls = sender.draggingPasteboard.readObjects(forClasses: [NSURL.self],
                                                               options: MEView.dropReadOptions) as? [URL],
              !urls.isEmpty else {
            return false
        }
        let pt  = convert(sender.draggingLocation, from: nil)
        let col = min(max(Int(pt.x) / MEView.cellW, 0), cols - 1)
        let row = min(max((Int(pixelHeight) - Int(pt.y)) / MEView.cellH, 0), rows - 1)
        let vp  = Unmanaged.passUnretained(self).toOpaque()
        for url in urls {
            meNativeDropFile(vp, Int32(col), Int32(row), url.path)
        }
        // Bring the dropped on window to the front so it can be edited
        NSApp.activate(ignoringOtherApps: true)
        window?.makeKeyAndOrderFront(nil)
        window?.makeFirstResponder(self)
        return true
    }
}

extension MEView: NSTextInputClient {

    func insertText(_ string: Any, replacementRange: NSRange) {
        let text = (string as? NSAttributedString)?.string ?? (string as? String) ?? ""
        if !markedText.isEmpty {
            markedText = ""
            setNeedsDisplay(bounds)
        }
        for us in text.unicodeScalars {
            switch us.value {
            case 0x0a, 0x0d: meNativePushKey(meSKeys.ret)
            case 0x09:       meNativePushKey(meSKeys.tab)
            case 0..<0x20:   meNativePushKey(us.value)
            default:         meNativePushChar(us.value, 0)
            }
        }
    }

    func setMarkedText(_ string: Any, selectedRange: NSRange, replacementRange: NSRange) {
        markedText = (string as? NSAttributedString)?.string ?? (string as? String) ?? ""
        setNeedsDisplay(bounds)
    }

    func unmarkText() {
        markedText = ""
        setNeedsDisplay(bounds)
    }

    func hasMarkedText() -> Bool {
        return !markedText.isEmpty
    }

    func markedRange() -> NSRange {
        return markedText.isEmpty ? NSRange(location: NSNotFound, length: 0)
                                  : NSRange(location: 0, length: (markedText as NSString).length)
    }

    func selectedRange() -> NSRange {
        return NSRange(location: 0, length: 0)
    }

    func validAttributesForMarkedText() -> [NSAttributedString.Key] {
        return []
    }

    func attributedSubstring(forProposedRange range: NSRange, actualRange: NSRangePointer?) -> NSAttributedString? {
        return nil
    }

    func characterIndex(for point: NSPoint) -> Int {
        return NSNotFound
    }

    /// The cursor's cell on the screen, where the input method puts its window
    func firstRect(forCharacterRange range: NSRange, actualRange: NSRangePointer?) -> NSRect {
        let rr = cellRect(col: cursorCol, row: cursorRow)
        guard let win = window else { return rr }
        return win.convertToScreen(convert(rr, to: nil))
    }
}

// ---------------------------------------------------------------------------
// MARK: - C callback trampolines
//
// C requires plain function pointers; these free functions capture the view
// via a global reference set by MEWindowController.
// ---------------------------------------------------------------------------

/// Reference to the active MEView — updated by meNativeSetActiveView on frame switches.
var gMEView: MEView? = nil

/// All MEWindowControllers (primary and secondary); keeps them alive.
var gWindowControllers: [MEWindowController] = []

// ---- Colour cache - NSColor objects keyed by MEColor palette index ---
// Main thread only: Swift dictionaries are not thread-safe and the cache is
// read by draw(), so changes requested by the ME engine thread are performed
// on the main queue. As the queue is serial these are always applied before
// the engine's next meNativeViewFlush (DispatchQueue.main.sync).
var colorCache: [Int: NSColor] = [:]
var bgColorIdx: UInt8 = 1

func nsColor(index: Int) -> NSColor {
    if let c = colorCache[index] { return c }
    let idx = index*3;
    var rr: CGFloat = 0;
    var gg: CGFloat = 0;
    var bb: CGFloat = 0;
    
    withUnsafeBytes(of: &meColorTable) { (tbl) -> Void in
        rr = CGFloat(tbl[idx]);
        gg = CGFloat(tbl[idx+1]);
        bb = CGFloat(tbl[idx+2]);
    }
    let c = NSColor(srgbRed: rr/255.0,green: gg/255.0,blue: bb/255.0,alpha: 1.0);
    colorCache[index] = c;
    return c
}

@_cdecl("meNativeBell")
func meNativeBell()
{
    DispatchQueue.main.async { NSSound.beep() }
}


// Run block on the main thread, asynchronously if called from another thread
private func meRunOnMain(_ block: @escaping () -> Void) {
    if Thread.isMainThread { block() } else { DispatchQueue.main.async(execute: block) }
}

// Main thread only
private func applyBgColor(_ bgColor: UInt8)
{
    bgColorIdx = bgColor;
    let bc = nsColor(index: Int(bgColor)).cgColor;
    for wc in gWindowControllers
    {
        if let view = wc.meView {
            view.layer?.backgroundColor = bc;
        }
    }
}

@_cdecl("meNativeSetBgColor")
func meNativeSetBgColor(_ bgColor: UInt8)
{
    meRunOnMain { applyBgColor(bgColor) }
}

@_cdecl("meNativeColorTableChanged")
func meNativeColorTableChanged() {
    meRunOnMain {
        colorCache.removeAll();
        applyBgColor(bgColorIdx);
    }
}

@_cdecl("meNativeCharsetChanged")
func meNativeCharsetChanged() {
    // The glyph tables are read by draw() so must be replaced on the main thread
    meRunOnMain { MEView.updateCharset() }
}

@_cdecl("meNativeViewChangeFont")
func meNativeViewChangeFont(_ viewPtr: UnsafeMutableRawPointer,_ n: Int32, _ name: UnsafePointer<CChar>?, _ size: Float) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
    let nameStr = name.map { String(cString: $0) } ?? "";
    view.cChangeFont(n: n, name: nameStr, size: size);
}

@_cdecl("meNativeViewSetTitle")
func meNativeViewSetTitle(_ viewPtr: UnsafeMutableRawPointer,_ title: UnsafePointer<CChar>) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
    view.cSetTitle(String(cString: title));
}

@_cdecl("meNativeViewSetCursor")
func meNativeViewSetCursor(_ viewPtr: UnsafeMutableRawPointer,_ col: Int32,_ row: Int32) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
    view.cSetCursor(Int(col), Int(row));
}

@_cdecl("meNativeViewHideCursor")
func meNativeViewHideCursor(_ viewPtr: UnsafeMutableRawPointer) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
    view.cHideCursor();
}

@_cdecl("meNativeViewSetWindowSize")
func meNativeViewSetWindowSize(_ viewPtr: UnsafeMutableRawPointer,_ cols: Int32,_ rows: Int32) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
    view.cSetWindowSize(Int(cols), Int(rows));
}

// ---------------------------------------------------------------------------
// MARK: - meNativeViewFlush  (@_cdecl - called by C TTflush)
//
// TTflush calls this with frameCur->termData so every flush is routed to the
// correct window even inside the screenUpdate multi-frame loop (where frameCur
// cycles through all frames but gMEView would otherwise stay fixed on the
// primary window).  Also used by pokeNotFocus to push [NOT FOCUS] into the
// OS-focused frame while frameCur->store holds that frame's data temporarily.
// ---------------------------------------------------------------------------

@_cdecl("meNativeViewFlush")
func meNativeViewFlush(_ viewPtr: UnsafeMutableRawPointer,
                       _ rowMin: Int32, _ rowMax: Int32,
                       _ colStart: UnsafePointer<Int32>,
                       _ colEnd:   UnsafePointer<Int32>)
{
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
    let rm = Int(rowMin), rx = Int(rowMax);
    let block = {
        view.cFlushDirty(rowMin: rm, rowMax: rx, colStart: colStart, colEnd: colEnd)
    }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
}

// ---------------------------------------------------------------------------
// MARK: - meNativeSetActiveView  (@_cdecl - called directly from C meFrameTermMakeCur)
//
// Redirects gMEView to the view that owns the frame becoming current.
// viewPtr is frame->termData; NULL means the primary frame (restore gInitialMEView).
// Dispatches synchronously to the main thread so gMEView is consistent before
// the next TTflush or callback on the engine thread.
// ---------------------------------------------------------------------------

@_cdecl("meNativeSetActiveView")
func meNativeSetActiveView(_ viewPtr: UnsafeMutableRawPointer) {
    let block = {
        gMEView = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue()
    }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
}

// ---------------------------------------------------------------------------
// MARK: - meNativeGetPrimaryView  (@_cdecl - called from C meFrameTermInit)
//
// The primary NSWindow+MEView is created by MEWindowController.launch() before
// the engine thread starts.  This just wraps the existing gMEView in a
// retained pointer so meFrameTermInit can store it in frame->termData like any
// other frame.  meNativeDestroyWindow will release it when the frame is freed.
// ---------------------------------------------------------------------------

@_cdecl("meNativeGetPrimaryView")
func meNativeGetPrimaryView() -> UnsafeMutableRawPointer {
    guard let view = gMEView else { fatalError("meNativeGetPrimaryView: no primary view") }
    return Unmanaged.passRetained(view).toOpaque()
}

// ---------------------------------------------------------------------------
// MARK: - meNativeCreateWindow  (@_cdecl - called from C meFrameTermInit)
//
// Creates a new NSWindow+MEView for an additional external frame.
// Dispatches to the main thread, returns a retained opaque MEView pointer.
// ---------------------------------------------------------------------------

@_cdecl("meNativeCreateWindow")
func meNativeCreateWindow(_ cols: Int32, _ rows: Int32) -> UnsafeMutableRawPointer? {
    var result: UnsafeMutableRawPointer? = nil
    let block = {
        let wc = MEWindowController()
        let view = wc.setupSecondary(cols: Int(cols), rows: Int(rows))
        gWindowControllers.append(wc)
        result = Unmanaged.passRetained(view).toOpaque()
    }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
    return result
}

// ---------------------------------------------------------------------------
// MARK: - meNativeDestroyWindow  (@_cdecl - called from C meFrameTermFree)
//
// Hides the NSWindow that owns viewPtr (orderOut avoids triggering the close
// delegates), removes its controller from gWindowControllers so ARC collects
// it, then releases the retained MEView pointer.
// ---------------------------------------------------------------------------

@_cdecl("meNativeDestroyWindow")
func meNativeDestroyWindow(_ viewPtr: UnsafeMutableRawPointer) {
    let block = {
        let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue()
        gWindowControllers.removeAll { $0.meView === view }
        view.window?.orderOut(nil)
        Unmanaged<MEView>.fromOpaque(viewPtr).release()
    }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
}
