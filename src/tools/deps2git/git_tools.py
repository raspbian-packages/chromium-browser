#!/usr/bin/python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import re
import subprocess


# Show more information about the commands being executed.
VERBOSE = False


def GetStatusOutput(cmd):
  """Return (status, output) of executing cmd in a shell."""
  if VERBOSE:
    print ''
    print '[DEBUG] Running "%s"' % cmd
  proc = subprocess.Popen(cmd, shell=True, universal_newlines=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  output = ''.join(proc.stdout.readlines())
  status = proc.wait()
  if status is None:
    status = 0

  if VERBOSE:
    short_output = ' '.join(output.splitlines())
    short_output = short_output.strip(' \t\n\r')
    print '[DEBUG] Output: %d, %-60s' % (status, short_output)

  return (status, output)


def Git(git_repo, command):
  """Execute a git command within a local git repo."""
  cmd = 'git --git-dir=%s %s' % (git_repo, command)
  (status, output) = GetStatusOutput(cmd)
  if status != 0:
    raise Exception('Failed to run %s. error %d. output %s' % (cmd, status,
                                                               output))
  return (status, output)


def Clone(git_url, git_repo):
  """Clone a repository."""
  Git(git_repo, 'clone --mirror %s %s' % (git_url, git_repo))


def Fetch(git_repo):
  """Fetch the latest objects for a given git repository."""
  Git(git_repo, 'fetch')


def Ping(git_repo):
  """Confirm that a remote repository URL is valid."""
  status, output = GetStatusOutput('git ls-remote ' + git_repo)
  return status == 0


def CreateLessThanOrEqualRegex(number):
  """ Return a regular expression to test whether an integer less than or equal
      to 'number' is present in a given string.
  """

  # In three parts, build a regular expression that match any numbers smaller
  # than 'number'.
  # For example, 78656 would give a regular expression that looks like:
  # Part 1
  # (78356|                        # 78356
  # Part 2
  #  7835[0-5]|                    # 78350-78355
  #  783[0-4][0-9]|                # 78300-78349
  #  78[0-2][0-9][0-9]|            # 78000-78299
  #  7[0-7][0-9][0-9][0-9]|        # 70000-77999
  #  [0-6][0-9][0-9][0-9][0-9]|    # 10000-69999
  # Part 3
  #  [0-9][0-9][0-9][0-9]|         # 1000-9999
  #  [0-9][0-9][0-9]|              # 100-999
  #  [0-9][0-9]|                   # 10-99
  #  [0-9])                        # 0-9

  # Part 1: Create an array with all the regexes, as described above.
  # Prepopulate it with the number itself.
  number = str(number)
  expressions = [number]

  # Convert the number to a list, so we can translate digits in it to
  # expressions.
  num_list = list(number)
  num_len = len(num_list)

  # Part 2: Go through all the digits in the number, starting from the end.
  # Each iteration appends a line to 'expressions'.
  for index in range (num_len - 1, -1, -1):
    # Convert this digit back to an integer.
    digit = int(num_list[index])

    # Part 2.1: No processing if this digit is a zero.
    if digit == 0:
      continue

    # Part 2.2: We switch the current digit X by a range "[0-(X-1)]".
    num_list[index] = '[0-%d]' % (digit - 1)

    # Part 2.3: We set all following digits to be "[0-9]".
    # Since we just decrementented a digit in a most important position, all
    # following digits don't matter. The possible numbers will always be smaller
    # than before we decremented.
    for next_digit in range(index + 1, num_len):
      num_list[next_digit] = '[0-9]'

    # Part 2.4: Add this new sub-expression to the list.
    expressions.append(''.join(num_list))

  # Part 3: We add all the full ranges to match all numbers that are at least
  # one order of magnitude smaller than the original numbers.
  for index in range(1, num_len):
    expressions.append('[0-9]'*index)

  # All done. We now have our final regular expression.
  regex = '(%s)' % ('|'.join(expressions))
  return regex


def Search(git_repo, svn_rev):
  """Return the Git commit id matching the given SVN revision."""
  regex = CreateLessThanOrEqualRegex(svn_rev)
  (status, output) = Git(git_repo, ('log -E --grep=".*git-svn-id:.*@%s " '
                                    '-1 --format=%%H FETCH_HEAD') % regex)
  if output != '':
    output = output.splitlines()[0]

  print '%s: %s <-> %s' % (git_repo, output, svn_rev)
  if re.match('^[0-9a-fA-F]{40}$', output):
    return output
  raise Exception('Cannot find revision %s in %s' % (svn_rev, git_repo))
