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

import json

def load( path ):
  with open(path, 'r') as src:
    return json.load( src )

def save( path, data ):
  # Check is this is possible to serialise...
  try:
    json.dumps(data)
  except:
    return False
  
  # If we make it here, go for it
  with open(path, 'w') as dst:
    json.dump(data, dst, ensure_ascii=False, indent=4)
  
  return True;

def merge(base_obj, delta_obj):
  if not isinstance(base_obj, dict):
    return delta_obj
  common_keys = set(base_obj).intersection(delta_obj)
  new_keys = set(delta_obj).difference(common_keys)
  for k in common_keys:
    base_obj[k] = merge(base_obj[k], delta_obj[k])
  for k in new_keys:
    base_obj[k] = delta_obj[k]
  return base_obj