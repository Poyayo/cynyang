(TeX-add-style-hook
 "slides"
 (lambda ()
   (TeX-run-style-hooks
    "latex2e"
    "swfubeamer"
    "swfubeamer10"
    "ctex"
    "biolinum")
   (TeX-add-symbols
    '("Ctrl" 1)))
 :latex)

