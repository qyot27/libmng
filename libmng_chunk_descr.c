/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : libmng_chunk_descr.c      copyright (c) 2004 G.Juyn        * */
/* * version   : 1.0.9                                                      * */
/* *                                                                        * */
/* * purpose   : Chunk descriptor functions (implementation)                * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* *                                                                        * */
/* * comment   : implementation of the chunk- anf field-descriptor          * */
/* *             routines                                                   * */
/* *                                                                        * */
/* * changes   : 1.0.9 - 12/06/2004 - G.Juyn                                * */
/* *             - added conditional MNG_OPTIMIZE_CHUNKREADER               * */
/* *                                                                        * */
/* ************************************************************************** */

#include "libmng.h"
#include "libmng_data.h"
#include "libmng_error.h"
#include "libmng_trace.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "libmng_memory.h"
#include "libmng_chunks.h"
#include "libmng_chunk_descr.h"
#include "libmng_chunk_prc.h"
#include "libmng_chunk_io.h"

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

/* ************************************************************************** */

#ifdef MNG_OPTIMIZE_CHUNKREADER
#if defined(MNG_INCLUDE_READ_PROCS) || defined(MNG_INCLUDE_WRITE_PROCS)

/* ************************************************************************** */

void mng_get_chunkheader (mng_chunkid       iChunkname,
                          mng_chunk_headerp pResult)
{
  /* the table-idea & binary search code was adapted from
     libpng 1.1.0 (pngread.c) */
  /* NOTE1: the table must remain sorted by chunkname, otherwise the binary
     search will break !!! (ps. watch upper-/lower-case chunknames !!) */
  /* NOTE2: the layout must remain equal to the header part of all the
     chunk-structures (yes, that means even the pNext and pPrev fields;
     it's wasting a bit of space, but hey, the code is a lot easier) */

  mng_chunk_header mng_chunk_unknown = {MNG_UINT_HUH, mng_init_general, mng_free_unknown,
                                        mng_read_unknown, mng_write_unknown, mng_assign_unknown, 0, 0, sizeof(mng_unknown_chunk)};

  mng_chunk_header mng_chunk_table [] =
  {
#ifndef MNG_SKIPCHUNK_BACK
    {MNG_UINT_BACK, mng_init_general, mng_free_general, mng_read_back, mng_write_back, mng_assign_general, 0, 0, sizeof(mng_back)},
#endif
#ifndef MNG_SKIPCHUNK_BASI
    {MNG_UINT_BASI, mng_init_general, mng_free_general, mng_read_basi, mng_write_basi, mng_assign_general, 0, 0, sizeof(mng_basi)},
#endif
#ifndef MNG_SKIPCHUNK_CLIP
    {MNG_UINT_CLIP, mng_init_general, mng_free_general, mng_read_clip, mng_write_clip, mng_assign_general, 0, 0, sizeof(mng_clip)},
#endif
#ifndef MNG_SKIPCHUNK_CLON
    {MNG_UINT_CLON, mng_init_general, mng_free_general, mng_read_clon, mng_write_clon, mng_assign_general, 0, 0, sizeof(mng_clon)},
#endif
#ifndef MNG_NO_DELTA_PNG
#ifndef MNG_SKIPCHUNK_DBYK
    {MNG_UINT_DBYK, mng_init_general, mng_free_dbyk,    mng_read_dbyk, mng_write_dbyk, mng_assign_dbyk,    0, 0, sizeof(mng_dbyk)},
#endif
#endif
#ifndef MNG_SKIPCHUNK_DEFI
    {MNG_UINT_DEFI, mng_init_general, mng_free_general, mng_read_defi, mng_write_defi, mng_assign_general, 0, 0, sizeof(mng_defi)},
#endif
#ifndef MNG_NO_DELTA_PNG
    {MNG_UINT_DHDR, mng_init_general, mng_free_general, mng_read_dhdr, mng_write_dhdr, mng_assign_general, 0, 0, sizeof(mng_dhdr)},
#endif
#ifndef MNG_SKIPCHUNK_DISC
    {MNG_UINT_DISC, mng_init_general, mng_free_disc,    mng_read_disc, mng_write_disc, mng_assign_disc,    0, 0, sizeof(mng_disc)},
#endif
#ifndef MNG_NO_DELTA_PNG
#ifndef MNG_SKIPCHUNK_DROP
    {MNG_UINT_DROP, mng_init_general, mng_free_drop,    mng_read_drop, mng_write_drop, mng_assign_drop,    0, 0, sizeof(mng_drop)},
#endif
#endif
#ifndef MNG_SKIPCHUNK_LOOP
    {MNG_UINT_ENDL, mng_init_general, mng_free_general, mng_read_endl, mng_write_endl, mng_assign_general, 0, 0, sizeof(mng_endl)},
#endif
#ifndef MNG_SKIPCHUNK_FRAM
    {MNG_UINT_FRAM, mng_init_general, mng_free_fram,    mng_read_fram, mng_write_fram, mng_assign_fram,    0, 0, sizeof(mng_fram)},
#endif
    {MNG_UINT_IDAT, mng_init_general, mng_free_idat,    mng_read_idat, mng_write_idat, mng_assign_idat,    0, 0, sizeof(mng_idat)},  /* 12-th element! */
    {MNG_UINT_IEND, mng_init_general, mng_free_general, mng_read_iend, mng_write_iend, mng_assign_general, 0, 0, sizeof(mng_iend)},
    {MNG_UINT_IHDR, mng_init_general, mng_free_general, mng_read_ihdr, mng_write_ihdr, mng_assign_general, 0, 0, sizeof(mng_ihdr)},
#ifndef MNG_NO_DELTA_PNG
#ifdef MNG_INCLUDE_JNG
    {MNG_UINT_IJNG, mng_init_general, mng_free_general, mng_read_ijng, mng_write_ijng, mng_assign_general, 0, 0, sizeof(mng_ijng)},
#endif
    {MNG_UINT_IPNG, mng_init_general, mng_free_general, mng_read_ipng, mng_write_ipng, mng_assign_general, 0, 0, sizeof(mng_ipng)},
#endif
#ifdef MNG_INCLUDE_JNG
    {MNG_UINT_JDAA, mng_init_general, mng_free_jdaa,    mng_read_jdaa, mng_write_jdaa, mng_assign_jdaa,    0, 0, sizeof(mng_jdaa)},
    {MNG_UINT_JDAT, mng_init_general, mng_free_jdat,    mng_read_jdat, mng_write_jdat, mng_assign_jdat,    0, 0, sizeof(mng_jdat)},
    {MNG_UINT_JHDR, mng_init_general, mng_free_general, mng_read_jhdr, mng_write_jhdr, mng_assign_general, 0, 0, sizeof(mng_jhdr)},
    {MNG_UINT_JSEP, mng_init_general, mng_free_general, mng_read_jsep, mng_write_jsep, mng_assign_general, 0, 0, sizeof(mng_jsep)},
    {MNG_UINT_JdAA, mng_init_general, mng_free_jdaa,    mng_read_jdaa, mng_write_jdaa, mng_assign_jdaa,    0, 0, sizeof(mng_jdaa)},
#endif
#ifndef MNG_SKIPCHUNK_LOOP
    {MNG_UINT_LOOP, mng_init_general, mng_free_loop,    mng_read_loop, mng_write_loop, mng_assign_loop,    0, 0, sizeof(mng_loop)},
#endif
#ifndef MNG_SKIPCHUNK_MAGN
    {MNG_UINT_MAGN, mng_init_general, mng_free_general, mng_read_magn, mng_write_magn, mng_assign_general, 0, 0, sizeof(mng_magn)},
#endif
    {MNG_UINT_MEND, mng_init_general, mng_free_general, mng_read_mend, mng_write_mend, mng_assign_general, 0, 0, sizeof(mng_mend)},
    {MNG_UINT_MHDR, mng_init_general, mng_free_general, mng_read_mhdr, mng_write_mhdr, mng_assign_general, 0, 0, sizeof(mng_mhdr)},
#ifndef MNG_SKIPCHUNK_MOVE
    {MNG_UINT_MOVE, mng_init_general, mng_free_general, mng_read_move, mng_write_move, mng_assign_general, 0, 0, sizeof(mng_move)},
#endif
#ifndef MNG_NO_DELTA_PNG
#ifndef MNG_SKIPCHUNK_ORDR
    {MNG_UINT_ORDR, mng_init_general, mng_free_ordr,    mng_read_ordr, mng_write_ordr, mng_assign_ordr,    0, 0, sizeof(mng_ordr)},
#endif
#endif
#ifndef MNG_SKIPCHUNK_PAST
    {MNG_UINT_PAST, mng_init_general, mng_free_past,    mng_read_past, mng_write_past, mng_assign_past,    0, 0, sizeof(mng_past)},
#endif
    {MNG_UINT_PLTE, mng_init_general, mng_free_general, mng_read_plte, mng_write_plte, mng_assign_general, 0, 0, sizeof(mng_plte)},
#ifndef MNG_NO_DELTA_PNG
    {MNG_UINT_PPLT, mng_init_general, mng_free_general, mng_read_pplt, mng_write_pplt, mng_assign_general, 0, 0, sizeof(mng_pplt)},
    {MNG_UINT_PROM, mng_init_general, mng_free_general, mng_read_prom, mng_write_prom, mng_assign_general, 0, 0, sizeof(mng_prom)},
#endif
#ifndef MNG_SKIPCHUNK_SAVE
    {MNG_UINT_SAVE, mng_init_general, mng_free_save,    mng_read_save, mng_write_save, mng_assign_save,    0, 0, sizeof(mng_save)},
#endif
#ifndef MNG_SKIPCHUNK_SEEK
    {MNG_UINT_SEEK, mng_init_general, mng_free_seek,    mng_read_seek, mng_write_seek, mng_assign_seek,    0, 0, sizeof(mng_seek)},
#endif
#ifndef MNG_SKIPCHUNK_SHOW
    {MNG_UINT_SHOW, mng_init_general, mng_free_general, mng_read_show, mng_write_show, mng_assign_general, 0, 0, sizeof(mng_show)},
#endif
#ifndef MNG_SKIPCHUNK_TERM
    {MNG_UINT_TERM, mng_init_general, mng_free_general, mng_read_term, mng_write_term, mng_assign_general, 0, 0, sizeof(mng_term)},
#endif
#ifndef MNG_SKIPCHUNK_bKGD
    {MNG_UINT_bKGD, mng_init_general, mng_free_general, mng_read_bkgd, mng_write_bkgd, mng_assign_general, 0, 0, sizeof(mng_bkgd)},
#endif
#ifndef MNG_SKIPCHUNK_cHRM
    {MNG_UINT_cHRM, mng_init_general, mng_free_general, mng_read_chrm, mng_write_chrm, mng_assign_general, 0, 0, sizeof(mng_chrm)},
#endif
#ifndef MNG_SKIPCHUNK_eXPI
    {MNG_UINT_eXPI, mng_init_general, mng_free_expi,    mng_read_expi, mng_write_expi, mng_assign_expi,    0, 0, sizeof(mng_expi)},
#endif
#ifndef MNG_SKIPCHUNK_evNT
    {MNG_UINT_evNT, mng_init_general, mng_free_evnt,    mng_read_evnt, mng_write_evnt, mng_assign_evnt,    0, 0, sizeof(mng_evnt)},
#endif
#ifndef MNG_SKIPCHUNK_fPRI
    {MNG_UINT_fPRI, mng_init_general, mng_free_general, mng_read_fpri, mng_write_fpri, mng_assign_general, 0, 0, sizeof(mng_fpri)},
#endif
#ifndef MNG_SKIPCHUNK_gAMA
    {MNG_UINT_gAMA, mng_init_general, mng_free_general, mng_read_gama, mng_write_gama, mng_assign_general, 0, 0, sizeof(mng_gama)},
#endif
#ifndef MNG_SKIPCHUNK_hIST
    {MNG_UINT_hIST, mng_init_general, mng_free_general, mng_read_hist, mng_write_hist, mng_assign_general, 0, 0, sizeof(mng_hist)},
#endif
#ifndef MNG_SKIPCHUNK_iCCP
    {MNG_UINT_iCCP, mng_init_general, mng_free_iccp,    mng_read_iccp, mng_write_iccp, mng_assign_iccp,    0, 0, sizeof(mng_iccp)},
#endif
#ifndef MNG_SKIPCHUNK_iTXt
    {MNG_UINT_iTXt, mng_init_general, mng_free_itxt,    mng_read_itxt, mng_write_itxt, mng_assign_itxt,    0, 0, sizeof(mng_itxt)},
#endif
#ifndef MNG_SKIPCHUNK_nEED
    {MNG_UINT_nEED, mng_init_general, mng_free_need,    mng_read_need, mng_write_need, mng_assign_need,    0, 0, sizeof(mng_need)},
#endif
/* TODO:     {MNG_UINT_oFFs, 0, 0, 0, 0, 0, 0},  */
/* TODO:     {MNG_UINT_pCAL, 0, 0, 0, 0, 0, 0},  */
#ifndef MNG_SKIPCHUNK_pHYg
    {MNG_UINT_pHYg, mng_init_general, mng_free_general, mng_read_phyg, mng_write_phyg, mng_assign_general, 0, 0, sizeof(mng_phyg)},
#endif
#ifndef MNG_SKIPCHUNK_pHYs
    {MNG_UINT_pHYs, mng_init_general, mng_free_general, mng_read_phys, mng_write_phys, mng_assign_general, 0, 0, sizeof(mng_phys)},
#endif
#ifndef MNG_SKIPCHUNK_sBIT
    {MNG_UINT_sBIT, mng_init_general, mng_free_general, mng_read_sbit, mng_write_sbit, mng_assign_general, 0, 0, sizeof(mng_sbit)},
#endif
/* TODO:     {MNG_UINT_sCAL, 0, 0, 0, 0, 0, 0},  */
#ifndef MNG_SKIPCHUNK_sPLT
    {MNG_UINT_sPLT, mng_init_general, mng_free_splt,    mng_read_splt, mng_write_splt, mng_assign_splt,    0, 0, sizeof(mng_splt)},
#endif
    {MNG_UINT_sRGB, mng_init_general, mng_free_general, mng_read_srgb, mng_write_srgb, mng_assign_general, 0, 0, sizeof(mng_srgb)},
#ifndef MNG_SKIPCHUNK_tEXt
    {MNG_UINT_tEXt, mng_init_general, mng_free_text,    mng_read_text, mng_write_text, mng_assign_text,    0, 0, sizeof(mng_text)},
#endif
#ifndef MNG_SKIPCHUNK_tIME
    {MNG_UINT_tIME, mng_init_general, mng_free_general, mng_read_time, mng_write_time, mng_assign_general, 0, 0, sizeof(mng_time)},
#endif
    {MNG_UINT_tRNS, mng_init_general, mng_free_general, mng_read_trns, mng_write_trns, mng_assign_general, 0, 0, sizeof(mng_trns)},
#ifndef MNG_SKIPCHUNK_zTXt
    {MNG_UINT_zTXt, mng_init_general, mng_free_ztxt,    mng_read_ztxt, mng_write_ztxt, mng_assign_ztxt,    0, 0, sizeof(mng_ztxt)},
#endif
  };

                                       /* binary search variables */
  mng_int32         iTop, iLower, iUpper, iMiddle;
  mng_chunk_headerp pEntry;            /* pointer to found entry */
                                       /* determine max index of table */
  iTop = (sizeof (mng_chunk_table) / sizeof (mng_chunk_table [0])) - 1;

  /* binary search; with 54 chunks, worst-case is 7 comparisons */
  iLower  = 0;
#ifndef MNG_NO_DELTA_PNG
  iMiddle = 11;                        /* start with the IDAT entry */
#else
  iMiddle = 8;
#endif
  iUpper  = iTop;
  pEntry  = 0;                         /* no goods yet! */

  do                                   /* the binary search itself */
    {
      if (mng_chunk_table [iMiddle].iChunkname < iChunkname)
        iLower = iMiddle + 1;
      else if (mng_chunk_table [iMiddle].iChunkname > iChunkname)
        iUpper = iMiddle - 1;
      else
      {
        pEntry = &mng_chunk_table [iMiddle];
        break;
      }
      iMiddle = (iLower + iUpper) >> 1;
    }
  while (iLower <= iUpper);

  if (!pEntry)                         /* unknown chunk ? */
    pEntry = &mng_chunk_unknown;       /* make it so! */

  MNG_COPY (pResult, pEntry, sizeof(mng_chunk_header))

  return;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_READ_PROCS || MNG_INCLUDE_WRITE_PROCS */
#endif /* MNG_OPTIMIZE_CHUNKREADER */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

