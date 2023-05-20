(TeX-add-style-hook
 "ch2"
 (lambda ()
   (LaTeX-add-labels
    "sec:overalldesign"
    "fig:boot")
   (LaTeX-add-environments
    '("codeblock" LaTeX-env-args ["argument"] 0)))
 :latex)

