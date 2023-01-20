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

from genericpath import exists
from importlib import import_module
from components.log import Log
import components.json_ops as json_ops
import components.net_tools as net_tools
import os
import subprocess

BASE_ROOT = os.getcwd()

def setRoot( root ):
  BASE_ROOT = root

def version( name ):
  try:
    return subprocess.check_output( "git rev-parse --short HEAD", cwd=os.path.join( BASE_ROOT, "libraries", name ), shell=True ).decode( 'utf-8' )
  except subprocess.CalledProcessError as err:
    Log.error( F"library_version error: {err}" )
    return "BAD-VERSION"
  except FileNotFoundError as err:
    Log.error( F"library_version file not found: {err}" )
    return "BAD-REPO"

def add( url, name=None, branch='Master' ):
  if not net_tools.is_url(url):
    Log.error( f"The string: '{url}' is not a correctly formatted URL" )
    exit( 1 )
  codalJson = json_ops.load( os.path.join( BASE_ROOT, "codal.json" ) )
  if 'libraries' not in codalJson:
    codalJson['libraries'] = []
  
  if name == None:
    name = url.split('/')[-1].split('.git')[0]
  git_root = os.path.join( BASE_ROOT, 'libraries', name )

  if any(x['name'] == name for x in codalJson['libraries']):
    Log.error( f'The library {name} already exists in codal.json.' )
    exit( 1 )

  Log.info( f'Adding library {name} to codal.json and downloading sources (if required)' )

  clone( url, name, branch, "library.json" )
  codalJson['libraries'].append( {
    "url": url,
    "name": name,
    "branch": branch,
    "type": "git"
  } )
  json_ops.save( os.path.join( BASE_ROOT, "codal.json" ), codalJson )

def remove( name ):
  codalJson = json_ops.load( os.path.join( BASE_ROOT, "codal.json" ) )
  if 'libraries' not in codalJson:
    codalJson['libraries'] = []
  codalJson['libraries'] = [x for x in codalJson['libraries'] if x['name'] != name]
  json_ops.save( os.path.join( BASE_ROOT, "codal.json" ), codalJson )


def clone( url, name, branch = "master", specfile = "module.json" ):
  Log.info( f'Downloading library {name}...' )
  git_root = os.path.join( BASE_ROOT, 'libraries', name )
  if not exists( os.path.join( git_root, '.git' ) ):
    os.system( f'git clone --recurse-submodules --branch "{branch}" "{url}" "{git_root}"' )

  if exists( os.path.join( git_root, specfile ) ):
    return json_ops.load( os.path.join( git_root, specfile ) )

  Log.info( f'WARN: Missing specification file for {name}: {specfile}' )
  return {}

def update( name, branch="", specfile = "module.json"):
  Log.info( f'Updating library {name}...' )
  git_root = os.path.join( BASE_ROOT, 'libraries', name )
  if not exists( git_root ):
    raise Exception( f'No such library {name}' )
  
  if branch != "":
    try:
      subprocess.run( f'git checkout {branch}', cwd=git_root, shell=True )
    except subprocess.CalledProcessError as err:
      raise Exception( f'No such branch {branch} for library {name}' )

  try:
    subprocess.run( "git pull", cwd=git_root, shell=True )
  except subprocess.CalledProcessError as err:
    raise Exception( 'Unable to pull changes for ${name}' )

  if exists( os.path.join( git_root, specfile ) ):
    return json_ops.load( os.path.join( git_root, specfile ) )

  Log.warn( f'WARN: Missing specification file for {name}: {specfile}' )
  return {}

def list():
  codalJson = json_ops.load( os.path.join( BASE_ROOT, "codal.json" ) )
  if 'libraries' not in codalJson:
    codalJson['libraries'] = []
  return codalJson['libraries']