(TeX-add-style-hook
 "slides"
 (lambda ()
   (TeX-run-style-hooks
    "latex2e"
    "swfubeamer"
    "swfubeamer10"))
 :latex)

