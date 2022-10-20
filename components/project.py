#!/usr/bin/env python

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

import os
from genericpath import exists
from log import Log
import shutil
import pathlib
import library
import json

CODAL_URL = "https://github.com/lancaster-university/codal.git" # Hopefully this'll never change :)

BASE_ROOT = os.getcwd()
BOOTSTRAP_ROOT = pathlib.Path(__file__).parent.parent.absolute() # We're two levels deeper than we expect here...

def setRoot( root ):
  BASE_ROOT = root

def create_tree():
  path_list = [
    "libraries",
    "docs",
    "build",
    "source"
  ]
  for p in path_list:
    if not exists( os.path.join( BASE_ROOT, p ) ):
      os.mkdir( os.path.join( BASE_ROOT, p ) )
  
  if not exists( os.path.join( BASE_ROOT, ".gitignore" ) ):
    shutil.copy2(
      os.path.join( BOOTSTRAP_ROOT, "templates", "gitignore.template" ),
      os.path.join( BASE_ROOT, ".gitignore" )
    )

  if not exists( os.path.join( BASE_ROOT, "source", "main.cpp" ) ):
    shutil.copy2(
      os.path.join( BOOTSTRAP_ROOT, "templates", "main.cpp" ),
      os.path.join( BASE_ROOT, "source", "main.cpp" )
    )

def go_configure( info, config={} ):
  create_tree()
  library.clone( CODAL_URL, "codal", branch="feature/bootstrap" )

  # Copy out the base CMakeLists.txt, can't run from the library, and this is a CMake limitation
  # Note; use copy2 here to preserve metadata
  shutil.copy2(
    os.path.join( BASE_ROOT, "libraries", "codal", "CMakeLists.txt" ),
    os.path.join( BASE_ROOT, "CMakeLists.txt" )
  )

  Log.info( "Downloading target support files..." )
  details = library.clone( info["url"], info["name"], branch = info["branch"], specfile = "target.json" )

  # This is _somewhat_ redundant as cmake does this as well, but it might be worth doing anyway as there might be
  # additional library files needed for other, as-yet unidentified features. Plus, it makes the build faster afterwards -JV
  Log.info( "Downloading libraries..." )
  for lib in details["libraries"]:
    library.clone( lib["url"], lib["name"], branch = lib["branch"] )

  with open( os.path.join( BASE_ROOT, "codal.json" ), "w" ) as codal_json:
    config["target"] = info
    config["target"]["test_ignore"] = True
    config["target"]["dev"] = True

    json.dump( config, codal_json, indent=4 )
  
  print( "\n" )
  print( "All done! You can now start developing your code in the source/ folder. Running ./build.py will now defer to the actual build tools" )
  print( "Happy coding!" )
  print( "" )