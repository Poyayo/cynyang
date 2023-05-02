(TeX-add-style-hook
 "ch3"
 (lambda ()
   (add-to-list 'LaTeX-verbatim-environments-local "minted")
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
    "subsec:protect"))
 :latex)

