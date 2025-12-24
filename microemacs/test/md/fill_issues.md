---
<!-- -!- md; fill-col:78; -!- -->
title: "MD examples which causes fill issues"
author: Steven Phillips
date: 2025-12-24 17:19
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
- The file synchronization will keep one file of the "workAspace synced with oneA", or multiple files in GoogleADrive, DropboxA or AGitHubA.
- The file synchronization will keep one file of the "workBBspace synced with oneBB", or multiple files in GoogleBBDriveBB, BBDropbox or GitHubBB.
- The file synchronization will keep one file of the "work**space synced with one**", or multiple files in Google**Drive**, **Dropbox or GitHub**.

    The file synchronization will keep one file of the "work**space synced with one**", or multiple files in Google**Drive**, **Dropbox or GitHub**.
    - Before starting to sync files, you must **link an account in the Synchronize** sub-menu.

        The file synchronization will keep one file of the "work**space synced with one**", or multiple files in Google**Drive**, **Dropbox or GitHub**.

- The file synchronization will keep one file of the "work*space synced with one*", or multiple files in Google*Drive, Dropbox* or *GitHub*.
    - Before starting to sync files, you must *link an account in the Synchronize* sub-menu.

- The file synchronization will keep one file of the "__work__space_synced with one__", or multiple files in __GoogleDrive__, __Dropbox or GitHub__.
    - Before starting to sync files, you must __link an account in the Synchronize__ sub-menu.

- The file synchronization will keep one file of the "_work_space synced with one_", or multiple files in _Google_Drive, Dropbox_ or _GitHub_.
    - Before starting to sync files, you must _link an account in the Synchronize_ sub-menu.

        The file synchronization will keep one file of the "_work_space synced with one_", or multiple files in _Google_Drive, Dropbox_ or _GitHub_.

### Problem Next...



