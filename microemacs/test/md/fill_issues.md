---
title: "MD examples which causes fill issues"
author: Steven Phillips
date: 2025-12-22 14:24
abstract: >
    Some abstract ...
    on several lines...
---

### Problem 1

- Item list 1a
  this should be treated
  like normal item text
- Item list 1b

    - Item list 1b2a
      this should also be treated
      like normal item text
      
- Item list 1c

#### Expected

- Item list 1a this should be treated like normal item text
- Item list 1b

    - Item list 1b2a this should also be treated like normal item text
      
- Item list 1c


### Problem 2

- The bold text should not split over multiple lines when wrapped.
- The file synchronization will keep one file of the workspace **synced with one or multiple files in Google Drive, Dropbox or GitHub**.
	> Before starting to sync files, you must **link an account in the Synchronize** sub-menu.


### Problem Next...



