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

#include <stddef.h>                    // needed for offsetof()

#include "libmng.h"
#include "libmng_data.h"
#include "libmng_error.h"
#include "libmng_trace.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "libmng_memory.h"
#include "libmng_objects.h"
#include "libmng_chunks.h"
#include "libmng_chunk_descr.h"
#include "libmng_object_prc.h"
#include "libmng_chunk_prc.h"
#include "libmng_chunk_io.h"
#include "libmng_display.h"

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

/* ************************************************************************** */

#ifdef MNG_OPTIMIZE_CHUNKREADER
#if defined(MNG_INCLUDE_READ_PROCS) || defined(MNG_INCLUDE_WRITE_PROCS)

/* ************************************************************************** */
/* ************************************************************************** */
/* PNG chunks */

mng_field_descriptor mng_fields_ihdr [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iWidth), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     1, 0x7FFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iHeight), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     1, 0x7FFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iBitdepth), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     1, 16, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iColortype), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 6, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iCompression), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iFilter), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ihdr, iInterlace), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_NULL, 0,
     0, 0}
  };

/* ************************************************************************** */

mng_field_descriptor mng_fields_plte [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_plte, aEntries), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 255, 1, 1,
     MNG_FIELD_REPETITIVE, 256,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_plte, aEntries), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 255, 1, 1,
     MNG_FIELD_REPETITIVE, 256,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_plte, aEntries), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 255, 1, 1,
     MNG_FIELD_REPETITIVE, 256,
     1, 0}
  };

/* ************************************************************************** */

mng_field_descriptor mng_fields_idat [] =
  {
    {mng_field_char, MNG_NULL,
     offsetof(mng_idat, pData), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_idat, iDatasize), MNG_NULL,
     0, 0, 0, 0,
     MNG_NULL, 0,
     0, 0}
  };

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_gAMA
mng_field_descriptor mng_fields_gama [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_gama, iGamma), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_cHRM
mng_field_descriptor mng_fields_chrm [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iWhitepointx), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iWhitepointy), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iRedx), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iRedy), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iGreeny), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iGreeny), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iBluex), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_chrm, iBluey), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_sRGB
mng_field_descriptor mng_fields_srgb [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_srgb, iRenderingintent), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 4, 1, 1,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_iCCP
mng_field_descriptor mng_fields_iccp [] =
  {
    {mng_field_char, MNG_NULL,
     offsetof(mng_iccp, zName), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_iccp, iNamesize), MNG_NULL,
     0, 0, 1, 79,
     MNG_FIELD_TERMINATOR, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_iccp, iCompression), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_char, MNG_NULL,
     offsetof(mng_iccp, pProfile), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_iccp, iProfilesize), MNG_NULL,
     0, 0, 0, 0,
     MNG_FIELD_DEFLATED, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_tEXt
mng_field_descriptor mng_fields_text [] =
  {
    {mng_field_char, MNG_NULL,
     offsetof(mng_text, zKeyword), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_text, iKeywordsize), MNG_NULL,
     0, 0, 1, 79,
     MNG_FIELD_TERMINATOR, 0,
     0, 0},
    {mng_field_char, MNG_NULL,
     offsetof(mng_text, zText), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_text, iTextsize), MNG_NULL,
     0, 0, 0, 0,
     0, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_zTXt
mng_field_descriptor mng_fields_ztxt [] =
  {
    {mng_field_char, MNG_NULL,
     offsetof(mng_ztxt, zKeyword), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_ztxt, iKeywordsize), MNG_NULL,
     0, 0, 1, 79,
     MNG_FIELD_TERMINATOR, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_ztxt, iCompression), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_char, MNG_NULL,
     offsetof(mng_ztxt, zText), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_ztxt, iTextsize), MNG_NULL,
     0, 0, 0, 0,
     MNG_FIELD_DEFLATED, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_iTXt
mng_field_descriptor mng_fields_itxt [] =
  {
    {mng_field_char, MNG_NULL,
     offsetof(mng_itxt, zKeyword), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_itxt, iKeywordsize), MNG_NULL,
     0, 0, 1, 79,
     MNG_FIELD_TERMINATOR, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_itxt, iCompressionflag), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_itxt, iCompressionmethod), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_char, MNG_NULL,
     offsetof(mng_itxt, zLanguage), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_itxt, iLanguagesize), MNG_NULL,
     0, 0, 0, 0,
     MNG_FIELD_TERMINATOR, 0,
     0, 0},
    {mng_field_char, MNG_NULL,
     offsetof(mng_itxt, zTranslation), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_itxt, iTranslationsize), MNG_NULL,
     0, 0, 0, 0,
     MNG_FIELD_TERMINATOR, 0,
     0, 0},
    {mng_field_char, mng_deflate_itxt,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 0, 0,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_bKGD
mng_field_descriptor mng_fields_bkgd [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_bkgd, iType), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 0, 0,
     MNG_FIELD_PUTIMGTYPE, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_bkgd, iIndex), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_IFIMGTYPE3, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_bkgd, iGray), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_IFIMGTYPE0 | MNG_FIELD_IFIMGTYPE4, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_bkgd, iRed), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_IFIMGTYPE2 | MNG_FIELD_IFIMGTYPE6, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_bkgd, iGreen), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_IFIMGTYPE2 | MNG_FIELD_IFIMGTYPE6, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_bkgd, iBlue), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_IFIMGTYPE2 | MNG_FIELD_IFIMGTYPE6, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_pHYs
mng_field_descriptor mng_fields_phys [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_phys, iSizex), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_phys, iSizey), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_phys, iUnit), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_sBIT
mng_field_descriptor mng_fields_sbit [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_sbit, iType), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 0, 0,
     MNG_FIELD_PUTIMGTYPE, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_sbit, aBits[0]), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_IFIMGTYPES, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_sbit, aBits[1]), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_IFIMGTYPE2 | MNG_FIELD_IFIMGTYPE3 | MNG_FIELD_IFIMGTYPE4 | MNG_FIELD_IFIMGTYPE6, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_sbit, aBits[2]), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_IFIMGTYPE2 | MNG_FIELD_IFIMGTYPE3 | MNG_FIELD_IFIMGTYPE6, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_sbit, aBits[3]), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_IFIMGTYPE6, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_tIME
mng_field_descriptor mng_fields_time [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_time, iYear), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_time, iMonth), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     1, 12, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_time, iDay), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     1, 31, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_time, iHour), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 24, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_time, iMinute), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 60, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_time, iSecond), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 60, 1, 1,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* JNG chunks */

/* ************************************************************************** */
/* ************************************************************************** */
/* MNG chunks */

mng_field_descriptor mng_fields_mhdr [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iWidth), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iHeight), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iTicks), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iLayercount), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iFramecount), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iPlaytime), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_mhdr, iSimplicity), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0}
  };

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_LOOP
mng_field_descriptor mng_fields_endl [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_endl, iLevel), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_DEFI
mng_field_descriptor mng_fields_defi [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iObjectid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iDonotshow), MNG_NULL,
     offsetof(mng_defi, bHasdonotshow), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iConcrete), MNG_NULL,
     offsetof(mng_defi, bHasconcrete), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFF, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iXlocation), MNG_NULL,
     offsetof(mng_defi, bHasloca), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iYlocation), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iLeftcb), MNG_NULL, 
     offsetof(mng_defi, bHasclip), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     2, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iRightcb), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     2, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iTopcb), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     2, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_defi, iBottomcb), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     2, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_BASI
mng_field_descriptor mng_fields_basi [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iWidth), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iHeight), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iBitdepth), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     1, 16, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iColortype), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 6, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iCompression), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iFilter), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iInterlace), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iRed), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iGreen), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iBlue), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iAlpha), MNG_NULL,
     offsetof(mng_basi, bHasalpha), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_basi, iViewable), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_CLON
mng_field_descriptor mng_fields_clon [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iSourceid), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iCloneid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iClonetype), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 2, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iDonotshow), MNG_NULL,
     offsetof(mng_clon, bHasdonotshow), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iConcrete), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iLocationtype), MNG_NULL,
     offsetof(mng_clon, bHasloca), MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 2, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iLocationx), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clon, iLocationy), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     1, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_BACK
mng_field_descriptor mng_fields_back [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_back, iRed), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_back, iGreen), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_back, iBlue), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_back, iMandatory), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 3, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_back, iImageid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_back, iTile), MNG_NULL, 
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_MOVE
mng_field_descriptor mng_fields_move [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_move, iFirstid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL, 
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_move, iLastid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL, 
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_move, iMovetype), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL, 
     0, 1, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_move, iMovex), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_move, iMovey), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_CLIP
mng_field_descriptor mng_fields_clip [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iFirstid), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iLastid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iCliptype), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 1, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iClipl), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iClipr), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iClipt), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_clip, iClipb), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_SHOW
mng_field_descriptor mng_fields_show [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_show, iFirstid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     1, 0xFFFF, 2, 2,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_show, iLastid), MNG_NULL,
     offsetof(mng_show, bHaslastid), MNG_NULL,
     MNG_NULL, MNG_NULL,
     1, 0xFFFF, 2, 2,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_show, iMode), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 7, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     0, 0},
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_TERM
mng_field_descriptor mng_fields_term [] =
  {
    {mng_field_int, MNG_NULL,
     offsetof(mng_term, iTermaction), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 3, 1, 1,
     MNG_NULL, 0,
     0, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_term, iIteraction), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 2, 1, 1,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_term, iDelay), MNG_NULL,
     MNG_NULL, MNG_NULL, 
     MNG_NULL, MNG_NULL,
     0, 0xFFFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     1, 0},
    {mng_field_int, MNG_NULL,
     offsetof(mng_term, iItermax), MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     0, 0x7FFFFFFF, 4, 4,
     MNG_FIELD_OPTIONAL, 0,
     1, 0}
  };
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_SEEK
mng_field_descriptor mng_fields_seek [] =
  {
    {mng_field_char, MNG_NULL,
     offsetof(mng_seek, zName), MNG_NULL,
     MNG_NULL, MNG_NULL,
     offsetof(mng_seek, iNamesize), MNG_NULL,
     0, 0, 1, 79,
     MNG_NULL, 0,
     0, 0}
  };
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* PNG chunks */

mng_chunk_descriptor mng_chunk_descr_ihdr =
    {mng_it_png, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_ihdr,
     mng_fields_ihdr, (sizeof(mng_fields_ihdr) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL,
     MNG_NULL,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOJHDR | MNG_DESCR_NOBASI | MNG_DESCR_NOIDAT | MNG_DESCR_NOPLTE};

mng_chunk_descriptor mng_chunk_descr_plte =
    {mng_it_png, mng_create_none, 0, offsetof(mng_plte, bEmpty),
     MNG_NULL, MNG_NULL, MNG_NULL,
     mng_fields_plte, (sizeof(mng_fields_plte) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA | MNG_DESCR_NOPLTE};

mng_chunk_descriptor mng_chunk_descr_idat =
    {mng_it_png, mng_create_none, 0, offsetof(mng_idat, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_idat,
     mng_fields_idat, (sizeof(mng_fields_idat) / sizeof(mng_field_descriptor)),
     MNG_DESCR_EMPTYEMBED,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOJSEP};

mng_chunk_descriptor mng_chunk_descr_iend =
    {mng_it_png, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_iend,
     MNG_NULL, 0,
     MNG_DESCR_EMPTY | MNG_DESCR_EMPTYEMBED,
     MNG_DESCR_GenHDR,
     MNG_NULL};

mng_chunk_descriptor mng_chunk_descr_trns =
    {mng_it_png, mng_create_none, 0, offsetof(mng_trns, bEmpty),
     MNG_NULL, MNG_NULL, MNG_NULL,
     MNG_NULL, MNG_NULL,
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR | MNG_DESCR_PLTE,
     MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};

#ifndef MNG_SKIPCHUNK_gAMA
mng_chunk_descriptor mng_chunk_descr_gama =
    {mng_it_png, mng_create_none, 0, offsetof(mng_gama, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_gama,
     mng_fields_gama, (sizeof(mng_fields_gama) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOPLTE | MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_cHRM
mng_chunk_descriptor mng_chunk_descr_chrm =
    {mng_it_png, mng_create_none, 0, offsetof(mng_chrm, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_chrm,
     mng_fields_chrm, (sizeof(mng_fields_chrm) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOPLTE | MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_sRGB
mng_chunk_descriptor mng_chunk_descr_srgb =
    {mng_it_png, mng_create_none, 0, offsetof(mng_srgb, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_srgb,
     mng_fields_srgb, (sizeof(mng_fields_srgb) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOPLTE | MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_iCCP
mng_chunk_descriptor mng_chunk_descr_iccp =
    {mng_it_png, mng_create_none, 0, offsetof(mng_iccp, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_iccp,
     mng_fields_iccp, (sizeof(mng_fields_iccp) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOPLTE | MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_tEXt
mng_chunk_descriptor mng_chunk_descr_text =
    {mng_it_png, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_text,
     mng_fields_text, (sizeof(mng_fields_text) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL,
     MNG_DESCR_GenHDR,
     MNG_NULL};
#endif

#ifndef MNG_SKIPCHUNK_zTXt
mng_chunk_descriptor mng_chunk_descr_ztxt =
    {mng_it_png, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_ztxt,
     mng_fields_ztxt, (sizeof(mng_fields_ztxt) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL,
     MNG_DESCR_GenHDR,
     MNG_NULL};
#endif

#ifndef MNG_SKIPCHUNK_iTXt
mng_chunk_descriptor mng_chunk_descr_itxt =
    {mng_it_png, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_itxt,
     mng_fields_itxt, (sizeof(mng_fields_itxt) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL,
     MNG_DESCR_GenHDR,
     MNG_NULL};
#endif

#ifndef MNG_SKIPCHUNK_bKGD
mng_chunk_descriptor mng_chunk_descr_bkgd =
    {mng_it_png, mng_create_none, 0, offsetof(mng_bkgd, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_bkgd,
     mng_fields_bkgd, (sizeof(mng_fields_bkgd) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_pHYs
mng_chunk_descriptor mng_chunk_descr_phys =
    {mng_it_png, mng_create_none, 0, offsetof(mng_phys, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_phys,
     mng_fields_phys, (sizeof(mng_fields_phys) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_sBIT
mng_chunk_descriptor mng_chunk_descr_sbit =
    {mng_it_png, mng_create_none, 0, offsetof(mng_sbit, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_sbit,
     mng_fields_sbit, (sizeof(mng_fields_sbit) / sizeof(mng_field_descriptor)),
     MNG_DESCR_GLOBAL | MNG_DESCR_EMPTYEMBED | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_GenHDR,
     MNG_DESCR_NOIDAT | MNG_DESCR_NOJDAT | MNG_DESCR_NOJDAA};
#endif

#ifndef MNG_SKIPCHUNK_tIME
mng_chunk_descriptor mng_chunk_descr_time =
    {mng_it_png, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_time,
     mng_fields_time, (sizeof(mng_fields_time) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_GenHDR,
     MNG_NULL};
#endif

/* ************************************************************************** */
/* JNG chunks */


/* ************************************************************************** */
/* MNG chunks */

mng_chunk_descriptor mng_chunk_descr_mhdr =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_mhdr,
     mng_fields_mhdr, (sizeof(mng_fields_mhdr) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_NULL,
     MNG_DESCR_NOMHDR | MNG_DESCR_NOIHDR | MNG_DESCR_NOJHDR};

mng_chunk_descriptor mng_chunk_descr_mend =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_mend,
     MNG_NULL, 0,
     MNG_DESCR_EMPTY | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_MHDR,
     MNG_NULL};

#ifndef MNG_SKIPCHUNK_LOOP
mng_chunk_descriptor mng_chunk_descr_endl =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_endl,
     mng_fields_endl, (sizeof(mng_fields_endl) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_DEFI
mng_chunk_descriptor mng_chunk_descr_defi =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_defi,
     mng_fields_defi, (sizeof(mng_fields_defi) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_BASI
mng_chunk_descriptor mng_chunk_descr_basi =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_basi,
     mng_fields_basi, (sizeof(mng_fields_basi) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_CLON
mng_chunk_descriptor mng_chunk_descr_clon =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_clon,
     mng_fields_clon, (sizeof(mng_fields_clon) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_BACK
mng_chunk_descriptor mng_chunk_descr_back =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_back,
     mng_fields_back, (sizeof(mng_fields_back) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_MOVE
mng_chunk_descriptor mng_chunk_descr_move =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_move,
     mng_fields_move, (sizeof(mng_fields_move) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_CLIP
mng_chunk_descriptor mng_chunk_descr_clip =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_clip,
     mng_fields_clip, (sizeof(mng_fields_clip) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_SHOW
mng_chunk_descriptor mng_chunk_descr_show =
    {mng_it_mng, mng_create_none, 0, offsetof(mng_show, bEmpty),
     MNG_NULL, MNG_NULL, mng_special_show,
     mng_fields_show, (sizeof(mng_fields_show) / sizeof(mng_field_descriptor)),
     MNG_DESCR_EMPTY | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

#ifndef MNG_SKIPCHUNK_TERM
mng_chunk_descriptor mng_chunk_descr_term =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_term,
     mng_fields_term, (sizeof(mng_fields_term) / sizeof(mng_field_descriptor)),
     MNG_NULL,
     MNG_DESCR_MHDR,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR | MNG_DESCR_NOTERM | MNG_DESCR_NOLOOP};
#endif

#ifndef MNG_SKIPCHUNK_SEEK
mng_chunk_descriptor mng_chunk_descr_seek =
    {mng_it_mng, mng_create_none, 0, 0,
     MNG_NULL, MNG_NULL, mng_special_seek,
     mng_fields_seek, (sizeof(mng_fields_seek) / sizeof(mng_field_descriptor)),
     MNG_DESCR_EMPTY | MNG_DESCR_EMPTYGLOBAL,
     MNG_DESCR_MHDR | MNG_DESCR_SAVE,
     MNG_DESCR_NOIHDR | MNG_DESCR_NOBASI | MNG_DESCR_NODHDR | MNG_DESCR_NOJHDR};
#endif

/* ************************************************************************** */
/* ************************************************************************** */

mng_chunk_header mng_chunk_unknown = {MNG_UINT_HUH, mng_init_general, mng_free_unknown,
                                      mng_read_unknown, mng_write_unknown, mng_assign_unknown, 0, 0, sizeof(mng_unknown_chunk)};

/* ************************************************************************** */

  /* the table-idea & binary search code was adapted from
     libpng 1.1.0 (pngread.c) */
  /* NOTE1: the table must remain sorted by chunkname, otherwise the binary
     search will break !!! (ps. watch upper-/lower-case chunknames !!) */
  /* NOTE2: the layout must remain equal to the header part of all the
     chunk-structures (yes, that means even the pNext and pPrev fields;
     it's wasting a bit of space, but hey, the code is a lot easier) */

mng_chunk_header mng_chunk_table [] =
  {
#ifndef MNG_SKIPCHUNK_BACK
    {MNG_UINT_BACK, mng_init_general, mng_free_general, mng_read_general, mng_write_back, mng_assign_general, 0, 0, sizeof(mng_back), &mng_chunk_descr_back},
#endif
#ifndef MNG_SKIPCHUNK_BASI
    {MNG_UINT_BASI, mng_init_general, mng_free_general, mng_read_general, mng_write_basi, mng_assign_general, 0, 0, sizeof(mng_basi), &mng_chunk_descr_basi},
#endif
#ifndef MNG_SKIPCHUNK_CLIP
    {MNG_UINT_CLIP, mng_init_general, mng_free_general, mng_read_general, mng_write_clip, mng_assign_general, 0, 0, sizeof(mng_clip), &mng_chunk_descr_clip},
#endif
#ifndef MNG_SKIPCHUNK_CLON
    {MNG_UINT_CLON, mng_init_general, mng_free_general, mng_read_general, mng_write_clon, mng_assign_general, 0, 0, sizeof(mng_clon), &mng_chunk_descr_clon},
#endif
#ifndef MNG_NO_DELTA_PNG
#ifndef MNG_SKIPCHUNK_DBYK
    {MNG_UINT_DBYK, mng_init_general, mng_free_dbyk,    mng_read_dbyk,    mng_write_dbyk, mng_assign_dbyk,    0, 0, sizeof(mng_dbyk), MNG_NULL},
#endif
#endif
#ifndef MNG_SKIPCHUNK_DEFI
    {MNG_UINT_DEFI, mng_init_general, mng_free_general, mng_read_general, mng_write_defi, mng_assign_general, 0, 0, sizeof(mng_defi), &mng_chunk_descr_defi},
#endif
#ifndef MNG_NO_DELTA_PNG
    {MNG_UINT_DHDR, mng_init_general, mng_free_general, mng_read_dhdr,    mng_write_dhdr, mng_assign_general, 0, 0, sizeof(mng_dhdr), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_DISC
    {MNG_UINT_DISC, mng_init_general, mng_free_disc,    mng_read_disc,    mng_write_disc, mng_assign_disc,    0, 0, sizeof(mng_disc), MNG_NULL},
#endif
#ifndef MNG_NO_DELTA_PNG
#ifndef MNG_SKIPCHUNK_DROP
    {MNG_UINT_DROP, mng_init_general, mng_free_drop,    mng_read_drop,    mng_write_drop, mng_assign_drop,    0, 0, sizeof(mng_drop), MNG_NULL},
#endif
#endif
#ifndef MNG_SKIPCHUNK_LOOP
    {MNG_UINT_ENDL, mng_init_general, mng_free_general, mng_read_general, mng_write_endl, mng_assign_general, 0, 0, sizeof(mng_endl), &mng_chunk_descr_endl},
#endif
#ifndef MNG_SKIPCHUNK_FRAM
    {MNG_UINT_FRAM, mng_init_general, mng_free_fram,    mng_read_fram,    mng_write_fram, mng_assign_fram,    0, 0, sizeof(mng_fram), MNG_NULL},
#endif
    {MNG_UINT_IDAT, mng_init_general, mng_free_idat,    mng_read_general, mng_write_idat, mng_assign_idat,    0, 0, sizeof(mng_idat), &mng_chunk_descr_idat},  /* 12-th element! */
    {MNG_UINT_IEND, mng_init_general, mng_free_general, mng_read_general, mng_write_iend, mng_assign_general, 0, 0, sizeof(mng_iend), &mng_chunk_descr_iend},
    {MNG_UINT_IHDR, mng_init_general, mng_free_general, mng_read_general, mng_write_ihdr, mng_assign_general, 0, 0, sizeof(mng_ihdr), &mng_chunk_descr_ihdr},
#ifndef MNG_NO_DELTA_PNG
#ifdef MNG_INCLUDE_JNG
    {MNG_UINT_IJNG, mng_init_general, mng_free_general, mng_read_ijng,    mng_write_ijng, mng_assign_general, 0, 0, sizeof(mng_ijng), MNG_NULL},
#endif
    {MNG_UINT_IPNG, mng_init_general, mng_free_general, mng_read_ipng,    mng_write_ipng, mng_assign_general, 0, 0, sizeof(mng_ipng), MNG_NULL},
#endif
#ifdef MNG_INCLUDE_JNG
    {MNG_UINT_JDAA, mng_init_general, mng_free_jdaa,    mng_read_jdaa,    mng_write_jdaa, mng_assign_jdaa,    0, 0, sizeof(mng_jdaa), MNG_NULL},
    {MNG_UINT_JDAT, mng_init_general, mng_free_jdat,    mng_read_jdat,    mng_write_jdat, mng_assign_jdat,    0, 0, sizeof(mng_jdat), MNG_NULL},
    {MNG_UINT_JHDR, mng_init_general, mng_free_general, mng_read_jhdr,    mng_write_jhdr, mng_assign_general, 0, 0, sizeof(mng_jhdr), MNG_NULL},
    {MNG_UINT_JSEP, mng_init_general, mng_free_general, mng_read_jsep,    mng_write_jsep, mng_assign_general, 0, 0, sizeof(mng_jsep), MNG_NULL},
    {MNG_UINT_JdAA, mng_init_general, mng_free_jdaa,    mng_read_jdaa,    mng_write_jdaa, mng_assign_jdaa,    0, 0, sizeof(mng_jdaa), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_LOOP
    {MNG_UINT_LOOP, mng_init_general, mng_free_loop,    mng_read_loop,    mng_write_loop, mng_assign_loop,    0, 0, sizeof(mng_loop), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_MAGN
    {MNG_UINT_MAGN, mng_init_general, mng_free_general, mng_read_magn,    mng_write_magn, mng_assign_general, 0, 0, sizeof(mng_magn), MNG_NULL},
#endif
    {MNG_UINT_MEND, mng_init_general, mng_free_general, mng_read_general, mng_write_mend, mng_assign_general, 0, 0, sizeof(mng_mend), &mng_chunk_descr_mend},
    {MNG_UINT_MHDR, mng_init_general, mng_free_general, mng_read_general, mng_write_mhdr, mng_assign_general, 0, 0, sizeof(mng_mhdr), &mng_chunk_descr_mhdr},
#ifndef MNG_SKIPCHUNK_MOVE
    {MNG_UINT_MOVE, mng_init_general, mng_free_general, mng_read_general, mng_write_move, mng_assign_general, 0, 0, sizeof(mng_move), &mng_chunk_descr_move},
#endif
#ifndef MNG_NO_DELTA_PNG
#ifndef MNG_SKIPCHUNK_ORDR
    {MNG_UINT_ORDR, mng_init_general, mng_free_ordr,    mng_read_ordr,    mng_write_ordr, mng_assign_ordr,    0, 0, sizeof(mng_ordr), MNG_NULL},
#endif
#endif
#ifndef MNG_SKIPCHUNK_PAST
    {MNG_UINT_PAST, mng_init_general, mng_free_past,    mng_read_past,    mng_write_past, mng_assign_past,    0, 0, sizeof(mng_past), MNG_NULL},
#endif
    {MNG_UINT_PLTE, mng_init_general, mng_free_general, mng_read_plte,    mng_write_plte, mng_assign_general, 0, 0, sizeof(mng_plte), &mng_chunk_descr_plte},
#ifndef MNG_NO_DELTA_PNG
    {MNG_UINT_PPLT, mng_init_general, mng_free_general, mng_read_pplt,    mng_write_pplt, mng_assign_general, 0, 0, sizeof(mng_pplt), MNG_NULL},
    {MNG_UINT_PROM, mng_init_general, mng_free_general, mng_read_prom,    mng_write_prom, mng_assign_general, 0, 0, sizeof(mng_prom), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_SAVE
    {MNG_UINT_SAVE, mng_init_general, mng_free_save,    mng_read_save,    mng_write_save, mng_assign_save,    0, 0, sizeof(mng_save), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_SEEK
    {MNG_UINT_SEEK, mng_init_general, mng_free_seek,    mng_read_general, mng_write_seek, mng_assign_seek,    0, 0, sizeof(mng_seek), &mng_chunk_descr_seek},
#endif
#ifndef MNG_SKIPCHUNK_SHOW
    {MNG_UINT_SHOW, mng_init_general, mng_free_general, mng_read_general, mng_write_show, mng_assign_general, 0, 0, sizeof(mng_show), &mng_chunk_descr_show},
#endif
#ifndef MNG_SKIPCHUNK_TERM
    {MNG_UINT_TERM, mng_init_general, mng_free_general, mng_read_general, mng_write_term, mng_assign_general, 0, 0, sizeof(mng_term), &mng_chunk_descr_term},
#endif
#ifndef MNG_SKIPCHUNK_bKGD
    {MNG_UINT_bKGD, mng_init_general, mng_free_general, mng_read_general, mng_write_bkgd, mng_assign_general, 0, 0, sizeof(mng_bkgd), &mng_chunk_descr_bkgd},
#endif
#ifndef MNG_SKIPCHUNK_cHRM
    {MNG_UINT_cHRM, mng_init_general, mng_free_general, mng_read_general, mng_write_chrm, mng_assign_general, 0, 0, sizeof(mng_chrm), &mng_chunk_descr_chrm},
#endif
#ifndef MNG_SKIPCHUNK_eXPI
    {MNG_UINT_eXPI, mng_init_general, mng_free_expi,    mng_read_expi,    mng_write_expi, mng_assign_expi,    0, 0, sizeof(mng_expi), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_evNT
    {MNG_UINT_evNT, mng_init_general, mng_free_evnt,    mng_read_evnt,    mng_write_evnt, mng_assign_evnt,    0, 0, sizeof(mng_evnt), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_fPRI
    {MNG_UINT_fPRI, mng_init_general, mng_free_general, mng_read_fpri,    mng_write_fpri, mng_assign_general, 0, 0, sizeof(mng_fpri), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_gAMA
    {MNG_UINT_gAMA, mng_init_general, mng_free_general, mng_read_general, mng_write_gama, mng_assign_general, 0, 0, sizeof(mng_gama), &mng_chunk_descr_gama},
#endif
#ifndef MNG_SKIPCHUNK_hIST
    {MNG_UINT_hIST, mng_init_general, mng_free_general, mng_read_hist,    mng_write_hist, mng_assign_general, 0, 0, sizeof(mng_hist), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_iCCP
    {MNG_UINT_iCCP, mng_init_general, mng_free_iccp,    mng_read_general, mng_write_iccp, mng_assign_iccp,    0, 0, sizeof(mng_iccp), &mng_chunk_descr_iccp},
#endif
#ifndef MNG_SKIPCHUNK_iTXt
    {MNG_UINT_iTXt, mng_init_general, mng_free_itxt,    mng_read_general, mng_write_itxt, mng_assign_itxt,    0, 0, sizeof(mng_itxt), &mng_chunk_descr_itxt},
#endif
#ifndef MNG_SKIPCHUNK_nEED
    {MNG_UINT_nEED, mng_init_general, mng_free_need,    mng_read_need,    mng_write_need, mng_assign_need,    0, 0, sizeof(mng_need), MNG_NULL},
#endif
/* TODO:     {MNG_UINT_oFFs, 0, 0, 0, 0, 0, 0},  */
/* TODO:     {MNG_UINT_pCAL, 0, 0, 0, 0, 0, 0},  */
#ifndef MNG_SKIPCHUNK_pHYg
    {MNG_UINT_pHYg, mng_init_general, mng_free_general, mng_read_phyg,    mng_write_phyg, mng_assign_general, 0, 0, sizeof(mng_phyg), MNG_NULL},
#endif
#ifndef MNG_SKIPCHUNK_pHYs
    {MNG_UINT_pHYs, mng_init_general, mng_free_general, mng_read_general, mng_write_phys, mng_assign_general, 0, 0, sizeof(mng_phys), &mng_chunk_descr_phys},
#endif
#ifndef MNG_SKIPCHUNK_sBIT
    {MNG_UINT_sBIT, mng_init_general, mng_free_general, mng_read_general, mng_write_sbit, mng_assign_general, 0, 0, sizeof(mng_sbit), &mng_chunk_descr_sbit},
#endif
/* TODO:     {MNG_UINT_sCAL, 0, 0, 0, 0, 0, 0},  */
#ifndef MNG_SKIPCHUNK_sPLT
    {MNG_UINT_sPLT, mng_init_general, mng_free_splt,    mng_read_splt,    mng_write_splt, mng_assign_splt,    0, 0, sizeof(mng_splt), MNG_NULL},
#endif
    {MNG_UINT_sRGB, mng_init_general, mng_free_general, mng_read_general, mng_write_srgb, mng_assign_general, 0, 0, sizeof(mng_srgb), &mng_chunk_descr_srgb},
#ifndef MNG_SKIPCHUNK_tEXt
    {MNG_UINT_tEXt, mng_init_general, mng_free_text,    mng_read_general, mng_write_text, mng_assign_text,    0, 0, sizeof(mng_text), &mng_chunk_descr_text},
#endif
#ifndef MNG_SKIPCHUNK_tIME
    {MNG_UINT_tIME, mng_init_general, mng_free_general, mng_read_general, mng_write_time, mng_assign_general, 0, 0, sizeof(mng_time), &mng_chunk_descr_time},
#endif
    {MNG_UINT_tRNS, mng_init_general, mng_free_general, mng_read_trns,    mng_write_trns, mng_assign_general, 0, 0, sizeof(mng_trns), &mng_chunk_descr_trns},
#ifndef MNG_SKIPCHUNK_zTXt
    {MNG_UINT_zTXt, mng_init_general, mng_free_ztxt,    mng_read_general, mng_write_ztxt, mng_assign_ztxt,    0, 0, sizeof(mng_ztxt), &mng_chunk_descr_ztxt},
#endif
  };

/* ************************************************************************** */
/* ************************************************************************** */

void mng_get_chunkheader (mng_chunkid       iChunkname,
                          mng_chunk_headerp pResult)
{
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

MNG_C_SPECIALFUNC (mng_special_ihdr)
{
  pData->bHasIHDR      = MNG_TRUE;     /* indicate IHDR is present */
                                       /* and store interesting fields */
  if ((!pData->bHasDHDR) || (pData->iDeltatype == MNG_DELTATYPE_NOCHANGE))
  {
    pData->iDatawidth  = ((mng_ihdrp)pChunk)->iWidth;
    pData->iDataheight = ((mng_ihdrp)pChunk)->iHeight;
  }

  pData->iBitdepth     = ((mng_ihdrp)pChunk)->iBitdepth;
  pData->iColortype    = ((mng_ihdrp)pChunk)->iColortype;
  pData->iCompression  = ((mng_ihdrp)pChunk)->iCompression;
  pData->iFilter       = ((mng_ihdrp)pChunk)->iFilter;
  pData->iInterlace    = ((mng_ihdrp)pChunk)->iInterlace;

#if defined(MNG_NO_1_2_4BIT_SUPPORT) || defined(MNG_NO_16BIT_SUPPORT)
  pData->iPNGmult = 1;
  pData->iPNGdepth = pData->iBitdepth;
#endif

#ifdef MNG_NO_1_2_4BIT_SUPPORT
  if (pData->iBitdepth < 8)
      pData->iBitdepth = 8;
#endif

#ifdef MNG_NO_16BIT_SUPPORT
  if (pData->iBitdepth > 8)
    {
      pData->iBitdepth = 8;
      pData->iPNGmult = 2;
    }
#endif

  if ((pData->iBitdepth !=  8)      /* parameter validity checks */
#ifndef MNG_NO_1_2_4BIT_SUPPORT
      && (pData->iBitdepth !=  1) &&
      (pData->iBitdepth !=  2) &&
      (pData->iBitdepth !=  4)
#endif
#ifndef MNG_NO_16BIT_SUPPORT
      && (pData->iBitdepth != 16)   
#endif
      )
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

  if ((pData->iColortype != MNG_COLORTYPE_GRAY   ) &&
      (pData->iColortype != MNG_COLORTYPE_RGB    ) &&
      (pData->iColortype != MNG_COLORTYPE_INDEXED) &&
      (pData->iColortype != MNG_COLORTYPE_GRAYA  ) &&
      (pData->iColortype != MNG_COLORTYPE_RGBA   )    )
    MNG_ERROR (pData, MNG_INVALIDCOLORTYPE)

  if ((pData->iColortype == MNG_COLORTYPE_INDEXED) && (pData->iBitdepth > 8))
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

  if (((pData->iColortype == MNG_COLORTYPE_RGB    ) ||
       (pData->iColortype == MNG_COLORTYPE_GRAYA  ) ||
       (pData->iColortype == MNG_COLORTYPE_RGBA   )    ) &&
      (pData->iBitdepth < 8                            )    )
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

  if (pData->iCompression != MNG_COMPRESSION_DEFLATE)
    MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

#if defined(FILTER192) || defined(FILTER193)
  if ((pData->iFilter != MNG_FILTER_ADAPTIVE ) &&
#if defined(FILTER192) && defined(FILTER193)
      (pData->iFilter != MNG_FILTER_DIFFERING) &&
      (pData->iFilter != MNG_FILTER_NOFILTER )    )
#else
#ifdef FILTER192
      (pData->iFilter != MNG_FILTER_DIFFERING)    )
#else
      (pData->iFilter != MNG_FILTER_NOFILTER )    )
#endif
#endif
    MNG_ERROR (pData, MNG_INVALIDFILTER)
#else
  if (pData->iFilter)
    MNG_ERROR (pData, MNG_INVALIDFILTER)
#endif

  if ((pData->iInterlace != MNG_INTERLACE_NONE ) &&
      (pData->iInterlace != MNG_INTERLACE_ADAM7)    )
    MNG_ERROR (pData, MNG_INVALIDINTERLACE)

#ifdef MNG_SUPPORT_DISPLAY 
#ifndef MNG_NO_DELTA_PNG
  if (pData->bHasDHDR)                 /* check the colortype for delta-images ! */
  {
    mng_imagedatap pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

    if (pData->iColortype != pBuf->iColortype)
    {
      if ( ( (pData->iColortype != MNG_COLORTYPE_INDEXED) ||
             (pBuf->iColortype  == MNG_COLORTYPE_GRAY   )    ) &&
           ( (pData->iColortype != MNG_COLORTYPE_GRAY   ) ||
             (pBuf->iColortype  == MNG_COLORTYPE_INDEXED)    )    )
        MNG_ERROR (pData, MNG_INVALIDCOLORTYPE)
    }
  }
#endif
#endif

  if (!pData->bHasheader)              /* first chunk ? */
  {
    pData->bHasheader = MNG_TRUE;      /* we've got a header */
    pData->eImagetype = mng_it_png;    /* then this must be a PNG */
    pData->iWidth     = pData->iDatawidth;
    pData->iHeight    = pData->iDataheight;
                                       /* predict alpha-depth ! */
    if ((pData->iColortype == MNG_COLORTYPE_GRAYA  ) ||
        (pData->iColortype == MNG_COLORTYPE_RGBA   )    )
      pData->iAlphadepth = pData->iBitdepth;
    else
    if (pData->iColortype == MNG_COLORTYPE_INDEXED)
      pData->iAlphadepth = 8;          /* worst case scenario */
    else
      pData->iAlphadepth = 1;  /* Possible tRNS cheap binary transparency */
                                       /* fits on maximum canvas ? */
    if ((pData->iWidth > pData->iMaxwidth) || (pData->iHeight > pData->iMaxheight))
      MNG_WARNING (pData, MNG_IMAGETOOLARGE)

    if (pData->fProcessheader)         /* inform the app ? */
      if (!pData->fProcessheader (((mng_handle)pData), pData->iWidth, pData->iHeight))
        MNG_ERROR (pData, MNG_APPMISCERROR)
  }

  if (!pData->bHasDHDR)
    pData->iImagelevel++;              /* one level deeper */

#ifdef MNG_SUPPORT_DISPLAY
  {
    mng_retcode iRetcode = mng_process_display_ihdr (pData);
    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

MNG_C_SPECIALFUNC (mng_special_idat)
{
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasJHDR) &&
      (pData->iJHDRalphacompression != MNG_COMPRESSION_DEFLATE))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#endif
                                       /* not allowed for deltatype NO_CHANGE */
#ifndef MNG_NO_DELTA_PNG
  if ((pData->bHasDHDR) && ((pData->iDeltatype == MNG_DELTATYPE_NOCHANGE)))
    MNG_ERROR (pData, MNG_CHUNKNOTALLOWED)
#endif
                                       /* can only be empty in BASI-block! */
  if ((((mng_idatp)pChunk)->bEmpty) && (!pData->bHasBASI))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)
                                       /* indexed-color requires PLTE */
  if ((pData->bHasIHDR) && (pData->iColortype == 3) && (!pData->bHasPLTE))
    MNG_ERROR (pData, MNG_PLTEMISSING)

  pData->bHasIDAT = MNG_TRUE;          /* got some IDAT now, don't we */

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

MNG_C_SPECIALFUNC (mng_special_iend)
{
                                       /* IHDR-block requires IDAT */
  if ((pData->bHasIHDR) && (!pData->bHasIDAT))
    MNG_ERROR (pData, MNG_IDATMISSING)

  pData->iImagelevel--;                /* one level up */

#ifdef MNG_SUPPORT_DISPLAY
  {                                    /* create an animation object */
    mng_retcode iRetcode = mng_create_ani_image (pData);
    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* display processing */
    iRetcode = mng_process_display_iend (pData);
    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_SUPPORT_DISPLAY
  if (!pData->bTimerset)               /* reset only if not broken !!! */
  {
#endif
                                       /* IEND signals the end for most ... */
    pData->bHasIHDR         = MNG_FALSE;
    pData->bHasBASI         = MNG_FALSE;
    pData->bHasDHDR         = MNG_FALSE;
#ifdef MNG_INCLUDE_JNG
    pData->bHasJHDR         = MNG_FALSE;
    pData->bHasJSEP         = MNG_FALSE;
    pData->bHasJDAA         = MNG_FALSE;
    pData->bHasJDAT         = MNG_FALSE;
#endif
    pData->bHasPLTE         = MNG_FALSE;
    pData->bHasTRNS         = MNG_FALSE;
    pData->bHasGAMA         = MNG_FALSE;
    pData->bHasCHRM         = MNG_FALSE;
    pData->bHasSRGB         = MNG_FALSE;
    pData->bHasICCP         = MNG_FALSE;
    pData->bHasBKGD         = MNG_FALSE;
    pData->bHasIDAT         = MNG_FALSE;
#ifdef MNG_SUPPORT_DISPLAY
  }
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

MNG_C_SPECIALFUNC (mng_special_gama)
{
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasGAMA = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalGAMA = (mng_bool)!((mng_gamap)pChunk)->bEmpty;

#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {
    mng_imagep pImage;

#ifndef MNG_NO_DELTA_PNG
    if (pData->bHasDHDR)               /* update delta image ? */
      pImage = (mng_imagep)pData->pObjzero;
    else
#endif
    {
      pImage = (mng_imagep)pData->pCurrentobj;
      if (!pImage)                     /* no object then dump it in obj 0 */
        pImage = (mng_imagep)pData->pObjzero;
    }
                                       /* store for color-processing routines */
    pImage->pImgbuf->iGamma   = ((mng_gamap)pChunk)->iGamma;
    pImage->pImgbuf->bHasGAMA = MNG_TRUE;
  }
  else
  {                                    /* store as global */
    if (!((mng_gamap)pChunk)->bEmpty)
      pData->iGlobalGamma = ((mng_gamap)pChunk)->iGamma;

    {                                  /* create an animation object */
      mng_retcode iRetcode = mng_create_ani_gama (pData, ((mng_gamap)pChunk)->bEmpty,
                                                  pData->iGlobalGamma);
      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_cHRM
MNG_C_SPECIALFUNC (mng_special_chrm)
{
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasCHRM = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalCHRM = (mng_bool)!((mng_chrmp)pChunk)->bEmpty;

#ifdef MNG_SUPPORT_DISPLAY
  {
#ifdef MNG_INCLUDE_JNG
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    {
      mng_imagep     pImage;
      mng_imagedatap pBuf;

#ifndef MNG_NO_DELTA_PNG
      if (pData->bHasDHDR)             /* update delta image ? */
        pImage = (mng_imagep)pData->pObjzero;
      else
#endif
      {
        pImage = (mng_imagep)pData->pCurrentobj;
        if (!pImage)                   /* no object then dump it in obj 0 */
          pImage = (mng_imagep)pData->pObjzero;
      }

      pBuf = pImage->pImgbuf;          /* address object buffer */
      pBuf->bHasCHRM = MNG_TRUE;       /* and tell it it's got a CHRM now */
                                       /* store for color-processing routines */
      pBuf->iWhitepointx   = ((mng_chrmp)pChunk)->iWhitepointx;
      pBuf->iWhitepointy   = ((mng_chrmp)pChunk)->iWhitepointy;
      pBuf->iPrimaryredx   = ((mng_chrmp)pChunk)->iRedx;
      pBuf->iPrimaryredy   = ((mng_chrmp)pChunk)->iRedy;
      pBuf->iPrimarygreenx = ((mng_chrmp)pChunk)->iGreenx;
      pBuf->iPrimarygreeny = ((mng_chrmp)pChunk)->iGreeny;
      pBuf->iPrimarybluex  = ((mng_chrmp)pChunk)->iBluex;
      pBuf->iPrimarybluey  = ((mng_chrmp)pChunk)->iBluey;
    }
    else
    {                                  /* store as global */
      if (!((mng_chrmp)pChunk)->bEmpty)
      {
        pData->iGlobalWhitepointx   = ((mng_chrmp)pChunk)->iWhitepointx;
        pData->iGlobalWhitepointy   = ((mng_chrmp)pChunk)->iWhitepointy;
        pData->iGlobalPrimaryredx   = ((mng_chrmp)pChunk)->iRedx;
        pData->iGlobalPrimaryredy   = ((mng_chrmp)pChunk)->iRedy;
        pData->iGlobalPrimarygreenx = ((mng_chrmp)pChunk)->iGreenx;
        pData->iGlobalPrimarygreeny = ((mng_chrmp)pChunk)->iGreeny;
        pData->iGlobalPrimarybluex  = ((mng_chrmp)pChunk)->iBluex;
        pData->iGlobalPrimarybluey  = ((mng_chrmp)pChunk)->iBluey;
      }

      {                                /* create an animation object */
        mng_retcode iRetcode = mng_create_ani_chrm (pData, ((mng_chrmp)pChunk)->bEmpty,
                                                    ((mng_chrmp)pChunk)->iWhitepointx,
                                                    ((mng_chrmp)pChunk)->iWhitepointy,
                                                    ((mng_chrmp)pChunk)->iRedx,
                                                    ((mng_chrmp)pChunk)->iRedy,
                                                    ((mng_chrmp)pChunk)->iGreenx,
                                                    ((mng_chrmp)pChunk)->iGreeny,
                                                    ((mng_chrmp)pChunk)->iBluex,
                                                    ((mng_chrmp)pChunk)->iBluey);
        if (iRetcode)                  /* on error bail out */
          return iRetcode;
      }
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

MNG_C_SPECIALFUNC (mng_special_srgb)
{
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasSRGB = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalSRGB = (mng_bool)!((mng_srgbp)pChunk)->bEmpty;

#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {
    mng_imagep pImage;

#ifndef MNG_NO_DELTA_PNG
    if (pData->bHasDHDR)               /* update delta image ? */
      pImage = (mng_imagep)pData->pObjzero;
    else
#endif
    {
      pImage = (mng_imagep)pData->pCurrentobj;
      if (!pImage)                     /* no object then dump it in obj 0 */
        pImage = (mng_imagep)pData->pObjzero;
    }
                                       /* store for color-processing routines */
    pImage->pImgbuf->iRenderingintent = ((mng_srgbp)pChunk)->iRenderingintent;
    pImage->pImgbuf->bHasSRGB         = MNG_TRUE;
  }
  else
  {                                    /* store as global */
    if (!((mng_srgbp)pChunk)->bEmpty)
      pData->iGlobalRendintent = ((mng_srgbp)pChunk)->iRenderingintent;

    {                                  /* create an animation object */
      mng_retcode iRetcode = mng_create_ani_srgb (pData, ((mng_srgbp)pChunk)->bEmpty,
                                                  pData->iGlobalRendintent);
      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_iCCP
MNG_C_SPECIALFUNC (mng_special_iccp)
{
  mng_retcode       iRetcode;
  mng_chunk_headerp pDummy;

#ifdef MNG_CHECK_BAD_ICCP              /* Check for bad iCCP chunk */
  if (!strncmp (((mng_iccpp)pChunk)->zName, "Photoshop ICC profile", 21))
  {
    if (((mng_iccpp)pChunk)->iProfilesize == 2615) /* is it the sRGB profile ? */
    {
      mng_chunk_header chunk_srgb;
      mng_get_chunkheader (MNG_UINT_sRGB, &chunk_srgb);
                                       /* pretend it's an sRGB chunk then ! */
      iRetcode = mng_read_general (pData, &chunk_srgb, 1, (mng_ptr)"0", &pDummy);
      if (iRetcode)                    /* on error bail out */
        return iRetcode;

      pDummy->fCleanup (pData, pDummy);  
    }
  }
  else
  {
#endif /* MNG_CHECK_BAD_ICCP */

#ifdef MNG_INCLUDE_JNG
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
      pData->bHasICCP = MNG_TRUE;      /* indicate we've got it */
    else
      pData->bHasglobalICCP = (mng_bool)!((mng_iccpp)pChunk)->bEmpty;

#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_INCLUDE_JNG
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    {
      mng_imagep pImage;

#ifndef MNG_NO_DELTA_PNG
      if (pData->bHasDHDR)             /* update delta image ? */
      {                                /* store in object 0 ! */
        pImage = (mng_imagep)pData->pObjzero;

        if (pImage->pImgbuf->pProfile) /* profile existed ? */
          MNG_FREEX (pData, pImage->pImgbuf->pProfile, pImage->pImgbuf->iProfilesize)
                                       /* allocate a buffer & copy it */
        MNG_ALLOC (pData, pImage->pImgbuf->pProfile, ((mng_iccpp)pChunk)->iProfilesize)
        MNG_COPY  (pImage->pImgbuf->pProfile, ((mng_iccpp)pChunk)->pProfile, ((mng_iccpp)pChunk)->iProfilesize)
                                       /* store it's length as well */
        pImage->pImgbuf->iProfilesize = ((mng_iccpp)pChunk)->iProfilesize;
        pImage->pImgbuf->bHasICCP     = MNG_TRUE;
      }
      else
#endif
      {
        pImage = (mng_imagep)pData->pCurrentobj;

        if (!pImage)                   /* no object then dump it in obj 0 */
          pImage = (mng_imagep)pData->pObjzero;

        if (pImage->pImgbuf->pProfile) /* profile existed ? */
          MNG_FREEX (pData, pImage->pImgbuf->pProfile, pImage->pImgbuf->iProfilesize)
                                       /* allocate a buffer & copy it */
        MNG_ALLOC (pData, pImage->pImgbuf->pProfile, ((mng_iccpp)pChunk)->iProfilesize)
        MNG_COPY  (pImage->pImgbuf->pProfile, ((mng_iccpp)pChunk)->pProfile, ((mng_iccpp)pChunk)->iProfilesize)
                                       /* store it's length as well */
        pImage->pImgbuf->iProfilesize = ((mng_iccpp)pChunk)->iProfilesize;
        pImage->pImgbuf->bHasICCP     = MNG_TRUE;
      }
    }
    else
    {                                  /* store as global */
      if (pData->pGlobalProfile)     /* did we have a global profile ? */
        MNG_FREEX (pData, pData->pGlobalProfile, pData->iGlobalProfilesize)

      if (((mng_iccpp)pChunk)->bEmpty) /* empty chunk ? */
      {
        pData->iGlobalProfilesize = 0; /* reset to null */
        pData->pGlobalProfile     = MNG_NULL;
      }
      else
      {                                /* allocate a global buffer & copy it */
        MNG_ALLOC (pData, pData->pGlobalProfile, ((mng_iccpp)pChunk)->iProfilesize)
        MNG_COPY  (pData->pGlobalProfile, ((mng_iccpp)pChunk)->pProfile, ((mng_iccpp)pChunk)->iProfilesize)
                                       /* store it's length as well */
        pData->iGlobalProfilesize = ((mng_iccpp)pChunk)->iProfilesize;
      }

                                       /* create an animation object */
      iRetcode = mng_create_ani_iccp (pData, ((mng_iccpp)pChunk)->bEmpty,
                                      pData->iGlobalProfilesize,
                                      pData->pGlobalProfile);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_CHECK_BAD_ICCP
  }
#endif

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_tEXt
MNG_C_SPECIALFUNC (mng_special_text)
{
  if (pData->fProcesstext)             /* inform the application ? */
  {
    mng_bool bOke = pData->fProcesstext ((mng_handle)pData, MNG_TYPE_TEXT,
                                         ((mng_textp)pChunk)->zKeyword,
                                         ((mng_textp)pChunk)->zText, 0, 0);
    if (!bOke)
      MNG_ERROR (pData, MNG_APPMISCERROR)
  }

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_zTXt
MNG_C_SPECIALFUNC (mng_special_ztxt)
{
  if (pData->fProcesstext)             /* inform the application ? */
  {
    mng_bool bOke = pData->fProcesstext ((mng_handle)pData, MNG_TYPE_ZTXT,
                                         ((mng_ztxtp)pChunk)->zKeyword,
                                         ((mng_ztxtp)pChunk)->zText, 0, 0);
    if (!bOke)
      MNG_ERROR (pData, MNG_APPMISCERROR)
  }

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_iTXt
MNG_F_SPECIALFUNC (mng_deflate_itxt)
{
  mng_itxtp  pITXT    = (mng_itxtp)pChunk;
  mng_uint32 iBufsize = 0;
  mng_uint8p pBuf     = 0;
  mng_uint32 iTextlen = 0;

  if (pITXT->iCompressionflag)         /* decompress the text ? */
  {
    mng_retcode iRetcode = mng_inflate_buffer (pData, pRawdata, iRawlen,
                                               &pBuf, &iBufsize, &iTextlen);

    if (iRetcode)                      /* on error bail out */
    {                                  /* don't forget to drop the temp buffer */
      MNG_FREEX (pData, pBuf, iBufsize)
      return iRetcode;
    }

    MNG_ALLOC (pData, pITXT->zText, iTextlen+1)
    MNG_COPY (pITXT->zText, pBuf, iTextlen)

    pITXT->iTextsize = iTextlen;

    MNG_FREEX (pData, pBuf, iBufsize)

  } else {

    MNG_ALLOC (pData, pITXT->zText, iRawlen+1)
    MNG_COPY (pITXT->zText, pRawdata, iRawlen)

    pITXT->iTextsize = iRawlen;
  }

  return MNG_NOERROR;
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_iTXt
MNG_C_SPECIALFUNC (mng_special_itxt)
{
  if (pData->fProcesstext)             /* inform the application ? */
  {
    mng_bool bOke = pData->fProcesstext ((mng_handle)pData, MNG_TYPE_ITXT,
                                         ((mng_itxtp)pChunk)->zKeyword,
                                         ((mng_itxtp)pChunk)->zText,
                                         ((mng_itxtp)pChunk)->zLanguage,
                                         ((mng_itxtp)pChunk)->zTranslation);
    if (!bOke)
      MNG_ERROR (pData, MNG_APPMISCERROR)
  }

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_bKGD
MNG_C_SPECIALFUNC (mng_special_bkgd)
{
#ifdef MNG_SUPPORT_DISPLAY
  mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
  mng_imagedatap pBuf;
#endif

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasBKGD = MNG_TRUE;        /* indicate bKGD available */
  else
    pData->bHasglobalBKGD = (mng_bool)!(((mng_bkgdp)pChunk)->bEmpty);

#ifdef MNG_SUPPORT_DISPLAY
  if (!pImage)                         /* if no object dump it in obj 0 */
    pImage = (mng_imagep)pData->pObjzero;
  pBuf = pImage->pImgbuf;              /* address object buffer */

#ifdef MNG_INCLUDE_JNG
  if (pData->bHasJHDR)
  {
    pBuf->bHasBKGD = MNG_TRUE;         /* tell the object it's got bKGD now */

    switch (pData->iJHDRcolortype)     /* store fields for future reference */
    {
      case  8 : ;                      /* gray */
      case 12 : {                      /* graya */
                  pBuf->iBKGDgray  = ((mng_bkgdp)pChunk)->iGray;
                  break;
                }
      case 10 : ;                      /* rgb */
      case 14 : {                      /* rgba */
                  pBuf->iBKGDred   = ((mng_bkgdp)pChunk)->iRed;
                  pBuf->iBKGDgreen = ((mng_bkgdp)pChunk)->iGreen;
                  pBuf->iBKGDblue  = ((mng_bkgdp)pChunk)->iBlue;
                  break;
                }
    }
  }
  else
#endif /* MNG_INCLUDE_JNG */
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
  {
    pBuf->bHasBKGD = MNG_TRUE;         /* tell the object it's got bKGD now */

    switch (pData->iColortype)         /* store fields for future reference */
    {
      case 0 : ;                        /* gray */
      case 4 : {                        /* graya */
                 pBuf->iBKGDgray  = ((mng_bkgdp)pChunk)->iGray;
                 break;
               }
      case 2 : ;                        /* rgb */
      case 6 : {                        /* rgba */
                 pBuf->iBKGDred   = ((mng_bkgdp)pChunk)->iRed;
                 pBuf->iBKGDgreen = ((mng_bkgdp)pChunk)->iGreen;
                 pBuf->iBKGDblue  = ((mng_bkgdp)pChunk)->iBlue;
                 break;
               }
      case 3 : {                        /* indexed */
                 pBuf->iBKGDindex = ((mng_bkgdp)pChunk)->iIndex;
                 break;
               }
    }
  }
  else                                 /* store as global */
  {
    if (!(((mng_bkgdp)pChunk)->bEmpty))
    {
      pData->iGlobalBKGDred   = ((mng_bkgdp)pChunk)->iRed;
      pData->iGlobalBKGDgreen = ((mng_bkgdp)pChunk)->iGreen;
      pData->iGlobalBKGDblue  = ((mng_bkgdp)pChunk)->iBlue;
    }

    {                                  /* create an animation object */
      mng_retcode iRetcode = mng_create_ani_bkgd (pData, pData->iGlobalBKGDred,
                                                  pData->iGlobalBKGDgreen,
                                                  pData->iGlobalBKGDblue);
      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_pHYs
MNG_C_SPECIALFUNC (mng_special_phys)
{
#ifdef MNG_SUPPORT_DISPLAY
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_sBIT
MNG_C_SPECIALFUNC (mng_special_sbit)
{
#ifdef MNG_SUPPORT_DISPLAY
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_tIME
MNG_C_SPECIALFUNC (mng_special_time)
{
/*  if (pData->fProcesstime) */            /* inform the application ? */
/*  {

    pData->fProcesstime ((mng_handle)pData, );
  } */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* JNG chunks */


/* ************************************************************************** */
/* ************************************************************************** */
/* MNG chunks */

MNG_C_SPECIALFUNC (mng_special_mhdr)
{
  if (pData->bHasheader)               /* can only be the first chunk! */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  pData->bHasMHDR     = MNG_TRUE;      /* oh boy, a real MNG */
  pData->bHasheader   = MNG_TRUE;      /* we've got a header */
  pData->eImagetype   = mng_it_mng;    /* fill header fields */
  pData->iWidth       = ((mng_mhdrp)pChunk)->iWidth;
  pData->iHeight      = ((mng_mhdrp)pChunk)->iHeight;
  pData->iTicks       = ((mng_mhdrp)pChunk)->iTicks;
  pData->iLayercount  = ((mng_mhdrp)pChunk)->iLayercount;
  pData->iFramecount  = ((mng_mhdrp)pChunk)->iFramecount;
  pData->iPlaytime    = ((mng_mhdrp)pChunk)->iPlaytime;
  pData->iSimplicity  = ((mng_mhdrp)pChunk)->iSimplicity;
  pData->bPreDraft48  = MNG_FALSE;
                                       /* predict alpha-depth */
  if ((pData->iSimplicity & 0x00000001) == 0)
#ifndef MNG_NO_16BIT_SUPPORT
    pData->iAlphadepth = 16;           /* no indicators = assume the worst */
#else
    pData->iAlphadepth = 8;            /* anything else = assume the worst */
#endif
  else
  if ((pData->iSimplicity & 0x00000008) == 0)
    pData->iAlphadepth = 0;            /* no transparency at all */
  else
  if ((pData->iSimplicity & 0x00000140) == 0x00000040)
    pData->iAlphadepth = 1;            /* no semi-transparency guaranteed */
  else
#ifndef MNG_NO_16BIT_SUPPORT
    pData->iAlphadepth = 16;           /* anything else = assume the worst */
#else
    pData->iAlphadepth = 8;            /* anything else = assume the worst */
#endif

#ifdef MNG_INCLUDE_JNG                 /* can we handle the complexity ? */
  if (pData->iSimplicity & 0x0000FC00)
#else
  if (pData->iSimplicity & 0x0000FC10)
#endif
    MNG_ERROR (pData, MNG_MNGTOOCOMPLEX)
                                       /* fits on maximum canvas ? */
  if ((pData->iWidth > pData->iMaxwidth) || (pData->iHeight > pData->iMaxheight))
    MNG_WARNING (pData, MNG_IMAGETOOLARGE)

  if (pData->fProcessheader)           /* inform the app ? */
    if (!pData->fProcessheader (((mng_handle)pData), pData->iWidth, pData->iHeight))
      MNG_ERROR (pData, MNG_APPMISCERROR)

  pData->iImagelevel++;                /* one level deeper */

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

MNG_C_SPECIALFUNC (mng_special_mend)
{
#ifdef MNG_SUPPORT_DISPLAY
  {                                    /* do something */
    mng_retcode iRetcode = mng_process_display_mend (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    if (!pData->iTotalframes)          /* save totals */
      pData->iTotalframes   = pData->iFrameseq;
    if (!pData->iTotallayers)
      pData->iTotallayers   = pData->iLayerseq;
    if (!pData->iTotalplaytime)
      pData->iTotalplaytime = pData->iFrametime;
  }
#endif /* MNG_SUPPORT_DISPLAY */

  pData->bHasMHDR = MNG_FALSE;         /* end of the line, bro! */

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_LOOP
MNG_C_SPECIALFUNC (mng_special_endl)
{
#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bHasLOOP)                 /* are we really processing a loop ? */
  {                                    
    mng_uint8 iLevel = ((mng_endlp)pChunk)->iLevel;
                                       /* create an ENDL animation object */
    mng_retcode iRetcode = mng_create_ani_endl (pData, iLevel);
    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    {                                  /* process it */
      mng_ani_endlp pENDL = (mng_ani_endlp)pData->pLastaniobj;

      iRetcode = pENDL->sHeader.fProcess (pData, pENDL);
      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
  }
  else
    MNG_ERROR (pData, MNG_NOMATCHINGLOOP)
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_DEFI
MNG_C_SPECIALFUNC (mng_special_defi)
{
#ifdef MNG_SUPPORT_DISPLAY
  mng_retcode iRetcode;

  pData->iDEFIobjectid     = ((mng_defip)pChunk)->iObjectid;
  pData->bDEFIhasdonotshow = ((mng_defip)pChunk)->bHasdonotshow;
  pData->iDEFIdonotshow    = ((mng_defip)pChunk)->iDonotshow;
  pData->bDEFIhasconcrete  = ((mng_defip)pChunk)->bHasconcrete;
  pData->iDEFIconcrete     = ((mng_defip)pChunk)->iConcrete;
  pData->bDEFIhasloca      = ((mng_defip)pChunk)->bHasloca;
  pData->iDEFIlocax        = ((mng_defip)pChunk)->iXlocation;
  pData->iDEFIlocay        = ((mng_defip)pChunk)->iYlocation;
  pData->bDEFIhasclip      = ((mng_defip)pChunk)->bHasclip;
  pData->iDEFIclipl        = ((mng_defip)pChunk)->iLeftcb;
  pData->iDEFIclipr        = ((mng_defip)pChunk)->iRightcb;
  pData->iDEFIclipt        = ((mng_defip)pChunk)->iTopcb;
  pData->iDEFIclipb        = ((mng_defip)pChunk)->iBottomcb;
                                       /* create an animation object */
  iRetcode = mng_create_ani_defi (pData);
  if (!iRetcode)                       /* do display processing */
    iRetcode = mng_process_display_defi (pData);
  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_BASI
MNG_C_SPECIALFUNC (mng_special_basi)
{
  pData->bHasBASI     = MNG_TRUE;      /* inside a BASI-IEND block now */
                                       /* store interesting fields */
  pData->iDatawidth   = ((mng_basip)pChunk)->iWidth;
  pData->iDataheight  = ((mng_basip)pChunk)->iHeight;
  pData->iBitdepth    = ((mng_basip)pChunk)->iBitdepth;   
  pData->iColortype   = ((mng_basip)pChunk)->iColortype;
  pData->iCompression = ((mng_basip)pChunk)->iCompression;
  pData->iFilter      = ((mng_basip)pChunk)->iFilter;
  pData->iInterlace   = ((mng_basip)pChunk)->iInterlace;

#if defined(MNG_NO_1_2_4BIT_SUPPORT) || defined(MNG_NO_16BIT_SUPPORT)
  pData->iPNGmult = 1;
  pData->iPNGdepth = pData->iBitdepth;
#endif

#ifdef MNG_NO_1_2_4BIT_SUPPORT
  if (pData->iBitdepth < 8)
    pData->iBitdepth = 8;
#endif
#ifdef MNG_NO_16BIT_SUPPORT
  if (pData->iBitdepth > 8)
    {
      pData->iBitdepth = 8;
      pData->iPNGmult = 2;
    }
#endif

  if ((pData->iBitdepth !=  8)      /* parameter validity checks */
#ifndef MNG_NO_1_2_4BIT_SUPPORT
      && (pData->iBitdepth !=  1) &&
      (pData->iBitdepth !=  2) &&
      (pData->iBitdepth !=  4)
#endif
#ifndef MNG_NO_16BIT_SUPPORT
      && (pData->iBitdepth != 16)
#endif
      )
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

  if ((pData->iColortype != MNG_COLORTYPE_GRAY   ) &&
      (pData->iColortype != MNG_COLORTYPE_RGB    ) &&
      (pData->iColortype != MNG_COLORTYPE_INDEXED) &&
      (pData->iColortype != MNG_COLORTYPE_GRAYA  ) &&
      (pData->iColortype != MNG_COLORTYPE_RGBA   )    )
    MNG_ERROR (pData, MNG_INVALIDCOLORTYPE)

  if ((pData->iColortype == MNG_COLORTYPE_INDEXED) && (pData->iBitdepth > 8))
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

  if (((pData->iColortype == MNG_COLORTYPE_RGB    ) ||
       (pData->iColortype == MNG_COLORTYPE_GRAYA  ) ||
       (pData->iColortype == MNG_COLORTYPE_RGBA   )    ) &&
      (pData->iBitdepth < 8                            )    )
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

#if defined(FILTER192) || defined(FILTER193)
  if ((pData->iFilter != MNG_FILTER_ADAPTIVE ) &&
#if defined(FILTER192) && defined(FILTER193)
      (pData->iFilter != MNG_FILTER_DIFFERING) &&
      (pData->iFilter != MNG_FILTER_NOFILTER )    )
#else
#ifdef FILTER192
      (pData->iFilter != MNG_FILTER_DIFFERING)    )
#else
      (pData->iFilter != MNG_FILTER_NOFILTER )    )
#endif
#endif
    MNG_ERROR (pData, MNG_INVALIDFILTER)
#else
  if (pData->iFilter)
    MNG_ERROR (pData, MNG_INVALIDFILTER)
#endif

  pData->iImagelevel++;                /* one level deeper */

#ifdef MNG_SUPPORT_DISPLAY
  {                                    /* create an animation object */
    mng_retcode iRetcode = mng_create_ani_basi (pData,
                                                ((mng_basip)pChunk)->iRed,
                                                ((mng_basip)pChunk)->iGreen,
                                                ((mng_basip)pChunk)->iBlue,
                                                ((mng_basip)pChunk)->bHasalpha,
                                                ((mng_basip)pChunk)->iAlpha,
                                                ((mng_basip)pChunk)->iViewable);

    if (!iRetcode)                     /* display-processing... */
      iRetcode = mng_process_display_basi (pData,
                                           ((mng_basip)pChunk)->iRed,
                                           ((mng_basip)pChunk)->iGreen,
                                           ((mng_basip)pChunk)->iBlue,
                                           ((mng_basip)pChunk)->bHasalpha,
                                           ((mng_basip)pChunk)->iAlpha,
                                           ((mng_basip)pChunk)->iViewable);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_NO_16BIT_SUPPORT
  if (((mng_basip)pChunk)->iBitdepth > 8)
    ((mng_basip)pChunk)->iBitdepth = 8;
#endif

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_CLON
MNG_C_SPECIALFUNC (mng_special_clon)
{
#ifdef MNG_SUPPORT_DISPLAY
  mng_retcode iRetcode = mng_create_ani_clon (pData,
                                              ((mng_clonp)pChunk)->iSourceid,
                                              ((mng_clonp)pChunk)->iCloneid,
                                              ((mng_clonp)pChunk)->iClonetype,
                                              ((mng_clonp)pChunk)->bHasdonotshow,
                                              ((mng_clonp)pChunk)->iDonotshow,
                                              ((mng_clonp)pChunk)->iConcrete,
                                              ((mng_clonp)pChunk)->bHasloca,
                                              ((mng_clonp)pChunk)->iLocationtype,
                                              ((mng_clonp)pChunk)->iLocationx,
                                              ((mng_clonp)pChunk)->iLocationy);

  if (!iRetcode)                       /* do display processing */
    iRetcode = mng_process_display_clon (pData,
                                         ((mng_clonp)pChunk)->iSourceid,
                                         ((mng_clonp)pChunk)->iCloneid,
                                         ((mng_clonp)pChunk)->iClonetype,
                                         ((mng_clonp)pChunk)->bHasdonotshow,
                                         ((mng_clonp)pChunk)->iDonotshow,
                                         ((mng_clonp)pChunk)->iConcrete,
                                         ((mng_clonp)pChunk)->bHasloca,
                                         ((mng_clonp)pChunk)->iLocationtype,
                                         ((mng_clonp)pChunk)->iLocationx,
                                         ((mng_clonp)pChunk)->iLocationy);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_BACK
MNG_C_SPECIALFUNC (mng_special_back)
{
#ifdef MNG_SUPPORT_DISPLAY
  mng_retcode iRetcode;
                                       /* retrieve the fields */
  pData->bHasBACK       = MNG_TRUE;
  pData->iBACKred       = ((mng_backp)pChunk)->iRed;
  pData->iBACKgreen     = ((mng_backp)pChunk)->iGreen;
  pData->iBACKblue      = ((mng_backp)pChunk)->iBlue;
  pData->iBACKmandatory = ((mng_backp)pChunk)->iMandatory;
  pData->iBACKimageid   = ((mng_backp)pChunk)->iImageid;
  pData->iBACKtile      = ((mng_backp)pChunk)->iTile;

  iRetcode = mng_create_ani_back (pData, pData->iBACKred, pData->iBACKgreen,
                                  pData->iBACKblue, pData->iBACKmandatory,
                                  pData->iBACKimageid, pData->iBACKtile);
  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_MOVE
MNG_C_SPECIALFUNC (mng_special_move)
{
#ifdef MNG_SUPPORT_DISPLAY
                                       /* create a MOVE animation object */
  mng_retcode iRetcode = mng_create_ani_move (pData,
                                              ((mng_movep)pChunk)->iFirstid,
                                              ((mng_movep)pChunk)->iLastid,
                                              ((mng_movep)pChunk)->iMovetype,
                                              ((mng_movep)pChunk)->iMovex,
                                              ((mng_movep)pChunk)->iMovey);

  if (!iRetcode)                       /* process the move */
    iRetcode = mng_process_display_move (pData,
                                         ((mng_movep)pChunk)->iFirstid,
                                         ((mng_movep)pChunk)->iLastid,
                                         ((mng_movep)pChunk)->iMovetype,
                                         ((mng_movep)pChunk)->iMovex,
                                         ((mng_movep)pChunk)->iMovey);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_CLIP
MNG_C_SPECIALFUNC (mng_special_clip)
{
#ifdef MNG_SUPPORT_DISPLAY
                                       /* create a CLIP animation object */
  mng_retcode iRetcode = mng_create_ani_clip (pData,
                                              ((mng_clipp)pChunk)->iFirstid,
                                              ((mng_clipp)pChunk)->iLastid,
                                              ((mng_clipp)pChunk)->iCliptype,
                                              ((mng_clipp)pChunk)->iClipl,
                                              ((mng_clipp)pChunk)->iClipr,
                                              ((mng_clipp)pChunk)->iClipt,
                                              ((mng_clipp)pChunk)->iClipb);

  if (!iRetcode)                       /* process the clipping */
    iRetcode = mng_process_display_clip (pData,
                                         ((mng_clipp)pChunk)->iFirstid,
                                         ((mng_clipp)pChunk)->iLastid,
                                         ((mng_clipp)pChunk)->iCliptype,
                                         ((mng_clipp)pChunk)->iClipl,
                                         ((mng_clipp)pChunk)->iClipr,
                                         ((mng_clipp)pChunk)->iClipt,
                                         ((mng_clipp)pChunk)->iClipb);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_SHOW
MNG_C_SPECIALFUNC (mng_special_show)
{
#ifdef MNG_SUPPORT_DISPLAY
  mng_retcode iRetcode;

  if (!((mng_showp)pChunk)->bEmpty)    /* any data ? */
  {
    if (!((mng_showp)pChunk)->bHaslastid)
      ((mng_showp)pChunk)->iLastid = ((mng_showp)pChunk)->iFirstid;

    pData->iSHOWfromid = ((mng_showp)pChunk)->iFirstid;
    pData->iSHOWtoid   = ((mng_showp)pChunk)->iLastid;
    pData->iSHOWmode   = ((mng_showp)pChunk)->iMode;
  }
  else                                 /* use defaults then */
  {
    pData->iSHOWfromid = 1;
    pData->iSHOWtoid   = 65535;
    pData->iSHOWmode   = 2;
  }
                                       /* create a SHOW animation object */
  iRetcode = mng_create_ani_show (pData,
                                  pData->iSHOWfromid,
                                  pData->iSHOWtoid,
                                  pData->iSHOWmode);

  if (!iRetcode)                       /* go and do it! */
    iRetcode = mng_process_display_show (pData);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_TERM
MNG_C_SPECIALFUNC (mng_special_term)
{
                                       /* should be behind MHDR or SAVE !! */
  if ((!pData->bHasSAVE) && (pData->iChunkseq > 2))
  {
    pData->bMisplacedTERM = MNG_TRUE;  /* indicate we found a misplaced TERM */
                                       /* and send a warning signal!!! */
    MNG_WARNING (pData, MNG_SEQUENCEERROR)
  }

  pData->bHasTERM = MNG_TRUE;

  if (pData->fProcessterm)             /* inform the app ? */
    if (!pData->fProcessterm (((mng_handle)pData),
                              ((mng_termp)pChunk)->iTermaction,
                              ((mng_termp)pChunk)->iIteraction,
                              ((mng_termp)pChunk)->iDelay,
                              ((mng_termp)pChunk)->iItermax))
      MNG_ERROR (pData, MNG_APPMISCERROR)

#ifdef MNG_SUPPORT_DISPLAY
  {                                    /* create the TERM ani-object */
    mng_retcode iRetcode = mng_create_ani_term (pData,
                                                ((mng_termp)pChunk)->iTermaction,
                                                ((mng_termp)pChunk)->iIteraction,
                                                ((mng_termp)pChunk)->iDelay,
                                                ((mng_termp)pChunk)->iItermax);
    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* save for future reference */
    pData->pTermaniobj = pData->pLastaniobj;
  }
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#ifndef MNG_SKIPCHUNK_SEEK
MNG_C_SPECIALFUNC (mng_special_seek)
{
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_DISPLAY
                                       /* create a SEEK animation object */
  iRetcode = mng_create_ani_seek (pData,
                                  ((mng_seekp)pChunk)->iNamesize,
                                  ((mng_seekp)pChunk)->zName);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

#endif /* MNG_SUPPORT_DISPLAY */

  if (pData->fProcessseek)             /* inform the app ? */
    if (!pData->fProcessseek ((mng_handle)pData, ((mng_seekp)pChunk)->zName))
      MNG_ERROR (pData, MNG_APPMISCERROR)

#ifdef MNG_SUPPORT_DISPLAY
                                       /* do display processing of the SEEK */
  iRetcode = mng_process_display_seek (pData);
  if (iRetcode)                        /* on error bail out */
    return iRetcode;
#endif /* MNG_SUPPORT_DISPLAY */

  return MNG_NOERROR;                  /* done */
}
#endif

/* ************************************************************************** */

#endif /* MNG_INCLUDE_READ_PROCS || MNG_INCLUDE_WRITE_PROCS */
#endif /* MNG_OPTIMIZE_CHUNKREADER */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */


