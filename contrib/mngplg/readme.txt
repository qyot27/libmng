MNGPLG (for lack of a name)
A simple browser plug-in for the MNG image/animation file format.
By Jason Summers  <jason1@pobox.com>   (c) 2000 by Jason Summers
Version 0.2.2  June 2000
Web site: <http://pobox.com/~jason1/imaging/mngplg/>

Based on libmng by Gerard Juyn <http://www.libmng.com/>.

Uses the zlib compression library.

This software is based in part on the work of the Independent JPEG Group.

Uses the lcms color management library by Marti Maria.

This software is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
FITNESS FOR A PARTICULAR PURPOSE.

This software is free to use, distribute, and modify, except that:
 - You must not misrepresent the origin of this software. 
 - Altered versions must be plainly marked as such.

This software uses several third-party libraries, some of which are 
optional. If you redistribute it, it is your responsibility to comply with 
the licenses of any libraries used.

---------

WARNING

This should be considered to be a preview, and not a fully functional 
program. It is pre-release software that uses a pre-release MNG library. It 
does not support all MNG files. It is almost certainly capable of crashing 
your browser. It is possible, though very unlikely, that it could even do 
more serious damage. If you are concerned about security, I recommend that 
you do not leave it enabled in your browser for an extended period of time. 
To disable it, simply rename the "npmngplg.dll" file to "npmngplg.old".


REQUIREMENTS

MNG Requires a 32-bit Windows operating system, and a 32-bit web browser 
that supports Netscape-style plug-ins. For example, it supports current 
versions of Netscape, Opera, and (more or less) Microsoft Internet Explorer.


INTRODUCTION

MNGPLG is a free Netscape-style browser plug-in which displays the MNG 
image/animation format. It is configured to claim the following MIME types:

video/mng
video/x-mng
image/jng
image/x-jng

It also claims the file extensions ".mng" and ".jng", but file extensions 
should only apply when no MIME type is available.


INSTALLATION

There's no install program. To install it, copy the included "npmngplg.dll" 
file to your browser's "Plugins" folder, then restart your browser.

For Netscape, the Plugins folder is typically located somewhere like:
c:\program files\netscape\communicator\program\plugins

For MSIE, it should be somewhere like:
c:\program files\internet explorer\plugins            or
c:\program files\microsoft internet\plugins           or
c:\program files\Plus!\microsoft internet\plugins     etc.

Note: Windows Explorer, by default, is configured to hide files that end in 
".dll". You should probably change that.

In Netscape 4.x, you can verify that the plug-in is installed by choosing 
Help|About Plug-ins from the main menu (with JavaScript enabled).


HOW TO USE

In your web page, use the <embed> tag. For example:

<embed src="foo.mng" width=100 height=100 type="video/mng">

The src, width, and height attributes are required. Width and height should 
match the actual width and height of the image.

If possible, configure your web server (not browser) to assign the MIME type 
"video/mng" (or "video/x-mng") to files that end in ".mng". Strictly 
speaking, this is *required*, but the "type" attribute will allow you to 
work around it in most browsers.

Right-click on an MNG image as it is being displayed to get a menu with some 
of the usual features.


SOURCE CODE

The C source code is included. I don't have any instructions for it yet. 
It's compatible with libmng 0.5.1 and 0.5.2 (possibly also later versions).
