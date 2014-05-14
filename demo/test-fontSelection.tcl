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

set fontName "SimSun Bold Italic 22"

set fsb1 [gnocl::fontButton \
    -fontName $fontName \
    -title "fontButton test" \
    -showSize 0 -showStyle 0 \
    -useFont 0 -useSize 0 \
    -onFontSet { puts "font = %f\n [%w cget -data] [%w cget -name]" ; set fontName %f ; $ent1 configure -baseFont $fontName } \
    -data OW -name fb1 \
    -tooltip "Font selection button." ]
set ent1 [gnocl::entry -value Gnocl]
set but1 [gnocl::button -text "Font Slection Dialog" -onClicked { set fnt [ gnocl::fontSelectionDialog] ; $ent1 configure -baseFont $fnt }]
set fs1 [gnocl::fontSelection -previewText "prajnaparamita"]
set lab1 [gnocl::label -textVariable fontName]

set but2 [gnocl::button -text "Get FontName" -onClicked { set fontName [ $fs1 cget -fontName] ; puts [$fs1 cget -previewText] ; $ent1 configure -baseFont $fontName }]

$mainBox add [list $ent1 [gnocl::separator] $fsb1 $fs1 $lab1 $but2 $but1]



$fs1 configure -fontName $fontName
$win center

gnocl::mainLoop
