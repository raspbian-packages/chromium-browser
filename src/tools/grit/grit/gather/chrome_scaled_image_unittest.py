#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Unit tests for ChromeScaledImage.'''


import re
import unittest

from grit import exception
from grit import util
from grit.tool import build


_OUTFILETYPES = [
  ('.h', 'rc_header'),
  ('_map.cc', 'resource_map_source'),
  ('_map.h', 'resource_map_header'),
  ('.pak', 'data_package'),
  ('.rc', 'rc_all'),
]


def _GetFilesInPak(pakname):
  '''Get a list of the files that were actually included in the .pak output.
  '''
  data = util.ReadFile(pakname, util.BINARY)
  # Hackity hack...
  return [m.group(1) for m in re.finditer(r'CONTENTS_OF\((.*?)\)', data)]


def _GetFilesInRc(rcname):
  '''Get a list of the files that were actually included in the .rc output.
  '''
  data = util.ReadFile(rcname, util.BINARY)
  return [m.group(1) for m in re.finditer(ur'(?m)^\w+\s+BINDATA\s+"([^"]+)"$',
                                          data.decode('utf-16'))]


def _MakeFallbackAttr(fallback):
  if fallback is None:
    return ''
  else:
    return ' fallback_to_low_resolution="%s"' % ('false', 'true')[fallback]


def _Structures(fallback, *body):
  return '<structures%s>\n%s\n</structures>' % (
      _MakeFallbackAttr(fallback), '\n'.join(body))


def _Structure(name, file, fallback=None):
  return '<structure name="%s" file="%s" type="chrome_scaled_image"%s />' % (
      name, file, _MakeFallbackAttr(fallback))


def _If(expr, *body):
  return '<if expr="%s">\n%s\n</if>' % (expr, '\n'.join(body))


def _RunBuildTest(self, structures, pngfiles, contexts_and_results):
  outputs = '\n'.join('<output filename="out/%s%s" type="%s" context="%s" />'
                              % (context, ext, type, context)
                      for ext, type in _OUTFILETYPES
                      for context, expected_includes in contexts_and_results)
  infiles = {
    'in/in.grd': '''<?xml version="1.0" encoding="UTF-8"?>
      <grit latest_public_release="0" current_release="1">
        <outputs>
          %s
        </outputs>
        <release seq="1">
          %s
        </release>
      </grit>
      ''' % (outputs, structures),
  }
  for pngpath in pngfiles:
    infiles['in/' + pngpath] = 'CONTENTS_OF(%s)' % pngpath
  class Options(object):
    pass
  with util.TempDir(infiles) as tmp_dir:
    with tmp_dir.AsCurrentDir():
      options = Options()
      options.input = tmp_dir.GetPath('in/in.grd')
      options.verbose = False
      options.extra_verbose = False
      build.RcBuilder().Run(options, [])
    for context, expected_includes in contexts_and_results:
      self.assertEquals(_GetFilesInPak(tmp_dir.GetPath('out/%s.pak' % context)),
                        expected_includes)
      self.assertEquals(_GetFilesInRc(tmp_dir.GetPath('out/%s.rc' % context)),
                        [tmp_dir.GetPath('in/' + x) for x in expected_includes])


class ChromeScaledImageUnittest(unittest.TestCase):
  def testNormalFallback(self):
    _RunBuildTest(self,
        _Structures(None,
            _Structure('IDR_A', 'a.png'),
            _Structure('IDR_B', 'b.png'),
        ),
        ['default_123_percent/a.png',
         'tactile_123_percent/a.png',
         'default_123_percent/b.png',
        ],
        [('default_123_percent', ['default_123_percent/a.png',
                                  'default_123_percent/b.png']),
         ('tactile_123_percent', ['tactile_123_percent/a.png',
                                  'default_123_percent/b.png']),
        ])

  def testNormalFallbackFailure(self):
    self.assertRaises(exception.FileNotFound,
        _RunBuildTest, self,
            _Structures(None,
                _Structure('IDR_A', 'a.png'),
            ),
            ['default_100_percent/a.png',
             'tactile_100_percent/a.png',
            ],
            [('tactile_123_percent', ['should fail before using this']),
            ])

  def testLowresFallback(self):
    for outer in (None, False, True):
      for inner in (None, False, True):
        args = (
            self,
            _Structures(outer,
                _Structure('IDR_A', 'a.png', inner),
            ),
            ['default_100_percent/a.png',
            ],
            [('tactile_123_percent', ['default_100_percent/a.png']),
            ])
        if inner or (inner is None and outer):
          # should fall back to 100%
          _RunBuildTest(*args)
        else:
          # shouldn't fall back
          self.assertRaises(exception.FileNotFound, _RunBuildTest, *args)

    # Test fallback failure with fallback_to_low_resolution=True
    self.assertRaises(exception.FileNotFound,
        _RunBuildTest, self,
            _Structures(True,
                _Structure('IDR_A', 'a.png'),
            ),
            [],  # no files
            [('tactile_123_percent', ['should fail before using this']),
            ])
