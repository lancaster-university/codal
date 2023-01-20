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

from imp import load_package
import json
from ntpath import join
import os
import sys
import shutil
import pathlib
import subprocess
import urllib.request
import urllib.parse
import optparse
from importlib import import_module
from genericpath import exists
from passthroughoptparser import PassThroughOptionParser
from components.log import Log

CODAL_URL = "https://github.com/lancaster-university/codal.git"
BASE_ROOT = os.getcwd()
BOOTSTRAP_ROOT = pathlib.Path(__file__).parent.absolute()




def go_build():
  sys.path.append( os.path.join(BASE_ROOT, "libraries", "codal") )
  import_module( f'libraries.codal.build' )

def go_build_docs():
  config = {
    "PROJECT_NAME": "Codal Project",
    "PROJECT_NUMBER": "",
    "PROJECT_BRIEF": "",
    "PROJECT_LOGO": "",
    "INPUT": [ "source" ]
  }
  if( exists( os.path.join( BASE_ROOT, "docs.json" ) ) ):
    Log.info( "Merging user config from docs.json" )
    userConfig = load_json( os.path.join(BASE_ROOT, "docs.json") )
    config = merge_json( config, userConfig )
  
  def quoteString( _in ):
    return '"' + str(_in) + '"'
  
  for lib in os.listdir( os.path.join(BASE_ROOT, "libraries") ):
    libdef = os.path.join(BASE_ROOT, "libraries", lib, "library.json")
    if exists( libdef ):
      Log.info( F"Including {lib} documentation..." )
      libspec = load_json( libdef )
      if "docs" in libspec and "INPUT" in libspec["docs"]:
        print( config["INPUT"] )
        for inc in libspec["docs"]["INPUT"]:
          config["INPUT"].append( os.path.join(BASE_ROOT, "libraries", lib, inc ) )
  
  for key in set(config):
    if type(config[key]) == list:
      config[key] = map( quoteString, config[key] )
      config[key] = " \\\n                         ".join( config[key] )
    else:
      config[key] = '"' +config[key]+ '"'

  with open( os.path.join( BOOTSTRAP_ROOT, "templates", "Doxyfile.template" ), 'r' ) as template:
    with open( os.path.join( BASE_ROOT, "docs", "Doxyfile" ), 'w' ) as output:
      for line in template.readlines():
        for key in set(config):
          line = line.replace( "{{" +key+ "}}", config[key] )
        output.write( line );

  Log.info( "Grabbing the default theme (if not present)" )
  if not exists( os.path.join(BASE_ROOT, "docs", "theme") ):
    Log.info( "Downloading the default theme - to inhibit this behaviour, create your own theme/ path inside docs/" )
    os.system( F'git clone -b v2.0.2 https://github.com/jothepro/doxygen-awesome-css.git "{os.path.join(BASE_ROOT, "docs", "theme")}"' )
  
  Log.info( "Building with doxygen..." )
  os.system( F'doxygen "{os.path.join( BASE_ROOT, "docs", "Doxyfile" )}"' )

def go_bootstrap( target_list ):
  if exists( os.path.join(BASE_ROOT, "codal.json") ) and exists( os.path.join(BASE_ROOT, "libraries", "codal", "build.py") ):
    parser = PassThroughOptionParser(add_help_option=False)
    parser.add_option( '--bootstrap', dest='force_bootstrap', action="store_true", default=False )
    parser.add_option( '--library-add', dest='library_add', action='store_true', default=False )
    parser.add_option( '--library-remove', dest='library_remove', action='store_true', default=False )
    (options, args) = parser.parse_args()

    if options.force_bootstrap:
      Log.warn( "WARNING: '--bootstrap' forces bootstrap to take over, downloaded build tools will not be run!" )
    
    if options.library_add:
      while len(args) > 0:
        library_add( args.pop() )
      exit( 0 )
    
    if options.library_remove:
      while len(args) > 0:
        library_remove( args.pop() )
      exit( 0 )

    if not options.force_bootstrap:
      go_build()
      exit(0)

  parser = optparse.OptionParser(usage="usage: %prog target-name [options]", description="BOOTSTRAP MODE - Configures the current project directory for a specified target. Will defer to the latest build tools once configured.")
  parser.add_option('--bootstrap', dest='force_bootstrap', action='store_true', help="Skips any already downloaded build toolchain, and runs in bootstrap mode directly.", default=False)
  parser.add_option('--ignore-codal', dest='ignore_codal', action='store_true', help="Skips any pre-existing codal.json in the project folder, and runs as if none exists.", default=False)
  parser.add_option('--merge-upstream-target', dest='merge_upstream_target', action='store_true', help="Keeps the existing codal.json, but only for non-target parameters, merging the new target definition in with the old arguments.", default=False)
  parser.add_option('--makedocs', dest='makedocs', action='store_true', help='Builds documentation (including supported libraries) with doxygen')
  parser.add_option('-u', dest='update', action='store_true', help="Update this file and the build tools library", default=False)
  (options, args) = parser.parse_args()

  if options.makedocs:
    go_build_docs()
    exit(0)

  if options.update:
    Log.info( "Attempting to automatically update bootstrap..." )
    old_vers = library_version( 'codal-bootstrap' )
    if exists(os.path.join( BASE_ROOT, "libraries", "codal-bootstrap" )):
      library_update( "codal-bootstrap" )
    else:
      library_clone( "https://github.com/lancaster-university/codal-bootstrap.git", "codal-bootstrap", branch="main" )
    vers = library_version( 'codal-bootstrap' )
    
    if vers == old_vers:
      Log.info( "Nothing to update, codal-bootstrap is already the latest version" )
      exit( 0 )

    Log.info( "Downloaded a new version of bootstrap, updating the project files..." )
    shutil.copy2(
      os.path.join( BASE_ROOT, "libraries", "codal-bootstrap", "build.py" ),
      os.path.join( BASE_ROOT, "build.py" )
    )

    Log.info( "Done!\n" )

    exit(0)

  if len(args) == 0:
    # We might have an existing device config already, so grab that and try and pull that...
    if exists( os.path.join( BASE_ROOT, "codal.json" ) ) and not options.ignore_codal:
      Log.info( "Project already has a codal.json, trying to use that to determine the build system and any missing dependencies..." )
      local_config = load_json( os.path.join( BASE_ROOT, "codal.json" ) )
      local_target = local_config["target"]

      print( "Configuring from codal.json!" )
      go_configure( local_target )

      # Jump to an actual build stage immediately, as older script-drivers will expect it
      go_build()
      exit(0)

    Log.warn( "Please supply an initial target to build against:" )
    list_valid_targets( target_list )
    exit( 0 )

  if len(args) == 1:

    # 'Magic' target to list all targets
    if args[0] == "ls":
      Log.info( "Available target platforms:" )
      list_valid_targets( target_list )
      exit( 0 )
    
    targets = download_targets( target_list )
    query = args[0]

    if query not in targets:
      Log.error( "Invalid or unknown target, try './build.py ls' to see available targets" )
      exit( 1 )

    local_config = {}
    if options.merge_upstream_target:
      Log.info( "Preserving local configuration, but ignoring the target and using supplied user target..." )
      local_config = load_json( os.path.join( BASE_ROOT, "codal.json" ) )

    go_configure( targets[query], config=local_config )