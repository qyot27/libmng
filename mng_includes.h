/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_includes.h            copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : generic dependancy definitions                             * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of generic include dependencies                 * */
/* *                                                                        * */
/* * requires  : **none**                                                   * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifndef _mng_includes_h_
#define _mng_includes_h_

/* ************************************************************************** */

#ifdef MNG_SUPPORT_READ
#define MNG_INCLUDE_READ_PROCS
#define MNG_INCLUDE_MEMORY_PROCS
#define MNG_INCLUDE_ERROR_PROCS
#endif

#ifdef MNG_SUPPORT_WRITE
#define MNG_INCLUDE_WRITE_PROCS
#define MNG_INCLUDE_MEMORY_PROCS
#define MNG_INCLUDE_ERROR_PROCS
#endif

#ifdef MNG_SUPPORT_DISPLAY
#define MNG_INCLUDE_DISPLAY_PROCS
#define MNG_INCLUDE_TIMING_PROCS
#define MNG_INCLUDE_MEMORY_PROCS
#define MNG_INCLUDE_ERROR_PROCS
#define MNG_INCLUDE_OBJECTS
#define MNG_INCLUDE_ZLIB
#endif

#ifdef MNG_ACCESS_CHUNKS
#define MNG_INCLUDE_MEMORY_PROCS
#define MNG_INCLUDE_ERROR_PROCS
#define MNG_INCLUDE_ZLIB
#endif

#ifdef MNG_SUPPORT_JNG
#define MNG_INCLUDE_JPEG
#endif

#ifdef MNG_FULL_CMS
#define MNG_INCLUDE_LCMS
#endif

#ifdef MNG_AUTO_TRANSPARENCY
#define MNG_INCLUDE_TRANSPARENCY
#endif

#ifdef MNG_AUTO_DITHER
#define MNG_INCLUDE_DITHERING
#endif

#ifdef MNG_INTERNAL_TRACE
#define MNG_INCLUDE_TRACE_PROCS
#ifdef MNG_TRACE_TELLTALE
#define MNG_INCLUDE_TRACE_STRINGS
#endif
#endif

#ifdef MNG_ERROR_TELLTALE
#define MNG_INCLUDE_ERROR_STRINGS
#endif

/* ************************************************************************** */

#endif /* _mng_includes_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
