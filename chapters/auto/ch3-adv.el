(TeX-add-style-hook
 "ch3-adv"
 (lambda ()
   (TeX-add-to-alist 'LaTeX-provided-class-options
                     '(("ctexart" "scheme=chinese")))
   (add-to-list 'LaTeX-verbatim-environments-local "minted")
   (TeX-run-style-hooks
    "latex2e"
    "article"
    "art10"
    "graphicx"
    "minted"
    "ctexart"
    "ctexart10")
   (LaTeX-add-labels
    "sec:booting"
    "sec:figure"
    "fig:linux-logo"
    "sec:label"
    "cha:pre-requisite"
    "tab:keys"
    "p:keys"
    "sec:math"
    "sec:code"
    "fig:code"
    "sec:cn"))
 :latex)

