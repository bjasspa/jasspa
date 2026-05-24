// MEAppDelegate.swift
// JASSPA MicroEmacs - native macOS frontend
//
// Application entry point.  Creates MEWindowController and passes any
// command-line arguments through to the ME engine.

import Cocoa
//import SwiftUI

@NSApplicationMain
class MEAppDelegate: NSObject, NSApplicationDelegate {

    private var windowController: MEWindowController?
    
    override init() {
        super.init();
    }
    func applicationWillFinishLaunching(_ aNotification: Notification) {
    }
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // ---- Parse command-line arguments --------------------------------
        // argv[0] is the executable path; forward everything to ME.
        let args = CommandLine.arguments

        // Build the window controller with sensible defaults.
        // Users can override font, size and geometry via environment variables
        // or future preferences:
        //   ME_FONT_NAME  - e.g. "Menlo-Regular"
        //   ME_FONT_SIZE  - e.g. "14"
        //   ME_COLS       - e.g. "100"
        //   ME_ROWS       - e.g. "40"

        let wc = MEWindowController()

        if let name = ProcessInfo.processInfo.environment["ME_FONT_NAME"] {
            wc.fontName = name
        }
        if let sizeStr = ProcessInfo.processInfo.environment["ME_FONT_SIZE"],
           let size = Double(sizeStr) {
            wc.fontSize = CGFloat(size)
        }
        if let colsStr = ProcessInfo.processInfo.environment["ME_COLS"],
           let cols = Int(colsStr) {
            wc.termCols = max(20, cols)
        }
        if let rowsStr = ProcessInfo.processInfo.environment["ME_ROWS"],
           let rows = Int(rowsStr) {
            wc.termRows = max(4, rows)
        }

        wc.engineArgv = args
        windowController = wc
        wc.launch()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        meNativeQuit()
    }
}
