#---------------
# gradients.tcl
#---------------
# author: William J Giddings
# date:   07/03/2010
#---------------

#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl

# horizontal gradient
#   1 ------ 2
#   |        |
#   |        |
#   2 -------2
proc horizontal_gradient { pb c1 c2} {

    # get size of the buffer
    set w [$pb cget -width]
    set h [$pb cget -height]

    foreach {r1 g1 b1} $c1 {}
    foreach {r2 g2 b2} $c2 {}

    for {set x 0} {$x < $w} {incr x } {
        for {set y 0} {$y <$h} {incr y } {
            # puts "$x $y"
            set r3 [expr $r1 +  (($r2 - $r1) * $x / $w) ]
            if {$r3>=255} {set r3 255}

            set g3 [expr $g1 +  (($g2 - $g1) * $x / $w) ]
            if {$g3>=255} {set g3 255}

            set b3 [expr $b1 +  (($b2 - $b1) * $x / $w) ]
            if {$b3>=255} {set 3 255}

            #$pb setPixel [list $x $y] [dec2rgb $r3 $g3 $b3]
            $pb setPixel [list $x $y] [gnocl::rgb2hex [list $r3 $g3 $b3] ]
        }
    }
}

# vertical gradient
#   1 ------ 1
#   |        |
#   |        |
#   2 -------2
proc vertical_gradient { pb c1 c2} {

    # get size of the buffer
    set w [$pb cget -width]
    set h [$pb cget -height]

    foreach {r1 g1 b1} $c1 {}
    foreach {r2 g2 b2} $c2 {}

    for {set y 0} {$y < $h} {incr y } {
        for {set x 0} {$x < $h} {incr x } {
            # puts "$x $y"
            set r3 [expr $r1+  (($r2 - $r1) * $y / $h) ]
            if {$r3>=255} {set r3 255}
            set g3 [expr $g1+  (($g2 - $g1) * $y / $h) ]
            if {$g3>=255} {set g3 255}
            set b3 [expr $b1+  (($b2 - $b1) * $y / $h) ]
            if {$b3>=255} {set 3 255}
            $pb setPixel [list $x $y] [gnocl::rgb2hex [list $r3 $g3 $b3] ]
        }
    }
}

# four corners gradient fill
#   1 ------ 2
#   |        |
#   |        |
#   3 -------4
proc four_corners_gradient { pb c1 c2 c3 c4} {

    # get size of the buffer
    set w [$pb cget -width]
    set h [$pb cget -height]

    foreach {r1 g1 b1} $c1 {}
    foreach {r2 g2 b2} $c2 {}
    foreach {r3 g3 b3} $c3 {}
    foreach {r4 g4 b4} $c4 {}

     # working top to bottom, left to right
    for {set y 0} {$y < $h} {incr y } {

        # vertical component (1-3)
        set ra [expr $r1+  (($r3 - $r1) * $y / $h) ]
        if {$r3>=255} {set r3 255}

        set ga [expr $g1+  (($g3 - $g1) * $y / $h) ]
        if {$g3>=255} {set g3 255}

        set ba [expr $b1+  (($b3 - $b1) * $y / $h) ]
        if {$b3>=255} {set b3 255}

        # horizontal component (2-4)
        set rb [expr $r2+  (($r4 - $r2) * $y / $h ) ]
        if {$r3>=255} {set r3 255}

        set gb [expr $g2+  (($g4 - $g2) * $y / $h) ]
        if {$g3>=255} {set g3 255}

        set bb [expr $b2+  (($b4 - $b2) * $y / $h ) ]
        if {$b3>=255} {set b3 255}

        for {set x 0} {$x < $w} {incr x } {

            # calculate spread
            set r [expr $ra+  (($rb - $ra) * $x / $w) ]
            if {$r>=255} {set r 255}

            set g [expr $ga+  (($gb - $ga) * $x / $w) ]
            if {$g>=255} {set g 255}

            set b [expr $ba+  (($bb - $ba) * $x / $w) ]
            if {$b>=255} {set b 255}

            $pb setPixel [list $x $y] [gnocl::rgb2hex [list $r $g $b] ]
        }

    }
}





# dec2rgb --
#
#   Takes a color name or dec triplet and returns a #RRGGBB color.
#   If any of the incoming values are greater than 255,
#   then 16 bit value are assumed, and #RRRRGGGGBBBB is
#   returned, unless $clip is set.
#
# Arguments:
#   r       red dec value, or list of {r g b} dec value or color name
#   g       green dec value, or the clip value, if $r is a list
#   b       blue dec value
#   clip    Whether to force clipping to 2 char hex
# Results:
#   Returns a #RRGGBB or #RRRRGGGGBBBB color
#
proc dec2rgb {r {g 0} {b UNSET} {clip 0}} {
    if {![string compare $b "UNSET"]} {
    set clip $g
    if {[regexp {^-?(0-9)+$} $r]} {
        foreach {r g b} $r {break}
    } else {
        foreach {r g b} [winfo rgb . $r] {break}
    }
    }
    set max 255
    set len 2
    if {($r > 255) || ($g > 255) || ($b > 255)} {
    if {$clip} {
        set r [expr {$r>>8}]; set g [expr {$g>>8}]; set b [expr {$b>>8}]
    } else {
        set max 65535
        set len 4
    }
    }
    return [format "#%.${len}X%.${len}X%.${len}X" \
        [expr {($r>$max)?$max:(($r<0)?0:$r)}] \
        [expr {($g>$max)?$max:(($g<0)?0:$g)}] \
        [expr {($b>$max)?$max:(($b<0)?0:$b)}]]
}


# the ubiquitous demo block
set pb1 [gnocl::pixBuf new -width 64 -height 64 ]
set pb2 [gnocl::pixBuf new -width 64 -height 64 ]
set pb3 [gnocl::pixBuf new -width 64 -height 64 ]

set img1 [gnocl::image -image "%?$pb1"]
set img2 [gnocl::image -image "%?$pb2"]
set img3 [gnocl::image -image "%?$pb3"]

set box [gnocl::box -children [list $img1 $img2 $img3 ] ]

gnocl::window -child $box

# modify the buffer, and the image will be automatically updated
horizontal_gradient $pb1 {0 0 0 } {128 128 128}
vertical_gradient $pb2 {255 0 255} {0 255 0}
four_corners_gradient $pb3 {0 255 255} {255 0 0} {0 2550} {0 0 255}

# set pb4 [$pb3 rotate -angle 45 -backgroundColor 0]



gnocl::mainLoop
