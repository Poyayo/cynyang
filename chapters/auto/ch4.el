(TeX-add-style-hook
 "ch4"
 (lambda ()
   (LaTeX-add-labels
    "sec:booting"
    "sec:inprotect"
    "fig:gdt"
    "fig:tequan"
    "subsec:kernel"
    "fig:xp"
    "fig:fenye"
    "fig:fenyezy"
    "fig:infogdt"
    "fig:hc"
    "fig:print_char"
    "subsec:str"
    "fig:print_str"
    "subsec:int"
    "fig:print_int"
    "sec:interrupt"
    "tbl:idt"
    "fig:8259A"
    "fig:interrupt_a"
    "fig:keybd"
    "sec:course"
    "sec:file"
    "sec:shell")
   (LaTeX-add-environments
    '("codeblock" LaTeX-env-args ["argument"] 0)))
 :latex)

