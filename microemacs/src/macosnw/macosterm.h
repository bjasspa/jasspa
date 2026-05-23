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
 *   ┌─────────────────────────────────────────────────────────┐
 *   │  Swift (MEView / MEWindowController / MEAppDelegate)    │
 *   │  • Owns NSWindow + NSView (MEView)                      │
 *   │  • Renders directly from the ME frame store at          │
 *   │    flush time (meNativeFlushDirty)                      │
 *   │  • Converts NSEvent keyboard/mouse → ME key codes       │
 *   │  • Calls meStartEngine() on a background thread         │
 *   └────────────────┬────────────────────────────────────────┘
 *                    │  C entry points declared below
 *   ┌────────────────▼────────────────────────────────────────┐
 *   │  macosterm.c  (implements the ME terminal interface)    │
 *   │  • TTputs / TTapplyArea  – accumulate per-row dirty     │
 *   │    column ranges (no drawing at this point)             │
 *   │  • TTflush               – calls meNativeFlushDirty,    │
 *   │    which (via main.sync) copies dirty rows from         │
 *   │    frameCur->store into the Swift render buffer and     │
 *   │    schedules setNeedsDisplay                            │
 *   │  • TTaddColor            – stores RGB palette           │
 *   │  • mecm populated from Swift font metrics               │
 *   └────────────────┬────────────────────────────────────────┘
 *                    │ calls ME core (compiled from src/.c)
 *   ┌────────────────▼────────────────────────────────────────┐
 *   │  ME core C code  (display.c, eval.c, main.c, …)        │
 *   └─────────────────────────────────────────────────────────┘
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

/* -------------------------------------------------------------------------
 * C → Swift callbacks for cursor, bell, title, font.
 * Registered via meNativeSetCallbacks at startup.
 * Drawing is no longer routed through callbacks; see meNativeFlushDirty.
 * ---------------------------------------------------------------------- */
typedef struct {
    /* Move the text cursor to (col, row) in cell coordinates. */
    void (*setCursor)(int col, int row);

    /* Hide the text cursor (blink-off phase). */
    void (*hideCursor)(void);

    /* Ring the terminal bell. */
    void (*bell)(void);

    /* Set the window title string (UTF-8). */
    void (*setTitle)(const char *title);

    /* Apply a new monospace font.  Called on the ME engine thread.
     * The Swift implementation MUST block until the font is applied and the
     * window resized (use DispatchQueue.main.sync).
     * name – PostScript font name, e.g. "Menlo-Regular".  Empty string means
     *         show the system font picker panel (async, returns immediately).
     * size – point size > 0.  0 means keep current size (dialog mode only). */
    void (*applyFont)(const char *name, float size);
} MENativeCallbacks;

/* Register the Swift-side callbacks.  Must be called before meStartEngine(). */
void meNativeSetCallbacks(const MENativeCallbacks *cbs);

/* -------------------------------------------------------------------------
 * Swift → C: font / geometry initialisation.
 * Called by MEView once the NSFont has been measured or window resized.
 *
 *  cols, rows    – initial terminal dimensions in characters
 *  cellW, cellH  – cell size in points (== pixels on non-Retina)
 *  ascent        – font ascent
 *  isMono        – 1 if the font is monospaced, 0 if proportional
 * ---------------------------------------------------------------------- */
void meNativeReturnFontSize(int cols, int rows, int cellW, int cellH, int ascent, int isMono);
void meNativeReturnFrameSize(int cols, int rows);

/* -------------------------------------------------------------------------
 * Swift → C: deliver an input event.
 * ---------------------------------------------------------------------- */
void meNativePushKey(uint16_t keyCode);
void meNativePushMouseMove(int col, int row, int dX, int dY, uint16_t modifiers);
void meNativePushMouseButton(int button, int pick, int col, int row, int dX, int dY, uint16_t modifiers);
void meNativeScrollWheel(int up, int col, int row, uint16_t modifiers);

/* -------------------------------------------------------------------------
 * Swift → C: window lifecycle events.
 * ---------------------------------------------------------------------- */
void meNativeQuit(void);
void meNativeFocusGained(void);
void meNativeFocusLost(void);

/* -------------------------------------------------------------------------
 * Swift → C: start the ME engine on a background thread.
 * ---------------------------------------------------------------------- */
void meStartEngine(int argc, char **argv);

/* -------------------------------------------------------------------------
 * Frame-store accessors.
 * These are called by Swift inside meNativeFlushDirty, while the ME engine
 * thread is blocked on DispatchQueue.main.sync, so access is safe.
 *
 *  meNativeGetRowText(row)    – pointer to frameCur->store[row].text
 *  meNativeGetRowScheme(row)  – pointer to frameCur->store[row].scheme
 *                               (uint16_t scheme indices)
 *  meNativeGetFrameCols()     – frameCur->width
 *  meNativeGetStyleTable()    – styleTable cast to const uint32_t *
 *                               Entry layout: fg=bits[7:0], bg=bits[15:8],
 *                               attrs=bits[23:16] (ME_ATTR_* flags)
 *                               Index with: scheme & 0x0fff
 * ---------------------------------------------------------------------- */
const uint8_t  *meNativeGetRowText  (int row);
const uint16_t *meNativeGetRowScheme(int row);
int             meNativeGetFrameCols(void);
const uint32_t *meNativeGetStyleTable(void);

/* -------------------------------------------------------------------------
 * C → Swift: flush dirty rows to the render buffer.
 * Implemented in MEView.swift with @_cdecl.
 *
 * Called by TTflush on the ME engine thread.  The Swift implementation
 * dispatches synchronously to the main thread (blocking the engine), copies
 * dirty rows from frameCur->store into the cells render buffer, then calls
 * setNeedsDisplay for the affected region.
 *
 *   rowMin, rowMax   – inclusive dirty row range
 *   colStart[r]      – first dirty column in row r  (INT_MAX if row clean)
 *   colEnd[r]        – one-past-last dirty column   (-1 if row clean)
 *                      arrays are valid for indices 0 … rowMax
 *   colorsDirty      – non-zero if the palette changed since last flush
 *   bgColor          – current background palette index
 * ---------------------------------------------------------------------- */
void meNativeFlushDirty(int rowMin, int rowMax,
                        const int *colStart, const int *colEnd,
                        int colorsDirty, unsigned char bgColor);

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
 * Colour palette – written by TTaddColor (C side), read by Swift.
 * ---------------------------------------------------------------------- */
extern unsigned char meColorTable[ME_MAX_COLORS*3];

/* Current cursor colour palette index – resolved in TTshowCur. */
extern unsigned char meCursorColor;

/* Background palette index for the global scheme – updated by TTsetBgcol(). */
extern unsigned char meNWBgColor;

/* Set to 1 by TTaddColor when a palette entry changes; cleared by Swift. */
extern int meNWColorsDirty;

/* -------------------------------------------------------------------------
 * C → Swift: clipboard / pasteboard bridge.
 * Implemented in MEClipboard.swift with @_cdecl.
 * ---------------------------------------------------------------------- */
int   meNativeGetPasteboardChangeCount(void);
char *meNativeGetClipboard(void);
int   meNativeSetClipboard(const char *text, int len);

#ifdef __cplusplus
}
#endif

#endif /* MACOSTERM_H */
