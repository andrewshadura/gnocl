# install-devlibs.tcl
#! /bin/sh
#\
exec tclsh "$0" "$@"

set devLibs {
	libglade2-devel
	libnotify-devel
	libcanberra-devel
	gtkimageview-devel
	gtksourceview-devel
	gtkspell-devel
	gnome-panel-devel
	libgnomecanvas-devel
	ImageMagick-devel
	libwebkitgtk-devel
	libpoppler-devel
	astyle
	doxygen
}

# options = install | remove | update
set opt install

foreach lib $devLibs {
	puts "installing $lib"
	eval exec "sudo zypper -n $opt $lib"
	}
	
