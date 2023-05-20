(TeX-add-style-hook
 "ch1"
 (lambda ()
   (LaTeX-add-labels
    "sec:env"
    "fig:os")
   (LaTeX-add-environments
    '("codeblock" LaTeX-env-args ["argument"] 0)))
 :latex)

