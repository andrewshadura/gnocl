#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl

set stockItems [lsort [gnocl::info allStockItems] ]

set lst [gnocl::list \
    -titles {"icon" "Stock Item"} \
    -types {image string}]

foreach {item} $stockItems {
	# trap non-existing icons
	if { [catch { $lst add [list [list %#$item $item ]] }] } {
		$lst cellConfigure end 1 -value $item -visible 1
		}
	}

gnocl::window -child $lst -widthRequest 225 -heightRequest 400

