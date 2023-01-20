#!/usr/bin/env python3

# The MIT License (MIT)

# Copyright (c) 2022 Lancaster University.

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

from components.log import Log
from importlib import import_module
from sys import path
import os
from optparse import OptionParser, OptionGroup

BASE_ROOT = os.getcwd()
LIBRARIES_ROOT = os.path.join( BASE_ROOT, 'libraries' )
CODAL_ROOT = os.path.join( BASE_ROOT, 'libraries', 'codal' )

# Add our libraries to the module search path
path.insert( 0, LIBRARIES_ROOT )

# Load all the tools:
import codal.buildtool as build_tool
import codal.flashtool as flash_tool
import codal.librarytool as library_tool

parser = OptionParser(
    usage="usage: %prog build|flash|status|library target-name-or-url [options]",
    description="This script manages the build system for a codal device. Passing a target-name generates a codal.json for that devices, to list all devices available specify the target-name as 'ls'."
)

# Combine all our tool flags
buildGroup = OptionGroup( parser, "BUILD Flags", "Flags and options applicable to the BUILD tool" )
build_tool.add_flags( buildGroup )
parser.add_option_group( buildGroup )

flashGroup = OptionGroup( parser, "FLASH Flags", "Flags and options applicable to the FLASH tool" )
flash_tool.add_flags( flashGroup )
parser.add_option_group( flashGroup )

libraryGroup = OptionGroup( parser, "LIBRARY Flags", "Flags and options applicable to the LIBRARY tool" )
library_tool.add_flags( libraryGroup )
parser.add_option_group( libraryGroup )

# Parser'n'go
(options, args) = parser.parse_args()

if len(args) < 1:
    Log.error( "No tool selected, please specify which action you want to perform (build, flash, status)" )
    exit( 0 )

# Grab and drop the first arg (our tool directive)
tool = args.pop(0).lower();

if tool == "build":
    build_tool.run_tool( options, args )
    exit( 0 )

if tool == "status":
    Log.warn( "Unimplemented tool, sorry :(" )
    exit( 0 )

if tool == "flash":
    flash_tool.run_tool( options, args )
    exit( 0 )

if tool == "library":
    library_tool.run_tool( options, args )
    exit( 0 )
