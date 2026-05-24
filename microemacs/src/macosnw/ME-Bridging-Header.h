/* ME-Bridging-Header.h
 * JASSPA MicroEmacs - native macOS frontend
 *
 * Swift bridging header.  Exposes the macosterm.h C API to all Swift
 * source files in the target.  Set this file as the target's
 * "Objective-C Bridging Header" in Xcode, or pass it via swiftc's
 * -import-objc-header flag (see macosnative.mak).
 */

#ifndef ME_BRIDGING_HEADER_H
#define ME_BRIDGING_HEADER_H

#include "macosterm.h"

#endif /* ME_BRIDGING_HEADER_H */
