/* ************************************************************************** */
/* *                                                                        * */
/* * COPYRIGHT NOTICE:                                                      * */
/* *                                                                        * */
/* * Copyright (c) 2000 Gerard Juyn (gerard@libmng.com)                     * */
/* * [You may insert additional notices after this sentence if you modify   * */
/* *  this source]                                                          * */
/* *                                                                        * */
/* * For the purposes of this copyright and license, "Contributing Authors" * */
/* * is defined as the following set of individuals:                        * */
/* *                                                                        * */
/* *    Gerard Juyn                                                         * */
/* *    (hopefully some more to come...)                                    * */
/* *                                                                        * */
/* * The MNG Library is supplied "AS IS".  The Contributing Authors         * */
/* * disclaim all warranties, expressed or implied, including, without      * */
/* * limitation, the warranties of merchantability and of fitness for any   * */
/* * purpose.  The Contributing Authors assume no liability for direct,     * */
/* * indirect, incidental, special, exemplary, or consequential damages,    * */
/* * which may result from the use of the MNG Library, even if advised of   * */
/* * the possibility of such damage.                                        * */
/* *                                                                        * */
/* * Permission is hereby granted to use, copy, modify, and distribute this * */
/* * source code, or portions hereof, for any purpose, without fee, subject * */
/* * to the following restrictions:                                         * */
/* *                                                                        * */
/* * 1. The origin of this source code must not be misrepresented;          * */
/* *    you must not claim that you wrote the original software.            * */
/* *                                                                        * */
/* * 2. Altered versions must be plainly marked as such and must not be     * */
/* *    misrepresented as being the original source.                        * */
/* *                                                                        * */
/* * 3. This Copyright notice may not be removed or altered from any source * */
/* *    or altered source distribution.                                     * */
/* *                                                                        * */
/* * The Contributing Authors specifically permit, without fee, and         * */
/* * encourage the use of this source code as a component to supporting     * */
/* * the MNG and JNG file format in commercial products.  If you use this   * */
/* * source code in a product, acknowledgment would be highly appreciated.  * */
/* *                                                                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * Parts of this software have been adapted from the libpng package.      * */
/* * Although this library supports all features from the PNG specification * */
/* * (as MNG descends from it) it does not require the libpng package.      * */
/* * It does require the zlib library and optionally the IJG jpeg library,  * */
/* * and/or the "little-cms" library by Marti Maria (depending on the       * */
/* * inclusion of support for JNG and Full-Color-Management respectively.   * */
/* *                                                                        * */
/* * This library's function is primarily to read and display MNG           * */
/* * animations. It is not meant as a full-featured image-editing           * */
/* * component! It does however offer creation and editing functionality    * */
/* * at the chunk level.                                                    * */
/* * (future modifications may include some more support for creation       * */
/* *  and or editing)                                                       * */
/* *                                                                        * */
/* ************************************************************************** */

/* ************************************************************************** */
/* *                                                                        * */
/* * Version numbering                                                      * */
/* *                                                                        * */
/* * X.Y.Z : X = release (0 = initial build)                                * */
/* *         Y = major version (uneven = test; even = production)           * */
/* *         Z = minor version (bugfixes; 2 is older than 10)               * */
/* *                                                                        * */
/* * production versions only appear when a test-version is extensively     * */
/* * tested and found stable or for intermediate bug-fixes (recognized by   * */
/* * a change in the Z number)                                              * */
/* *                                                                        * */
/* * x.1.x      = test version                                              * */
/* * x.2.x      = production version                                        * */
/* * x.3.x      = test version                                              * */
/* * x.4.x      = production version                                        * */
/* *  etc.                                                                  * */
/* *                                                                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * Identifier naming conventions throughout this library                  * */
/* *                                                                        * */
/* * iXxxx      = an integer                                                * */
/* * dXxxx      = a float                                                   * */
/* * pXxxx      = a pointer                                                 * */
/* * bXxxx      = a boolean                                                 * */
/* * eXxxx      = an enumeration                                            * */
/* * hXxxx      = a handle                                                  * */
/* * zXxxx      = a zero-terminated string (pchar)                          * */
/* * fXxxx      = a pointer to a function (callback)                        * */
/* * aXxxx      = an array                                                  * */
/* * sXxxx      = a structure                                               * */
/* *                                                                        * */
/* * Macros & defines are in all uppercase.                                 * */
/* * Functions & typedefs in all lowercase.                                 * */
/* * Exported stuff is prefixed with MNG_ or mng_ respectively.             * */
/* *                                                                        * */
/* * (I may have missed a couple; don't hesitate to let me know!)           * */
/* *                                                                        * */
/* ************************************************************************** */

/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : libmng.h                  copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : main application interface                                 * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : The main application interface. An application should not  * */
/* *             need access to any of the other modules!                   * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                        **nobody**  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _libmng_h_
#define _libmng_h_

/* ************************************************************************** */
/* *                                                                        * */
/* *  User-selectable compile-time options                                  * */
/* *                                                                        * */
/* ************************************************************************** */

/* enable exactly one of the MNG-(sub)set selectors */
/* use this to select which (sub)set of the MNG specification you wish
   to support */
/* generally you'll want full support as the library provides it automatically
   for you! if you're really strung on memory-requirements you can opt
   to enable less support (but it's just NOT a good idea!) */

#if !defined(MNG_SUPPORT_FULL) && !defined(MNG_SUPPORT_LC) && !defined(MNG_SUPPORT_VLC)
#define MNG_SUPPORT_FULL
/* #define MNG_SUPPORT_LC */
/* #define MNG_SUPPORT_VLC */
#endif

/* ************************************************************************** */

/* enable JPEG support if required */
/* use this to enable the JNG support routines */
/* (this requires an external jpeg package;
   currently only IJG's jpgsrc6b is supported!) */
/* note also that the IJG code can be either 8- or 12-bit (eg. not both);
   so choose the one you've defined in jconfig.h; if you don't know what
   the heck I'm talking about, just leave it at 8-bit support (thank you!) */

/* #define MNG_SUPPORT_IJG6B */
#if defined(MNG_SUPPORT_IJG6B) && !defined(MNG_SUPPORT_JPEG8) && !defined(MNG_SUPPORT_JPEG12)
#define MNG_SUPPORT_JPEG8
/* #define MNG_SUPPORT_JPEG12 */
#endif

/* ************************************************************************** */

/* enable required high-level functions */
/* use this to select the high-level functions you require */
/* if you only need to display a MNG, disable write support! */
/* if you only need to examine a MNG, disable write & display support! */
/* if you only need to copy a MNG, disable display support! */
/* if you only need to create a MNG, disable read & display support! */
/* (Note: turning all options off will be very unuseful!) */

#if !defined(MNG_SUPPORT_READ) && !defined(MNG_SUPPORT_WRITE) && !defined(MNG_SUPPORT_DISPLAY)
#define MNG_SUPPORT_READ
/* #define MNG_SUPPORT_WRITE */
#define MNG_SUPPORT_DISPLAY
#endif

/* ************************************************************************** */

/* enable chunk access functions */
/* use this to select whether you need access to the individual chunks */
/* useful if you want to examine a read MNG (you'll also need MNG_STORE_CHUNKS !)*/
/* required if you need to create & write a new MNG! */

/* #define MNG_ACCESS_CHUNKS */

/* ************************************************************************** */

/* enable exactly one of the color-management-functionality selectors */
/* use this to select the level of automatic color support */
/* MNG_FULL_CMS requires the lcms (little cms) external package ! */
/* if you want your own app (or the OS) to handle color-management
   select MNG_APP_CMS */

#if !defined(MNG_FULL_CMS) && !defined(MNG_GAMMA_ONLY) && !defined(MNG_NO_CMS) && !defined(MNG_APP_CMS)
#define MNG_FULL_CMS
/* #define MNG_GAMMA_ONLY */
/* #define MNG_NO_CMS */
/* #define MNG_APP_CMS */
#endif

/* ************************************************************************** */

/* enable automatic dithering */
/* use this if you need dithering support to convert high-resolution
   images to a low-resolution output-device */
/* PS: not supported yet */

/* #define MNG_AUTO_DITHER */

/* ************************************************************************** */

/* enable whether chunks should be stored for reference later */
/* use this if you need to examine the chunks of a MNG you have read,
   or (re-)write a MNG you have read */
/* turn this off if you want to reduce memory-consumption */

#ifdef MNG_ACCESS_CHUNKS
#define MNG_STORE_CHUNKS
#endif

/* ************************************************************************** */

/* enable internal memory management (if your compiler supports it) */
/* use this if your compiler supports the 'standard' memory functions
   (calloc & free), and you want the library to use these functions and not
   bother your app with memory-callbacks */

/* #define MNG_INTERNAL_MEMMNGMT */

/* ************************************************************************** */

/* enable internal tracing-functionality (manual debugging purposes) */
/* use this if you have trouble location bugs or problems */
/* NOTE: you'll need to specify the trace callback function! */

/* #define MNG_SUPPORT_TRACE */

/* ************************************************************************** */

/* enable extended error- and trace-telltaling */
/* use this if you need explanatory messages with errors and/or tracing */
/* PS: not supported yet */

/* #define MNG_ERROR_TELLTALE */
/* #define MNG_TRACE_TELLTALE */

/* ************************************************************************** */
/* *                                                                        * */
/* *  End of user-selectable compile-time options                           * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_SUPPORT_READ
#define MNG_INCLUDE_READ_PROCS
#endif

#ifdef MNG_SUPPORT_WRITE
#define MNG_INCLUDE_WRITE_PROCS
#endif

#ifdef MNG_SUPPORT_DISPLAY
#define MNG_INCLUDE_FILTERS
#define MNG_INCLUDE_INTERLACE
#define MNG_INCLUDE_OBJECTS
#define MNG_INCLUDE_DISPLAY_PROCS
#define MNG_INCLUDE_TIMING_PROCS
#define MNG_INCLUDE_ZLIB
#endif

#ifdef MNG_STORE_CHUNKS
#define MNG_INCLUDE_ZLIB
#endif

#ifdef MNG_SUPPORT_IJG6B
#define MNG_INCLUDE_JNG
#define MNG_INCLUDE_IJG6B
#endif

#ifdef MNG_FULL_CMS
#define MNG_INCLUDE_LCMS
#endif

#ifdef MNG_AUTO_DITHER
#define MNG_INCLUDE_DITHERING
#endif

#ifdef MNG_SUPPORT_TRACE
#define MNG_INCLUDE_TRACE_PROCS
#ifdef MNG_TRACE_TELLTALE
#define MNG_INCLUDE_TRACE_STRINGS
#endif
#endif

#ifdef MNG_ERROR_TELLTALE
#define MNG_INCLUDE_ERROR_STRINGS
#endif

/* ************************************************************************** */

#include <mng_types.h>

/* ************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
/* *                                                                        * */
/* *  High-level application functions                                      * */
/* *                                                                        * */
/* ************************************************************************** */

/* library initialization function */
/* must be the first called before anything can be done at all */
/* initializes internal datastructure(s) */
MNG_EXT mng_handle  MNG_DECL mng_initialize      (mng_int32     iUserdata,
                                                  mng_memalloc  fMemalloc,
                                                  mng_memfree   fMemfree,
                                                  mng_traceproc fTraceproc);

/* library reset function */
/* can be used to re-initialize the library, so another image can be
   processed. there's absolutely no harm in calling it, even when it's not
   really necessary */
MNG_EXT mng_retcode MNG_DECL mng_reset           (mng_handle    hHandle);

/* library cleanup function */
/* must be the last called to clean up internal datastructure(s) */
MNG_EXT mng_retcode MNG_DECL mng_cleanup         (mng_handle*   hHandle);

/* high-level read function */
/* use this if you want to read a Network Graphic */
#ifdef MNG_SUPPORT_READ
MNG_EXT mng_retcode MNG_DECL mng_read            (mng_handle    hHandle);
#endif

/* high-level write & create functions */
/* use this if you want to write a previously read Network Graphic or
   if you want to create a new graphic and write it */
/* to create a new graphic you'll also need access to the chunks
   (eg. #define MNG_ACCESS_CHUNKS !) */
#ifdef MNG_SUPPORT_WRITE
MNG_EXT mng_retcode MNG_DECL mng_write           (mng_handle    hHandle);
MNG_EXT mng_retcode MNG_DECL mng_create          (mng_handle    hHandle);
#endif

/* high-level display functions */
/* use these to display a previously read or created graphic or
   to read & display a graphic simultaneously */
/* mng_display_resume should be called after a timer-interval
   expires that was set through the settimer-callback, or to resume
   an animation after a call to mng_display_freeze */
/* mng_display_freeze thru mng_display_gotime can be used to influence
   the animation display of a MNG */
#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_SUPPORT_READ
MNG_EXT mng_retcode MNG_DECL mng_readdisplay     (mng_handle    hHandle);
#endif
MNG_EXT mng_retcode MNG_DECL mng_display         (mng_handle    hHandle);
MNG_EXT mng_retcode MNG_DECL mng_display_resume  (mng_handle    hHandle);
MNG_EXT mng_retcode MNG_DECL mng_display_freeze  (mng_handle    hHandle);
MNG_EXT mng_retcode MNG_DECL mng_display_reset   (mng_handle    hHandle);
MNG_EXT mng_retcode MNG_DECL mng_display_goframe (mng_handle    hHandle,
                                                  mng_uint32    iFramenr);
MNG_EXT mng_retcode MNG_DECL mng_display_golayer (mng_handle    hHandle,
                                                  mng_uint32    iLayernr);
MNG_EXT mng_retcode MNG_DECL mng_display_gotime  (mng_handle    hHandle,
                                                  mng_uint32    iPlaytime);
#endif /* MNG_SUPPORT_DISPLAY */

/* error reporting function */
/* use this if you need more detailed info on the last error */
/* iExtra1 & iExtra2 may contain errorcodes from zlib, jpeg, etc... */
/* zErrortext will only be filled if you #define MNG_ERROR_TELLTALE */
MNG_EXT mng_retcode MNG_DECL mng_getlasterror    (mng_handle    hHandle,
                                                  mng_int8*     iSeverity,
                                                  mng_chunkid*  iChunkname,
                                                  mng_uint32*   iChunkseq,
                                                  mng_int32*    iExtra1,
                                                  mng_int32*    iExtra2,
                                                  mng_pchar*    zErrortext);

/* ************************************************************************** */
/* *                                                                        * */
/* *  Callback set functions                                                * */
/* *                                                                        * */
/* ************************************************************************** */

/* memory callbacks */
/* called to allocate and release internal datastructures */
#ifndef MNG_INTERNAL_MEMMNGMT
MNG_EXT mng_retcode MNG_DECL mng_setcb_memalloc      (mng_handle        hHandle,
                                                      mng_memalloc      fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_memfree       (mng_handle        hHandle,
                                                      mng_memfree       fProc);
#endif /* MNG_INTERNAL_MEMMNGMT */

/* opne- & close-stream callbacks */
/* called to open & close streams for input or output */
#if defined(MNG_SUPPORT_READ) || defined(MNG_WRITE_SUPPORT)
MNG_EXT mng_retcode MNG_DECL mng_setcb_openstream    (mng_handle        hHandle,
                                                      mng_openstream    fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_closestream   (mng_handle        hHandle,
                                                      mng_closestream   fProc);
#endif

/* read callback */
/* called to get data from the inputstream */
#ifdef MNG_SUPPORT_READ
MNG_EXT mng_retcode MNG_DECL mng_setcb_readdata      (mng_handle        hHandle,
                                                      mng_readdata      fProc);
#endif

/* write callback */
/* called to put data into the outputstream */
#ifdef MNG_SUPPORT_WRITE
MNG_EXT mng_retcode MNG_DECL mng_setcb_writedata     (mng_handle        hHandle,
                                                      mng_writedata     fProc);
#endif

/* error callback */
/* called when an error occurs */
/* the application can determine if the error is recoverable,
   and may inform the library by setting specific returncodes */
MNG_EXT mng_retcode MNG_DECL mng_setcb_errorproc     (mng_handle        hHandle,
                                                      mng_errorproc     fProc);

/* trace callback */
/* called to show the currently executing function */
#ifdef MNG_INTERNAL_TRACE
MNG_EXT mng_retcode MNG_DECL mng_setcb_traceproc     (mng_handle        hHandle,
                                                      mng_traceproc     fProc);
#endif

/* callbacks for read processing */
/* processheader is called when all header information has been gathered
   from the inputstream */
/* processtext is called for every tEXt, zTXt and iTXt chunk in the
   inputstream (iType=0 for tEXt, 1 for zTXt and 2 for iTXt) */
#ifdef MNG_SUPPORT_READ
MNG_EXT mng_retcode MNG_DECL mng_setcb_processheader (mng_handle        hHandle,
                                                      mng_processheader fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_processtext   (mng_handle        hHandle,
                                                      mng_processtext   fProc);
#endif

/* callbacks for display processing */
/* getcanvasline is called to get an access-pointer to a line on the
   drawing-canvas */
/* getbkgdline is called to get an access-pointer to a line from the
   background-canvas */
/* refresh is called to inform the GUI to redraw the current canvas to
   it's output device (eg. in Win32 this would mean sending an
   invalidate message for the specified region */
#ifdef MNG_SUPPORT_DISPLAY
MNG_EXT mng_retcode MNG_DECL mng_setcb_getcanvasline (mng_handle        hHandle,
                                                      mng_getcanvasline fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_getbkgdline   (mng_handle        hHandle,
                                                      mng_getbkgdline   fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_refresh       (mng_handle        hHandle,
                                                      mng_refresh       fProc);

/* timing callbacks */
/* gettickcount is called to get the system tickcount (milliseconds);
   this is used to determine the remaining interval between frames */
/* settimer is called to inform the application that it should set a timer;
   when the timer is triggered the app must call mng_display_resume */
MNG_EXT mng_retcode MNG_DECL mng_setcb_gettickcount  (mng_handle        hHandle,
                                                      mng_gettickcount  fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_settimer      (mng_handle        hHandle,
                                                      mng_settimer      fProc);

/* color management callbacks */
/* called to transmit color management information to the application */
/* these are only used when you #define MNG_APP_CMS */
#ifdef MNG_APP_CMS
MNG_EXT mng_retcode MNG_DECL mng_setcb_processgamma  (mng_handle        hHandle,
                                                      mng_processgamma  fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_processchroma (mng_handle        hHandle,
                                                      mng_processchroma fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_processsrgb   (mng_handle        hHandle,
                                                      mng_processsrgb   fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_processiccp   (mng_handle        hHandle,
                                                      mng_processiccp   fProc);
MNG_EXT mng_retcode MNG_DECL mng_setcb_processarow   (mng_handle        hHandle,
                                                      mng_processarow   fProc);
#endif /* MNG_APP_CMS */
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */
/* *                                                                        * */
/* *  Callback get functions                                                * */
/* *                                                                        * */
/* ************************************************************************** */

/* see _setcb_ */
#ifndef MNG_INTERNAL_MEMMNGMT
MNG_EXT mng_memalloc      MNG_DECL mng_getcb_memalloc      (mng_handle hHandle);
MNG_EXT mng_memfree       MNG_DECL mng_getcb_memfree       (mng_handle hHandle);
#endif

/* see _setcb_ */
#if defined(MNG_SUPPORT_READ) || defined(MNG_WRITE_SUPPORT)
MNG_EXT mng_openstream    MNG_DECL mng_getcb_openstream    (mng_handle hHandle);
MNG_EXT mng_closestream   MNG_DECL mng_getcb_closestream   (mng_handle hHandle);
#endif

/* see _setcb_ */
#ifdef MNG_SUPPORT_READ
MNG_EXT mng_readdata      MNG_DECL mng_getcb_readdata      (mng_handle hHandle);
#endif

/* see _setcb_ */
#ifdef MNG_SUPPORT_WRITE
MNG_EXT mng_writedata     MNG_DECL mng_getcb_writedata     (mng_handle hHandle);
#endif

/* see _setcb_ */
MNG_EXT mng_errorproc     MNG_DECL mng_getcb_errorproc     (mng_handle hHandle);

/* see _setcb_ */
#ifdef MNG_INTERNAL_TRACE
MNG_EXT mng_traceproc     MNG_DECL mng_getcb_traceproc     (mng_handle hHandle);
#endif

/* see _setcb_ */
#ifdef MNG_SUPPORT_READ
MNG_EXT mng_processheader MNG_DECL mng_getcb_processheader (mng_handle hHandle);
MNG_EXT mng_processtext   MNG_DECL mng_getcb_processtext   (mng_handle hHandle);
#endif

/* see _setcb_ */
#ifdef MNG_SUPPORT_DISPLAY
MNG_EXT mng_getcanvasline MNG_DECL mng_getcb_getcanvasline (mng_handle hHandle);
MNG_EXT mng_getbkgdline   MNG_DECL mng_getcb_getbkgdline   (mng_handle hHandle);
MNG_EXT mng_refresh       MNG_DECL mng_getcb_refresh       (mng_handle hHandle);

/* see _setcb_ */
MNG_EXT mng_gettickcount  MNG_DECL mng_getcb_gettickcount  (mng_handle hHandle);
MNG_EXT mng_settimer      MNG_DECL mng_getcb_settimer      (mng_handle hHandle);

/* see _setcb_ */
#ifdef MNG_APP_CMS
MNG_EXT mng_processgamma  MNG_DECL mng_getcb_processgamma  (mng_handle hHandle);
MNG_EXT mng_processchroma MNG_DECL mng_getcb_processchroma (mng_handle hHandle);
MNG_EXT mng_processsrgb   MNG_DECL mng_getcb_processsrgb   (mng_handle hHandle);
MNG_EXT mng_processiccp   MNG_DECL mng_getcb_processiccp   (mng_handle hHandle);
MNG_EXT mng_processarow   MNG_DECL mng_getcb_processarow   (mng_handle hHandle);
#endif /* MNG_APP_CMS */
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */
/* *                                                                        * */
/* *  Property set functions                                                * */
/* *                                                                        * */
/* ************************************************************************** */

/* Application data pointer */
/* provided for application use; not used by the library */
MNG_EXT mng_retcode MNG_DECL mng_set_userdata        (mng_handle        hHandle,
                                                      mng_int32         iUserdata);

/* The style of the drawing- & background-canvas */
/* only used for displaying images */
/* both are initially set to 24-bit RGB */
MNG_EXT mng_retcode MNG_DECL mng_set_canvasstyle     (mng_handle        hHandle,
                                                      mng_uint32        iStyle);
MNG_EXT mng_retcode MNG_DECL mng_set_bkgdstyle       (mng_handle        hHandle,
                                                      mng_uint32        iStyle);

/* The default background color */
/* only used if the getbkgdline callback is not defined */
/* for initially painting the canvas and restoring (part of) the background */
MNG_EXT mng_retcode MNG_DECL mng_set_bgcolor         (mng_handle        hHandle,
                                                      mng_uint16        iRed,
                                                      mng_uint16        iGreen,
                                                      mng_uint16        iBlue);

/* Indicates storage of read chunks */
/* only useful if you #define mng_store_chunks */
/* can be used to dynamically change storage management */
MNG_EXT mng_retcode MNG_DECL mng_set_storechunks     (mng_handle        hHandle,
                                                      mng_bool          bStorechunks);

/* Color-management necessaties */
/* if you've defined MNG_FULL_CMS, you must specify the profile of the
   output-device and the sRGB conditions */
/* if you're on a sRGB system (Linux (intel), Windows, etc.), you can
   tell the CMS with mng_set_srgb and specify a default sRGB profile for
   the output-device; otherwise you'll need to specify the correct profile
   for your output-device and a default sRGB profile for input-images tagged
   with the sRGB chunk only */
/* NOTE: either call set_srgb with MNG_TRUE & call set_outputprofile
         or call set_srgb with MNG_FALSE & call set_outputprofile &
         set_srgbprofile */
/* BTW: the default for set_srgb is MNG_TRUE */
#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_FULL_CMS)
MNG_EXT mng_retcode MNG_DECL mng_set_srgb            (mng_handle        hHandle,
                                                      mng_bool          bIssRGB);
MNG_EXT mng_retcode MNG_DECL mng_set_outputprofile   (mng_handle        hHandle,
                                                      mng_pchar         zFilename);
MNG_EXT mng_retcode MNG_DECL mng_set_srgbprofile     (mng_handle        hHandle,
                                                      mng_pchar         zFilename);
#endif

/* Gamma settings */
/* only used if you #define MNG_FULL_CMS or #define MNG_GAMMA_ONLY */
/* ... blabla (explain gamma processing a little; eg. formula & stuff) ... */
MNG_EXT mng_retcode MNG_DECL mng_set_viewgamma       (mng_handle        hHandle,
                                                      mng_float         dGamma);
MNG_EXT mng_retcode MNG_DECL mng_set_displaygamma    (mng_handle        hHandle,
                                                      mng_float         dGamma);
MNG_EXT mng_retcode MNG_DECL mng_set_dfltimggamma    (mng_handle        hHandle,
                                                      mng_float         dGamma);
MNG_EXT mng_retcode MNG_DECL mng_set_viewgammaint    (mng_handle        hHandle,
                                                      mng_uint32        iGamma);
MNG_EXT mng_retcode MNG_DECL mng_set_displaygammaint (mng_handle        hHandle,
                                                      mng_uint32        iGamma);
MNG_EXT mng_retcode MNG_DECL mng_set_dfltimggammaint (mng_handle        hHandle,
                                                      mng_uint32        iGamma);

/* Ultimate clipping size */
/* used to limit extreme graphics from overloading the system */
/* if a graphic exceeds these limits an error is issued, which can
   be ignored by the app (using the errorproc callback). in that case
   the library will use these settings to clip the input graphic */
MNG_EXT mng_retcode MNG_DECL mng_set_maxcanvaswidth  (mng_handle        hHandle,
                                                      mng_uint32        iMaxwidth);
MNG_EXT mng_retcode MNG_DECL mng_set_maxcanvasheight (mng_handle        hHandle,
                                                      mng_uint32        iMaxheight);
MNG_EXT mng_retcode MNG_DECL mng_set_maxcanvassize   (mng_handle        hHandle,
                                                      mng_uint32        iMaxwidth,
                                                      mng_uint32        iMaxheight);

/* ************************************************************************** */
/* *                                                                        * */
/* *  Property get functions                                                * */
/* *                                                                        * */
/* ************************************************************************** */

/* see _set_ */
MNG_EXT mng_int32   MNG_DECL mng_get_userdata        (mng_handle        hHandle);

/* Network Graphic header details */
/* these get filled once the graphics header is processed,
   so they are available in the processheader callback; before that
   they are zeroed out and imagetype is set to it_unknown */
/* this might be a good point for the app to initialize the drawing-canvas! */
/* note that PNG and JNG files will not set the ticks thru simplicity fields! */
MNG_EXT mng_imgtype MNG_DECL mng_get_sigtype         (mng_handle        hHandle);
MNG_EXT mng_imgtype MNG_DECL mng_get_imagetype       (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_imagewidth      (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_imageheight     (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_ticks           (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_framecount      (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_layercount      (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_playtime        (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_simplicity      (mng_handle        hHandle);

/* see _set_ */
MNG_EXT mng_uint32  MNG_DECL mng_get_canvasstyle     (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_bkgdstyle       (mng_handle        hHandle);

/* see _set_ */
MNG_EXT mng_retcode MNG_DECL mng_get_bgcolor         (mng_handle        hHandle,
                                                      mng_uint16*       iRed,
                                                      mng_uint16*       iGreen,
                                                      mng_uint16*       iBlue);

/* see _set_ */
MNG_EXT mng_bool    MNG_DECL mng_get_storechunks     (mng_handle        hHandle);

/* see _set_ */
#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_FULL_CMS)
MNG_EXT mng_bool    MNG_DECL mng_get_srgb            (mng_handle        hHandle);
#endif

/* see _set_ */
MNG_EXT mng_float   MNG_DECL mng_get_viewgamma       (mng_handle        hHandle);
MNG_EXT mng_float   MNG_DECL mng_get_displaygamma    (mng_handle        hHandle);
MNG_EXT mng_float   MNG_DECL mng_get_dfltimggamma    (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_viewgammaint    (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_displaygammaint (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_dfltimggammaint (mng_handle        hHandle);

/* see _set_ */
MNG_EXT mng_uint32  MNG_DECL mng_get_maxcanvaswidth  (mng_handle        hHandle);
MNG_EXT mng_uint32  MNG_DECL mng_get_maxcanvasheight (mng_handle        hHandle);

/* ************************************************************************** */
/* *                                                                        * */
/* *  Chunk access functions                                                * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_ACCESS_CHUNKS

/* TODO: pretty much everything */

MNG_EXT mng_retcode MNG_DECL mng_iterate_chunks (mng_handle hHandle);

#endif /* mng_access_chunks */

/* ************************************************************************** */
/* *                                                                        * */
/* *  Canvas styles                                                         * */
/* *                                                                        * */
/* *  Note that the intentions are pretty darn good, but that the focus     * */
/* *  is currently on 8-bit color support                                   * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_CANVAS_RGB8      0x00000000L
#define MNG_CANVAS_RGBA8     0x00001000L
#define MNG_CANVAS_ARGB8     0x00003000L
#define MNG_CANVAS_BGR8      0x00000001L
#define MNG_CANVAS_BGRA8     0x00001001L
#define MNG_CANVAS_ABGR8     0x00003001L
#define MNG_CANVAS_RGB16     0x00000100L         /* not supported yet */
#define MNG_CANVAS_RGBA16    0x00001100L         /* not supported yet */
#define MNG_CANVAS_ARGB16    0x00003100L         /* not supported yet */
#define MNG_CANVAS_BGR16     0x00000101L         /* not supported yet */
#define MNG_CANVAS_BGRA16    0x00001101L         /* not supported yet */
#define MNG_CANVAS_ABGR16    0x00003101L         /* not supported yet */
#define MNG_CANVAS_GRAY8     0x00000002L         /* not supported yet */
#define MNG_CANVAS_GRAY16    0x00000102L         /* not supported yet */
#define MNG_CANVAS_GRAYA8    0x00001002L         /* not supported yet */
#define MNG_CANVAS_GRAYA16   0x00001102L         /* not supported yet */
#define MNG_CANVAS_AGRAY8    0x00003002L         /* not supported yet */
#define MNG_CANVAS_AGRAY16   0x00003102L         /* not supported yet */
#define MNG_CANVAS_DX15      0x00000003L         /* not supported yet */
#define MNG_CANVAS_DX16      0x00000004L         /* not supported yet */

#define MNG_CANVAS_PIXELTYPE(C)  (C & 0x000000ffL)
#define MNG_CANVAS_BITDEPTH(C)   (C & 0x00000100L)
#define MNG_CANVAS_HASALPHA(C)   (C & 0x00001000L)
#define MNG_CANVAS_ALPHAFIRST(C) (C & 0x00002000L)

#define MNG_CANVAS_RGB(C)        (MNG_CANVAS_PIXELTYPE (C) == 0)
#define MNG_CANVAS_BGR(C)        (MNG_CANVAS_PIXELTYPE (C) == 1)
#define MNG_CANVAS_GRAY(C)       (MNG_CANVAS_PIXELTYPE (C) == 2)
#define MNG_CANVAS_DIRECTX15(C)  (MNG_CANVAS_PIXELTYPE (C) == 3)
#define MNG_CANVAS_DIRECTX16(C)  (MNG_CANVAS_PIXELTYPE (C) == 4)
#define MNG_CANVAS_8BIT(C)       (!MNG_CANVAS_BITDEPTH (C))
#define MNG_CANVAS_16BIT(C)      (MNG_CANVAS_BITDEPTH (C))
#define MNG_CANVAS_PIXELFIRST(C) (!MNG_CANVAS_ALPHAFIRST (C))

/* ************************************************************************** */
/* *                                                                        * */
/* *  Chunk names (idea adapted from libpng 1.1.0 - png.h)                  * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_UINT_HUH  0x40404040L

#define MNG_UINT_BACK 0x4241434bL
#define MNG_UINT_BASI 0x42415349L
#define MNG_UINT_CLIP 0x434c4950L
#define MNG_UINT_CLON 0x434c4f4eL
#define MNG_UINT_DBYK 0x4442594bL
#define MNG_UINT_DEFI 0x44454649L
#define MNG_UINT_DHDR 0x44484452L
#define MNG_UINT_DISC 0x44495343L
#define MNG_UINT_DROP 0x44524f50L
#define MNG_UINT_ENDL 0x454e444cL
#define MNG_UINT_FRAM 0x4652414dL
#define MNG_UINT_IDAT 0x49444154L
#define MNG_UINT_IEND 0x49454e44L
#define MNG_UINT_IHDR 0x49484452L
#define MNG_UINT_IJNG 0x494a4e47L
#define MNG_UINT_IPNG 0x49504e47L
#define MNG_UINT_JDAT 0x4a444154L
#define MNG_UINT_JHDR 0x4a484452L
#define MNG_UINT_JSEP 0x4a534550L
#define MNG_UINT_LOOP 0x4c4f4f50L
#define MNG_UINT_MEND 0x4d454e44L
#define MNG_UINT_MHDR 0x4d484452L
#define MNG_UINT_MOVE 0x4d4f5645L
#define MNG_UINT_ORDR 0x4f524452L
#define MNG_UINT_PAST 0x50415354L
#define MNG_UINT_PLTE 0x504c5445L
#define MNG_UINT_PPLT 0x50504c54L
#define MNG_UINT_PROM 0x50524f4dL
#define MNG_UINT_SAVE 0x53415645L
#define MNG_UINT_SEEK 0x5345454bL
#define MNG_UINT_SHOW 0x53484f57L
#define MNG_UINT_TERM 0x5445524dL
#define MNG_UINT_bKGD 0x624b4744L
#define MNG_UINT_cHRM 0x6348524dL
#define MNG_UINT_eXPI 0x65585049L
#define MNG_UINT_fPRI 0x66505249L
#define MNG_UINT_gAMA 0x67414d41L
#define MNG_UINT_hIST 0x68495354L
#define MNG_UINT_iCCP 0x69434350L
#define MNG_UINT_iTXt 0x69545874L
#define MNG_UINT_nEED 0x6e454544L
#define MNG_UINT_oFFs 0x6f464673L
#define MNG_UINT_pCAL 0x7043414cL
#define MNG_UINT_pHYg 0x49444167L
#define MNG_UINT_pHYs 0x70485973L
#define MNG_UINT_sBIT 0x73424954L
#define MNG_UINT_sCAL 0x7343414cL
#define MNG_UINT_sPLT 0x73504c54L
#define MNG_UINT_sRGB 0x73524742L
#define MNG_UINT_tEXt 0x74455874L
#define MNG_UINT_tIME 0x74494d45L
#define MNG_UINT_tRNS 0x74524e53L
#define MNG_UINT_zTXt 0x7a545874L

/* ************************************************************************** */
/* *                                                                        * */
/* *  Chunk property values                                                 * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_COLORTYPE_GRAY            0
#define MNG_COLORTYPE_RGB             2
#define MNG_COLORTYPE_INDEXED         3
#define MNG_COLORTYPE_GRAYA           4
#define MNG_COLORTYPE_RGBA            6

#define MNG_COMPRESSION_DEFLATE       0

#define MNG_FILTER_ADAPTIVE           0

#define MNG_INTERLACE_NONE            0
#define MNG_INTERLACE_ADAM7           1

#ifdef MNG_INCLUDE_JNG
#define MNG_COLORTYPE_JPEGGRAY        8
#define MNG_COLORTYPE_JPEGCOLOR      10
#define MNG_COLORTYPE_JPEGGRAYA      12
#define MNG_COLORTYPE_JPEGCOLORA     14

#define MNG_COMPRESSION_BASELINEJPEG  8

#define MNG_INTERLACE_SEQUENTIAL      0
#define MNG_INTERLACE_PROGRESSIVE     8
#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */
/* *                                                                        * */
/* *  Processtext callback types                                            * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_TYPE_TEXT 0
#define MNG_TYPE_ZTXT 1
#define MNG_TYPE_ITXT 2

/* ************************************************************************** */

#ifdef __cplusplus
}
#endif

#endif /* _libmng_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

