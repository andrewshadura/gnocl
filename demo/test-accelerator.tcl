#---------------
# test-accelerator.tcl
#---------------
#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl

#---------------
#
#---------------
proc ent {txt tip} {
	set box [gnocl::box]
	set ent [gnocl::entry]
	set acc [gnocl::accelarator \
		-text $txt \
		-widget $ent \
		-widthGroup A \
		-align left \
		-tooltip $tip \
		]

	$box add [list $acc $ent]
return $box
}

set container [gnocl::box -orientation vertical]

foreach item [list Apple Bannana Cherry] {
	$container add [ent _$item "I am: $item"]
}

gnocl::window -child $container -onDestroy exit

gnocl::mainLoop
