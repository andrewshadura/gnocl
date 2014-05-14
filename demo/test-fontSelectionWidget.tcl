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
set lab1 [gnocl::label -textVariable fontName]

set fs1 [gnocl::fontSelection -previewText $previewText]

set but1 [gnocl::button \
    -text "Get FontName" \
    -onClicked {
        set fontName [ $fs1 cget -fontName]
        puts [$fs1 cget -previewText]
        $ent1 configure -baseFont $fontName }]

$mainBox add [list $ent1 $lab1 $but1 [gnocl::separator] $fs1  ]

# This option can only been set once the containing toplevel has been rendered.
$fs1 configure -fontName $fontName

$win center

gnocl::mainLoop
