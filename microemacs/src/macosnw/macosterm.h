/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * macosterm.h - Native macOS (NSWindow) terminal interface header.
 *
 * Copyright (C) 2024 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
/*
 * Synopsis:
 *   Header shared between the C core (macosterm.c) and the Swift wrapper.
 *   Declared with plain C linkage so it can be included both from C and from
 *   Swift via the bridging header.
 *
 * Architecture overview:
 *
 *   +---------------------------------------------------------+
 *   |  Swift (MEView / MEWindowController / MEAppDelegate)    |
 *   |  - Owns NSWindow + NSView (MEView)                      |
 *   |  - Renders directly from the ME frame store at          |
 *   |    flush time (meNativeViewFlush)                       |
 *   |  - Converts NSEvent keyboard/mouse - ME key codes       |
 *   |  - Calls meStartEngine() on a background thread         |
 *   +----------------+----------------------------------------+
 *                    |  C entry points declared below
 *   +----------------+----------------------------------------+
 *   |  macosterm.c  (implements the ME terminal interface)    |
 *   |  - TTputs / TTapplyArea  - accumulate per-row dirty     |
 *   |    column ranges (no drawing at this point)             |
 *   |  - TTflush  - calls meNativeViewFlush(termData,..)      |
 *   |    which (via main.sync) copies dirty rows from         |
 *   |    frameCur->store into that frame's render buffer and  |
 *   |    schedules setNeedsDisplay on the correct view        |
 *   |  - TTaddColor            - stores RGB palette           |
 *   |  - mecm populated from Swift font metrics               |
 *   +----------------+----------------------------------------+
 *                    | calls ME core (compiled from src/.c)
 *   +----------------+----------------------------------------+
 *   |  ME core C code  (display.c, eval.c, main.c, ...)       |
 *   +---------------------------------------------------------+
 */

#ifndef MACOSTERM_H
#define MACOSTERM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Maximum palette entries ME will ever allocate (matches meColor = meUByte) */
#define ME_MAX_COLORS 256

/* -------------------------------------------------------------------------
 * Cell attribute flags (bits 23:16 of meStyle, shifted down 16).
 * These mirror the meFONT_* constants in estruct.h.
 * ---------------------------------------------------------------------- */
#define ME_ATTR_BOLD       0x01
#define ME_ATTR_ITALIC     0x02
#define ME_ATTR_UNDERLINE  0x10

void meNativeBell(void);

void meNativeSetBgColor(unsigned char bgColor);
void meNativeColorTableChanged(void);
void meNativeCharsetChanged(void);

/* -------------------------------------------------------------------------
 * Swift - C: font / geometry initialisation.
 * Called by MEView once the NSFont has been measured or window resized.
 *
 *  cols, rows:    initial terminal dimensions in characters
 *  cellW, cellH:  cell size in points (== pixels on non-Retina)
 *  ascent:        font ascent
 *  isMono:        1 if the font is monospaced, 0 if proportional
 * ---------------------------------------------------------------------- */
void meNativeReturnFontName(char *name, int ptSize);
void meNativeReturnFontSize(int cellW, int cellH, int ascent, int isMono);
/* viewPtr identifies which frame to resize; NULL during initial startup. */
void meNativeReturnViewFrameSize(void *viewPtr, int cols, int rows);

/* -------------------------------------------------------------------------
 * Swift - C: deliver an input event.
 * ---------------------------------------------------------------------- */
void meNativePushKey(uint16_t keyCode);
void meNativePushMouseMove(int col, int row, int dX, int dY, uint16_t modifiers);
void meNativePushMouseButton(int button, int pick, int col, int row, int dX, int dY, uint16_t modifiers);
void meNativeScrollWheel(int up, int col, int row, uint16_t modifiers);

/* -------------------------------------------------------------------------
 * Swift - C: window lifecycle events.
 * ---------------------------------------------------------------------- */
void meStartEngine(int argc, char **argv);
void meNativeQuit(void);

/* -------------------------------------------------------------------------
 * C - Swift: return a retained pointer to the already-created primary view.
 * Called by meFrameTermInit for the very first frame (frameList==NULL).
 * Every frame's termData is always a valid non-NULL MEView pointer.
 * ---------------------------------------------------------------------- */
void *meNativeGetPrimaryView(void);

/* -------------------------------------------------------------------------
 * C - Swift: create a new NSWindow+MEView for an additional external frame.
 * Called by meFrameTermInit when frameList!=NULL and sibling==NULL.
 * Returns a retained opaque pointer stored in frame->termData.
 * ---------------------------------------------------------------------- */
void *meNativeCreateWindow(int cols, int rows);

/* -------------------------------------------------------------------------
 * C - Swift: destroy the NSWindow+MEView owned by this frame.
 * Called by meFrameTermFree when sibling==NULL.
 * Hides the window, removes its controller from the keeper registry, and
 * releases the retained MEView pointer stored in frame->termData.
 * ---------------------------------------------------------------------- */
void meNativeDestroyWindow(void *viewPtr);

/* -------------------------------------------------------------------------
 * C - Swift: redirect rendering to the view owned by the given frame.
 * Called by meFrameTermMakeCur on the ME engine thread.
 * viewPtr is always a valid non-NULL MEView pointer.
 * ---------------------------------------------------------------------- */
void meNativeSetActiveView(void *viewPtr);

/* -------------------------------------------------------------------------
 * C - Swift: resize the render buffer of a specific view.
 * Called from the TTwaitForChar resize handler to update the correct view
 * when a secondary window is resized while it is not the active frame.
 * Routes cSetWindowSize to viewPtr rather than the global gMEView.
 * ---------------------------------------------------------------------- */
void meNativeViewSetWindowSize(void *viewPtr, int cols, int rows);

/* Set the window title string (UTF-8). */
void meNativeViewSetTitle(void *viewPtr, const char *title);

/* Move the text cursor to (col, row) in cell coordinates. */
void meNativeViewSetCursor(void *viewPtr, int col, int row);

/* Hide the text cursor (blink-off phase). */
void meNativeViewHideCursor(void *viewPtr);

void meNativeViewFocusLost(void *viewPtr);

/* -------------------------------------------------------------------------
 * Swift - C: a view just gained keyboard focus.
 * Called by MEView.startCursorBlink on the main thread.
 * viewPtr is an unretained pointer to the view that received focus; the
 * engine uses it to switch frameCur when a different frame owns that view.
 * ---------------------------------------------------------------------- */
void meNativeViewFocusGained(void *viewPtr);

/* -------------------------------------------------------------------------
 * Swift - C: user clicked the close button on a window that is not the last.
 * Called by windowShouldClose on the main thread when other frames exist.
 * The engine finds the frame owning viewPtr and runs delete-frame on it.
 * ---------------------------------------------------------------------- */
void meNativeDeleteFrame(void *viewPtr);

/* -------------------------------------------------------------------------
 * Swift - C: start the ME engine on a background thread.
 * ---------------------------------------------------------------------- */

/* Change the font.  Called on the ME engine thread.
 * n:    Arg n, if bits 1+8 set then show the system font-picker panel and
 *       block until the user closes it; font changes are applied live.
 * name: PostScript font name, e.g. "Menlo-Regular".
 * size: point size > 0.  0 means keep current size (dialog mode only). */
void meNativeViewChangeFont(void *viewPtr, int n, const char *name, float size);

/* -------------------------------------------------------------------------
 * Frame-store accessors.
 * These are called by Swift inside meNativeViewFlush, while the ME engine
 * thread is blocked on DispatchQueue.main.sync, so access is safe.
 *
 *  meNativeGetRowText(row)    - pointer to frameCur->store[row].text
 *  meNativeGetRowScheme(row)  - pointer to frameCur->store[row].scheme
 *                               (uint16_t scheme indices)
 *  meNativeGetFrameCols()     - frameCur->width
 *  meNativeGetStyleTable()    - styleTable cast to const uint32_t *
 *                               Entry layout: fg=bits[7:0], bg=bits[15:8],
 *                               attrs=bits[23:16] (ME_ATTR_* flags)
 *                               Index with: scheme & 0x0fff
 * ---------------------------------------------------------------------- */
const uint8_t  *meNativeGetRowText(int row);
const uint16_t *meNativeGetRowScheme(int row);
int             meNativeGetFrameCols(void);
uint8_t         meNativeGetCursorColor(void *viewPtr);
const uint32_t *meNativeGetStyleTable(void);

/* -------------------------------------------------------------------------
 * C - Swift: flush view dirty rows to the render buffer.
 * Implemented in MEView.swift with @_cdecl.
 *
 * Called by TTflush on the ME engine thread.  The Swift implementation
 * dispatches synchronously to the main thread (blocking the engine), copies
 * dirty rows from frameCur->store into the cells render buffer, then calls
 * setNeedsDisplay for the affected region.
 *
 *   rowMin, rowMax   - inclusive dirty row range
 *   colStart[r]      - first dirty column in row r  (INT_MAX if row clean)
 *   colEnd[r]        - one-past-last dirty column   (-1 if row clean)
 *                      arrays are valid for indices 0 ... rowMax
 *   dirtyFlag        - Bit flag indicating if palette of charset changed since last flush
 *   bgColor          - current background palette index
 * ---------------------------------------------------------------------- */
#define ME_COLORS_DIRTY   0x01
#define ME_CHARSET_DIRTY  0x02
void meNativeViewFlush(void *viewPtr, int rowMin, int rowMax,
                       const int *colStart, const int *colEnd);

extern unsigned short mouseKeyState;

/* -------------------------------------------------------------------------
 * ME modifier bits and special-key code table.
 * ---------------------------------------------------------------------- */
#define ME_MOD_SHIFT    0x0100u
#define ME_MOD_CONTROL  0x0200u
#define ME_MOD_ALT      0x0400u
#define ME_SPECIAL_BIT  0x0800u

typedef struct {
    uint16_t backspace;
    uint16_t delete_;
    uint16_t down;
    uint16_t end_;
    uint16_t esc;
    uint16_t f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12;
    uint16_t home;
    uint16_t insert;
    uint16_t left;
    uint16_t pageDown;
    uint16_t pageUp;
    uint16_t ret;
    uint16_t right;
    uint16_t tab;
    uint16_t up;
    uint16_t kpBegin;
    uint16_t kpDelete;
    uint16_t kpEnd;
    uint16_t kpHome;
    uint16_t kpPageDown;
    uint16_t kpPageUp;
} MESKeyTable;

extern MESKeyTable meSKeys;

/* -------------------------------------------------------------------------
 * Charset - Char -> Unicode mapping, read by Swift.
 * ---------------------------------------------------------------------- */
extern uint16_t charToUnicode[128];

/* -------------------------------------------------------------------------
 * Colour palette - written by TTaddColor (C side), read by Swift.
 * ---------------------------------------------------------------------- */
extern unsigned char meColorTable[ME_MAX_COLORS*3];

/* -------------------------------------------------------------------------
 * C  Swift: clipboard / pasteboard bridge.
 * Implemented in MEClipboard.swift with @_cdecl.
 * ---------------------------------------------------------------------- */
int   meNativeGetPasteboardChangeCount(void);
char *meNativeGetClipboard(void);
int   meNativeSetClipboard(const char *text, int len);

#ifdef __cplusplus
}
#endif

#endif /* MACOSTERM_H */
