#---------------
# install.tcl
#---------------
# Author(s):
#   Peter G Baum, 2003
#   William J Giddings, 2009
#---------------
# Description:
#   Install package using Tcl.
#   This will ensure installation into correct directory.
# Notes:
#   Ensure that pkgIndex.tcl is up to date. To ensure this,run
#   make pkgIndex.tcl.
#---------------

#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

#---------------
# lazy way to get preformated time and date
#---------------
proc date { {i date} } {
  switch -- $i {
    d&t   { set f "%c" }
    year  { set f "%Y" }
    week  { set f "%W" }
    day   { set f "%A" }
    month { set f "%B" }
    time  { set f "%H:%M:%S" }
    date  { set f "%d/%m/%y" }
    date4 { set f "%Y-%m-%d" }
    D4T24 { set f "%Y-%m-%d %T" }
  }
  return [clock format [clock seconds] -format "$f"]
}

# error checking
if { $argc != 4 } {
   set name [file tail $argv0]
   error "Wrong number of args.\nUsage: $name package version install/uninstall"
}

# get parameters
foreach {package version name what} $argv { break }

# get installation directory
set dir [info library]

# create sub-directory to receive package
set destDir [file join $dir $name$version]
switch -- $what {
   "install"   {
       if { [file exists $destDir] } {
           puts "$destDir exists already. Aborting installation."
           exit -1
           }
           puts "Create updated package file."
           set fp [open pkgIndex.tcl "w"]
           puts $fp "# Created: [date] [date time]"
           puts $fp {# Tcl package index file, version 1.1}
           puts $fp {# This file is sourced either when an application starts up or}
           puts $fp {# by a "package unknown" script.  It invokes the}
           puts $fp {# "package ifneeded" command to set up package-related}
           puts $fp {# information so that packages will be loaded automatically}
           puts $fp {# in response to "package require" commands.  When this}
           puts $fp {# script is sourced, the variable $dir must contain the}
           puts $fp {# full path name of this file's directory.}
           puts $fp {}
           puts $fp "package ifneeded $package $version \[list load \[file join \$dir $name.so\]\]"
           close $fp
           puts "Creating $destDir"
           file mkdir $destDir
           set files [glob *.so]
           lappend files pkgIndex.tcl
           foreach file $files {
               puts "Copying $file"
               file copy $file $destDir
               exec chmod 444 $file
               }
        }
   "uninstall" {
       puts "Deleting $destDir"
       file delete -force $destDir
       }
   default {
       error "unknown type \"$what\" must be install, test or uninstall"
       }
}

