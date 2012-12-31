#!/usr/bin/python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Read .DEPS.git and use the information to update git submodules"""

import os
import subprocess
import sys

from deps_utils import GetDepsContent


def CollateDeps(deps_content):
  """
  Take the output of deps_utils.GetDepsContent and return a hash of:

  { submod_name : [ submod_os, submod_url, submod_sha1 ], ... }
  """
  fixdep = lambda x: x[4:] if x.startswith('src/') else x
  spliturl = lambda x: list(x.partition('@')[0::2])
  submods = {}
  for (dep, url) in deps_content[0].iteritems():
    submods[fixdep(dep)] = ['all'] + spliturl(url)
  for (deps_os, val) in deps_content[1].iteritems():
    for (dep, url) in val.iteritems():
      submods[fixdep(dep)] = [deps_os] + spliturl(url)
  return submods


def WriteGitmodules(submods):
  """
  Take the output of CollateDeps, use it to write a .gitmodules file and
  add submodules to the git index.
  """
  fh = open('.gitmodules', 'w')
  for submod in sorted(submods.keys()):
    [submod_os, submod_url, submod_sha1] = submods[submod]
    print >>fh, '[submodule "%s"]' % submod
    print >>fh, '\tpath = %s' % submod
    print >>fh, '\turl = %s' % submod_url
    print >>fh, '\tos = %s' % submod_os
    if not submod_sha1:
      # We don't know what sha1 to register, so we have to infer it from the
      # submodule's origin/master.
      if not os.path.exists(os.path.join(submod, '.git')):
        # Not cloned yet
        subprocess.check_call(['git', 'clone', '-n', submod_url, submod])
      else:
        # Already cloned; let's fetch
        subprocess.check_call(['git', 'fetch', 'origin'], cwd=submod)
      sub = subprocess.Popen(['git', 'rev-list', 'origin/HEAD^!'],
                             cwd=submod, stdout=subprocess.PIPE)
      submod_sha1 = sub.communicate()[0].rstrip()
    subprocess.check_call(['git', 'update-index', '--add',
                           '--cacheinfo', '160000', submod_sha1, submod])
  fh.close()
  subprocess.check_call(['git', 'add', '.gitmodules'])


def RemoveObsoleteSubmodules():
  """
  Delete from the git repository any submodules which aren't in .gitmodules.
  """
  lsfiles_proc = subprocess.Popen(['git', 'ls-files', '-s'],
                                  stdout=subprocess.PIPE)
  grep_proc = subprocess.Popen(['grep', '^160000'],
                               stdin = lsfiles_proc.stdout,
                               stdout=subprocess.PIPE)
  (grep_out, _) = grep_proc.communicate()
  lsfiles_proc.communicate()
  with open(os.devnull, 'w') as nullpipe:
    for line in grep_out.splitlines():
      [_, _, _, path] = line.split()
      cmd = ['git', 'config', '-f', '.gitmodules',
             '--get-regexp', 'submodule\..*\.path', path]
      try:
        subprocess.check_call(cmd, stdout=nullpipe)
      except subprocess.CalledProcessError:
        subprocess.check_call(['git', 'update-index', '--force-remove', path])


def main():
  deps_file = sys.argv[1] if len(sys.argv) > 1 else '.DEPS.git'
  WriteGitmodules(CollateDeps(GetDepsContent(deps_file)))
  RemoveObsoleteSubmodules()
  return 0


if __name__ == '__main__':
  sys.exit(main())
