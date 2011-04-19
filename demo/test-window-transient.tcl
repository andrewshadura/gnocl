#---------------
# test-window-transient.tcl
#---------------
# William J Giddings
# 17/Jan/2010
#---------------
#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
#---------------

package require Gnocl

set text(1) [gnocl::text \
    -justify center \
    -baseFont {Sans 18} \
    -pixelsAboveLines 15 \
    -text "Creation of Floating Pallete\nwith -transient option"]

set mainWindow [gnocl::window \
    -child $text(1) \
    -title "Transient Demo - Main Window" \
    -defaultWidth 320 \
    -defaultHeight 200 \
    -onDestroy {exit} ]

set box(palette) [gnocl::box -orientation vertical]

set but(1) [gnocl::button \
    -text "Button-1" \
    -onClicked {
        $text(1) configure -baseColor white
        $text(1) configure -text "Creation of Floating Pallete\nwith -transient option"
        gnocl::beep }]
$box(palette) add $but(1)

foreach {i c} [list 2 red 3 yellow 4 green 5 gray] {
    set but($i) [gnocl::button \
        -text "Button-$i" \
        -onClicked "
            $text(1) configure -baseColor $c
            $text(1) configure -text \"You clicked button $i\"
            gnocl::beep " ]
    $box(palette) add $but($i)
}

set palette [gnocl::window \
    -child $box(palette) \
    -title {} \
    -defaultWidth 100 \
    -defaultHeight 200 \
    -resizable 0 \
    -transient $mainWindow \
    -onDestroy {exit} ]
