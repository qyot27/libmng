MNGPLG
A simple browser plug-in for the MNG image/animation file format.
By Jason Summers  <jason1@pobox.com>
Version 0.9.0  August 5, 2000
Web site: <http://pobox.com/~jason1/imaging/mngplg/>


COPYRIGHT NOTICE

Copyright (c) 2000 by Jason Summers <jason1@pobox.com>

THIS SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT 
ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR 
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND 
PERFORMANCE OF THE LIBRARY IS WITH YOU.  SHOULD THE LIBRARY PROVE DEFECTIVE, 
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL 
ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR REDISTRIBUTE 
THIS SOFTWARE AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING 
ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE 
USE OR INABILITY TO USE THIS SOFTWARE (INCLUDING BUT NOT LIMITED TO LOSS OF 
DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD 
PARTIES OR A FAILURE OF THIS SOFTWARE TO OPERATE WITH ANY OTHER PROGRAMS), 
EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGES.

Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it 
freely.

This software uses several third-party libraries (listed below), some of 
which are optional. If you redistribute MNGPLB, it is your responsibility to 
comply with the licenses of any libraries used.

This package includes a compiled executable plug-in file, npmngplg.dll. This 
file includes code from lcms, which is distributed under the LGPL. To the 
best of my understanding, that basically means that anyone distributing that 
file must (1) make it possible for the recipient to modify the plug-in to 
use a new or modified version of lcms, and (2) make available the lcms 
source code. Requirement (1) is satisfied by the inclusion of the source 
code to the plug-in. For requirement (2), you can get the lcms source code 
at the web site listed at the beginning of this document, if necessary.


---------

Based on libmng.
   Copyright (c) 2000 Gerard Juyn (gerard@libmng.com)
   <http://www.libmng.com/>

Uses the zlib compression library.
   (C) 1995-1998 Jean-loup Gailly and Mark Adler

This software is based in part on the work of the Independent JPEG Group.
   Copyright (C) 1991-1998, Thomas G. Lane

Uses the lcms color management library by Martí Maria Saguer.
   (distributed under the GNU LESSER GENERAL PUBLIC LICENSE)

---------


INTRODUCTION

MNGPLG is a free Netscape-style browser plug-in which displays the MNG 
image/animation format. It is configured to claim the following MIME types:

video/x-mng
video/mng
image/x-jng
image/jng

It claims the file extensions ".mng" and ".jng", but file extensions should 
only apply when no MIME type is available (e.g. on an FTP site, or on your 
local hard disk).

It can also display PNG image files, but it would cause too many problems 
for it to try to claim the PNG data type.


REQUIREMENTS

MNG Requires a 32-bit Windows operating system, and a 32-bit web browser 
that supports Netscape-style plug-ins. For example, it supports current 
versions of Netscape, Opera, and (more or less) Microsoft Internet Explorer.


WARNING

Although I've tried to write it carefully, MNGPLG has not had any sort of 
security audit. Due to the nature of plug-ins, it is possible for certain 
types of bugs to exist which may allow remote web sites to take control of 
your computer or do harm to it by sending a carefully constructed data file 
to the plug-in. (Dozens of bugs like this have been discovered in every 
popular browser.) If you are paranoid about security, you may not wish to 
leave MNGPLG enabled in your browser for an extended period of time (or at 
all). To disable it, simply rename the "npmngplg.dll" file to 
"npmngplg.old".


INSTALLATION

There's no install program. To install it, copy the included "npmngplg.dll" 
file to your browser's "Plugins" folder, then restart your browser.

For Netscape, the Plugins folder is typically located somewhere like:
C:\Program Files\Netscape\Communicator\Program\Plugins

For MSIE, it should be somewhere like:
C:\Program Files\Internet Explorer\Plugins            or
C:\Program Files\Microsoft Internet\Plugins           or
C:\Program Files\Plus!\Microsoft Internet\Plugins     etc.

Note: Windows Explorer, by default, is configured to hide files that end in 
".dll". You should probably change that setting. I'd tell you how, but it's 
different in almost every version of Windows.

In Netscape 4.x, you can verify that the plug-in is installed by choosing 
Help|About Plug-ins from the main menu (with JavaScript enabled).

To uninstall, delete the npmngplg.dll file. It does not create any other 
files. It currently does not write anything to the Windows registry.

It's possible that copying the plug-in to your Plugins folder will not be 
sufficient to get Internet Explorer to use the plug-in. MSIE can easily be 
confused by various settings in the Windows registry. A complete MSIE 
plug-in troubleshooting guide is beyond the scope of this document. 
Searching the registry and removing every reference to "mng" will probably 
get it to work -- but back up your registry first, and don't blame me if you 
screw something up. If you are not able to get MSIE to use this plug-in, 
please ask a MSIE support group (not me) for help. Thanks.


HOW TO USE

In your web page, use the <embed> tag. For example:

<embed src="foo.mng" width="100" height="100" type="video/mng">

The src, width, and height attributes are required. Width and height should 
match the actual width and height of the image.

If possible, configure your web server (not browser) to assign the MIME type 
"video/x-mng" (or "video/mng") to files that end in ".mng", and assign type 
"video/x-jng" to files that end in ".jng". Strictly speaking, this is 
*required*, but the "type" attribute will allow you to work around it in 
most browsers.

Right-click on an MNG image as it is being displayed to get a menu with some 
of the usual features.

Transparency is not supported, and probably never will be. However, you can 
supply a background color to use in transparent areas by using the BGCOLOR 
attribute in the EMBED tag, i.e.:

<embed src="foo.mng" bgcolor="#ff0000"  ...>

You cannot use color names like "red"; you must use the hexadecimal format 
as in the example.

An image can be made into a "hotlink" by including an HREF and optionally a 
TARGET attribute in the EMBED tag. For example:

<embed src="foo.mng" href="http://www.libpng.org/pub/mng/" target="_blank" 
...>


SOURCE CODE

The C source code is included. It's compatible with libmng 0.9.2 (probably 
also later versions, possibly with minor changes).

To compile it, you'll need:

- The plug-in interface files from Netscape's plug-in SDK (preferably the 
one named winsdk40.zip). You need the files npapi.cpp and include\*.h. I 
recommend copying the files to your project directory and adding them to you 
project, rather than compiling them as a separate library.

- libmng MNG library <http://www.libmng.com/>. 

libmng in turn uses some other libraries:

    - zlib compression library

    - IJG JPEG library

    - [optional] lcms "Little Color Management System" library. 

If you include lcms, turn on the MNG_FULL_CMS option in mng_conf.h before 
compiling. Note that lcms is distributed under the LGPL -- be sure you 
understand the implications of that before distributing any resulting 
executable files.

If you don't include lcms, comment out the "#define MNGPLG_CMS" line in 
npmngplg.c.

I also recommend turning on the MNG_ERROR_TELLTALE option in mng_conf.h.

Make sure to include the npmngplg.def file in your project, or declare the 
necessary DLL entry points in some other way.
