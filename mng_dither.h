/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_dither.h              copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Dithering routines (definition)                            * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of the dithering routines                       * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _mng_dither_h_
#define _mng_dither_h_

#ifndef _libmng_h_                     /* save some compilation-time */
#include <libmng.h>
#endif

/* ************************************************************************** */

mng_retcode dither_a_row (mng_datap  pData,
                          mng_uint8p pRow);

/* ************************************************************************** */

#endif /* _mng_dither_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
