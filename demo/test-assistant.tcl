#---------------
# test-assistant.tcl
#---------------
# Author:   William J Giddings
# Date:     07/05/09
#---------------
#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
#---------------

package require Gnocl

proc gotoPage {w p} {
    gnocl::signalStop $w prepare
    $w page set $p
}

#---------------
# create widget
#---------------
set asst [ gnocl::assistant \
    -title "THIS IS THE TITLE" \
    -linear 0 \
    -resizable 0 \
    -keepAbove 1 \
    -modal 1 \
    -widthRequest 640 \
    -heightRequest 400 \
    -onPrepare {puts "Prepare for what? %w %n"} \
    -onClose {puts "this is where we would apply changes %w"; exit } \
    -onCancel {puts "Cancel %w %n" ; exit } \
    -onApply  { puts "Apply %w %n" } \
    -onDestroy { exit }]

#    -backgroundColor beige \
#    -backgroundImage gnocl_logo_sml.png

#---------------
# add the necessary blank pages
#---------------
$asst addPage intro
$asst addPage content -title "Eggs"
$asst addPage progress
$asst addPage confirm -title "Bacon"
$asst addPage summary




#---------------
# page 0 intro
#---------------
set str(1) "
Note that an assistant needs to end its page flow with a page of type
GTK_ASSISTANT_PAGE_CONFIRM or GTK_ASSISTANT_PAGE_SUMMARY to be correct.

Intro:     The page contains an introduction to the assistant task.

Content:   The page has regular contents.

Progress:  Used for tasks that take a long time to complete, blocks the
           assistant until the page is marked as complete.

Confirm:   The page lets the user confirm or deny the changes.

Summary:   The page informs the user of the changes done.

-----
[$asst pages] added
"

set page(0) [gnocl::text -text $str(1) -leftMargin 10 -cursorVisible no -baseFont {Monospace 10} -sensitive no]

#---------------
# page 1 content
#---------------

set page(1) [gnocl::button -text "Eggs goto BACON" -onClicked {$asst page set 3} ]


#---------------
# page 2 content
#---------------

set page(2) [gnocl::button -text "Nothing Here Yet" -onClicked {gnocl::beep} ]
$asst page configure -title "RENAMED"


#---------------
# page 3 content
#---------------

set page(3) [gnocl::button -text "Bacon goto EGGS" -onClicked {$asst page set 1} ]


#---------------
# page 4 content
#---------------

set page(4) [gnocl::button -text "DONE!" -onClicked {gnocl::beep} ]


#---------------
# configure pages in one swoop!
#---------------
for {set i 0} {$i< [$asst pages]} {incr i} {
    $asst page configure $i -child $page($i)
}

gnocl::mainLoop
