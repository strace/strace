Summary: Tracks and displays system calls associated with a running process.
Name: strace
Version: 4.4.98
Release: 1
License: BSD
Group: Development/Debuggers
URL: http://sourceforge.net/projects/strace/
Source0: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-root

%description
The strace program intercepts and records the system calls called and
received by a running process.  Strace can print a record of each
system call, its arguments and its return value.  Strace is useful for
diagnosing problems and debugging, as well as for instructional
purposes.

Install strace if you need a tool to track the system calls made and
received by a process.

%prep
%setup -q

%build
%configure
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_mandir}/man1
mkdir -p %{buildroot}%{_bindir}
%makeinstall man1dir=%{buildroot}%{_mandir}/man1

# remove unpackaged files from the buildroot
rm -f %{buildroot}%{_bindir}/strace-graph

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/strace
%{_mandir}/man1/*

%changelog
* Mon Jun 10 2003 Roland McGrath <roland@redhat.com> 4.4.98-1
- new upstream version, more fixes (#90754, #91085)

* Wed Jun 04 2003 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Sun Mar 30 2003 Roland McGrath <roland@redhat.com> 4.4.96-1
- new upstream version, handles yet more 2.5 syscalls, x86_64 & ia64 fixes

* Mon Feb 24 2003 Elliot Lee <sopwith@redhat.com> 4.4.95-2
- rebuilt

* Mon Feb 24 2003 Roland McGrath <roland@redhat.com> 4.4.95-1
- new upstream version, fixed getresuid/getresgid (#84959)

* Wed Feb 19 2003 Roland McGrath <roland@redhat.com> 4.4.94-1
- new upstream version, new option -E to set environment variables (#82392)

* Wed Jan 22 2003 Tim Powers <timp@redhat.com> 4.4.93-2
- rebuilt

* Tue Jan 21 2003 Roland McGrath <roland@redhat.com> 4.4.93-1
- new upstream version, fixes ppc and s390 bugs, adds missing ptrace requests

* Fri Jan 10 2003 Roland McGrath <roland@redhat.com> 4.4.91-1
- new upstream version, fixes -f on x86-64

* Fri Jan 10 2003 Roland McGrath <roland@redhat.com> 4.4.90-1
- new upstream version, fixes all known bugs modulo ia64 and s390 issues

* Fri Jan 03 2003 Florian La Roche <Florian.LaRoche@redhat.de> 4.4-11
- add further s390 patch from IBM

* Wed Nov 27 2002 Tim Powers <timp@redhat.com> 4.4-10
- remove unpackaged files from the buildroot

* Mon Oct 07 2002 Phil Knirsch <pknirsch@redhat.com> 4.4-9.1
- Added latest s390(x) patch.

* Fri Sep 06 2002 Karsten Hopp <karsten@redhat.de> 4.4-9
- preliminary x86_64 support with an ugly patch to help
  debugging. Needs cleanup!

* Mon Sep  2 2002 Jakub Jelinek <jakub@redhat.com> 4.4-8
- newer version of the clone fixing patch (Roland McGrath)
- aio syscalls for i386/ia64/ppc (Ben LaHaise)

* Wed Aug 28 2002 Jakub Jelinek <jakub@redhat.com> 4.4-7
- fix strace -f (Roland McGrath, #68994)
- handle ?et_thread_area, SA_RESTORER (Ulrich Drepper)

* Fri Jun 21 2002 Jakub Jelinek <jakub@redhat.com> 4.4-6
- handle futexes, *xattr, sendfile64, etc. (Ulrich Drepper)
- handle modify_ldt (#66894)

* Thu May 23 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Tue Apr 16 2002 Jakub Jelinek <jakub@redhat.com> 4.4-4
- fix for the last patch by Jeff Law (#62591)

* Mon Mar  4 2002 Preston Brown <pbrown@redhat.com> 4.4-3
- integrate patch from Jeff Law to eliminate hang tracing threads

* Sat Feb 23 2002 Florian La Roche <Florian.LaRoche@redhat.de>
- minor update from debian tar-ball

* Wed Jan 02 2002 Florian La Roche <Florian.LaRoche@redhat.de>
- update to 4.4

* Sun Jul 22 2001 Florian La Roche <Florian.LaRoche@redhat.de>
- disable s390 patches, they are already included

* Wed Jul 18 2001 Preston Brown <pbrown@redhat.com> 4.3-1
- new upstream version.  Seems to have integrated most new syscalls
- tracing threaded programs is now functional.

* Mon Jun 11 2001 Than Ngo <than@redhat.com>
- port s390 patches from IBM

* Wed May 16 2001 Nalin Dahyabhai <nalin@redhat.com>
- modify new syscall patch to allocate enough heap space in setgroups32()

* Wed Feb 14 2001 Jakub Jelinek <jakub@redhat.com>
- #include <time.h> in addition to <sys/time.h>

* Fri Jan 26 2001 Karsten Hopp <karsten@redhat.com>
- clean up conflicting patches. This happened only
  when building on S390

* Fri Jan 19 2001 Bill Nottingham <notting@redhat.com>
- update to CVS, reintegrate ia64 support

* Sat Dec  8 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Get S/390 support into the normal package

* Sat Nov 18 2000 Florian La Roche <Florian.LaRoche@redhat.de>
- added S/390 patch from IBM, adapting it to not conflict with
  IA64 patch

* Sat Aug 19 2000 Jakub Jelinek <jakub@redhat.com>
- doh, actually apply the 2.4 syscalls patch
- make it compile with 2.4.0-test7-pre4+ headers, add
  getdents64 and fcntl64

* Thu Aug  3 2000 Jakub Jelinek <jakub@redhat.com>
- add a bunch of new 2.4 syscalls (#14036)

* Wed Jul 12 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild
- excludearch ia64

* Fri Jun  2 2000 Matt Wilson <msw@redhat.com>
- use buildinstall for FHS

* Wed May 24 2000 Jakub Jelinek <jakub@redhat.com>
- make things compile on sparc
- fix sigreturn on sparc

* Fri Mar 31 2000 Bill Nottingham <notting@redhat.com>
- fix stat64 misdef (#10485)

* Tue Mar 21 2000 Michael K. Johnson <johnsonm@redhat.com>
- added ia64 patch

* Thu Feb 03 2000 Cristian Gafton <gafton@redhat.com>
- man pages are compressed
- version 4.2 (why are we keeping all these patches around?)

* Sat Nov 27 1999 Jeff Johnson <jbj@redhat.com>
- update to 4.1 (with sparc socketcall patch).

* Fri Nov 12 1999 Jakub Jelinek <jakub@redhat.com>
- fix socketcall on sparc.

* Thu Sep 02 1999 Cristian Gafton <gafton@redhat.com>
- fix KERN_SECURELVL compile problem

* Tue Aug 31 1999 Cristian Gafton <gafton@redhat.com>
- added alpha patch from HJLu to fix the osf_sigprocmask interpretation

* Sat Jun 12 1999 Jeff Johnson <jbj@redhat.com>
- update to 3.99.1.

* Wed Jun  2 1999 Jeff Johnson <jbj@redhat.com>
- add (the other :-) jj's sparc patch.

* Wed May 26 1999 Jeff Johnson <jbj@redhat.com>
- upgrade to 3.99 in order to
-    add new 2.2.x open flags (#2955).
-    add new 2.2.x syscalls (#2866).
- strace 3.1 patches carried along for now.

* Sun May 16 1999 Jeff Johnson <jbj@redhat.com>
- don't rely on (broken!) rpm %patch (#2735)

* Tue Apr 06 1999 Preston Brown <pbrown@redhat.com>
- strip binary

* Sun Mar 21 1999 Cristian Gafton <gafton@redhat.com>
- auto rebuild in the new build environment (release 16)

* Tue Feb  9 1999 Jeff Johnson <jbj@redhat.com>
- vfork est arrive!

* Tue Feb  9 1999 Christopher Blizzard <blizzard@redhat.com>
- Add patch to follow clone() syscalls, too.

* Sun Jan 17 1999 Jeff Johnson <jbj@redhat.com>
- patch to build alpha/sparc with glibc 2.1.

* Thu Dec 03 1998 Cristian Gafton <gafton@redhat.com>
- patch to build on ARM

* Wed Sep 30 1998 Jeff Johnson <jbj@redhat.com>
- fix typo (printf, not tprintf).

* Sat Sep 19 1998 Jeff Johnson <jbj@redhat.com>
- fix compile problem on sparc.

* Tue Aug 18 1998 Cristian Gafton <gafton@redhat.com>
- buildroot

* Mon Jul 20 1998 Cristian Gafton <gafton@redhat.com>
- added the umoven patch from James Youngman <jay@gnu.org>
- fixed build problems on newer glibc releases

* Mon Jun 08 1998 Prospector System <bugs@redhat.com>
- translations modified for de, fr, tr
