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
from sys import path, argv
import os
from io import StringIO
from optparse import OptionParser, OptionGroup, HelpFormatter

BASE_ROOT = os.getcwd()
LIBRARIES_ROOT = os.path.join( BASE_ROOT, 'libraries' )
CODAL_ROOT = os.path.join( BASE_ROOT, 'libraries', 'codal' )

# Add our libraries to the module search path
path.insert( 0, LIBRARIES_ROOT )

# Load all the tools:
import codal.buildtool as build_tool
import codal.flashtool as flash_tool
import codal.librarytool as library_tool
import codal.verifytool as verify_tool

TOOLBOX = {
    "build":   build_tool,
    "library": library_tool,
    "flash":   flash_tool,
    "verify":  verify_tool
}

def getTagVersion():
    wd = os.getcwd()
    os.chdir( CODAL_ROOT )
    version = os.popen('git describe --tags --abbrev=0').read().strip()
    os.chdir(wd)
    return version

parser = OptionParser(
    usage="usage: %prog [build|flash|library] [options]",
    description="A frontent to various tools to manage CODAL projects.",
    add_help_option=False,
    version=getTagVersion()
)
parser.add_option( '-h', '--help', dest="help", action="store_true", help="", default=False )

def printGlobalHelp():
    parser.print_help()
    Log.info( "\nAvailable tools:" )
    for tool in TOOLBOX.keys():
        Log.info( f"  $> ./codal.py {tool}" )
    Log.info( "\nUse '-h' on each command to see its arguments.\nFor example:\n  $> ./codal.py build -h" )

if len(argv[1:2]) > 0 and argv[1:2][0].lower() not in TOOLBOX.keys():
    (options, args) = parser.parse_args()
    if options.help and (argv[1:2][0].lower() == '-h' or argv[1:2][0].lower() == '--help'):
        printGlobalHelp()
        exit( 0 )

if len(argv[1:2]) < 1:
    Log.warn( "codal.py requires at least 1 argument" )
    printGlobalHelp()
    exit( 0 )

# Grab and drop the first arg (our tool directive)
tool = argv[1:2][0].lower()

if tool not in TOOLBOX.keys():
    Log.warn( f"No such tool: {tool}" )
    printGlobalHelp()
    exit( 0 )

toolParser = TOOLBOX.get( tool ).get_opt_parser()
(options, args) = toolParser.parse_args( argv[2:] )
TOOLBOX.get( tool ).run_tool( options, args )
exit( 0 )