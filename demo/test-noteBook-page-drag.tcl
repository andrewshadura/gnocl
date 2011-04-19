# basic Tcl/Gnocl Script
#!/bin/sh \
exec tclsh "$0" "$@"

package require Gnocl

set notebook1 [gnocl::notebook \
	-scrollable 1 \
	-tabPosition left \
	-groupId 1 \
	-onCreateWindow {
		puts "%w %g %n %c %s %x %y"
		}]

$notebook1 addPage [gnocl::label -text "First Page"] "%__First"

$notebook1 addPage [gnocl::label -text "Second Page"] "%__Second"

$notebook1 addPage [gnocl::label -text "Third Page"] "%__Third"

$notebook1 addPage [gnocl::label -text "Fourth Page"] "%__Fourth"


gnocl::window \
    -title "Notebook1" \
    -child $notebook1 \
	-x 100 \
    -y 100 \
    -width 200 \
    -height 100

set notebook2 [gnocl::notebook -tabPosition right -groupId 1]

$notebook2 addPage [gnocl::label \
    -text "Fifth Page"] "%__Fifth"

$notebook2 addPage [gnocl::label \
    -text "Sixth Page"] "%__Sixth"

gnocl::window \
    -title "Notebook2" \
    -child $notebook2 \
	-x 100 \
    -y 250 \
    -width 200 \
    -height 100


set notebook3 [gnocl::notebook -groupId 2]

$notebook3 addPage [gnocl::label \
    -text "Seventh Page"] "%__Seventh"

$notebook3 addPage [gnocl::label \
    -text "Eigth Page"] "%__Eigth"

gnocl::window \
    -title "Notebook3" \
    -child $notebook3 \
    -x 100 \
    -y 400 \
    -width 200 \
    -height 100
