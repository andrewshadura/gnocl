#---------------
# test-gladeControl&DisplayWidgets.tcl
#---------------
# William J Giddings
# 07-Dec-2009
#---------------

#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl

# load a simple glade file, containing three buttons in a horizontal row
# the variable glade1_widgets contains a list of all widgets created
# by loading this file, they will include the names given in the glade file itself
# along with the names assigned when registering these with Tcl
set glade1 control&display.glade
set glade1_widgets [gnocl::glade new $glade1]

# for convenience sake, create a list of aliases for these new widgets
# we can use the glade widget names to do this, to make things manageable

foreach item $glade1_widgets {
    foreach {gnocl glade} $item {}
    set $glade $gnocl
}

foreach item $glade1_widgets {
    foreach {gnocl glade} $item {}
    set class [$gnocl class]
    catch {
        # not all widgets have a tooltip
        $gnocl configure -tooltip "$class Supported."
        }
}

$button1 configure \
    -text "Supported" \
    -onClicked {
            puts "My gnocl name is %w, my glade name is %g."
            ${%g} configure -text "CLICKED-1" ;# this works, as this we access the command name held in str of same name!

            }

$label1 configure \
    -text "CHANGED"

puts "togglebutton1 is a  [$togglebutton1 class]"
puts "it's geometry is [$togglebutton1 geometry]"


# reconfigure the mainwindow
$window1 configure -onDestroy {exit}
$window1 center
$window1 show

gnocl::mainLoop
