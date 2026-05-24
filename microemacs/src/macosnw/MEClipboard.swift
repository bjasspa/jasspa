// MEClipboard.swift
// JASSPA MicroEmacs - native macOS frontend
//
// Thin NSPasteboard wrappers exported to C via @_cdecl.
// Called from TTinitClipboard / TTgetClipboard / TTsetClipboard in macosnw.c.
//
// Threading: the ME engine runs on a background thread.  All NSPasteboard
// access must be on the main thread; each function dispatches there with
// sync so the engine thread blocks until the operation completes.

import Cocoa

// ---------------------------------------------------------------------------
// MARK: - meNativeGetPasteboardChangeCount
// ---------------------------------------------------------------------------
// Returns NSPasteboard.general.changeCount, incremented by the system every
// time any app (including us) writes to the pasteboard.  macosnw.c records
// the count from its own last write so it can tell whether another app has
// since put something new on the pasteboard.

@_cdecl("meNativeGetPasteboardChangeCount")
func meNativeGetPasteboardChangeCount() -> Int32 {
    var cc: Int32 = -1
    let block = { cc = Int32(NSPasteboard.general.changeCount) }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
    return cc
}

// ---------------------------------------------------------------------------
// MARK: - meNativeGetClipboard
// ---------------------------------------------------------------------------
// Reads NSPasteboard.general and returns its plain-text content as a
// NUL-terminated UTF-8 C string allocated with strdup().
// The CALLER is responsible for free()ing the returned pointer.
// Returns NULL when the pasteboard is empty or holds no plain text.

@_cdecl("meNativeGetClipboard")
func meNativeGetClipboard() -> UnsafeMutablePointer<CChar>? {
    var result: UnsafeMutablePointer<CChar>? = nil
    let block = {
        guard let str = NSPasteboard.general.string(forType: .string),
              !str.isEmpty else { return }
        result = strdup(str)
    }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
    return result
}

// ---------------------------------------------------------------------------
// MARK: - meNativeSetClipboard
// ---------------------------------------------------------------------------
// Writes len bytes of UTF-8 text to NSPasteboard.general and returns the
// new changeCount so macosnw.c can track pasteboard ownership.

@_cdecl("meNativeSetClipboard")
func meNativeSetClipboard(_ text: UnsafePointer<CChar>?, _ len: Int32) -> Int32 {
    guard let text = text, len > 0 else { return -1 }
    // Build a Swift String from the raw UTF-8 bytes without copying the buffer.
    let str = String(bytesNoCopy: UnsafeMutableRawPointer(mutating: text),
                     length: Int(len), encoding: .utf8,
                     freeWhenDone: false) ?? String(cString: text)
    var cc: Int32 = -1
    let block = {
        let pb = NSPasteboard.general
        pb.clearContents()
        pb.setString(str, forType: .string)
        cc = Int32(pb.changeCount)
    }
    if Thread.isMainThread { block() } else { DispatchQueue.main.sync(execute: block) }
    return cc
}
