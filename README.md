sqar
====

As I recall...

Defects in Tru64 UNIX (nee Digital UNIX) were called Quality Assurance Reports,
or QARs for short. As engineers, we could create and manage QARs by telnetting
into a VMS system and using a menu-driven terminal interface.

Around 2000 this system was in the process of being replaced by WebQAR, a
web-based front end which used the VMS QAR system as its backend. Eventually the
VMS system would be replaced by a modern database, a move that would be
transparent to WebQAR users except for the significant speedup it would afford.

Except some of us liked the terminal interface, so we created `sqar`, a
command-line interface for WebQAR. This repository contains that 
[single-file Perl script](./sqar) for posterity.

See also https://github.com/agriffis-archive/sreq
