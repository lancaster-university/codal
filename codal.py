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
    usage="usage: %prog [build|flash|library] [options]",
    description="A frontent to various tools to manage CODAL projects, see each section below for the tool details"
)

# Combine all our tool flags
parser.add_option_group( build_tool.create_opt_group( parser ) )
parser.add_option_group( flash_tool.create_opt_group( parser ) )
parser.add_option_group( library_tool.create_opt_group( parser ) )

# Parser'n'go
(options, args) = parser.parse_args()

validTools = [ "build", "flash", "library" ]

if len(args) < 1 or args[0].lower() not in validTools:
    parser.print_help()
    Log.error( f"No tool selected, please specify which action you want to perform ({', '.join(validTools)})" )
    if len(args) > 0:
        Log.error( f"Undefined tool: {args[0].lower()}" )
    exit( 0 )

# Grab and drop the first arg (our tool directive)
tool = args.pop(0).lower();

if tool == "build":
    build_tool.run_tool( options, args )
    exit( 0 )

if tool == "flash":
    flash_tool.run_tool( options, args )
    exit( 0 )

if tool == "library":
    library_tool.run_tool( options, args )
    exit( 0 )
