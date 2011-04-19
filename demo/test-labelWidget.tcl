#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl

set lab [gnocl::label \
	-xPad 15 \
	-yPad 5 \
	-text "<span foreground = \"red\"><b>LABEL WIDGET</b></span>" ]

set lab2 [gnocl::label \
	-text "<span foreground = \"blue\"><b>ANOTHER LABEL</b></span>" ]

set box [gnocl::box \
	-shadow in \
	-labelWidget $lab ]

$box add $lab2

gnocl::window \
	-child $box \
	-width 250 \
	-height 75

gnocl::mainLoop
