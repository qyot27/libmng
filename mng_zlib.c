/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_zlib.c                copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : ZLIB library interface (implementation)                    * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the ZLIB library interface               * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#include "libmng.h"
#include "mng_data.h"
#include "mng_memory.h"
#include "mng_error.h"
#include "mng_trace.h"
#include "mng_pixels.h"
#include "mng_filter.h"
#include "mng_zlib.h"

#ifdef MNG_INTERNAL_MEMMNGMT
#include <stdlib.h>
#endif

/* ************************************************************************** */

#ifdef MNG_INCLUDE_ZLIB

/* ************************************************************************** */

voidpf mngzlib_alloc (voidpf pData,
                      uInt   iCount,
                      uInt   iSize)
{
  voidpf pPtr;                         /* temporary space */

#ifdef MNG_INTERNAL_MEMMNGMT
  pPtr = calloc (iCount, iSize);       /* local allocation */
#else
  if (((mng_datap)pData)->fMemalloc)   /* callback function set ? */
    pPtr = ((mng_datap)pData)->fMemalloc (iCount * iSize);
  else
    pPtr = Z_NULL;                     /* can't allocate! */
#endif

  return pPtr;                         /* return the result */
}

/* ************************************************************************** */

void mngzlib_free (voidpf pData,
                   voidpf pAddress)
{
#ifdef MNG_INTERNAL_MEMMNGMT
  free (pAddress);                     /* free locally */
#else
  if (((mng_datap)pData)->fMemfree)    /* callback set? */
    ((mng_datap)pData)->fMemfree (pAddress, 1);
#endif
}

/* ************************************************************************** */

mng_retcode mngzlib_initialize (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INITIALIZE, MNG_LC_START);
#endif

#ifdef MNG_INTERNAL_MEMMNGMT
  pData->sZlib.zalloc = Z_NULL;        /* let zlib figure out memory management */
  pData->sZlib.zfree  = Z_NULL;
  pData->sZlib.opaque = Z_NULL;
#else                                  /* use user-provided callbacks */
  pData->sZlib.zalloc = mngzlib_alloc;
  pData->sZlib.zfree  = mngzlib_free;
  pData->sZlib.opaque = (voidpf)pData;
#endif
                                       /* initialize bufferspace */
  pData->iZoutsize    = MNG_ZLIB_MAXBUF;
  MNG_ALLOC (pData, pData->pZoutbuf, pData->iZoutsize)
                                       /* default zlib compression parameters */
  pData->iZlevel      = MNG_ZLIB_LEVEL;
  pData->iZmethod     = MNG_ZLIB_METHOD;
  pData->iZwindowbits = MNG_ZLIB_WINDOWBITS;
  pData->iZmemlevel   = MNG_ZLIB_MEMLEVEL;
  pData->iZstrategy   = MNG_ZLIB_STRATEGY;

  pData->bInflating   = MNG_FALSE;     /* not performing any action yet */
  pData->bDeflating   = MNG_FALSE;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INITIALIZE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

mng_retcode mngzlib_cleanup (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_CLEANUP, MNG_LC_START);
#endif

  if (pData->bInflating)               /* force zlib cleanup */
    mngzlib_inflatefree (pData);
  if (pData->bDeflating)
    mngzlib_deflatefree (pData);

  if (pData->pZoutbuf)                 /* free the zlib buffer */
  {
    MNG_FREE (pData, pData->pZoutbuf, pData->iZoutsize)
    pData->pZoutbuf = 0;               /* and make sure we do it only once! */
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_CLEANUP, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

mng_retcode mngzlib_inflateinit (mng_datap pData)
{
  uInt iZrslt;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEINIT, MNG_LC_START);
#endif
                                       /* initialize zlib structures and such */
  iZrslt = inflateInit (&pData->sZlib);

  if (iZrslt != Z_OK)                  /* on error bail out */
    MNG_ERRORZ (pData, (mng_uint32)iZrslt)

  pData->bInflating      = MNG_TRUE;   /* really inflating something now */
  pData->sZlib.next_out  = 0;          /* force JIT initialization */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEINIT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode mngzlib_inflaterows (mng_datap  pData,
                                 mng_uint32 iInlen,
                                 mng_uint8p pIndata)
{
  uInt        iZrslt;
  mng_retcode iRslt;
  mng_ptr     pSwap;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEROWS, MNG_LC_START);
#endif

  pData->sZlib.next_in   = pIndata;    /* let zlib know where to get stuff */
  pData->sZlib.avail_in  = (uInt)iInlen;

  if (pData->sZlib.next_out == 0)      /* initialize output variables ? */
  {                                    /* let zlib know where to store stuff */
    pData->sZlib.next_out  = pData->pWorkrow;
    pData->sZlib.avail_out = (uInt)(pData->iRowsize + 1);
  }

  do
  {                                    /* now inflate a row */
    iZrslt = inflate (&pData->sZlib, Z_SYNC_FLUSH);
                                       /* produced a full row ? */
    if (((iZrslt == Z_OK) || (iZrslt == Z_STREAM_END)) &&
        (pData->sZlib.avail_out == 0))
    {                                   /* shouldn't we be at the end ? */
      if (pData->iRow >= (mng_int32)pData->iDataheight)
/*        MNG_ERROR (pData, MNG_TOOMUCHIDAT) */ ;
      else
      {                                 /* filter the row if necessary */
        if (pData->pWorkrow[0])
          iRslt = filter_a_row (pData);
        else
          iRslt = MNG_NOERROR;

        if (!iRslt)                    /* now process this row */
          iRslt = ((mng_processrow)pData->fProcessrow) (pData);
                                       /* store in object ? */
        if ((!iRslt) && (pData->fStorerow))
          iRslt = ((mng_correctrow)pData->fStorerow) (pData);
                                       /* color correction ? */
        if ((!iRslt) && (pData->fCorrectrow))
          iRslt = ((mng_correctrow)pData->fCorrectrow) (pData);
                                       /* slap onto canvas ? */
        if ((!iRslt) && (pData->fDisplayrow))
          iRslt = ((mng_displayrow)pData->fDisplayrow) (pData);

        if (iRslt)                     /* on error bail out */
          MNG_ERROR (pData, iRslt);
                                       /* swap row-pointers */
        pSwap           = pData->pWorkrow;
        pData->pWorkrow = pData->pPrevrow;
        pData->pPrevrow = pSwap;       /* so prev points to the processed row! */

        next_row (pData);              /* adjust variables for next row */
      }
                                       /* let zlib know where to store next output */
      pData->sZlib.next_out  = pData->pWorkrow;
      pData->sZlib.avail_out = (uInt)(pData->iRowsize + 1);
    }
  }                                    /* until some error or EOI */
  while ((iZrslt == Z_OK) && (pData->sZlib.avail_in > 0));
                                       /* on error bail out */
  if ((iZrslt != Z_OK) && (iZrslt != Z_STREAM_END))
    MNG_ERRORZ (pData, (mng_uint32)iZrslt)

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEROWS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

mng_retcode mngzlib_inflatedata (mng_datap  pData,
                                 mng_uint32 iInlen,
                                 mng_uint8p pIndata)
{
  uInt iZrslt;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEDATA, MNG_LC_START);
#endif
                                       /* let zlib know where to get stuff */
  pData->sZlib.next_in   = pIndata;
  pData->sZlib.avail_in  = (uInt)iInlen;
                                       /* now inflate the data in go! */
  iZrslt = inflate (&pData->sZlib, Z_FINISH);
                                       /* not enough room in output-buffer ? */
  if ((iZrslt == Z_BUF_ERROR) || (pData->sZlib.avail_in > 0))
    return MNG_BUFOVERFLOW; 
                                       /* on error bail out */
  if ((iZrslt != Z_OK) && (iZrslt != Z_STREAM_END))
    MNG_ERRORZ (pData, (mng_uint32)iZrslt)

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEDATA, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode mngzlib_inflatefree (mng_datap pData)
{
  uInt iZrslt;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEFREE, MNG_LC_START);
#endif

  pData->bInflating = MNG_FALSE;       /* stopped it */

  iZrslt = inflateEnd (&pData->sZlib); /* let zlib cleanup it's own stuff */

  if (iZrslt != Z_OK)                  /* on error bail out */
    MNG_ERRORZ (pData, (mng_uint32)iZrslt)

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_INFLATEFREE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

mng_retcode mngzlib_deflateinit (mng_datap pData)
{
  uInt iZrslt;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEINIT, MNG_LC_START);
#endif
                                       /* initialize zlib structures and such */
  iZrslt = deflateInit2 (&pData->sZlib, pData->iZlevel, pData->iZmethod,
                         pData->iZwindowbits, pData->iZmemlevel,
                         pData->iZstrategy);

  if (iZrslt != Z_OK)                  /* on error bail out */
    MNG_ERRORZ (pData, (mng_uint32)iZrslt)

  pData->bDeflating      = MNG_TRUE;   /* really deflating something now */
                                       /* let zlib know where to find the buffer */
  pData->sZlib.next_out  = pData->pZoutbuf;
  pData->sZlib.avail_out = (uInt)pData->iZoutsize;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEINIT, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

mng_retcode mngzlib_deflaterows (mng_datap  pData,
                                 mng_uint32 iInlen,
                                 mng_uint8p pIndata)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEROWS, MNG_LC_START);
#endif




#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEROWS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode mngzlib_deflatedata (mng_datap  pData,
                                 mng_uint32 iInlen,
                                 mng_uint8p pIndata)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEDATA, MNG_LC_START);
#endif




#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEDATA, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode mngzlib_deflatefree (mng_datap pData)
{
  uInt iZrslt;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEFREE, MNG_LC_START);
#endif

  iZrslt = deflateEnd (&pData->sZlib); /* let zlib cleanup it's own stuff */

  if (iZrslt != Z_OK)                  /* on error bail out */
    MNG_ERRORZ (pData, (mng_uint32)iZrslt)

  pData->bDeflating = MNG_FALSE;       /* stopped it */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_ZLIB_DEFLATEFREE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* done */
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_ZLIB */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

