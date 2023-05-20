(TeX-add-style-hook
 "thesis"
 (lambda ()
   (TeX-run-style-hooks
    "latex2e"
    "chapters/ch1"
    "chapters/ch2"
    "chapters/ch3"
    "chapters/ch4"
    "chapters/ch5"
    "chapters/ch6"
    "swfuthesis"
    "swfuthesis10"
    "biolinum")
   (TeX-add-symbols
    '("Ctrl" 1))
   (LaTeX-add-environments
    '("codeblock" LaTeX-env-args ["argument"] 0))
   (LaTeX-add-bibliographies))
 :latex)

