#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# test-fontSelection.tcl
# 28/Dec/2009

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical]
set win [gnocl::window -child $mainBox -allowShrink 0 -onDestroy exit]

#--------------- put demo code here ---------------

set fontName "Sans Bold Italic 22"
set previewText "How now brown cow."

set ent1 [gnocl::entry -value Gnocl -baseFont $fontName]

set but1 [gnocl::button \
    -text "Font Slection Dialog" \
    -onClicked {
        set fontName [ gnocl::fontSelectionDialog \
            -onClicked {
                puts "On Clicked %w %f %x"
                set fontName %f
                $ent1 configure -baseFont $fontName} \
            -modal 1 \
            -visible 1 \
            -name ABC \
            -title "FONT SELECTION DIALOG" \
            -onDestroy {puts "BYE-BYE" } \
            -font $fontName \
            -previewText $previewText ] }]


$mainBox add [list $ent1 [gnocl::separator] $but1 ]

gnocl::mainLoop
