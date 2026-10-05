;;; lucy-mode.el --- Major mode for Lucy -*- lexical-binding: t; -*-

(defconst lucy-keywords
  '("if" "unless" "else" "while" "repeat" "for" "foreach" "loop" "switch" "case" "default" "func" "lambda"
    "class" "struct" "return" "break" "continue" "try" "catch" "finally"
    "throw" "import" "from" "as" "const" "global" "in" "new" "self" "super"))

(defconst lucy-font-lock-keywords
  `((,(regexp-opt lucy-keywords 'words) . font-lock-keyword-face)
    ("\\<\\(true\\|false\\|nil\\)\\>" . font-lock-constant-face)
    ("\\<\\(PI\\|E\\|VERSION\\|PLATFORM\\)\\>" . font-lock-constant-face)
    ("\\$[A-Za-z_][A-Za-z0-9_.]*" . font-lock-variable-name-face)
    ("\\_<\\(def\\|function\\)[ \t]+\\([A-Za-z_][A-Za-z0-9_]*\\)"
     (1 font-lock-keyword-face) (2 font-lock-function-name-face))
    ("\\_<class[ \t]+\\([A-Za-z_][A-Za-z0-9_]*\\)"
     (1 font-lock-type-face))
    ("\\_<\\(0[xX][0-9A-Fa-f]+\\|[0-9]+\\(?:\\.[0-9]+\\)?\\)\\>"
     . font-lock-constant-face)))

(define-derived-mode lucy-mode prog-mode "Lucy"
  "Major mode for the Lucy programming language."
  (setq-local comment-start "#")
  (setq-local comment-end "")
  (setq-local font-lock-defaults '(lucy-font-lock-keywords))
  (setq-local indent-tabs-mode nil)
  (setq-local tab-width 4))

(add-to-list 'auto-mode-alist '("\\.lucy\\'" . lucy-mode))

(provide 'lucy-mode)
;;; lucy-mode.el ends here
