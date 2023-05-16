(TeX-add-style-hook
 "slides"
 (lambda ()
   (TeX-run-style-hooks
    "latex2e"
    "swfubeamer"
    "swfubeamer10"
    "ctex"
    "pifont"
    "biolinum")
   (TeX-add-symbols
    '("Ctrl" 1))
   (LaTeX-add-environments
    '("codeblock" LaTeX-env-args ["argument"] 0)))
 :latex)

