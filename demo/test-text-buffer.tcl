#---------------
# test-text-buffer.tcl
#---------------
# William J Giddings
# 16/Jan/2010
#---------------
#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
#---------------

package require Gnocl

set box1 [gnocl::box -orientation vertical]
set box2 [gnocl::box ]
$box1 add $box2

set txt [gnocl::text \
            -baseFont {Sans 14} \
            -editable no \
            -text "Come to me in the silence of the night\nChristina Rossetti (1830-1894)" \
            -scrollbar always]

foreach {i j} {0 Title 1 Verse-1 2 Verse-2  3 Verse-3} {
    set rad($i) [gnocl::radioButton \
        -text $j \
        -variable buff \
        -onValue $i]
    $rad($i) configure -onToggled { $txt configure -buffer $tbuf($buff) }
    $box2 add [set rad($i)]
}

$box1 add $txt -fill {1 1} -expand 1

gnocl::window -child $box1 -defaultWidth 480 -defaultHeight 240 -onDestroy {exit}

set verse(0) [$txt get start end]

set verse(1) {Come to me in the silence of the night;
   Come to me in the speaking silence of a dream;
Come with soft rounded cheeks and eyes as bright
   As sunlight on a stream;
      Come back in tears,
O memory, hope, love of finished years.}

set verse(2) {O dream how sweet, too sweet, too bitter sweet,
   Whose wakening should have been in Paradise,
Where souls brimfull of love abide and meet;
   Where thirsting longing eyes
      Watch the slow door
That opening, letting in, lets out no more.}

set verse(3) {Yet come to me in dreams that I may live
   My very life again cold in death:
Come back to me in dreams, that I may give
   Pulse for pulse, breath for breath:
      Speak low, lean low,
As long ago. my love, how long ago.}

set tbuf(0) [gnocl::textBuffer -text $verse(0)]
set tbuf(1) [gnocl::textBuffer -text $verse(1)]
set tbuf(2) [gnocl::textBuffer -text $verse(2)]
set tbuf(3) [gnocl::textBuffer -text $verse(3)]

gnocl::mainLoop
