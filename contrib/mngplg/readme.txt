MNGPLG
A simple browser plug-in for the MNG image/animation file format.
By Jason Summers  <jason1@pobox.com>
Version 0.4.0  June 2000
Web site: <http://pobox.com/~jason1/imaging/mngplg/>


COPYRIGHT NOTICE

Copyright (c) 2000 by Jason Summers <jason1@pobox.com>

This software is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
FITNESS FOR A PARTICULAR PURPOSE.

Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it 
freely.


This software uses several third-party libraries (listed below), some of 
which are optional. It is your responsibility to comply with the licenses of 
any libraries used.

---------

Based on libmng.
   Copyright (c) 2000 Gerard Juyn (gerard@libmng.com)
   <http://www.libmng.com/>

Uses the zlib compression library.
   (C) 1995-1998 Jean-loup Gailly and Mark Adler

This software is based in part on the work of the Independent JPEG Group.
   Copyright (C) 1991-1998, Thomas G. Lane

Uses the lcms color management library by Marti Maria.
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

MNGPLG is pre-release software that uses a pre-release MNG library. It is 
not a stable and mature product. It is almost certainly capable of crashing 
your browser. It is possible, though very unlikely, that it could even do 
more serious damage. If you are paranoid about security, I recommend that 
you do not leave it enabled in your browser for an extended period of time. 
To disable it, simply rename the "npmngplg.dll" file to "npmngplg.old".


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
".dll". You should probably change that.

In Netscape 4.x, you can verify that the plug-in is installed by choosing 
Help|About Plug-ins from the main menu (with JavaScript enabled).


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

The C source code is included. I don't have any instructions for it yet. 
It's compatible with libmng 0.5.1 and 0.5.2 (probably also later versions).
