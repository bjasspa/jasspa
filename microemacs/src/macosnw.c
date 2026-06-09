/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * macosterm.c - Native macOS (NSWindow) terminal backend.
 *
 * Copyright (C) 2024 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * Synopsis:
 *   Implements the ME terminal interface for a native macOS NSWindow.
 *
 *   Build flags required:
 *     -D_MACOS -D_UNIX -D_POSIX_SIGNALS -D_ME_WINDOW -D_ME_NATIVE -D_MOUSE
 *
 * Architecture:
 *   - SIGALRM  - ME's timer system (termio.c) uses setitimer(ITIMER_REAL).
 *                sigAlarm() (declared extern in eterm.h) is implemented here
 *                and signals the pthread condvar to wake waitForEvent().
 *
 *   - Input    - A lock-protected circular buffer.  Swift pushes events via
 *                meNativePush*; the ME engine thread drains them in
 *                drainQueue() -> addKeyToBuffer().
 *
 *   - Drawing  - TTputs/TTapplyArea mark per-row dirty column ranges.
 *                TTflush calls meNativeFlushDirtyView(frameCur->termData, ...)
 *                (Swift @_cdecl) which, via DispatchQueue.main.sync, copies
 *                dirty rows from the frame store into the Swift cells[] render
 *                buffer and schedules setNeedsDisplay on that frame's view.
 */

/* =========================================================================
 * Includes (after the macros above so emain.h sees __MACOSTERMINAL)
 * ====================================================================== */
#include "emain.h"
#include "evers.h"
#include "efunc.h"
#include "eskeys.h"
#include "macosnw/macosterm.h"

#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <mach-o/dyld.h>   /* _NSGetExecutablePath */
#include <CoreGraphics/CoreGraphics.h>

struct  termios otermio;        /* original terminal characteristics */
struct  termios ntermio;        /* charactoristics to use inside */
typedef struct termios *meTermio ;

/* =========================================================================
 * Globals exported to Swift (declared in macosterm.h)
 * ====================================================================== */

/* Setup the macosterm variables and things */
meCellMetrics mecm;

/* =========================================================================
 * Mouse button state bookkeeping
 *
 * These macros/data are defined as static locals in unixterm.c.  Since we
 * are not compiling unixterm.c we must provide them here.
 * ====================================================================== */
/* mouseKeyState - The state of the control keys when the mouse was pressed. */
meUShort mouseKeyState;          /* State of keyboard control */

/* mouseKeys maps physical button index to allows swap. */
static meUShort mouseKeys[8] = { 0, 1, 2, 3, 4, 5, 0, 0 };

#define mouseButtonPick(bb)  (mouseState |=  (1<<((bb)-1)))
#define mouseButtonDrop(bb)  (mouseState &= ~(1<<((bb)-1)))
#define mouseButtonGetPick() (((mouseState & MOUSE_STATE_BUTTONS) == 0) ? 0 : \
     (mouseState & MOUSE_STATE_LEFT)   ? 1 : (mouseState & MOUSE_STATE_RIGHT) ? 3 : \
     (mouseState & MOUSE_STATE_MIDDLE) ? 2 : (mouseState & MOUSE_STATE_BUT4)  ? 4 : 5)


/* Special-key code table, read by Swift so it never hard-codes SKEY values */
MESKeyTable meSKeys = {
    .backspace  = ME_SPECIAL | SKEY_backspace,
    .delete_    = ME_SPECIAL | SKEY_delete,
    .down       = ME_SPECIAL | SKEY_down,
    .end_       = ME_SPECIAL | SKEY_end,
    .esc        = ME_SPECIAL | SKEY_esc,
    .f1         = ME_SPECIAL | SKEY_f1,
    .f2         = ME_SPECIAL | SKEY_f2,
    .f3         = ME_SPECIAL | SKEY_f3,
    .f4         = ME_SPECIAL | SKEY_f4,
    .f5         = ME_SPECIAL | SKEY_f5,
    .f6         = ME_SPECIAL | SKEY_f6,
    .f7         = ME_SPECIAL | SKEY_f7,
    .f8         = ME_SPECIAL | SKEY_f8,
    .f9         = ME_SPECIAL | SKEY_f9,
    .f10        = ME_SPECIAL | SKEY_f10,
    .f11        = ME_SPECIAL | SKEY_f11,
    .f12        = ME_SPECIAL | SKEY_f12,
    .home       = ME_SPECIAL | SKEY_home,
    .insert     = ME_SPECIAL | SKEY_insert,
    .left       = ME_SPECIAL | SKEY_left,
    .pageDown   = ME_SPECIAL | SKEY_page_down,
    .pageUp     = ME_SPECIAL | SKEY_page_up,
    .ret        = ME_SPECIAL | SKEY_return,
    .right      = ME_SPECIAL | SKEY_right,
    .tab        = ME_SPECIAL | SKEY_tab,
    .up         = ME_SPECIAL | SKEY_up,
    .kpBegin    = ME_SPECIAL | SKEY_kp_begin,
    .kpDelete   = ME_SPECIAL | SKEY_kp_delete,
    .kpEnd      = ME_SPECIAL | SKEY_kp_end,
    .kpHome     = ME_SPECIAL | SKEY_kp_home,
    .kpPageDown = ME_SPECIAL | SKEY_kp_page_down,
    .kpPageUp   = ME_SPECIAL | SKEY_kp_page_up,
};

meUByte meColorTable[ME_MAX_COLORS*3];

/* ---- Thread-safe input queue -------------------------------------------
 * Swift event thread writes; ME engine thread reads.
 * Protected by gInputMutex.
 * ---------------------------------------------------------------------- */
#define INPUT_QUEUE_SIZE 512

static meUShort        gInputQueue[INPUT_QUEUE_SIZE];
static int             gInputHead  = 0;
static int             gInputTail  = 0;
static int             gInputCount = 0;
static pthread_mutex_t gInputMutex = PTHREAD_MUTEX_INITIALIZER;

/* ---- Resize / quit / focus flags (written by Swift, read by ME engine) -*/
static volatile int   gQuitPending      = 0;
static volatile int   gFocusChange      = 0;
static volatile void *gFocusedView      = NULL;
static volatile int   gResizePending    = 0;
static volatile int   gResizeCols       = 80;
static volatile int   gResizeRows       = 24;
static volatile void *gResizeView       = NULL;
static volatile void *gDeleteFrameView  = NULL;

/* ---- Self-pipe - used to wake select() from any thread or signal --------
 * gWakePipe[0] = read end (watched by ME engine in waitForEvent)
 * gWakePipe[1] = write end (written by Swift threads and sigAlarm)
 * The write end is non-blocking so it is safe from signal handlers.
 * ---------------------------------------------------------------------- */
static int gWakePipe[2] = { -1, -1 };

/* Wake the ME engine's select() - safe from any thread or signal handler. */
static void
wakeSelect(void)
{
    char b = 1;
    (void)write(gWakePipe[1], &b, 1);
}

/* Drain the read end of the wake pipe after select() returns. */
static void
drainWakePipe(void)
{
    char buf[64];
    while (read(gWakePipe[0], buf, sizeof(buf)) > 0)
        ;
}

/* ---- Per-row dirty column ranges - accumulated by TTputs/TTapplyArea ----
 * At TTflush time the accumulated ranges are handed to meNativeFlushDirtyView,
 * which (via DispatchQueue.main.sync) copies the dirty cells from the frame
 * store into that frame's Swift render buffer and schedules setNeedsDisplay.
 *
 * colStart[r] = INT_MAX and colEnd[r] = -1 means row r is clean.
 * ---------------------------------------------------------------------- */
#define ME_DIRTY_NONE  0x7fffffff
#define ME_DIRTY_CLEAR (-1)

static int gDirtyColStart[meFRAME_DEPTH_MAX + 1];
static int gDirtyColEnd  [meFRAME_DEPTH_MAX + 1];
static int gDirtyRowMin  = ME_DIRTY_NONE;
static int gDirtyRowMax  = ME_DIRTY_CLEAR;

static void
clearDirty(void)
{
    int r;
    for(r = gDirtyRowMin ; r <= gDirtyRowMax ; r++) {
        gDirtyColStart[r] = ME_DIRTY_NONE;
        gDirtyColEnd[r]   = ME_DIRTY_CLEAR;
    }
    gDirtyRowMin = ME_DIRTY_NONE;
    gDirtyRowMax = ME_DIRTY_CLEAR;
}

static void
meNativeMarkDirty(int row, int startCol, int endCol)
{
    if(!frameCur || row < 0 || row > (int)frameCur->depth)
        return;
    if (startCol < gDirtyColStart[row]) gDirtyColStart[row] = startCol;
    if (endCol   > gDirtyColEnd[row])   gDirtyColEnd[row]   = endCol;
    if (row < gDirtyRowMin) gDirtyRowMin = row;
    if (row > gDirtyRowMax) gDirtyRowMax = row;
}

/* =========================================================================
 * SIGALRM handler
 *
 * ME's timer system (termio.c) uses setitimer(ITIMER_REAL, ...) + SIGALRM.
 * sigAlarm is declared extern in eterm.h (under _UNIX) and previously lived
 * in unixterm.c.  We implement it here.
 *
 * We call timerCheck(0): passing 0 tells timerCheck to call gettimeofday()
 * itself, mark any expired timers, and reschedule the itimer.  Then we
 * signal the condvar so blockUntilEvent() wakes up immediately.
 * ====================================================================== */
void
sigAlarm(SIGNAL_PROTOTYPE)
{
#if !((defined _POSIX_SIGNALS) || (defined _BSD_SIGNALS))
    signal(SIGALRM, sigAlarm);
#endif
    timerCheck(0);
    wakeSelect();
}

/* =========================================================================
 * Queue helpers
 * ====================================================================== */

/* Push a key code - safe to call from any thread, including signal handlers
 * (pthread_mutex_lock is async-signal-safe on macOS via the kernel). */
static void
queuePush(meUShort key)
{
    pthread_mutex_lock(&gInputMutex);
    if (gInputCount < INPUT_QUEUE_SIZE) {
        gInputQueue[gInputHead] = key;
        gInputHead = (gInputHead + 1) % INPUT_QUEUE_SIZE;
        gInputCount++;
    }
    pthread_mutex_unlock(&gInputMutex);
    wakeSelect();
}

/* Pop one key - ME engine thread only. */
static int
queuePop(meUShort *key)
{
    int ok = 0;
    pthread_mutex_lock(&gInputMutex);
    if (gInputCount > 0) {
        *key = gInputQueue[gInputTail];
        gInputTail = (gInputTail + 1) % INPUT_QUEUE_SIZE;
        gInputCount--;
        ok = 1;
    }
    pthread_mutex_unlock(&gInputMutex);
    return ok;
}

/* Move all queued keys into ME's internal key buffer.
 * Must only be called from the ME engine thread. */
static void
drainQueue(void)
{
    meUShort k;
    while (queuePop(&k))
        addKeyToBuffer(k);
}

/* Maximum time to block in select() before re-checking timer expiry.
 * Short enough that a SIGALRM that fires just before select() is called
 * is recovered within one cycle even if the signal write() was lost. */
#define ME_MAX_WAIT_MS 50

/* =========================================================================
 * Frame-store accessors - called from Swift inside meNativeViewFlush
 * while the ME engine thread is blocked on DispatchQueue.main.sync.
 * ====================================================================== */

const uint8_t *
meNativeGetRowText(int row)
{
    return ((row >= 0) && (row <= (int) frameCur->depth)) ? frameCur->store[row].text:NULL;
}

const uint16_t *
meNativeGetRowScheme(int row)
{
    return ((row >= 0) && (row <= (int) frameCur->depth)) ? frameCur->store[row].scheme:NULL;
}

int
meNativeGetFrameCols(void)
{
    return frameCur->width;
}

uint8_t
meNativeGetCursorColor(void *viewPtr)
{
    meFrame *ff;
    if((ff=frameCur)->termData != viewPtr)
    {
        ff = frameList;
        while((ff != NULL) && (ff->termData != viewPtr))
            ff = ff->next;
        if(ff == NULL)
            ff=frameCur;
    }
    return (cursorColor[meModeTest(ff->windowCur->buffer->mode, MDOVER) ? 1 : 0]);
}

const uint32_t *
meNativeGetStyleTable(void)
{
    return (const uint32_t *) styleTable;
}

void
meNativeReturnFontName(char *fontName, int ptSize)
{
    mecm.fontSize = ptSize;
    snprintf(mecm.fontName,meFONTNAME_MAX,"%s:size=%d",(char *) fontName,ptSize);
}

void
meNativeReturnFontSize(int cellW, int cellH, int ascent, int isMono)
{
    mecm.fwidth    = cellW;
    mecm.fdepth    = cellH;
    mecm.fhwidth   = cellW >> 1;
    mecm.fhdepth   = cellH >> 1;
    mecm.ascent    = ascent;
    mecm.underline = ascent + 1;
    if (mecm.underline >= cellH)
        mecm.underline = cellH - 1;
    mecm.fadepth   = (cellW + 1 < cellH) ? cellW + 1 : cellH;
    mecm.fontMono  = isMono;
}

void
meNativeReturnViewFrameSize(void *viewPtr, int cols, int rows)
{
    pthread_mutex_lock(&gInputMutex);
    gResizeView = viewPtr;
    gResizeCols = cols;
    gResizeRows = rows;
    gResizePending = 1;
    pthread_mutex_unlock(&gInputMutex);
    if(TTwidthDefault == 0)
    {
        TTwidthDefault = (meUShort) cols;
        TTdepthDefault = (meUShort) rows;
    }
    wakeSelect();
}

void
meNativePushKey(uint16_t keyCode)
{
    queuePush((meUShort)keyCode);
}

void
meNativePushMouseMove(int col, int row, int dX, int dY, uint16_t modifiers)
{
    meUShort cc;
    meUInt arg;
    mouse_X  = (meShort)col;
    mouse_Y  = (meShort)row;
    mouse_dX = (meShort)dX;
    mouse_dY = (meShort)dY;
    mouseKeyState = (meUShort) (modifiers & (ME_SHIFT | ME_CONTROL | ME_ALT));
    cc = (ME_SPECIAL | mouseKeyState | 
          (SKEY_mouse_move + mouseKeys[mouseState & (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT)]));
    if((TTallKeys & 0x1) || (!TTallKeys && (decode_key(cc,&arg) != -1)))
        addKeyToBufferOnce(cc);
}

void
meNativePushMouseButton(int button, int pick,
                        int col, int row, int dX, int dY,
                        uint16_t modifiers)
{
    mouse_X  = (meShort)col;
    mouse_Y  = (meShort)row;
    mouse_dX = (meShort)dX;
    mouse_dY = (meShort)dY;
    mouseKeyState = (meUShort)(modifiers & (ME_SHIFT | ME_CONTROL | ME_ALT));

    if (pick) {
        mouseButtonPick((meUInt)button);
        addKeyToBuffer((meUShort)(ME_SPECIAL
                      | (SKEY_mouse_pick_1 + mouseKeys[button] - 1)
                      | mouseKeyState));
    } else {
        mouseButtonDrop((meUInt)button);
        timerKill(MOUSE_TIMER_ID);
        addKeyToBuffer((meUShort)(ME_SPECIAL
                      | (SKEY_mouse_drop_1 + mouseKeys[button] - 1)
                      | mouseKeyState));
    }
}

void
meNativeScrollWheel(int up, int col, int row, uint16_t modifiers)
{
    mouse_X  = (meShort)col;
    mouse_Y  = (meShort)row;
    mouse_dX = 0;
    mouse_dY = 0;
    mouseKeyState = (meUShort)(modifiers & (ME_SHIFT | ME_CONTROL | ME_ALT));
    addKeyToBuffer((meUShort)(ME_SPECIAL
                  | (up ? SKEY_mouse_wup : SKEY_mouse_wdown)
                  | mouseKeyState));
}

void
meNativeQuit(void)
{
    gQuitPending = 1;
    wakeSelect();
}

void
meNativeViewFocusLost(void *viewPtr)
{
    if(gFocusedView == viewPtr)
        gFocusedView = NULL;
    gFocusChange = 1;
    wakeSelect();
}

void
meNativeViewFocusGained(void *viewPtr)
{
    gFocusedView = viewPtr;
    gFocusChange = 1;
    wakeSelect();
}

void
meNativeDeleteFrame(void *viewPtr)
{
    gDeleteFrameView = viewPtr;
    wakeSelect();
}

#ifdef _CLIPBRD
/* =========================================================================
 * Clipboard support (TTinitClipboard / TTsetClipboard / TTgetClipboard)
 *
 * NSPasteboard access must happen on the main thread; the Swift wrappers
 * in MEClipboard.swift dispatch there with sync, so these functions block
 * briefly but are safe to call from the ME engine thread.
 *
 * Ownership model (lazy, matching X11/Windows):
 *
 *   TTsetClipboard(0)    - claim ownership lazily; mark dirty but do NOT
 *                          write to NSPasteboard yet.  Called on every kill,
 *                          so must be fast.
 *   TTsetClipboard(n!=0) - explicit copy (e.g. copy-region): write to
 *                          NSPasteboard immediately and claim ownership.
 *   TTgetClipboard       - if CLIP_OWNER, return immediately (kill buffer is
 *                          authoritative).  Otherwise read from NSPasteboard.
 *   TTflushClipboard     - called on focus-lost: push dirty content so other
 *                          apps can paste while ME is in the background.
 *   TTcheckClipboard     - called on focus-gained: detect if another app
 *                          replaced the pasteboard; clear CLIP_OWNER if so.
 * ====================================================================== */
static int gClipChangeCount = -1; /* changeCount from our last NSPasteboard write */
static int gClipDirty       =  0; /* kill buffer changed but not yet written to pasteboard */

/* Build a UTF-8 buffer from the current kill buffer.
 * Returns a malloc'd buffer (caller must meFree) and sets *outLen.
 * Returns NULL on allocation failure or empty kill buffer. */
static meUByte *
clipBuildUtf8(int *outLen)
{
    meUByte    *utf8buf;
    int         utf8cap, utf8len;
    meKillNode *killp;
    meUByte     cc;

    utf8cap = 4096;
    utf8len = 0;
    if((utf8buf = (meUByte *) meMalloc(utf8cap)) == NULL)
        return NULL;

    if(klhead == NULL || klhead->kill == NULL) {
        if(meSystemCfg & meSYSTEM_NOEMPTYANK)
            utf8buf[utf8len++] = ' ';
    } else {
        for(killp = klhead->kill; killp != NULL; killp = killp->next) {
            meUByte *dd = killp->data;
            while((cc = *dd++) != '\0') {
                if(utf8len + 4 > utf8cap) {
                    meUByte *nb = (meUByte *) meRealloc(utf8buf, utf8cap * 2);
                    if(nb == NULL) { meFree(utf8buf); return NULL; }
                    utf8buf = nb;
                    utf8cap *= 2;
                }
                if(cc == meCHAR_NL) {
                    utf8buf[utf8len++] = '\n';
#if MEOPT_EXTENDED
                } else if(cc >= 0x80) {
                    meUShort uc = charToUnicode[cc - 128];
                    if(uc == 0) uc = cc;
                    if(uc < 0x0080) {
                        utf8buf[utf8len++] = (meUByte) uc;
                    } else if(uc < 0x0800) {
                        utf8buf[utf8len++] = (meUByte)(0xc0 | (uc >> 6));
                        utf8buf[utf8len++] = (meUByte)(0x80 | (uc & 0x3f));
                    } else {
                        utf8buf[utf8len++] = (meUByte)(0xe0 | (uc >> 12));
                        utf8buf[utf8len++] = (meUByte)(0x80 | ((uc >> 6) & 0x3f));
                        utf8buf[utf8len++] = (meUByte)(0x80 | (uc & 0x3f));
                    }
                } else if(cc < 0x20) {
                    meUShort uc = (meUShort) ttSpeChars[cc];
                    if(uc < 0x80) {
                        utf8buf[utf8len++] = (meUByte) uc;
                    } else if(uc < 0x0800) {
                        utf8buf[utf8len++] = (meUByte)(0xc0 | (uc >> 6));
                        utf8buf[utf8len++] = (meUByte)(0x80 | (uc & 0x3f));
                    } else {
                        utf8buf[utf8len++] = (meUByte)(0xe0 | (uc >> 12));
                        utf8buf[utf8len++] = (meUByte)(0x80 | ((uc >> 6) & 0x3f));
                        utf8buf[utf8len++] = (meUByte)(0x80 | (uc & 0x3f));
                    }
#endif
                } else {
                    utf8buf[utf8len++] = cc;
                }
            }
        }
    }
    utf8buf[utf8len] = '\0';
    *outLen = utf8len;
    return utf8buf;
}

void
TTinitClipboard(void)
{
    gClipChangeCount = -1;
    gClipDirty       =  0;
}

void
TTsetClipboard(int cpData)
{
    if((meSystemCfg & meSYSTEM_NOCLIPBRD)          ||
       (clipState  & (CLIP_RECEIVING|CLIP_DISABLED))||
       (!cpData && (kbdmode == mePLAY))             ||
       (frameCur  && (frameCur->flags & meFRAME_NOT_FOCUS)))
        return;

    /* Already own the clipboard: just note it's dirty for flush on focus-loss */
    if((clipState & CLIP_OWNER) && !cpData) {
        gClipDirty = 1;
        return;
    }

    if(cpData) {
        /* Explicit copy (e.g. copy-region): write to NSPasteboard now */
        meUByte *utf8buf;
        int      utf8len;
        if((utf8buf = clipBuildUtf8(&utf8len)) == NULL)
            return;
        gClipChangeCount = meNativeSetClipboard((const char *)utf8buf, utf8len);
        meFree(utf8buf);
        gClipDirty = 0;
    } else {
        /* Lazy claim: record current pasteboard state so we can detect
         * foreign writes while ME is focused, but don't write yet */
        gClipChangeCount = meNativeGetPasteboardChangeCount();
        gClipDirty = 1;
    }
    clipState |= CLIP_OWNER;
    clipState &= ~CLIP_STALE;
}

/* Called on focus-lost: push kill buffer to NSPasteboard so other apps
 * can paste while ME is in the background. */
void
TTflushClipboard(void)
{
    meUByte *utf8buf;
    int      utf8len;

    if(!(clipState & CLIP_OWNER) || !gClipDirty ||
       (meSystemCfg & meSYSTEM_NOCLIPBRD) ||
       (clipState & CLIP_DISABLED))
        return;

    if((utf8buf = clipBuildUtf8(&utf8len)) == NULL)
        return;
    gClipChangeCount = meNativeSetClipboard((const char *)utf8buf, utf8len);
    meFree(utf8buf);
    gClipDirty = 0;
}

/* Called on focus-gained: detect if another app replaced the pasteboard
 * while ME was in the background. */
void
TTcheckClipboard(void)
{
    if((clipState & CLIP_OWNER) && !(meSystemCfg & meSYSTEM_NOCLIPBRD) &&
       !(clipState & CLIP_DISABLED) && (meNativeGetPasteboardChangeCount() != gClipChangeCount))
        clipState &= ~CLIP_OWNER;
}

void
TTgetClipboard(int flag)
{
    char       *utf8text;
    meKillNode *kn = NULL;

    if((clipState & (CLIP_OWNER|CLIP_DISABLED)) || (meSystemCfg & meSYSTEM_NOCLIPBRD) || 
       (((flag & 1) == 0) && ((clexec != meFALSE) || (kbdmode == mePLAY) || (frameCur->flags & meFRAME_NOT_FOCUS))))
        return;

    if((utf8text = meNativeGetClipboard()) == NULL)
        return;

    /* ---- Convert UTF-8 clipboard text - ME kill buffer ---- */
    {
        const meUByte *ss;
        meUByte       *dd;
        int            len, ll;
        meUShort       uc;

        /* First pass: compute the number of ME bytes needed */
        ss = (const meUByte *)utf8text;
        len = ll = 0;
        while(*ss) {
            meUByte c0 = *ss;
            if(c0 < 0x80) {
                ss++;
                if(c0 == '\r') {
                    if(*ss == '\n') ss++;
                    len += ll + (ll >> 15) + 1; ll = 0;
                } else if(c0 == '\n') {
                    len += ll + (ll >> 15) + 1; ll = 0;
                } else {
                    ll++;
                }
            } else if(c0 < 0xe0) {
                ss += 2; ll++;
            } else {
                ss += 3; ll++;
            }
        }
        len += ll + (ll >> 15);

        if((len > 0) &&
           ((kn = (meKillNode *) meMalloc(
               ((size_t)(&(((meKillNode *)0)->data[1])))+len)) != NULL))
        {
            ss = (const meUByte *)utf8text;
            dd = kn->data;
            ll = 0;

            while(*ss) {
                meUByte c0 = *ss;

                if(c0 < 0x80) {
                    uc = c0; ss++;
                } else if(c0 < 0xe0) {
                    uc = (meUShort)(((c0 & 0x1f) << 6) | (ss[1] & 0x3f));
                    ss += 2;
                } else {
                    uc = (meUShort)(((c0 & 0x0f) << 12) |
                                    ((ss[1] & 0x3f) << 6) |
                                    (ss[2] & 0x3f));
                    ss += 3;
                }

                if(uc == '\r') {
                    if(*ss == '\n') ss++;
                    *dd++ = '\n'; ll = 0;
                } else if(uc == '\n') {
                    *dd++ = '\n'; ll = 0;
                } else {
                    if(ll == meLINE_ELEN_MAX) { *dd++ = '\n'; ll = 0; }
#if MEOPT_EXTENDED
                    if(uc >= 0x80) {
                        int ii = 127;
                        while((charToUnicode[ii] != uc) && (--ii >= 0))
                            ;
                        *dd++ = (ii >= 0) ? (meUByte)(ii + 128) : meCHAR_UNDEF;
                    } else
#endif
                        *dd++ = (meUByte) uc;
                    ll++;
                }
            }
            *dd = '\0';

            /* Discard if identical to the current kill-buffer head */
            if((klhead != NULL) && (klhead->kill != NULL) &&
               (klhead->kill->next == NULL) &&
               !meStrcmp(klhead->kill->data, kn->data)) {
                meFree(kn);
                kn = NULL;
            }
        }
    }

    free(utf8text);     /* allocated by strdup() inside meNativeGetClipboard */

    if(kn != NULL) {
        killInit(0);
        killInsertNode(kn);
    }
    TTsetClipboard(0);  /* claim pasteboard ownership */
}

#endif /* _CLIPBRD */

/* Called from the Swift background thread; blocks until ME exits. */
void
meStartEngine(int argc, char **argv)
{
    meMain(argc, argv);
}

/* =========================================================================
 * Mouse initialisation  (called from main.c and eval.c via TTinitMouse)
 * ====================================================================== */

#if MEOPT_MOUSE
void
TTinitMouse(meInt nCfg)
{
    meUShort b1 = 1, b2 = 2, b3 = 3;

    if (nCfg & meMOUSE_SWAPBUTTONS) {
        b1 = 3;
        b3 = 1;
    }
    /* meMOUSE_NOBUTTONS field encodes the number of buttons */
    if ((nCfg & meMOUSE_NOBUTTONS) < 3)
        b2 = b3;

    mouseKeys[1] = b1;
    mouseKeys[2] = b2;
    mouseKeys[3] = b3;
    mouseKeys[4] = 4;
    mouseKeys[5] = 5;
    meMouseCfg   = nCfg;
}
#endif /* MEOPT_MOUSE */

/* =========================================================================
 * ME terminal interface: waitForEvent / TTahead / TTwaitForChar / TTsleep
 * ====================================================================== */

/*
 * waitForEvent - block until something interesting happens.
 *
 * mode 0 - return on key, timer expiry, resize, quit, or ipipe data
 * mode 1 - same as 0, explicit ipipe-event return
 * mode 2 - return only on non-key events (timer, resize, quit, ipipe data)
 *
 * Uses select() so that ipipe file descriptors are watched alongside the
 * self-pipe wake channel.  SIGALRM wakes select() via wakeSelect().
 */
void
waitForEvent(int mode)
{
    for(;;)
    {
        /* ---- Check conditions for immediate return ---- */
        if (isTimerExpired(AUTOS_TIMER_ID)  ||
#if MEOPT_CALLBACK
            isTimerExpired(CALLB_TIMER_ID)  ||
#endif
            isTimerExpired(CURSOR_TIMER_ID) ||
            isTimerExpired(SLEEP_TIMER_ID)  ||
#if MEOPT_MOUSE
            isTimerExpired(MOUSE_TIMER_ID)  ||
#endif
#if MEOPT_SOCKET
            isTimerExpired(SOCKET_TIMER_ID) ||
#endif
            (sgarbf == meTRUE) || gResizePending || gQuitPending || gFocusChange || gDeleteFrameView != NULL)
            return;

        if ((mode & 2) == 0 && TTahead())
            return;

        TTdieTest();

#if MEOPT_IPIPES
        /* ---- Drain already-closed ipipes (pid<0 means process exited) ---- */
        {
            meIPipe *ip = ipipes;
            int ipipeEvent = 0;
            while(ip)
            {
                meIPipe *next = ip->next;
                if(ip->pid < 0)
                {
                    ipipeRead(ip);
                    ipipeEvent = 1;
                }
                ip = next;
            }
            drainQueue();
            if(ipipeEvent)
            {
                if(mode || (TTnoKeys > 0))
                    return;
                continue;
            }
        }
#endif

        /* ---- Build fd_set: self-pipe + active ipipe read fds ---- */
        {
            fd_set rset;
            struct timeval tv;
            int maxfd = gWakePipe[0];
            int rc;

            FD_ZERO(&rset);
            FD_SET(gWakePipe[0], &rset);

#if MEOPT_IPIPES
            {
                meIPipe *ip = ipipes;
                while(ip)
                {
                    if(ip->pid >= 0)
                    {
                        FD_SET(ip->rfd, &rset);
                        if(ip->rfd > maxfd)
                            maxfd = ip->rfd;
                    }
                    ip = ip->next;
                }
            }
#endif

            tv.tv_sec  = 0;
            tv.tv_usec = (long)ME_MAX_WAIT_MS * 1000L;
            rc = select(maxfd + 1, &rset, NULL, NULL, &tv);

            drainWakePipe();
            drainQueue();

#if MEOPT_IPIPES
            if (rc > 0)
            {
                meIPipe *ip = ipipes;
                int ipipeEvent = 0;
                while(ip)
                {
                    meIPipe *next = ip->next;
                    if(ip->pid < 0 || FD_ISSET(ip->rfd, &rset))
                    {
                        ipipeRead(ip);
                        ipipeEvent = 1;
                    }
                    ip = next;
                }
                if(ipipeEvent && mode)
                    return;
            }
#endif
        }

        if((mode & 2) == 0 && TTnoKeys > 0)
            return;
    }
}

/* TTahead - non-zero if there are keys waiting in the ME key buffer */
int
TTahead(void)
{
    if (TTnoKeys > 0)
        return TTnoKeys;
    drainQueue();
#if MEOPT_MOUSE
    if (isTimerExpired(MOUSE_TIMER_ID)) {
        meUShort mc;
        meUInt   arg;
        timerClearExpired(MOUSE_TIMER_ID);
        mc = (meUShort)(ME_SPECIAL | mouseKeyState |
                        (SKEY_mouse_time + mouseKeys[mouseState & (MOUSE_STATE_LEFT|MOUSE_STATE_MIDDLE|MOUSE_STATE_RIGHT)]));
        if ((!TTallKeys && decode_key(mc, &arg) != -1) || (TTallKeys & 0x2)) {
            addKeyToBufferOnce(mc);
            timerSet(MOUSE_TIMER_ID, -1, repeatTime);
        }
    }
#endif
    return TTnoKeys;
}

/* TTwaitForChar - block until at least one key is in the ME key buffer */
void
TTwaitForChar(void)
{
#if MEOPT_MOUSE
    /* If a mouse button is held and a mouse-time binding exists, start timer */
    if (!isTimerSet(MOUSE_TIMER_ID) && mouseState != 0)
    {
        meUShort mc;
        meUInt arg;
        mc = (meUShort)(ME_SPECIAL | mouseKeyState |
                        (SKEY_mouse_time + mouseKeys[mouseButtonGetPick()]));
        if ((!TTallKeys && decode_key(mc, &arg) != -1) || (TTallKeys & 0x2))
            timerSet(MOUSE_TIMER_ID, -1, delayTime);
    }
#endif
#if MEOPT_CALLBACK
    if(kbdmode == meIDLE)
        doIdlePickEvent();
#endif

    for(;;)
    {
        handleTimerExpired();
        drainQueue();

        if(TTahead())
            break;

        if(gQuitPending)
        {
            int savcle;
            gQuitPending = 0;
            savcle = clexec;
            clexec = meFALSE;
            if(!exitEmacs(1, 3))
                sgarbf = meTRUE;
            clexec = savcle;
        }
        /* Handle a pending window resize */
        if(gResizePending)
        {
            int cols, rows;
            void *resizeView;
            meFrame *resizeFrame;
            pthread_mutex_lock(&gInputMutex);
            cols = gResizeCols;
            rows = gResizeRows;
            resizeView = (void *) gResizeView;
            gResizePending = 0;
            pthread_mutex_unlock(&gInputMutex);

            /* Find the frame whose window was resized. */
            resizeFrame = frameList;
            while(resizeFrame != NULL)
            {
                if(resizeFrame->termData == resizeView)
                {
                    if(cols != resizeFrame->width)
                        meFrameChangeWidth(resizeFrame,cols);
                    if(rows != resizeFrame->depth + 1)
                        meFrameChangeDepth(resizeFrame,rows);
                    meNativeViewSetWindowSize(resizeFrame->termData,resizeFrame->width,resizeFrame->depth+1);
                    if(resizeFrame == frameCur && !screenUpdateDisabledCount)
                        screenUpdate(meTRUE,2 - sgarbf);
                    break;
                }
                resizeFrame = resizeFrame->next;
            }
        }

        /* Handle focus change - switch frame if needed, then handle NOT_FOCUS */
        if(gFocusChange)
        {
            void *focusedView = (void *)gFocusedView;
            meFrame *ff = frameList;
            gFocusChange = 0;
            
            while(ff != NULL)
            {
                if(ff->termData == focusedView)
                {
                    if(ff->flags & meFRAME_NOT_FOCUS)
                    {
                        /* Record the fact we have focus */
                        ff->flags &= ~meFRAME_NOT_FOCUS;
#if MEOPT_MWFRAME
                        if(frameCur != ff)
                        {
                            meUByte scheme=(meUByte)(globScheme / meSCHEME_STYLES);
                            meFrame *fc=frameCur;
                            frameFocus = ff;
                            frameCur = ff;
                            // TODO this leads to a flash of [NOT FOCUS] when swapping windows even when ml not in use
                            pokeScreen(0x11,ff->depth,(ff->width >> 1)-5,&scheme,(meUByte *)"[NOT FOCUS]");
                            frameCur = fc;
                        }
#endif
#ifdef _CLIPBRD
                        TTcheckClipboard();
#endif
                        if(cursorState >= 0)
                        {
#if MEOPT_MWFRAME
                            if(frameCur != ff)
                            {
                                /* another frame has input, we need to hide this frame's cursor and ensure
                                 * frameCur's cursor is shown so the input location is visible */
                                meFrameHideCursor(ff);
                                meFrameShowCursor(frameCur);
                                blinkState = 1;
                            }
                            else
#endif
                                if(cursorBlink)
                                TThandleBlink(2);
                            else
                                meFrameShowCursor(ff);
                        }
                    }
                }
                else if((ff->flags & meFRAME_NOT_FOCUS) == 0)
                {
                    ff->flags |= meFRAME_NOT_FOCUS;
#if MEOPT_MWFRAME
                    if(frameFocus == ff)
                    {
                        meUByte scheme=(mlScheme/meSCHEME_STYLES);
                        meFrame *fc=frameCur;
                        frameFocus = NULL;
                        frameCur = ff;
                        pokeScreen(0x01,ff->depth,(ff->width >> 1)-5,&scheme,(meUByte *) "           ");
                        frameCur = fc;
                    }
#endif
#ifdef _CLIPBRD
                    TTflushClipboard();
#endif
                    if(cursorState >= 0)
                    {
                        /* because the cursor is a part of the solid cursor we must
                         * remove the old one first and then redraw */
                        if(blinkState)
                            meFrameHideCursor(ff);
                        meFrameShowCursor(ff);
                    }
                }
                ff = ff->next;
            }
        }
        /* Handle close-button delete-frame request */
        if(gDeleteFrameView != NULL)
        {
            void *deleteView = (void *)gDeleteFrameView;
            meFrame *ff;
            gDeleteFrameView = NULL;
            ff = frameList;
            while(ff != NULL)
            {
                if(ff->termData == deleteView)
                {
                    meFrameDelete(ff, 3);
                    break;
                }
                ff = ff->next;
            }
        }
        if(sgarbf == meTRUE)
        {
            update(meTRUE);
            mlerase(MWCLEXEC);
        }
        waitForEvent(0);
    }
}

/* TTsleep - sleep for msec ms, optionally interruptible by keys */
void
TTsleep(int msec, int intable, meVariable **waitVarList)
{
    meUByte *ss;
    int sgarbfOld;

    if (intable && ((kbdmode == mePLAY) || (clexec == meTRUE)))
        return;

    if (msec >= 0)
        timerSet(SLEEP_TIMER_ID, -1, msec);
    else if (waitVarList != NULL)
        timerKill(SLEEP_TIMER_ID);
    else
        return;

    sgarbfOld = sgarbf;
    do {
        handleTimerExpired();
        if (intable && TTahead())
            break;
        if (waitVarList != NULL) {
            ss = getUsrLclCmdVar((meUByte *)"wait", *waitVarList);
            if (ss == errorm || !meAtoi(ss))
                break;
        }
        sgarbfOld |= sgarbf;
        sgarbf = meFALSE;
        waitForEvent(intable ? 1 : 2);
    } while (!isTimerExpired(SLEEP_TIMER_ID));

    timerKill(SLEEP_TIMER_ID);
    sgarbf |= sgarbfOld;
}

/* =========================================================================
 * Colour management  - TTaddColor / TTsetBgcol
 * ====================================================================== */

void
TTsetBgcol(void)
{
    meNativeSetBgColor((meUByte) meStyleGetBColor(meSchemeGetStyle(globScheme)));
}

int
TTaddColor(meColor index, meUByte r, meUByte g, meUByte b)
{
    int ii;
    if ((int)index >= ME_MAX_COLORS)
        return meFALSE;
    ii = ((int) index) * 3;
    meColorTable[ii++] = r;
    meColorTable[ii++] = g;
    meColorTable[ii] = b;
    if (noColors <= (int)index)
        noColors = (int)index + 1;
    meNativeColorTableChanged();
    return meTRUE;
}

void
TTcharsetChanged(void)
{
    meNativeCharsetChanged();
}

void
meFrameRepositionWindow(meFrame *frame, int resize)
{
}

/* =========================================================================
 * Frame lifecycle
 * ====================================================================== */

int
meFrameTermInit(meFrame *frame, meFrame *sibling)
{
    if(sibling != NULL)
        /* Internal split: share the same window as the sibling. */
        frame->termData = sibling->termData;
    else if(frameList == NULL)
        /* First frame: the primary NSWindow already exists; just wrap its view. */
        frame->termData = meNativeGetPrimaryView();
    else
        /* Additional external frame: open a new NSWindow. */
        frame->termData = meNativeCreateWindow(frame->width, frame->depth + 1);
    return meTRUE;
}

void
meFrameTermFree(meFrame *frame, meFrame *sibling)
{
    /* Destroy the window only when this is the last frame using it.
     * Sibling frames share termData (same window) and must not close it. */
    if(sibling == NULL)
        meNativeDestroyWindow(frame->termData);
}

void
meFrameTermMakeCur(meFrame *frame)
{
    meNativeSetActiveView(frame->termData);
}

/* =========================================================================
 * Cursor
 * ====================================================================== */

void
meFrameShowCursor(meFrame *frame)
{
    meNativeViewSetCursor(frame->termData,frame->cursorColumn,frame->cursorRow);
}

void
meFrameHideCursor(meFrame *frame)
{
    meNativeViewHideCursor(frame->termData);
}

/* =========================================================================
 * Miscellaneous terminal functions
 * ====================================================================== */

void
TTflush(void)
{
    if (gDirtyRowMax < 0)
        return;
    meNativeViewFlush(frameCur->termData,gDirtyRowMin,gDirtyRowMax,
                      gDirtyColStart,gDirtyColEnd);
    clearDirty();
}

void
TTNbell(void)
{
    meNativeBell();
}

/* meFrameSetWindowSize - called when $frame-width or $frame-depth is set
 * programmatically.  Notify Swift so it resizes both the render buffer and
 * the NSWindow to match the new frame dimensions. */
void
meFrameSetWindowSize(meFrame *frame)
{
    meNativeViewSetWindowSize(frame->termData,frame->width,frame->depth + 1);
}

/* Dirty-region rectangle - accumulated by TTaddArea, consumed by TTapplyArea */
meNWRect ttRect;

void
TTapplyArea(void)
{
    int r;
    if (ttRect.bottom <= ttRect.top || !frameCur)
        return;
    for (r = ttRect.top; r < ttRect.bottom; r++)
        meNativeMarkDirty(r,0,frameCur->width);
}

void
TTputs(int row, int col, int len)
{
    meNativeMarkDirty(row, col, col + len);
}

void
meFrameSetWindowTitle(meFrame *frame, meUByte *str)
{
    char buf[meBUF_SIZE_MAX];
    
#if MEOPT_EXTENDED
    if(frameTitle != NULL)
        meStrcpy(buf,frameTitle);
    else
#endif
        meStrcpy(buf,ME_FULLNAME);
    if(str != NULL)
    {
        meStrcat(buf,": ");
        meStrcat(buf,str);
    }
    meNativeViewSetTitle(frame->termData,(const char *) buf);
}

/* =========================================================================
 * changeFont - implements the ME "change-font" command.
 *
 * Numeric argument n bit flags:
 *   0x01  Prompt for font name and size (ME minibuffer)
 *   0x02  Don't apply (query-only - not used here)
 *   0x04  Don't set $result
 *   0x08  Dialog: show the system font-picker panel
 *   0x10  (Windows: list all fonts - ignored on macOS)
 *
 * With no argument (f==0) the dialog panel is shown.
 * ====================================================================== */
int
changeFont(int f, int n)
{
    meUByte *pp, *ss, fontBuf[meFONTNAME_MAX];
    meCellMetrics bmecm;
    int sz=0;

    if(!f)
        n = 0x09;   /* default: open the font-picker dialog */

    if(n & 0x01)
    {
        if(n & 0x02)
            /* not applying the font so back up the current font details */
            memcpy(&bmecm,&mecm,sizeof(meCellMetrics));
        if(n & 0x08)
        {
            /* Dialog mode: pass empty name to Swift which shows NSFontPanel.
             * Blocks here until the user closes the panel; font changes are
             * applied live as the user browses. */
            meNativeViewChangeFont(frameCur->termData,n,"",0.0f);
        }
        else
        {
            /* Prompt mode: ask the user for a PostScript font name and size. */
            if(meGetString((meUByte *)"Font Name",0,0,fontBuf,meFONTNAME_MAX) == meABORT)
                return meFALSE;
            if((pp = meStrchr(fontBuf,':')) != NULL)
            {
                if((ss = meStrstr(pp,":size=")) != NULL)
                    sz = meAtoi(ss+6);
                *pp = '\0';
            }
            if(sz <= 0)
            {
                meStrcpy(mecm.fontName,fontBuf);
                sz = (int) ((CGDisplayBounds(CGMainDisplayID()).size.height / 95.0) + 0.5);
            }
            else
                snprintf(mecm.fontName,meFONTNAME_MAX,"%s:size=%d",(char *) fontBuf,sz);
            mecm.fontSize = sz;
            meNativeViewChangeFont(frameCur->termData,n,(const char *)fontBuf,sz);
        }
    }
    if((n & 0x04) == 0)
    {
        /* Return current font info in $result.
         * Format mirrors XTerm TTchangeFont:
         *   |pitch|||sizeX|sizeY||reqY|name|
         * pitch:   '0'=fixed  '1'=proportional
         * sizeX:   mecm.fwidth   (actual cell width in pixels)
         * sizeY:   mecm.fdepth   (actual cell height in pixels)
         * name:    mecm.fontName (Xft-style, e.g. "Menlo:size=13") */
        sprintf((char *) resultStr, "|%c|||%d|%d||%d|%s|",(mecm.fontMono ? '0':'1'),
                mecm.fwidth,mecm.fdepth,mecm.fontSize,mecm.fontName);
    }
    /* if not applying prompted font, restore font details */
    if((n & 0x03) == 3)
        memcpy(&mecm,&bmecm,sizeof(meCellMetrics));
    return meTRUE;
}

int TTopen(void)  { return meTRUE; }
int TTclose(void) { return meTRUE; }

/* Get the terminal attributes structure. There are cases when the call will
 * fail we have been launched off the desktop via an open action. Return an
 * integer status of -1 if the call failed and we filled in the values. */
static int
TCAPgetattr(meTermio p, int isX)
{
    int status;                         /* Status for attribute retrieval. */
    
    if((status = tcgetattr(0, p)) < 0)
    {
        /* Input flag defaults */
        p->c_iflag = BRKINT|ICRNL|IXANY;
        /* Output flag defaults */
        p->c_oflag = OPOST|ONLCR;
        /* Control modes */
        p->c_cflag = CS8|CREAD|HUPCL;
        /* Local modes */
        p->c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE;
        
        /* Terminal special characters */
        p->c_cc [VINTR] = 'C' - '@';    /* C-c : CINTR */
        p->c_cc [VQUIT] = CQUIT;        /* FS, cntl | */
        p->c_cc [VERASE] = 'H' - '@';   /* Backspace or '#' 0x7f */
        p->c_cc [VKILL] = 'K' - '@';    /* C-k */
        p->c_cc [VEOF] = CEOF;          /* C-d */
        p->c_cc [VEOL] = 'J' - '@';     /* C-j */
        p->c_cc [VMIN] = 1;
        p->c_cc [VTIME] = 0;
#ifdef VWERASE
        p->c_cc [VWERASE] = 'W' - '@';  /* C-w */
#endif
        p->c_cc [VLNEXT] = CLNEXT;      /* C-v */
#ifdef VDSUSP
        p->c_cc [VDSUSP] = CDSUSP;      /* C-y */
#endif
        p->c_cc [VSUSP] = CSUSP;        /* C-z */
        p->c_cc [VSTART] = CSTART;      /* C-q */
        p->c_cc [VSTOP]  = CSTOP;       /* C-s */
    }
    else if (isX != 0)
    {
        /* We are running X-Windows. We have been launched from the command
         * line as we have obtained the terminal environment. Set this up for
         * an interactive shell session. */
        /* Correct the erase if it is defaulted */
        if (p->c_cc [VERASE] == 0x7f)
            p->c_cc [VERASE] = 'H' - '@';  /* Backspace */
    }
    /* Return the status of the system call. */
    return status;
}

/* =========================================================================
 * Client/server - Unix domain socket IPC (identical to unixterm.c)
 * ====================================================================== */
#if MEOPT_CLIENTSERVER

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

int meCSSock=-1;
int meCCSSock=-1;
void
TTopenClientServer(void)
{
    if(meCSSock == -1)
    {
        struct sockaddr_un cssa;
        meIPipe *ipipe;
        meBuffer *bp;
        meMode sglobMode;
        int ii;

        if((meCSSock=socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            return;
        }
        ii = sprintf(cssa.sun_path,"/tmp/mesrv%d",(int) meUid);
        ii += sizeof(cssa.sun_family);
        cssa.sun_family = AF_UNIX;

        if(!meTestExist(cssa.sun_path) &&
           (connect(meCSSock,(struct sockaddr *)&cssa,ii) >= 0))
        {
            /* theres another me acting as a server, quit! */
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            close(meCSSock);
            meCSSock = -1;
            return;
        }
        meUnlinkNT(cssa.sun_path);
        if((bind(meCSSock,(struct sockaddr *)&cssa,ii) < 0) ||
           (listen(meCSSock,20) < 0))
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            close(meCSSock);
            meCSSock = -1;
            return;
        }
        /* Change socket permissions so only the user can send commands */
        meFileSetAttributes((char *) cssa.sun_path,0700);
        /* Create the ipipe buffer */
        meModeCopy(sglobMode,globMode);
        meModeSet(globMode,MDPIPE);
        meModeSet(globMode,MDLOCK);
        meModeSet(globMode,MDHIDE);
        meModeClear(globMode,MDWRAP);
        meModeClear(globMode,MDUNDO);
        if(((bp=bfind(BserverN,BFND_CREAT)) == NULL) ||
           ((ipipe = meMalloc(sizeof(meIPipe))) == NULL))
        {
            meSystemCfg &= ~meSYSTEM_CLNTSRVR;
            close(meCSSock);
            meCSSock = -1;
            return;
        }
        meModeCopy(globMode,sglobMode);
        bp->intFlag |= BIFNODEL;
        ipipe->next = ipipes;
        ipipe->pid = 0;
        ipipe->rfd = meCSSock;
        ipipe->outWfd = meCSSock;
        ipipes = ipipe;
        noIpipes++;
        ipipe->bp = bp;
        {
            meUByte buff[meBUF_SIZE_MAX];
            sprintf((char *)buff,"Client Server: /tmp/mesrv%d\n\n",(int) meUid);
            addLineToEob(bp,buff);
            bp->dotLine = meLineGetPrev(bp->baseLine);
            bp->dotOffset = 0;
            bp->dotLineNo = bp->lineCount-1;
            meAnchorSet(bp,'I',bp->dotLine,bp->dotLineNo,bp->dotOffset,1);
        }
        ipipe->flag = 0;
        ipipe->strRow = 0;
        ipipe->strCol = 0;
        ipipe->noRows = 0;
        ipipe->noCols = 0;
        ipipe->curRow = 0;
        ipipe->curRow = bp->dotLineNo;
        ipipeSetSize(frameCur->windowCur,bp);
    }
}

void
TTkillClientServer(void)
{
    if(meCSSock != -1)
    {
        meIPipe *ipipe;
        char fname[meBUF_SIZE_MAX];

        ipipe = ipipes;
        while(ipipe != NULL)
        {
            if(ipipe->pid == 0)
                break;
            ipipe = ipipe->next;
        }
        if(ipipe != NULL)
        {
            ipipe->bp->intFlag |= BIFBLOW;
            zotbuf(ipipe->bp,1);
        }
        else
            close(meCSSock);
        sprintf(fname,"/tmp/mesrv%d",(int) meUid);
        meUnlinkNT(fname);
        meSystemCfg &= ~meSYSTEM_CLNTSRVR;
        meCSSock = -1;
    }
    if(meCCSSock != -1)
    {
        close(meCSSock);
        meCCSSock = -1;
    }
}

int
TTconnectClientServer(void)
{
    if(meCCSSock == -1)
    {
        struct sockaddr_un cssa;
        int ii;

        if((meCCSSock=socket(AF_UNIX,SOCK_STREAM, 0)) < 0)
            return 0;
        ii = sizeof(cssa.sun_family);
        ii += sprintf(cssa.sun_path,"/tmp/mesrv%d",(int) meUid);
        cssa.sun_family = AF_UNIX;
        if(connect(meCCSSock,(struct sockaddr *)&cssa,ii) < 0)
        {
            close(meCCSSock);
            meCCSSock = -1;
            return 0;
        }
    }
    return 1;
}

void
TTsendClientServer(meUByte *line)
{
    if(meCCSSock >= 0)
    {
        int ii, ll=meStrlen(line);
        while(ll)
        {
            if((ii=write(meCCSSock,line,ll)) < 0)
                return;
            ll -= ii;
            line += ii;
        }
    }
}
#endif /* MEOPT_CLIENTSERVER */

/* =========================================================================
 * TTstart - called once via TTstart() macro during mesetup() in main.c
 * ====================================================================== */

int
TTstart(void)
{
    /* Copy the Terminal I/O. We may spawn a terminal in the window later and
     * the termio structure must be initialised. The structure may be
     * uninitialised if we have been launched off the desktop via an open
     * action. */
    TCAPgetattr(&otermio, 1);

    memset(meColorTable,0,sizeof(meColorTable));
    /* Clear CONSOLE flag so ME uses _ME_WINDOW code paths throughout */
    meSystemCfg &= ~meSYSTEM_CONSOLE;

    /* Initialise dirty-region tracking */
    {
        int r;
        for (r = 0; r <= meFRAME_DEPTH_MAX; r++) {
            gDirtyColStart[r] = ME_DIRTY_NONE;
            gDirtyColEnd[r]   = ME_DIRTY_CLEAR;
        }
        gDirtyRowMin = ME_DIRTY_NONE;
        gDirtyRowMax = ME_DIRTY_CLEAR;
    }

    /* Create the self-pipe used to wake select() from Swift threads / sigAlarm */
    if (pipe(gWakePipe) == 0) {
        /* Make the write end non-blocking so it is safe from signal handlers */
        fcntl(gWakePipe[1], F_SETFL, O_NONBLOCK);
        /* Make the read end non-blocking so drainWakePipe() never stalls */
        fcntl(gWakePipe[0], F_SETFL, O_NONBLOCK);
    }

    return meTRUE;
}

/* =========================================================================
 * meSetupProgname
 *
 * Resolve the absolute path of the ME executable and store it in
 * meProgName.  Also establishes curdir (current working directory).
 * ====================================================================== */
void
meSetupProgname(char *progname)
{
    struct stat dotstat, pwdstat;
    meUByte cc, *ss, *dd;
    int ii, ll;

    /* Use $PWD if it accurately reflects the current directory */
    if (((ss = meGetenv("PWD")) != NULL) &&
        (ss[0] == DIR_CHAR) && (stat((char *)ss, &pwdstat) == 0) &&
        (stat(".", &dotstat) == 0) && (dotstat.st_ino == pwdstat.st_ino) &&
        (dotstat.st_dev == pwdstat.st_dev)) {
        ll = meStrlen(ss);
        curdir = dd = meMalloc(ll + 2);
        while ((cc = *ss++) != '\0')
            *dd++ = cc;
        if (dd[-1] != DIR_CHAR)
            *dd++ = DIR_CHAR;
        *dd = 0;
    } else {
        curdir = gwd(0);
    }

    if (curdir == NULL)
        mlwrite(MWCURSOR | MWABORT | MWWAIT,
                (meUByte *)"Failed to get cwd\n");

    ii = meBUF_SIZE_MAX;
    if (_NSGetExecutablePath((char *)resultStr, (uint32_t *)&ii) == 0)
        fileNameCorrect(resultStr, evalResult, NULL);
    else
        ii = 0;

    if (ii == 0)
        ii = executableLookup((meUByte *)progname, evalResult);

    if ((ii == 0) && (meStrrchr((meUByte *)progname, DIR_CHAR) == NULL))
        ii = fileLookup((meUByte *)progname, 0, NULL,
                        meFL_CHECKPATH | meFL_EXEC, evalResult);

    /* Resolve any symlinks in the found path */
    if ((ii > 0) &&
        ((ll = readlink((char *)evalResult, (char *)resultStr, meBUF_SIZE_MAX)) > 0)) {
        memcpy(evalResult, resultStr, ll);
        evalResult[ll] = '\0';
    }

    if (ii != 0)
        meProgName = meStrdup(evalResult);
    else {
#ifdef _ME_FREE_ALL_MEMORY
        meProgName = meStrdup((meUByte *)progname);
#else
        meProgName = (meUByte *)progname;
#endif
    }
}

/* =========================================================================
 * meSetupPathsAndUser
 *
 * Establish ME's search path, user name, user path and home directory by
 * examining environment variables and the password database.
 * ====================================================================== */
void
meSetupPathsAndUser(void)
{
#ifndef _SEARCH_PATH
#define _SEARCH_PATH _DEFAULT_SEARCH_PATH
#endif
    static meUByte lpath[] = _SEARCH_PATH;
    struct passwd *pwdp;
    meUByte *ss, buff[meBUF_SIZE_MAX];
    int ii, ll, gotPaths;

    if ((meUserName == NULL) &&
        ((ss = meGetenv("MENAME")) != NULL) && (ss[0] != '\0'))
        meUserName = meStrdup(ss);

    /* Home directory: prefer $HOME env var */
    if (((ss = meGetenv("HOME")) != NULL) && (ss[0] != '\0'))
        fileNameSetHome(ss);

    /* Fill in from password database if we still need homedir or username */
    pwdp = getpwuid(meUid);
    if (pwdp != NULL) {
        if ((homedir == NULL) && (pwdp->pw_dir != NULL))
            fileNameSetHome((meUByte *)pwdp->pw_dir);
        if ((pwdp->pw_name != NULL) && (meUserName == NULL))
            meUserName = meStrdup((meUByte *)pwdp->pw_name);
        endpwent();
    }

    if (meUserName == NULL)
        meUserName = meStrdup((meUByte *)"user");

    /* meUserPath may already be set from -v command-line option */
    if ((((ss = meUserPath) != NULL) && (ss[0] != '\0')) ||
        (((ss = meGetenv("MEUSERPATH")) != NULL) && (ss[0] != '\0'))) {
        ll = meStrlen(ss);
        if (ss[ll - 1] == DIR_CHAR)
            ll--;
        meUserPath = meMalloc(ll + 2);
        memcpy(meUserPath, ss, ll);
        meUserPath[ll++] = DIR_CHAR;
        meUserPath[ll]   = '\0';
    }

    if ((searchPath == NULL) &&
        ((ss = meGetenv("MEPATH")) != NULL) && (ss[0] != '\0'))
        searchPath = meStrdup(ss);

    if (searchPath != NULL) {
        /* Explicit path: just prepend meUserPath if needed */
        if ((meUserPath != NULL) &&
            (memcmp(searchPath, meUserPath, ll - 1) ||
             ((searchPath[ll - 1] != mePATH_CHAR) &&
              ((searchPath[ll - 1] != DIR_CHAR) ||
               (searchPath[ll] != mePATH_CHAR))))) {
            ss = meMalloc(ll + meStrlen(searchPath) + 2);
            memcpy(ss, meUserPath, ll);
            ss[ll] = mePATH_CHAR;
            meStrcpy(ss + ll + 1, searchPath);
            meFree(searchPath);
            searchPath = ss;
        }
        gotPaths = 8;
    } else {
        gotPaths = (meUserPath != NULL) ? 8 : 0;
        if (gotPaths)
            meStrcpy(evalResult, meUserPath);
        else {
            evalResult[0] = '\0';
            ll = 0;
        }

        if (((ss = meGetenv("MEINSTALLPATH")) != NULL) && (ss[0] != '\0')) {
            meStrcpy(buff, ss);
            ll = mePathAddSearchPath(ll, evalResult, buff, 6, &gotPaths);
        }

        if ((homedir != NULL) && (gotPaths != 0x0f)) {
            ii = gotPaths;
            meStrcpy(buff, homedir);
            meStrcat(buff, ".config/jasspa");
            ll = mePathAddSearchPath(ll, evalResult, buff, 6, &ii);
            gotPaths |= (ii & 0x0c);
        }

        if ((ss = meStrrchr(meProgName, DIR_CHAR)) != NULL) {
            ii = (int)(((size_t)ss) - ((size_t)meProgName));
            meStrncpy(buff, meProgName, ii);
            buff[ii] = '\0';
            ll = mePathAddSearchPath(ll, evalResult, buff, 9, &gotPaths);
        }

#if MEOPT_TFS
        if (tfsdev != NULL)
            ll = mePathAddSearchPath(ll, evalResult, (meUByte *)"tfs://", 1, &gotPaths);
#endif

        if (!(gotPaths & 2) && (lpath[0] != '\0')) {
            meStrcpy(buff, lpath);
            ll = mePathAddSearchPath(ll, evalResult, buff, 6, &gotPaths);
        }

        if (ll > 0)
            searchPath = meStrdup(evalResult);
    }

    if (meUserPath == NULL) {
        if ((gotPaths & 0x08) && (searchPath != NULL)) {
            ss = searchPath;
#if MEOPT_TFS
#if mePATH_CHAR == ':'
            if ((ss[0] == 't') && (ss[1] == 'f') && (ss[2] == 's') && (ss[3] == ':'))
                ss += 4;
#endif
#endif
            if ((ss = meStrchr(ss, mePATH_CHAR)) != NULL)
                ll = (int)(ss - searchPath);
            else
                ll = meStrlen(searchPath);
            if (searchPath[ll - 1] == DIR_CHAR)
                ll--;
            meUserPath = meMalloc(ll + 2);
            memcpy(meUserPath, searchPath, ll);
            meUserPath[ll++] = DIR_CHAR;
            meUserPath[ll]   = '\0';
        } else {
            meUserPath = meStrdup((meUByte *)"tfs://new-user/");
        }
    }
}

/* =========================================================================
 * meFileGetAttributes
 *
 * Return the st_mode of a file (or -1 if stat fails).
 * Declared extern in eextrn.h for _UNIX builds.
 * ====================================================================== */
meInt
meFileGetAttributes(meUByte *fname)
{
    struct stat statbuf;
    if (stat((char *)fname, &statbuf) != 0)
        return -1;
    return statbuf.st_mode;
}

/* =========================================================================
 * meGidInGidList
 *
 * Return 1 if gid is in the supplementary group list, 0 otherwise.
 * ====================================================================== */
int
meGidInGidList(gid_t gid)
{
    int ii = meGidSize;
    while (--ii >= 0)
        if (meGidList[ii] == gid)
            return 1;
    return 0;
}
