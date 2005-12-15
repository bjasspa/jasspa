;;; Oracle SQL*Forms 3 editing support package.

;; Author: Karel Sprenger <ks@ic.uva.nl>
;; Maintainer: Karel Sprenger <ks@ic.uva.nl>
;; Created: 28 Nov 1996
;; Version: $Revision: 1.1 $
;; Keywords: SQL*Forms major-mode

;; Copyright (C) 1996 Karel Sprenger

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
;;     (autoload 'sqlforms-mode "sqlforms-mode" "SQL*Forms Mode" t)
;;   To invoke plsql-mode automatically on .inp files, do this:
;;     (setq auto-mode-alist (cons (cons "\\.inp$" 'sqlforms-mode) auto-mode-alist))
;;   To change the default plsql-indent value, and to turn on font-lock-mode, add:
;;     (add-hook 'sqlforms-mode-hook'(lambda ()
;;				    (progn (setq sqlforms-indent 3)
;;					   (turn-on-font-lock))))
;;

(eval-when-compile
  (load "make-regexp"))

(defvar sqlforms-mode-syntax-table nil
  "Syntax table in use in SQL*Forms-mode buffers.")

(if sqlforms-mode-syntax-table
    ()
  (setq sqlforms-mode-syntax-table (make-syntax-table (standard-syntax-table)))
  (modify-syntax-entry ?_  "w"     sqlforms-mode-syntax-table) ;word constituent
  (modify-syntax-entry ?\# "_" 	   sqlforms-mode-syntax-table) ;symbol constituent
  (modify-syntax-entry ?$  "_" 	   sqlforms-mode-syntax-table) ;symbol constituent
  (modify-syntax-entry ?\( "()"    sqlforms-mode-syntax-table) ;open-parenthesis
  (modify-syntax-entry ?\) ")("    sqlforms-mode-syntax-table) ;close-parenthesis
;  (modify-syntax-entry ?-  ". 12"  sqlforms-mode-syntax-table) ;-- starts comment to eol
;  (modify-syntax-entry ?\n ">" 	   sqlforms-mode-syntax-table) ;LFD ends comment
;  (modify-syntax-entry ?\^m ">"    sql-mode-syntax-table) ;RET ends comment
  (modify-syntax-entry ?/  ". 14"  sqlforms-mode-syntax-table) ;c-style comment /*...*/
  (modify-syntax-entry ?*  ". 23"  sqlforms-mode-syntax-table)
  (modify-syntax-entry ?\' "\""    sqlforms-mode-syntax-table) ;string quote
;  (modify-syntax-entry ?\" "\""    sqlforms-mode-syntax-table) ;string quote
  (modify-syntax-entry ?%  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?+  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?=  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?&  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?|  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?<  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?>  "."     sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\[ "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\] "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\{ "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\} "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?.  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?@  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\\ "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?:  "." 	   sqlforms-mode-syntax-table) ;punctuation
  (modify-syntax-entry ?\; "." 	   sqlforms-mode-syntax-table) ;punctuation
  )

(defvar sqlforms-mode-map nil
  "Keymap used in SQL*Forms mode.")

(let ((map (make-sparse-keymap)))
  (define-key map "\C-m" 	'sqlforms-newline)
  (define-key map "\C-?" 	'backward-delete-char-untabify)
  (define-key map "\C-i" 	'sqlforms-tab)
  (define-key map "\C-c\C-i" 	'sqlforms-untab)
  (define-key map "\C-c<" 	'sqlforms-backward-to-same-indent)
  (define-key map "\C-c>" 	'sqlforms-forward-to-same-indent)
  (define-key map "\C-ch" 	'sqlforms-header)
  (define-key map "\C-c(" 	'sqlforms-paired-parens)
  (define-key map "\C-c-" 	'sqlforms-inline-comment)
  (define-key map "\C-c\C-d" 	'sqlforms-declare)
  (define-key map "\C-c\C-e" 	'sqlforms-exception)
  (define-key map "\C-c\C-ps" 	'sqlforms-package-spec)
  (define-key map "\C-c\C-pb" 	'sqlforms-package-body)
  (define-key map "\C-c\C-pr" 	'sqlforms-procedure-spec)
  (define-key map "\C-c\C-f" 	'sqlforms-function-spec)
  (define-key map "\C-cb" 	'sqlforms-subprogram-body)
  (define-key map "\C-cf" 	'sqlforms-for-loop)
  (define-key map "\C-cl" 	'sqlforms-loop)
  (define-key map "\C-ci" 	'sqlforms-if)
  (define-key map "\C-cI" 	'sqlforms-elsif)
  (define-key map "\C-ce" 	'sqlforms-else)
  (define-key map "\C-cr" 	'sqlforms-record)
  (define-key map "\C-ct" 	'sqlforms-table)
  (define-key map "\C-c\C-t" 	'sqlforms-tabsize)
  (define-key map "\C-cw" 	'sqlforms-while-loop)
  (define-key map "\C-c\C-w" 	'sqlforms-when)
  (define-key map "\C-cx" 	'sqlforms-exit)
  (setq sqlforms-mode-map map))


(defvar sqlforms-font-lock-keywords nil
  "Additional expressions to highlight in SQL*Forms-Mode.")

(let ((types-1				;SQL
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
      (functions-a			;SQL*Forms packaged procedures and functions
       (eval-when-compile
	 (make-regexp '("abort_query" "anchor_view" "application_characteristic" 
			"bell" "block_characteristic" "block_menu" "break" 
			"call" "call_input" "call_query" "clear_block" "clear_eol" 
			"clear_field" "clear_form" "clear_record" "commit_form" 
			"copy" "copy_region" "count_query" "create_record" 
			"cut_region" 
			"default_value" "delete_record" "display_error" "display_field" 
			"display_page" "do_key" "down" "duplicate_field" "duplicate_record" 
			"edit_field" "enter" "enter_query" "erase" "error_code" 
			"error_text" "error_type" "execute_query" "execute_trigger" 
			"exit_form" 
			"field_characteristic" "first_record" "form_characteristic" 
			"form_failure" "form_fatal" "form_success" 
			"go_block" "go_field" "go_record" 
			"help" "hide_menu" "hide_page" "host" 
			"last_record" "list_values" "lock_record" 
			 "message" "message_code" "message_text" "message_type" "move_view" 
			 "name_in" "new_form" "next_block" "next_field" "next_key" 
			 "next_record" "next_set" 
			 "paste_region" "pause" "post" "previous_block" "previous_field" 
			 "previous_record" "print" 
			 "redisplay" "replace_menu" "resize_view" 
			 "scroll_down" "scroll_up" "set_field" "set_input_focus" 
			 "show_keys" "show_menu" "show_page" "synchronize" 
			 "up" "user_exit"))))
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
      (exceptions-1			;Predefined Exceptions + OTHERS
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
      (exceptions-2			;Predefined SQL*Forms exceptions
       (eval-when-compile
	    (make-regexp '("form_trigger_failure"))))

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
      (keywords-3			;SQL*Forms DEFINE's
       (eval-when-compile
	 (make-regexp '("block" 
			"field" "form" 
			"page" "procedure" 
			"reference" 
			"screen" 
			"trigger"))))
      (triggers				;SQL*Forms triggers
       (eval-when-compile
	 (make-regexp '("key-clrblk" "key-clrfrm" "key-clrrec" "key-commit" "key-cquery" 
			"key-crerec" "key-delrec" "key-down" "key-dupfld" "key-duprec" 
			"key-edit" "key-enter" "key-entqry" "key-exeqry" "key-exit" 
			"key-f0" "key-f1" "key-f2" "key-f3" "key-f4" "key-f5" "key-f6" 
			"key-f7" "key-f8" "key-f9" "key-help" "key-listval" "key-menu" 
			"key-nxtblk" "key-nxtfld" "key-nxtkey" "key-nxtrec" "key-nxtset" 
			"key-others" "key-print" "key-prvblk" "key-prvfld" "key-prvrec" 
			"key-scrdown" "key-scrup" "key-startup" "key-up" "key-updrec" 
			"on-clear-block" "on-database-record" "on-delete" "on-error" 
			"on-insert" "on-lock" "on-message" "on-new-field-instance" 
			"on-new-record" "on-remove-record" "on-update" "on-validate-field" 
			"on-validate-record" "post-block" "post-change" "post-commit" 
			"post-delete" "post-field" "post-form" "post-insert" "post-query" 
			"post-record" "post-update" "pre-block" "pre-commit" "pre-delete" 
			"pre-field" "pre-form" "pre-insert" "pre-query" "pre-record" 
			"pre-update"))))
      )
  (setq sqlforms-font-lock-keywords
	(list
	 (list (concat "\\b\\(" types-1     "\\)\\b") 
	       1 'font-lock-type-face)
	 (list (concat "\\b\\(" types-2     "\\)\\b") 
	       1 'font-lock-type-face)
	 (list (concat "\\w\\(" types-3     "\\)\\b")
	       1 'font-lock-type-face)
	 (list (concat "\\b\\(" functions-1 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-2 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-3 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-4 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-5 "\\)\\b[ \n\t;]") ;No parentheses!
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-6 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-7 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-8 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-9 "\\)\\b[ \n\t]*(") 
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" functions-a "\\)\\b[ \n\t]*(?") ;Possibly no parentheses!
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" cmndwords-1 "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" cmndwords-2 "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" exceptions-1  "\\)\\b")
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" exceptions-2  "\\)\\b")
	       1 'font-lock-function-name-face)
	 (list (concat "\\b\\(" operators   "\\)\\b") 
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" keywords-1  "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(" keywords-2  "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list "\\b\\(function\\|procedure\\|package\\)\\b[ \t]+\\([^ \t(]+\\)"
	       2 'font-lock-function-name-face)
	 (list (concat "\\b\\(" keywords-3  "\\)\\b")
	       1 'font-lock-keyword-face)
	 (list (concat "\\b\\(\\(end\\)?define[ \t]+\\(" keywords-3 "\\)\\)\\b")
	       1 'font-lock-keyword-face t)
	 (list "\\b\\(function\\|procedure\\)\\b[ \t]+\\([^ \t(]+\\)"
	       2 'font-lock-function-name-face)
	 (list (concat "^[ \t]+\\b\\(name\\|procedure\\|application\\)\\b[ \t]+=[ \t]+\\(.+\\)$")
	       2 'font-lock-function-name-face)
	 (list (concat "\\b\\(" triggers  "\\)\\b")
	       1 'font-lock-function-name-face t)
	 (list "'\\([^']*\\)'" 1 'font-lock-string-face t)
	 (list "^\\(rem\\([ \t].*\\)?\\)$" 1 'font-lock-comment-face t)
	 (list "\\(/\\*.*\\*/\\)" 1 'font-lock-comment-face t))))

(defvar sqlforms-indent 3 "*Value is the number of columns to indent in SQL*Forms-Mode.")
  
(defun sqlforms-mode ()
"This is a mode intended to support program development in SQL*Forms.
Most control constructs and declarations of SQL*Forms can be inserted in the buffer
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
C-c (     paired parens  C-c -    inline comment
                         C-c h    header sec

C-c < and C-c > move backward and forward respectively to the next line
having the same (or lesser) level of indentation.

Variable sqlforms-indent controls the number of spaces for indent/undent.

\\{sqlforms-mode-map}
"
  (interactive)
  (kill-all-local-variables)
  (use-local-map sqlforms-mode-map)
  (setq major-mode 'sqlforms-mode)
  (setq mode-name "SQL*Forms")
  (make-local-variable 'comment-column)
  (setq comment-column 41)
  (make-local-variable 'end-comment-column)
  (setq end-comment-column 72)
  (set-syntax-table sqlforms-mode-syntax-table)
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
  (make-local-variable 'comment-indent-hook)
  (setq comment-indent-hook 'c-comment-indent)
  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)
  (setq case-fold-search t)		;Automatically buffer-local
  (cond	((string-match "Win-Emacs" (emacs-version)) ; Win-Emacs
	 (make-local-variable 'font-lock-keywords-case-fold-search)
	 (make-local-variable 'font-lock-keywords)
	 (setq font-lock-keywords-case-fold-search t)
	 (setq font-lock-keywords sqlforms-font-lock-keywords))
	;;
        ((string-match "XEmacs\\|Lucid" (emacs-version)) ; XEmacs/Lucid
	 (put major-mode 'font-lock-keywords-case-fold-search t))
	;; XEmacs (19.13, at least) guesses the rest correctly.
	;; If any older XEmacsen don't, then tell me.
	;;
	((string-lessp "19.28.89" emacs-version) ; Emacs 19.29 and later
	 (make-local-variable 'font-lock-defaults)
	 (setq font-lock-defaults 
	       '(sqlforms-font-lock-keywords t t nil beginning-of-line
		 (font-lock-comment-start-regexp . "--\\|/\*"))))
	;;
	(t ; Emacs 19.28 and older
	 (make-local-variable 'font-lock-keywords-case-fold-search)
	 (make-local-variable 'font-lock-keywords)
	 (make-local-variable 'font-lock-no-comments)
	 (setq font-lock-keywords-case-fold-search t)
	 (setq font-lock-keywords sqlforms-font-lock-keywords)
	 (setq font-lock-no-comments t)))
  (run-hooks 'sqlforms-mode-hook))

(defun sqlforms-tabsize (s)
  "changes spacing used for indentation. Reads spacing from minibuffer."
  (interactive "nnew indentation spacing: ")
  (setq sqlforms-indent s))

(defun sqlforms-newline ()
  "Start new line and indent to current tab stop."
  (interactive)
  (let ((sqlforms-cc (current-indentation)))
    (newline)
    (indent-to sqlforms-cc)))

(defun sqlforms-tab ()
  "Indent to next tab stop."
  (interactive)
  (indent-to (* (1+ (/ (current-indentation) sqlforms-indent)) sqlforms-indent)))

(defun sqlforms-untab ()
  "Delete backwards to previous tab stop."
  (interactive)
  (backward-delete-char-untabify sqlforms-indent nil))

(defun sqlforms-go-to-this-indent (step indent-level)
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

(defun sqlforms-backward-to-same-indent ()
  "Move point backwards to nearest line with same indentation or less.
If not found, point is left at top of buffer."
  (interactive)
  (sqlforms-go-to-this-indent -1 (current-indentation))
  (back-to-indentation))

(defun sqlforms-forward-to-same-indent ()
  "Move point forwards to nearest line with same indentation or less.
If not found, point is left at start of last line in buffer."
  (interactive)
  (sqlforms-go-to-this-indent 1 (current-indentation))
  (back-to-indentation))

(defun sqlforms-table ()
  "Insert SQL*Formsn table type definition, prompting for type name and component type,
then insert the INDEX BY clause."
  (interactive)
  (insert "type ")
  (insert (read-string "type name: "))
  (insert " is table of ")
  (insert (read-string "component type: "))
  (sqlforms-newline)
  (sqlforms-tab)
  (insert "index by binary_integer;")
  (sqlforms-newline))

(defun sqlforms-declare ()
  "Start a declare part and indent for the 1st declaration."
  (interactive)
  (insert "declare")
  (sqlforms-newline)
  (sqlforms-newline)
  (end-of-line -1)
  (sqlforms-tab))

(defun sqlforms-exception ()
  "Undent and insert an exception part into a block. Reindent."
  (interactive)
  (sqlforms-untab)
  (insert "exception")
  (sqlforms-newline)
  (sqlforms-tab))

(defun sqlforms-else ()
  "Add an else clause inside an if-then-end-if clause."
  (interactive)
  (sqlforms-untab)
  (insert "else")
  (sqlforms-newline)
  (sqlforms-tab))

(defun sqlforms-exit ()
  "Insert an exit statement, prompting for loop name and condition."
  (interactive)
  (insert "exit")
  (let ((sqlforms-loop-name (read-string "[name of loop to exit]: ")))
    (if (not (string-equal sqlforms-loop-name "")) (insert " " sqlforms-loop-name)))
  (let ((sqlforms-exit-condition (read-string "[exit condition]: ")))
    (if (not (string-equal sqlforms-exit-condition ""))
	(if (string-match "^ *[Ww][Hh][Ee][Nn] +" sqlforms-exit-condition)
	    (insert " " sqlforms-exit-condition)
	  (insert " when " sqlforms-exit-condition))))
  (insert ";"))

(defun sqlforms-when ()
  "Start an excption handler with a when clause."
  (interactive)
  (insert "when ")
  (insert (read-string "exception: "))
  (sqlforms-newline)
  (sqlforms-tab))

(defun sqlforms-for-loop ()
  "Build a skeleton for-loop statement, prompting for the loop parameters."
  (interactive)
  (insert "for ")
  (let* ((sqlforms-loop-name (read-string "[loop name]: "))
	 (sqlforms-loop-is-named (not (string-equal sqlforms-loop-name ""))))
    (if sqlforms-loop-is-named
	(progn
	  (beginning-of-line)
	  (open-line 1)
	  (insert "<<" sqlforms-loop-name ">>")
	  (next-line 1)
	  (end-of-line 1)))
    (insert (read-string "loop variable: ") " in ")
    (insert (read-string "range: ") " loop")
    (sqlforms-newline)
    (sqlforms-newline)
    (insert "end loop")
    (if sqlforms-loop-is-named (insert " " sqlforms-loop-name))
    (insert ";"))
  (end-of-line 0)
  (sqlforms-tab))

(defun sqlforms-header ()
  "Insert a comment block containing the module title, author, etc."
  (interactive)
  (insert "--\n--  Title: \t")
  (insert (read-string "Title: "))
  (insert "\n--  Created:\t" (current-time-string))
  (insert "\n--  Author: \t" (user-full-name))
  (insert "\n--\t\t<" (user-login-name) "@" (system-name) ">\n--\n"))

(defun sqlforms-if ()
  "Insert skeleton if statment, prompting for a boolean-expression."
  (interactive)
  (insert "if ")
  (insert (read-string "condition: ") " then")
  (sqlforms-newline)
  (sqlforms-newline)
  (insert "end if;")
  (end-of-line 0)
  (sqlforms-tab))

(defun sqlforms-elsif ()
  "Add an elsif clause to an if statement, prompting for the boolean-expression."
  (interactive)
  (sqlforms-untab)
  (insert "elsif ")
  (insert (read-string "condition: ") " then")
  (sqlforms-newline)
  (sqlforms-tab))

(defun sqlforms-loop ()
  "insert a skeleton loop statement.  exit statement added by hand."
  (interactive)
  (insert "loop ")
  (let* ((sqlforms-loop-name (read-string "[loop name]: "))
	 (sqlforms-loop-is-named (not (string-equal sqlforms-loop-name ""))))
    (if sqlforms-loop-is-named
	(progn
	  (beginning-of-line)
	  (open-line 1)
	  (insert "<<" sqlforms-loop-name ">>")
	  (forward-line 1)
	  (end-of-line 1)))
    (sqlforms-newline)
    (sqlforms-newline)
    (insert "end loop")
    (if sqlforms-loop-is-named (insert " " sqlforms-loop-name))
    (insert ";"))
  (end-of-line 0)
  (sqlforms-tab))

(defun sqlforms-package-spec ()
  "Insert a skeleton package specification."
  (interactive)
  (insert "create or replace package ")
  (let ((sqlforms-package-name (read-string "package name: " )))
    (insert sqlforms-package-name " as")
    (sqlforms-newline)
    (sqlforms-newline)
    (insert "end " sqlforms-package-name ";")
    (end-of-line 0)
    (sqlforms-tab)))

(defun sqlforms-package-body ()
  "Insert a skeleton package body --  includes a begin statement."
  (interactive)
  (insert "create or replace package body ")
  (let ((sqlforms-package-name (read-string "package name: " )))
    (insert sqlforms-package-name " as")
    (sqlforms-newline)
    (sqlforms-newline)
    (insert "begin")
    (sqlforms-newline)
    (insert "end " sqlforms-package-name ";")
    (end-of-line -1)
    (sqlforms-tab)))

(defun sqlforms-get-arg-list ()
  "Read from user a procedure or function argument list.
Add parens unless arguments absent, and insert into buffer.
Individual arguments are arranged vertically if entered one-at-a-time.
Arguments ending with ';' are presumed single and stacked."
  (insert " (")
  (let ((sqlforms-arg-indent (current-column))
	(sqlforms-args (read-string "[arguments]: ")))
    (if (string-equal sqlforms-args "")
	(backward-delete-char 2)
      (progn
	(while (string-match ";$" sqlforms-args)
	  (insert sqlforms-args)
	  (newline)
	  (indent-to sqlforms-arg-indent)
	  (setq sqlforms-args (read-string "next argument: ")))
	(insert sqlforms-args ")")))))

(defun sqlforms-function-spec ()
  "Insert a function specification.  Prompts for name and arguments."
  (interactive)
  (insert "function ")
  (insert (read-string "function name: "))
  (sqlforms-get-arg-list)
  (insert " return ")
  (insert (read-string "result type: ")))

(defun sqlforms-procedure-spec ()
  "Insert a procedure specification, prompting for its name and arguments."
  (interactive)
  (insert "procedure ")
  (insert (read-string "procedure name: " ))
  (sqlforms-get-arg-list))

(defun get-sqlforms-subprogram-name ()
  "Return (without moving point or mark) a pair whose CAR is
the name of the function or procedure whose spec immediately precedes point,
and whose CDR is the column nbr the procedure/function keyword was found at."
  (save-excursion
    (let ((sqlforms-proc-indent 0))
      (if (re-search-backward
  ;;;; Unfortunately, comments are not ignored in this string search.
	     "[PpFf][RrUu][OoNn][Cc][EeTt][DdIi][UuOo][RrNn]" nil t)
	  (if (or (looking-at "\\<[Pp][Rr][Oo][Cc][Ee][Dd][Uu][Rr][Ee]\\>")
		  (looking-at "\\<[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]\\>"))
	      (progn
		(setq sqlforms-proc-indent (current-column))
		(forward-word 2)
		(let ((p2 (point)))
		  (forward-word -1)
		  (cons (buffer-substring (point) p2) sqlforms-proc-indent)))
	      (get-sqlforms-subprogram-name))
	   (cons "NAME?" sqlforms-proc-indent)))))

(defun sqlforms-subprogram-body ()
  "Insert frame for subprogram body.
Invoke right after sqlforms-function-spec or sqlforms-procedure-spec."
  (interactive)
  (insert " is")
  (let ((sqlforms-subprogram-name-col (get-sqlforms-subprogram-name)))
    (newline)
    (indent-to (cdr sqlforms-subprogram-name-col))
    (sqlforms-newline)
    (insert "begin")
    (sqlforms-newline)
    (sqlforms-newline)
    (insert "end " (car sqlforms-subprogram-name-col) ";"))
  (end-of-line -2)
  (sqlforms-tab))

(defun sqlforms-record ()
  "Insert a skeleton record type declaration, prompting for type name."
  (interactive)
  (insert "type ")
  (insert (read-string "type name: "))
  (insert " is record")
  (sqlforms-newline)
  (sqlforms-tab)
  (insert "();")
  (sqlforms-newline)
  (end-of-line 0)
  (backward-char 2))

(defun sqlforms-while-loop ()
  (interactive)
  (insert "while ")
  (let* ((sqlforms-loop-name (read-string "loop name: "))
	 (sqlforms-loop-is-named (not (string-equal sqlforms-loop-name ""))))
    (if sqlforms-loop-is-named
	(progn
	  (beginning-of-line)
	  (open-line 1)
	  (insert "<<" sqlforms-loop-name ">>")
	  (next-line 1)
	  (end-of-line 1)))
    (insert (read-string "entry condition: ") " loop")
    (sqlforms-newline)
    (sqlforms-newline)
    (insert "end loop")
    (if sqlforms-loop-is-named (insert " " sqlforms-loop-name))
    (insert ";"))
  (end-of-line 0)
  (sqlforms-tab))

(defun sqlforms-paired-parens ()
  "Insert a pair of round parentheses, placing point between them."
  (interactive)
  (insert "()")
  (backward-char))

(defun sqlforms-inline-comment ()
  "Start a comment after the end of the line, indented at least COMMENT-COLUMN.
If starting after END-COMMENT-COLUMN, start a new line."
  (interactive)
  (end-of-line)
  (if (> (current-column) end-comment-column) (newline))
  (if (< (current-column) comment-column) (indent-to comment-column))
  (insert " -- "))

(defun sqlforms-display-comment ()
"Inserts 3 comment lines, making a display comment."
  (interactive)
  (insert "--\n-- \n--")
  (end-of-line 0))

(provide 'sqlforms-mode)

;; sqlforms-mode.el ends here
