# test-printing.tcl

#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl


 set box [gnocl::box -orientation vertical]
 set toolbar [gnocl::toolBar]
 $toolbar add item -text "%#Print" -onClicked {gnocl::print print b}
 $toolbar add item -text "Print File" -onClicked {gnocl::print file b}
 $toolbar add item -text "Print Preview" -onClicked {gnocl::print preview $txt}
 set txt [gnocl::text]
 $box add $toolbar
 $box add $txt -fill {1 1} -expand 1
 set top1 [gnocl::window -child $box -onDestroy {exit}  ]


gnocl::mainLoop
