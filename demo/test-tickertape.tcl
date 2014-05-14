#!/usr/bin/sh
#\
exec tclsh "$0" "$@"
package require Gnocl


gnocl::window -child [gnocl::tickerTape \
	-baseFont {Sans 12 } \
	-baseColor "#FFFFFF" \
	-background white \
	-speed 10 \
	-message "Gnocl 0.9.95 * Gnocl 0.9.95 * Gnocl 0.9.95 * Gnocl 0.9.95 *"]
