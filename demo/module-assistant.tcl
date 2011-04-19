#---------------
# module-assistant.tcl
#---------------
# Author:   William J Giddings
# Date:     12/Jan/10
#---------------
# Note that an assistant needs to end its page flow with a page of type
# GTK_ASSISTANT_PAGE_CONFIRM or GTK_ASSISTANT_PAGE_SUMMARY to be correct.

# Intro:    The page contains an introduction to the assistant task.
# Content:  The page has regular contents.
# Progress: Used for tasks that take a long time to complete, blocks the assistant until the page is marked as complete.
# Confirm:  The page lets the user confirm or deny the changes.
# Summary:  The page informs the user of the changes done.
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
# set some defaults
#---------------
set sidebar(default) Porky_pig.jpg
set header(default) banner.png



#---------------
# create widget
#---------------
set wid [ gnocl::assistant \
    -title "Gnocl Assistant [gnocl::info version] Gtk+ [gnocl::info gtkVersion] " \
    -linear yes \
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

$wid addPage intro -title "Create New Gnocl Widget Module"
$wid addPage content -title "Eggs"
$wid addPage progress
$wid addPage confirm -title "Bacon"
$wid addPage summary


# add element to various paages

set str(0) \
{This program will help you create a new module for the implementation of
additional Gtk Widget bindings for inclusion in the Gnocl package core.

During the creation of the necessary files, the current project Makefile
will be backed-up and the new module object file added to the list of compiled
packages based upon the information that you provide.

Before continuing ensure that you have security archives of all vital project files.

Finally, you will also be asked if you wish to create documentation and
demo scripts for the new module.}


set str(1) "page 1"
set str(2) "page 2"
set str(3) "page 3"
set str(4) "last page"


#---------------
# set default graphics for each page
#---------------
for {set i 0} {$i < [$wid pages] } { incr i } {
    set header($i)  $header(default)
    #set sidebar($i) $sidebar(default)
}

for {set i 0} {$i < [$wid pages] } { incr i } {
    set page($i) [gnocl::label -text $str($i) -baseFont {Sans 12}]
    $wid page configure $i -child $page($i) -headerImage $header($i) ;# -sidebarImage $sidebar($i)
}


gnocl::mainLoop
