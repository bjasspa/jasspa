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

    // Files received via an open documents event (Finder Open With, Dock
    // drop etc) before the engine was started; passed on the command line.
    private var pendingOpenPaths: [String] = []
    
    override init() {
        super.init();
    }
    func applicationWillFinishLaunching(_ aNotification: Notification) {
        let env = ProcessInfo.processInfo.environment;
        if env["TERM"] == nil {
            // When launched from Finder, the Dock or Launchpad the app is spawned
            // by launchd and gets a minimal environment; the user's shell start-up
            // files (.zprofile, .zshrc etc) are never read so $PATH, $MENAME etc
            // are missing. Import the environment from the user's login shell.
            // This must be done before the ME engine thread starts as setenv is
            // not thread-safe and mesetup reads the environment.
            if(env["ME_NO_SHELL_ENV"] == nil) {
                importLoginShellEnvironment();
            }
            // launchd also starts the app in '/', start in the user's home
            // directory instead, as a terminal does. Only done when launched from
            // the GUI and the directory is '/' so an explicit start directory is
            // kept. $PWD is set as meSetupProgname uses it to get curdir.
            if(FileManager.default.currentDirectoryPath == "/") {
                let home = ProcessInfo.processInfo.environment["HOME"] ?? NSHomeDirectory();
                if(!home.isEmpty && (chdir(home) == 0)) {
                    setenv("PWD", home, 1);
                }
            }
        }
    }

    // Run the user's shell as an interactive login shell, get it to print its
    // environment and apply that to this process. The output is NUL separated
    // (env -0) and bracketed by markers so any noise printed by the start-up
    // files is ignored. Gives up after a timeout so a hanging .zshrc cannot
    // stop the editor from starting.
    private func importLoginShellEnvironment() {
        var shell = ""
        if let pw = getpwuid(getuid()), let sh = pw.pointee.pw_shell {
            shell = String(cString: sh)
        }
        if shell.isEmpty || !FileManager.default.isExecutableFile(atPath: shell) {
            shell = ProcessInfo.processInfo.environment["SHELL"] ?? ""
            if shell.isEmpty || !FileManager.default.isExecutableFile(atPath: shell) {
                shell = "/bin/zsh"
            }
        }
        let marker = "__ME_ENV_\(getpid())__"
        let markerData = marker.data(using: .utf8)!
        let proc = Process()
        proc.executableURL = URL(fileURLWithPath: shell)
        proc.arguments = ["-l", "-i", "-c",
                          "printf '\(marker)'; /usr/bin/env -0; printf '\(marker)'"]
        proc.standardInput = FileHandle.nullDevice
        proc.standardError = FileHandle.nullDevice
        let pipe = Pipe()
        proc.standardOutput = pipe

        // Read asynchronously and stop as soon as the closing marker is seen;
        // a background process started by the start-up files may inherit
        // stdout and keep the pipe open indefinitely.
        let lock = NSLock()
        var output = Data()
        var done = false
        let finished = DispatchSemaphore(value: 0)
        pipe.fileHandleForReading.readabilityHandler = { fh in
            let chunk = fh.availableData
            lock.lock()
            defer { lock.unlock() }
            if done { return }
            if chunk.isEmpty {
                done = true
            } else {
                output.append(chunk)
                if let r1 = output.range(of: markerData),
                   output.range(of: markerData, in: r1.upperBound..<output.endIndex) != nil {
                    done = true
                }
            }
            if done { finished.signal() }
        }
        do {
            try proc.run()
        } catch {
            pipe.fileHandleForReading.readabilityHandler = nil
            return
        }
        let timedOut = (finished.wait(timeout: .now() + 5.0) == .timedOut)
        pipe.fileHandleForReading.readabilityHandler = nil
        // Interactive shells ignore SIGTERM so use SIGKILL if it is stuck
        if timedOut && proc.isRunning {
            kill(proc.processIdentifier, SIGKILL)
        }
        lock.lock()
        done = true
        let data = output
        lock.unlock()

        guard let r1 = data.range(of: markerData),
              let r2 = data.range(of: markerData, in: r1.upperBound..<data.endIndex) else {
            NSLog("MicroEmacs: failed to import environment from %@", shell)
            return
        }
        let skip: Set<String> = ["PWD", "OLDPWD", "SHLVL", "_", "TERM"]
        for entry in data[r1.upperBound..<r2.lowerBound].split(separator: 0) {
            guard let eq = entry.firstIndex(of: UInt8(ascii: "=")),
                  let key = String(data: entry[entry.startIndex..<eq], encoding: .utf8),
                  let val = String(data: entry[(eq + 1)...], encoding: .utf8) else {
                continue
            }
            if key.isEmpty || skip.contains(key) || key.hasPrefix("__CF") || key.hasPrefix("XPC_") {
                continue
            }
            setenv(key, val, 1)
        }
    }
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // ---- Parse command-line arguments --------------------------------
        // argv[0] is the executable path; forward everything to ME.
        // Old versions of macOS add a -psn_X_Y process serial number argument
        // when launched by Finder, ME does not understand it.
        var args = CommandLine.arguments.filter { !$0.hasPrefix("-psn_") }
        args += pendingOpenPaths
        pendingOpenPaths = []

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

    // Finder "Open With", double-click on an associated file, drop onto the
    // Dock icon or "open -a MicroEmacs file". When launching, this is called
    // before applicationDidFinishLaunching so the files are added to the
    // engine's command line; otherwise they are passed to the running engine.
    func application(_ application: NSApplication, open urls: [URL]) {
        let paths = urls.filter { $0.isFileURL }.map { $0.path }
        if windowController == nil {
            // AppKit also reports files given on the command line as open
            // requests, skip these as ME will already load them.
            let argPaths = Set(CommandLine.arguments.dropFirst().map {
                URL(fileURLWithPath: $0).standardizedFileURL.path
            })
            for path in paths where !argPaths.contains(URL(fileURLWithPath: path).standardizedFileURL.path) {
                pendingOpenPaths.append(path)
            }
        } else {
            for path in paths {
                meNativeOpenFile(path)
            }
        }
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        meNativeQuit()
    }
}
