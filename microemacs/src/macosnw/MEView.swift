// MEView.swift
// JASSPA MicroEmacs - native macOS frontend
//
// Owns the NSView that ME renders into.  Responsibilities:
//   - Measure the monospace font and tell macosterm.c the cell metrics
//   - Register MENativeCallbacks (setCursor, bell, title, font)
//   - Implement meNativeFlushDirtyView (@_cdecl): copies dirty rows from
//     frameCur->store into the cells[] render buffer, then calls setNeedsDisplay
//   - Translate NSEvent keyboard and mouse events into ME key codes
//   - Forward events to macosterm.c via meNativePushKey / meNativePushMouseButton

import Cocoa
import CoreText

// ---------------------------------------------------------------------------
// MARK: - ME modifier bits  (from estruct.h via macosterm.h ME_MOD_* defines)
// ---------------------------------------------------------------------------
// Special-key codes are read from meSKeys (a C global) so they always match
// the SKEY_* enum values in the current build.

private enum MEMod {
    static let shift:   UInt16 = UInt16(ME_MOD_SHIFT)
    static let control: UInt16 = UInt16(ME_MOD_CONTROL)
    static let alt:     UInt16 = UInt16(ME_MOD_ALT)
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
private struct Cell {
    var char8:  UInt8  = 0x20   // ISO-8859-1 value stored by ME
    var fg:     UInt8  = 0
    var bg:     UInt8  = 1
    var attrs:  UInt8  = 0
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

    // ---- Colour cache - NSColor objects keyed by MEColor palette index ---
    private var colorCache: [Int: NSColor] = [:]

    // ---- Layer background tracking - updated in cFlushDirty (main thread) --
    private var layerBgIndex: Int = -1
    
    // TODO SSP - this should be the current ME charset. 
    // ---- Glyph cache - pre-computed for all 256 ISO-8859-1 chars ---------
    // Built once per font change; shared across all windows.
    private static var glyphs:           [CGGlyph] = Array(repeating: 0, count: 256)
    private static var boldGlyphs:       [CGGlyph] = Array(repeating: 0, count: 256)
    private static var italicGlyphs:     [CGGlyph] = Array(repeating: 0, count: 256)
    private static var boldItalicGlyphs: [CGGlyph] = Array(repeating: 0, count: 256)

    // ---- Scratch buffers reused each drawRow render call (main thread only) ------
    private var scratchGlyphs:    [CGGlyph] = Array(repeating: 0, count: 512)
    private var scratchPositions: [CGPoint] = Array(repeating: .zero, count: 512)

    // ---- Initialisation ---------------------------------------------------

    override init(frame: NSRect) {
        super.init(frame: frame)
        commonInit(registerCallbacks: true)
    }

    /// Secondary init: no C callbacks registered (primary view owns them).
    init(secondaryFrame frame: NSRect) {
        super.init(frame: frame)
        commonInit(registerCallbacks: false)
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit(registerCallbacks: true)
    }

    private func commonInit(registerCallbacks: Bool) {
        wantsLayer = true;
        layer?.backgroundColor = NSColor.black.cgColor;

        guard registerCallbacks else { return }
        var cbs = MENativeCallbacks();
        cbs.bell          = meViewBell;
        cbs.changeFont    = meViewChangeFont;
        cbs.setTitle      = meViewSetTitle;
        cbs.setCursor     = meViewSetCursor;
        cbs.hideCursor    = meViewHideCursor;
        cbs.setWindowSize = meViewSetWindowSize;
        meNativeSetCallbacks(&cbs);
    }

    /// Initialise a secondary view's cell buffer; font/glyph/cell metrics are shared statics.
    func copyMetrics(from source: MEView, cols: Int, rows: Int) {
        self.cols = cols
        self.rows = rows
        var blank = Cell()
        blank.bg = UInt8(meNWBgColor)
        cells = Array(repeating: blank, count: cols * rows)
    }

    func setSize(cols: Int, rows: Int)
    {
        let sz = cols * rows;
        if (sz > (self.cols * self.rows))
        {
            var blank = Cell();
            blank.bg = UInt8(meNWBgColor);
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
            MEView.glyphs = makeGlyphTable(for: font);
            MEView.boldGlyphs = makeGlyphTable(for: MEView.boldFont);
            MEView.italicGlyphs = makeGlyphTable(for: MEView.italicFont);
            MEView.boldItalicGlyphs = makeGlyphTable(for: MEView.boldItalicFont);
            MEView.cellW = w;
            MEView.cellH = h;
        }
    }

    private func makeGlyphTable(for nsFont: NSFont) -> [CGGlyph]
    {
        var chars = (0..<256).map { UniChar($0) }
        var g     = [CGGlyph](repeating: 0, count: 256)
        CTFontGetGlyphsForCharacters(nsFont as CTFont, &chars, &g, 256)
        return g
    }

    private func ctFontVariant(attrs: UInt8) -> CTFont
    {
        switch (attrs & 0x03)
        {
        case 0:  return MEView.font as CTFont;
        case 1:  return MEView.italicFont as CTFont;
        case 2:  return MEView.boldFont as CTFont;
        default: return MEView.boldItalicFont as CTFont;
        }
    }

    private func glyphTable(for attrs: UInt8) -> [CGGlyph]
    {
        switch(attrs & 0x03)
        {
        case 0:  return MEView.glyphs;
        case 1:  return MEView.italicGlyphs;
        case 2:  return MEView.boldGlyphs;
        default: return MEView.boldItalicGlyphs;
        }
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

    // MARK: - Colour helpers

    private func nsColor(index: Int) -> NSColor {
        if let c = colorCache[index] { return c }
        let idx = index*3;
        var rr: CGFloat = 0;
        var gg: CGFloat = 0;
        var bb: CGFloat = 0;
        
        withUnsafeMutablePointer(to: &meColorTable) { (tuplePtr) -> Void in
                  tuplePtr.withMemoryRebound(to: UInt8.self, capacity: MemoryLayout.size(ofValue: meColorTable)) {
                      rr = CGFloat($0[idx])
                      gg = CGFloat($0[idx+1])
                      bb = CGFloat($0[idx+2])
                        }
        }
        let c = NSColor(srgbRed:  rr/255.0,
                        green:    gg/255.0,
                        blue:     bb/255.0,
                        alpha:    1.0)
        colorCache[index] = c
        return c
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
        // by pass 1 and will be drawn over in pass 3).
        col = 0
        while col < cols {
            let cell0  = cells[rowBase + col];
            let attrs0 = cell0.attrs & 0x03;   // bold + italic bits
            let fg0    = cell0.fg;
            let gtable = glyphTable(for: attrs0);

            var runCount = 0;
            var c = col;
            while((c < cols) && (runCount < scratchGlyphs.count))
            {
                let cell = cells[rowBase + c];
                if(((cell.attrs & 0x03) != attrs0) || (cell.fg != fg0)) {
                    break;
                }
                if(cell.char8 > 0x20)
                {
                    let g = gtable[Int(cell.char8)]
                    if g != 0 {
                        scratchGlyphs[runCount]    = g
                        scratchPositions[runCount] = CGPoint(x: CGFloat(c * MEView.cellW), y: baseline)
                        runCount += 1
                    }
                }
                c += 1
            }

            if runCount > 0 {
                ctx.setFillColor(nsColor(index: Int(fg0)).cgColor)
                CTFontDrawGlyphs(ctFontVariant(attrs: attrs0),
                                 scratchGlyphs, scratchPositions, runCount, ctx)
            }
            col = c
        }

        // ---- Pass 3: specials and underlines --------------------------------
        for col in 0..<cols {
            let cell = cells[rowBase + col];
            if(cell.char8 < 0x20) {
                drawSpecial(cell.char8,
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

        let rect = cellRect(col: cursorCol, row: cursorRow, rows: rows);
        let cursorNSColor = nsColor(index: Int(meCursorColor));

        if(hasFocus)
        {
            // Focused: filled block, character redrawn in cell background colour
            ctx.setFillColor(cursorNSColor.cgColor);
            ctx.fill(rect);
            let idx  = cursorRow * cols + cursorCol;
            if(idx < cells.count)
            {
                let cell = cells[idx];
                if (cell.char8 > 0x20)
                {
                    let g = glyphTable(for: cell.attrs)[Int(cell.char8)];
                    if(g != 0)
                    {
                        let cellY    = CGFloat((rows - 1 - cursorRow) * MEView.cellH);
                        let baseline = cellY - CGFloat(MEView.font.descender);
                        var glyph    = g;
                        var position = CGPoint(x: rect.minX, y: baseline);
                        ctx.setFillColor(nsColor(index: Int(cell.bg)).cgColor);
                        CTFontDrawGlyphs(ctFontVariant(attrs: cell.attrs),&glyph,&position,1,ctx);
                    }
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
        setNeedsDisplay(cellRect(col: cursorCol, row: cursorRow));
        meNativeViewFocusGained(Unmanaged.passUnretained(self).toOpaque());
    }

    func stopCursorBlink() {
        hasFocus = false;
        setNeedsDisplay(cellRect(col: cursorCol, row: cursorRow));
        meNativeViewFocusLost(Unmanaged.passUnretained(self).toOpaque());
    }

    // MARK: - Flush: copy dirty rows from the C frame store into cells[]

    // Called on the main thread (via DispatchQueue.main.sync from meNativeFlushDirtyView).
    // The ME engine thread is blocked for the duration, so frameCur->store is stable.
    func cFlushDirty(rowMin: Int, rowMax: Int,
                     colStart: UnsafePointer<Int32>,
                     colEnd:   UnsafePointer<Int32>,
                     colorsDirty: Bool, bgColor: UInt8) {

        if colorsDirty {
            colorCache.removeAll()
            layerBgIndex = -1
        }
        let bgIdx = Int(bgColor)
        if bgIdx != layerBgIndex {
            layerBgIndex = bgIdx
            layer?.backgroundColor = nsColor(index: bgIdx).cgColor
        }

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

            guard let textPtr   = meNativeGetRowText(Int32(row)),
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
                    char8:  ch,
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
            self.setNeedsDisplay(self.cellRect(col: self.cursorCol, row: self.cursorRow))
            self.cursorCol = col
            self.cursorRow = row
            self.cursorOn  = true   // TTshowCur - cursor visible
            self.setNeedsDisplay(self.cellRect(col: self.cursorCol, row: self.cursorRow))
        }
    }

    func cHideCursor() {
        // Called from ME engine thread - dispatch UI work to main thread
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.cursorOn = false   // TThideCur - blink-off phase
            self.setNeedsDisplay(self.cellRect(col: self.cursorCol, row: self.cursorRow))
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
                fm.setSelectedFont(MEView.font, isMultiple: false);
                NotificationCenter.default.addObserver(
                    self,
                    selector: #selector(MEView.fontPanelWillClose(_:)),
                    name: NSWindow.willCloseNotification,
                    object: NSFontPanel.shared);
                fm.orderFrontFontPanel(nil);
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
    @objc func fontPanelFontChanged(_ sender: Any?) {
        let fm = NSFontManager.shared;
        let newFont = fm.convert(MEView.font);
        let psName = newFont.fontName;
        let ptSize = Int32(newFont.pointSize);
        psName.withCString { meNativeReturnFontName(UnsafeMutablePointer(mutating: $0), ptSize) };
        applyFont(newFont);
    }

    /// Apply a resolved NSFont: update shared metrics once, then reset and resize all windows.
    private func applyFont(_ newFont: NSFont) {
        setFont(n: self.changeFontN, font: newFont);
        if((self.changeFontN & 2) == 0)
        {
            for wc in gWindowControllers {
                if let view = wc.meView {
                    view.layerBgIndex = -1;
                    wc.fontDidChange(in: view);
                }
            }
        }
    }

    /// Called from the ME engine thread when $frame-width or $frame-depth is set.
    /// Resizes the cells buffer and the NSWindow to match the new dimensions.
    func cSetWindowSize(_ cols: Int, _ rows: Int) {
        let block: () -> Void = { [weak self] in
            guard let self = self else { return }
            let sz = cols * rows
            if sz > self.cols * self.rows {
                var blank = Cell()
                blank.bg = UInt8(meNWBgColor)
                self.cells = Array(repeating: blank, count: sz)
            }
            self.cols = cols
            self.rows = rows
            if let wc = self.window?.windowController as? MEWindowController {
                wc.sizeWindow(to: self)
            }
            self.needsDisplay = true
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
        let keyCode   = event.keyCode
        let modifiers = event.modifierFlags

        let isCtrl  = modifiers.contains(.control)
        let isAlt   = modifiers.contains(.option)
        let isShift = modifiers.contains(.shift)

        // ME_SHIFT / ME_CONTROL / ME_ALT for special keys only.
        // For printable chars shift is encoded in the character value itself.
        var specialMod: UInt16 = 0
        if isShift { specialMod |= MEMod.shift }
        if isCtrl  { specialMod |= MEMod.control }
        if isAlt   { specialMod |= MEMod.alt }

        // ---- Function / navigation keys (layout-independent) -------------
        // SKEY values come from meSKeys (C global) - correct for this build.
        let specialKey: UInt16?
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
        // For Ctrl/Alt use charactersIgnoringModifiers to get the base letter.
        // For plain/shift keys use characters - shift is already in the char.
        guard let chars = (isCtrl || isAlt)
                ? event.charactersIgnoringModifiers
                : event.characters,
              let scalar = chars.unicodeScalars.first
        else { return }

        let code = scalar.value

        if code >= 0x20 && code <= 0x7e {
            let keyVal: UInt16
            if isCtrl && code >= 0x61 && code <= 0x7a {
                // Ctrl+a-z -> C0 codes 0x01-0x1a, Alt may combine
                keyVal = UInt16(code ^ 0x60) | (specialMod & MEMod.alt)
            } else if isCtrl && code >= 0x40 && code <= 0x5f {
                // Ctrl+@, A-Z, [, \, ], ^, _ -> C0 codes 0x00-0x1f
                keyVal = UInt16(code ^ 0x40) | (specialMod & MEMod.alt)
            } else if isCtrl {
                // Ctrl+punctuation/digits: keep ME_CONTROL, strip ME_SHIFT
                keyVal = UInt16(code) | MEMod.control | (specialMod & MEMod.alt)
            } else {
                // Plain or Alt+char: shift already in character, never add ME_SHIFT
                keyVal = UInt16(code) | (specialMod & MEMod.alt)
            }
            meNativePushKey(keyVal)
            return
        }

        if code < 0x20 {
            // C0 from OS (defensive - shouldn't happen with Ctrl using ignoringModifiers)
            meNativePushKey(UInt16(code) | (specialMod & MEMod.alt))
            return
        }

        if code <= 0xFFFF {
            meNativePushKey(UInt16(code) | (specialMod & MEMod.alt))
        }
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

    private func meModifiers(_ event: NSEvent) -> UInt16 {
        var m: UInt16 = 0
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

private func meViewBell() {
    DispatchQueue.main.async { NSSound.beep() }
}

private func meViewChangeFont(_ viewPtr: UnsafeMutableRawPointer?,_ n: Int32, _ name: UnsafePointer<CChar>?, _ size: Float) {
    if let viewPtr = viewPtr {
        let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
        let nameStr = name.map { String(cString: $0) } ?? "";
        view.cChangeFont(n: n, name: nameStr, size: size);
    }
}

private func meViewSetTitle(_ viewPtr: UnsafeMutableRawPointer?,_ title: UnsafePointer<CChar>?) {
    if let viewPtr = viewPtr, let title = title {
        let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
        view.cSetTitle(String(cString: title));
    }
}

private func meViewSetCursor(_ viewPtr: UnsafeMutableRawPointer?,_ col: Int32,_ row: Int32) {
    if let viewPtr = viewPtr {
        let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
        view.cSetCursor(Int(col), Int(row));
    }
}

private func meViewHideCursor(_ viewPtr: UnsafeMutableRawPointer?) {
    if let viewPtr = viewPtr {
        let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
        view.cHideCursor();
    }
}

private func meViewSetWindowSize(_ viewPtr: UnsafeMutableRawPointer?,_ cols: Int32,_ rows: Int32) {
    if let viewPtr = viewPtr {
        let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue();
        view.cSetWindowSize(Int(cols), Int(rows));
    }
}

// ---------------------------------------------------------------------------
// MARK: - meNativeFlushDirtyView  (@_cdecl - called by C TTflush)
//
// TTflush calls this with frameCur->termData so every flush is routed to the
// correct window even inside the screenUpdate multi-frame loop (where frameCur
// cycles through all frames but gMEView would otherwise stay fixed on the
// primary window).  Also used by pokeNotFocus to push [NOT FOCUS] into the
// OS-focused frame while frameCur->store holds that frame's data temporarily.
// ---------------------------------------------------------------------------

@_cdecl("meNativeFlushDirtyView")
func meNativeFlushDirtyView(_ viewPtr: UnsafeMutableRawPointer,
                            _ rowMin: Int32, _ rowMax: Int32,
                            _ colStart: UnsafePointer<Int32>,
                            _ colEnd:   UnsafePointer<Int32>,
                            _ colorsDirty: Int32,
                            _ bgColor: UInt8) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue()
    let rm = Int(rowMin), rx = Int(rowMax)
    let cd = colorsDirty != 0
    let bg = bgColor
    let block = {
        view.cFlushDirty(rowMin: rm, rowMax: rx,
                         colStart: colStart, colEnd: colEnd,
                         colorsDirty: cd, bgColor: bg)
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

@_cdecl("meNativeSetViewWindowSize")
func meNativeSetViewWindowSize(_ viewPtr: UnsafeMutableRawPointer, _ cols: Int32, _ rows: Int32) {
    let view = Unmanaged<MEView>.fromOpaque(viewPtr).takeUnretainedValue()
    view.cSetWindowSize(Int(cols), Int(rows))
}

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
