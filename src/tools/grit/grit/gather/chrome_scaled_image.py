#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Gatherer for <structure type="chrome_scaled_image">.
'''

import os

from grit import exception
from grit import lazy_re
from grit import util
from grit.gather import interface


def _RescaleImage(data, from_scale, to_scale):
  # TODO(benrg): Actually rescale the image.
  return data


def _MakeBraceGlob(strings):
  '''Given ['foo', 'bar'], return '{foo,bar}', for error reporting.
  '''
  if len(strings) == 1:
    return strings[0]
  else:
    return '{' + ','.join(strings) + '}'


class ChromeScaledImage(interface.GathererBase):
  '''Represents an image that exists in multiple layout variants
  (e.g. "default", "touch") and multiple scale variants
  (e.g. "100_percent", "200_percent").
  '''

  split_context_re_ = lazy_re.compile(r'(.+)_(\d+)_percent\Z')

  def _FindInputFile(self):
    output_context = self.grd_node.GetRoot().output_context
    match = self.split_context_re_.match(output_context)
    if not match:
      raise exception.MissingMandatoryAttribute(
          'All <output> nodes must have an appropriate context attribute'
          ' (e.g. context="touch_200_percent")')
    req_layout, req_scale = match.group(1), int(match.group(2))

    layouts = [req_layout]
    if 'default' not in layouts:
      layouts.append('default')
    scales = [req_scale]
    try_low_res = self.grd_node.FindBooleanAttribute(
        'fallback_to_low_resolution', default=False, skip_self=False)
    if try_low_res and 100 not in scales:
      scales.append(100)
    for layout in layouts:
      for scale in scales:
        dir = '%s_%s_percent' % (layout, scale)
        path = os.path.join(dir, self.rc_file)
        if os.path.exists(self.grd_node.ToRealPath(path)):
          return path, scale, req_scale
    # If we get here then the file is missing, so fail.
    dir = "%s_%s_percent" % (_MakeBraceGlob(layouts),
                             _MakeBraceGlob(map(str, scales)))
    raise exception.FileNotFound(
        'Tried ' + self.grd_node.ToRealPath(os.path.join(dir, self.rc_file)))

  def GetInputPath(self):
    path, scale, req_scale = self._FindInputFile()
    return path

  def Parse(self):
    pass

  def GetTextualIds(self):
    return [self.extkey]

  def GetData(self, *args):
    path, scale, req_scale = self._FindInputFile()
    data = util.ReadFile(self.grd_node.ToRealPath(path), util.BINARY)
    return _RescaleImage(data, scale, req_scale)

  def Translate(self, *args, **kwargs):
    return self.GetData()
