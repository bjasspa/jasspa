// MEWindowController.swift
// JASSPA MicroEmacs - native macOS frontend
//
// Creates and owns the NSWindow + MEView.  Measures the monospace font,
// sizes the window to the configured terminal dimensions, and starts the
// ME engine on a dedicated background thread.

import Cocoa

final class MEWindowController: NSWindowController, NSWindowDelegate {

    // ---- Configuration ---------------------------------------------------
    /// Font family name; nil = system monospaced font
    var fontName:   String? = nil
    var fontSize:   CGFloat = 14.0
    var termCols:   Int     = 80
    var termRows:   Int     = 50

    // ---- The view --------------------------------------------------------
    private(set) var meView: MEView!

    // ---- Engine thread ---------------------------------------------------
    private var engineThread: Thread?

    // ---- Command-line arguments forwarded to ME --------------------------
    var engineArgv: [String] = []

    // =========================================================================
    // MARK: - Initialisation
    // =========================================================================

    convenience init() {
        // Build NSWindow programmatically - no nib required
        let contentRect = NSRect(x: 0, y: 0, width: 640, height: 800)
        let style: NSWindow.StyleMask = [.titled, .closable, .miniaturizable, .resizable]
        let win = NSWindow(contentRect: contentRect,
                           styleMask: style,
                           backing: .buffered,
                           defer: false)
        win.title                     = "MicroEmacs"
        win.isReleasedWhenClosed      = false
        win.backgroundColor           = .black
        win.collectionBehavior        = [.fullScreenPrimary]
        self.init(window: win)
    }

    // Called from MEAppDelegate.applicationDidFinishLaunching
    func launch() {
        // init(window:) marks isWindowLoaded=true so loadWindow() is a no-op
        // and windowDidLoad() is never called automatically; call setup() directly.
        setup()
        showWindow(nil)
        window?.center()
        window?.makeKeyAndOrderFront(nil)
        startEngine()
    }

    // =========================================================================
    // MARK: - Setup
    // =========================================================================

    private func setup() {
        guard let win = window else { return }
        win.delegate = self

        // ---- Build MEView -------------------------------------------------
        let view = MEView(frame: win.contentView!.bounds)
        view.autoresizingMask = [.width, .height]
        win.contentView!.addSubview(view)
        meView = view

        // Expose to C trampolines; add to keeper so ARC doesn't collect this controller.
        gMEView = view
        gWindowControllers.append(self)

        // ---- Configure view (measures font, sets mecm) -------------------
        let font = resolveFont()
        view.setFont(n: 1, font: font);
        view.setSize(cols: termCols, rows: termRows);

        // ---- Size window to terminal dimensions --------------------------
        sizeWindow(to: view)

        // ---- Cursor blink ------------------------------------------------
        view.startCursorBlink()
    }

    private func resolveFont() -> NSFont {
        let size = fontSize
        if let name = fontName, let f = NSFont(name: name, size: size) {
            return f
        }
        // Prefer "Menlo", fall back to system monospaced
        if let menlo = NSFont(name: "Menlo-Regular", size: size) { return menlo }
        return NSFont.monospacedSystemFont(ofSize: size, weight: .regular)
    }

    /// Called by MEView after a font change to resize the window to the new cell metrics.
    func fontDidChange(in view: MEView) {
        sizeWindow(to: view)
    }

    func sizeWindow(to view: MEView) {
        guard let win = window else { return }
        let w = view.pixelWidth
        let h = view.pixelHeight
        // Ask NSWindow for the frame rect that gives the desired content size
        let contentSize = NSSize(width: w, height: h)
        let frameSize   = win.frameRect(forContentRect:
                              NSRect(origin: .zero, size: contentSize)).size
        // Keep the window's current top-left position
        let curFrame = win.frame
        let newOrigin = NSPoint(x: curFrame.minX,
                                y: curFrame.maxY - frameSize.height)
        win.setFrame(NSRect(origin: newOrigin, size: frameSize), display: false)
        win.resizeIncrements = NSSize(width: MEView.cellW, height: MEView.cellH)
        win.minSize = NSSize(width:  CGFloat(MEView.cellW  * 10),
                             height: CGFloat(MEView.cellH  *  4))
    }

    // =========================================================================
    // MARK: - Engine thread
    // =========================================================================

    private func startEngine() {
        let argv = engineArgv
        let t = Thread {
            // Build C argc/argv
            var cStrings = argv.map { strdup($0) }
            cStrings.append(nil)   // sentinel
            let argc = Int32(argv.count)
            cStrings.withUnsafeMutableBufferPointer { buf in
                meStartEngine(argc, buf.baseAddress)
            }
            // ME exited - clean up strings
            for p in cStrings where p != nil { free(p) }
            // Terminate the app on the main thread
            DispatchQueue.main.async { NSApp.terminate(nil) }
        }
        t.name             = "MEEngineThread"
        t.qualityOfService = .userInteractive
        t.start()
        engineThread = t
    }

    // =========================================================================
    // MARK: - Secondary window (Chunk 1)
    // =========================================================================

    /// Creates window+view for a secondary external frame.
    /// Copies font metrics from the primary view; does NOT start an engine
    /// thread and does NOT call back into C (no meNativeReturnFrameSize etc.).
    /// Returns the new MEView (caller retains it via Unmanaged.passRetained).
    func setupSecondary(cols: Int, rows: Int) -> MEView {
        guard let win = window, let primaryView = gMEView else {
            fatalError("setupSecondary: called before primary window is ready")
        }
        win.delegate = self

        let view = MEView(frame: win.contentView!.bounds)
        view.autoresizingMask = [.width, .height]
        win.contentView!.addSubview(view)
        meView = view

        // Copy font and cell geometry from primary without triggering C callbacks.
        view.copyMetrics(from: primaryView, cols: cols, rows: rows)

        sizeWindow(to: view)
        win.center()
        win.makeKeyAndOrderFront(nil)

        return view
    }

    // =========================================================================
    // MARK: - NSWindowDelegate
    // =========================================================================

    func windowWillClose(_ notification: Notification) {
        meView?.stopCursorBlink()
        meNativeQuit()
    }

    func windowDidResize(_ notification: Notification) {
        guard let view = meView, let win = window else { return }
        let contentSize = win.contentView!.bounds.size
        let newCols = max(10, Int(contentSize.width)  / MEView.cellW)
        let newRows = max(4,  Int(contentSize.height) / MEView.cellH)

        guard newCols != view.cols || newRows != view.rows else { return }

        view.setSize(cols: newCols, rows: newRows)
    }

    func windowDidBecomeKey(_ notification: Notification) {
        // Defer to next run-loop cycle: AppKit may call this during its own
        // layout pass; setNeedsDisplay on a layer-backed view then triggers
        // a recursive layoutSubtreeIfNeeded.
        DispatchQueue.main.async { [weak self] in self?.meView?.startCursorBlink() }
    }

    func windowDidResignKey(_ notification: Notification) {
        DispatchQueue.main.async { [weak self] in self?.meView?.stopCursorBlink() }
    }

    func windowShouldClose(_ sender: NSWindow) -> Bool {
        if gWindowControllers.count > 1, let view = meView {
            // Other windows exist: ask ME to delete just this frame.
            meNativeDeleteFrame(Unmanaged.passUnretained(view).toOpaque())
        } else {
            // Last window: ask ME for an orderly shutdown.
            meNativeQuit()
        }
        return false   // ME owns the window lifecycle
    }
}
