/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_chunk_io.c            copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Chunk I/O routines (implementation)                        * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of chunk input/output routines              * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                        **nobody**  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#include "libmng.h"
#include "mng_data.h"
#include "mng_objects.h"
#include "mng_object_prc.h"
#include "mng_chunks.h"
#include "mng_memory.h"
#include "mng_error.h"
#include "mng_trace.h"
#include "mng_display.h"
#include "mng_zlib.h"
#include "mng_pixels.h"
#include "mng_chunk_io.h"

/* ************************************************************************** */
/* *                                                                        * */
/* * CRC - Cyclic Redundancy Check                                          * */
/* *                                                                        * */
/* * The code below is taken directly from the sample provided with the     * */
/* * PNG specification.                                                     * */
/* * (it is only adapted to the library's internal data-definitions)        * */
/* *                                                                        * */
/* ************************************************************************** */

/* Table of CRCs of all 8-bit messages. */
mng_uint32 crc_table[256];

/* Flag: has the table been computed? Initially false. */
mng_bool crc_table_computed = MNG_FALSE;

/* Make the table for a fast CRC. */
void make_crc_table (void)
{
  mng_uint32 c;
  mng_int32  n, k;

  for (n = 0; n < 256; n++)
  {
    c = (mng_uint32) n;

    for (k = 0; k < 8; k++)
    {
      if (c & 1)
        c = 0xedb88320L ^ (c >> 1);
      else
        c = c >> 1;
    };

    crc_table [n] = c;
  };

  crc_table_computed = MNG_TRUE;
};

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

mng_uint32 update_crc (mng_uint32 crc,
                       mng_uint8p buf,
                       mng_int32  len)
{
  mng_uint32 c = crc;
  mng_int32 n;

  if (!crc_table_computed)
    make_crc_table ();

  for (n = 0; n < len; n++)
    c = crc_table [(c ^ buf [n]) & 0xff] ^ (c >> 8);

  return c;
};

/* Return the CRC of the bytes buf[0..len-1]. */
mng_uint32 crc (mng_uint8p buf,
                mng_int32  len)
{
  return update_crc (0xffffffffL, buf, len) ^ 0xffffffffL;
};

/* ************************************************************************** */
/* *                                                                        * */
/* * Routines for swapping byte-order from and to graphic files             * */
/* * (This code is adapted from the libpng package)                         * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_SWAP_ENDIAN

/* ************************************************************************** */

mng_uint32 mng_get_uint32 (mng_uint8p pBuf)
{
   mng_uint32 i = ((mng_uint32)(*pBuf)       << 24) +
                  ((mng_uint32)(*(pBuf + 1)) << 16) +
                  ((mng_uint32)(*(pBuf + 2)) <<  8) +
                   (mng_uint32)(*(pBuf + 3));
   return (i);
};

/* ************************************************************************** */

mng_int32 mng_get_int32 (mng_uint8p pBuf)
{
   mng_int32 i = ((mng_int32)(*pBuf)       << 24) +
                 ((mng_int32)(*(pBuf + 1)) << 16) +
                 ((mng_int32)(*(pBuf + 2)) <<  8) +
                  (mng_int32)(*(pBuf + 3));
   return (i);
};

/* ************************************************************************** */

mng_uint16 mng_get_uint16 (mng_uint8p pBuf)
{
   mng_uint16 i = (mng_uint16)(((mng_uint16)(*pBuf) << 8) +
                                (mng_uint16)(*(pBuf + 1)));
   return (i);
};

/* ************************************************************************** */

void mng_put_uint32 (mng_uint8p pBuf,
                     mng_uint32 i)
{
   pBuf[0] = (mng_uint8)((i >> 24) & 0xff);
   pBuf[1] = (mng_uint8)((i >> 16) & 0xff);
   pBuf[2] = (mng_uint8)((i >> 8) & 0xff);
   pBuf[3] = (mng_uint8)(i & 0xff);
}

/* ************************************************************************** */

void mng_put_int32 (mng_uint8p pBuf,
                    mng_int32  i)
{
   pBuf[0] = (mng_uint8)((i >> 24) & 0xff);
   pBuf[1] = (mng_uint8)((i >> 16) & 0xff);
   pBuf[2] = (mng_uint8)((i >> 8) & 0xff);
   pBuf[3] = (mng_uint8)(i & 0xff);
}

/* ************************************************************************** */

void mng_put_uint16 (mng_uint8p pBuf,
                     mng_uint16 i)
{
   pBuf[0] = (mng_uint8)((i >> 8) & 0xff);
   pBuf[1] = (mng_uint8)(i & 0xff);
}

/* ************************************************************************** */

#endif /* MNG_SWAP_ENDIAN */

/* ************************************************************************** */
/* *                                                                        * */
/* * Helper routines to simplify chunk-data extraction                      * */
/* *                                                                        * */
/* ************************************************************************** */

mng_uint8p find_null (mng_uint8p pIn)
{
  mng_uint8p pOut = pIn;

  while (*pOut)                        /* the read_graphic routine has made sure there's */
    pOut++;                            /* always at least 1 zero-byte in the buffer */

  return pOut;
}

/* ************************************************************************** */

mng_retcode inflate_buffer (mng_datap  pData,
                            mng_uint8p pInbuf,
                            mng_uint32 iInsize,
                            mng_uint8p *pOutbuf,
                            mng_uint32 *iOutsize,
                            mng_uint32 *iRealsize)
{
  mng_retcode iRetcode;

  if (iInsize)                         /* anything to do ? */
  {
    *iOutsize = iInsize * 3;           /* estimate uncompressed size */
                                       /* and allocate a temporary buffer */
    MNG_ALLOC (pData, *pOutbuf, *iOutsize)

    do
    {
      mngzlib_inflateinit (pData);     /* initialize zlib */
                                       /* let zlib know where to store the output */
      pData->sZlib.next_out  = *pOutbuf;
                                       /* "size - 1" so we've got space for the
                                          zero-termination of a possible string */
      pData->sZlib.avail_out = *iOutsize - 1;
                                       /* ok; let's inflate... */
      iRetcode = mngzlib_inflatedata (pData, iInsize, pInbuf);
                                       /* determine actual output size */
      *iRealsize = pData->sZlib.total_out;

      mngzlib_inflatefree (pData);     /* zlib's done */

      if (iRetcode == MNG_BUFOVERFLOW) /* not enough space ? */
      {                                /* then get some more */
        MNG_FREE (pData, *pOutbuf, *iOutsize)
        *iOutsize = *iOutsize + iInsize;
        MNG_ALLOC (pData, *pOutbuf, *iOutsize)
      }
    }                                  /* repeat if we didn't have enough space */
    while (iRetcode == MNG_BUFOVERFLOW);

    if (!iRetcode)                     /* if oke ? */
      *((*pOutbuf) + *iRealsize) = 0;  /* then put terminator zero */

  }
  else
  {
    *pOutbuf   = 0;                    /* nothing to do; then there's no output */
    *iOutsize  = 0;
    *iRealsize = 0;
  }

  return iRetcode;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * chunk read functions                                                   * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_READ_PROCS

/* ************************************************************************** */

READ_CHUNK (read_ihdr)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IHDR, MNG_LC_START);
#endif

  if (iRawlen != 13)                   /* length oke ? */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)
                                       /* only allowed inside PNG or MNG */
  if ((pData->eSigtype != mng_it_png) && (pData->eSigtype != mng_it_mng))
    MNG_ERROR (pData, MNG_CHUNKNOTALLOWED)
                                       /* sequence checks */
  if ((pData->eSigtype == mng_it_png) && (pData->iChunkseq > 1))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if (pData->bHasJHDR)
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#endif

  pData->bHasIHDR     = MNG_TRUE;      /* indicate IHDR is present */
                                       /* and store interesting fields */
  pData->iDatawidth   = mng_get_uint32 (pRawdata);
  pData->iDataheight  = mng_get_uint32 (pRawdata+4);
  pData->iBitdepth    = *(pRawdata+8);
  pData->iColortype   = *(pRawdata+9);
  pData->iCompression = *(pRawdata+10);
  pData->iFilter      = *(pRawdata+11);
  pData->iInterlace   = *(pRawdata+12);

  if ((pData->iBitdepth !=  1) &&      /* parameter validity checks */
      (pData->iBitdepth !=  2) &&
      (pData->iBitdepth !=  4) &&
      (pData->iBitdepth !=  8) &&
      (pData->iBitdepth != 16)    )
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

  if (pData->iFilter != MNG_FILTER_ADAPTIVE)
    MNG_ERROR (pData, MNG_INVALIDFILTER)

  if ((pData->iInterlace != MNG_INTERLACE_NONE ) &&
      (pData->iInterlace != MNG_INTERLACE_ADAM7)    )
    MNG_ERROR (pData, MNG_INVALIDINTERLACE)

  if (!pData->bHasheader)              /* first chunk ? */
  {
    pData->bHasheader = MNG_TRUE;      /* we've got a header */
    pData->eImagetype = mng_it_png;    /* then this must be a PNG */
    pData->iWidth     = pData->iDatawidth;
    pData->iHeight    = pData->iDataheight;
                                       /* fits on maximum canvas ? */
    if ((pData->iWidth > pData->iMaxwidth) || (pData->iHeight > pData->iMaxheight))
      MNG_WARNING (pData, MNG_IMAGETOOLARGE)

    if (pData->fProcessheader)         /* inform the app ? */
      pData->fProcessheader (((mng_handle)pData), pData->iWidth, pData->iHeight);
  }

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_retcode iRetcode = process_display_ihdr (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    mng_retcode iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* fill the fields */
    ((mng_ihdrp)*ppChunk)->iWidth       = pData->iDatawidth;
    ((mng_ihdrp)*ppChunk)->iHeight      = pData->iDataheight;
    ((mng_ihdrp)*ppChunk)->iBitdepth    = pData->iBitdepth;
    ((mng_ihdrp)*ppChunk)->iColortype   = pData->iColortype;
    ((mng_ihdrp)*ppChunk)->iCompression = pData->iCompression;
    ((mng_ihdrp)*ppChunk)->iFilter      = pData->iFilter;
    ((mng_ihdrp)*ppChunk)->iInterlace   = pData->iInterlace;
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_plte)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif
#if defined(MNG_SUPPORT_DISPLAY) || defined(MNG_STORE_CHUNKS)
  mng_uint32  iX;
  mng_uint8p  pRawdata2;
#endif
#ifdef MNG_SUPPORT_DISPLAY
  mng_uint32  iRawlen2;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PLTE, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasJHDR))
#else
  if (pData->bHasIDAT)
#endif  
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* multiple PLTE only inside BASI */
  if ((pData->bHasPLTE) && (!pData->bHasBASI))
    MNG_ERROR (pData, MNG_MULTIPLEERROR)
                                       /* length must be multiple of 3 */
  if (((iRawlen % 3) != 0) || (iRawlen > 768))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
  {                                    /* only allowed for indexed-color or
                                          rgb(a)-color! */
    if ((pData->iColortype != 2) && (pData->iColortype != 3) && (pData->iColortype != 6))
      MNG_ERROR (pData, MNG_CHUNKNOTALLOWED)
                                       /* empty only allowed if global present */
    if ((iRawlen == 0) && (!pData->bHasglobalTRNS))
        MNG_ERROR (pData, MNG_CANNOTBEEMPTY)
  }
  else
  {
    if (iRawlen == 0)                  /* cannot be empty as global! */
      MNG_ERROR (pData, MNG_CANNOTBEEMPTY)
  }

  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
    pData->bHasPLTE = MNG_TRUE;        /* got it! */
  else
    pData->bHasglobalPLTE = MNG_TRUE;

#ifdef MNG_SUPPORT_DISPLAY
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
  {
    mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
    mng_imagedatap pBuf;

    if (!pImage)                       /* no object then dump it in obj 0 */
      pImage = (mng_imagep)pData->pObjzero;

    pBuf = pImage->pImgbuf;            /* address the object buffer */
    pBuf->bHasPLTE = MNG_TRUE;         /* and tell it it's got a PLTE now */

    if (!iRawlen)                      /* if empty, inherit from global */
    {
      pBuf->iPLTEcount = pData->iGlobalPLTEcount;
      MNG_COPY (&pBuf->aPLTEentries, &pData->aGlobalPLTEentries,
                sizeof (pBuf->aPLTEentries))

      if (pData->bHasglobalTRNS)       /* also copy global tRNS ? */
      {                                /* indicate tRNS available */
        pBuf->bHasTRNS = MNG_TRUE;

        iRawlen2  = pData->iGlobalTRNSrawlen;
        pRawdata2 = (mng_uint8p)(&pData->aGlobalTRNSrawdata);
                                       /* global length oke ? */
        if ((iRawlen2 == 0) || (iRawlen2 > pBuf->iPLTEcount))
          MNG_ERROR (pData, MNG_GLOBALLENGTHERR)
                                       /* copy it */
        pBuf->iTRNScount = iRawlen2;
        MNG_COPY (&pBuf->aTRNSentries, pRawdata2, iRawlen2)
      }
    }
    else
    {
      pBuf->iPLTEcount = iRawlen / 3;  /* store fields for future reference */
      pRawdata2        = pRawdata;

      for (iX = 0; iX < pBuf->iPLTEcount; iX++)
      {
        pBuf->aPLTEentries[iX].iRed   = *pRawdata2;
        pBuf->aPLTEentries[iX].iGreen = *(pRawdata2+1);
        pBuf->aPLTEentries[iX].iBlue  = *(pRawdata2+2);

        pRawdata2 += 3;
      }
    }
  }
  else                                 /* store as global */
  {
    pData->iGlobalPLTEcount = iRawlen / 3;
    pRawdata2               = pRawdata;

    for (iX = 0; iX < pData->iGlobalPLTEcount; iX++)
    {
      pData->aGlobalPLTEentries[iX].iRed   = *pRawdata2;
      pData->aGlobalPLTEentries[iX].iGreen = *(pRawdata2+1);
      pData->aGlobalPLTEentries[iX].iBlue  = *(pRawdata2+2);

      pRawdata2 += 3;
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_pltep)*ppChunk)->bEmpty      = (mng_bool)(iRawlen == 0);
    ((mng_pltep)*ppChunk)->iEntrycount = iRawlen / 3;
    pRawdata2                          = pRawdata;

    for (iX = 0; iX < ((mng_pltep)*ppChunk)->iEntrycount; iX++)
    {
      ((mng_pltep)*ppChunk)->aEntries[iX].iRed   = *pRawdata2;
      ((mng_pltep)*ppChunk)->aEntries[iX].iGreen = *(pRawdata2+1);
      ((mng_pltep)*ppChunk)->aEntries[iX].iBlue  = *(pRawdata2+2);

      pRawdata2 += 3;
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PLTE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_idat)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IDAT, MNG_LC_START);
#endif

#ifdef MNG_INCLUDE_JNG                 /* sequence checks */
  if ((!pData->bHasIHDR) && (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#else
  if ((!pData->bHasIHDR) && (!pData->bHasBASI) && (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#endif

#ifdef MNG_INCLUDE_JNG
  if (pData->bHasJSEP)
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#endif
                                       /* can only be empty in BASI-block! */
  if ((iRawlen == 0) && (!pData->bHasBASI))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)
                                       /* indexed-color requires PLTE */
  if ((pData->bHasIHDR) && (pData->iColortype == 3) && (!pData->bHasPLTE))
    MNG_ERROR (pData, MNG_PLTEMISSING)

  pData->bHasIDAT = MNG_TRUE;          /* got some IDAT now, don't we */

#ifdef MNG_SUPPORT_DISPLAY
  if ((pData->bDisplaying) && (iRawlen))
  {                                    /* display processing for non-empty chunks */
    iRetcode = process_display_idat (pData, iRawlen, pRawdata);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_idatp)*ppChunk)->bEmpty    = (mng_bool)(iRawlen == 0);
    ((mng_idatp)*ppChunk)->iDatasize = iRawlen;

    if (iRawlen != 0)                  /* is there any data ? */
    {
      MNG_ALLOC (pData, ((mng_idatp)*ppChunk)->pData, iRawlen)
      MNG_COPY  (((mng_idatp)*ppChunk)->pData, pRawdata, iRawlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IDAT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_iend)
{
#if defined (MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode = MNG_NOERROR;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IEND, MNG_LC_START);
#endif

  if (iRawlen > 0)                     /* must not contain data! */
    MNG_ERROR (pData, MNG_INVALIDLENGTH);

#ifdef MNG_INCLUDE_JNG                 /* sequence checks */
  if ((!pData->bHasIHDR) && (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#else
  if ((!pData->bHasIHDR) && (!pData->bHasBASI) && (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
#endif
                                       /* IHDR-block requires IDAT */
  if ((pData->bHasIHDR) && (!pData->bHasIDAT))
    MNG_ERROR (pData, MNG_IDATMISSING)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {                                    /* save object for animation later ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_imagep pImage;           /* create an animation object then */
      iRetcode = create_ani_image (pData, &pImage);
    }

    if (!iRetcode)                     /* do display processing */
      iRetcode = process_display_iend (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

  pData->bHasIHDR         = MNG_FALSE; /* IEND signals the end */
  pData->bHasBASI         = MNG_FALSE; /* for most ... */
  pData->bHasDHDR         = MNG_FALSE;
#ifdef MNG_INCLUDE_JNG
  pData->bHasJHDR         = MNG_FALSE;
  pData->bHasJSEP         = MNG_FALSE;
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

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IEND, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_trns)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TRNS, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasJHDR))
#else
  if (pData->bHasIDAT)
#endif  
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* multiple tRNS only inside BASI */
  if ((pData->bHasTRNS) && (!pData->bHasBASI))
    MNG_ERROR (pData, MNG_MULTIPLEERROR)

  if (iRawlen > 256)                   /* it just can't be bigger than that! */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
  {                                    /* not allowed with full alpha-channel */
    if ((pData->iColortype == 4) || (pData->iColortype == 6))
      MNG_ERROR (pData, MNG_CHUNKNOTALLOWED)

    if (iRawlen != 0)                  /* filled ? */
    {                                  /* length checks */
      if ((pData->iColortype == 0) && (iRawlen != 2))
        MNG_ERROR (pData, MNG_INVALIDLENGTH)

      if ((pData->iColortype == 2) && (iRawlen != 6))
        MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
      if (pData->iColortype == 3)
      {
        mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
        mng_imagedatap pBuf;

        if (!pImage)                   /* no object then check obj 0 */
          pImage = (mng_imagep)pData->pObjzero;

        pBuf = pImage->pImgbuf;        /* address object buffer */

        if ((iRawlen == 0) || (iRawlen > pBuf->iPLTEcount))
          MNG_ERROR (pData, MNG_INVALIDLENGTH)
      }
#endif
    }
    else                               /* if empty there must be global stuff! */
    {
      if (!pData->bHasglobalTRNS)
        MNG_ERROR (pData, MNG_CANNOTBEEMPTY)
    }
  }

  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
    pData->bHasTRNS = MNG_TRUE;        /* indicate tRNS available */
  else
    pData->bHasglobalTRNS = MNG_TRUE;

#ifdef MNG_SUPPORT_DISPLAY
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
  {
    mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
    mng_imagedatap pBuf;
    mng_uint8p     pRawdata2;
    mng_uint32     iRawlen2;

    if (!pImage)                       /* no object then dump it in obj 0 */
      pImage = (mng_imagep)pData->pObjzero;

    pBuf = pImage->pImgbuf;            /* address object buffer */
    pBuf->bHasTRNS = MNG_TRUE;         /* and tell it it's got a tRNS now */

    if (iRawlen == 0)                  /* if empty, inherit from global */
    {
      iRawlen2  = pData->iGlobalTRNSrawlen;
      pRawdata2 = (mng_ptr)(&pData->aGlobalTRNSrawdata);
                                       /* global length oke ? */
      if ((pData->iColortype == 0) && (iRawlen2 != 2))
        MNG_ERROR (pData, MNG_GLOBALLENGTHERR)

      if ((pData->iColortype == 2) && (iRawlen2 != 6))
        MNG_ERROR (pData, MNG_GLOBALLENGTHERR)

      if ((pData->iColortype == 3) && ((iRawlen2 == 0) || (iRawlen2 > pBuf->iPLTEcount)))
        MNG_ERROR (pData, MNG_GLOBALLENGTHERR)
    }
    else
    {
      iRawlen2  = iRawlen;
      pRawdata2 = pRawdata;
    }

    switch (pData->iColortype)          /* store fields for future reference */
    {
      case 0: {                        /* gray */
                pBuf->iTRNSgray  = mng_get_uint16 (pRawdata2);
                pBuf->iTRNSred   = 0;
                pBuf->iTRNSgreen = 0;
                pBuf->iTRNSblue  = 0;
                pBuf->iTRNScount = 0;
                break;
              }
      case 2: {                        /* rgb */
                pBuf->iTRNSgray  = 0;
                pBuf->iTRNSred   = mng_get_uint16 (pRawdata2);
                pBuf->iTRNSgreen = mng_get_uint16 (pRawdata2+2);
                pBuf->iTRNSblue  = mng_get_uint16 (pRawdata2+4);
                pBuf->iTRNScount = 0;
                break;
              }
      case 3: {                        /* indexed */
                pBuf->iTRNSgray  = 0;
                pBuf->iTRNSred   = 0;
                pBuf->iTRNSgreen = 0;
                pBuf->iTRNSblue  = 0;
                pBuf->iTRNScount = iRawlen2;
                MNG_COPY (&pBuf->aTRNSentries, pRawdata2, iRawlen2)
                break;
              }
    }
  }
  else                                 /* store as global */
  {
    pData->iGlobalTRNSrawlen = iRawlen;
    MNG_COPY (&pData->aGlobalTRNSrawdata, pRawdata, iRawlen)
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                  
    if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
    {                                  /* not global! */
      ((mng_trnsp)*ppChunk)->bGlobal  = MNG_FALSE;

      if (iRawlen == 0)                /* if empty, indicate so */
        ((mng_trnsp)*ppChunk)->bEmpty = MNG_TRUE;
      else
      {
        ((mng_trnsp)*ppChunk)->bEmpty = MNG_FALSE;

        switch (pData->iColortype)     /* store fields */
        {
          case 0: {                    /* gray */
                    ((mng_trnsp)*ppChunk)->iGray  = mng_get_uint16 (pRawdata);
                    break;
                  }
          case 2: {                    /* rgb */
                    ((mng_trnsp)*ppChunk)->iRed   = mng_get_uint16 (pRawdata);
                    ((mng_trnsp)*ppChunk)->iGreen = mng_get_uint16 (pRawdata+2);
                    ((mng_trnsp)*ppChunk)->iBlue  = mng_get_uint16 (pRawdata+4);
                    break;
                  }
          case 3: {                    /* indexed */
                    ((mng_trnsp)*ppChunk)->iCount = iRawlen;
                    MNG_COPY (&((mng_trnsp)*ppChunk)->aEntries, pRawdata, iRawlen)
                    break;
                  }
        }
      }
    }
    else                               /* it's global! */
    {
      ((mng_trnsp)*ppChunk)->bEmpty  = (mng_bool)(iRawlen == 0);
      ((mng_trnsp)*ppChunk)->bGlobal = MNG_TRUE;
      ((mng_trnsp)*ppChunk)->iRawlen = iRawlen;

      MNG_COPY (&((mng_trnsp)*ppChunk)->aRawdata, pRawdata, iRawlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TRNS, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_gama)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_GAMA, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasPLTE) || (pData->bHasJDAT))
#else
  if ((pData->bHasIDAT) || (pData->bHasPLTE))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {                                    /* length must be exactly 4 */
    if (iRawlen != 4)
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }
  else
  {                                    /* length must be empty or exactly 4 */
    if ((iRawlen != 0) && (iRawlen != 4))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasGAMA = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalGAMA = (mng_bool)(iRawlen != 0);

#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {
    mng_imagep pImage = (mng_imagep)pData->pCurrentobj;

    if (!pImage)                       /* no object then dump it in obj 0 */
      pImage = (mng_imagep)pData->pObjzero;
                                       /* store for color-processing routines */
    pImage->pImgbuf->iGamma = mng_get_uint32 (pRawdata);
    pImage->pImgbuf->bHasGAMA = MNG_TRUE;
  }
  else
  {                                    /* store as global */
    if (iRawlen != 0)
      pData->iGlobalGamma = mng_get_uint32 (pRawdata);
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_gamap)*ppChunk)->bEmpty = (mng_bool)(iRawlen == 0);

    if (iRawlen)
      ((mng_gamap)*ppChunk)->iGamma = mng_get_uint32 (pRawdata);

  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_GAMA, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_chrm)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_CHRM, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasPLTE) || (pData->bHasJDAT))
#else
  if ((pData->bHasIDAT) || (pData->bHasPLTE))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {                                    /* length must be exactly 32 */
    if (iRawlen != 32)
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }
  else
  {                                    /* length must be empty or exactly 32 */
    if ((iRawlen != 0) && (iRawlen != 32))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasCHRM = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalCHRM = (mng_bool)(iRawlen != 0);

#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {
    mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
    mng_imagedatap pBuf;

    if (!pImage)                       /* no object then dump it in obj 0 */
      pImage = (mng_imagep)pData->pObjzero;

    pBuf = pImage->pImgbuf;            /* address object buffer */
    pBuf->bHasCHRM = MNG_TRUE;         /* and tell it it's got a CHRM now */
                                       /* store for color-processing routines */
    pBuf->iWhitepointx   = mng_get_uint32 (pRawdata);
    pBuf->iWhitepointy   = mng_get_uint32 (pRawdata+4);
    pBuf->iPrimaryredx   = mng_get_uint32 (pRawdata+8);
    pBuf->iPrimaryredy   = mng_get_uint32 (pRawdata+12);
    pBuf->iPrimarygreenx = mng_get_uint32 (pRawdata+16);
    pBuf->iPrimarygreeny = mng_get_uint32 (pRawdata+20);
    pBuf->iPrimarybluex  = mng_get_uint32 (pRawdata+24);
    pBuf->iPrimarybluey  = mng_get_uint32 (pRawdata+28);
  }
  else
  {                                    /* store as global */
    if (iRawlen != 0)
    {
      pData->iGlobalWhitepointx   = mng_get_uint32 (pRawdata);
      pData->iGlobalWhitepointy   = mng_get_uint32 (pRawdata+4);
      pData->iGlobalPrimaryredx   = mng_get_uint32 (pRawdata+8);
      pData->iGlobalPrimaryredy   = mng_get_uint32 (pRawdata+12);
      pData->iGlobalPrimarygreenx = mng_get_uint32 (pRawdata+16);
      pData->iGlobalPrimarygreeny = mng_get_uint32 (pRawdata+20);
      pData->iGlobalPrimarybluex  = mng_get_uint32 (pRawdata+24);
      pData->iGlobalPrimarybluey  = mng_get_uint32 (pRawdata+28);
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_chrmp)*ppChunk)->bEmpty = (mng_bool)(iRawlen == 0);

    if (iRawlen)
    {
      ((mng_chrmp)*ppChunk)->iWhitepointx = mng_get_uint32 (pRawdata);
      ((mng_chrmp)*ppChunk)->iWhitepointy = mng_get_uint32 (pRawdata+4);
      ((mng_chrmp)*ppChunk)->iRedx        = mng_get_uint32 (pRawdata+8);
      ((mng_chrmp)*ppChunk)->iRedy        = mng_get_uint32 (pRawdata+12);
      ((mng_chrmp)*ppChunk)->iGreenx      = mng_get_uint32 (pRawdata+16);
      ((mng_chrmp)*ppChunk)->iGreeny      = mng_get_uint32 (pRawdata+20);
      ((mng_chrmp)*ppChunk)->iBluex       = mng_get_uint32 (pRawdata+24);
      ((mng_chrmp)*ppChunk)->iBluey       = mng_get_uint32 (pRawdata+28);
    }  
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_CHRM, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_srgb)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SRGB, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasPLTE) || (pData->bHasJDAT))
#else
  if ((pData->bHasIDAT) || (pData->bHasPLTE))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {                                    /* length must be exactly 1 */
    if (iRawlen != 1)
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }
  else
  {                                    /* length must be empty or exactly 1 */
    if ((iRawlen != 0) && (iRawlen != 1))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasSRGB = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalSRGB = (mng_bool)(iRawlen != 0);

#ifdef MNG_SUPPORT_DISPLAY
#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {
    mng_imagep pImage = (mng_imagep)pData->pCurrentobj;

    if (!pImage)                       /* no object then dump it in obj 0 */
      pImage = (mng_imagep)pData->pObjzero;
                                       /* store for color-processing routines */
    pImage->pImgbuf->iRenderingintent = *pRawdata;
    pImage->pImgbuf->bHasSRGB         = MNG_TRUE;
  }
  else
  {                                    /* store as global */
    if (iRawlen != 0)
      pData->iGlobalRendintent = *pRawdata;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_srgbp)*ppChunk)->bEmpty = (mng_bool)(iRawlen == 0);

    if (iRawlen)
      ((mng_srgbp)*ppChunk)->iRenderingintent = *pRawdata;

  }    
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SRGB, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_iccp)
{
#if defined(MNG_SUPPORT_DISPLAY) || defined(MNG_STORE_CHUNKS)
  mng_uint8p  pTemp;
  mng_uint32  iCompressedsize;
  mng_uint32  iProfilesize;
  mng_uint32  iBufsize = 0;
  mng_retcode iRetcode;
#endif
  mng_uint8p  pBuf = 0;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ICCP, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasPLTE) || (pData->bHasJDAT))
#else
  if ((pData->bHasIDAT) || (pData->bHasPLTE))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {                                    /* length must be at least 2 */
    if (iRawlen < 2)
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }
  else
  {                                    /* length must be empty or at least 2 */
    if ((iRawlen != 0) && (iRawlen < 2))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasICCP = MNG_TRUE;        /* indicate we've got it */
  else
    pData->bHasglobalICCP = (mng_bool)(iRawlen != 0);

#ifdef MNG_SUPPORT_DISPLAY
  pTemp = find_null (pRawdata);        /* find null-separator */
                                       /* not found inside input-data ? */
  if ((pTemp - pRawdata) > (mng_int32)iRawlen)
    MNG_ERROR (pData, MNG_NULLNOTFOUND)
                                       /* determine size of compressed profile */
  iCompressedsize = iRawlen - (pTemp - pRawdata) - 2;
                                       /* decompress the profile */
  iRetcode = inflate_buffer (pData, pTemp+2, iCompressedsize,
                             &pBuf, &iBufsize, &iProfilesize);

  if (iRetcode)                        /* on error bail out */
  {
    MNG_FREE (pData, pBuf, iBufsize)   /* don't forget to drop the temp buffer */
    return iRetcode;
  }

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
  {
    mng_imagep pImage = (mng_imagep)pData->pCurrentobj;

    if (!pImage)                       /* no object then dump it in obj 0 */
      pImage = (mng_imagep)pData->pObjzero;
                                       /* store as local */
                                       /* allocate a buffer & copy it */
    MNG_ALLOC (pData, pImage->pImgbuf->pProfile, iProfilesize)
    MNG_COPY  (pImage->pImgbuf->pProfile, pBuf, iProfilesize)
                                       /* store it's length as well */
    pImage->pImgbuf->iProfilesize = iProfilesize;
    pImage->pImgbuf->bHasICCP     = MNG_TRUE;
  }
  else
  {                                    /* store as global */
    if (iRawlen == 0)                  /* empty chunk ? */
    {
      if (pData->pGlobalProfile)       /* did we have a global profile ? */
        MNG_FREE (pData, pData->pGlobalProfile, pData->iGlobalProfilesize)

      pData->iGlobalProfilesize = 0;   /* reset to null */
      pData->pGlobalProfile     = 0; 
    }
    else
    {                                  /* allocate a global buffer & copy it */
      MNG_ALLOC (pData, pData->pGlobalProfile, iProfilesize)
      MNG_COPY  (pData->pGlobalProfile, pBuf, iProfilesize)
                                       /* store it's length as well */
      pData->iGlobalProfilesize = iProfilesize;
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */  

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
    {
      MNG_FREE (pData, pBuf, iBufsize) /* don't forget to drop the temp buffer */
      return iRetcode;
    }
                                       /* store the fields */
    ((mng_iccpp)*ppChunk)->bEmpty = (mng_bool)(iRawlen == 0);

    if (iRawlen)                       /* not empty ? */
    {
      if (!pBuf)                       /* hasn't been unpuzzled it yet ? */
      {
        pTemp = find_null (pRawdata);  /* find null-separator */
                                       /* not found inside input-data ? */
        if ((pTemp - pRawdata) > (mng_int32)iRawlen)
          MNG_ERROR (pData, MNG_NULLNOTFOUND)
                                       /* determine size of compressed profile */
        iCompressedsize = iRawlen - (pTemp - pRawdata) - 2;
                                       /* decompress the profile */
        iRetcode = inflate_buffer (pData, pTemp+2, iCompressedsize,
                                   &pBuf, &iBufsize, &iProfilesize);

        if (iRetcode)                  /* on error bail out */
        {                              /* don't forget to drop the temp buffer */
          MNG_FREE (pData, pBuf, iBufsize)
          return iRetcode;
        }
      }

      ((mng_iccpp)*ppChunk)->iNamesize = (mng_uint32)(pTemp - pRawdata);

      if (((mng_iccpp)*ppChunk)->iNamesize)
      {
        MNG_ALLOC (pData, ((mng_iccpp)*ppChunk)->zName,
                          ((mng_iccpp)*ppChunk)->iNamesize + 1)
        MNG_COPY  (((mng_iccpp)*ppChunk)->zName, pRawdata,
                   ((mng_iccpp)*ppChunk)->iNamesize)
      }

      ((mng_iccpp)*ppChunk)->iCompression = *(pTemp+1);
      ((mng_iccpp)*ppChunk)->iProfilesize = iProfilesize;

      MNG_ALLOC (pData, ((mng_iccpp)*ppChunk)->pProfile, iProfilesize)
      MNG_COPY  (((mng_iccpp)*ppChunk)->pProfile, pBuf, iProfilesize)
    }
  }
#endif /* MNG_STORE_CHUNKS */

  if (pBuf)
    MNG_FREE (pData, pBuf, iBufsize)   /* free the temporary buffer */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ICCP, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_text)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

  mng_uint32 iKeywordlen, iTextlen;
  mng_pchar  zKeyword, zText;
  mng_uint8p pTemp;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TEXT, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen < 2)                     /* length must be at least 2 */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  pTemp = find_null (pRawdata);        /* find the null separator */
                                       /* not found inside input-data ? */
  if ((pTemp - pRawdata) > (mng_int32)iRawlen)
    MNG_ERROR (pData, MNG_NULLNOTFOUND)

  if (pTemp == pRawdata)               /* there must be at least 1 char for keyword */
    MNG_ERROR (pData, MNG_KEYWORDNULL)

  iKeywordlen = (mng_uint32)(pTemp - pRawdata);
  iTextlen    = iRawlen - iKeywordlen - 1;

  if (pData->fProcesstext)             /* inform the application ? */
  {
    MNG_ALLOC (pData, zKeyword, iKeywordlen + 1)
    MNG_COPY  (zKeyword, pRawdata, iKeywordlen)

    MNG_ALLOCX (pData, zText, iTextlen + 1)

    if (!zText)                        /* on error bail out */
    {
      MNG_FREEX (pData, zKeyword, iKeywordlen + 1)
      MNG_ERROR (pData, MNG_OUTOFMEMORY)
    }

    if (iTextlen)
      MNG_COPY (zText, pTemp+1, iTextlen)

    pData->fProcesstext ((mng_handle)pData, MNG_TYPE_TEXT, zKeyword, zText, 0, 0);

    MNG_FREEX (pData, zText, iTextlen + 1)
    MNG_FREEX (pData, zKeyword, iKeywordlen + 1)
  }

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_textp)*ppChunk)->iKeywordsize = iKeywordlen;
    ((mng_textp)*ppChunk)->iTextsize    = iTextlen;

    if (iKeywordlen)
    {
      MNG_ALLOC (pData, ((mng_textp)*ppChunk)->zKeyword, iKeywordlen + 1)
      MNG_COPY  (((mng_textp)*ppChunk)->zKeyword, pRawdata, iKeywordlen)
    }

    if (iTextlen)
    {
      MNG_ALLOC (pData, ((mng_textp)*ppChunk)->zText, iTextlen + 1)
      MNG_COPY  (((mng_textp)*ppChunk)->zText, pRawdata, iTextlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TEXT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_ztxt)
{
  mng_retcode iRetcode;
  mng_uint32  iKeywordlen, iTextlen;
  mng_pchar   zKeyword;
  mng_uint8p  pTemp;
  mng_uint32  iCompressedsize;
  mng_uint32  iBufsize;
  mng_uint8p  pBuf;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ZTXT, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen < 3)                     /* length must be at least 3 */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  pTemp = find_null (pRawdata);        /* find the null separator */
                                       /* not found inside input-data ? */
  if ((pTemp - pRawdata) > (mng_int32)iRawlen)
    MNG_ERROR (pData, MNG_NULLNOTFOUND)

  if (pTemp == pRawdata)               /* there must be at least 1 char for keyword */
    MNG_ERROR (pData, MNG_KEYWORDNULL)

  if (*(pTemp+1) != 0)                 /* only deflate compression-method allowed */
    MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

  iKeywordlen     = (mng_uint32)(pTemp - pRawdata);
  iCompressedsize = (mng_uint32)(iRawlen - iKeywordlen - 2);

  zKeyword        = 0;                 /* there's no keyword buffer yet */
  pBuf            = 0;                 /* or a temporary buffer ! */

  if (pData->fProcesstext)             /* inform the application ? */
  {                                    /* decompress the text */
    iRetcode = inflate_buffer (pData, pTemp+2, iCompressedsize,
                               &pBuf, &iBufsize, &iTextlen);

    if (iRetcode)                      /* on error bail out */
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, pBuf, iBufsize)
      return iRetcode;
    }

    MNG_ALLOCX (pData, zKeyword, iKeywordlen + 1)

    if (!zKeyword)                     /* on error bail out */
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, pBuf, iBufsize)
      return iRetcode;
    }

    MNG_COPY (zKeyword, pRawdata, iKeywordlen)

    pData->fProcesstext ((mng_handle)pData, MNG_TYPE_ZTXT, zKeyword, (mng_pchar)pBuf, 0, 0);
  }

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, pBuf, iBufsize)
      MNG_FREEX (pData, zKeyword, iKeywordlen + 1)
      return iRetcode;
    }
                                       /* store the fields */
    ((mng_ztxtp)*ppChunk)->iKeywordsize = iKeywordlen;
    ((mng_ztxtp)*ppChunk)->iCompression = *(pTemp+1);

    if ((!pBuf) && (iCompressedsize))  /* did we not get a text-buffer yet ? */
    {                                  /* decompress the text */
      iRetcode = inflate_buffer (pData, pTemp+2, iCompressedsize,
                                 &pBuf, &iBufsize, &iTextlen);

      if (iRetcode)                    /* on error bail out */
      {                                /* don't forget to drop the temp buffers */
        MNG_FREEX (pData, pBuf, iBufsize)
        MNG_FREEX (pData, zKeyword, iKeywordlen + 1)
        return iRetcode;
      }
    }

    MNG_ALLOCX (pData, ((mng_ztxtp)*ppChunk)->zKeyword, iKeywordlen + 1)

    if (!zKeyword)                     /* on error bail out */
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, pBuf, iBufsize)
      MNG_FREEX (pData, zKeyword, iKeywordlen + 1)
      return iRetcode;
    }

    MNG_COPY  (((mng_ztxtp)*ppChunk)->zKeyword, pRawdata, iKeywordlen)

    ((mng_ztxtp)*ppChunk)->iTextsize = iTextlen;

    if (iCompressedsize)
    {
      MNG_ALLOCX (pData, ((mng_ztxtp)*ppChunk)->zText, iTextlen + 1)
                                       /* on error bail out */
      if (!((mng_ztxtp)*ppChunk)->zText)
      {                                /* don't forget to drop the temp buffers */
        MNG_FREEX (pData, pBuf, iBufsize)
        MNG_FREEX (pData, zKeyword, iKeywordlen + 1)
        return iRetcode;
      }

      MNG_COPY (((mng_ztxtp)*ppChunk)->zText, pBuf, iTextlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */

  MNG_FREEX (pData, pBuf, iBufsize)    /* free the temporary buffers */
  MNG_FREEX (pData, zKeyword, iKeywordlen + 1)

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ZTXT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_itxt)
{
  mng_retcode iRetcode;
  mng_uint32  iKeywordlen, iTextlen, iLanguagelen, iTranslationlen;
  mng_pchar   zKeyword, zLanguage, zTranslation;
  mng_uint8p  pNull1, pNull2, pNull3;
  mng_uint32  iCompressedsize;
  mng_uint32  iBufsize;
  mng_uint8p  pBuf;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ITXT, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen < 6)                     /* length must be at least 6 */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  pNull1 = find_null (pRawdata);       /* find the null separators */
  pNull2 = find_null (pNull1+3);
  pNull3 = find_null (pNull2+1);
                                       /* not found inside input-data ? */
  if (((pNull1 - pRawdata) > (mng_int32)iRawlen) ||
      ((pNull2 - pRawdata) > (mng_int32)iRawlen) ||
      ((pNull3 - pRawdata) > (mng_int32)iRawlen)    )
    MNG_ERROR (pData, MNG_NULLNOTFOUND)

  if (pNull1 == pRawdata)              /* there must be at least 1 char for keyword */
    MNG_ERROR (pData, MNG_KEYWORDNULL)
                                       /* compression or not ? */
  if ((*(pNull1+1) != 0) && (*(pNull1+1) != 1))
    MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

  if (*(pNull1+2) != 0)                /* only deflate compression-method allowed */
    MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

  iKeywordlen     = (mng_uint32)(pNull1 - pRawdata);
  iLanguagelen    = (mng_uint32)(pNull2 - pNull1 - 3);
  iTranslationlen = (mng_uint32)(pNull3 - pNull2 - 1);
  iCompressedsize = (mng_uint32)(iRawlen - iKeywordlen - iLanguagelen - iTranslationlen - 5);

  zKeyword     = 0;                    /* no buffers acquired yet */
  zLanguage    = 0;
  zTranslation = 0;
  pBuf         = 0;
  iTextlen     = 0;

  if (pData->fProcesstext)             /* inform the application ? */
  {                                    /* decompress the text */
    iRetcode = inflate_buffer (pData, pNull3+1, iCompressedsize,
                               &pBuf, &iBufsize, &iTextlen);

    if (iRetcode)                      /* on error bail out */
    {                                  /* don't forget to drop the temp buffer */
      MNG_FREEX (pData, pBuf, iBufsize)
      return iRetcode;
    }

    MNG_ALLOCX (pData, zKeyword,     iKeywordlen     + 1)
    MNG_ALLOCX (pData, zLanguage,    iLanguagelen    + 1)
    MNG_ALLOCX (pData, zTranslation, iTranslationlen + 1)
                                       /* on error bail out */
    if ((!zKeyword) || (!zLanguage) || (!zTranslation))
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, zTranslation, iTranslationlen + 1)
      MNG_FREEX (pData, zLanguage,    iLanguagelen    + 1)
      MNG_FREEX (pData, zKeyword,     iKeywordlen     + 1)
      MNG_FREEX (pData, pBuf, iBufsize)
      MNG_ERROR (pData, MNG_OUTOFMEMORY)
    }

    MNG_COPY (zKeyword,     pRawdata, iKeywordlen)
    MNG_COPY (zLanguage,    pNull1+3, iLanguagelen)
    MNG_COPY (zTranslation, pNull2+1, iTranslationlen)

    pData->fProcesstext ((mng_handle)pData, MNG_TYPE_ZTXT, zKeyword, (mng_pchar)pBuf,
                                                           zLanguage, zTranslation);
  }

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, zTranslation, iTranslationlen + 1)
      MNG_FREEX (pData, zLanguage,    iLanguagelen    + 1)
      MNG_FREEX (pData, zKeyword,     iKeywordlen     + 1)
      MNG_FREEX (pData, pBuf,         iBufsize)
      return iRetcode;
    }
                                       /* store the fields */
    ((mng_itxtp)*ppChunk)->iKeywordsize       = iKeywordlen;
    ((mng_itxtp)*ppChunk)->iLanguagesize      = iLanguagelen;
    ((mng_itxtp)*ppChunk)->iTranslationsize   = iTranslationlen;
    ((mng_itxtp)*ppChunk)->iCompressionflag   = *(pNull1+1);
    ((mng_itxtp)*ppChunk)->iCompressionmethod = *(pNull1+2);

    if ((!pBuf) && (iCompressedsize))  /* did we not get a text-buffer yet ? */
    {                                  /* decompress the text */
      iRetcode = inflate_buffer (pData, pNull3+1, iCompressedsize,
                                 &pBuf, &iBufsize, &iTextlen);

      if (iRetcode)                    /* on error bail out */
      {                                /* don't forget to drop the temp buffers */
        MNG_FREEX (pData, zTranslation, iTranslationlen + 1)
        MNG_FREEX (pData, zLanguage,    iLanguagelen    + 1)
        MNG_FREEX (pData, zKeyword,     iKeywordlen     + 1)
        MNG_FREEX (pData, pBuf,         iBufsize)
        return iRetcode;
      }
    }

    MNG_ALLOCX (pData, ((mng_itxtp)*ppChunk)->zKeyword,     iKeywordlen     + 1)
    MNG_ALLOCX (pData, ((mng_itxtp)*ppChunk)->zLanguage,    iLanguagelen    + 1)
    MNG_ALLOCX (pData, ((mng_itxtp)*ppChunk)->zTranslation, iTranslationlen + 1)
                                       /* on error bail out */
    if ((!((mng_itxtp)*ppChunk)->zKeyword    ) ||
        (!((mng_itxtp)*ppChunk)->zLanguage   ) ||
        (!((mng_itxtp)*ppChunk)->zTranslation)    )
    {                                  /* don't forget to drop the temp buffers */
      MNG_FREEX (pData, zTranslation, iTranslationlen + 1)
      MNG_FREEX (pData, zLanguage,    iLanguagelen    + 1)
      MNG_FREEX (pData, zKeyword,     iKeywordlen     + 1)
      MNG_FREEX (pData, pBuf,         iBufsize)
      MNG_ERROR (pData, MNG_OUTOFMEMORY)
    }

    MNG_COPY (((mng_itxtp)*ppChunk)->zKeyword,     pRawdata, iKeywordlen)
    MNG_COPY (((mng_itxtp)*ppChunk)->zLanguage,    pNull1+3, iLanguagelen)
    MNG_COPY (((mng_itxtp)*ppChunk)->zTranslation, pNull2+1, iTranslationlen)

    ((mng_itxtp)*ppChunk)->iTextsize = iTextlen;

    if (iCompressedsize)
    {
      MNG_ALLOCX (pData, ((mng_itxtp)*ppChunk)->zText, iTextlen + 1)

      if (!((mng_itxtp)*ppChunk)->zText)
      {                                /* don't forget to drop the temp buffers */
        MNG_FREEX (pData, zTranslation, iTranslationlen + 1)
        MNG_FREEX (pData, zLanguage,    iLanguagelen    + 1)
        MNG_FREEX (pData, zKeyword,     iKeywordlen     + 1)
        MNG_FREEX (pData, pBuf,         iBufsize)
        MNG_ERROR (pData, MNG_OUTOFMEMORY)
      }

      MNG_COPY  (((mng_itxtp)*ppChunk)->zText, pBuf, iTextlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */
                                       /* free the temporary buffers */
  MNG_FREEX (pData, zTranslation, iTranslationlen + 1)
  MNG_FREEX (pData, zLanguage,    iLanguagelen    + 1)
  MNG_FREEX (pData, zKeyword,     iKeywordlen     + 1)
  MNG_FREEX (pData, pBuf,         iBufsize)

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ITXT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_bkgd)
{
#ifdef MNG_SUPPORT_DISPLAY
  mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
  mng_imagedatap pBuf;
#endif
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_BKGD, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIDAT) || (pData->bHasJDAT))
#else
  if (pData->bHasIDAT)
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen > 6)                     /* it just can't be bigger than that! */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_INCLUDE_JNG                 /* length checks */
  if (pData->bHasJHDR)
  {
    if (((pData->iJHDRcolortype == 8) || (pData->iJHDRcolortype == 12)) && (iRawlen != 2))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)

    if (((pData->iJHDRcolortype == 10) || (pData->iJHDRcolortype == 14)) && (iRawlen != 6))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }
  else
#endif /* MNG_INCLUDE_JNG */
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
  {
    if (((pData->iColortype == 0) || (pData->iColortype == 4)) && (iRawlen != 2))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)

    if (((pData->iColortype == 2) || (pData->iColortype == 6)) && (iRawlen != 6))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)

    if ((pData->iColortype == 3) && (iRawlen != 1))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }
  else
  {
    if (iRawlen != 6)                  /* global is always 16-bit RGB ! */
      MNG_ERROR (pData, MNG_INVALIDLENGTH)
  }

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    pData->bHasBKGD = MNG_TRUE;        /* indicate bKGD available */
  else
    pData->bHasglobalBKGD = (mng_bool)(iRawlen != 0);

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
      case  8 : ;
      case 12 : {                      /* gray */
                  pBuf->iBKGDgray  = mng_get_uint16 (pRawdata);
                  break;
                }
      case 10 : ;
      case 14 : {                      /* rgb */
                  pBuf->iBKGDred   = mng_get_uint16 (pRawdata);
                  pBuf->iBKGDgreen = mng_get_uint16 (pRawdata+2);
                  pBuf->iBKGDblue  = mng_get_uint16 (pRawdata+4);
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
      case 0: {                        /* gray */
                pBuf->iBKGDgray  = mng_get_uint16 (pRawdata);
                break;
              }
      case 2: {                        /* rgb */
                pBuf->iBKGDred   = mng_get_uint16 (pRawdata);
                pBuf->iBKGDgreen = mng_get_uint16 (pRawdata+2);
                pBuf->iBKGDblue  = mng_get_uint16 (pRawdata+4);
                break;
              }
      case 3: {                        /* indexed */
                pBuf->iBKGDindex = *pRawdata;
                break;
              }
    }
  }
  else                                 /* store as global */
  {
    if (iRawlen)
    {
      pData->iGlobalBKGDred   = mng_get_uint16 (pRawdata);
      pData->iGlobalBKGDgreen = mng_get_uint16 (pRawdata+2);
      pData->iGlobalBKGDblue  = mng_get_uint16 (pRawdata+4);
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_bkgdp)*ppChunk)->bEmpty = (mng_bool)(iRawlen == 0);

    if (iRawlen)
    {
      switch (iRawlen)                 /* guess from length */
      {
        case 1 : {                     /* indexed */
                   ((mng_bkgdp)*ppChunk)->iType  = 3;
                   ((mng_bkgdp)*ppChunk)->iIndex = *pRawdata;
                   break;
                 }
        case 2 : {                     /* gray */
                   ((mng_bkgdp)*ppChunk)->iType  = 0;
                   ((mng_bkgdp)*ppChunk)->iGray  = mng_get_uint16 (pRawdata);
                   break;
                 }
        case 6 : {                     /* rgb */
                   ((mng_bkgdp)*ppChunk)->iType  = 2;
                   ((mng_bkgdp)*ppChunk)->iRed   = mng_get_uint16 (pRawdata);
                   ((mng_bkgdp)*ppChunk)->iGreen = mng_get_uint16 (pRawdata+2);
                   ((mng_bkgdp)*ppChunk)->iBlue  = mng_get_uint16 (pRawdata+4);
                   break;
                 }
      }
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_BKGD, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_phys)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PHYS, MNG_LC_START);
#endif


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PHYS, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_sbit)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SBIT, MNG_LC_START);
#endif


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SBIT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_splt)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SPLT, MNG_LC_START);
#endif


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SPLT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_hist)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_HIST, MNG_LC_START);
#endif


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_HIST, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_time)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TIME, MNG_LC_START);
#endif
                                       /* sequence checks */
#ifdef MNG_INCLUDE_JNG
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR) && (!pData->bHasJHDR))
#else
  if ((!pData->bHasMHDR) && (!pData->bHasIHDR) &&
      (!pData->bHasBASI) && (!pData->bHasDHDR)    )
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen != 7)                    /* length must be exactly 7 */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

/*  if (pData->fProcesstime) */            /* inform the application ? */
/*  {

    pData->fProcesstime ((mng_handle)pData, );
  } */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_timep)*ppChunk)->iYear   = mng_get_uint16 (pRawdata);
    ((mng_timep)*ppChunk)->iMonth  = *(pRawdata+2);
    ((mng_timep)*ppChunk)->iDay    = *(pRawdata+3);
    ((mng_timep)*ppChunk)->iHour   = *(pRawdata+4);
    ((mng_timep)*ppChunk)->iMinute = *(pRawdata+5);
    ((mng_timep)*ppChunk)->iSecond = *(pRawdata+6);
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TIME, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_mhdr)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_MHDR, MNG_LC_START);
#endif

  if (iRawlen != 28)                   /* correct length ? */
    MNG_ERROR (pData, MNG_INVALIDLENGTH);

  if (pData->eSigtype != mng_it_mng)   /* sequence checks */
    MNG_ERROR (pData, MNG_CHUNKNOTALLOWED)

  if (pData->bHasheader)               /* can only be the first chunk! */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  pData->bHasMHDR       = MNG_TRUE;    /* oh boy, a real MNG */
  pData->bHasheader     = MNG_TRUE;    /* we've got a header */
  pData->eImagetype     = mng_it_mng;  /* fill header fields */
  pData->iWidth         = mng_get_uint32 (pRawdata);
  pData->iHeight        = mng_get_uint32 (pRawdata+4);
  pData->iTicks         = mng_get_uint32 (pRawdata+8);
  pData->iLayercount    = mng_get_uint32 (pRawdata+12);
  pData->iFramecount    = mng_get_uint32 (pRawdata+16);
  pData->iPlaytime      = mng_get_uint32 (pRawdata+20);
  pData->iSimplicity    = mng_get_uint32 (pRawdata+24);
                                       /* fits on maximum canvas ? */
  if ((pData->iWidth > pData->iMaxwidth) || (pData->iHeight > pData->iMaxheight))
    MNG_WARNING (pData, MNG_IMAGETOOLARGE)

  if (pData->fProcessheader)           /* inform the app ? */
    pData->fProcessheader (((mng_handle)pData), pData->iWidth, pData->iHeight);

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_mhdrp)*ppChunk)->iWidth      = pData->iWidth;
    ((mng_mhdrp)*ppChunk)->iHeight     = pData->iHeight;
    ((mng_mhdrp)*ppChunk)->iTicks      = pData->iTicks;
    ((mng_mhdrp)*ppChunk)->iLayercount = pData->iLayercount;
    ((mng_mhdrp)*ppChunk)->iFramecount = pData->iFramecount;
    ((mng_mhdrp)*ppChunk)->iPlaytime   = pData->iPlaytime;
    ((mng_mhdrp)*ppChunk)->iSimplicity = pData->iSimplicity;
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_MHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_mend)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_MEND, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen > 0)                     /* must not contain data! */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {                                    /* do something */
    iRetcode = process_display_mend (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

  pData->bHasMHDR = MNG_FALSE;         /* end of the line, bro! */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_MEND, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_loop)
{
#if defined(MNG_STORE_CHUNKS) || defined (MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_LOOP, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen >= 5)                    /* length checks */
  {
    if (iRawlen >= 6)
    {
      if ((iRawlen - 6) % 4 != 0)
        MNG_ERROR (pData, MNG_INVALIDLENGTH)
    }
  }
  else
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_uint8     iLevel;
    mng_uint32    iRepeat;
    mng_uint8     iTermination = 0;
    mng_uint32    iItermin     = 1;
    mng_uint32    iItermax     = 0x7fffffffL;
    mng_ani_loopp pLOOP;

    pData->bHasLOOP = MNG_TRUE;        /* indicate we're inside a loop */

    iLevel  = *pRawdata;               /* determine the fields for processing */
    iRepeat = mng_get_uint32 (pRawdata+1);

    if (iRawlen >= 6)
    {
      iTermination = *(pRawdata+5);

      if (iRawlen >= 10)
      {
        iItermin = mng_get_uint32 (pRawdata+6);

        if (iRawlen >= 14)
        {
          iItermax = mng_get_uint32 (pRawdata+10);

          /* TODO: process signals */

        }
      }
    }
                                       /* create the LOOP ani-object */
    iRetcode = create_ani_loop (pData, iLevel, iRepeat, iTermination,
                                       iItermin, iItermax, 0, 0, &pLOOP);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    if (iRawlen >= 5)                  /* store the fields */
    {
      ((mng_loopp)*ppChunk)->iLevel  = *pRawdata;
      ((mng_loopp)*ppChunk)->iRepeat = mng_get_uint32 (pRawdata+1);

      if (iRawlen >= 6)
      {
        ((mng_loopp)*ppChunk)->iTermination = *(pRawdata+5);

        if (iRawlen >= 10)
        {
          ((mng_loopp)*ppChunk)->iItermin = mng_get_uint32 (pRawdata+6);

          if (iRawlen >= 14)
          {
            ((mng_loopp)*ppChunk)->iItermax = mng_get_uint32 (pRawdata+10);
            ((mng_loopp)*ppChunk)->iCount   = (iRawlen - 14) / 4;

            if (((mng_loopp)*ppChunk)->iCount)
            {
              MNG_ALLOC (pData, ((mng_loopp)*ppChunk)->pSignals,
                                ((mng_loopp)*ppChunk)->iCount << 2)

#ifdef MNG_SWAP_ENDIAN
              {
                mng_uint32  iX;
                mng_uint8p  pIn  = pRawdata + 14;
                mng_uint32p pOut = (mng_uint32p)((mng_loopp)*ppChunk)->pSignals;

                for (iX = 0; iX < ((mng_loopp)*ppChunk)->iCount; iX++)
                {
                  *pOut++ = mng_get_uint32 (pIn);
                  pIn += 4;
                }
              }
#else
              MNG_COPY (((mng_loopp)*ppChunk)->pSignals, pRawdata + 14,
                        ((mng_loopp)*ppChunk)->iCount << 2)
#endif /* MNG_SWAP_ENDIAN */
            }
          }
        }
      }
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_LOOP, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_endl)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ENDL, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen != 1)                    /* length must be exactly 1 */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    if (pData->bHasLOOP)               /* are we really processing a loop ? */
    {                                  /* get the nest level */
      mng_uint8     iLevel = *pRawdata;
      mng_ani_endlp pENDL;
                                       /* create an ENDL animation object */
      iRetcode = create_ani_endl (pData, iLevel, &pENDL);

      if (!iRetcode)                   /* still oke ? then process it */
        iRetcode = pENDL->sHeader.fProcess (pData, pENDL);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
    else
    {

      /* TODO: error abort ??? */

    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_endlp)*ppChunk)->iLevel = *pRawdata;
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ENDL, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_defi)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNF_SUPPORT_DISPLAY)
  mng_retcode iRetcode = MNG_NOERROR;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DEFI, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* check the length */
  if ((iRawlen != 2) && (iRawlen != 3) && (iRawlen != 4) &&
      (iRawlen != 12) && (iRawlen != 28))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    pData->iDEFIobjectid    = mng_get_uint16 (pRawdata);

    if (iRawlen > 2)
      pData->iDEFIdonotshow = *(pRawdata+2);
    else
      pData->iDEFIdonotshow = 0;

    if (iRawlen > 3)
      pData->iDEFIconcrete  = *(pRawdata+3);
    else
      pData->iDEFIconcrete  = 0;

    if (iRawlen > 4)
    {
      pData->bDEFIhasloca   = MNG_TRUE;
      pData->iDEFIlocax     = mng_get_int32 (pRawdata+4);
      pData->iDEFIlocay     = mng_get_int32 (pRawdata+8);
    }
    else
      pData->bDEFIhasloca   = MNG_FALSE;

    if (iRawlen > 12)
    {
      pData->bDEFIhasclip   = MNG_TRUE;
      pData->iDEFIclipl     = mng_get_int32 (pRawdata+12);
      pData->iDEFIclipr     = mng_get_int32 (pRawdata+16);
      pData->iDEFIclipt     = mng_get_int32 (pRawdata+20);
      pData->iDEFIclipb     = mng_get_int32 (pRawdata+24);
    }
    else
      pData->bDEFIhasclip   = MNG_FALSE;
                                       /* are we processing loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_defip pDEFI;             /* create an animation object then */
      iRetcode = create_ani_defi (pData, &pDEFI);
    }

    if (!iRetcode)                     /* do the display processing */
      iRetcode = process_display_defi (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_defip)*ppChunk)->iObjectid    = mng_get_uint16 (pRawdata);

    if (iRawlen > 2)
      ((mng_defip)*ppChunk)->iDonotshow = *(pRawdata+2);

    if (iRawlen > 3)
      ((mng_defip)*ppChunk)->iConcrete  = *(pRawdata+3);

    if (iRawlen > 4)
    {
      ((mng_defip)*ppChunk)->bHasloca   = MNG_TRUE;
      ((mng_defip)*ppChunk)->iXlocation = mng_get_int32 (pRawdata+4);
      ((mng_defip)*ppChunk)->iYlocation = mng_get_int32 (pRawdata+8);
    }
    else
      ((mng_defip)*ppChunk)->bHasloca   = MNG_FALSE;

    if (iRawlen > 12)
    {
      ((mng_defip)*ppChunk)->bHasclip   = MNG_TRUE;
      ((mng_defip)*ppChunk)->iLeftcb    = mng_get_int32 (pRawdata+12);
      ((mng_defip)*ppChunk)->iRightcb   = mng_get_int32 (pRawdata+16);
      ((mng_defip)*ppChunk)->iTopcb     = mng_get_int32 (pRawdata+20);
      ((mng_defip)*ppChunk)->iBottomcb  = mng_get_int32 (pRawdata+24);
    }
    else
      ((mng_defip)*ppChunk)->bHasclip   = MNG_FALSE;

  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DEFI, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_basi)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode = MNG_NOERROR;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_BASI, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* check the length */
  if ((iRawlen != 13) && (iRawlen != 19) && (iRawlen != 21) && (iRawlen != 22))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  pData->bHasBASI     = MNG_TRUE;      /* inside a BASI-IEND block now */
                                       /* store interesting fields */
  pData->iDatawidth   = mng_get_uint32 (pRawdata);
  pData->iDataheight  = mng_get_uint32 (pRawdata+4);
  pData->iBitdepth    = *(pRawdata+8);
  pData->iColortype   = *(pRawdata+9);
  pData->iCompression = *(pRawdata+10);
  pData->iFilter      = *(pRawdata+11);
  pData->iInterlace   = *(pRawdata+12);

  if ((pData->iBitdepth !=  1) &&      /* parameter validity checks */
      (pData->iBitdepth !=  2) &&
      (pData->iBitdepth !=  4) &&
      (pData->iBitdepth !=  8) &&
      (pData->iBitdepth != 16)    )
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

  if (pData->iFilter != MNG_FILTER_ADAPTIVE)
    MNG_ERROR (pData, MNG_INVALIDFILTER)

  if ((pData->iInterlace != MNG_INTERLACE_NONE ) &&
      (pData->iInterlace != MNG_INTERLACE_ADAM7)    )
    MNG_ERROR (pData, MNG_INVALIDINTERLACE)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_uint16 iRed      = 0;
    mng_uint16 iGreen    = 0;
    mng_uint16 iBlue     = 0;
    mng_bool   bHasalpha = MNG_FALSE;
    mng_uint16 iAlpha    = 0xFFFF;
    mng_uint8  iViewable = 0;

    if (iRawlen > 13)                  /* get remaining fields, if any */
    {
      iRed      = mng_get_uint16 (pRawdata+13);
      iGreen    = mng_get_uint16 (pRawdata+15);
      iBlue     = mng_get_uint16 (pRawdata+17);
    }

    if (iRawlen > 19)
    {
      bHasalpha = MNG_TRUE;
      iAlpha    = mng_get_uint16 (pRawdata+19);
    }

    if (iRawlen > 21)
      iViewable = *(pRawdata+21);
                                       /* processing inside loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_basip pBASI;             /* create an animation object */
      iRetcode = create_ani_basi (pData, iRed, iGreen, iBlue,
                                  bHasalpha, iAlpha, iViewable, &pBASI);
    }

    if (!iRetcode)                     /* display-processing... */
      iRetcode = process_display_basi (pData, iRed, iGreen, iBlue,
                                       bHasalpha, iAlpha, iViewable);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_basip)*ppChunk)->iWidth       = mng_get_uint32 (pRawdata);
    ((mng_basip)*ppChunk)->iHeight      = mng_get_uint32 (pRawdata+4);
    ((mng_basip)*ppChunk)->iBitdepth    = *(pRawdata+8);
    ((mng_basip)*ppChunk)->iColortype   = *(pRawdata+9);
    ((mng_basip)*ppChunk)->iCompression = *(pRawdata+10);
    ((mng_basip)*ppChunk)->iFilter      = *(pRawdata+11);
    ((mng_basip)*ppChunk)->iInterlace   = *(pRawdata+12);

    if (iRawlen > 13)
    {
      ((mng_basip)*ppChunk)->iRed       = mng_get_uint16 (pRawdata+13);
      ((mng_basip)*ppChunk)->iGreen     = mng_get_uint16 (pRawdata+15);
      ((mng_basip)*ppChunk)->iBlue      = mng_get_uint16 (pRawdata+17);
    }

    if (iRawlen > 19)
      ((mng_basip)*ppChunk)->iAlpha     = mng_get_uint16 (pRawdata+19);

    if (iRawlen > 21)
      ((mng_basip)*ppChunk)->iViewable  = *(pRawdata+21);

  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_BASI, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_clon)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode = MNG_NOERROR;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_CLON, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* check the length */
  if ((iRawlen != 4) && (iRawlen != 5) && (iRawlen != 6) &&
      (iRawlen != 7) && (iRawlen != 16))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_uint16 iSourceid, iCloneid;
    mng_uint8  iClonetype    = 0;
    mng_bool   bHasdonotshow = MNG_FALSE;
    mng_uint8  iDonotshow    = 0;
    mng_uint8  iConcrete     = 0;
    mng_bool   bHasloca      = MNG_FALSE;
    mng_uint8  iLocationtype = 0;
    mng_int32  iLocationx    = 0;
    mng_int32  iLocationy    = 0;

    iSourceid       = mng_get_uint16 (pRawdata);
    iCloneid        = mng_get_uint16 (pRawdata+2);

    if (iRawlen > 4)
      iClonetype    = *(pRawdata+4);

    if (iRawlen > 5)
    {
      bHasdonotshow = MNG_TRUE;
      iDonotshow    = *(pRawdata+5);
    }

    if (iRawlen > 6)
      iConcrete     = *(pRawdata+6);

    if (iRawlen > 7)
    {
      bHasloca      = MNG_TRUE;
      iLocationtype = *(pRawdata+7);
      iLocationx    = mng_get_int32 (pRawdata+8);
      iLocationy    = mng_get_int32 (pRawdata+12);
    }
                                       /* processing inside loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_clonp pCLON;             /* create an animation object */
      iRetcode = create_ani_clon (pData, iSourceid, iCloneid, iClonetype,
                                  bHasdonotshow, iDonotshow, iConcrete,
                                  bHasloca, iLocationtype, iLocationx, iLocationy,
                                  &pCLON);
    }

    if (!iRetcode)                     /* do display processing */
      iRetcode = process_display_clon (pData, iSourceid, iCloneid, iClonetype,
                                       bHasdonotshow, iDonotshow, iConcrete,
                                       bHasloca, iLocationtype, iLocationx, iLocationy);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_clonp)*ppChunk)->iSourceid       = mng_get_uint16 (pRawdata);
    ((mng_clonp)*ppChunk)->iCloneid        = mng_get_uint16 (pRawdata+2);

    if (iRawlen > 4)
      ((mng_clonp)*ppChunk)->iClonetype    = *(pRawdata+4);

    if (iRawlen > 5)
      ((mng_clonp)*ppChunk)->iDonotshow    = *(pRawdata+5);

    if (iRawlen > 6)
      ((mng_clonp)*ppChunk)->iConcrete     = *(pRawdata+6);

    if (iRawlen > 7)
    {
      ((mng_clonp)*ppChunk)->bHasloca      = MNG_TRUE;
      ((mng_clonp)*ppChunk)->iLocationtype = *(pRawdata+7);
      ((mng_clonp)*ppChunk)->iLocationx    = mng_get_int32 (pRawdata+8);
      ((mng_clonp)*ppChunk)->iLocationy    = mng_get_int32 (pRawdata+12);
    }
    else
    {
      ((mng_clonp)*ppChunk)->bHasloca      = MNG_FALSE;
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_CLON, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_past)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PAST, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PAST, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_disc)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DISC, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if ((iRawlen % 2) != 0)              /* check the length */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_retcode iRetcode = process_display_disc (pData, (iRawlen / 2),
                                                 (mng_uint16p)pRawdata);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_discp)*ppChunk)->iCount = iRawlen / 2;

    MNG_ALLOC (pData, ((mng_discp)*ppChunk)->pObjectids, iRawlen)

#ifdef MNG_SWAP_ENDIAN
    {
      mng_uint32  iX;
      mng_uint8p  pIn  = pRawdata;
      mng_uint16p pOut = ((mng_discp)*ppChunk)->pObjectids;

      for (iX = 0; iX < ((mng_discp)*ppChunk)->iCount; iX++)
      {
        *pOut++ = mng_get_uint16 (pIn);
        pIn += 2;
      }  
    }
#else
    MNG_COPY (((mng_discp)*ppChunk)->pObjectids, pRawdata, iRawlen)
#endif /* MNG_SWAP_ENDIAN */
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DISC, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_back)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_BACK, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* check the length */
  if ((iRawlen != 6) && (iRawlen != 7) && (iRawlen != 9) && (iRawlen != 10))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {                                    /* store the fields for later */
/*    pData->bHasnextBACK         = MNG_TRUE;
    pData->iNextBACKred         = mng_get_uint16 (pRawdata);
    pData->iNextBACKgreen       = mng_get_uint16 (pRawdata+2);
    pData->iNextBACKblue        = mng_get_uint16 (pRawdata+4);

    if (iRawlen > 6)
      pData->iNextBACKmandatory = *(pRawdata+6);
    else
      pData->iNextBACKmandatory = 0;

    if (iRawlen > 7)
      pData->iNextBACKimageid   = mng_get_uint16 (pRawdata+7);
    else
      pData->iNextBACKimageid   = 0;

    if (iRawlen > 9)
      pData->iNextBACKtile      = *(pRawdata+9);
    else
      pData->iNextBACKtile      = 0; */

    pData->bHasBACK         = MNG_TRUE;
    pData->iBACKred         = mng_get_uint16 (pRawdata);
    pData->iBACKgreen       = mng_get_uint16 (pRawdata+2);
    pData->iBACKblue        = mng_get_uint16 (pRawdata+4);

    if (iRawlen > 6)
      pData->iBACKmandatory = *(pRawdata+6);
    else
      pData->iBACKmandatory = 0;

    if (iRawlen > 7)
      pData->iBACKimageid   = mng_get_uint16 (pRawdata+7);
    else
      pData->iBACKimageid   = 0;

    if (iRawlen > 9)
      pData->iBACKtile      = *(pRawdata+9);
    else
      pData->iBACKtile      = 0;
                                       /* processing inside loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_backp pBACK;             /* create an animation object */
/*      iRetcode = create_ani_back (pData, pData->iNextBACKred, pData->iNextBACKgreen,
                                  pData->iNextBACKblue, pData->iNextBACKmandatory,
                                  pData->iNextBACKimageid, pData->iNextBACKtile,
                                  &pBACK); */
      iRetcode = create_ani_back (pData, pData->iBACKred, pData->iBACKgreen,
                                  pData->iBACKblue, pData->iBACKmandatory,
                                  pData->iBACKimageid, pData->iBACKtile,
                                  &pBACK);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_backp)*ppChunk)->iRed         = mng_get_uint16 (pRawdata);
    ((mng_backp)*ppChunk)->iGreen       = mng_get_uint16 (pRawdata+2);
    ((mng_backp)*ppChunk)->iBlue        = mng_get_uint16 (pRawdata+4);

    if (iRawlen > 6)
      ((mng_backp)*ppChunk)->iMandatory = *(pRawdata+6);

    if (iRawlen > 7)
      ((mng_backp)*ppChunk)->iImageid   = mng_get_uint16 (pRawdata+7);

    if (iRawlen > 9)
      ((mng_backp)*ppChunk)->iTile      = *(pRawdata+9);

  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_BACK, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_fram)
{
  mng_uint8p pTemp;
  mng_uint32 iNamelen;
  mng_uint32 iRemain;
  mng_uint32 iRequired;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_FRAM, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen <= 1)                    /* only framing-mode ? */
  {
    iNamelen = 0;                      /* indicate so */
    iRemain  = 0;
  }
  else
  {
    pTemp = find_null (pRawdata+1);    /* find null-separator */
                                       /* not found inside input-data ? */
    if ((pTemp - pRawdata) > (mng_int32)iRawlen)
      MNG_ERROR (pData, MNG_NULLNOTFOUND)

    iNamelen = (pTemp - pRawdata - 1);
    iRemain  = iRawlen - (pTemp - pRawdata) - 1;
                                       /* remains must be empty or at least 4 bytes */
    if ((iRemain != 0) && (iRemain < 4))
      MNG_ERROR (pData, MNG_INVALIDLENGTH)

    if (iRemain)
    {
      iRequired = 4;                   /* calculate and check required remaining length */

      if (*(pTemp+1)) { iRequired += 4; }
      if (*(pTemp+2)) { iRequired += 4; }
      if (*(pTemp+3)) { iRequired += 17; }

      if (*(pTemp+4))
      {
        if ((iRemain - iRequired) % 4 != 0)
          MNG_ERROR (pData, MNG_INVALIDLENGTH)
      }
      else
      {
        if (iRemain != iRequired)
          MNG_ERROR (pData, MNG_INVALIDLENGTH)
      }
    }
  }

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_uint8p  pWork           = pTemp;
    mng_uint8   iFramemode      = 0;
    mng_uint8   iChangedelay    = 0;
    mng_uint32  iDelay          = 0;
    mng_uint8   iChangetimeout  = 0;
    mng_uint32  iTimeout        = 0;
    mng_uint8   iChangeclipping = 0;
    mng_uint8   iCliptype       = 0;
    mng_int32   iClipl          = 0;
    mng_int32   iClipr          = 0;
    mng_int32   iClipt          = 0;
    mng_int32   iClipb          = 0;
    mng_retcode iRetcode        = MNG_NOERROR;

    if (iRawlen)                       /* any data specified ? */
    {
      if (*(pRawdata))                 /* save the new framing mode ? */
        iFramemode = *(pRawdata);

      if (iRemain)
      {
        iChangedelay    = *(pWork+1);
        iChangetimeout  = *(pWork+2);
        iChangeclipping = *(pWork+3);
        pWork += 5;

        if (iChangedelay)              /* delay changed ? */
        {
          iDelay = mng_get_uint32 (pWork);
          pWork += 4;
        }

        if (iChangetimeout)            /* timeout changed ? */
        {
          iTimeout = mng_get_uint32 (pWork);
          pWork += 4;
        }

        if (iChangeclipping)           /* clipping changed ? */
        {
          iCliptype = *pWork;
          iClipl    = mng_get_int32 (pWork+1);
          iClipr    = mng_get_int32 (pWork+5);
          iClipt    = mng_get_int32 (pWork+9);
          iClipb    = mng_get_int32 (pWork+13);
        }
      }
    }
                                       /* processing inside loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_framp pFRAM;             /* create an animation object */
      iRetcode = create_ani_fram (pData, iFramemode, iChangedelay, iDelay,
                                  iChangetimeout, iTimeout,
                                  iChangeclipping, iCliptype,
                                  iClipl, iClipr, iClipt, iClipb,
                                  &pFRAM);
    }

    if (!iRetcode)                     /* now go and do something */
      iRetcode = process_display_fram (pData, iFramemode, iChangedelay, iDelay,
                                       iChangetimeout, iTimeout,
                                       iChangeclipping, iCliptype,
                                       iClipl, iClipr, iClipt, iClipb);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    mng_retcode iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_framp)*ppChunk)->bEmpty              = (mng_bool)(iRawlen == 0);

    if (iRawlen)
    {
      ((mng_framp)*ppChunk)->iMode             = *(pRawdata);
      ((mng_framp)*ppChunk)->iNamesize         = iNamelen;

      if (iNamelen)
      {
        MNG_ALLOC (pData, ((mng_framp)*ppChunk)->zName, iNamelen+1)
        MNG_COPY (((mng_framp)*ppChunk)->zName, pRawdata+1, iNamelen)
      }

      if (iRemain)
      {
        ((mng_framp)*ppChunk)->iChangedelay    = *(pTemp+1);
        ((mng_framp)*ppChunk)->iChangetimeout  = *(pTemp+2);
        ((mng_framp)*ppChunk)->iChangeclipping = *(pTemp+3);
        ((mng_framp)*ppChunk)->iChangesyncid   = *(pTemp+4);

        pTemp += 5;

        if (((mng_framp)*ppChunk)->iChangedelay)
        {
          ((mng_framp)*ppChunk)->iDelay        = mng_get_uint32 (pTemp);
          pTemp += 4;
        }

        if (((mng_framp)*ppChunk)->iChangetimeout)
        {
          ((mng_framp)*ppChunk)->iTimeout      = mng_get_uint32 (pTemp);
          pTemp += 4;
        }

        if (((mng_framp)*ppChunk)->iChangeclipping)
        {
          ((mng_framp)*ppChunk)->iBoundarytype = *pTemp;
          ((mng_framp)*ppChunk)->iBoundaryl    = mng_get_int32 (pTemp+1);
          ((mng_framp)*ppChunk)->iBoundaryr    = mng_get_int32 (pTemp+5);
          ((mng_framp)*ppChunk)->iBoundaryt    = mng_get_int32 (pTemp+9);
          ((mng_framp)*ppChunk)->iBoundaryb    = mng_get_int32 (pTemp+13);
          pTemp += 17;
        }

        if (((mng_framp)*ppChunk)->iChangesyncid)
        {
          ((mng_framp)*ppChunk)->iCount        = (iRemain - iRequired) / 4;

          if (((mng_framp)*ppChunk)->iCount)
          {
            MNG_ALLOC (pData, ((mng_framp)*ppChunk)->pSyncids,
                              ((mng_framp)*ppChunk)->iCount * 4);

#ifdef MNG_SWAP_ENDIAN
            {
              mng_uint32 iX;
              mng_uint32p pOut = ((mng_framp)*ppChunk)->pSyncids;

              for (iX = 0; iX < ((mng_framp)*ppChunk)->iCount; iX++)
              {
                *pOut++ = mng_get_uint32 (pTemp);
                pTemp += 4;
              }
            }
#else
            MNG_COPY (((mng_framp)*ppChunk)->pSyncids, pTemp,
                      ((mng_framp)*ppChunk)->iCount * 4)
#endif /* MNG_SWAP_ENDIAN */
          }
        }
      }
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_FRAM, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_move)
{
#if defined(MNG_STORE_CHUNKS) || defined (MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_MOVE, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen != 13)                   /* check the length */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {                                    /* are we processing a loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_movep pMOVE;
                                       /* create a MOVE animation object */
      iRetcode = create_ani_move (pData, mng_get_uint16 (pRawdata),
                                         mng_get_uint16 (pRawdata+2),
                                         *(pRawdata+4),
                                         mng_get_int32 (pRawdata+5),
                                         mng_get_int32 (pRawdata+9),
                                         &pMOVE);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
                                       /* process the move */
    iRetcode = process_display_move (pData,
                                     mng_get_uint16 (pRawdata),
                                     mng_get_uint16 (pRawdata+2),
                                     *(pRawdata+4),
                                     mng_get_int32 (pRawdata+5),
                                     mng_get_int32 (pRawdata+9));

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_movep)*ppChunk)->iFirstid  = mng_get_uint16 (pRawdata);
    ((mng_movep)*ppChunk)->iLastid   = mng_get_uint16 (pRawdata+2);
    ((mng_movep)*ppChunk)->iMovetype = *(pRawdata+4);
    ((mng_movep)*ppChunk)->iMovex    = mng_get_int32 (pRawdata+5);
    ((mng_movep)*ppChunk)->iMovey    = mng_get_int32 (pRawdata+9);
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_MOVE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_clip)
{
#if defined(MNG_STORE_CHUNKS) || defined (MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_CLIP, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen != 21)                   /* check the length */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {                                    /* are we processing a loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_clipp pCLIP;
                                       /* create a CLIP animation object */
      iRetcode = create_ani_clip (pData, mng_get_uint16 (pRawdata),
                                         mng_get_uint16 (pRawdata+2),
                                         *(pRawdata+4),
                                         mng_get_int32 (pRawdata+5),
                                         mng_get_int32 (pRawdata+9),
                                         mng_get_int32 (pRawdata+13),
                                         mng_get_int32 (pRawdata+17),
                                         &pCLIP);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
                                       /* process the clipping */
    iRetcode = process_display_clip (pData,
                                     mng_get_uint16 (pRawdata),
                                     mng_get_uint16 (pRawdata+2),
                                     *(pRawdata+4),
                                     mng_get_int32 (pRawdata+5),
                                     mng_get_int32 (pRawdata+9),
                                     mng_get_int32 (pRawdata+13),
                                     mng_get_int32 (pRawdata+17));

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_clipp)*ppChunk)->iFirstid  = mng_get_uint16 (pRawdata);
    ((mng_clipp)*ppChunk)->iLastid   = mng_get_uint16 (pRawdata+2);
    ((mng_clipp)*ppChunk)->iCliptype = *(pRawdata+4);
    ((mng_clipp)*ppChunk)->iClipl    = mng_get_int32 (pRawdata+5);
    ((mng_clipp)*ppChunk)->iClipr    = mng_get_int32 (pRawdata+9);
    ((mng_clipp)*ppChunk)->iClipt    = mng_get_int32 (pRawdata+13);
    ((mng_clipp)*ppChunk)->iClipb    = mng_get_int32 (pRawdata+17);
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_CLIP, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_show)
{
#if defined(MNG_STORE_CHUNKS) || defined(MNG_SUPPORT_DISPLAY)
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SHOW, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)
                                       /* check the length */
  if ((iRawlen != 0) && (iRawlen != 2) && (iRawlen != 4) && (iRawlen != 5))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    if (iRawlen)                       /* determine parameters if any */
    {
      pData->iSHOWfromid = mng_get_uint16 (pRawdata);

      if (iRawlen > 2)
        pData->iSHOWtoid = mng_get_uint16 (pRawdata+2);
      else
        pData->iSHOWtoid = pData->iSHOWfromid;

      if (iRawlen > 4)
        pData->iSHOWmode = *(pRawdata+4);
      else
        pData->iSHOWmode = 0;
    }
    else                               /* use defaults then */
    {
      pData->iSHOWmode   = 2;
      pData->iSHOWfromid = 1;
      pData->iSHOWtoid   = 65535;
    }
                                       /* are we processing a loop/term ? */
    if ((pData->bHasLOOP) || (pData->bHasTERM))
    {
      mng_ani_showp pSHOW;
                                       /* create a SHOW animation object */
      iRetcode = create_ani_show (pData, pData->iSHOWfromid, pData->iSHOWtoid,
                                         pData->iSHOWmode, &pSHOW);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
                                       /* go and do it! */
    iRetcode = process_display_show (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_showp)*ppChunk)->bEmpty      = (mng_bool)(iRawlen == 0);

    if (iRawlen)
    {
      ((mng_showp)*ppChunk)->iFirstid  = mng_get_uint16 (pRawdata);

      if (iRawlen > 2)
        ((mng_showp)*ppChunk)->iLastid = mng_get_uint16 (pRawdata+2);

      if (iRawlen > 4)
        ((mng_showp)*ppChunk)->iMode   = *(pRawdata+4);
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SHOW, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_term)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TERM, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (pData->bHasLOOP)                 /* no way, jose! */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (pData->bHasTERM)                 /* only 1 allowed! */
    MNG_ERROR (pData, MNG_MULTIPLEERROR)
                                       /* check the length */
  if ((iRawlen != 1) && (iRawlen != 10))
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  pData->bHasTERM = MNG_TRUE;

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    mng_uint8     iTermaction;
    mng_uint8     iIteraction = 0;
    mng_uint32    iDelay      = 0;
    mng_uint32    iItermax    = 0;
    mng_ani_termp pTERM;

    iTermaction = *pRawdata;           /* get the fields */

    if (iRawlen > 1)
    {
      iIteraction = *(pRawdata+1);
      iDelay      = mng_get_uint32 (pRawdata+2);
      iItermax    = mng_get_uint32 (pRawdata+6);
    }
                                       /* create the TERM ani-object */
    iRetcode = create_ani_term (pData, iTermaction, iIteraction,
                                       iDelay, iItermax, &pTERM);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_termp)*ppChunk)->iTermaction = *pRawdata;

    if (iRawlen > 1)
    {
      ((mng_termp)*ppChunk)->iIteraction = *(pRawdata+1);
      ((mng_termp)*ppChunk)->iDelay      = mng_get_uint32 (pRawdata+2);
      ((mng_termp)*ppChunk)->iItermax    = mng_get_uint32 (pRawdata+6);
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_TERM, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_save)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SAVE, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (pData->bHasSAVE))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length check */


  pData->bHasSAVE = MNG_TRUE;

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SAVE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_seek)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SEEK, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasSAVE))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length check */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_SEEK, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_expi)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_EXPI, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen < 3)                     /* check the length */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_expip)*ppChunk)->iSnapshotid = mng_get_uint16 (pRawdata);
    ((mng_expip)*ppChunk)->iNamesize   = iRawlen - 2;

    if (((mng_expip)*ppChunk)->iNamesize)
    {
      MNG_ALLOC (pData, ((mng_expip)*ppChunk)->zName,
                        ((mng_expip)*ppChunk)->iNamesize + 1)
      MNG_COPY (((mng_expip)*ppChunk)->zName, pRawdata+2,
                ((mng_expip)*ppChunk)->iNamesize)
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_EXPI, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_fpri)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_FPRI, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_FPRI, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_need)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_NEED, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_NEED, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_phyg)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PHYG, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PHYG, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG
READ_CHUNK (read_jhdr)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_JHDR, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((pData->eSigtype != mng_it_jng) && (pData->eSigtype != mng_it_mng))
    MNG_ERROR (pData, MNG_CHUNKNOTALLOWED)

  if ((pData->eSigtype == mng_it_jng) && (pData->iChunkseq > 1))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  if (iRawlen != 16)                   /* length oke ? */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)
                                       /* inside a JHDR-IEND block now */
  pData->bHasJHDR              = MNG_TRUE;
                                       /* and store interesting fields */
  pData->iDatawidth            = mng_get_uint32 (pRawdata);
  pData->iDataheight           = mng_get_uint32 (pRawdata+4);
  pData->iJHDRcolortype        = *(pRawdata+8);
  pData->iJHDRimgbitdepth      = *(pRawdata+9);
  pData->iJHDRimgcompression   = *(pRawdata+10);
  pData->iJHDRimginterlace     = *(pRawdata+11);
  pData->iJHDRalphabitdepth    = *(pRawdata+12);
  pData->iJHDRalphacompression = *(pRawdata+13);
  pData->iJHDRalphafilter      = *(pRawdata+14);
  pData->iJHDRalphainterlace   = *(pRawdata+15);
                                       /* parameter validity checks */
  if ((pData->iJHDRcolortype != MNG_COLORTYPE_JPEGGRAY  ) &&
      (pData->iJHDRcolortype != MNG_COLORTYPE_JPEGCOLOR ) &&
      (pData->iJHDRcolortype != MNG_COLORTYPE_JPEGGRAYA ) &&
      (pData->iJHDRcolortype != MNG_COLORTYPE_JPEGCOLORA)    )
    MNG_ERROR (pData, MNG_INVALIDCOLORTYPE)

  if ((pData->iJHDRimgbitdepth !=  8) &&
      (pData->iJHDRimgbitdepth != 12) &&
      (pData->iJHDRimgbitdepth != 20)    )
    MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

  if (pData->iJHDRimgcompression !=  MNG_COMPRESSION_BASELINEJPEG)
    MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

  if ((pData->iJHDRimginterlace !=  MNG_INTERLACE_SEQUENTIAL ) &&
      (pData->iJHDRimginterlace !=  MNG_INTERLACE_PROGRESSIVE)    )
    MNG_ERROR (pData, MNG_INVALIDINTERLACE)

  if ((pData->iJHDRcolortype == MNG_COLORTYPE_JPEGGRAYA ) ||
      (pData->iJHDRcolortype == MNG_COLORTYPE_JPEGCOLORA)    )
  {
    if ((pData->iJHDRalphabitdepth !=  1) &&
        (pData->iJHDRalphabitdepth !=  2) &&
        (pData->iJHDRalphabitdepth !=  4) &&
        (pData->iJHDRalphabitdepth !=  8) &&
        (pData->iJHDRalphabitdepth != 16)    )
      MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

    if (pData->iJHDRalphacompression !=  MNG_COMPRESSION_DEFLATE)
      MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

    if (pData->iJHDRalphafilter !=  MNG_FILTER_ADAPTIVE)
      MNG_ERROR (pData, MNG_INVALIDFILTER)

    if ((pData->iJHDRalphainterlace !=  MNG_INTERLACE_NONE ) &&
        (pData->iJHDRalphainterlace !=  MNG_INTERLACE_ADAM7)    )
      MNG_ERROR (pData, MNG_INVALIDINTERLACE)

  }
  else
  {
    if (pData->iJHDRalphabitdepth    != 0)
      MNG_ERROR (pData, MNG_INVALIDBITDEPTH)

    if (pData->iJHDRalphacompression != 0)
      MNG_ERROR (pData, MNG_INVALIDCOMPRESS)

    if (pData->iJHDRalphafilter      != 0)
      MNG_ERROR (pData, MNG_INVALIDFILTER)

    if (pData->iJHDRalphainterlace   != 0)
      MNG_ERROR (pData, MNG_INVALIDINTERLACE)
      
  }

  if (!pData->bHasheader)              /* first chunk ? */
  {
    pData->bHasheader = MNG_TRUE;      /* we've got a header */
    pData->eImagetype = mng_it_jng;    /* then this must be a JNG */
    pData->iWidth     = mng_get_uint32 (pRawdata);
    pData->iHeight    = mng_get_uint32 (pRawdata+4);
                                       /* fits on maximum canvas ? */
    if ((pData->iWidth > pData->iMaxwidth) || (pData->iHeight > pData->iMaxheight))
      MNG_WARNING (pData, MNG_IMAGETOOLARGE)

    if (pData->fProcessheader)         /* inform the app ? */
      pData->fProcessheader (((mng_handle)pData), pData->iWidth, pData->iHeight);
  }

  pData->iColortype = 0;               /* fake grayscale for other routines */

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    pData->fDisplayrow = 0;            /* do nothing by default */
    pData->fCorrectrow = 0;
    pData->fStorerow   = 0;
    pData->fProcessrow = 0;

    switch (pData->iJHDRalphabitdepth) /* determine alpha processing routine */
    {
      case  1 : {
                  if (!pData->iJHDRalphainterlace)
                    pData->fInitrowproc = (mng_ptr)init_g1_ni;
                  else
                    pData->fInitrowproc = (mng_ptr)init_g1_i;

                  break;
                }
      case  2 : {
                  if (!pData->iJHDRalphainterlace)
                    pData->fInitrowproc = (mng_ptr)init_g2_ni;
                  else
                    pData->fInitrowproc = (mng_ptr)init_g2_i;

                  break;
                }
      case  4 : {
                  if (!pData->iJHDRalphainterlace)
                    pData->fInitrowproc = (mng_ptr)init_g4_ni;
                  else
                    pData->fInitrowproc = (mng_ptr)init_g4_i;

                  break;
                }
      case  8 : {
                  if (!pData->iJHDRalphainterlace)
                    pData->fInitrowproc = (mng_ptr)init_g8_ni;
                  else
                    pData->fInitrowproc = (mng_ptr)init_g8_i;

                  break;
                }
      case 16 : {
                  if (!pData->iJHDRalphainterlace)
                    pData->fInitrowproc = (mng_ptr)init_g16_ni;
                  else
                    pData->fInitrowproc = (mng_ptr)init_g16_i;

                  break;
                }
    }


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_jhdrp)*ppChunk)->iWidth            = mng_get_uint32 (pRawdata);
    ((mng_jhdrp)*ppChunk)->iHeight           = mng_get_uint32 (pRawdata+4);
    ((mng_jhdrp)*ppChunk)->iColortype        = *(pRawdata+8);
    ((mng_jhdrp)*ppChunk)->iImagesampledepth = *(pRawdata+9);
    ((mng_jhdrp)*ppChunk)->iImagecompression = *(pRawdata+10);
    ((mng_jhdrp)*ppChunk)->iImageinterlace   = *(pRawdata+11);
    ((mng_jhdrp)*ppChunk)->iAlphasampledepth = *(pRawdata+12);
    ((mng_jhdrp)*ppChunk)->iAlphacompression = *(pRawdata+13);
    ((mng_jhdrp)*ppChunk)->iAlphafilter      = *(pRawdata+14);
    ((mng_jhdrp)*ppChunk)->iAlphainterlace   = *(pRawdata+15);
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_JHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}
#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG
READ_CHUNK (read_jdat)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_JDAT, MNG_LC_START);
#endif

  if (iRawlen == 0)                    /* can never be empty */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  if (!pData->bHasJHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  pData->bHasJDAT = MNG_TRUE;          /* got some JDAT now, don't we */

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something */
    

  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */
    ((mng_jdatp)*ppChunk)->bEmpty    = (mng_bool)(iRawlen == 0);
    ((mng_jdatp)*ppChunk)->iDatasize = iRawlen;

    if (iRawlen != 0)                  /* is there any data ? */
    {
      MNG_ALLOC (pData, ((mng_jdatp)*ppChunk)->pData, iRawlen)
      MNG_COPY  (((mng_jdatp)*ppChunk)->pData, pRawdata, iRawlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_JDAT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}
#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG
READ_CHUNK (read_jsep)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_JSEP, MNG_LC_START);
#endif

  if (iRawlen != 0)                    /* must be empty ! */
    MNG_ERROR (pData, MNG_INVALIDLENGTH)

  if (!pData->bHasJHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

  pData->bHasJSEP = MNG_TRUE;          /* indicate we've had the 8-/12-bit separator */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_JSEP, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}
#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */

READ_CHUNK (read_dhdr)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DHDR, MNG_LC_START);
#endif

  if (!pData->bHasMHDR)                /* sequence checks */
    MNG_ERROR (pData, MNG_SEQUENCEERROR)

#ifdef MNG_INCLUDE_JNG
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR) || (pData->bHasJHDR))
#else
  if ((pData->bHasIHDR) || (pData->bHasBASI) || (pData->bHasDHDR))
#endif
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


  pData->bHasDHDR = MNG_TRUE;          /* inside a DHDR-IEND block now */

#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {
    pData->fDisplayrow = 0;            /* do nothing by default */
    pData->fCorrectrow = 0;
    pData->fStorerow   = 0;
    pData->fProcessrow = 0;



    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_prom)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PROM, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PROM, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_ipng)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IPNG, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IPNG, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_pplt)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PPLT, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_PPLT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_ijng)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IJNG, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_IJNG, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_drop)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DROP, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DROP, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_dbyk)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DBYK, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_DBYK, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_ordr)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ORDR, MNG_LC_START);
#endif
                                       /* sequence checks */
  if ((!pData->bHasMHDR) || (!pData->bHasDHDR))
    MNG_ERROR (pData, MNG_SEQUENCEERROR)


  /* TODO: length & sequence checks */


#ifdef MNG_SUPPORT_DISPLAY
  if (pData->bDisplaying)
  {


    /* TODO: something !!! */


  }
#endif /* MNG_SUPPORT_DISPLAY */

#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the fields */


    /* TODO: something !!! */


  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_ORDR, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

READ_CHUNK (read_unknown)
{
#ifdef MNG_STORE_CHUNKS
  mng_retcode iRetcode;
#endif

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_UNKNOWN, MNG_LC_START);
#endif


  /* TODO: sequence checks ??? */


#ifdef MNG_STORE_CHUNKS
  if (pData->bStorechunks)
  {                                    /* initialize storage */
    iRetcode = ((mng_chunk_headerp)pHeader)->fCreate (pData, pHeader, ppChunk);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* store the length */
    ((mng_unknown_chunkp)*ppChunk)->iDatasize = iRawlen;

    if (iRawlen == 0)                  /* any data at all ? */
      ((mng_unknown_chunkp)*ppChunk)->pData = 0;
    else
    {                                  /* then store it */
      MNG_ALLOC (pData, ((mng_unknown_chunkp)*ppChunk)->pData, iRawlen)
      MNG_COPY (((mng_unknown_chunkp)*ppChunk)->pData, pRawdata, iRawlen)
    }
  }
#endif /* MNG_STORE_CHUNKS */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_READ_UNKNOWN, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_READ_PROCS */

/* ************************************************************************** */
/* *                                                                        * */
/* * chunk write functions                                                  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_WRITE_PROCS

/* ************************************************************************** */

WRITE_CHUNK (write_ihdr)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IHDR, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_plte)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PLTE, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PLTE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_idat)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IDAT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IDAT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_iend)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IEND, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IEND, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_trns)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TRNS, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TRNS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_gama)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_GAMA, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_GAMA, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_chrm)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_CHRM, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_CHRM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_srgb)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SRGB, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SRGB, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_iccp)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ICCP, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ICCP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_text)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TEXT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TEXT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_ztxt)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ZTXT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ZTXT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_itxt)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ITXT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ITXT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_bkgd)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_BKGD, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_BKGD, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_phys)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PHYS, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PHYS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_sbit)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SBIT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SBIT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_splt)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SPLT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SPLT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_hist)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_HIST, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_HIST, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_time)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TIME, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TIME, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_mhdr)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_MHDR, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_MHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_mend)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_MEND, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_MEND, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_loop)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_LOOP, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_LOOP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_endl)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ENDL, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ENDL, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_defi)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DEFI, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DEFI, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_basi)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_BASI, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_BASI, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_clon)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_CLON, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_CLON, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_past)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PAST, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PAST, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_disc)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DISC, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DISC, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_back)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_BACK, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_BACK, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_fram)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_FRAM, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_FRAM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_move)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_MOVE, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_MOVE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_clip)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_CLIP, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_CLIP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_show)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SHOW, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SHOW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_term)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TERM, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_TERM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_save)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SAVE, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SAVE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_seek)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SEEK, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_SEEK, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_expi)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_EXPI, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_EXPI, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_fpri)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_FPRI, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_FPRI, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_phyg)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PHYG, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PHYG, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_jhdr)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_JHDR, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_JHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_jdat)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_JDAT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_JDAT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_jsep)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_JSEP, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_JSEP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_dhdr)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DHDR, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_prom)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PROM, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PROM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_ipng)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IPNG, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IPNG, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_pplt)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PPLT, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_PPLT, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_ijng)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IJNG, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_IJNG, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_drop)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DROP, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DROP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_dbyk)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DBYK, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_DBYK, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_ordr)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ORDR, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_ORDR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_need)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_NEED, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_NEED, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

WRITE_CHUNK (write_unknown)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_UNKNOWN, MNG_LC_START);
#endif


  /* TODO: write chunk */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_WRITE_UNKNOWN, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_WRITE_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */





