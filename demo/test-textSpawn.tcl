 #---------------
 # textSpawn.tcl
 #---------------
 # Demonstrate gnocl::text textview spawining.
 #---------------
 # Created by William J Giddings
 # 06/12/08
 #---------------
 # Notes:
 # GtkPaned widgets are not as flexible as their Tk equivalents.
 # A paned can only be divided into two, and its orientation cannot
 # be reconfigured during run-time. Also, child widgets are not easily
 # released, consquently, force a relase by assigning new child widgets.
 #---------------

 #!/bin/sh
 # the next line restarts using tclsh \
 exec tclsh "$0" "$@"

 package require Gnocl

set tb [gnocl::textBuffer]

 set title "textSpawn"

 # Repack the textwidget into a new paned widget, then repack the
 # paned widget into the text's original container.
 # parentText   name of the textwidget to divide
 # orientation  orientation of the pane
 proc splitView {textWidget toolBar} {
	 	 $textWidget save /home/wjgiddings/Desktop/test
	 


$::sibling load /home/wjgiddings/Desktop/test

puts SV-2
    # create paned view
    set parent [$textWidget parent]
    set paned [gnocl::paned -orientation vertical]
    $parent configure -children $paned -fill {1 1} -expand 1
    $paned configure -children [list $textWidget $::sibling]
    
	set ::sib_button [gnocl::button \
		-text "Delete Spawn text" \
		-onClicked {
			 $::sibling delete
			 %w delete } ]

    $toolBar add widget $::sib_button
    
    return $::sibling
 }

 # Spawn a new textview prior to deletion of the paned widget.
 # parentText   name of the textwidget to divide
 # orientation  orientation of the pane
 proc unSplitView { textWidget } {

    return $newText
 }

 # create maincontainer
 set box(main) [gnocl::box -orientation vertical -align topLeft]

 # create container for the text areas
 set box(texts) [gnocl::box]

 # create the base text
 set text(1) [gnocl::text -wrapMode word]
 
 
 $text(1) load /home/wjgiddings/Desktop/test
 
 
 $box(texts) add $text(1) -fill {1 1} -expand 1

 # create toolBar
 set toolBar [gnocl::toolBar]
 set split 0
 $toolBar add widget [gnocl::toggleButton \
	-variable split \
	-text "Split" \
	-onToggled {
    if {$split} {
        splitView $text(1) $toolBar
    } else {
#set text(1) [unSplitView $text(1)]  }
    } ]

 # pack the main container
 $box(main) add $toolBar -fill {1 0} -expand 0
 $box(main) add $box(texts) -fill {1 1} -expand 1

 set main [gnocl::window \
	-title $title \
	-onDelete { exit } \
	-child $box(main) \
	-defaultWidth 480 \
	-defaultHeight 320]



 gnocl::mainLoop
