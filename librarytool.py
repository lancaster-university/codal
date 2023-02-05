#!/usr/bin/env python

# The MIT License (MIT)

# Copyright (c) 2017 Lancaster University.

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

from optparse import OptionParser, OptionGroup
from components.log import Log
from components.net_tools import is_url
import components.library as library

def run_tool( options, args ):
    if len(args) < 1:
        Log.warn( "This tool requires at least one operation add/remove/status" )
        get_opt_parser().print_help()
        exit( 1 )
    
    op = args.pop(0).lower()
    if op == "add":
        while( len(args) > 0 ):
            url = args.pop(0)
            Log.info( f"Adding library {url}..." )
            if not is_url( url ):
                Log.error( f'The path "{url}" is not a valid URL. STOP' )
                exit( 1 )
            
            library.add( url )
        exit( 0 );
    
    if op == "remove":
        while( len(args) > 0 ):
            libName = args.pop(0)
            Log.info( f"Removing library {libName}..." )
            library.remove( libName )
        exit( 0 );
    
    if op == "update":
        Log.info( "Updating configured libraries..." )
        libs = args
        print( options )
        if options.all_libraries:
            libs = []
            for libDef in library.list():
                libs.append( libDef['name'] )
            
        for libName in libs:
            library.update( libName )
        
        exit( 0 )
    
    if op == "list" or op == "ls":
        Log.info( "Configured libraries:" )
        for lib in library.list():
            Log.info( f"\t{lib['name']}, ({lib['url']}, branch = {lib['branch']}) - {library.version(lib['name'])}" )
        exit( 0 );
    
    exit( 1 )

def get_opt_parser():
    parser = OptionParser(
        usage="%prog library [add|remove|update|list|ls]",
        description="Modify application libraries for this CODAL project"
    )
    parser.add_option( '--all', dest='all_libraries', action='store_true', help='Attempt to apply this operation to all configured libraries (if applicable)', default=False )
    parser.add_option( '--branch', dest='branch', action='store', help="Select a branch for this library (valid only for 'add' and 'update' operations)" )
    return parser

if __name__ == "__main__":
    parser = get_opt_parser()
    (options, args) = parser.parse_args()
    run_tool( options, args )
    exit( 0 )