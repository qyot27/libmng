/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : libmng_chunk_descr.h      copyright (c) 2004 G.Juyn        * */
/* * version   : 1.0.9                                                      * */
/* *                                                                        * */
/* * purpose   : Chunk descriptor functions (implementation)                * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* *                                                                        * */
/* * comment   : definition of the chunk- anf field-descriptor routines     * */
/* *                                                                        * */
/* * changes   : 1.0.9 - 12/06/2004 - G.Juyn                                * */
/* *             - added conditional MNG_OPTIMIZE_CHUNKREADER               * */
/* *                                                                        * */
/* ************************************************************************** */

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _libmng_chunk_descr_h_
#define _libmng_chunk_descr_h_

/* ************************************************************************** */

#ifdef MNG_OPTIMIZE_CHUNKREADER
#if defined(MNG_INCLUDE_READ_PROCS) || defined(MNG_INCLUDE_WRITE_PROCS)
void mng_get_chunkheader (mng_chunkid       iChunkname,
                          mng_chunk_headerp pResult);
#endif /* MNG_INCLUDE_READ_PROCS) || MNG_INCLUDE_WRITE_PROCS */
#endif /* MNG_OPTIMIZE_CHUNKREADER */

/* ************************************************************************** */

#endif /* _libmng_chunk_descr_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
