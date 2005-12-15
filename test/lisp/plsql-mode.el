;;; Oracle PL/SQL editing support package.

;; Author: Karel Sprenger <ks@ic.uva.nl>
;; Maintainer: Karel Sprenger <ks@ic.uva.nl>
;; Created: 13 Nov 1995
;; Version: $Revision: 1.1 $
;; Keywords: PL/SQL major-mode

;; Copyright (C) 1995 Karel Sprenger

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Massachusettes Ave,
;; Cambridge, MA 02139, USA.

;; Installation:
;;   Add this line in your .emacs:
;;     (autoload 'plsql-mode "plsql-mode" "PL/SQL Mode" t)
;;   To invoke plsql-mode automatically on .sql files, do this:
;;     (setq auto-mode-alist (cons (cons "\\.sql$" 'plsql-mode) auto-mode-alist))
;;   To change the default plsql-indent value, and to turn on font-lock-mode, add:
;;     (add-hook 'plsql-mode-hook'(lambda ()
;;				    (progn (setq plsql-indent 3)
;;					   (turn-on-font-lock))))
;;

(eval-when-compile
  (load "make-regexp"))

(defvar plsql-mode-syntax-table nil
  "Syntax table in use in PL/SQL-mode buffers.")

(if plsql-mode-syntax-table
    ()
  (setq plsql-mode-syntax-table (make-syntax-table))
  ;C-style comments /**/ (see elisp manual "Syntax Flags")
  (modify-syntax-entry ?/ ". 14"   plsql-mode-syntax-table)
  (modify-syntax-entry ?* ". 12b"  plsql-mode-syntax-table)
  ;; double-dash starts comment
  (modify-syntax-entry ?-  ". 12b" plsql-mode-syntax-table)
  ;; newline and formfeed end comment
  (modify-syntax-entry ?\n "> b"   plsql-mode-syntax-table)
  (modify-syntax-entry ?\f "> b"   plsql-mode-syntax-table)
  ;; single quotes (') quotes delimit strings
  (modify-syntax-entry ?\' "\""    plsql-mode-syntax-table)
  ;; other stuff
  (modify-syntax-entry ?_  "w"     plsql-mode-syntax-table) ;word constituent
  (modify-syntax-entry ?\# "_" 	   plsql-mode-syntax-table) ;symbol constituent
  (modify-syntax-entry ?$  "_" 	   plsql-mode-syntax-table) ;symbol constituent
  (modify-syntax-entry ?\( "()"    plsql-mode-syntax-table) ;open-parenthesis
  (modify-syntax-entry ?\) ")("    plsql-mode-syntax-table) ;close-parenthesis
  (modify-syntax-entry ?%  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?+  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?=  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?&  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?|  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?<  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?>  "."     plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\[ "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\] "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\{ "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\} "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?.  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?@  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\\ "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?:  "." 	   plsql-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\; "." 	   plsql-mode-syntax-table) ;punctuation
  )

(defvar plsql-mode-map nil
  "Keymap used in PL/SQL mode.")

(let ((map (make-sparse-keymap)))
  (define-key map "\C-m" 	'plsql-newline)
  (define-key map "\C-?" 	'backward-delete-char-untabify)
  (define-key map "\C-i" 	'plsql-tab)
  (define-key map "\C-c\C-i" 	'plsql-untab)
  (define-key map "\C-c<" 	'plsql-backward-to-same-indent)
  (define-key map "\C-c>" 	'plsql-forward-to-same-indent)
  (define-key map "\C-ch" 	'plsql-header)
  (define-key map "\C-c(" 	'plsql-paired-parens)
  (define-key map "\C-c-" 	'plsql-inline-comment)
  (define-key map "\C-c\C-d" 	'plsql-declare)
  (define-key map "\C-c\C-e" 	'plsql-exception)
  (define-key map "\C-c\C-ps" 	'plsql-package-spec)
  (define-key map "\C-c\C-pb" 	'plsql-package-body)
  (define-key map "\C-c\C-pr" 	'plsql-procedure-spec)
  (define-key map "\C-c\C-f" 	'plsql-function-spec)
  (define-key map "\C-cb" 	'plsql-subprogram-body)
  (define-key map "\C-cf" 	'plsql-for-loop)
  (define-key map "\C-cl" 	'plsql-loop)
  (define-key map "\C-ci" 	'plsql-if)
  (define-key map "\C-cI" 	'plsql-elsif)
  (define-key map "\C-ce" 	'plsql-else)
  (define-key map "\C-cr" 	'plsql-record)
  (define-key map "\C-ct" 	'plsql-table)
  (define-key map "\C-c\C-t" 	'plsql-tabsize)
  (define-key map "\C-cw" 	'plsql-while-loop)
  (define-key map "\C-c\C-w" 	'plsql-when)
  (define-key map "\C-cx" 	'plsql-exit)
  (setq plsql-mode-map map))


(defvar plsql-font-lock-keywords nil
  "Additional expressions to highlight in PL/SQL-Mode.")

(let ((types-1				;SQL Types
       (eval-when-compile
	 (make-regexp '("char" "character" 
			"date" "dec" "decimal" "double[ \\n\\t]+precision" 
			"float" 
			"int" "integer" 
			"long" 
			"mlslabel" 
			"number" "raw" "real" "rowid" 
			"smallint" 
			"varchar" "varchar2"))))
      (types-2				;PL/SQL Additional Types
       (eval-when-compile
	 (make-regexp '("binary_integer" "boolean" 
			"cursor"))))
      (types-3				;PL/SQL Column and Record Type Indicators
       (eval-when-compile
	 (make-regexp '("%type" 
		       "%rowtype"))))
      (functions-1			;Single Row Number Functions
       (eval-when-compile
	 (make-regexp '("abs" 
			"ceil" "cos" "cosh" 
			"exp" 
			"floor" 
			"ln" "log" 
			"mod" 
			"power"
			"round" 
			"sign" "sin" "sinh" "sqrt" 
			"tan" "tanh" "trunc"))))
      (functions-2			;Single Row Character Functions Returning Char
       (eval-when-compile
	 (make-regexp '("chr" "concat" 
			"initcap" 
			"lower" "lpad" "ltrim" 
			"nls_initcap" "nls_lower" "nls_upper" 
			"replace" "rpad" "rtrim" 
			"soundex" "substr" "substrb" 
			"translate" 
			"upper"))))
      (functions-3			;Single Row Character Functions Returning Number
       (eval-when-compile
	 (make-regexp '("ascii" 
			"instr" "instrb" 
			"length" "lengthb" 
			"nlssort"))))
      (functions-4			;Single Row Date Functions
       (eval-when-compile
	 (make-regexp '("add_months" 
			"last_day" 
			"months_between" 
			"new_time" "next_day"))))
      (functions-5			;Other Single Row Functions (no parens needed!)
       (eval-when-compile
	 (make-regexp '("currval" 
			"nextval" 
			"sqlcode" "sqlerrm" "sysdate" 
			"uid" "user"))))
      (functions-6			;Single Row Conversion Functions
       (eval-when-compile
	 (make-regexp '("chartorowid" "convert" 
			"hextoraw" 
			"rawtohex" "rowidtochar" 
			"to_char" "to_date" "to_label" "to_multi_byte" "to_number" 
			"to_single_byte"))))
      (functions-7			;Other Single Row Functions (parens needed) 
       (eval-when-compile
	 (make-regexp '("decode" "dump" 
			"greatest" "greatest_lb" 
			"least" "least_ub" 
			"nvl" 
			"userenv" 
			"vsize"))))
      (functions-8			;Group Functions
       (eval-when-compile
	 (make-regexp '("avg" 
			"count" 
			"glb" 
			"lub" 
			"max" "min" 
			"stddev" "sum" 
			"variance"))))
      (functions-9			;PL/SQL Pragma Clause and Function
       (eval-when-compile
	 (make-regexp '("exception_init" 
			"raise_application_error"))))
      (cmndwords-1			;These words start SQL commands
       (eval-when-compile
	 (make-regexp '("alter" "analyze" "audit" 
			"comment" "commit" "create" 
			"delete" "drop" 
			"explain\\([ \\t]+plan\\)?" 
			"grant" 
			"insert" 
			"lock\\([ \\t]+table\\)?" 
			"noaudit" 
			"rename" "revoke" "rollback\\([ \\t]+segment\\)?" 
			"savepoint" "select" "set" 
			"truncate" 
			"update"))))
      (cmndwords-2			;These words appear in SQL command names
       (eval-when-compile
	 (make-regexp '("cluster" "column" "controlfile" 
			"database\\([ \\t]+link\\)?" 
			"function" 
			"index" 
			"package\\([ \\t]+body\\)?" "procedure" "profile" 
			"resource cost" "role" 
			"schema" "sequence" "session" "snapshot\\([ \\t]+log\\)?" 
			"synonym" "system" 
			"table" "tablespace" "transaction" "trigger" 
			"user" 
			"view"))))
      (exceptions			;Predefined Exceptions + OTHERS
       (eval-when-compile
	 (make-regexp '("cursor_already_open" 
			"dup_val_on_index" 
			"invalid_cursor"
			"invalid_number" 
			"login_denied" 
			"no_data_found" 
			"not_logged_on" 
			"program_error" 
			"storage_error"
			"timeout_on_resource" 
			"too_many_rows" 
			"transaction_backed_out" 
			"value_error" 
			"zero_divide"
			"others"))))
      (operators			;SQL Set Operators
       (eval-when-compile
	 (make-regexp '("and" 
			"in" "intersect" 
			"like" 
			"minus" 
			"not" 
			"or" 
			"union\\([ \\n\\t]+all\\)?"))))
      (keywords-1			;SQL Keywords (most, not all :-)
       (eval-when-compile
	 (make-regexp '("add" "admin" "after" "all" "allocate\\([ \\n\\t]+extent\\)?" 
			"any" "\\(no\\)?archivelog" "as" "asc" 
			"backup" "before" "between" "by" 
			"\\(no\\)?cache" "cancel" "cascade" "change" "check" 
			"checkpoint" "connect[ \\n\\t]+by" "constraints?" "convert" 
			"\\(no\\)?cycle" 
			"datafiles?" "default" "desc" "distinct" "drop" 
			"exclusive" "execute" "externally" 
			"false" "for[ \\n\\t]+update\\(\\n\\t]+of\\)?" "foreign" 
			"from" 
			"group[ \\n\\t]+by" 
			"having"
			"identified" "increment" "instance" "is" 
			"key" 
			"limit" "logfile\\([ \\n\\t]+member\\)?"
			"maxtrans" "\\(no\\)?maxvalue" "\\(no\\)?minvalue" 
			"modify" "mount" 
			"nowait" "null" 
			"on" "option" "\\(no\\)?order" "order[ \\n\\t]+by"
			"primary" "private" "public"
			"recover" "references" "replace" "reuse" 
			"start[ \\n\\t]+with" "size" "storage" 
			"to" "true" 
			"unique" "unlimited" "using" 
			"values" "where" "with"))))
      (keywords-2			;PL/SQL Keywords (hopefully all)
       (eval-when-compile
	 (make-regexp '("begin" "close" "current" "declare" "deleting" "each" "else" 
			"elsif" "end" "exception" "exit\\([ \\n\\t]+when\\)?" "fetch" 
			"for" "goto" "if" "in" "inserting" "into" "is" "loop" "of" 
			"open" "out" "pragma" "raise" "record" "return" "reverse" 
			"row" "then" "type" "updating" "when" "while"))))
      )
  (setq plsql-font-lock-keywords
	(list
	 (list "'\\([^']*\\)'" 1 'font-lock-string-face)
	 (list (concat "\\b\\(" types-1     "\\)\\b") 
	       1 'font-lock-type-face)
	 (list (concat "\\b\\(" types-2     "\\)\\b") 
	       1 'font-lock-type-face)
	 (list (concat "\\w\\(" types-3     "\\)\\b")
	       1 'font-lock-type-face)
	 (list (concat "\\b\\(" functions-1 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-2 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-3 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-4 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-5 "\\)[ \n\t;]") ;No parentheses!
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-6 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-7 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-8 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-9 "\\)[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" cmndwords-1 "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" cmndwords-2 "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" exceptions  "\\)\\b")
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" operators   "\\)\\b") 
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" keywords-1  "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" keywords-2  "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list "\\b\\(function\\|procedure\\|package\\)[ \t]+\\([^ \t(]+\\)"
	       2 'font-lock-function-name-face)
	 (list "\\(--+.*\\)" 1 'font-lock-comment-face t)
	 (list "\\(/\\*.*\\*/\\)" 1 'font-lock-comment-face t)
	 (list "^\\(rem\\([ \t].*\\)?\\)$" 1 'font-lock-comment-face t))))

(defvar plsql-indent 4 "*Value is the number of columns to indent in PL/SQL-Mode.")
  
(defun plsql-mode ()
"This is a mode intended to support program development in PL/SQL.
Most control constructs and declarations of PL/SQL can be inserted in the buffer
by typing Control-C followed by a character mnemonic for the construct.

C-c C-d   declare
C-c C-e   exception      
C-c C-ps  package spec   C-c C-pb package body
C-c C-pr  procedure spec C-c b    proc/func body
C-c C-f   func spec      C-c f    for loop
                         C-c i    if
                         C-c I    elsif
                         C-c e    else
                         C-c l    loop
C-c r     record type    C-c t    table type
                         C-c C-t  tab spacing for indents
C-c C-w   when           C-c w    while loop
                         C-c x    exit
C-c (    paired parens   C-c -    inline comment
                         C-c h    header sec

C-c < and C-c > move backward and forward respectively to the next line
having the same (or lesser) level of indentation.

Variable plsql-indent controls the number of spaces for indent/undent.

\\{plsql-mode-map}
"
  (interactive)
  (kill-all-local-variables)
  (use-local-map plsql-mode-map)
  (setq major-mode 'plsql-mode)
  (setq mode-name "PL/SQL")
  (make-local-variable 'comment-column)
  (setq comment-column 41)
  (make-local-variable 'end-comment-column)
  (setq end-comment-column 72)
  (set-syntax-table plsql-mode-syntax-table)
  (make-local-variable 'paragraph-start)
  (setq paragraph-start (concat "^$\\|" page-delimiter))
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'paragraph-ignore-fill-prefix)
  (setq paragraph-ignore-fill-prefix t)
;  (make-local-variable 'indent-line-function)
;  (setq indent-line-function 'c-indent-line)
  (make-local-variable 'require-final-newline)
  (setq require-final-newline t)
  (make-local-variable 'comment-start)
  (setq comment-start "--")
  (make-local-variable 'comment-end)
  (setq comment-end "")
  (make-local-variable 'comment-column)
  (setq comment-column 41)
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "--+ *")
  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)
  (setq case-fold-search t)		;Automatically buffer-local
  (cond	((string-match "Win-Emacs" (emacs-version)) ; Win-Emacs
	 (make-local-variable 'font-lock-keywords-case-fold-search)
	 (make-local-variable 'font-lock-keywords)
	 (setq font-lock-keywords-case-fold-search t)
	 (setq font-lock-keywords plsql-font-lock-keywords))
	;;
        ((string-match "XEmacs\\|Lucid" (emacs-version)) ; XEmacs/Lucid
	 (put major-mode 'font-lock-keywords-case-fold-search t))
	;; XEmacs (19.13, at least) guesses the rest correctly.
	;; If any older XEmacsen don't, then tell me.
	;;
	((string-lessp "19.28.89" emacs-version) ; Emacs 19.29 and later
	 (make-local-variable 'font-lock-defaults)
	 (setq font-lock-defaults 
	       '(plsql-font-lock-keywords t t nil beginning-of-line
		 (font-lock-comment-start-regexp . "--\\|/\*"))))
	;;
	(t ; Emacs 19.28 and older
	 (make-local-variable 'font-lock-keywords-case-fold-search)
	 (make-local-variable 'font-lock-keywords)
	 (make-local-variable 'font-lock-no-comments)
	 (setq font-lock-keywords-case-fold-search t)
	 (setq font-lock-keywords plsql-font-lock-keywords)
	 (setq font-lock-no-comments t)))
  (run-hooks 'plsql-mode-hook))

(defun plsql-tabsize (s)
  "changes spacing used for indentation. Reads spacing from minibuffer."
  (interactive "nnew indentation spacing: ")
  (setq plsql-indent s))

(defun plsql-newline ()
  "Start new line and indent to current tab stop."
  (interactive)
  (let ((plsql-cc (current-indentation)))
    (newline)
    (indent-to plsql-cc)))

(defun plsql-tab ()
  "Indent to next tab stop."
  (interactive)
  (indent-to (* (1+ (/ (current-indentation) plsql-indent)) plsql-indent)))

(defun plsql-untab ()
  "Delete backwards to previous tab stop."
  (interactive)
  (backward-delete-char-untabify plsql-indent nil))

(defun plsql-go-to-this-indent (step indent-level)
  "Move point repeatedly by <step> lines till the current line
has given indent-level or less, or the start/end of the buffer is hit.
Ignore blank lines, statement labels, block/loop names."
  (while (and
	  (zerop (forward-line step))
	  (or (looking-at "^[ 	]*$")
	      (looking-at "^[ 	]*--")
	      (looking-at "^<<[A-Za-z0-9_]+>>")
	      (looking-at "^[A-Za-z0-9_]+:")
	      (> (current-indentation) indent-level)))
    nil))

(defun plsql-backward-to-same-indent ()
  "Move point backwards to nearest line with same indentation or less.
If not found, point is left at top of buffer."
  (interactive)
  (plsql-go-to-this-indent -1 (current-indentation))
  (back-to-indentation))

(defun plsql-forward-to-same-indent ()
  "Move point forwards to nearest line with same indentation or less.
If not found, point is left at start of last line in buffer."
  (interactive)
  (plsql-go-to-this-indent 1 (current-indentation))
  (back-to-indentation))

(defun plsql-table ()
  "Insert PL/SQL table type definition, prompting for type name and component type,
then insert the INDEX BY clause."
  (interactive)
  (insert "type ")
  (insert (read-string "type name: "))
  (insert " is table of ")
  (insert (read-string "component type: "))
  (plsql-newline)
  (plsql-tab)
  (insert "index by binary_integer;")
  (plsql-newline))

(defun plsql-declare ()
  "Start a declare part and indent for the 1st declaration."
  (interactive)
  (insert "declare")
  (plsql-newline)
  (plsql-newline)
  (end-of-line -1)
  (plsql-tab))

(defun plsql-exception ()
  "Undent and insert an exception part into a block. Reindent."
  (interactive)
  (plsql-untab)
  (insert "exception")
  (plsql-newline)
  (plsql-tab))

(defun plsql-else ()
  "Add an else clause inside an if-then-end-if clause."
  (interactive)
  (plsql-untab)
  (insert "else")
  (plsql-newline)
  (plsql-tab))

(defun plsql-exit ()
  "Insert an exit statement, prompting for loop name and condition."
  (interactive)
  (insert "exit")
  (let ((plsql-loop-name (read-string "[name of loop to exit]: ")))
    (if (not (string-equal plsql-loop-name "")) (insert " " plsql-loop-name)))
  (let ((plsql-exit-condition (read-string "[exit condition]: ")))
    (if (not (string-equal plsql-exit-condition ""))
	(if (string-match "^ *[Ww][Hh][Ee][Nn] +" plsql-exit-condition)
	    (insert " " plsql-exit-condition)
	  (insert " when " plsql-exit-condition))))
  (insert ";"))

(defun plsql-when ()
  "Start an excption handler with a when clause."
  (interactive)
  (insert "when ")
  (insert (read-string "exception: "))
  (insert " then")
  (plsql-newline)
  (plsql-tab))

(defun plsql-for-loop ()
  "Build a skeleton for-loop statement, prompting for the loop parameters."
  (interactive)
  (insert "for ")
  (let* ((plsql-loop-name (read-string "[loop name]: "))
	 (plsql-loop-is-named (not (string-equal plsql-loop-name ""))))
    (if plsql-loop-is-named
	(progn
	  (beginning-of-line)
	  (open-line 1)
	  (insert "<<" plsql-loop-name ">>")
	  (next-line 1)
	  (end-of-line 1)))
    (insert (read-string "loop variable: ") " in ")
    (insert (read-string "range: ") " loop")
    (plsql-newline)
    (plsql-newline)
    (insert "end loop")
    (if plsql-loop-is-named (insert " " plsql-loop-name))
    (insert ";"))
  (end-of-line 0)
  (plsql-tab))

(defun plsql-header ()
  "Insert a comment block containing the module title, author, etc."
  (interactive)
  (insert "--\n--  Title: \t")
  (insert (read-string "Title: "))
  (insert "\n--  Created:\t" (current-time-string))
  (insert "\n--  Author: \t" (user-full-name))
  (insert "\n--\t\t<" (user-login-name) "@" (system-name) ">\n--\n"))

(defun plsql-if ()
  "Insert skeleton if statment, prompting for a boolean-expression."
  (interactive)
  (insert "if ")
  (insert (read-string "condition: ") " then")
  (plsql-newline)
  (plsql-newline)
  (insert "end if;")
  (end-of-line 0)
  (plsql-tab))

(defun plsql-elsif ()
  "Add an elsif clause to an if statement, prompting for the boolean-expression."
  (interactive)
  (plsql-untab)
  (insert "elsif ")
  (insert (read-string "condition: ") " then")
  (plsql-newline)
  (plsql-tab))

(defun plsql-loop ()
  "insert a skeleton loop statement.  exit statement added by hand."
  (interactive)
  (insert "loop ")
  (let* ((plsql-loop-name (read-string "[loop name]: "))
	 (plsql-loop-is-named (not (string-equal plsql-loop-name ""))))
    (if plsql-loop-is-named
	(progn
	  (beginning-of-line)
	  (open-line 1)
	  (insert "<<" plsql-loop-name ">>")
	  (forward-line 1)
	  (end-of-line 1)))
    (plsql-newline)
    (plsql-newline)
    (insert "end loop")
    (if plsql-loop-is-named (insert " " plsql-loop-name))
    (insert ";"))
  (end-of-line 0)
  (plsql-tab))

(defun plsql-package-spec ()
  "Insert a skeleton package specification."
  (interactive)
  (insert "create or replace package ")
  (let ((plsql-package-name (read-string "package name: " )))
    (insert plsql-package-name " as")
    (plsql-newline)
    (plsql-newline)
    (insert "end " plsql-package-name ";")
    (end-of-line 0)
    (plsql-tab)))

(defun plsql-package-body ()
  "Insert a skeleton package body --  includes a begin statement."
  (interactive)
  (insert "create or replace package body ")
  (let ((plsql-package-name (read-string "package name: " )))
    (insert plsql-package-name " as")
    (plsql-newline)
    (plsql-newline)
    (insert "begin")
    (plsql-newline)
    (insert "end " plsql-package-name ";")
    (end-of-line -1)
    (plsql-tab)))

(defun plsql-get-arg-list ()
  "Read from user a procedure or function argument list.
Add parens unless arguments absent, and insert into buffer.
Individual arguments are arranged vertically if entered one-at-a-time.
Arguments ending with ';' are presumed single and stacked."
  (insert " (")
  (let ((plsql-arg-indent (current-column))
	(plsql-args (read-string "[arguments]: ")))
    (if (string-equal plsql-args "")
	(backward-delete-char 2)
      (progn
	(while (string-match ";$" plsql-args)
	  (insert plsql-args)
	  (newline)
	  (indent-to plsql-arg-indent)
	  (setq plsql-args (read-string "next argument: ")))
	(insert plsql-args ")")))))

(defun plsql-function-spec ()
  "Insert a function specification.  Prompts for name and arguments."
  (interactive)
  (insert "function ")
  (insert (read-string "function name: "))
  (plsql-get-arg-list)
  (insert " return ")
  (insert (read-string "result type: ")))

(defun plsql-procedure-spec ()
  "Insert a procedure specification, prompting for its name and arguments."
  (interactive)
  (insert "procedure ")
  (insert (read-string "procedure name: " ))
  (plsql-get-arg-list))

(defun get-plsql-subprogram-name ()
  "Return (without moving point or mark) a pair whose CAR is
the name of the function or procedure whose spec immediately precedes point,
and whose CDR is the column nbr the procedure/function keyword was found at."
  (save-excursion
    (let ((plsql-proc-indent 0))
      (if (re-search-backward
  ;;;; Unfortunately, comments are not ignored in this string search.
	     "[PpFf][RrUu][OoNn][Cc][EeTt][DdIi][UuOo][RrNn]" nil t)
	  (if (or (looking-at "\\<[Pp][Rr][Oo][Cc][Ee][Dd][Uu][Rr][Ee]\\>")
		  (looking-at "\\<[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]\\>"))
	      (progn
		(setq plsql-proc-indent (current-column))
		(forward-word 2)
		(let ((p2 (point)))
		  (forward-word -1)
		  (cons (buffer-substring (point) p2) plsql-proc-indent)))
	      (get-plsql-subprogram-name))
	   (cons "NAME?" plsql-proc-indent)))))

(defun plsql-subprogram-body ()
  "Insert frame for subprogram body.
Invoke right after plsql-function-spec or plsql-procedure-spec."
  (interactive)
  (insert " is")
  (let ((plsql-subprogram-name-col (get-plsql-subprogram-name)))
    (newline)
    (indent-to (cdr plsql-subprogram-name-col))
    (plsql-newline)
    (insert "begin")
    (plsql-newline)
    (plsql-newline)
    (insert "end " (car plsql-subprogram-name-col) ";"))
  (end-of-line -2)
  (plsql-tab))

(defun plsql-record ()
  "Insert a skeleton record type declaration, prompting for type name."
  (interactive)
  (insert "type ")
  (insert (read-string "type name: "))
  (insert " is record")
  (plsql-newline)
  (plsql-tab)
  (insert "();")
  (plsql-newline)
  (end-of-line 0)
  (backward-char 2))

(defun plsql-while-loop ()
  (interactive)
  (insert "while ")
  (let* ((plsql-loop-name (read-string "loop name: "))
	 (plsql-loop-is-named (not (string-equal plsql-loop-name ""))))
    (if plsql-loop-is-named
	(progn
	  (beginning-of-line)
	  (open-line 1)
	  (insert "<<" plsql-loop-name ">>")
	  (next-line 1)
	  (end-of-line 1)))
    (insert (read-string "entry condition: ") " loop")
    (plsql-newline)
    (plsql-newline)
    (insert "end loop")
    (if plsql-loop-is-named (insert " " plsql-loop-name))
    (insert ";"))
  (end-of-line 0)
  (plsql-tab))

(defun plsql-paired-parens ()
  "Insert a pair of round parentheses, placing point between them."
  (interactive)
  (insert "()")
  (backward-char))

(defun plsql-inline-comment ()
  "Start a comment after the end of the line, indented at least COMMENT-COLUMN.
If starting after END-COMMENT-COLUMN, start a new line."
  (interactive)
  (end-of-line)
  (if (> (current-column) end-comment-column) (newline))
  (if (< (current-column) comment-column) (indent-to comment-column))
  (insert " -- "))

(defun plsql-display-comment ()
"Inserts 3 comment lines, making a display comment."
  (interactive)
  (insert "--\n-- \n--")
  (end-of-line 0))

(provide 'plsql-mode)

;;; plsql-mode.el ends here

