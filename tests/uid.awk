#!/bin/gawk
#
# Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BEGIN {
  r_uint = "(0|[1-9][0-9]*)"
  regexp = "^getx?uid" suffix "\\(\\)[[:space:]]+= " r_uint "$"
  expected = "getuid"
  fail = 0
}

regexp == "" {
  fail = 1
  next
}

{
  if (match($0, regexp, a)) {
    if (expected == "getuid") {
      uid = a[1]
      expected = "setuid"
      regexp = "^setuid" suffix "\\(" uid "\\)[[:space:]]+= 0$"
    } else if (expected == "setuid") {
      expected = "getresuid"
      regexp = "^getresuid" suffix "\\(\\[" uid "\\], \\[" uid "\\], \\[" uid "\\]\\)[[:space:]]+= 0$"
    } else if (expected == "getresuid") {
      expected = "setreuid"
      regexp = "^setreuid" suffix "\\(-1, -1\\)[[:space:]]+= 0$"
    } else if (expected == "setreuid") {
      expected = "setresuid"
      regexp = "^setresuid" suffix "\\(" uid ", -1, -1\\)[[:space:]]+= 0$"
    } else if (expected == "setresuid") {
      expected = "fchown"
      regexp = "^fchown" suffix "\\(1, -1, -1\\)[[:space:]]+= 0$"
    } else if (expected == "fchown") {
      expected = "1st getgroups"
      regexp = "^getgroups" suffix "\\(0, NULL\\)[[:space:]]+= " r_uint "$"
    } else if (expected == "1st getgroups") {
      ngroups = a[1]
      if (ngroups == "0")
        list=""
      else if (ngroups == "1")
        list=r_uint
      else
        list=r_uint "(, " r_uint "){" (ngroups - 1) "}"
      expected = "2nd getgroups"
      regexp = "^getgroups" suffix "\\(" ngroups ", \\[" list "\\]\\)[[:space:]]+= " ngroups "$"
    } else if (expected == "2nd getgroups") {
      expected = "the last line"
      regexp = "^\\+\\+\\+ exited with 0 \\+\\+\\+$"
    } else if (expected == "the last line") {
      expected = "nothing"
      regexp = ""
    }
  }
}

END {
  if (fail) {
    print "Unexpected output after exit"
    exit 1
  }
  if (regexp == "")
    exit 0
  print "error: " expected " doesn't match"
  exit 1
}
