 #!/bin/sh
 # the next line restarts using tclsh \
 exec tclsh "$0" "$@"

package require Gnocl

set opacity 1.0
set di -0.1

proc refresh { w } {
    global opacity
    global di

    # gnocl::beep

    if { $opacity  ==  "0.0" } { set di "0.1" }
    if { $opacity ==  "1.0" } {  set di "-0.1" }

    set opacity  [expr $opacity + $di]

    $w configure -opacity $opacity

    update

    after 100 refresh $w

}

set win [gnocl::window \
    -onDelete exit \
    -borderWidth 1 \
    -defaultWidth 600\
    -defaultHeight 400 \
    -backgroundImage gnocl_logo.png \
    -mask gnocl_logo.png \
    -opacity 0.5  \
    -tooltip "Click MB-1 to Close" \
    -decorated 0 \
    -onButtonRelease { exit}
    ]

$win center

after 100 refresh $win


gnocl::mainLoop
