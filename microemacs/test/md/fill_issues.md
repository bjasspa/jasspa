---
# <!-- -!- markdown; fill-column:78; -!- -->
title: "MD examples which causes fill issues"
author: Steven Phillips
date: 2026-02-20 15:10
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

### Problem 3

what to do with embedded html tags? treat like code blocks?

<a href="http://www.youtube.com/watch_something" target="_blank"> <img src="http://img.youtube.com/vi/YOUTUBE_VIDEO_ID_HERE/0.jpg" alt="IMAGE ALT TEXT HERE" width="240" height="180" border="10"> </a>

### Problem 4

* [pantcl.html](https://htmlpreview.github.io/?https://raw.githubusercontent.com/mittelmark/pantcl/master/pantcl.html) - main documentation
* [pantcl-tutorial.html](https://htmlpreview.github.io/?https://raw.githubusercontent.com/mittelmark/pantcl/master/pantcl-tutorial.html) - more extensive tutorial

#### Expected

* [pantcl.html](https://htmlpreview.github.io/?https://raw.githubusercontent.com/mittelmark/pantcl/master/pantcl.html) - 
  main documentation
* [pantcl-tutorial.html](https://htmlpreview.github.io/?https://raw.githubusercontent.com/mittelmark/pantcl/master/pantcl-tutorial.html) - 
  more extensive tutorial

### Problem 5 - breaking text in parenthesis

We use here $buffer-fill-col = 78.

Some text with later will be filled with some abbreviations written in (parenthesis) and then some more text.

Output:

Some text with later will be filled with some abbreviations written in (
parenthesis) and then some more text.

Expected output:

Some text with later will be filled with some abbreviations written in
(parenthesis) and then some more text.


### Problem Next ...




