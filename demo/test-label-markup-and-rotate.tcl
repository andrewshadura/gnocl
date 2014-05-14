#---------------
# test-label-markup-and-rotate.tcl
#---------------
# Author:   William J Giddings
# Date:     13/Jan/10
#---------------
#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
#---------------

package require Gnocl

proc rotate {} {
    global angle
    global lab
    incr angle 5
    if {$angle == "360" } {set angle 0}
    $lab configure -angle $angle
    $lab configure -text "Angle = <span foreground = \"red\"><b>$angle</b></span> degs"
    after 5 rotate
    update
}


 proc setString {angle} {
         #return "Angle = <span foreground ="red"$angle degs>"
         return  "Angle = <span foreground = \"red\"><b>$angle</b></span> degs"
        }

 set angle 0
 set string [setString $angle]

 set lab [gnocl::label \
    -textVariable string  \
    -text "Angle = <span foreground = \"red\"><b>\"0\"</b></span> degs" \
    -baseFont {Sans 30} \
    -angle $angle]

 set but [gnocl::button \
         -icon "%#Refresh" \
         -onClicked {
                 incr angle 15
                 if {$angle == "360" } {set angle 0}
                 set string [setString $angle]
                 $lab configure -angle $angle
        }]

 set box [gnocl::box -orientation vertical]
 $box add $but
 $box add $lab -fill {1 1} -expand 1

 set win [gnocl::window -title "Label Text Pango Markup and Cairo Rotate" \
    -defaultWidth 480 \
    -defaultHeight 300 \
    -child $box \
    -onDelete {exit} ]

rotate
