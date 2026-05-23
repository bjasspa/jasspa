/* -*- swift -*- 
 * Copyright (C) 2026 Maxinity Software Ltd (maxinity.co.uk).
 *
 * All rights reserved.
 * 
 * This document may not, in whole or in part, be copied, photocopied,
 * reproduced, translated, or reduced to any electronic medium or machine
 * readable form without prior written consent from Maxinity Software Ltd.
 *
 * Synopsis:    
 * Authors:     Steven Phillips
 */
import Cocoa

class MEApplication: NSApplication {

    let appDelegate = MEAppDelegate()

    override init() {
        super.init();
        self.delegate = appDelegate;
    }
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}
