#---------------
# test-pixMap.tcl
#---------------
# William J Giddings
# 17/Jan/2010
#---------------
#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
#---------------

package require Gnocl

gnocl::pixMap cmd1 subCmd2 3
gnocl::pixMap cmd2 -option1 {a b c d}
gnocl::pixMap cmd3 2 3
