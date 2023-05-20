(TeX-add-style-hook
 "ch3"
 (lambda ()
   (LaTeX-add-labels
    "cha:OSboot"
    "sec:BIOS"
    "fig:img2-1"
    "sec:MBR"
    "fig:mbr"
    "fig:img2-2"
    "sec:Loader"
    "fig:neicun"
    "sec:kernel"
    "subsec:protect")
   (LaTeX-add-environments
    '("codeblock" LaTeX-env-args ["argument"] 0)))
 :latex)

