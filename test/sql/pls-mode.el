;;; pls-mode.el - An Emacs major-mode for editing PL/SQL source.
;;; Copyright (C) 1996 Oracle Corp.

;;; Author:          Dmitry Nizhegorodov <dnizhego@us.oracle.com>
;;; Created:         Jan 1996
;;; Version:         1.3
;;; Last Modified:   May 29 11:22:11 1996
;;; Keywords:        PL/SQL languages ADA
;;;

;;;  This program is free software; you can redistribute it and/or
;;;  modify it under the terms of the GNU General Public License as
;;;  published by the Free Software Foundation; either version 1, or
;;;  (at your option) any later version.
;;;
;;;  This program is distributed in the hope that it will be useful,
;;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;;  General Public License for more details.
;;;
;;;  You should have received a copy of the GNU General Public License
;;;  along with this program; if not, write to the Free Software
;;;  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


;;;  CHANGE LOG
;;;  ====== ===
;;;
;;;  03/29/96 dnizhego - most regexp generation goes eval-when-compile
;;;  03/27/96 dnizhego - fixed buffer case adjuster (C-c C-b)
;;;  03/23/96 dnizhego - aded STANDARD keywords, names containing # and $
;;;  03/06/96 dnizhego - added style menu
;;;  02/15/96 dnizhego - version 1.1: using SRX, added most "hard" PLS syntax
;;;  01/11/96 dnizhego - version 1.0 created
;;;

;;; Initially derived from ada-mode.el, (C) 1994-1995 by
;;; Markus Heritsch <Markus.Heritsch@studbox.uni-stuttgart.de> and
;;; Rolf Ebert      <ebert@inf.enst.fr>

;;; USAGE
;;; =====
;;;
;;; To start using pls-mode, add the following to your .emacs file.
;;;
;;; (autoload 'pls-mode  "pls-mode" "PL/SQL Editing Mode" t)
;;; (autoload 'diana-mode  "pls-mode" "DIANA for PL/SQL Browsing Mode" t)
;;; (setq auto-mode-alist
;;;   (append '(("\\.pls$"  . pls-mode)
;;;             ("\\.sql$"  . pls-mode)
;;;             ("\\.pld$"  . diana-mode)
;;;            ) auto-mode-alist))

;;; Emacs should enter pls-mode when you load a PL/SQL source (*.pls),
;;; or DIANA dump file (*.pld).

;;; When you have entered pls-mode, you may get more info by pressing
;;; C-h m. You may also get online help describing various functions by:
;;; C-h d <Name of function you want described>

;;;

;;; FEATURES
;;; ========

;;; PLS mode helps you to properly indent PL/SQL code while you type it.
;;; Additionally, it decorates the code with fonts, colors and case.
;;; For instance, typing
;;;
;;; declare
;;; x integer;
;;; y number;
;;; begin
;;; x := y;
;;; end;
;;;
;;; would result in
;;;
;;; DECLARE
;;;    x INTEGER;
;;;    y NUMBER;
;;; BEGIN
;;;    x := y;
;;; END;
;;;
;;; PLS mode uses 4 distinct "faces" to decorate the following elements
;;; of PL/SQL programs: keywords, comments, definitions, strings.
;;;
;;; PLS mode recognizes all SQL keywords and PL/SQL reserved names and
;;; keywords, and understands other elements of PL/SQL syntax - comments,
;;; strings, double-quoted names, etc.

;;; When in .pls file, use button3 to pop up the menu of PLS-specific
;;; commands. Alternatively, the same menu can be pulled out the manu-bar.

;;; Icon-lovers and Word-heads: no PLS XEmacs icons provided so far.  Send
;;; your favorite xpm files to dnizhego@us.oracle.com.

;;; Most of the entries on PLS popup menu are annotated with "hot key"
;;; combinations - you may find it convenient to learn these key
;;; bindins over time.

;;; PLS mode supports automatic case-adjusting and font-highlighting.
;;; There are 3 styles the user can choose from (see
;;; PLS pop-up menu):
;;;
;;; Official - as in Oracle SQL & PL/SQL user guide:
;;;        DECLARE mynum NUMBER; BEGIN  mynum := mynum + 1; END;
;;; Classical - as in algol, pascal or ADA literature:
;;;        declare MYNUM number; begin  MYNUM := MYNUM + 1; end;
;;; Contemporary (identifiers are capitalized):
;;;        declare MyNum number; begin  MyNum := MyNum + 1; end;
;;; Budget - everything lowercased.
;;;
;;; After switching to another style, type C-c C-b or use menu entry
;;; "Adjust Case Buffer" to propagate the style.
;;;
;;; The default style is Official.
;;;
;;; The automatic case-adjusting can be toggled on/of from the pop-up
;;; menu. By default, the automatic case-adjusting is ON.

;;; COMMENTARY
;;; ==========
;;;
;;; This packages is derived from XEmacs Ada mode written by
;;; M. Heritsch and R. Ebert.

;;; Althoug PLSQL closely resembles ADA, there are many subtle
;;; differences between the two languages, making the direct use of
;;; ADA mode very difficult. Among the things that confuse any ADA
;;; mode: %-delimited type-modifying attributes, ()-grouped
;;; record/type attributes, separated with ",", hanging toplevel "/",
;;; C-style comments, "pls 2quoted names", embedded SQL just to
;;; mention few. IN addition, PLSQL carries its own set of reserved
;;; words and keywords and a hearty supply of SQL keywords.

;;; In the current version of PLS mode, complex regesps are handled
;;; with the Structured Regexp package.  Some existing bugs are fixed,
;;; the pretty-printer can work even in absence of an external
;;; formatting tool, althoug the performance leaves a lot to
;;; desire.

;;; Some ADA constucts that never made it into PL/SQL (tasks,
;;; generics) are still "supported" and may affect certain PLSQL
;;; constructs. Please report such atavisms to dnizhego@us.oracle.com
;;;


;;; FUTURE
;;; =======
;;;
;;; We plan to extend this package in several directions:
;;; - improve speed
;;; - support C-M-[fb] that jump between matching begin/else if/then/else[if]
;;; - add flags for versions of PLS.
;;; - support `compile-buffer' `compile-line', `comile-region'
;;; - support simultaneous browsing of .pls .pld files.
;;; - better design of the pop-up/menu-bar menues.
;;; - more support for DIANA files ( *.pld)
;;; - add support for other pls-compier-generated formats (.plm, etc)
;;; - add support for SQL files (*.sql)

;;; PROBLEMS
;;; ==========
;;;
;;; 1. Many ADA constructs not supported by PL/SQL are still recognized,
;;; which may affect appearance of some PLSQL code.
;;; Just to name few: tasks, generics, private, limited.
;;;
;;; 2. Trailing ")" if placed on a separate line, will be indented
;;; up to "(", regardless what is the value of `pls-indent-to-open-paren'
;;;
;;; define TYPE emp_ad (
;;;   idxn NUMBER,
;;;   sal VARCHAR2(20)  <----- put it here instead !!
;;; -wh-o-o-o------->  );
;;;
;;; In the presence of comments and/or incorrect syntax
;;; pls-format-paramlist produces weird results.
;;;

;;; 3. Comments at the very beginning of the buffer (_before_ any
;;; code) are not indented to the left and not aligned with comments
;;; above.  Also, the very first top-level form can be positioned
;;; "freely" and will not be forced to the left by the indenter.
;;; Thus, it is possible to justify the contents of the whole buffer
;;; to the rigth by shifting the very first code line and then C-c
;;; C-p.  Consider this a bug or a feature?


;;; RANDOM ISSUES
;;; ====== ======
;;;
;;; For those who is working with PL/SQL examples from an excellent
;;; book `ORACLE PL/SQL Programming" by Steven Feuerstein.
;;;
;;; By adding this to your .emacs file you'll automatically
;;; bring pls mode when opening each of pls/sql files from the
;;; companion floppy:
;;;
;;; (setq auto-mode-alist
;;;   (append
;;;    '(
;;; 	 ("\\.ff$" . pls-mode)  ;; Oracle Forms function
;;; 	 ("\\.fp$" . pls-mode)  ;; Oracle Forms procedure
;;; 	 ("\\.fpp$" . pls-mode) ;; Oracle Forms package
;;; 	 ("\\.rpp$" . pls-mode) ;; Oracle Reports package
;;; 	 ("\\.sf$" . pls-mode)  ;; Stored function
;;; 	 ("\\.sp$" . pls-mode)  ;; Stored procedure
;;; 	 ("\\.spb$" . pls-mode) ;; Stored package body
;;; 	 ("\\.spp$" . pls-mode) ;; Stored package specand body
;;; 	 ("\\.sps$" . pls-mode) ;; Stored package specification
;;; 	 ("\\.sql$" . pls-mode) ;; SQL script
;;; 	 ) auto-mode-alist))



;;;--------------------
;;;    Dependencies
;;;--------------------

(eval-when-compile (require 'structured-regexp))

(defvar pls-mode-loaded t)
(eval-when-compile (or (boundp 'pls-mode-loaded) (load-file "pls-mode.el")))

;;;--------------------
;;;    USER OPTIONS
;;;--------------------

;;; ---- configure indentation

(defvar pls-indent 3
  "*Defines the size of Pls indentation.")

(defvar pls-broken-indent 2
  "*# of columns to indent the continuation of a broken line.")

(defvar pls-label-indent -4
  "*# of columns to indent a label.")

(defvar pls-stmt-end-indent 0
  "*# of columns to indent a statement end keyword in a separate line.
Examples are 'is', 'loop', 'record', ...")

(defvar pls-when-indent 3
  "*Defines the indentation for 'when' relative to 'exception' or 'case'.")

(defvar pls-indent-record-rel-type 3
  "*Defines the indentation for 'record' relative to 'type' or 'use'.")

(defvar pls-indent-comment-as-code t
  "*If non-nil, comment-lines get indented as pls-code.")

(defvar pls-indent-is-separate t
  "*If non-nil, 'is separate' or 'is abstract' on a separate line are
indented.")

(defvar pls-indent-to-open-paren nil
  "*If non-nil, following lines get indented according to the innermost
open parenthesis.")

(defvar pls-search-paren-char-count-limit 3000
  "*Search that many characters for an open parenthesis.")


;; ---- other user options

(defvar pls-tab-policy 'indent-auto
  "*Control behaviour of the TAB key.
Must be one of 'indent-rigidly, 'indent-auto, 'gei, 'indent-af or 'always-tab.

'indent-rigidly : always adds pls-indent blanks at the beginning of the line.
'indent-auto    : use indentation functions in this file.
'gei            : use David Kegedal's Generic Indentation Engine.
'indent-af      : use Gary E. Barnes' ada-format.el
'always-tab     : do indent-relative.")

(defvar pls-move-to-declaration nil
  "*If non-nil, pls-move-to-start moves point to the subprog-declaration,
not to 'begin'.")

(defvar pls-spec-suffix ".pls"
  "*Suffix of Pls specification files.")

(defvar pls-body-suffix ".pls"
  "*Suffix of Pls body files.")

(defvar pls-case-keyword 'upcase-word
  "*According to the Oracle PL/SQL preferred style, reserved words and
keywords are all uppercased. Alternatives are: downcase-word,
upcase-word, pls-loose-case-word or capitalize-word to adjust pls
keywords case.")

(defvar pls-case-identifier 'downcase-word
  "*downcase-word, upcase-word, pls-loose-case-word or capitalize-word
to adjust pls identifier case.")

(defvar pls-case-attribute 'upcase-word
  "*downcase-word, upcase-word, pls-loose-case-word or capitalize-word
to adjust pls identifier case.")

(defvar pls-auto-case t
  "*Non-nil automatically changes casing of preceeding word while typing.
Casing is done according to pls-case-keyword and pls-case-identifier.")

(defvar pls-clean-buffer-before-saving  nil
  "*If non-nil, remove-trailing-spaces and untabify buffer before saving.")

(defvar pls-mode-hook nil
  "*List of functions to call when Pls Mode is invoked.
This is a good place to add Pls environment specific bindings.")

(defvar pls-external-pretty-print-program nil
  "*External pretty printer to call from within PL/SQL Mode.")

(defvar pls-tmp-directory "/tmp/"
  "*Directory to store the temporary file for the Pls pretty printer.")

(defvar pls-fill-comment-prefix "-- "
  "*This is inserted in the first columns when filling a comment paragraph.")

(defvar pls-fill-comment-postfix " --"
  "*This is inserted at the end of each line when filling a comment paragraph
with pls-fill-comment-paragraph postfix.")

(defvar pls-krunch-args "0"
  "*Argument of gnatk8, a string containing the max number of characters.
Set to 0, if you dont use crunched filenames.")

;;; ---- end of user configurable variables


(defvar pls-mode-abbrev-table nil
  "Abbrev table used in Pls mode.")
(define-abbrev-table 'pls-mode-abbrev-table ())

(defvar pls-mode-map ()
  "Local keymap used for pls-mode.")

(defvar pls-mode-syntax-table nil
  "Syntax table to be used for editing Pls source code.")

(defvar pls-mode-symbol-syntax-table nil
  "Syntax table for Pls, where `_' is a word constituent.")

;;; ---- The plethora of PL/SQL keywords broken into several groups


(defconst pls-ada-83-keywords
 '(

   "abort" "abs" "accept" "access" "all" "and" "array" "at" "begin"
   "body" "case" "constant" "declare" "delay" "delta" "digits" "do"
   "else" "elsif" "end" "entry" "exception" "exit" "for" "function"
   "generic" "goto" "if" "in" "is" "limited" "loop" "mod" "new" "not"
   "null" "of" "or" "others" "out" "package" "pragma" "private"
   "procedure" "raise" "range" "record" "rem" "renames" "return"
   "reverse" "select" "separate" "subtype" "task" "terminate" "then"
   "type" "use" "when" "wherer" "while" "with" "xor"

   ) "complete list of ADA-83 keywords")

(defconst pls-ada-95-extra-keywords
  '("abstract" "aliased" "protected" "requeue" "tagged" "task" "until")
  "list of keywords added in ADA-95")

(defconst pls-sql-keywords
 '(

   "add" "all" "alter" "and" "any" "as" "asc" "by" "check" "between"
   "cluster" "connect" "compress" "create" "current" "date" "decimal"
   "default" "delete" "desc" "distinct" "drop" "else" "exists" "float"
   "for" "from" "grant" "group" "having" "identified" "in" "index"
   "insert" "integer" "intersect" "into" "is" "level" "like" "minus"
   "mode" "not" "null" "number" "of" "on" "or" "order" "prior"
   "public" "rename" "resource" "revoke" "rowlabel" "rownum" "select"
   "set" "size" "smallint" "start" "table" "then" "to" "unique"
   "union" "update" "values" "varchar" "varchar2" "view" "where"
   "with" "ascii" "chr" "concat" "initcap" "instr" "length" "lower" "lpad"
   "ltrim" "rpad" "rtrim" "soundex" "substr" "translate" "upper" 
   "dbms_output" "dbms_sql"

   )
  "Oracle SQL keywords  used in PL/SQL source")

(defconst pls-plsql-standard-type-keywords
 '(

   "dec" "in" "int" "boolean" "date_base" "number_base" "real"
   "numeric" "binary_integer" "natural" "naturaln" "positive"
   "positiven" "signtype" "string" "raw" "rowid" "char" "character"
   "mlslabel" "pls_integer"

   ) "PL/SQL types defined in package   STANDARD")

(defconst pls-plsql-extra-keywords
 '(
   "define" "open" "close"   "fetch"
   ) "keywords specific to PL/SQL")

(defconst pls-3-keywords
 '(
   "member" "current" "ref" "alias" "value" "oid"
   )
  "New keywords introduced in PL/SQL version 3")

(defconst pls-3-x-keywords
 '(
   )
  "New keywords planned for PL/SQL 3.X")

(defconst pls-object-plsql-keywords
 '(
   "class" "interface" "implements"
   "under"
   )
  "New keywords we plan for Object PL/SQL - (3.X ... 4.0)")

(defconst pls-definitive-keywords
 '(
   "accept" "entry" "function" "package" "body" "procedure" "type" "table"
   )
  "keywords that used to define PL/SQL named programmatic constructs")

(defconst pls-minimalists-keywords
 '(

   "begin" "declare" "do" "else" "elsif" "end"
   "exception" "exit" "for" "if" "or" "package" "pragma" "raise"
   "record" "return" "select" "then" "type" "use" "when" "while" "loop"
   "insert" "delete" "function"  "table" "create" "define" "exception"
   "in" "into" "at" "as" "insert" "update"

   )
  "list of the most important PL/SQL keywords")

(defvar pls-full-keyword-list
  (eval-when-compile
    (append
     pls-ada-83-keywords
     pls-ada-95-extra-keywords
     pls-sql-keywords
     pls-plsql-extra-keywords
     pls-3-keywords
     pls-plsql-standard-type-keywords
     ))
  " full list of PL/SQL keywords")

;;(ip pls-full-keyword-list)
;;(ip pls-keywords-regexp)



;;;----------------------------------
;;;    Regular Expressions (regexps)
;;;----------------------------------

(defvar pls-keywords-regexp
  (eval-when-compile
    (srx-word
     (srx-build-regex pls-full-keyword-list 3)))
  "regular expression for looking at Pls
  keywords.")

(defvar pls-ret-binding nil
  "Variable to save key binding of RET when casing is activated.")

(defvar pls-lfd-binding nil
  "Variable to save key binding of LFD when casing is activated.")

;;; ---- Regexps to find identifiers.names/procedures/functions/packages

(defconst pls-identifier-charset
  "a-zA-Z0-9_$#"
  "A set of legal PLS identifier characters.")

(defconst pls-name-charset
  (eval-when-compile
    (concat pls-identifier-charset "\\."))
  "A set of legal PLS name characters.")

(defconst pls-identifier-char-regexp
  (eval-when-compile
    (concat "[" pls-identifier-charset "]"))
  "Regexp matching a legal PLS identifier character.")

(defconst pls-identifier-regexp
  (eval-when-compile
    (concat pls-identifier-char-regexp "+"))
  "Regexp matching PLS identifiers.")

(defconst pls-name-regexp
  (eval-when-compile
    (concat "[" pls-name-charset "]+"))
  "Regexp matching PLS names.")

(defvar pls-procedure-start-regexp
  (eval-when-compile
    (concat
     "^[ \t]*"
     (srx-or "procedure" "function" "task")
     "[ \t\n]+"
     (srx-or pls-identifier-regexp)))
  "Regexp used to find Pls procedures/functions.")

(defvar pls-package-start-regexp
  "^[ \t]*\\(package\\)"
  "Regexp used to find Pls packages")


;;; ---- regexps for indentation functions

(defvar pls-block-start-re
  "\\<\\(begin\\|declare\\|private\\|or\\|generic\\|\
exception\\|loop\\|else\\|\
\\(\\(limited\\|abstract\\|tagged\\)[ \t]+\\)*record\\)\\>"
  "Regexp for keywords starting  blocks.")

(defvar pls-end-stmt-re
  (eval-when-compile
    (concat
     "\\(;\\|=>\\|^[ \t]*separate[ \t]+("
     pls-name-regexp ")\\|\
\\<\\(begin\\|else\\|record\\|loop\\|select\\|do\\|\
^[ \t]*package[ \t]+" pls-name-regexp "is\\|\
^[ \t]*exception\\|declare\\|generic\\|private\\)\\>\\|\/\\)"))
  "Regexp of possible ends for a non-broken statement.'end' means that there has to start a new statement after these.")

(defvar pls-loop-start-re
  (eval-when-compile
    (srx-word
     (srx-build-tree
      "for" "while" "loop")))
  "Regexp for the start of a loop.")

(defvar pls-subprog-start-re
  (eval-when-compile
    (srx-word
     (srx-build-tree
      "procedure" "protected" "package[ \t]+body" "function")))
  "Regexp for the start of a subprogram.")

;;; ---- random parameters


(defvar font-lock-highlighting-of-Nth-regexp-broken-nesting  t)


;;;-------------
;;;  functions
;;;-------------

(defun pls-xemacs ()
  (or (string-match "Lucid"  emacs-version)
      (string-match "XEmacs" emacs-version)))

;; (pls-create-syntax-table)

(defun pls-create-syntax-table ()
  "Create the syntax table for pls-mode."
  ;; There are two different syntax-tables.  The standard one declares
  ;; `_' a symbol constituent, in the second one, it is a word
  ;; constituent.  For some search and replacing routines we
  ;; temporarily switch between the two.
  (setq pls-mode-syntax-table (make-syntax-table))
  (set-syntax-table  pls-mode-syntax-table)

  (modify-syntax-entry ?\' "\"" pls-mode-syntax-table)

  (modify-syntax-entry ?\%  "." pls-mode-syntax-table)

  ;; DN: having pl/sql double quoted names exposed as strings
  ;; is the least possible evil: any attempt to avoid that
  ;; I tried resulted in a mess. Remeber that _any_ character can
  ;; appear inside of a double quoted name and that emacs does not,
  ;; alas, provide more than one "style" of strings.
  (modify-syntax-entry ?\" "\"" pls-mode-syntax-table)

  (modify-syntax-entry ?:  "." pls-mode-syntax-table)
  (modify-syntax-entry ?\; "." pls-mode-syntax-table)
  (modify-syntax-entry ?&  "." pls-mode-syntax-table)
  (modify-syntax-entry ?\|  "." pls-mode-syntax-table)
  (modify-syntax-entry ?+  "." pls-mode-syntax-table)

  (modify-syntax-entry ?=  "." pls-mode-syntax-table)
  (modify-syntax-entry ?<  "." pls-mode-syntax-table)
  (modify-syntax-entry ?>  "." pls-mode-syntax-table)
  (modify-syntax-entry ?$ "." pls-mode-syntax-table)
  (modify-syntax-entry ?\[ "." pls-mode-syntax-table)
  (modify-syntax-entry ?\] "." pls-mode-syntax-table)
  (modify-syntax-entry ?\{ "." pls-mode-syntax-table)
  (modify-syntax-entry ?\} "." pls-mode-syntax-table)
  (modify-syntax-entry ?. "." pls-mode-syntax-table)
  (modify-syntax-entry ?\\ "." pls-mode-syntax-table)

  ;; a single hyphen is punctuation, but a double hyphen starts a comment
  (modify-syntax-entry ?-  ". 12" pls-mode-syntax-table)

  ;; and \f and \n end a comment
  (modify-syntax-entry ?\f  "> b" pls-mode-syntax-table)
  (modify-syntax-entry ?\n  "> b" pls-mode-syntax-table)

  (if t  ;; PLS has C-style comments
      (progn
	(cond
	 ((pls-xemacs)
	  ;; XEmacs (formerly Lucid) has the best implementation
	  (modify-syntax-entry ?/  ". 14" pls-mode-syntax-table)
	  (modify-syntax-entry ?*  ". 23"   pls-mode-syntax-table))
	 (t
	  ;; FSF Emacs 19 does things differently, but we can work with it
	  (modify-syntax-entry ?/  ". 124b" pls-mode-syntax-table)
	  (modify-syntax-entry ?*  ". 23"   pls-mode-syntax-table)
	  ))))
  ;; define what belongs in pls symbols
  (modify-syntax-entry ?_  "_" pls-mode-syntax-table)
  (modify-syntax-entry ?\# "_" pls-mode-syntax-table)
  (modify-syntax-entry ?$  "_" pls-mode-syntax-table)

  ;; define parentheses to match
  (modify-syntax-entry ?\( "()" pls-mode-syntax-table)
  (modify-syntax-entry ?\) ")(" pls-mode-syntax-table)

  (setq pls-mode-symbol-syntax-table (copy-syntax-table pls-mode-syntax-table))
  (modify-syntax-entry ?_ "w" pls-mode-symbol-syntax-table)
  )


;;;###autoload
(defun pls-mode ()
  "Pls Mode is the major mode for editing PL/SQL code.

Bindings are as follows: (Note: 'LFD' is control-j.)

 Indent line                               '\\[pls-tab]'
 Indent line, insert newline and indent it '\\[newline-and-indent]'

 Re-format the parameter-list point is in  '\\[pls-format-paramlist]'
 Indent all lines in region                '\\[pls-indent-region]'

 Adjust case in region                     '\\[pls-adjust-case-region]'
 Adjust case in buffer                     '\\[pls-adjust-case-buffer]'

 Indent entire  buffer                       '\\[pls-call-pretty-printer]'
 Call EXTERNAL pretty printer, if any      '\\[pls-call-pretty-printer]'

 Fill comment paragraph                    '\\[pls-fill-comment-paragraph]'
 Fill comment paragraph and justify  '\\[pls-fill-comment-paragraph-justify]'
 Fill comment paragraph, justify and
                    append postfix   '\\[pls-fill-comment-paragraph-postfix]'

 Next func/proc/task                 '\\[pls-next-procedure]'
 Previous func/proc/task             '\\[pls-previous-procedure]'

 Next package                        '\\[pls-next-package]'
 Previous package                    '\\[pls-previous-package]'

 Goto matching start of current `end'            '\\[pls-move-to-start]'
 Goto end of current block                       '\\[pls-move-to-end]'

Comments are handled using standard GNU Emacs conventions, including:
 Start a comment                                 '\\[indent-for-comment]'
 Comment region                                  '\\[comment-region]'
 Uncomment region                                '\\[pls-uncomment-region]'
 Continue comment on next line                   '\\[indent-new-comment-line]'

If you use imenu.el:
 Display index-menu of functions & procedures    '\\[imenu]'

If you use find-file.el:
 Switch to other file (Body <-> Spec)            '\\[ff-find-other-file]'
                                              or '\\[ff-mouse-find-other-file]
 Switch to other file in other window            '\\[pls-ff-other-window]'
                              or '\\[ff-mouse-find-other-file-other-window]
 If you use this function in a spec and no body is available, it gets created
 with body stubs.

If you use pls-xref.el:
 Goto declaration:          '\\[pls-point-and-xref]' on the identifier
                         or '\\[pls-goto-declaration]' with point there
 Complete identifier:       '\\[pls-complete-identifier]'
 Execute Gnatf:             '\\[pls-gnatf-current]'"

  (interactive)
  (kill-all-local-variables)

  (make-local-variable 'require-final-newline)
  (setq require-final-newline t)

  (make-local-variable 'comment-start)
  (setq comment-start "-- ")

  ;; comment end must be set because it may hold a wrong value if
  ;; this buffer had been in another mode before. RE
  (make-local-variable 'comment-end)
  (setq comment-end "")

  (make-local-variable 'comment-start-skip) ;; used by autofill
  (setq comment-start-skip "--+[ \t]*")

  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'pls-indent-current-function)

  (make-local-variable 'fill-column)
  (setq fill-column 75)

  (make-local-variable 'comment-column)
  (setq comment-column 40)

  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)

  (make-local-variable 'case-fold-search)
  (setq case-fold-search t)

  (make-local-variable 'fill-paragraph-function)
  (setq fill-paragraph-function 'pls-fill-comment-paragraph)

  ;;(make-local-variable 'font-lock-defaults)
  ;;(setq font-lock-defaults '(pls-font-lock-keywords-spec nil t ((?\_ . "w"))))

  (setq major-mode 'pls-mode)
  (setq mode-name "PL/SQL")

  (setq blink-matching-paren t)

  (use-local-map pls-mode-map)

  (if pls-mode-syntax-table
      (set-syntax-table pls-mode-syntax-table)
    (pls-create-syntax-table))

  (if pls-clean-buffer-before-saving
      (progn
	;; remove all spaces at the end of lines in the whole buffer.
	(add-hook 'local-write-file-hooks 'pls-remove-trailing-spaces)
	;; convert all tabs to the correct number of spaces.
	(add-hook 'local-write-file-hooks 'pls-untabify-buffer)))


  ;; add menu 'Pls' to the menu bar
  (pls-add-pls-menu)

  ;;(make-local-variable 'font-lock-keywords)
  (setq font-lock-keywords
;pm	(if font-lock-use-maximal-decoration 
	(if font-lock-maximum-decoration 
	    pls-font-lock-keywords-spec-max-decoration
	  pls-font-lock-keywords-spec-min-decoration))

  ;;(make-local-variable 'font-lock-keywords-case-fold-search)
  (setq font-lock-keywords-case-fold-search t)

  (put 'pls-mode 'font-lock-keywords-case-fold-search t)

  (run-hooks 'pls-mode-hook)

  ;; the following has to be done after running the pls-mode-hook
  ;; because users might want to set the values of these variable
  ;; inside the hook (MH)

  (if pls-auto-case
      (pls-activate-keys-for-case))

)


(defvar pls-diana-mode-map nil)

;;;###autoload
(defun diana-mode ()
  "mode for browsing DIANA code"
  (interactive)
  ;;; so far, just use c-mode
  ;;(c-mode)
  (if pls-diana-mode-map
      nil
    (autoload 'c-mode-map "cc-mode.el")
    (setq pls-diana-mode-map (copy-keymap c-mode-map))
    )
  (set-buffer (current-buffer))
  (kill-all-local-variables)
  (use-local-map pls-diana-mode-map)
  (setq buffer-read-only t)
  (setq mode-name "DIANA mode")
  ;;;(set (make-local-variable 'indent-line-function) 'c-indent-line)
  (set (make-local-variable 'bar-cursor) t)
  (pls-add-diana-menu)
)







;;;--------------------------
;;;  Fill Comment Paragraph
;;;--------------------------

(defun pls-fill-comment-paragraph-justify ()
  "Fills current comment paragraph and justifies each line as well."
  (interactive)
  (pls-fill-comment-paragraph t))


(defun pls-fill-comment-paragraph-postfix ()
  "Fills current comment paragraph and justifies each line as well.
Prompts for a postfix to be appended to each line."
  (interactive)
  (pls-fill-comment-paragraph t t))


(defun pls-fill-comment-paragraph (&optional justify postfix)
  "Fills the current comment paragraph.
If JUSTIFY is non-nil, each line is justified as well.
If POSTFIX and JUSTIFY are  non-nil, pls-fill-comment-postfix is appended
to each filled and justified line.
If pls-indent-comment-as code is non-nil, the paragraph is idented."
  (interactive "P")
  (let ((opos (point-marker))
        (begin nil)
        (end nil)
        (end-2 nil)
        (indent nil)
        (pls-fill-comment-old-postfix "")
        (fill-prefix nil))

    ;; check if inside comment
    (if (not (pls-in-comment-p))
        (error "not inside comment"))

    ;; prompt for postfix if wanted
    (if (and justify
             postfix)
        (setq pls-fill-comment-postfix
              (read-from-minibuffer "enter new postfix string: "
                                    pls-fill-comment-postfix)))

    ;; prompt for old postfix to remove if necessary
    (if (and justify
             postfix)
        (setq pls-fill-comment-old-postfix
              (read-from-minibuffer "enter already existing postfix string: "
                                    pls-fill-comment-postfix)))

    ;;
    ;; find limits of paragraph
    ;;
    (message "filling comment paragraph ...")
    (save-excursion
      (back-to-indentation)
      ;; find end of paragraph
      (while (and (looking-at "--.*$")
                  (not (looking-at "--[ \t]*$")))
        (forward-line 1)
        (back-to-indentation))
      (beginning-of-line)
      (setq end (point-marker))
      (goto-char opos)
      ;; find begin of paragraph
      (back-to-indentation)
      (while (and (looking-at "--.*$")
                  (not (looking-at "--[ \t]*$")))
        (forward-line -1)
        (back-to-indentation))
      (forward-line 1)
      ;; get indentation to calculate width for filling
      (pls-indent-current)
      (back-to-indentation)
      (setq indent (current-column))
      (setq begin (point-marker)))

    ;; delete old postfix if necessary
    (if (and justify
             postfix)
        (save-excursion
          (goto-char begin)
          (while (re-search-forward (concat pls-fill-comment-old-postfix
                                            "\n")
                                    end t)
            (replace-match "\n"))))

    ;; delete leading whitespace and uncomment
    (save-excursion
      (goto-char begin)
      (beginning-of-line)
      (while (re-search-forward "^[ \t]*--[ \t]*" end t)
        (replace-match "")))

    ;; calculate fill width
    (setq fill-column (- fill-column indent
                         (length pls-fill-comment-prefix)
                         (if postfix
                             (length pls-fill-comment-postfix)
                           0)))
    ;; fill paragraph
    (fill-region begin (1- end) justify)
    (setq fill-column (+ fill-column indent
                         (length pls-fill-comment-prefix)
                         (if postfix
                             (length pls-fill-comment-postfix)
                           0)))
   ;; find end of second last line
    (save-excursion
      (goto-char end)
      (forward-line -2)
      (end-of-line)
      (setq end-2 (point-marker)))

    ;; re-comment and re-indent region
    (save-excursion
      (goto-char begin)
      (indent-to indent)
      (insert pls-fill-comment-prefix)
      (while (re-search-forward "\n" (1- end-2) t)
        (replace-match (concat "\n" pls-fill-comment-prefix))
        (beginning-of-line)
        (indent-to indent)))

    ;; append postfix if wanted
    (if (and justify
             postfix
             pls-fill-comment-postfix)
        (progn
          ;; append postfix up to there
          (save-excursion
            (goto-char begin)
            (while (re-search-forward "\n" (1- end-2) t)
              (replace-match (eval-when-compile
			       (concat pls-fill-comment-postfix "\n"))))

            ;; fill last line and append postfix
            (end-of-line)
            (insert-char ?
                         (- fill-column
                            (current-column)
                            (length pls-fill-comment-postfix)))
            (insert pls-fill-comment-postfix))))

    ;; delete the extra line that gets inserted somehow(??)
    (save-excursion
      (goto-char (1- end))
      (end-of-line)
      (delete-char 1))

     (message "filling comment paragraph ... done")
    (goto-char opos))
  t)


;;;--------------------------------;;;
;;;  Pretty Print Buffer           ;;;
;;;--------------------------------;;;

(defun pls-pretty-print ()
  "Calls the external Pretty Printer, if any.See documentation for
function `pls-call-pretty-printer'.  Otherwise just indents each line in the buffer.
This might be quite slow. Line numbers are printed in the minibuffer to indicate the
progress. Can be terminated with Ctrl-g."
  (interactive)
  (if pls-external-pretty-print-program
      (pls-call-pretty-printer)
    (save-excursion
      (let ((current-line (read (substring (what-line) 5))))
	(beginning-of-buffer)
	(while (< (point) (point-max))
	  (message (what-line))
	  (pls-tab)
	  (forward-line 1))
	(goto-line current-line)
	(message "Done.")))))


;;;-----------------------------------------------;;;
;;; Call External Pretty Printer, If you have any ;;;
;;;-----------------------------------------------;;;

(defun pls-call-pretty-printer ()
  "Calls the external Pretty Printer.
The name is specified in pls-external-pretty-print-program.  Saves the
current buffer in a directory specified by pls-tmp-directory,
starts the Pretty Printer as external process on that file and then
reloads the beautyfied program in the buffer and cleans up
pls-tmp-directory."
  (interactive)
  (let ((filename-with-path buffer-file-name)
        (curbuf (current-buffer))
        (orgpos (point))
        (mesgbuf nil) ;; for byte-compiling
        (file-path (file-name-directory buffer-file-name))
        (filename-without-path (file-name-nondirectory buffer-file-name))
        (tmp-file-with-directory
         (concat pls-tmp-directory
                 (file-name-nondirectory buffer-file-name))))
    ;;
    ;; save buffer in temporary file
    ;;
    (message "saving current buffer to temporary file ...")
    (write-file tmp-file-with-directory)
    (auto-save-mode nil)
    (message "saving current buffer to temporary file ... done")
    ;;
    ;; call external pretty printer program
    ;;

    (message "running external pretty printer ...")
    ;; create a temporary buffer for messages of pretty printer
    (setq mesgbuf (get-buffer-create "Pretty Printer Messages"))
    ;; execute pretty printer on temporary file
    (call-process pls-external-pretty-print-program
                  nil mesgbuf t
                  tmp-file-with-directory)
    ;; display messages if there are some
    (if (buffer-modified-p mesgbuf)
        ;; show the message buffer
        (display-buffer mesgbuf t)
      ;; kill the message buffer
      (kill-buffer mesgbuf))
    (message "running external pretty printer ... done")
    ;;
    ;; kill current buffer and load pretty printer output
    ;; or restore old buffer
    ;;
    (if (y-or-n-p
         "Really replace current buffer with pretty printer output ? ")
        (progn
          (set-buffer-modified-p nil)
          (kill-buffer curbuf)
          (find-file tmp-file-with-directory))
      (message "old buffer contents restored"))
    ;;
    ;; delete temporary file and restore information of current buffer
    ;;
    (delete-file tmp-file-with-directory)
    (set-visited-file-name filename-with-path)
    (auto-save-mode t)
    (goto-char orgpos)))


;;;---------------
;;;  auto-casing
;;;---------------

(defun pls-after-keyword-p ()
  ;; returns t if cursor is after a keyword.
  (save-excursion
    (forward-word -1)
    (and (save-excursion
           (or
            (= (point) (point-min))
            (backward-char 1))
           (not (looking-at "[_$#]")))
         (looking-at (eval-when-compile
		       (concat pls-keywords-regexp "[^_$#]"))))))


(defun pls-adjust-case (&optional force-identifier)
  "Adjust the case of the word before the just-typed character,
according to pls-case-keyword and pls-case-identifier
If FORCE-IDENTIFIER is non-nil then also adjust keyword as
identifier."
  (if pls-auto-case
      (progn
	(forward-char -1)
	(if (and (> (point) 1) (not (or (pls-in-string-p)
					(pls-in-comment-p))))
	    (if (eq (char-syntax (char-after (1- (point)))) ?w)
		(if (save-excursion
		      (forward-word -1)
		      (or (= (point) (point-min))
			  (backward-char 1))
		      (looking-at "%"))
		    (funcall pls-case-attribute -1)
		  (if (and
		       (not force-identifier) ; (MH)
		       (pls-after-keyword-p))
		      (funcall pls-case-keyword -1)
		    (funcall pls-case-identifier -1)))))
	(forward-char 1))))



(defun pls-adjust-case-interactive (arg)
  (interactive "P")
  (if pls-auto-case
      (let ((lastk last-command-char))
	(cond ((or (eq lastk ?\n)
		   (eq lastk ?\r))
	       ;; horrible kludge
	       (insert " ")
	       (pls-adjust-case)
	       ;; horrible dekludge
	       (delete-backward-char 1)
	       ;; some special keys and their bindings
	       (cond
		((eq lastk ?\n)
		 (funcall pls-lfd-binding))
		((eq lastk ?\r)
		 (funcall pls-ret-binding))))
	      ((eq lastk ?\C-i) (pls-tab))
	      ((self-insert-command (prefix-numeric-value arg))))
	;; if there is a keyword in front of the underscore
	;; then it should be part of an identifier (MH)
	(if (eq lastk ?_)
	    (pls-adjust-case t)
	  (pls-adjust-case)))
    (self-insert-command (prefix-numeric-value arg))))


(defun pls-toggle-auto-case ()
  (interactive)
  (setq pls-auto-case (not pls-auto-case)))

(defvar pls-case-style-official-p
  (and pls-auto-case
       (eq pls-case-identifier 'downcase-word)
       (eq pls-case-keyword 'upcase-word)))

(defvar pls-case-style-classical-p nil)
(defvar pls-case-style-contemporary-p nil)
(defvar pls-case-style-relaxed-p nil)

(defun pls-set-case-style-reset-all ()
  (setq pls-case-style-official-p nil
	pls-case-style-classical-p nil
	pls-case-style-contemporary-p nil
	pls-case-style-relaxed-p nil))


(defun pls-set-case-style-official ()
  (interactive)
  (setq pls-case-keyword    'upcase-word)
  (setq pls-case-identifier 'downcase-word)
  (setq pls-case-attribute  'upcase-word)
  (pls-set-case-style-reset-all)
  (setq pls-case-style-official-p t))

(defun pls-set-case-style-classical ()
  (interactive)
  (setq pls-case-keyword    'downcase-word)
  (setq pls-case-identifier 'upcase-word)
  (setq pls-case-attribute  'downcase-word)
  (pls-set-case-style-reset-all)
  (setq pls-case-style-classical-p t))

(defun pls-set-case-style-contemporary ()
  (interactive)
  (setq pls-case-keyword    'downcase-word)
  (setq pls-case-identifier 'capitalize-word)
  (setq pls-case-attribute  'downcase-word)
  (pls-set-case-style-reset-all)
  (setq pls-case-style-contemporary-p t))

(defun pls-set-case-style-relaxed ()
  (interactive)
  (setq pls-case-keyword    'downcase-word)
  (setq pls-case-identifier 'downcase-word)
  (setq pls-case-attribute  'downcase-word)
  (pls-set-case-style-reset-all)
  (setq pls-case-style-relaxed-p t))


(defun pls-activate-keys-for-case ()
  ;; save original keybindings to allow swapping ret/lfd
  ;; when casing is activated
  ;; the 'or ...' is there to be sure that the value will not
  ;; be changed again when pls-mode is called more than once (MH)
  (or pls-ret-binding
      (setq pls-ret-binding (key-binding "\C-M")))
  (or pls-lfd-binding
      (setq pls-lfd-binding (key-binding "\C-j")))
  ;; call case modifying function after certain keys.
  (mapcar (function (lambda(key) (define-key
                                   pls-mode-map
                                   (char-to-string key)
                                   'pls-adjust-case-interactive)))
          '( ?` ?~ ?! ?@ ?# ?$ ?% ?^ ?& ?* ?( ?)  ?- ?= ?+ ?[ ?{ ?] ?}
                ?_ ?\\ ?| ?\; ?: ?' ?\" ?< ?, ?. ?> ?? ?/ ?\n 32)))
;; deleted ?\t from above list

(defun pls-loose-case-word (&optional arg)
  "Capitalizes the first and the letters following _
ARG is ignored, it's there to fit the standard casing functions' style."
  (let ((pos (point))
        (first t))
    (skip-chars-backward pls-identifier-charset)
    (while (or first
               (re-search-forward "[_$#]" pos t))
      (and first
           (setq first nil))
      (insert-char (upcase (following-char)) 1)
      (delete-char 1))
    (goto-char pos)))

(defun pls-adjust-case-region (from to)
  "Adjusts the case of all identifiers and keywords in the region.
ATTENTION: This function might take long for big regions !"
  (interactive "*r")
  (if pls-auto-case
      (let ((begin nil)
	    (end nil)
	    (keywordp nil)
	    (reldiff nil))
	(unwind-protect
	    (save-excursion
	      (set-syntax-table pls-mode-symbol-syntax-table)
	      (goto-char to)
	      ;;
	      ;; loop: look for all identifiers and keywords
	      ;;
	      (while (re-search-backward
		      pls-identifier-regexp
		      from t)
		;;
		;; print status message
		;;
		(setq reldiff (- (point) from))
		(message (format "adjusting case ... %5d characters left"
				 (- (point) from)))
		;; (forward-char 1)
		(or
		 ;; do nothing if it is a string or comment
		 (pls-in-string-or-comment-p)
		 (progn
		   ;;
		   ;; get the identifier or keyword
		   ;;
		   (setq begin (point))
		   (setq keywordp
			 (looking-at (eval-when-compile
				       (concat pls-keywords-regexp "[^_$#]"))))
		   (skip-chars-forward pls-identifier-regexp)
		   ;;
		   ;; casing according to user-option
		   ;;
		   (if keywordp
		       (funcall pls-case-keyword -1)
		     (funcall pls-case-identifier -1))
		   (goto-char begin))))
	      (message "adjusting case ... done"))
	  (set-syntax-table pls-mode-syntax-table)))
    (message "No ajustment needed")))


(defun pls-adjust-case-buffer ()
  "Adjusts the case of all identifiers and keywords in the whole buffer.
ATTENTION: This function might take very long for big buffers !"
  (interactive "*")
  (pls-adjust-case-region (point-min) (point-max)))


;;;------------------------;;;
;;; Format Parameter Lists ;;;
;;;------------------------;;;


(defvar pls-format-paramlist-regexp
  (eval-when-compile
    (srx-word
     (srx-or
      "procedure" "function" "body" "package" "task" "entry" "accept"))))

(defun pls-format-paramlist ()
  "Re-formats a parameter-list.
ATTENTION:  1) Comments inside the list are killed !
            2) If the syntax is not correct (especially, if there are
               semicolons missing), it can get totally confused !
In such a case, use 'undo', correct the syntax and try again."

  (interactive)
  (let ((begin nil)
        (end nil)
        (delend nil)
        (paramlist nil))
    (unwind-protect
	(progn
	  (set-syntax-table pls-mode-symbol-syntax-table)

	  ;; check if really inside parameter list
	  (or (pls-in-paramlist-p)
	      (error "not in parameter list"))
	  ;;
	  ;; find start of current parameter-list
	  ;;
	  (pls-search-ignore-string-comment
	   pls-format-paramlist-regexp
	   t nil)
	  (pls-search-ignore-string-comment "(" nil nil t)
	  (backward-char 1)
	  (setq begin (point))

	  ;;
	  ;; find end of parameter-list
	  ;;
	  (forward-sexp 1)
	  (setq delend (point))
	  (delete-char -1)

	  ;;
	  ;; find end of last parameter-declaration
	  ;;
	  (pls-search-ignore-string-comment "[^ \t\n]" t nil t)
	  (forward-char 1)
	  (setq end (point))

	  ;;
	  ;; build a list of all elements of the parameter-list
	  ;;
	  (setq paramlist (pls-scan-paramlist (1+ begin) end))

	  ;;
	  ;; delete the original parameter-list
	  ;;
	  (delete-region begin (1- delend))

	  ;;
	  ;; insert the new parameter-list
	  ;;
	  (goto-char begin)
	  (pls-insert-paramlist paramlist))

      ;;
      ;; restore syntax-table
      ;;
      (set-syntax-table pls-mode-syntax-table)
      )))


(defun pls-scan-paramlist (begin end)
  ";; Scans a parameter-list  between BEGIN and END and returns a list
  ;; of its contents.
  ;; The list has the following format:
  ;;
  ;;   Name of Param  in? out? accept?  Name of Type   Default-Exp or nil
  ;;
  ;; ( ('Name_Param_1' t   nil    t      Type_Param_1   ':= expression')
  ;;   ('Name_Param_2' nil nil    t      Type_Param_2    nil) )"

 (let ((paramlist (list))
       (param (list))
       (notend t)
       (apos nil)
       (epos nil)
       (semipos nil)
       (match-cons nil))

    (goto-char begin)
    ;;
    ;; loop until end of last parameter
    ;;
    (while notend

      ;;
      ;; find first character of parameter-declaration
      ;;
      (pls-goto-next-non-ws)
      (setq apos (point))

      ;;
      ;; find last character of parameter-declaration
      ;;
      (if (setq match-cons
                (pls-search-ignore-string-comment "[ \t\n]*;" nil end t))
          (progn
            (setq epos (car match-cons))
            (setq semipos (cdr match-cons)))
        (setq epos end))

      ;;
      ;; read name(s) of parameter(s)
      ;;
      (goto-char apos)
      (looking-at
       (eval-when-compile
	 (concat "\\("
		      "[" pls-identifier-charset ", \t\n]*"
		 pls-identifier-char-regexp
		 "\\)[ \t\n]*:[^=]")))
      (setq param (list (buffer-substring (match-beginning 1)
                                          (match-end 1))))
      (pls-search-ignore-string-comment ":" nil epos t)

      ;;
      ;; look for 'in'
      ;;
      (setq apos (point))
      (setq param
            (append param
                    (list
                     (consp
                      (pls-search-ignore-string-comment "\\<in\\>"
                                                        nil
                                                        epos
                                                        t)))))

      ;;
      ;; look for 'out'
      ;;
      (goto-char apos)
      (setq param
            (append param
                    (list
                     (consp
                      (pls-search-ignore-string-comment "\\<out\\>"
                                                        nil
                                                        epos
                                                        t)))))

      ;;
      ;; look for 'accept'
      ;;
      (goto-char apos)
      (setq param
            (append param
                    (list
                     (consp
                      (pls-search-ignore-string-comment "\\<accept\\>"
                                                        nil
                                                        epos
                                                        t)))))

      ;;
      ;; skip 'in'/'out'/'accept'
      ;;
      (goto-char apos)
      (pls-goto-next-non-ws)
      (while (looking-at "\\<\\(in\\|out\\|accept\\)\\>")
        (forward-word 1)
        (pls-goto-next-non-ws))

      ;;
      ;; read type of parameter
      ;;
      (looking-at
       (eval-when-compile
	 (srx-word pls-name-regexp)))
      (setq param
            (append param
                    (list
                     (buffer-substring (match-beginning 0)
                                       (match-end 0)))))

      ;;
      ;; read default-expression, if there is one
      ;;
      (goto-char (setq apos (match-end 0)))
      (setq param
            (append param
                    (list
                     (if (setq match-cons
                               (pls-search-ignore-string-comment ":="
                                                                 nil
                                                                 epos
                                                                 t))
                         (buffer-substring (car match-cons)
                                           epos)
                       nil))))
      ;;
      ;; add this parameter-declaration to the list
      ;;
      (setq paramlist (append paramlist (list param)))

      ;;
      ;; check if it was the last parameter
      ;;
      (if (eq epos end)
          (setq notend nil)
        (goto-char semipos))

      ) ; end of loop

    (reverse paramlist)))


(defun pls-insert-paramlist (paramlist)
  ;; Inserts a formatted PARAMLIST in the buffer.
  ;; See doc of pls-scan-paramlist for the format.
  (let ((i (length paramlist))
        (parlen 0)
        (typlen 0)
        (temp 0)
        (inp nil)
        (outp nil)
        (acceptp nil)
        (column nil)
        (orgpoint 0)
        (firstcol nil))

    ;;
    ;; loop until last parameter
    ;;
    (while (not (zerop i))
      (setq i (1- i))

      ;;
      ;; get max length of parameter-name
      ;;
      (setq parlen
            (if (<= parlen (setq temp
                              (length (nth 0 (nth i paramlist)))))
                temp
              parlen))

      ;;
      ;; get max length of type-name
      ;;
      (setq typlen
            (if (<= typlen (setq temp
                              (length (nth 4 (nth i paramlist)))))
                temp
              typlen))

      ;;
      ;; is there any 'in' ?
      ;;
      (setq inp
            (or inp
                (nth 1 (nth i paramlist))))

      ;;
      ;; is there any 'out' ?
      ;;
      (setq outp
            (or outp
                (nth 2 (nth i paramlist))))

      ;;
      ;; is there any 'accept' ?
      ;;
      (setq acceptp
            (or acceptp
                (nth 3 (nth i paramlist))))) ; end of loop

    ;;
    ;; does paramlist already start on a separate line ?
    ;;
    (if (save-excursion
          (re-search-backward "^.\\|[^ \t]" nil t)
          (looking-at "^."))
        ;; yes => re-indent it
        (pls-indent-current)
      ;;
      ;; no => insert newline and indent it
      ;;
      (progn
        (pls-indent-current)
        (newline)
        (delete-horizontal-space)
        (setq orgpoint (point))
        (setq column (save-excursion
                       (funcall (pls-indent-function) orgpoint)))
        (indent-to column)
        ))

    (insert "(")

    (setq firstcol (current-column))
    (setq i (length paramlist))

    ;;
    ;; loop until last parameter
    ;;
    (while (not (zerop i))
      (setq i (1- i))
      (setq column firstcol)

      ;;
      ;; insert parameter-name, space and colon
      ;;
      (insert (nth 0 (nth i paramlist)))
      (indent-to (+ column parlen 1))
      (insert ": ")
      (setq column (current-column))

      ;;
      ;; insert 'in' or space
      ;;
      (if (nth 1 (nth i paramlist))
          (insert "in ")
        (if (and
             (or inp
                 acceptp)
             (not (nth 3 (nth i paramlist))))
            (insert "   ")))

      ;;
      ;; insert 'out' or space
      ;;
      (if (nth 2 (nth i paramlist))
          (insert "out ")
        (if (and
             (or outp
                 acceptp)
             (not (nth 3 (nth i paramlist))))
            (insert "    ")))

      ;;
      ;; insert 'accept'
      ;;
      (if (nth 3 (nth i paramlist))
          (insert "accept "))

      (setq column (current-column))

      ;;
      ;; insert type-name and, if necessary, space and default-expression
      ;;
      (insert (nth 4 (nth i paramlist)))
      (if (nth 5 (nth i paramlist))
          (progn
            (indent-to (+ column typlen 1))
            (insert (nth 5 (nth i paramlist)))))

      ;;
      ;; check if it was the last parameter
      ;;
      (if (not (zerop i))
          ;; no => insert ';' and newline and indent
          (progn
            (insert ";")
            (newline)
            (indent-to firstcol))
        ;; yes
        (insert ")"))

      ) ; end of loop

    ;;
    ;; if anything follows, except semicolon:
    ;; put it in a new line and indent it
    ;;
    (if (not (looking-at "[ \t]*[;\n]"))
        (pls-indent-newline-indent))

    ))


;;;----------------------------;;;
;;; Move To Matching Start/End ;;;
;;;----------------------------;;;

(defun pls-move-to-start ()
  "Moves point to the matching start of the current end ... around point."
  (interactive)
  (let ((pos (point)))
    (unwind-protect
	(progn
	  (set-syntax-table pls-mode-symbol-syntax-table)

	  (message "searching for block start ...")
	  (save-excursion
	    ;;
	    ;; do nothing if in string or comment or not on 'end ...;'
	    ;;            or if an error occurs during processing
	    ;;
	    (or
	     (pls-in-string-or-comment-p)
	     (and (progn
		    (or (looking-at "[ \t]*\\<end\\>")
			(backward-word 1))
		    (or (looking-at "[ \t]*\\<end\\>")
			(backward-word 1))
		    (or (looking-at "[ \t]*\\<end\\>")
			(error "not on end ...;")))
		  (pls-goto-matching-start 1)
		  (setq pos (point))

		  ;;
		  ;; on 'begin' => go on, according to user option
		  ;;
		  pls-move-to-declaration
		  (looking-at "\\<begin\\>")
		  (pls-goto-matching-decl-start)
		  (setq pos (point))))

	    ) ; end of save-excursion

	  ;; now really move to the found position
	  (goto-char pos)
	  (message "searching for block start ... done"))

      ;;
      ;; restore syntax-table
      ;;
      (set-syntax-table pls-mode-syntax-table))))


(defun pls-move-to-end ()
  "Moves point to the matching end of the current block around point.
Moves to 'begin' if in a declarative part."
  (interactive)
  (let ((pos (point))
        (decstart nil)
        (packdecl nil))
    (unwind-protect
	(progn
	  (set-syntax-table pls-mode-symbol-syntax-table)

	  (message "searching for block end ...")
	  (save-excursion

	    (forward-char 1)
	    (cond
	     ;; directly on 'begin'
	     ((save-excursion
		(pls-goto-previous-word)
		(looking-at "\\<begin\\>"))
	      (pls-goto-matching-end 1))
	     ;; on first line of defun declaration
	     ((save-excursion
		(and (pls-goto-stmt-start)
		     (looking-at "\\<function\\>\\|\\<procedure\\>" )))
	      (pls-search-ignore-string-comment "\\<begin\\>"))
	     ;; on first line of task declaration
	     ((save-excursion
		(and (pls-goto-stmt-start)
		     (looking-at "\\<task\\>" )
		     (forward-word 1)
		     (pls-search-ignore-string-comment "[^ \n\t]")
		     (not (backward-char 1))
		     (looking-at "\\<body\\>")))
	      (pls-search-ignore-string-comment "\\<begin\\>"))
	     ;; accept block start
	     ((save-excursion
		(and (pls-goto-stmt-start)
		     (looking-at "\\<accept\\>" )))
	      (pls-goto-matching-end 0))
	     ;; package start
	     ((save-excursion
		(and (pls-goto-matching-decl-start t)
		     (looking-at "\\<package\\>")))
	      (pls-goto-matching-end 1))
	     ;; inside a 'begin' ... 'end' block
	     ((save-excursion
		(pls-goto-matching-decl-start t))
	      (pls-search-ignore-string-comment "\\<begin\\>"))
	     ;; (hopefully ;-) everything else
	     (t
	      (pls-goto-matching-end 1)))
	    (setq pos (point))

	    ) ; end of save-excursion

	  ;; now really move to the found position
	  (goto-char pos)
	  (message "searching for block end ... done"))

      ;;
      ;; restore syntax-table
      ;;
      (set-syntax-table pls-mode-syntax-table))))


;;;-----------------------------;;;
;;;  Functions For Indentation  ;;;
;;;-----------------------------;;;

;; ---- main functions for indentation

(defun pls-indent-region (beg end)
  "Indents the region using pls-indent-current on each line."
  (interactive "*r")
  (goto-char beg)
  (let ((block-done 0)
	(lines-remaining (count-lines beg end))
	(msg (format "indenting %4d lines %%4d lines remaining ..."
		     (count-lines beg end)))
        (endmark (copy-marker end)))
    ;; catch errors while indenting
    (condition-case err
        (while (< (point) endmark)
          (if (> block-done 9)
              (progn (message (format msg lines-remaining))
                     (setq block-done 0)))
	  (if (looking-at "^$") nil
	    (pls-indent-current))
          (forward-line 1)
	  (setq block-done (1+ block-done))
	  (setq lines-remaining (1- lines-remaining)))
      ;; show line number where the error occured
      (error
       (error (format "line %d: %s"
                      (1+ (count-lines (point-min) (point)))
                      err) nil)))
    (message "indenting ... done")))


(defun pls-indent-newline-indent ()
  "Indents the current line, inserts a newline and then indents the new line."
  (interactive "*")
  (let ((column)
        (orgpoint))

    ;; horrible kludge
    (insert " ")
    (pls-adjust-case)
    ;; horrible dekludge
    (delete-backward-char 1)

    (pls-indent-current)
    (newline)
    (delete-horizontal-space)
    (setq orgpoint (point))

    (unwind-protect
	(progn
	  (set-syntax-table pls-mode-symbol-syntax-table)

	  (setq column (save-excursion
			 (funcall (pls-indent-function) orgpoint))))

      ;;
      ;; restore syntax-table
      ;;
      (set-syntax-table pls-mode-syntax-table))

    (indent-to column)

    ;; The following is needed to ensure that indentation will still be
    ;; correct if something follows behind point when typing LFD
    ;; For example: Imagine point to be there (*) when LFD is typed:
    ;;              while cond loop
    ;;                 null; *end loop;
    ;; Result without the following statement would be:
    ;;              while cond loop
    ;;                 null;
    ;;                *end loop;
    ;; You would then have to type TAB to correct it.
    ;; If that doesn't bother you, you can comment out the following
    ;; statement to speed up indentation a LITTLE bit.

    (if (not (looking-at "[ \t]*$"))
        (pls-indent-current))
    ))


(defun pls-indent-current ()
  "Indents current line as Pls code.
This works by two steps:
 1) It moves point to the end of the previous code-line.
    Then it calls the function to calculate the indentation for the
    following line as if a newline would be inserted there.
    The calculated column # is saved and the old position of point
    is restored.
 2) Then another function is called to calculate the indentation for
    the current line, based on the previously calculated column #."

  (interactive)

  (if (eq (pls-in-string-or-comment-p) 'a)
      nil
    (unwind-protect
	(progn
	  (set-syntax-table pls-mode-symbol-syntax-table)

	  (let ((line-end)
		(orgpoint (point-marker))
		(cur-indent)
		(prev-indent)
		(prevline t))

	    ;;
	    ;; first step
	    ;;
	    (save-excursion
	      (if (pls-goto-prev-nonblank-line t)
		  ;;
		  ;; we are not in the first accessible line in the buffer
		  ;;
		  (progn
		    ;;(end-of-line)
		    ;;(forward-char 1)
		    ;; we are already at the BOL
		    (forward-line 1)
		    (setq line-end (point))
		    (setq prev-indent
			  (save-excursion
			    (funcall (pls-indent-function) line-end))))
		(setq prevline nil)))

	    (if prevline
		;;
		;; we are not in the first accessible line in the buffer
		;;
		(progn
		  ;;
		  ;; second step
		  ;;
		  (back-to-indentation)
		  (setq cur-indent (pls-get-current-indent prev-indent))
		  (delete-horizontal-space)
		  (indent-to cur-indent)

		  ;;
		  ;; restore position of point
		  ;;
		  (goto-char orgpoint)
		  (if (< (current-column) (current-indentation))
		      (back-to-indentation))))))

      ;;
      ;; restore syntax-table
      ;;
      (set-syntax-table pls-mode-syntax-table))))

(defun pls-get-current-indent (prev-indent)
  ;; Returns the column # to indent the current line to.
  ;; PREV-INDENT is the indentation resulting from the previous lines.
  (let ((column nil)
        (pos nil)
        (match-cons nil))

    (cond
     ;;
     ;; in open parenthesis, but not in parameter-list
     ;;
     ((and
       pls-indent-to-open-paren
       (not (pls-in-paramlist-p))
       (setq column (pls-in-open-paren-p)))
      ;; check if we have something like this  (Table_Component_Type =>
      ;;                                          Source_File_Record,)
      (save-excursion
        (if (and (pls-search-ignore-string-comment "[^ \t]" t nil)
                 (looking-at "\n")
                 (pls-search-ignore-string-comment "[^ \t\n]" t nil)
                 (looking-at ">"))
            (setq column (+ pls-broken-indent column))))
      column)

     ;;
     ;; end
     ;;
     ((looking-at "\\<end\\>")
      (save-excursion
        (pls-goto-matching-start 1)

        ;;
        ;; found 'loop' => skip back to 'while' or 'for'
        ;;                 if 'loop' is not on a separate line
        ;;
        (if (and
             (looking-at "\\<loop\\>")
             (save-excursion
               (back-to-indentation)
               (not (looking-at "\\<loop\\>"))))
            (if (save-excursion
                  (and
                   (setq match-cons
                         (pls-search-ignore-string-comment
                          pls-loop-start-re t nil))
                   (not (looking-at "\\<loop\\>"))))
                (goto-char (car match-cons))))

        (current-indentation)))
     ;;
     ;; exception
     ;;
     ((looking-at "\\<exception\\>")
      (save-excursion
        (pls-goto-matching-start 1)
        (current-indentation)))
     ;;
     ;; when
     ;;
     ((looking-at "\\<when\\>")
      (save-excursion
        (pls-goto-matching-start 1)
        (+ (current-indentation) pls-when-indent)))
     ;;
     ;; else
     ;;
     ((looking-at "\\<else\\>")
      (if (save-excursion
            (pls-goto-previous-word)
            (looking-at "\\<or\\>"))
          prev-indent
        (save-excursion
          (pls-goto-matching-start 1 nil t)
          (current-indentation))))
     ;;
     ;; elsif
     ;;
     ((looking-at "\\<elsif\\>")
      (save-excursion
        (pls-goto-matching-start 1 nil t)
        (current-indentation)))
     ;;
     ;; then
     ;;
     ((looking-at "\\<then\\>")
      (if (save-excursion
            (pls-goto-previous-word)
            (looking-at "\\<and\\>"))
          prev-indent
        (save-excursion
          (pls-search-ignore-string-comment "\\<elsif\\>\\|\\<if\\>" t nil)
          (+ (current-indentation) pls-stmt-end-indent))))
     ;;
     ;; loop
     ;;
     ((looking-at "\\<loop\\>")
      (setq pos (point))
      (save-excursion
        (goto-char (match-end 0))
        (pls-goto-stmt-start)
        (if (looking-at "\\<loop\\>\\|\\<if\\>")
            prev-indent
          (progn
            (if (not (looking-at pls-loop-start-re))
                (pls-search-ignore-string-comment pls-loop-start-re
                                                  nil pos))
            (if (looking-at "\\<loop\\>")
                prev-indent
              (+ (current-indentation) pls-stmt-end-indent))))))
     ;;
     ;; begin
     ;;
     ((looking-at "\\<begin\\>")
      (save-excursion
        (if (pls-goto-matching-decl-start t)
            (current-indentation)
          (progn
            (message "no matching declaration start")
            prev-indent))))
     ;;
     ;; is
     ;;
     ((looking-at "\\<is\\>")
      (if (and
           pls-indent-is-separate
           (save-excursion
             (goto-char (match-end 0))
             (pls-goto-next-non-ws (save-excursion
                                     (end-of-line)
                                     (point)))
             (looking-at "\\<abstract\\>\\|\\<separate\\>")))
          (save-excursion
            (pls-goto-stmt-start)
            (+ (current-indentation) pls-indent))
        (save-excursion
          (pls-goto-stmt-start)
          (+ (current-indentation) pls-stmt-end-indent))))
     ;;
     ;; record
     ;;
     ((looking-at "\\<record\\>")
      (save-excursion
        (pls-search-ignore-string-comment
         "\\<\\(type\\|use\\)\\>" t nil)
        (if (looking-at "\\<use\\>")
            (pls-search-ignore-string-comment "\\<for\\>" t nil))
        (+ (current-indentation) pls-indent-record-rel-type)))
     ;;
     ;; or as statement-start
     ;;
     ((pls-looking-at-semi-or)
      (save-excursion
        (pls-goto-matching-start 1)
        (current-indentation)))
     ;;
     ;; private as statement-start
     ;;
     ((pls-looking-at-semi-private)
      (save-excursion
        (pls-goto-matching-decl-start)
        (current-indentation)))
     ;;
     ;; new/abstract/separate
     ;;
     ((looking-at "\\<\\(new\\|abstract\\|separate\\)\\>")
      (- prev-indent pls-indent (- pls-broken-indent)))
     ;;
     ;; return
     ;;
     ((looking-at "\\<return\\>")
      (save-excursion
        (forward-sexp -1)
        (if (and (looking-at "(")
                 (save-excursion
                   (backward-sexp 2)
                   (looking-at "\\<function\\>")))
            (1+ (current-column))
          prev-indent)))
     ;;
     ;; do
     ;;
     ((looking-at "\\<do\\>")
      (save-excursion
        (pls-goto-stmt-start)
        (+ (current-indentation) pls-stmt-end-indent)))
     ;;
     ;; package/function/procedure
     ;;
     ((and (looking-at "\\<\\(package\\|function\\|procedure\\)\\>")
           (save-excursion
             (forward-char 1)
             (pls-goto-stmt-start)
             (looking-at "\\<\\(package\\|function\\|procedure\\)\\>")))
      (save-excursion
        ;; look for 'generic'
        (if (and (pls-goto-matching-decl-start t)
                 (looking-at "generic"))
            (current-column)
          prev-indent)))
     ;;
     ;; label
     ;;
     ((looking-at
       (eval-when-compile
	 (concat "\\<" pls-identifier-regexp "[ \t\n]*:[^=]")))
      (if (pls-in-decl-p)
          prev-indent
        (+ prev-indent pls-label-indent)))
     ;;
     ;; identifier and other noindent-statements
     ;;
     ((looking-at
       (eval-when-compile
	 (concat "\\<" pls-identifier-regexp "[ \t\n]*")))
      prev-indent)
     ;;
     ;; beginning of a parameter list
     ;;
     ((looking-at "(")
      prev-indent)
     ;;
     ;; end of a parameter list
     ;;
     ((looking-at ")")
      (save-excursion
        (forward-char 1)
        (backward-sexp 1)
        (current-column)))
     ;;
     ;; comment
     ;;
     ((looking-at "--")
      (if pls-indent-comment-as-code
          prev-indent
        (current-indentation)))
     ;;
     ;; unknown syntax - maybe this should signal an error ?
     ;;
     (t
      prev-indent))))


(defun pls-indent-function (&optional nomove)
  ;; Returns the function to calculate the indentation for the current
  ;; line according to the previous statement, ignoring the contents
  ;; of the current line after point.  Moves point to the beginning of
  ;; the current statement, if NOMOVE is nil.

  (let ((orgpoint (point))
        (func nil)
        (stmt-start nil))
    ;;
    ;; inside a parameter-list
    ;;
    (if (pls-in-paramlist-p)
        (setq func 'pls-get-indent-paramlist)
      (progn
        ;;
        ;; move to beginning of current statement
        ;;
        (if (not nomove)
            (setq stmt-start (pls-goto-stmt-start)))
        ;;
        ;; no beginning found => don't change indentation
        ;;
        (if (and
             (eq orgpoint (point))
             (not nomove))
              (setq func 'pls-get-indent-nochange)

          (cond
           ;;
           ((and
             pls-indent-to-open-paren
             (pls-in-open-paren-p))
            (setq func 'pls-get-indent-open-paren))
           ;;
           ((looking-at "\\<end\\>")
            (setq func 'pls-get-indent-end))
           ;;
           ((looking-at pls-loop-start-re)
            (setq func 'pls-get-indent-loop))
           ;;
           ((looking-at pls-subprog-start-re)
            (setq func 'pls-get-indent-subprog))
           ;;
           ((looking-at "\\<package\\>")
            (setq func 'pls-get-indent-subprog)) ; maybe it needs a
                                                 ; special function
                                                 ; sometimes ?
           ;;
           ((looking-at pls-block-start-re)
            (setq func 'pls-get-indent-block-start))
           ;;
           ((looking-at "\\<type\\>")
            (setq func 'pls-get-indent-type))
           ;;
           ((looking-at "\\<\\(els\\)?if\\>")
            (setq func 'pls-get-indent-if))
           ;;
           ((looking-at "\\<case\\>")
            (setq func 'pls-get-indent-case))
           ;;
           ((looking-at "\\<when\\>")
            (setq func 'pls-get-indent-when))
           ;;
           ((looking-at "--")
            (setq func 'pls-get-indent-comment))
           ;;
           ((looking-at
	     (eval-when-compile
	       (concat pls-identifier-regexp "[ \t\n]*:[^=]")))
            (setq func 'pls-get-indent-label))
           ;;
	   ((looking-at "\\<separate\\>")
	    (setq func 'pls-get-indent-nochange))
           (t
            (setq func 'pls-get-indent-noindent))))))
    ;;;(message "%s %s %S %s" (looking-at pls-subprog-start-re) (point) func orgpoint)(sleep-for 2)

    func))


;; ---- functions to return indentation for special cases

(defun pls-get-indent-open-paren (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be behind an open paranthesis not yet closed.
  (pls-in-open-paren-p))


(defun pls-get-indent-nochange (orgpoint)
  ;; Returns the indentation (column #) of the current line.
  (save-excursion
    (forward-line -1)
    (current-indentation)))


(defun pls-get-indent-paramlist (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be inside a parameter-list.
  (save-excursion
    (pls-search-ignore-string-comment "[^ \t\n]" t nil t)
    (cond
     ;;
     ;; in front of the first parameter
     ;;
     ((looking-at "(")
      (goto-char (match-end 0))
      (current-column))
     ;;
     ;; in front of another parameter
     ;;
     ((looking-at ";")
      (goto-char (cdr (pls-search-ignore-string-comment "(\\|;" t nil t)))
      (pls-goto-next-non-ws)
      (current-column))
     ;;
     ;; inside a parameter declaration
     ;;
     (t
      (goto-char (cdr (pls-search-ignore-string-comment "(\\|;" t nil t)))
      (pls-goto-next-non-ws)
      (+ (current-column) pls-broken-indent)))))

 ;
;;; (looking-at   "(\\|;" )

(defun pls-get-indent-end (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of an end-statement.
  ;; Therefore it has to find the corresponding start. This can be a little
  ;; slow, if it has to search through big files with many nested blocks.
  ;; Signals an error if the corresponding block-start doesn't match.
  (let ((defun-name nil)
        (indent nil))
    ;;
    ;; is the line already terminated by ';' ?
    ;;
    (if (save-excursion
          (pls-search-ignore-string-comment ";" nil orgpoint))
        ;;
        ;; yes, look what's following 'end'
        ;;
        (progn
          (forward-word 1)
          (pls-goto-next-non-ws)
          (cond
           ;;
           ;; loop/select/if/case/record/select
           ;;
           ((looking-at "\\<\\(loop\\|if\\|case\\|record\\)\\>")
            (save-excursion
              (pls-check-matching-start
               (buffer-substring (match-beginning 0)
                                 (match-end 0)))
              (if (looking-at "\\<\\(loop\\|record\\)\\>")
                  (progn
                    (forward-word 1)
                    (pls-goto-stmt-start)))
              ;; a label ? => skip it
              (if (looking-at
		   (eval-when-compile
		     (concat pls-identifier-regexp "[ \n\t]+:")))
                  (progn
                    (goto-char (match-end 0))
                    (pls-goto-next-non-ws)))
              ;; really looking-at the right thing ?
              (or (looking-at
		   "\\<\\(loop\\|if\\|case\\|record\\|while\\|type\\)\\>")
                  (progn
                    (pls-search-ignore-string-comment
                     "\\<\\(loop\\|if\\|case\\|record\\|while\\|type\\)\\>"))
                  (backward-word 1))
              (current-indentation)))
           ;;
           ;; a named block end
           ;;
           ((looking-at pls-identifier-regexp)
            (setq defun-name (buffer-substring (match-beginning 0)
                                               (match-end 0)))
            (save-excursion
              (pls-goto-matching-start 0)
              (pls-check-defun-name defun-name)
              (current-indentation)))
           ;;
           ;; a block-end without name
           ;;
           ((looking-at ";")
            (save-excursion
              (pls-goto-matching-start 0)
              (if (looking-at "\\<begin\\>")
                  (progn
                    (setq indent (current-column))
                    (if (pls-goto-matching-decl-start t)
                        (current-indentation)
                      indent)))))
           ;;
           ;; anything else - should maybe signal an error ?
           ;;
           (t
            (+ (current-indentation) pls-broken-indent))))

      (+ (current-indentation) pls-broken-indent))))


(defun pls-get-indent-case (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of an case-statement.
  (let ((cur-indent (current-indentation))
        (match-cons nil)
        (opos (point)))
    (cond
     ;;
     ;; case..is..when..=>
     ;;
     ((save-excursion
       (setq match-cons (pls-search-ignore-string-comment
                         "[ \t\n]+=>" nil orgpoint)))
      (save-excursion
        (goto-char (car match-cons))
        (if (not (pls-search-ignore-string-comment "\\<when\\>" t opos))
            (error "missing 'when' between 'case' and '=>'"))
        (+ (current-indentation) pls-indent)))
     ;;
     ;; case..is..when
     ;;
     ((save-excursion
       (setq match-cons (pls-search-ignore-string-comment
                         "\\<when\\>" nil orgpoint)))
      (goto-char (cdr match-cons))
      (+ (current-indentation) pls-broken-indent))
     ;;
     ;; case..is
     ;;
     ((save-excursion
       (setq match-cons (pls-search-ignore-string-comment
                         "\\<is\\>" nil orgpoint)))
      (+ (current-indentation) pls-when-indent))
     ;;
     ;; incomplete case
     ;;
     (t
      (+ (current-indentation) pls-broken-indent)))))


(defun pls-get-indent-when (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of an when-statement.
  (let ((cur-indent (current-indentation)))
    (if (pls-search-ignore-string-comment
         "[ \t\n]+=>" nil orgpoint)
        (+ cur-indent  pls-indent)
      (+ cur-indent pls-broken-indent))))


(defun pls-get-indent-if (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of an if-statement.
  (let ((cur-indent (current-indentation))
        (match-cons nil))
    ;;
    ;; if..then ?
    ;;
    (if (pls-search-but-not
         "\\<then\\>" "\\<and\\>[ \t\n]+\\<then\\>" nil orgpoint)

        (progn
          ;;
          ;; 'then' first in separate line ?
          ;; => indent according to 'then'
          ;;
          (if (save-excursion
                (back-to-indentation)
                (looking-at "\\<then\\>"))
              (setq cur-indent (current-indentation)))
          (forward-word 1)
          ;;
          ;; something follows 'then' ?
          ;;
          (if (setq match-cons
                    (pls-search-ignore-string-comment
                     "[^ \t\n]" nil orgpoint))
              (progn
                (goto-char (car match-cons))
                (+ pls-indent
                   (- cur-indent (current-indentation))
                   (funcall (pls-indent-function t) orgpoint)))

            (+ cur-indent pls-indent)))

      (+ cur-indent pls-broken-indent))))


(defun pls-get-indent-block-start (orgpoint)
  ;; Returns the indentation (column #) for the new line after
  ;; ORGPOINT.  Assumes point to be at the beginning of a block start
  ;; keyword.
  (let ((cur-indent (current-indentation))
        (pos nil))
    (cond
     ((save-excursion
        (forward-word 1)
        (setq pos (car (pls-search-ignore-string-comment
                        "[^ \t\n]" nil orgpoint))))
      (goto-char pos)
      (save-excursion
        (funcall (pls-indent-function t) orgpoint)))
     ;;
     ;; nothing follows the block-start
     ;;
     (t
      (+ (current-indentation) pls-indent)))))


(defun pls-get-indent-subprog (orgpoint)
  ;;(message "orgpoint: %s %s " orgpoint (point)) (sleep-for 2)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of a subprog-/package-declaration.
  (let ((match-cons nil)
        (cur-indent (current-indentation))
        (foundis nil)
        (addind 0)
        (fstart (point)))
    ;;
    ;; is there an 'is' in front of point ?
    ;;
    (if (save-excursion
          (setq match-cons
                (pls-search-ignore-string-comment
                 "\\<is\\>\\|\\<do\\>" nil orgpoint)))
        ;;
        ;; yes, then skip to its end
        ;;
        (progn
          (setq foundis t)
          (goto-char (cdr match-cons)))
      ;;
      ;; no, then goto next non-ws, if there is one in front of point
      ;;
      (progn
        (if (pls-search-ignore-string-comment "[^ \t\n]" nil orgpoint)
            (pls-goto-next-non-ws)
          (goto-char orgpoint))))

    (cond
     ;;
     ;; nothing follows 'is'
     ;;
     ((and
       foundis
       (save-excursion
         (not (pls-search-ignore-string-comment
               "[^ \t\n]" nil orgpoint t))))
      (+ cur-indent pls-indent))
     ;;
     ;; is abstract/separate/new ...
     ;;
     ((and
       foundis
       (save-excursion
         (setq match-cons
               (pls-search-ignore-string-comment
                "\\<\\(separate\\|new\\|abstract\\)\\>"
                nil orgpoint))))
      (goto-char (car match-cons))
      (pls-search-ignore-string-comment (eval-when-compile
					  (concat pls-subprog-start-re
                                                "\\|\\<package\\>")) t)
      (pls-get-indent-noindent orgpoint))
     ;;
     ;; something follows 'is'
     ;;
     ((and
       foundis
       (save-excursion
         (pls-search-ignore-string-comment "[^ \t\n]" nil orgpoint))
       (pls-goto-next-non-ws)
      (funcall (pls-indent-function t) orgpoint)))
     ;;
     ;; no 'is' but ';'
     ;;
     ((save-excursion
        (pls-search-ignore-string-comment ";" nil orgpoint))
      cur-indent)
     ;;
     ;; no 'is' or ';'
     ;;
     (t
      (+ cur-indent pls-broken-indent)))))

(setq pls-statement-end-regexp  "[;/]")

(defun pls-get-indent-noindent (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of a 'noindent statement'.
  (if (save-excursion
        (pls-search-ignore-string-comment
	 pls-statement-end-regexp nil orgpoint))
      (current-indentation)
    (+ (current-indentation) pls-broken-indent)))


(defun pls-get-indent-label (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of a label or variable declaration.
  ;; Checks the context to decide if it's a label or a variable declaration.
  ;; This check might be a bit slow.
  (let ((match-cons nil)
        (cur-indent (current-indentation)))
    (goto-char (cdr (pls-search-ignore-string-comment ":")))
    (cond
     ;;
     ;; loop label
     ;;
     ((save-excursion
        (setq match-cons (pls-search-ignore-string-comment
                          pls-loop-start-re nil orgpoint)))
      (goto-char (car match-cons))
      (pls-get-indent-loop orgpoint))
     ;;
     ;; declare label
     ;;
     ((save-excursion
        (setq match-cons (pls-search-ignore-string-comment
                          "\\<declare\\>" nil orgpoint)))
      (save-excursion
        (goto-char (car match-cons))
        (+ (current-indentation) pls-indent)))
     ;;
     ;; complete statement following colon
     ;;
     ((save-excursion
        (pls-search-ignore-string-comment ";" nil orgpoint))
      (if (pls-in-decl-p)
          cur-indent                      ; variable-declaration
        (- cur-indent pls-label-indent))) ; label
     ;;
     ;; broken statement
     ;;
     ((save-excursion
        (pls-search-ignore-string-comment "[^ \t\n]" nil orgpoint))
      (if (pls-in-decl-p)
          (+ cur-indent pls-broken-indent)
        (+ cur-indent pls-broken-indent (- pls-label-indent))))
     ;;
     ;; nothing follows colon
     ;;
     (t
      (if (pls-in-decl-p)
          (+ cur-indent pls-broken-indent)   ; variable-declaration
        (- cur-indent pls-label-indent)))))) ; label


(defun pls-get-indent-loop (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of a loop statement
  ;; or (unfortunately) also a for ... use statement.
  (let ((match-cons nil)
        (pos (point)))
    (cond

     ;;
     ;; statement complete
     ;;
     ((save-excursion
        (pls-search-ignore-string-comment ";" nil orgpoint))
      (current-indentation))
     ;;
     ;; simple loop
     ;;
     ((looking-at "loop\\>")
      (pls-get-indent-block-start orgpoint))

     ;;
     ;; 'for'- loop (or also a for ... use statement)
     ;;
     ((looking-at "for\\>")
      (cond
       ;;
       ;; for ... use
       ;;
       ((save-excursion
          (and
           (goto-char (match-end 0))
           (pls-search-ignore-string-comment "[^ /n/t]" nil orgpoint)
           (not (backward-char 1))
           (not (zerop (skip-chars-forward
			(eval-when-compile
			  (concat pls-identifier-charset "%")))))
           (pls-search-ignore-string-comment "[^ /n/t]" nil orgpoint)
           (not (backward-char 1))
           (looking-at "\\<use\\>")
           ;;
           ;; check if there is a 'record' before point
           ;;
           (progn
             (setq match-cons (pls-search-ignore-string-comment
                               "\\<record\\>" nil orgpoint))
             t)))
        (if match-cons
            (goto-char (car match-cons)))
        (+ (current-indentation) pls-indent))
       ;;
       ;; for..loop
       ;;
       ((save-excursion
          (setq match-cons (pls-search-ignore-string-comment
                            "\\<loop\\>" nil orgpoint)))
        (goto-char (car match-cons))
        ;;
        ;; indent according to 'loop', if it's first in the line;
        ;; otherwise to 'for'
        ;;
        (if (not (save-excursion
                   (back-to-indentation)
                   (looking-at "\\<loop\\>")))
            (goto-char pos))
        (+ (current-indentation) pls-indent))
       ;;
       ;; for-statement is broken
       ;;
       (t
        (+ (current-indentation) pls-broken-indent))))

     ;;
     ;; 'while'-loop
     ;;
     ((looking-at "while\\>")
      ;;
      ;; while..loop ?
      ;;
      (if (save-excursion
            (setq match-cons (pls-search-ignore-string-comment
                              "\\<loop\\>" nil orgpoint)))

          (progn
            (goto-char (car match-cons))
            ;;
            ;; indent according to 'loop', if it's first in the line;
            ;; otherwise to 'while'.
            ;;
            (if (not (save-excursion
                       (back-to-indentation)
                       (looking-at "\\<loop\\>")))
                (goto-char pos))
            (+ (current-indentation) pls-indent))

        (+ (current-indentation) pls-broken-indent))))))


(defun pls-get-indent-type (orgpoint)
  ;; Returns the indentation (column #) for the new line after ORGPOINT.
  ;; Assumes point to be at the beginning of a type statement.
  (let ((match-dat nil))
    (cond
     ;;
     ;; complete record declaration
     ;;
     ((save-excursion
        (and
         (setq match-dat (pls-search-ignore-string-comment "\\<end\\>"
                                                           nil
                                                           orgpoint))
         (pls-goto-next-non-ws)
         (looking-at "\\<record\\>")
         (forward-word 1)
         (pls-goto-next-non-ws)
         (looking-at ";")))
      (goto-char (car match-dat))
      (current-indentation))
     ;;
     ;; record type
     ;;
     ((save-excursion
        (setq match-dat (pls-search-ignore-string-comment "\\<record\\>"
                                                          nil
                                                          orgpoint)))
      (goto-char (car match-dat))
      (+ (current-indentation) pls-indent))
     ;;
     ;; complete type declaration
     ;;
     ((save-excursion
        (pls-search-ignore-string-comment ";" nil orgpoint))
      (current-indentation))
     ;;
     ;; "type ... is", but not "type ... is ...", which is broken
     ;;
     ((save-excursion
	(and
	 (pls-search-ignore-string-comment "\\<is\\>" nil orgpoint)
	 (not (pls-search-ignore-string-comment "[^ \t\n]" nil orgpoint))))
      (+ (current-indentation) pls-indent))
     ;;
     ;; broken statement
     ;;
     (t
      (+ (current-indentation) pls-broken-indent)))))


;;; ---- support-functions for indentation

;;; ---- searching and matching

(defun pls-goto-stmt-start (&optional limit)
  ;; Moves point to the beginning of the statement that point is in or
  ;; after.  Returns the new position of point.  Beginnings are found
  ;; by searching for 'pls-end-stmt-re' and then moving to the
  ;; following non-ws that is not a comment.  LIMIT is actually not
  ;; used by the indentation functions.
  (let ((match-dat nil)
        (orgpoint (point)))

    (setq match-dat (pls-search-prev-end-stmt limit))
    (if match-dat
        ;;
        ;; found a previous end-statement => check if anything follows
        ;;
        (progn
          (if (not
               (save-excursion
                 (goto-char (cdr match-dat))
                 (pls-search-ignore-string-comment
                  "[^ \t\n]" nil orgpoint)))
              ;;
              ;; nothing follows => it's the end-statement directly in
              ;;                    front of point => search again
              ;;
              (setq match-dat (pls-search-prev-end-stmt limit)))
          ;;
          ;; if found the correct end-stetement => goto next non-ws
          ;;
          (if match-dat
              (goto-char (cdr match-dat)))
          (pls-goto-next-non-ws))

      ;;
      ;; no previous end-statement => we are at the beginning of the
      ;;                              accessible part of the buffer
      ;;
      (progn
        (goto-char (point-min))
        ;;
        ;; skip to the very first statement, if there is one
        ;;
        (if (setq match-dat
                  (pls-search-ignore-string-comment
                   "[^ \t\n]" nil orgpoint))
            (goto-char (car match-dat))
          (goto-char orgpoint))))


    (point)))


(defun pls-search-prev-end-stmt (&optional limit)
  ;; Moves point to previous end-statement.  Returns a cons cell whose
  ;; car is the beginning and whose cdr the end of the match.
  ;; End-statements are defined by 'pls-end-stmt-re'.  Checks for
  ;; certain keywords if they follow 'end', which means they are no
  ;; end-statement there.
  (let ((match-dat nil)
        (pos nil)
        (found nil))
    ;;
    ;; search until found or beginning-of-buffer
    ;;
    (while
        (and
         (not found)
         (setq match-dat (pls-search-ignore-string-comment pls-end-stmt-re
                                                           t
                                                           limit)))

      (goto-char (car match-dat))

      (if (not (pls-in-open-paren-p))
          ;;
          ;; check if there is an 'end' in front of the match
          ;;
          (if (not (and
                    (looking-at "\\<\\(record\\|loop\\)\\>")
                    (save-excursion
                      (pls-goto-previous-word)
                      (looking-at "\\<end\\>"))))
              (setq found t)

            (backward-word 1)))) ; end of loop

    (if found
        match-dat
      nil)))


(defun pls-goto-next-non-ws (&optional limit)
  ;; Skips whitespaces, newlines and comments to next non-ws
  ;; character.  Signals an error if there is no more such character
  ;; and limit is nil.
  (let ((match-cons nil))
    (setq match-cons (pls-search-ignore-string-comment
                      "[^ \t\n]" nil limit t))
    (if match-cons
        (goto-char (car match-cons))
      (if (not limit)
          (error "no more non-ws")
        nil))))


(defun pls-goto-stmt-end (&optional limit)
  ;; Moves point to the end of the statement that point is in or
  ;; before.  Returns the new position of point or nil if not found.
  (if (pls-search-ignore-string-comment pls-end-stmt-re nil limit)
      (point)
    nil))


(defun pls-goto-previous-word ()
  ;; Moves point to the beginning of the previous word of pls-code.
  ;; Returns the new position of point or nil if not found.
  (let ((match-cons nil)
        (orgpoint (point)))
    (if (setq match-cons
              (pls-search-ignore-string-comment "[^ \t\n]" t nil t))
        ;;
        ;; move to the beginning of the word found
        ;;
        (progn
          (goto-char (cdr match-cons))
          (skip-chars-backward
	   (eval-when-compile pls-identifier-charset))
	  (point))
      ;;
      ;; if not found, restore old position of point
      ;;
      (progn
        (goto-char orgpoint)
        'nil))))


(defun pls-check-matching-start (keyword)
  ;; Signals an error if matching block start is not KEYWORD.
  ;; Moves point to the matching block start.
  (pls-goto-matching-start 0)
  (if (not (looking-at (concat "\\<" keyword "\\>")))
      (error (concat
              "matching start is not '"
              keyword "'"))))


(defun pls-check-defun-name (defun-name)
  ;; Checks if the name of the matching defun really is DEFUN-NAME.
  ;; Assumes point to be already positioned by 'pls-goto-matching-start'.
  ;; Moves point to the beginning of the declaration.

  ;;
  ;; 'accept' or 'package' ?
  ;;
  (if (not (looking-at "\\<\\(accept\\|package\\|task\\|protected\\)\\>"))
      (pls-goto-matching-decl-start))
  ;;
  ;; 'begin' of 'procedure'/'function'/'task' or 'declare'
  ;;
  (save-excursion
    ;;
    ;; a named 'declare'-block ?
    ;;
    (if (looking-at "\\<declare\\>")
        (pls-goto-stmt-start)
      ;;
      ;; no, => 'procedure'/'function'/'task'/'protected'
      ;;
      (progn
        (forward-word 2)
        (backward-word 1)
        ;;
        ;; skip 'body' 'protected' 'type'
        ;;
        (if (looking-at "\\<\\(body\\|type\\)\\>")
            (forward-word 1))
        (forward-sexp 1)
        (backward-sexp 1)))
    ;;
    ;; should be looking-at the correct name
    ;;
    (if (not (looking-at (concat "\\<" defun-name "\\>")))
        (error
         (concat
          "matching defun has different name: "
          (buffer-substring
           (point)
           (progn
             (forward-sexp 1)
             (point))))))))


(defvar pls-goto-matching-decl-start-regexp
  (eval-when-compile
    (srx-word
     (srx-or
      "is" "separate" "end" "declare" "new" "begin" "generic"))))

(defun pls-goto-matching-decl-start (&optional noerror nogeneric)
  ;; Moves point to the matching declaration start of the current 'begin'.
  ;; If NOERROR is non-nil, it only returns nil if no match was found.
  (let ((nest-count 1)
        (pos nil)
        (first t)
        (flag nil))
    ;;
    ;; search backward for interesting keywords
    ;;
    (while (and
            (not (zerop nest-count))
            (pls-search-ignore-string-comment
	     pls-goto-matching-decl-start-regexp t))
      ;;
      ;; calculate nest-depth
      ;;
      (cond
       ;;
       ((looking-at "end")
        (pls-goto-matching-start 1 noerror)
        (if (looking-at "begin")
            (setq nest-count (1+ nest-count))))
       ;;
       ((looking-at "declare\\|generic")
        (setq nest-count (1- nest-count))
        (setq first nil))
       ;;
       ((looking-at "is")
        ;; check if it is only a type definition
        (if (save-excursion
              (pls-goto-previous-word)
              (skip-chars-backward
	       (eval-when-compile
		 (concat pls-name-charset "%")))
              (if (save-excursion
                    (backward-char 1)
                    (looking-at ")"))
                  (progn
                    (forward-char 1)
                    (backward-sexp 1)
                    (skip-chars-backward
		     (eval-when-compile
		       (concat pls-name-charset "%")))
                    ))
              (pls-goto-previous-word)
              (looking-at "\\<type\\>")) ; end of save-excursion
            (goto-char (match-beginning 0))
          (progn
            (setq nest-count (1- nest-count))
            (setq first nil))))

       ;;
       ((looking-at "new")
        (if (save-excursion
              (pls-goto-previous-word)
              (looking-at "is"))
            (goto-char (match-beginning 0))))
       ;;
       ((and first
             (looking-at "begin"))
        (setq nest-count 0)
        (setq flag t))
       ;;
       (t
        (setq nest-count (1+ nest-count))
        (setq first nil)))

      )  ;; end of loop

    ;; check if declaration-start is really found
    (if (not
         (and
          (zerop nest-count)
          (not flag)
          (progn
            (if (looking-at "is")
                  (pls-search-ignore-string-comment
                   pls-subprog-start-re t)
              (looking-at "declare\\|generic")))))
        (if noerror nil
          (error "no matching procedure/function/task/declare/package"))
      t)))

(setq pls-matching-start-regexp
      (eval-when-compile
	(srx-word
	 (srx-build-tree
	  "end"
	  "loop"
	  "begin"
	  "case"
	  "do"
	  "if"
	  "task"
	  "package"
	  "record"
	  "protected"))))

(defun pls-goto-matching-start (&optional nest-level noerror gotothen)
  ;; Moves point to the beginning of a block-start.  Which block
  ;; depends on the value of NEST-LEVEL, which defaults to zero.  If
  ;; NOERROR is non-nil, it only returns nil if no matching start was
  ;; found.  If GOTOTHEN is non-nil, point moves to the 'then'
  ;; following 'if'.
  (let ((nest-count (if nest-level nest-level 0))
        (found nil)
        (pos nil))

    ;;
    ;; search backward for interesting keywords
    ;;
    (while (and
            (not found)
            (pls-search-ignore-string-comment
	     pls-matching-start-regexp
	     t))

      ;;
      ;; calculate nest-depth
      ;;
      (cond
       ;; found block end => increase nest depth
       ((looking-at "end")
        (setq nest-count (1+ nest-count)))
       ;; found loop/select/record/case/if => check if it starts or
       ;; ends a block
       ((looking-at "loop\\|record\\|case\\|if")
        (setq pos (point))
        (save-excursion
          ;;
          ;; check if keyword follows 'end'
          ;;
          (pls-goto-previous-word)
          (if (looking-at "\\<end\\>")
              ;; it ends a block => increase nest depth
              (progn
                (setq nest-count (1+ nest-count))
                (setq pos (point)))
            ;; it starts a block => decrease nest depth
            (setq nest-count (1- nest-count))))
        (goto-char pos))
       ;; found package start => check if it really is a block
       ((looking-at "package")
        (save-excursion
          (pls-search-ignore-string-comment "\\<is\\>")
          (pls-goto-next-non-ws)
          ;; ignore it if it is only a declaration with 'new'
          (if (not (looking-at "\\<new\\>"))
              (setq nest-count (1- nest-count)))))
       ;; found task start => check if it has a body
       ((looking-at "task")
        (save-excursion
          (forward-word 1)
          (pls-goto-next-non-ws)
          ;; ignore it if it has no body
          (if (not (looking-at "\\<body\\>"))
              (setq nest-count (1- nest-count)))))
       ;; all the other block starts
       (t
        (setq nest-count (1- nest-count)))) ; end of 'cond'

      ;; match is found, if nest-depth is zero
      ;;
      (setq found (zerop nest-count))) ; end of loop

    (if found
        ;;
        ;; match found => is there anything else to do ?
        ;;
        (progn
          (cond
           ;;
           ;; found 'if' => skip to 'then', if it's on a separate line
           ;;                               and GOTOTHEN is non-nil
           ;;
           ((and
             gotothen
             (looking-at "if")
             (save-excursion
               (pls-search-ignore-string-comment "\\<then\\>" nil nil)
               (back-to-indentation)
               (looking-at "\\<then\\>")))
            (goto-char (match-beginning 0)))
           ;;
           ;; found 'do' => skip back to 'accept'
           ;;
           ((looking-at "do")
            (if (not (pls-search-ignore-string-comment "\\<accept\\>" t nil))
                (error "missing 'accept' in front of 'do'"))))
          (point))

      (if noerror
          nil
        (error "no matching start")))))


(defun pls-goto-matching-end (&optional nest-level noerror)
  ;; Moves point to the end of a block.  Which block depends on the
  ;; value of NEST-LEVEL, which defaults to zero.  If NOERROR is
  ;; non-nil, it only returns nil if found no matching start.
  (let ((nest-count (if nest-level nest-level 0))
        (found nil))

    ;;
    ;; search forward for interesting keywords
    ;;
    (while (and
            (not found)
            (pls-search-ignore-string-comment
	     pls-matching-start-regexp))
      ;;
      ;; calculate nest-depth
      ;;
      (backward-word 1)
      (cond
       ;; found block end => decrease nest depth
       ((looking-at "\\<end\\>")
        (setq nest-count (1- nest-count))
        ;; skip the following keyword
        (if (progn
              (skip-chars-forward "end")
              (pls-goto-next-non-ws)
              (looking-at "\\<\\(loop\\|record\\|case\\|if\\)\\>"))
            (forward-word 1)))
       ;; found package start => check if it really starts a block
       ((looking-at "\\<package\\>")
        (pls-search-ignore-string-comment "\\<is\\>")
        (pls-goto-next-non-ws)
        ;; ignore and skip it if it is only a 'new' package
        (if (not (looking-at "\\<new\\>"))
            (setq nest-count (1+ nest-count))
          (skip-chars-forward "new")))
       ;; all the other block starts
       (t
        (setq nest-count (1+ nest-count))
        (forward-word 1))) ; end of 'cond'

      ;; match is found, if nest-depth is zero
      ;;
      (setq found (zerop nest-count))) ; end of loop

    (if (not found)
        (if noerror
            nil
          (error "no matching end"))
      t)))


(defun pls-forward-sexp-ignore-comment ()
  ;; Skips one sexp forward, ignoring comments.
  (while (looking-at "[ \t\n]*--")
    (skip-chars-forward "[ \t\n]")
    (end-of-line))
  (forward-sexp 1))


(defun pls-search-ignore-string-comment
  (search-re &optional backward limit paramlists)
  ;; Regexp-Search for SEARCH-RE, ignoring comments, strings and
  ;; parameter lists, if PARAMLISTS is nil. Returns a cons cell of
  ;; begin and end of match data or nil, if not found.
  (let ((found nil)
        (begin nil)
        (end nil)
        (pos nil)
        (search-func
         (if backward 're-search-backward
           're-search-forward)))

    ;;
    ;; search until found or end-of-buffer
    ;;
    (while (and (not found)
                (funcall search-func search-re limit 1))
      (setq begin (match-beginning 0))
      (setq end (match-end 0))
      (let ((delimiter))

	(cond
	 ;;
	 ;; found in comment => skip it
	 ;;
	 ((pls-in-comment-p)
	  (if backward
	      (progn
		(re-search-backward "--" nil 1)
		(goto-char (match-beginning 0)))
	    (progn
	      (forward-line 1)
	      (beginning-of-line))))
	 ;;
	 ;; found in string => skip it
	 ;;
	 ((setq delimiter (pls-in-string-p))
	  (if backward
	      (progn
		(re-search-backward (make-string 1 delimiter) nil 1)
		(goto-char (match-beginning 0))))
	  (re-search-forward (make-string 1 delimiter) nil 1))
       ;;
       ;; found character constant => ignore it
       ;;
       ((save-excursion
          (setq pos (- (point) (if backward 1 2)))
          (and (char-after pos)
               (= (char-after pos) ?')
               (= (char-after (+ pos 2)) ?')))
        ())
       ;;
       ;; found a parameter-list but should ignore it => skip it
       ;;
       ((and (not paramlists)
             (pls-in-paramlist-p))
        (if backward
            (pls-search-ignore-string-comment "(" t nil t)))
       ;;
       ;; directly in front of a comment => skip it, if searching forward
       ;;
       ((save-excursion
          (goto-char begin)
          (looking-at "--"))
        (if (not backward)
            (progn
              (forward-line 1)
              (beginning-of-line))))
       ;;
       ;; found what we were looking for
       ;;
       (t
        (setq found t))))) ; end of loop

    (if found
        (cons begin end)
      nil)))


(defun pls-search-but-not (search-re not-search-re &optional backward limit)
  ;; Searches SEARCH-RE, ignoring parts of NOT-SEARCH-RE, strings,
  ;; comments and parameter-lists.
  (let ((begin nil)
        (end nil)
        (begin-not nil)
        (begin-end nil)
        (end-not nil)
        (ret-cons nil)
        (found nil))

    ;;
    ;; search until found or end-of-buffer
    ;;
    (while (and
            (not found)
            (save-excursion
              (setq ret-cons
                    (pls-search-ignore-string-comment search-re
                                                      backward limit))
              (if (consp ret-cons)
                  (progn
                    (setq begin (car ret-cons))
                    (setq end (cdr ret-cons))
                    t)
                nil)))

      (if (or
           ;;
           ;; if no NO-SEARCH-RE was found
           ;;
           (not
            (save-excursion
              (setq ret-cons
                    (pls-search-ignore-string-comment not-search-re
                                                      backward nil))
              (if (consp ret-cons)
                  (progn
                    (setq begin-not (car ret-cons))
                    (setq end-not (cdr ret-cons))
                    t)
                nil)))
           ;;
           ;;  or this NO-SEARCH-RE is not a part of the SEARCH-RE
           ;;  found before.
           ;;
           (or
            (<= end-not begin)
            (>= begin-not end)))

          (setq found t)

        ;;
        ;; not found the correct match => skip this match
        ;;
        (goto-char (if backward
                       begin
                     end)))) ; end of loop

    (if found
        (progn
          (goto-char begin)
          (cons begin end))
      nil)))


(defun pls-goto-prev-nonblank-line ( &optional ignore-comment)
  ;; Moves point to the beginning of previous non-blank line,
  ;; ignoring comments if IGNORE-COMMENT is non-nil.
  ;; It returns t if a matching line was found.
  (let ((notfound t)
        (newpoint nil))

    (save-excursion
      ;;
      ;; backward one line, if there is one
      ;;
      (if (zerop (forward-line -1))
          ;;
          ;; there is some kind of previous line
          ;;
          (progn
            (beginning-of-line)
            (setq newpoint (point))

            ;;
            ;; search until found or beginning-of-buffer
            ;;
            (while (and (setq notfound
                              (or (looking-at "[ \t]*$")
                                  (and (looking-at "[ \t]*--")
                                       ignore-comment)))
                        (not (pls-in-limit-line-p)))
              (forward-line -1)
              ;;(beginning-of-line)
              (setq newpoint (point))) ; end of loop

            )) ; end of if

      ) ; end of save-excursion

    (if notfound nil
      (progn
        (goto-char newpoint)
        t))))


(defun pls-goto-next-nonblank-line ( &optional ignore-comment)
  ;; Moves point to next non-blank line,
  ;; ignoring comments if IGNORE-COMMENT is non-nil.
  ;; It returns t if a matching line was found.
  (let ((notfound t)
        (newpoint nil))

    (save-excursion
    ;;
    ;; forward one line
    ;;
      (if (zerop (forward-line 1))
          ;;
          ;; there is some kind of previous line
          ;;
          (progn
            (beginning-of-line)
            (setq newpoint (point))

            ;;
            ;; search until found or end-of-buffer
            ;;
            (while (and (setq notfound
                              (or (looking-at "[ \t]*$")
                                  (and (looking-at "[ \t]*--")
                                       ignore-comment)))
                        (not (pls-in-limit-line-p)))
              (forward-line 1)
              (beginning-of-line)
              (setq newpoint (point))) ; end of loop

            )) ; end of if

      ) ; end of save-excursion

    (if notfound nil
      (progn
        (goto-char newpoint)
        t))))


;; ---- boolean functions for indentation

(defun pls-in-decl-p ()
  ;; Returns t if point is inside a declarative part.
  ;; Assumes point to be at the end of a statement.
  (or
   (pls-in-paramlist-p)
   (save-excursion
     (pls-goto-matching-decl-start t))))


(defun pls-looking-at-semi-or ()
  ;; Returns t if looking-at an 'or' following a semicolon.
  (save-excursion
    (and (looking-at "\\<or\\>")
         (progn
           (forward-word 1)
           (pls-goto-stmt-start)
           (looking-at "\\<or\\>")))))


(defun pls-looking-at-semi-private ()
  ;; Returns t if looking-at an 'private' following a semicolon.
  (save-excursion
    (and (looking-at "\\<private\\>")
         (progn
           (forward-word 1)
           (pls-goto-stmt-start)
           (looking-at "\\<private\\>")))))


;;; make a faster??? pls-in-limit-line-p not using count-lines
(defun pls-in-limit-line-p ()
  ;; return t if point is in first or last accessible line.
  (or (save-excursion (beginning-of-line) (= (point-min) (point)))
      (save-excursion (end-of-line) (= (point-max) (point)))))


(defun pls-in-comment-p-silly  ()
  ;; Returns t if inside a comment.
  (save-excursion (and (re-search-backward "\\(--\\|\n\\)" nil 1)
                       (looking-at "-"))))

(defun pls-in-comment-p ()
  ;; Returns t if inside a comment.
  (save-excursion
    (let* ((here (point))
	   (start (prog1 (progn
			   (beginning-of-defun) (point))
		    (goto-char here)))
	   (state (parse-partial-sexp start here)))
      (and (nth 4 state)
	   (if (nth 7 state) 'b 'a)))))

(defun pls-in-string-p ()
  ;; Returns t if point is inside a string
  (save-excursion
    (let ((here (point)))
      (nth 3 (parse-partial-sexp
	      (progn
		(beginning-of-line)
		(point))
	      here)))))


(defun pls-in-string-or-comment-p ()
  ;; Returns t if point is inside a string or a comment.
  (or (pls-in-comment-p)
      (pls-in-string-p)))

(setq pls-words-followed-by-paren
      (eval-when-compile
	(srx-word
	 (srx-build-tree

	  "procedure"
	  "function"
	  "body"
	  "package"
	  "task"
	  "entry"
	  "accept"))))


(defun pls-in-paramlist-p ()
  ;; Returns t if point is inside a parameter-list
  ;; following 'function'/'procedure'/'package'.
  (save-excursion
    (and
     (re-search-backward "(\\|)" nil t)
     ;; inside parentheses ?
     (looking-at "(")
     (backward-word 2)
     ;; right keyword before paranthesis ?
     (looking-at pls-words-followed-by-paren)
     (re-search-forward ")\\|:" nil t)
     ;; at least one ':' inside the parentheses ?
     (not (backward-char 1))
     (looking-at ":"))))


;; not really a boolean function ...
(defun pls-in-open-paren-p ()
  ;; If point is somewhere behind an open parenthesis not yet closed,
  ;; it returns the column # of the first non-ws behind this open
  ;; parenthesis, otherwise nil."

  (let ((start (if (< (point) pls-search-paren-char-count-limit)
                   1
                 (- (point) pls-search-paren-char-count-limit)))
        parse-result
        (col nil))
    (setq parse-result (parse-partial-sexp start (point)))
    (if (nth 1 parse-result)
        (save-excursion
          (goto-char (1+ (nth 1 parse-result)))
          (if (save-excursion
                (re-search-forward "[^ \t]" nil 1)
                (backward-char 1)
                (and
                 (not (looking-at "\n"))
                 (setq col (current-column))))
              col
            (current-column)))
      nil)))



;;;----------------------;;;
;;; Behaviour Of TAB Key ;;;
;;;----------------------;;;

(defun pls-tab ()
  "Do indenting or tabbing according to `pls-tab-policy'."
  (interactive)
;pm If not at the beginning of a line insert a tab
  (if (save-excursion 
	(skip-chars-backward " \t")
	(not (bolp)))
      (insert-tab)
    (cond ((eq pls-tab-policy 'indent-and-tab) (error "not implemented"))
	  ;; pls-indent-and-tab
	  ((eq pls-tab-policy 'indent-rigidly) (pls-tab-hard))
	  ((eq pls-tab-policy 'indent-auto) (pls-indent-current))
	  ((eq pls-tab-policy 'gei) (ada-tab-gei))
	  ((eq pls-tab-policy 'indent-af) (af-indent-line)) ; GEB
	  ((eq pls-tab-policy 'always-tab) (error "not implemented"))
	  )
    )
  )


(defun pls-untab (arg)
  "Delete leading indenting according to `pls-tab-policy'."
  (interactive "P")
  (cond ((eq pls-tab-policy 'indent-rigidly) (pls-untab-hard))
        ((eq pls-tab-policy 'indent-af) (backward-delete-char-untabify ; GEB
                                         (prefix-numeric-value arg) ; GEB
                                         arg)) ; GEB
        ((eq pls-tab-policy 'indent-auto) (error "not implemented"))
        ((eq pls-tab-policy 'always-tab) (error "not implemented"))
        ))

(defun pls-indent-current-function ()
  "Pls Mode version of the indent-line-function."
  (interactive "*")
  (let ((starting-point (point-marker)))
    (pls-beginning-of-line)
    (pls-tab)
    (if (< (point) starting-point)
        (goto-char starting-point))
    (set-marker starting-point nil)
    ))


(defun pls-tab-hard ()
  "Indent current line to next tab stop."
  (interactive)
  (save-excursion
    (beginning-of-line)
    (insert-char ?  pls-indent))
  (if (save-excursion (= (point) (progn (beginning-of-line) (point))))
      (forward-char pls-indent)))


(defun pls-untab-hard ()
  "indent current line to previous tab stop."
  (interactive)
  (let  ((bol (save-excursion (progn (beginning-of-line) (point))))
        (eol (save-excursion (progn (end-of-line) (point)))))
    (indent-rigidly bol eol  (- 0 pls-indent))))



;;;---------------;;;
;;; Miscellaneous ;;;
;;;---------------;;;

(defun pls-remove-trailing-spaces  ()
;; remove all trailing spaces at the end of lines.
 "remove trailing spaces in the whole buffer."
  (interactive)
  (save-excursion
    (goto-char (point-min))
    (while (re-search-forward "[ \t]+$" nil t)
      (replace-match "" nil nil))))


(defun pls-untabify-buffer ()
;; change all tabs to spaces
  (save-excursion
    (untabify (point-min) (point-max))))


(defun pls-uncomment-region (beg end)
  "delete comment-start at the beginning of a line in the region."
  (interactive "r")
  (comment-region beg end -1))


;; define a function to support find-file.el if loaded
(defun pls-ff-other-window ()
  "Find other file in other window using ff-find-other-file."
  (interactive)
  (and (fboundp 'ff-find-other-file)
       (ff-find-other-file t)))


;;;-------------------------------;;;
;;; Moving To Procedures/Packages ;;;
;;;-------------------------------;;;

(defun pls-next-procedure ()
  "Moves point to next procedure."
  (interactive)
  (end-of-line)
  (if (re-search-forward pls-procedure-start-regexp nil t)
      (goto-char (match-beginning 1))
    (error "No more functions/procedures/tasks")))

(defun pls-previous-procedure ()
  "Moves point to previous procedure."
  (interactive)
  (beginning-of-line)
  (if (re-search-backward pls-procedure-start-regexp nil t)
      (goto-char (match-beginning 1))
    (error "No more functions/procedures/tasks")))

(defun pls-next-package ()
  "Moves point to next package."
  (interactive)
  (end-of-line)
  (if (re-search-forward pls-package-start-regexp nil t)
      (goto-char (match-beginning 1))
    (error "No more packages")))

(defun pls-previous-package ()
  "Moves point to previous package."
  (interactive)
  (beginning-of-line)
  (if (re-search-backward pls-package-start-regexp nil t)
      (goto-char (match-beginning 1))
    (error "No more packages")))


;;;-----------------------
;;; define keymap for Pls
;;;-----------------------

(if (not pls-mode-map)
    (progn
      (setq pls-mode-map (make-sparse-keymap))

      ;; Indentation and Formatting
      (define-key pls-mode-map "\C-j"     'pls-indent-newline-indent)
      (define-key pls-mode-map "\r"     'pls-indent-newline-indent)
      (define-key pls-mode-map "\t"       'pls-tab)
      (define-key pls-mode-map "\C-c\C-l" 'pls-indent-region)
      (if (pls-xemacs)
	  (define-key pls-mode-map '(shift tab)    'pls-untab)
	(define-key pls-mode-map [S-tab]    'pls-untab))
      (define-key pls-mode-map "\C-c\C-f" 'pls-format-paramlist)
      (define-key pls-mode-map "\C-c\C-p" 'pls-pretty-print)
;;; We don't want to make meta-characters case-specific.
;;;   (define-key pls-mode-map "\M-Q"     'pls-fill-comment-paragraph-justify)
      (define-key pls-mode-map "\M-\C-q"  'pls-fill-comment-paragraph-postfix)

      ;; Movement
;;; It isn't good to redefine these.  What should be done instead?  -- rms.
;;;   (define-key pls-mode-map "\M-e"     'pls-next-package)
;;;   (define-key pls-mode-map "\M-a"     'pls-previous-package)
      (define-key pls-mode-map "\M-\C-e"  'pls-next-procedure)
      (define-key pls-mode-map "\M-\C-a"  'pls-previous-procedure)
      (define-key pls-mode-map "\C-c\C-a" 'pls-move-to-start)
      (define-key pls-mode-map "\C-c\C-e" 'pls-move-to-end)

      ;; Compilation
      (define-key pls-mode-map "\C-c\C-c" 'compile)

      ;; Casing
      (define-key pls-mode-map "\C-c\C-r" 'pls-adjust-case-region)
      (define-key pls-mode-map "\C-c\C-b" 'pls-adjust-case-buffer)

      (define-key pls-mode-map "\177"     'backward-delete-char-untabify)

      ;; Use predefined function of emacs19 for comments (RE)
      (define-key pls-mode-map "\C-c;"    'comment-region)
      (define-key pls-mode-map "\C-c:"    'pls-uncomment-region)

      ;; Change basic functionality

      ;; substitute-key-definition is not defined equally in GNU Emacs
      ;; and XEmacs, you cannot put in an optional 4th parameter in
      ;; XEmacs.  I don't think it's necessary, so I leave it out for
      ;; GNU Emacs as well.  If you encounter any problems with the
      ;; following three functions, please tell me. RE
      (mapcar (function (lambda (pair)
			  (substitute-key-definition (car pair) (cdr pair)
						     pls-mode-map)))
	      '((beginning-of-line      . pls-beginning-of-line)
		(end-of-line            . pls-end-of-line)
		(forward-to-indentation . pls-forward-to-indentation)
		))
      ;; else GNU Emacs
      ;;(mapcar (lambda (pair)
      ;;             (substitute-key-definition (car pair) (cdr pair)
      ;;				   pls-mode-map global-map))

      ))


;;;-------------------
;;; define menu 'Pls'
;;;-------------------

(require 'easymenu)

(defun pls-add-pls-menu ()
  "Adds the menu 'Pls' to the menu-bar in Pls Mode."
  (easy-menu-define pls-mode-menu pls-mode-map "Menu keymap for Pls mode."
                    '("PL/SQL"
                      ["Toggle automatic case" pls-toggle-auto-case
                       :style toggle :selected  pls-auto-case]
		      ("Choose identifier-case style"
		       ["Official  (as in Oracle SQL & PL/SQL user guide)"
			pls-set-case-style-official
			:style toggle :selected pls-case-style-official-p]
		       ["Classical (as in algol, pascal or ADA literature)"
			pls-set-case-style-classical
			:style toggle :selected pls-case-style-classical-p]
		       ["Contemporary (identifiers are capitalized)"
			pls-set-case-style-contemporary
			:style toggle :selected pls-case-style-contemporary-p]
		       ["Budget (everything lowercased)"
			pls-set-case-style-relaxed
			:style toggle :selected pls-case-style-relaxed-p]
		       )
                      ["------------------" nil nil]
		      ["Next Package" pls-next-package t]
                      ["Previous Package" pls-previous-package t]
                      ["Next Procedure" pls-next-procedure t]
                      ["Previous Procedure" pls-previous-procedure t]
                      ["Goto Start" pls-move-to-start t]
                      ["Goto End" pls-move-to-end t]
                      ["------------------" nil nil]
                      ["Find DIANA file" pls-find-sibling-file t]
                      ["------------------" nil nil]
                      ["Indent Current Line    (TAB)"
                       pls-indent-current-function t]
                      ["Indent Lines in Region" pls-indent-region t]
                      ["Indent Entire Buffer" pls-pretty-print t]
                      ["Format Parameter List" pls-format-paramlist t]
                      ["------------" nil nil]
                      ["Fill Comment Paragraph"
                       pls-fill-comment-paragraph t]
                      ["Justify Comment Paragraph"
                       pls-fill-comment-paragraph-justify t]
                      ["Postfix Comment Paragraph"
                       pls-fill-comment-paragraph-postfix t]
                      ["------------" nil nil]
                      ["Adjust Case Region" pls-adjust-case-region t]
                      ["Adjust Case Buffer" pls-adjust-case-buffer t]
                      ["----------" nil nil]
                      ["Comment   Region" comment-region t]
                      ["Uncomment Region" pls-uncomment-region t]
                      ["----------------" nil nil]
                      ["Compile" compile (fboundp 'compile)]
                      ["Next Error" next-error (fboundp 'next-error)]
                      ["---------------" nil nil]
                      ["Index" imenu (fboundp 'imenu)]
                      ["--------------" nil nil]
                      ["Other File Other Window" pls-ff-other-window
                       (fboundp 'ff-find-other-file)]
                      ["Other File" ff-find-other-file
                       (fboundp 'ff-find-other-file)]

		      ))
  (if (pls-xemacs) (progn
                     (easy-menu-add pls-mode-menu)
                     (setq mode-popup-menu (cons "Pls Mode" pls-mode-menu)))))

(defun pls-add-diana-menu ()
  "Adds the menu 'DIANA' to the menu-bar in DIANA Mode."
  (easy-menu-define diana-mode-menu pls-diana-mode-map "Menu keymap for Pls mode."
                    '("DIANA"
                      ["Up DIANA form" backward-up-list t]
                      ["Over DIANA form" forward-sexp t]
                      ["Over DIANA form backward" backward-sexp  t]
                      ["Beginning of current unit " beginning-of-defun t]
                      ["Select (highlight) next DIANA form " mark-sexp  t]
                      ["Find .pls file" pls-find-sibling-file t]
                      ["Go to label" pls-goto-diana-label t]))
  (if (pls-xemacs) (progn
                     (easy-menu-add diana-mode-menu)
                     (setq mode-popup-menu
			   (cons "DIANA Mode" diana-mode-menu)))))


(defun pls-find-sibling-file ()
  (interactive)
  (let* ((current-name  (buffer-file-name))
	 (len-1 (1- (length current-name)))
	 (last-char (aref current-name len-1)))
    (find-file
     (concat (substring current-name 0 len-1)
	     (if (eq last-char ?s) "d" "s")))))







;;;-------------------------------
;;; Define Some Support Functions
;;;-------------------------------

(defun pls-beginning-of-line (&optional arg)
  (interactive "P")
  (cond
   ((eq pls-tab-policy 'indent-af) (af-beginning-of-line arg))
   (t (beginning-of-line arg))
   ))

(defun pls-end-of-line (&optional arg)
  (interactive "P")
  (cond
   ((eq pls-tab-policy 'indent-af) (af-end-of-line arg))
   (t (end-of-line arg))
   ))

(defun pls-current-column ()
  (cond
   ((eq pls-tab-policy 'indent-af) (af-current-column))
   (t (current-column))
   ))

(defun pls-forward-to-indentation (&optional arg)
  (interactive "P")
  (cond
   ((eq pls-tab-policy 'indent-af) (af-forward-to-indentation arg))
   (t (forward-to-indentation arg))
   ))

;;;---------------------------------------------------
;;; support for find-file
;;;---------------------------------------------------


;;; functions for placing the cursor on the corresponding subprogram
(defun pls-which-function-are-we-in ()
  "Determine whether we are on a function definition/declaration and remember
the name of that function."

  (setq ff-function-name nil)

  (save-excursion
    (if (re-search-backward pls-procedure-start-regexp nil t)
	(setq ff-function-name (buffer-substring (match-beginning 0)
						 (match-end 0)))
      ; we didn't find a procedure start, perhaps there is a package
      (if (re-search-backward pls-package-start-regexp nil t)
	  (setq ff-function-name (buffer-substring (match-beginning 0)
						   (match-end 0)))
	))))


;;;---------------------------------------------------
;;; support for imenu
;;;---------------------------------------------------

(defun imenu-create-pls-index (&optional regexp)
  "create index alist for Pls files."
  (let ((index-alist '())
        prev-pos char)
    (goto-char (point-min))
    ;(imenu-progress-message prev-pos 0)
    ;; Search for functions/procedures
    (save-match-data
     (while (re-search-forward
             (or regexp pls-procedure-start-regexp)
             nil t)
       ;(imenu-progress-message prev-pos)
       ;; do not store forward definitions
       ;; right now we store them. We want to avoid them only in
       ;; package bodies, not in the specs!! ???RE???
       (save-match-data
            (setq index-alist (cons (imenu-example--name-and-position)
                        index-alist))
	)
       ;(imenu-progress-message 100)
       ))
    (nreverse index-alist)))


;;;---------------------------------------------------
;;; support for font-lock
;;;---------------------------------------------------

(defconst pls-font-lock-spec-definitive-entities
  (list
   (eval-when-compile
     (concat
      (srx-or "[^_$#]" "^");; to rule out x_type but not ^type  :-<
      (srx-word
       (apply
	(if font-lock-highlighting-of-Nth-regexp-broken-nesting
	    'srx-or
	  'srx-build-tree)
	pls-definitive-keywords))
      "[ \t]+"
      (srx-or "\"[^\"]+";; things like foo() foo; foo xx
	      "[^'(			;,\\[ \t]+"  ;; things like "foo bar" ()
	      )
      ))
   3
   'font-lock-function-name-face)
  "For consideration as a value of `pls-font-lock-keywords-spec'.
responsible for highlighting of user-defined entities.")

(defvar pls-font-lock-spec-highlighted-keywords-delimiter-regexp
  "[ \n\t,';()]"
  "when highlighting a keyword, what is the trailing delimiter")

(defconst pls-font-lock-spec-highlighted-keywords-max-decoration
  (list
   (eval-when-compile
     (concat
      (srx-or "[^_$#]" "^");; to rule out x_type but not ^type  :-<
      (srx-word
       (srx-build-regex
	pls-full-keyword-list
	4))
      pls-font-lock-spec-highlighted-keywords-delimiter-regexp))
   2
   'font-lock-keyword-face)
  "For consideration as a value of `pls-font-lock-keywords-spec'.
Responsible for highlighting of keywords when
 `font-lock-maximum-decoration' is t.")
;pm `font-lock-use-maximal-decoration' is t.")

(defconst pls-font-lock-spec-highlighted-keywords-min-decoration
  (list
   (eval-when-compile
     (concat
      (srx-or "[^_$#]" "^");; to rule out x_type but not ^type  :-<
      (srx-word
       (srx-build-regex
	pls-minimalists-keywords
	4))
      pls-font-lock-spec-highlighted-keywords-delimiter-regexp))
   2
   'font-lock-keyword-face)
  "For consideration as a value of `pls-font-lock-keywords-spec'.
Responsible for highlighting of keywords when
 `font-lock-maximum-decoration' is nil.")
;pm `font-lock-use-maximal-decoration' is nil.")

(defvar pls-font-lock-spec-highlighted-end
  (list
   (eval-when-compile
     (concat
      (srx-word (srx-build-tree "end"))
      "[ \t]+\\([^',(;\\[ \t]+\\)"))
   2 'font-lock-function-name-face)
  "Font-lock spec to highlighte constructs END FOO;")

(defvar pls-font-lock-double-quoted-identifier
  (list
   (eval-when-compile
     (concat
      "\""
      (srx-build-tree "[^\"]+")
      "\""))
   1 'font-lock-type-face))


(defvar pls-font-lock-keywords-spec-max-decoration
  (list
   pls-font-lock-spec-highlighted-keywords-max-decoration
   pls-font-lock-spec-definitive-entities
   pls-font-lock-spec-highlighted-end
   pls-font-lock-double-quoted-identifier
   ) "*Expressions to highlight in PLS mode., maximum decoration")

(defvar pls-font-lock-keywords-spec-min-decoration
  (list
   pls-font-lock-spec-highlighted-keywords-min-decoration
   pls-font-lock-spec-definitive-entities
   pls-font-lock-spec-highlighted-end
   pls-font-lock-double-quoted-identifier
   ) "*Expressions to highlight in PLS mode., minimum decoration")


;;; (ip pls-font-lock-keywords-spec)

(defun pls-gen-comment-until-proc ()
  ;; comment until spec of a procedure or a function.
  (forward-line 1)
  (set-mark-command (point))
  (if (re-search-forward pls-procedure-start-regexp nil t)
      (progn (goto-char (match-beginning 1))
             (comment-region (mark) (point)))
    (error "No more functions/procedures")))


(defun pls-gen-treat-proc (match)
  ;; make dummy body of a procedure/function specification.
  ;; MATCH is a cons cell containing the start and end location of the
  ;; last search for pls-procedure-start-regexp.
  (goto-char (car match))
  (let (proc-found func-found)
    (cond
     ((or (setq proc-found (looking-at "^[ \t]*procedure"))
	  (setq func-found (looking-at "^[ \t]*function")))
      ;; treat it as a proc/func
      (forward-word 2)
      (forward-word -1)
      (setq procname (buffer-substring (point) (cdr match))) ; store  proc name

    ;; goto end of procname
    (goto-char (cdr match))

    ;; skip over parameterlist
    (forward-sexp)
    ;; if function, skip over 'return' and result type.
    (if func-found
	(progn
	  (forward-word 1)
	  (skip-chars-forward " \t\n")
	  (setq functype
		(buffer-substring
		 (point)
		 (progn
		   (skip-chars-forward
		    (eval-when-compile pls-name-charset))
		   (point))))))
    ;; look for next non WS
    (cond
     ((looking-at "[ \t]*;")
      (delete-region (match-beginning 0) (match-end 0)) ;; delete the ';'
      (pls-indent-newline-indent)
      (insert " is")
      (pls-indent-newline-indent)
      (if func-found
	  (progn
	    (insert "Result : ")
	    (insert functype)
	    (insert ";")
	    (pls-indent-newline-indent)))
      (insert "begin -- ")
      (insert procname)
      (pls-indent-newline-indent)
      (insert "null;")
      (pls-indent-newline-indent)
      (if func-found
	  (progn
	    (insert "return Result;")
	    (pls-indent-newline-indent)))
      (insert "end ")
      (insert procname)
      (insert ";")
      (pls-indent-newline-indent)
      )
      ;; else
     ((looking-at "[ \t\n]*is")
      ;; do nothing
      )
     ((looking-at "[ \t\n]*rename")
      ;; do nothing
      )
     (t
      (message "unknown syntax")))
    ))))


(defun pls-make-body ()
  "Create an Pls package body in the current buffer.
The potential old buffer contents is deleted first, then we copy the
spec buffer in here and modify it to make it a body.

This function typically is to be hooked into `ff-file-created-hooks'."
  (interactive)
  (delete-region (point-min) (point-max))
  (insert-buffer (car (cdr (buffer-list))))
  (pls-mode)

  (let (found)
    (if (setq found
	      (pls-search-ignore-string-comment pls-package-start-regexp))
	(progn (goto-char (cdr found))
	       (insert " body")
	       ;; (forward-line -1)
	       ;;(comment-region (point-min) (point))
	       )
      (error "No package"))

    ;; (comment-until-proc)
    ;;   does not work correctly
    ;;   must be done by hand

    (while (setq found
		 (pls-search-ignore-string-comment pls-procedure-start-regexp))
      (pls-gen-treat-proc found))))

;;; (load-file "/nfs/dlsun42/sw1/pkgs.dld/solaris1.1/xemacs-19.13/lib/xemacs/site-lisp/pls-mode.elc")


;;; provide ourself

(provide 'pls-mode)


;;; (load-file "pls-mode.elc")
;;; pls-mode.el ends here

