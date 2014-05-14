# basic Tcl/Gnocl Script
#!/bin/sh \
exec tclsh "$0" "$@"
package require Gnocl

set but1 [gnocl::button -text play ]
set but2 [gnocl::button -text stop -sensitive 0 ]

$but1 configure -onClicked \
	{
		$but2 configure -sensitive 1
		%w configure -sensitive 0
		gnocl::sound bell.wav -onFinished { $but1 configure -sensitive 1 ; $but2 configure -sensitive 0 }

	}

$but2 configure -onClicked \
	{
		$but1 configure -sensitive 1
		%w configure -sensitive 0
		gnocl::sound cancel
	}


set box [gnocl::box]

$box add [list $but1 $but2] -fill {1 1 } -expand 1

gnocl::window -child $box -width 200

gnocl::mainLoop
