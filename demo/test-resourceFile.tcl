 #!/bin/sh
 # the next line restarts using tclsh \
 exec tclsh "$0" "$@"

 package require Gnocl

 gnocl::resourceFile test.gtkrc

 set box [gnocl::box -orientation vertical]
 set ent [gnocl::entry -name entry -value test.gtkrc]

 $box add [list $ent] -fill {1 1} -expand 1
 set topl [gnocl::window -title "Resource File Test: test.gtkrc" -child $box -widthRequest 200 -heightRequest 150]

 gnocl::mainLoop
