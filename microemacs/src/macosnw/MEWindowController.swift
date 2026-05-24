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

        // Expose to C trampolines
        gMEView = view

        // ---- Configure view (measures font, sets mecm) -------------------
        let font = resolveFont()
        view.setFont(n: 2, font: font);
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

    private func sizeWindow(to view: MEView) {
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
        win.resizeIncrements = NSSize(width: view.cellW, height: view.cellH)
        win.minSize = NSSize(width:  CGFloat(view.cellW  * 10),
                             height: CGFloat(view.cellH  *  4))
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
    // MARK: - NSWindowDelegate
    // =========================================================================

    func windowWillClose(_ notification: Notification) {
        meView?.stopCursorBlink()
        meNativeQuit()
    }

    func windowDidResize(_ notification: Notification) {
        guard let view = meView, let win = window else { return }
        let contentSize = win.contentView!.bounds.size
        let newCols = max(10, Int(contentSize.width)  / view.cellW)
        let newRows = max(4,  Int(contentSize.height) / view.cellH)

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
        // Ask ME to do an orderly shutdown; let it close the window
        meNativeQuit()
        return false   // ME will call NSApp.terminate when ready
    }
}
