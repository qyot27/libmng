/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_pixels.c              copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.2                                                      * */
/* *                                                                        * */
/* * purpose   : Pixel-row management routines (implementation)             * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the pixel-row management routines        * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/11/2000 - G.Juyn                                * */
/* *             - added callback error-reporting support                   * */
/* *             0.5.1 - 05/12/2000 - G.Juyn                                * */
/* *             - changed trace to macro for callback error-reporting      * */
/* *                                                                        * */
/* *             0.5.2 - 05/22/2000 - G.Juyn                                * */
/* *             - added JNG support                                        * */
/* *                                                                        * */
/* ************************************************************************** */

#include "libmng.h"
#include "mng_data.h"
#include "mng_error.h"
#include "mng_trace.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "mng_objects.h"
#include "mng_memory.h"
#include "mng_cms.h"
#include "mng_pixels.h"

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

/* ************************************************************************** */

#ifdef MNG_INCLUDE_DISPLAY_PROCS

/* TODO: magnification & canvas-positioning/-clipping */

/* TODO: major optimization of pixel-loops by using assembler (?) */

/* ************************************************************************** */
/* *                                                                        * */
/* * Interlace tables                                                       * */
/* *                                                                        * */
/* ************************************************************************** */

mng_uint32 const interlace_row      [7] = { 0, 0, 4, 0, 2, 0, 1 };
mng_uint32 const interlace_rowskip  [7] = { 8, 8, 8, 4, 4, 2, 2 };
mng_uint32 const interlace_col      [7] = { 0, 4, 0, 2, 0, 1, 0 };
mng_uint32 const interlace_colskip  [7] = { 8, 8, 4, 4, 2, 2, 1 };
mng_uint32 const interlace_roundoff [7] = { 7, 7, 3, 3, 1, 1, 0 };
mng_uint32 const interlace_divider  [7] = { 3, 3, 2, 2, 1, 1, 0 };

/* ************************************************************************** */
/* *                                                                        * */
/* * Alpha composing macros                                                 * */
/* * the code below is slightly modified from the libpng package            * */
/* * the original was last optimized by Greg Roelofs & Mark Adler           * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_COMPOSE8(RET,FG,ALPHA,BG) {                                    \
       mng_uint16 iH = (mng_uint16)((mng_uint16)(FG) * (mng_uint16)(ALPHA) \
                        + (mng_uint16)(BG)*(mng_uint16)(255 -              \
                          (mng_uint16)(ALPHA)) + (mng_uint16)128);         \
       (RET) = (mng_uint8)((iH + (iH >> 8)) >> 8); }

#define MNG_COMPOSE16(RET,FG,ALPHA,BG) {                                   \
       mng_uint32 iH = (mng_uint32)((mng_uint32)(FG) * (mng_uint32)(ALPHA) \
                        + (mng_uint32)(BG)*(mng_uint32)(65535L -           \
                          (mng_uint32)(ALPHA)) + (mng_uint32)32768L);      \
       (RET) = (mng_uint16)((iH + (iH >> 16)) >> 16); }

/* ************************************************************************** */
/* *                                                                        * */
/* * Display routines - convert rowdata (which is already color-corrected)  * */
/* * to the output canvas, respecting the opacity information               * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode display_rgb8 (mng_datap pData)
{
  mng_uint8p pScanline;
  mng_uint8p pDataline;
  mng_int32  iX;
  mng_uint16 iA16;
  mng_uint16 iFGr16, iFGg16, iFGb16;
  mng_uint16 iBGr16, iBGg16, iBGb16;
  mng_uint8  iA8;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_RGB8, MNG_LC_START);
#endif
                                       /* viewable row ? */
  if ((pData->iRow >= pData->iSourcet) && (pData->iRow < pData->iSourceb))
  {                                    /* address destination row */
    pScanline = (mng_uint8p)pData->fGetcanvasline (((mng_handle)pData),
                                                   pData->iRow + pData->iDestt -
                                                   pData->iSourcet);
                                       /* adjust destination row starting-point */
    pScanline = pScanline + (pData->iCol * 3) + (pData->iDestl * 3);
    pDataline = pData->pRGBArow;       /* address source row */

    if (pData->bIsRGBA16)              /* adjust source row starting-point */
      pDataline = pDataline + (pData->iSourcel / pData->iColinc) * 8;
    else
      pDataline = pDataline + (pData->iSourcel / pData->iColinc) * 4;

    if (pData->bIsOpaque)              /* forget about transparency ? */
    {
      if (pData->bIsRGBA16)            /* 16-bit input row ? */
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {                              /* scale down by dropping the LSB */
          *pScanline     = *pDataline;
          *(pScanline+1) = *(pDataline+2);
          *(pScanline+2) = *(pDataline+4);

          pScanline += (pData->iColinc * 3);
          pDataline += 8;
        }
      }
      else
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {                              /* copy the values */
          *pScanline     = *pDataline;
          *(pScanline+1) = *(pDataline+1);
          *(pScanline+2) = *(pDataline+2);

          pScanline += (pData->iColinc * 3);
          pDataline += 4;
        }
      }
    }
    else
    {
      if (pData->bIsRGBA16)            /* 16-bit input row ? */
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {
#ifdef MNG_SWAP_ENDIAN                 /* get alpha value */
          iA16 = mng_get_uint16 (pDataline+6);
#else
          iA16 = *(mng_uint16p)(pDataline+6);
#endif
          if (iA16)                    /* any opacity at all ? */
          {
            if (iA16 == 0xFFFF)        /* fully opaque ? */
            {                          /* scale down by dropping the LSB */
              *pScanline     = *pDataline;
              *(pScanline+1) = *(pDataline+2);
              *(pScanline+2) = *(pDataline+4);
            }
            else
            {
#ifdef MNG_SWAP_ENDIAN                 /* get the proper values */
              iFGr16 = mng_get_uint16 (pDataline  );
              iFGg16 = mng_get_uint16 (pDataline+2);
              iFGb16 = mng_get_uint16 (pDataline+4);
#else
              iFGr16 = *(mng_uint16p)(pDataline  );
              iFGg16 = *(mng_uint16p)(pDataline+2);
              iFGb16 = *(mng_uint16p)(pDataline+4);
#endif                                 /* scale background up */
              iBGr16 = (mng_uint16)(*(pScanline+2));
              iBGg16 = (mng_uint16)(*(pScanline+1));
              iBGb16 = (mng_uint16)(*pScanline    );
              iBGr16 = (mng_uint16)((mng_uint32)iBGr16 << 8) | iBGr16;
              iBGg16 = (mng_uint16)((mng_uint32)iBGg16 << 8) | iBGg16;
              iBGb16 = (mng_uint16)((mng_uint32)iBGb16 << 8) | iBGb16;
                                       /* now compose */
              MNG_COMPOSE16(iFGr16, iFGr16, iA16, iBGr16)
              MNG_COMPOSE16(iFGg16, iFGg16, iA16, iBGg16)
              MNG_COMPOSE16(iFGb16, iFGb16, iA16, iBGb16)
                                       /* and return the composed values */
              *pScanline     = (mng_uint8)(iFGr16 >> 8);
              *(pScanline+1) = (mng_uint8)(iFGg16 >> 8);
              *(pScanline+2) = (mng_uint8)(iFGb16 >> 8);
            }
          }

          pScanline += (pData->iColinc * 3);
          pDataline += 8;
        }
      }
      else
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {
          iA8 = *(pDataline+3);        /* get alpha value */

          if (iA8)                     /* any opacity at all ? */
          {
            if (iA8 == 0xFF)           /* fully opaque ? */
            {                          /* then simply copy the values */
              *pScanline     = *pDataline;
              *(pScanline+1) = *(pDataline+1);
              *(pScanline+2) = *(pDataline+2);
            }
            else
            {                          /* do alpha composing */
              MNG_COMPOSE8 (*pScanline,     *pDataline,     iA8, *pScanline    )
              MNG_COMPOSE8 (*(pScanline+1), *(pDataline+1), iA8, *(pScanline+1))
              MNG_COMPOSE8 (*(pScanline+2), *(pDataline+2), iA8, *(pScanline+2))
            }
          }

          pScanline += (pData->iColinc * 3);
          pDataline += 4;
        }
      }
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_RGB8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode display_rgba8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_RGBA8, MNG_LC_START);
#endif


  /* TODO: YEP */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_RGBA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode display_argb8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_ARGB8, MNG_LC_START);
#endif


  /* TODO: YEP */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_ARGB8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode display_bgr8 (mng_datap pData)
{
  mng_uint8p pScanline;
  mng_uint8p pDataline;
  mng_int32  iX;
  mng_uint16 iA16;
  mng_uint16 iFGr16, iFGg16, iFGb16;
  mng_uint16 iBGr16, iBGg16, iBGb16;
  mng_uint8  iA8;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_BGR8, MNG_LC_START);
#endif
                                       /* viewable row ? */
  if ((pData->iRow >= pData->iSourcet) && (pData->iRow < pData->iSourceb))
  {                                    /* address destination row */
    pScanline = (mng_uint8p)pData->fGetcanvasline (((mng_handle)pData),
                                                   pData->iRow + pData->iDestt -
                                                   pData->iSourcet);
                                       /* adjust destination row starting-point */
    pScanline = pScanline + (pData->iCol * 3) + (pData->iDestl * 3);
    pDataline = pData->pRGBArow;       /* address source row */

    if (pData->bIsRGBA16)              /* adjust source row starting-point */
      pDataline = pDataline + (pData->iSourcel / pData->iColinc) * 8;
    else
      pDataline = pDataline + (pData->iSourcel / pData->iColinc) * 4;

    if (pData->bIsOpaque)              /* forget about transparency ? */
    {
      if (pData->bIsRGBA16)            /* 16-bit input row ? */
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {                              /* scale down by dropping the LSB */
          *pScanline     = *(pDataline+4);
          *(pScanline+1) = *(pDataline+2);
          *(pScanline+2) = *pDataline;
/*          *pScanline     = *(pDataline+5);
          *(pScanline+1) = *(pDataline+3);
          *(pScanline+2) = *(pDataline+1); */

          pScanline += (pData->iColinc * 3);
          pDataline += 8;
        }
      }
      else
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {                              /* copy the values */
          *pScanline     = *(pDataline+2);
          *(pScanline+1) = *(pDataline+1);
          *(pScanline+2) = *pDataline;

          pScanline += (pData->iColinc * 3);
          pDataline += 4;
        }
      }
    }
    else
    {
      if (pData->bIsRGBA16)            /* 16-bit input row ? */
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {
#ifdef MNG_SWAP_ENDIAN                 /* get alpha value */
          iA16 = mng_get_uint16 (pDataline+6);
#else
          iA16 = *(mng_uint16p)(pDataline+6);
#endif
          if (iA16)                    /* any opacity at all ? */
          {
            if (iA16 == 0xFFFF)        /* fully opaque ? */
            {                          /* scale down by dropping the LSB */
              *pScanline     = *(pDataline+4);
              *(pScanline+1) = *(pDataline+2);
              *(pScanline+2) = *pDataline;
            }
            else
            {
#ifdef MNG_SWAP_ENDIAN                 /* get the proper values */
              iFGr16 = mng_get_uint16 (pDataline  );
              iFGg16 = mng_get_uint16 (pDataline+2);
              iFGb16 = mng_get_uint16 (pDataline+4);
#else
              iFGr16 = *(mng_uint16p)(pDataline  );
              iFGg16 = *(mng_uint16p)(pDataline+2);
              iFGb16 = *(mng_uint16p)(pDataline+4);
#endif                                 /* scale background up */
              iBGr16 = (mng_uint16)(*(pScanline+2));
              iBGg16 = (mng_uint16)(*(pScanline+1));
              iBGb16 = (mng_uint16)(*pScanline    );
              iBGr16 = (mng_uint16)((mng_uint32)iBGr16 << 8) | iBGr16;
              iBGg16 = (mng_uint16)((mng_uint32)iBGg16 << 8) | iBGg16;
              iBGb16 = (mng_uint16)((mng_uint32)iBGb16 << 8) | iBGb16;
                                       /* now compose */
              MNG_COMPOSE16(iFGr16, iFGr16, iA16, iBGr16)
              MNG_COMPOSE16(iFGg16, iFGg16, iA16, iBGg16)
              MNG_COMPOSE16(iFGb16, iFGb16, iA16, iBGb16)
                                       /* and return the composed values */
              *pScanline     = (mng_uint8)(iFGb16 >> 8);
              *(pScanline+1) = (mng_uint8)(iFGg16 >> 8);
              *(pScanline+2) = (mng_uint8)(iFGr16 >> 8);
            }
          }

          pScanline += (pData->iColinc * 3);
          pDataline += 8;
        }
      }
      else
      {
        for (iX = pData->iSourcel; iX < pData->iSourcer; iX += pData->iColinc)
        {
          iA8 = *(pDataline+3);        /* get alpha value */

          if (iA8)                     /* any opacity at all ? */
          {
            if (iA8 == 0xFF)           /* fully opaque ? */
            {                          /* then simply copy the values */
              *pScanline     = *(pDataline+2);
              *(pScanline+1) = *(pDataline+1);
              *(pScanline+2) = *pDataline;
            }
            else
            {                          /* do alpha composing */
              MNG_COMPOSE8 (*pScanline,     *(pDataline+2), iA8, *pScanline    )
              MNG_COMPOSE8 (*(pScanline+1), *(pDataline+1), iA8, *(pScanline+1))
              MNG_COMPOSE8 (*(pScanline+2), *pDataline,     iA8, *(pScanline+2))
            }
          }

          pScanline += (pData->iColinc * 3);
          pDataline += 4;
        }
      }
    }
  }

  /* TODO: a smoother approximation for progressive intervals;
     nb. certainly take stream-input-time into consideration */

                                       /* progressive display ? */
  if (((pData->eImagetype != mng_it_mng) || (pData->iDataheight > 300)) &&
      (pData->iDestb - pData->iDestt > 100))
  {
    mng_int32 iC = pData->iRow + pData->iDestt - pData->iSourcet;

    if (iC % 100 == 0)                 /* every 100th line (???) */
      if (!pData->fRefresh ((mng_handle)pData, 0, 0, pData->iWidth, pData->iHeight))
        MNG_ERROR (pData, MNG_APPMISCERROR)
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_BGR8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode display_bgra8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_BGRA8, MNG_LC_START);
#endif


  /* TODO: YEP */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_BGRA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode display_abgr8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_ABGR8, MNG_LC_START);
#endif


  /* TODO: YEP */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_ABGR8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Background restore routines - restore the background with info from    * */
/* * the BACK and/or bKGD chunk or the app's background canvas              * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode restore_bkgd_backimage (mng_datap pData)
{
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BACKIMAGE, MNG_LC_START);
#endif
                                       /* make it easy on yourself */
  iRetcode = restore_bkgd_backcolor (pData);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

    
  /* TODO: loading the background-image */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BACKIMAGE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode restore_bkgd_backcolor (mng_datap pData)
{
  mng_int32  iX;
  mng_uint8p pWork = pData->pRGBArow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BACKCOLOR, MNG_LC_START);
#endif

  for (iX = pData->iSourcel; iX < pData->iSourcer; iX++)
  {                                    /* ok; drop the background-color in there */
    *pWork     = (mng_uint8)(pData->iBACKred   >> 8);
    *(pWork+1) = (mng_uint8)(pData->iBACKgreen >> 8);
    *(pWork+2) = (mng_uint8)(pData->iBACKblue  >> 8);
    *(pWork+3) = 0xff;                 /* opaque my man */

    pWork += 4;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BACKCOLOR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode restore_bkgd_bgcolor (mng_datap pData)
{
  mng_int32  iX;
  mng_uint8p pWork = pData->pRGBArow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BGCOLOR, MNG_LC_START);
#endif

  for (iX = pData->iSourcel; iX < pData->iSourcer; iX++)
  {                                    /* ok; drop the background-color in there */
    *pWork     = (mng_uint8)(pData->iBGred   >> 8);
    *(pWork+1) = (mng_uint8)(pData->iBGgreen >> 8);
    *(pWork+2) = (mng_uint8)(pData->iBGblue  >> 8);
    *(pWork+3) = 0xff;                 /* opaque my man */

    pWork += 4;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BGCOLOR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode restore_bkgd_rgb8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_RGB8, MNG_LC_START);
#endif


  /* TODO: something */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_RGB8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode restore_bkgd_bgr8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BGR8, MNG_LC_START);
#endif


  /* TODO: something */


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESTORE_BGR8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Row retrieval routines - retrieve processed & uncompressed row-data    * */
/* * from the current "object"                                              * */
/* *                                                                        * */
/* ************************************************************************** */

/* TODO: a serious optimization is to retrieve only those pixels that will
         actually be displayed; this would require changes in
         the "display_image" routine (in mng_display.c) &
         all the "retrieve_xxx" routines below &
         the "display_xxx" routines above !!!!!
         NOTE that "correct_xxx" routines would not require modification */

mng_retcode retrieve_g8 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iG;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_G8, MNG_LC_START);
#endif

  pRGBArow = pData->pRGBArow;          /* temporary work pointers */
  pWorkrow = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  if (pBuf->bHasTRNS)                  /* tRNS in buffer ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iG = *pWorkrow;                  /* get the gray-value */
                                       /* is it transparent ? */
      if ((mng_uint16)iG == pBuf->iTRNSgray)
      {
        *pRGBArow     = 0x00;          /* nuttin to display */
        *(pRGBArow+1) = 0x00;
        *(pRGBArow+2) = 0x00;
        *(pRGBArow+3) = 0x00;
      }
      else
      {
        *pRGBArow     = iG;            /* put in intermediate row */
        *(pRGBArow+1) = iG;
        *(pRGBArow+2) = iG;
        *(pRGBArow+3) = 0xFF;
      }

      pWorkrow++;                      /* next pixel */
      pRGBArow += 4;
    }
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iG = *pWorkrow;                  /* get the gray-value */
      *pRGBArow     = iG;              /* put in intermediate row */
      *(pRGBArow+1) = iG;
      *(pRGBArow+2) = iG;
      *(pRGBArow+3) = 0xFF;

      pWorkrow++;                      /* next pixel */
      pRGBArow += 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_G8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_g16 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pRGBArow;
  mng_int32      iX;
  mng_uint16     iG;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_G16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pRGBArow = (mng_uint16p)pData->pRGBArow;
  pWorkrow = (mng_uint16p)pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  if (pBuf->bHasTRNS)                  /* tRNS in buffer ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iG = *pWorkrow;                  /* get the gray-value */
                                       /* is it transparent ? */
      if (iG == pBuf->iTRNSgray)
      {
        *pRGBArow     = 0x0000;        /* nuttin to display */
        *(pRGBArow+1) = 0x0000;
        *(pRGBArow+2) = 0x0000;
        *(pRGBArow+3) = 0x0000;
      }
      else
      {
        *pRGBArow     = iG;            /* put in intermediate row */
        *(pRGBArow+1) = iG;
        *(pRGBArow+2) = iG;
        *(pRGBArow+3) = 0xFFFF;
      }

      pWorkrow++;                      /* next pixel */
      pRGBArow += 4;
    }
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iG = *pWorkrow;                  /* get the gray-value */
      *pRGBArow     = iG;              /* put in intermediate row */
      *(pRGBArow+1) = iG;
      *(pRGBArow+2) = iG;
      *(pRGBArow+3) = 0xFFFF;

      pWorkrow++;                      /* next pixel */
      pRGBArow += 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_G16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_rgb8 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iR, iG, iB;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGB8, MNG_LC_START);
#endif

  pRGBArow = pData->pRGBArow;          /* temporary work pointers */
  pWorkrow = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  if (pBuf->bHasTRNS)                  /* tRNS in buffer ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iR = *pWorkrow;                  /* get the rgb-values */
      iG = *(pWorkrow+1);
      iB = *(pWorkrow+2);
                                       /* is it transparent ? */
      if (((mng_uint16)iR == pBuf->iTRNSred  ) &&
          ((mng_uint16)iG == pBuf->iTRNSgreen) &&
          ((mng_uint16)iB == pBuf->iTRNSblue )    )
      {
        *pRGBArow     = 0x00;          /* nothing to display */
        *(pRGBArow+1) = 0x00;
        *(pRGBArow+2) = 0x00;
        *(pRGBArow+3) = 0x00;
      }
      else
      {
        *pRGBArow     = iR;            /* put in intermediate row */
        *(pRGBArow+1) = iG;
        *(pRGBArow+2) = iB;
        *(pRGBArow+3) = 0xFF;
      }

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      *pRGBArow     = *pWorkrow;       /* just copy the pixel */
      *(pRGBArow+1) = *(pWorkrow+1);
      *(pRGBArow+2) = *(pWorkrow+2);
      *(pRGBArow+3) = 0xFF;

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGB8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_rgb16 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pRGBArow;
  mng_int32      iX;
  mng_uint16     iR, iG, iB;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGB16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pRGBArow = (mng_uint16p)pData->pRGBArow;
  pWorkrow = (mng_uint16p)pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  if (pBuf->bHasTRNS)                  /* tRNS in buffer ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iR = *pWorkrow;                  /* get the rgb-values */
      iG = *(pWorkrow+1);
      iB = *(pWorkrow+2);
                                       /* is it transparent ? */
      if ((iR == pBuf->iTRNSred  ) &&
          (iG == pBuf->iTRNSgreen) &&
          (iB == pBuf->iTRNSblue )    )
      {
        *pRGBArow     = 0x0000;        /* nothing to display */
        *(pRGBArow+1) = 0x0000;
        *(pRGBArow+2) = 0x0000;
        *(pRGBArow+3) = 0x0000;
      }
      else
      {
        *pRGBArow     = iR;            /* put in intermediate row */
        *(pRGBArow+1) = iG;
        *(pRGBArow+2) = iB;
        *(pRGBArow+3) = 0xFFFF;
      }

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      *pRGBArow     = *pWorkrow;       /* just copy the pixel */
      *(pRGBArow+1) = *(pWorkrow+1);
      *(pRGBArow+2) = *(pWorkrow+2);
      *(pRGBArow+3) = 0xFF;

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGB16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_idx8 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iQ;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_IDX8, MNG_LC_START);
#endif

  pRGBArow = pData->pRGBArow;          /* temporary work pointers */
  pWorkrow = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  if (pBuf->bHasTRNS)                  /* tRNS in buffer ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iQ = *pWorkrow;                  /* get the index */
                                       /* is it valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        *pRGBArow     = pBuf->aPLTEentries [iQ].iRed;
        *(pRGBArow+1) = pBuf->aPLTEentries [iQ].iGreen;
        *(pRGBArow+2) = pBuf->aPLTEentries [iQ].iBlue;
                                       /* transparency for this index ? */
        if ((mng_uint32)iQ < pBuf->iTRNScount)
          *(pRGBArow+3) = pBuf->aTRNSentries [iQ];
        else
          *(pRGBArow+3) = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pWorkrow++;                      /* next pixel */
      pRGBArow += 4;
    }
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iQ = *pWorkrow;                  /* get the index */
                                       /* is it valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        *pRGBArow     = pBuf->aPLTEentries [iQ].iRed;
        *(pRGBArow+1) = pBuf->aPLTEentries [iQ].iGreen;
        *(pRGBArow+2) = pBuf->aPLTEentries [iQ].iBlue;
        *(pRGBArow+3) = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pWorkrow++;                      /* next pixel */
      pRGBArow += 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_IDX8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_ga8 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iG;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_GA8, MNG_LC_START);
#endif

  pRGBArow = pData->pRGBArow;          /* temporary work pointers */
  pWorkrow = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    iG = *pWorkrow;                    /* get the gray-value */
    *pRGBArow     = iG;                /* put in intermediate row */
    *(pRGBArow+1) = iG;
    *(pRGBArow+2) = iG;
    *(pRGBArow+3) = *(pWorkrow+1);

    pWorkrow += 2;                     /* next pixel */
    pRGBArow += 4;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_GA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_ga16 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pRGBArow;
  mng_int32      iX;
  mng_uint16     iG;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_GA16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pRGBArow = (mng_uint16p)pData->pRGBArow;
  pWorkrow = (mng_uint16p)pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    iG = *pWorkrow;                    /* get the gray-value */
    *pRGBArow     = iG;                /* put in intermediate row */
    *(pRGBArow+1) = iG;
    *(pRGBArow+2) = iG;
    *(pRGBArow+3) = *(pWorkrow+1);

    pWorkrow += 2;                     /* next pixel */
    pRGBArow += 4;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_GA16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_rgba8 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGBA8, MNG_LC_START);
#endif

  pRGBArow = pData->pRGBArow;          /* temporary work pointers */
  pWorkrow = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);
                                       /* can't be easier than this ! */
  MNG_COPY (pRGBArow, pWorkrow, pBuf->iRowsize)

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGBA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode retrieve_rgba16 (mng_datap pData)
{
  mng_imagedatap pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pRGBArow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGBA16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pRGBArow = (mng_uint16p)pData->pRGBArow;
  pWorkrow = (mng_uint16p)pBuf->pImgdata + (pData->iRow * pBuf->iRowsize);
                                       /* can't be easier than this ! */
  MNG_COPY (pRGBArow, pWorkrow, pBuf->iRowsize)

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RETRIEVE_RGBA16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Row storage routines - store processed & uncompressed row-data         * */
/* * into the current "object"                                              * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode store_g1 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G1, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;

  while (iX < pData->iRowsamples)
  {
    if (iB & iM)                       /* is it white ? */
      *pOutrow = 0xFF;                 /* white */
    else
      *pOutrow = 0x00;                 /* black */

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;
    iM >>= 1;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0x80;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G1, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_g2 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G2, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  while (iX < pData->iRowsamples)
  {
    switch ((iB & iM) >> iS)           /* determine the gray level */
    {
      case 0x03 : { *pOutrow = 0xFF; break; }
      case 0x02 : { *pOutrow = 0xAA; break; }
      case 0x01 : { *pOutrow = 0x55; break; }
      default   : { *pOutrow = 0x00; }
    }

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;
    iM >>= 2;
    iS -= 2;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xC0;
      iS = 6;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G2, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_g4 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G4, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  while (iX < pData->iRowsamples)
  {                                    /* get the gray level */
    iQ = (mng_uint8)((iB & iM) >> iS);
    iQ = (mng_uint8)(iQ + (iQ << 4));  /* expand to 8-bit by replication */

    *pOutrow = iQ;                     /* put in object buffer */

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;
    iM >>= 4;
    iS -= 4;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xF0;
      iS = 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G4, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_g8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = iB;                     /* put in object buffer */

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;

    iB = *pWorkrow;                    /* get next input-byte */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_g16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;
  mng_uint16     iW;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G16, MNG_LC_START);
#endif

  pWorkrow = (mng_uint16p)pData->pWorkrow + 1;
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize)   );
  iX       = 0;                        /* start at pixel 0 */
  iW       = *pWorkrow;                /* and get first input 2-byte */
  pWorkrow++;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = iW;                     /* put in object buffer */

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;

    iW = *pWorkrow;                    /* get next input 2-byte */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_G16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_rgb8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGB8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy the RGB bytes */
    *(pOutrow+1) = *(pWorkrow+1);
    *(pOutrow+2) = *(pWorkrow+2);

    pWorkrow += 3;                     /* next pixel */
    pOutrow  += (pData->iColinc * 3);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGB8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_rgb16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGB16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pWorkrow = (mng_uint16p)pData->pWorkrow + 1;
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize)   );

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy the RGB bytes */
    *(pOutrow+1) = *(pWorkrow+1);
    *(pOutrow+2) = *(pWorkrow+2);

    pWorkrow += 3;                     /* next pixel */
    pOutrow  += (pData->iColinc * 3);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGB16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_idx1 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX1, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;

  while (iX < pData->iRowsamples)
  {
    if (iB & iM)                       /* store the index */
      *pOutrow = 0x01;
    else
      *pOutrow = 0x00;

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;
    iM >>= 1;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0x80;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX1, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_idx2 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX2, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  while (iX < pData->iRowsamples)
  {                                    /* store the index */
    *pOutrow = (mng_uint8)((iB & iM) >> iS);

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;
    iM >>= 2;
    iS -= 2;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xC0;
      iS = 6;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX2, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_idx4 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX4, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  while (iX < pData->iRowsamples)
  {                                    /* store the index */
    *pOutrow = (mng_uint8)((iB & iM) >> iS);

    pOutrow += pData->iColinc;         /* next pixel */
    iX++;
    iM >>= 4;
    iS -= 4;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xF0;
      iS = 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX4, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_idx8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow = *pWorkrow;              /* put in object buffer */

    pOutrow += pData->iColinc;         /* next pixel */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_IDX8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_ga8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_GA8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy the GA bytes */
    *(pOutrow+1) = *(pWorkrow+1);

    pWorkrow += 2;                     /* next pixel */
    pOutrow  += (pData->iColinc << 1);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_GA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_ga16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_GA16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pWorkrow = (mng_uint16p)pData->pWorkrow + 1;
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize)   );

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy the GA bytes */
    *(pOutrow+1) = *(pWorkrow+1);

    pWorkrow += 2;                     /* next pixel */
    pOutrow  += (pData->iColinc << 1);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_GA16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_rgba8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGBA8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy the RGBA bytes */
    *(pOutrow+1) = *(pWorkrow+1);
    *(pOutrow+2) = *(pWorkrow+2);
    *(pOutrow+3) = *(pWorkrow+3);

    pWorkrow += 4;                     /* next pixel */
    pOutrow  += (pData->iColinc << 2);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGBA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode store_rgba16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGBA16, MNG_LC_START);
#endif
                                       /* temporary work pointers */
  pWorkrow = (mng_uint16p)pData->pWorkrow + 1;
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize)   );

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy the RGBA bytes */
    *(pOutrow+1) = *(pWorkrow+1);
    *(pOutrow+2) = *(pWorkrow+2);
    *(pOutrow+3) = *(pWorkrow+3);

    pWorkrow += 4;                     /* next pixel */
    pOutrow  += (pData->iColinc << 2);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_RGBA16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Row storage routines (JPEG) - store processed & uncompressed row-data  * */
/* * into the current "object"                                              * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG

/* ************************************************************************** */

mng_retcode store_jpeg_g8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8, MNG_LC_START);
#endif

  pWorkrow = pData->pJPEGrow;          /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iJPEGrow * pBuf->iRowsize);
                                       /* easy as pie ... */
  MNG_COPY (pOutrow, pWorkrow, pData->iRowsamples)

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8, MNG_LC_END);
#endif

  return next_jpeg_row (pData);        /* we've got one more row of gray-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgb8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8, MNG_LC_START);
#endif

  pWorkrow = pData->pJPEGrow;          /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iJPEGrow * pBuf->iRowsize);
                                       /* easy as pie ... */
  MNG_COPY (pOutrow, pWorkrow, pData->iRowsamples * 3)

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8, MNG_LC_END);
#endif

  return next_jpeg_row (pData);        /* we've got one more row of rgb-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_ga8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_GA8, MNG_LC_START);
#endif

  pWorkrow = pData->pJPEGrow;          /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iJPEGrow * pBuf->iRowsize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow = *pWorkrow;              /* copy into object buffer */

    pOutrow += 2;                      /* next pixel */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_GA8, MNG_LC_END);
#endif

  return next_jpeg_row (pData);        /* we've got one more row of gray-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgba8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGBA8, MNG_LC_START);
#endif

  pWorkrow = pData->pJPEGrow;          /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iJPEGrow * pBuf->iRowsize);

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pOutrow     = *pWorkrow;          /* copy pixel into object buffer */
    *(pOutrow+1) = *(pWorkrow+1);
    *(pOutrow+2) = *(pWorkrow+2);

    pOutrow  += 4;                     /* next pixel */
    pWorkrow += 3;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGBA8, MNG_LC_END);
#endif

  return next_jpeg_row (pData);        /* we've got one more row of rgb-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g8_a1 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A1, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 1;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;

  while (iX < pData->iRowsamples)
  {
    if (iB & iM)                       /* is it white ? */
      *pOutrow = 0xFF;                 /* white */
    else
      *pOutrow = 0x00;                 /* black */

    pOutrow += 2;                      /* next pixel */
    iX++;
    iM >>= 1;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0x80;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A1, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g8_a2 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A2, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 1;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  while (iX < pData->iRowsamples)
  {
    switch ((iB & iM) >> iS)           /* determine the gray level */
    {
      case 0x03 : { *pOutrow = 0xFF; break; }
      case 0x02 : { *pOutrow = 0xAA; break; }
      case 0x01 : { *pOutrow = 0x55; break; }
      default   : { *pOutrow = 0x00; }
    }

    pOutrow += 2;                      /* next pixel */
    iX++;
    iM >>= 2;
    iS -= 2;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xC0;
      iS = 6;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A2, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g8_a4 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A4, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 1;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  while (iX < pData->iRowsamples)
  {                                    /* get the gray level */
    iQ = (mng_uint8)((iB & iM) >> iS);
    iQ = (mng_uint8)(iQ + (iQ << 4));  /* expand to 8-bit by replication */

    *pOutrow = iQ;                     /* put in object buffer */

    pOutrow += 2;                      /* next pixel */
    iX++;
    iM >>= 4;
    iS -= 4;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xF0;
      iS = 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A4, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g8_a8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 1;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = iB;                     /* put in object buffer */

    pOutrow += 2;                      /* next pixel */
    iX++;

    iB = *pWorkrow;                    /* get next input-byte */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A8, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g8_a16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint16     iW;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A16, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 1;
  iX = 0;                              /* start at pixel 0 */
  iW = mng_get_uint16 (pWorkrow);      /* and get first input 2-byte */
  pWorkrow += 2;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = (mng_uint8)(iW >> 8);   /* only high-order byte! */

    pOutrow += 2;                      /* next pixel */
    iX++;

    iW = mng_get_uint16 (pWorkrow);    /* get next input 2-byte */
    pWorkrow += 2;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G8_A16, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgb8_a1 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A1, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 3;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;

  while (iX < pData->iRowsamples)
  {
    if (iB & iM)                       /* is it white ? */
      *pOutrow = 0xFF;                 /* white */
    else
      *pOutrow = 0x00;                 /* black */

    pOutrow += 4;                      /* next pixel */
    iX++;
    iM >>= 1;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0x80;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A1, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgb8_a2 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A2, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 3;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  while (iX < pData->iRowsamples)
  {
    switch ((iB & iM) >> iS)           /* determine the gray level */
    {
      case 0x03 : { *pOutrow = 0xFF; break; }
      case 0x02 : { *pOutrow = 0xAA; break; }
      case 0x01 : { *pOutrow = 0x55; break; }
      default   : { *pOutrow = 0x00; }
    }

    pOutrow += 4;                      /* next pixel */
    iX++;
    iM >>= 2;
    iS -= 2;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xC0;
      iS = 6;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A2, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgb8_a4 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A4, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 3;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  while (iX < pData->iRowsamples)
  {                                    /* get the gray level */
    iQ = (mng_uint8)((iB & iM) >> iS);
    iQ = (mng_uint8)(iQ + (iQ << 4));  /* expand to 8-bit by replication */

    *pOutrow = iQ;                     /* put in object buffer */

    pOutrow += 4;                      /* next pixel */
    iX++;
    iM >>= 4;
    iS -= 4;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xF0;
      iS = 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A4, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgb8_a8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint8      iB;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 3;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = iB;                     /* put in object buffer */

    pOutrow += 4;                      /* next pixel */
    iX++;

    iB = *pWorkrow;                    /* get next input-byte */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A8, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_rgb8_a16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint8p     pOutrow;
  mng_int32      iX;
  mng_uint16     iW;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A16, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;
  pOutrow  = pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                              (pData->iCol * pBuf->iSamplesize) + 3;
  iX = 0;                              /* start at pixel 0 */
  iW = mng_get_uint16 (pWorkrow);      /* and get first input 2-byte */
  pWorkrow += 2;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = (mng_uint8)(iW >> 8);   /* only high-order byte! */

    pOutrow += 4;                      /* next pixel */
    iX++;

    iW = mng_get_uint16 (pWorkrow);    /* get next input 2-byte */
    pWorkrow += 2;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_RGB8_A16, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g12_a1 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A1, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize) + 2);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;

  while (iX < pData->iRowsamples)
  {
    if (iB & iM)                       /* is it white ? */
      *pOutrow = 0xFFFF;               /* white */
    else
      *pOutrow = 0x0000;               /* black */

    pOutrow += 2;                      /* next pixel */
    iX++;
    iM >>= 1;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0x80;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A1, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g12_a2 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A2, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize) + 2);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  while (iX < pData->iRowsamples)
  {
    switch ((iB & iM) >> iS)           /* determine the gray level */
    {
      case 0x03 : { *pOutrow = 0xFFFF; break; }
      case 0x02 : { *pOutrow = 0xAAAA; break; }
      case 0x01 : { *pOutrow = 0x5555; break; }
      default   : { *pOutrow = 0x0000; }
    }

    pOutrow += 2;                      /* next pixel */
    iX++;
    iM >>= 2;
    iS -= 2;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xC0;
      iS = 6;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A2, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g12_a4 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint16     iQ;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A4, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize) + 2);
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  while (iX < pData->iRowsamples)
  {                                    /* get the gray level */
    iQ = (mng_uint16)((iB & iM) >> iS);
    iQ = (mng_uint16)(iQ + (iQ << 4)); /* expand to 16-bit by replication */
    iQ = (mng_uint16)(iQ + (iQ << 8));
                                       /* put in object buffer */
    mng_put_uint16 ((mng_uint8p)pOutrow, iQ);

    pOutrow += 2;                      /* next pixel */
    iX++;
    iM >>= 4;
    iS -= 4;

    if (!iM)                           /* mask underflow ? */
    {
      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
      iM = 0xF0;
      iS = 4;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A4, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g12_a8 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint8p     pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;
  mng_uint16     iB;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize) + 2);
  iX       = 0;                        /* start at pixel 0 */
  iB       = (mng_uint16)(*pWorkrow);  /* and get first input byte */
  pWorkrow++;

  while (iX < pData->iRowsamples)
  {
    iB = (mng_uint16)(iB + (iB << 8)); /* expand to 16-bit by replication */
                                       /* put in object buffer */
    mng_put_uint16 ((mng_uint8p)pOutrow, iB);

    pOutrow += 2;                      /* next pixel */
    iX++;

    iB = (mng_uint16)(*pWorkrow);      /* get next input-byte */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A8, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

mng_retcode store_jpeg_g12_a16 (mng_datap pData)
{
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;
  mng_uint16p    pWorkrow;
  mng_uint16p    pOutrow;
  mng_int32      iX;
  mng_uint16     iW;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A16, MNG_LC_START);
#endif

  pWorkrow = (mng_uint16p)(pData->pWorkrow + 1);
  pOutrow  = (mng_uint16p)(pBuf->pImgdata + (pData->iRow * pBuf->iRowsize   ) +
                                            (pData->iCol * pBuf->iSamplesize) + 2);
  iX = 0;                              /* start at pixel 0 */
  iW = *pWorkrow;                      /* and get first input 2-byte */
  pWorkrow++;

  while (iX < pData->iRowsamples)
  {
    *pOutrow = iW;                     /* only high-order byte! */

    pOutrow += 2;                      /* next pixel */
    iX++;

    iW = *pWorkrow;                    /* get next input 2-byte */
    pWorkrow++;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_STORE_JPEG_G12_A16, MNG_LC_END);
#endif

  return next_jpeg_alpharow (pData);   /* we've got one more row of alpha-samples */
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */
/* *                                                                        * */
/* * Row processing routines - convert uncompressed data from zlib to       * */
/* * managable row-data which serves as input to the color-management       * */
/* * routines                                                               * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode process_g1 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint32p    pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G1, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = (mng_uint32p)pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    if (pBuf->iTRNSgray)               /* white transparent ? */
    {
      while (iX < pData->iRowsamples)
      {
        if (iB & iM)                   /* is it white ? */
          *pRGBArow = 0x00000000;      /* transparent ! */
        else
#ifdef MNG_SWAP_ENDIAN
          *pRGBArow = 0xFF000000;      /* opaque black */
#else
          *pRGBArow = 0x000000FF;
#endif

        pRGBArow++;                    /* next pixel */
        iX++;
        iM >>= 1;

        if (!iM)                       /* mask underflow ? */
        {
          iB = *pWorkrow;              /* get next input-byte */
          pWorkrow++;
          iM = 0x80;
        }
      }
    }
    else                               /* black transparent */
    {
      while (iX < pData->iRowsamples)
      {
        if (iB & iM)                   /* is it white ? */
          *pRGBArow = 0xFFFFFFFF;      /* opaque white */
        else
          *pRGBArow = 0x00000000;      /* transparent */

        pRGBArow++;                    /* next pixel */
        iX++;
        iM >>= 1;

        if (!iM)                       /* mask underflow ? */
        {
          iB = *pWorkrow;              /* get next input-byte */
          pWorkrow++;
          iM = 0x80;
        }
      }
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else                                 /* no transparency */
  {
    while (iX < pData->iRowsamples)
    {
      if (iB & iM)                     /* is it white ? */
        *pRGBArow = 0xFFFFFFFF;        /* opaque white */
      else
#ifdef MNG_SWAP_ENDIAN
        *pRGBArow = 0xFF000000;        /* opaque black */
#else
        *pRGBArow = 0x000000FF;
#endif

      pRGBArow++;                      /* next pixel */
      iX++;
      iM >>= 1;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0x80;
      }
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G1, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_g2 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint32p    pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G2, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = (mng_uint32p)pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {                                  /* determine gray level */
      iQ = (mng_uint8)((iB & iM) >> iS);

      if (iQ == pBuf->iTRNSgray)       /* transparent ? */
        *pRGBArow = 0x00000000;
      else
      {
        switch (iQ)                    /* determine the gray level */
        {
#ifdef MNG_SWAP_ENDIAN
          case 0x03 : { *pRGBArow = 0xFFFFFFFF; break; }
          case 0x02 : { *pRGBArow = 0xFFAAAAAA; break; }
          case 0x01 : { *pRGBArow = 0xFF555555; break; }
          default   : { *pRGBArow = 0xFF000000; }
#else
          case 0x03 : { *pRGBArow = 0xFFFFFFFF; break; }
          case 0x02 : { *pRGBArow = 0xAAAAAAFF; break; }
          case 0x01 : { *pRGBArow = 0x555555FF; break; }
          default   : { *pRGBArow = 0x000000FF; }
#endif
        }
      }

      pRGBArow++;                      /* next pixel */
      iX++;
      iM >>= 2;
      iS -= 2;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0xC0;
        iS = 6;
      }
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {
      switch ((iB & iM) >> iS)         /* determine the gray level */
      {
#ifdef MNG_SWAP_ENDIAN
        case 0x03 : { *pRGBArow = 0xFFFFFFFF; break; }
        case 0x02 : { *pRGBArow = 0xFFAAAAAA; break; }
        case 0x01 : { *pRGBArow = 0xFF555555; break; }
        default   : { *pRGBArow = 0xFF000000; }
#else
        case 0x03 : { *pRGBArow = 0xFFFFFFFF; break; }
        case 0x02 : { *pRGBArow = 0xAAAAAAFF; break; }
        case 0x01 : { *pRGBArow = 0x555555FF; break; }
        default   : { *pRGBArow = 0x000000FF; }
#endif
      }

      pRGBArow++;                      /* next pixel */
      iX++;
      iM >>= 2;
      iS -= 2;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0xC0;
        iS = 6;
      }
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G2, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_g4 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G4, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the gray level */
      iQ = (mng_uint8)((iB & iM) >> iS);
      iQ = (mng_uint8)(iQ + (iQ << 4));/* expand to 8-bit by replication */

      if (iQ == pBuf->iTRNSgray)       /* transparent ? */
      {
        *pRGBArow     = 0;             /* put in intermediate row */
        *(pRGBArow+1) = 0;
        *(pRGBArow+2) = 0;
        *(pRGBArow+3) = 0;
      }
      else
      {
        *pRGBArow      = iQ;            /* put in intermediate row */
        *(pRGBArow+1) = iQ;
        *(pRGBArow+2) = iQ;
        *(pRGBArow+3) = 0xFF;
      }

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 4;
      iS -= 4;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0xF0;
        iS = 4;
      }
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the gray level */
      iQ = (mng_uint8)((iB & iM) >> iS);
      iQ = (mng_uint8)(iQ + (iQ << 4));/* expand to 8-bit by replication */

      *pRGBArow     = iQ;              /* put in intermediate row */
      *(pRGBArow+1) = iQ;
      *(pRGBArow+2) = iQ;
      *(pRGBArow+3) = 0xFF;

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 4;
      iS -= 4;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0xF0;
        iS = 4;
      }
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G4, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_g8 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G8, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {
      if (iB == pBuf->iTRNSgray)       /* transparent ? */
      {
        *pRGBArow     = 0;             /* put in intermediate row */
        *(pRGBArow+1) = 0;
        *(pRGBArow+2) = 0;
        *(pRGBArow+3) = 0;
      }
      else
      {
        *pRGBArow     = iB;            /* put in intermediate row */
        *(pRGBArow+1) = iB;
        *(pRGBArow+2) = iB;
        *(pRGBArow+3) = 0xFF;
      }

      pRGBArow += 4;                   /* next pixel */
      iX++;

      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {
      *pRGBArow     = iB;              /* put in intermediate row */
      *(pRGBArow+1) = iB;
      *(pRGBArow+2) = iB;
      *(pRGBArow+3) = 0xFF;

      pRGBArow += 4;                   /* next pixel */
      iX++;

      iB = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_g16 (mng_datap pData)
{
  mng_uint16p    pWorkrow;
  mng_uint16p    pRGBArow;
  mng_int32      iX;
  mng_uint16     iW;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G16, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;
                                       /* temporary work pointers */
  pWorkrow = (mng_uint16p)(pData->pWorkrow + 1);
  pRGBArow = (mng_uint16p)pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iW       = *pWorkrow;                /* and get first input 2-byte */
  pWorkrow++;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {
#ifdef MNG_SWAP_ENDIAN                 /* transparent ? */
      if (mng_get_uint16((mng_uint8p)pWorkrow) == pBuf->iTRNSgray)
#else
      if (iW == pBuf->iTRNSgray)
#endif
      {
        *pRGBArow     = 0;             /* put in intermediate row */
        *(pRGBArow+1) = 0;
        *(pRGBArow+2) = 0;
        *(pRGBArow+3) = 0;
      }
      else
      {
        *pRGBArow     = iW;            /* put in intermediate row */
        *(pRGBArow+1) = iW;
        *(pRGBArow+2) = iW;
        *(pRGBArow+3) = 0xFFFF;
      }

      pRGBArow += 4;                   /* next pixel */
      iX++;

      iW = *pWorkrow;                  /* get next input 2-byte */
      pWorkrow++;
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {
      *pRGBArow     = iW;              /* put in intermediate row */
      *(pRGBArow+1) = iW;
      *(pRGBArow+2) = iW;
      *(pRGBArow+3) = 0xFFFF;

      pRGBArow += 4;                   /* next pixel */
      iX++;

      iW = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_G16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_rgb8 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iR, iG, iB;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGB8, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary workpointers */
  pRGBArow = pData->pRGBArow;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iR = *pWorkrow;                  /* get the RGB values */
      iG = *(pWorkrow+1);
      iB = *(pWorkrow+2);
                                       /* transparent ? */
      if ((iR == pBuf->iTRNSred) && (iG == pBuf->iTRNSgreen) &&
          (iB == pBuf->iTRNSblue))
      {
        *pRGBArow     = 0;             /* this pixel is transparent ! */
        *(pRGBArow+1) = 0;
        *(pRGBArow+2) = 0;
        *(pRGBArow+3) = 0;
      }
      else
      {
        *pRGBArow     = iR;            /* copy the RGB values */
        *(pRGBArow+1) = iG;
        *(pRGBArow+2) = iB;
        *(pRGBArow+3) = 0xFF;          /* this one isn't transparent */
      }

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      *pRGBArow     = *pWorkrow;       /* copy the RGB bytes */
      *(pRGBArow+1) = *(pWorkrow+1);
      *(pRGBArow+2) = *(pWorkrow+2);
      *(pRGBArow+3) = 0xFF;            /* no alpha; so always fully opaque */

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGB8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_rgb16 (mng_datap pData)
{
  mng_uint16p    pWorkrow;
  mng_uint16p    pRGBArow;
  mng_int32      iX;
  mng_uint16     iR, iG, iB;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGB16, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = (mng_uint16p)(pData->pWorkrow + 1);
  pRGBArow = (mng_uint16p)pData->pRGBArow;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      iR = *pWorkrow;                  /* get the RGB values */
      iG = *(pWorkrow+1);
      iB = *(pWorkrow+2);

#ifdef MNG_SWAP_ENDIAN                 /* transparent ? */
      if ((mng_get_uint16((mng_uint8p)pWorkrow      ) == pBuf->iTRNSred  ) &&
          (mng_get_uint16((mng_uint8p)(pWorkrow + 1)) == pBuf->iTRNSgreen) &&
          (mng_get_uint16((mng_uint8p)(pWorkrow + 2)) == pBuf->iTRNSblue )    )
#else
      if ((iR == pBuf->iTRNSred) && (iG == pBuf->iTRNSgreen) &&
          (iB == pBuf->iTRNSblue))
#endif
      {
        *pRGBArow     = 0;             /* put in intermediate row */
        *(pRGBArow+1) = 0;
        *(pRGBArow+2) = 0;
        *(pRGBArow+3) = 0;
      }
      else
      {
        *pRGBArow     = iR;            /* put in intermediate row */
        *(pRGBArow+1) = iG;
        *(pRGBArow+2) = iB;
        *(pRGBArow+3) = 0xFFFF;
      }

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    for (iX = 0; iX < pData->iRowsamples; iX++)
    {
      *pRGBArow     = *pWorkrow;       /* copy the RGB values */
      *(pRGBArow+1) = *(pWorkrow+1);
      *(pRGBArow+2) = *(pWorkrow+2);
      *(pRGBArow+3) = 0xFFFF;

      pWorkrow += 3;                   /* next pixel */
      pRGBArow += 4;
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGB16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_idx1 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX1, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;
  iM       = 0x80;
  iS       = 7;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the index */
      iQ = (mng_uint8)((iB & iM) >> iS);
                                       /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        *pRGBArow     = pBuf->aPLTEentries [iQ].iRed;
        *(pRGBArow+1) = pBuf->aPLTEentries [iQ].iGreen;
        *(pRGBArow+2) = pBuf->aPLTEentries [iQ].iBlue;
                                       /* transparency for this index ? */
        if ((mng_uint32)iQ < pBuf->iTRNScount)
          *(pRGBArow+3) = pBuf->aTRNSentries [iQ];
        else
          *(pRGBArow+3) = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 1;
      iS -= 1;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0x80;
        iS = 7;
      }
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the index */
      iQ = (mng_uint8)((iB & iM) >> iS);
                                       /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        *pRGBArow     = pBuf->aPLTEentries [iQ].iRed;
        *(pRGBArow+1) = pBuf->aPLTEentries [iQ].iGreen;
        *(pRGBArow+2) = pBuf->aPLTEentries [iQ].iBlue;
        *(pRGBArow+3) = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 1;
      iS -= 1;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0x80;
        iS = 7;
      }
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX1, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_idx2 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX2, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = pWorkrow [0];             /* and get first input byte */
  pWorkrow++;
  iM       = 0xC0;
  iS       = 6;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the index */
      iQ = (mng_uint8)((iB & iM) >> iS);
                                       /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        *pRGBArow     = pBuf->aPLTEentries [iQ].iRed;
        *(pRGBArow+1) = pBuf->aPLTEentries [iQ].iGreen;
        *(pRGBArow+2) = pBuf->aPLTEentries [iQ].iBlue;
                                       /* transparency for this index ? */
        if ((mng_uint32)iQ < pBuf->iTRNScount)
          *(pRGBArow+3) = pBuf->aTRNSentries [iQ];
        else
          *(pRGBArow+3) = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 2;
      iS -= 2;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0xC0;
        iS = 6;
      }
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the index */
      iQ = (mng_uint8)((iB & iM) >> iS);
                                       /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        *pRGBArow     = pBuf->aPLTEentries [iQ].iRed;
        *(pRGBArow+1) = pBuf->aPLTEentries [iQ].iGreen;
        *(pRGBArow+2) = pBuf->aPLTEentries [iQ].iBlue;
        *(pRGBArow+3) = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 2;
      iS -= 2;

      if (!iM)                         /* mask underflow ? */
      {
        iB = *pWorkrow;                /* get next input-byte */
        pWorkrow++;
        iM = 0xC0;
        iS = 6;
      }
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX2, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_idx4 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iB;
  mng_uint8      iM;
  mng_uint32     iS;
  mng_uint8      iQ;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX4, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iB       = pWorkrow [0];             /* and get first input byte */
  pWorkrow++;
  iM       = 0xF0;
  iS       = 4;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the index */
      iQ = (mng_uint8)((iB & iM) >> iS);
                                       /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        pRGBArow [0] = pBuf->aPLTEentries [iQ].iRed;
        pRGBArow [1] = pBuf->aPLTEentries [iQ].iGreen;
        pRGBArow [2] = pBuf->aPLTEentries [iQ].iBlue;
                                       /* transparency for this index ? */
        if ((mng_uint32)iQ < pBuf->iTRNScount)
          pRGBArow [3] = pBuf->aTRNSentries [iQ];
        else
          pRGBArow [3] = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 4;
      iS -= 4;

      if (!iM)                         /* mask underflow ? */
      {
        iB = pWorkrow [0];             /* get next input-byte */
        pWorkrow++;
        iM = 0xF0;
        iS = 4;
      }
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {                                  /* get the index */
      iQ = (mng_uint8)((iB & iM) >> iS);
                                       /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        pRGBArow [0] = pBuf->aPLTEentries [iQ].iRed;
        pRGBArow [1] = pBuf->aPLTEentries [iQ].iGreen;
        pRGBArow [2] = pBuf->aPLTEentries [iQ].iBlue;
        pRGBArow [3] = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iM >>= 4;
      iS -= 4;

      if (!iM)                         /* mask underflow ? */
      {
        iB = pWorkrow [0];             /* get next input-byte */
        pWorkrow++;
        iM = 0xF0;
        iS = 4;
      }
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX4, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_idx8 (mng_datap pData)
{
  mng_uint8p     pWorkrow;
  mng_uint8p     pRGBArow;
  mng_int32      iX;
  mng_uint8      iQ;
  mng_imagedatap pBuf = (mng_imagedatap)pData->pStorebuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX8, MNG_LC_START);
#endif

  if (!pBuf)                           /* no object? then use obj 0 */
    pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

  pWorkrow = pData->pWorkrow + 1;      /* temporary work pointers */
  pRGBArow = pData->pRGBArow;
  iX       = 0;                        /* start at pixel 0 */
  iQ       = *pWorkrow;                /* and get first input byte */
  pWorkrow++;

  if (pBuf->bHasTRNS)                  /* tRNS encountered ? */
  {
    while (iX < pData->iRowsamples)
    {                                  /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        pRGBArow [0] = pBuf->aPLTEentries [iQ].iRed;
        pRGBArow [1] = pBuf->aPLTEentries [iQ].iGreen;
        pRGBArow [2] = pBuf->aPLTEentries [iQ].iBlue;
                                       /* transparency for this index ? */
        if ((mng_uint32)iQ < pBuf->iTRNScount)
          pRGBArow [3] = pBuf->aTRNSentries [iQ];
        else
          pRGBArow [3] = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iQ = *pWorkrow;                  /* get next input-byte */
      pWorkrow++;
    }

    pData->bIsOpaque = MNG_FALSE;      /* it's not fully opaque */
  }
  else
  {
    while (iX < pData->iRowsamples)
    {                                  /* index valid ? */
      if ((mng_uint32)iQ < pBuf->iPLTEcount)
      {                                /* put in intermediate row */
        pRGBArow [0] = pBuf->aPLTEentries [iQ].iRed;
        pRGBArow [1] = pBuf->aPLTEentries [iQ].iGreen;
        pRGBArow [2] = pBuf->aPLTEentries [iQ].iBlue;
        pRGBArow [3] = 0xFF;
      }
      else
        MNG_ERROR (pData, MNG_PLTEINDEXERROR)

      pRGBArow += 4;                   /* next pixel */
      iX++;
      iQ = pWorkrow [0];               /* get next input-byte */
      pWorkrow++;
    }

    pData->bIsOpaque = MNG_TRUE;       /* it's fully opaque */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_IDX8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ga8 (mng_datap pData)
{
  mng_uint8p pWorkrow;
  mng_uint8p pRGBArow;
  mng_int32  iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_GA8, MNG_LC_START);
#endif

  pWorkrow = pData->pWorkrow + 1;      /* temporary workpointers */
  pRGBArow = pData->pRGBArow;

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pRGBArow     = *pWorkrow;         /* copy the gray value */
    *(pRGBArow+1) = *pWorkrow;
    *(pRGBArow+2) = *pWorkrow;
    *(pRGBArow+3) = *(pWorkrow+1);     /* copy the alpha value */

    pWorkrow += 2;                     /* next pixel */
    pRGBArow += 4;
  }

  pData->bIsOpaque = MNG_FALSE;        /* it's definitely not fully opaque */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_GA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ga16 (mng_datap pData)
{
  mng_uint16p pWorkrow;
  mng_uint16p pRGBArow;
  mng_int32  iX;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_GA16, MNG_LC_START);
#endif
                                       /* temporary workpointers */
  pWorkrow = (mng_uint16p)(pData->pWorkrow + 1);
  pRGBArow = (mng_uint16p)pData->pRGBArow;

  for (iX = 0; iX < pData->iRowsamples; iX++)
  {
    *pRGBArow     = *pWorkrow;         /* copy the gray value */
    *(pRGBArow+1) = *pWorkrow;
    *(pRGBArow+2) = *pWorkrow;
    *(pRGBArow+3) = *(pWorkrow+1);     /* copy the alpha value */

    pWorkrow += 2;                     /* next pixel */
    pRGBArow += 4;
  }

  pData->bIsOpaque = MNG_FALSE;        /* it's definitely not fully opaque */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_GA16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_rgba8 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGBA8, MNG_LC_START);
#endif
                                       /* this is the easiest transform */
  MNG_COPY (pData->pRGBArow, pData->pWorkrow + 1, pData->iRowsize)

  pData->bIsOpaque = MNG_FALSE;        /* it's definitely not fully opaque */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGBA8, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_rgba16 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGBA16, MNG_LC_START);
#endif
                                       /* this is the easiest transform */
  MNG_COPY (pData->pRGBArow, pData->pWorkrow + 1, pData->iRowsize)

  pData->bIsOpaque = MNG_FALSE;        /* it's definitely not fully opaque */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_RGBA16, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Row processing initialization routines - set up the variables needed   * */
/* * to process uncompressed row-data                                       * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode init_g1_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G1_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g1;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g1;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 7;
  pData->iSamplediv  = 3;
  pData->iRowsize    = (pData->iRowsamples + 7) >> 3;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G1_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g1_i      (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G1_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g1;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g1;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 7;
  pData->iSamplediv  = 3;
  pData->iRowsize    = ((pData->iRowsamples + 7) >> 3);
  pData->iRowmax     = ((pData->iDatawidth + 7) >> 3) + 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G1_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g2_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G2_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g2;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g2;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 3;
  pData->iSamplediv  = 2;
  pData->iRowsize    = (pData->iRowsamples + 3) >> 2;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G2_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g2_i      (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G2_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g2;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g2;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 3;
  pData->iSamplediv  = 2;
  pData->iRowsize    = ((pData->iRowsamples + 3) >> 2);
  pData->iRowmax     = ((pData->iDatawidth + 3) >> 2) + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G2_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g4_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G4_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g4;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g4;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 1;
  pData->iSamplediv  = 1;
  pData->iRowsize    = (pData->iRowsamples + 1) >> 1;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G4_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g4_i      (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G4_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g4;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g4;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 1;
  pData->iSamplediv  = 1;
  pData->iRowsize    = ((pData->iRowsamples + 1) >> 1);
  pData->iRowmax     = ((pData->iDatawidth + 1) >> 1) + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G4_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g8_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G8_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g8;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G8_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g8_i      (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G8_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g8;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples;
  pData->iRowmax     = pData->iDatawidth + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G8_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g16_ni    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G16_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g16;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 2;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 1;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 2;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G16_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_g16_i     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G16_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_g16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_g16;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 2;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 1;
  pData->iRowmax     = (pData->iDatawidth << 1) + 1;
  pData->iFilterbpp  = 2;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_G16_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgb8_ni   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB8_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgb8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgb8;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 3;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples * 3;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 3;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB8_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgb8_i    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB8_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgb8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgb8;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 3;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples * 3;
  pData->iRowmax     = (pData->iDatawidth * 3) + 1;
  pData->iFilterbpp  = 3;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB8_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgb16_ni  (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB16_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgb16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgb16;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 6;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples * 6;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 6;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB16_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgb16_i   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB16_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgb16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgb16;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 6;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples * 6;
  pData->iRowmax     = (pData->iDatawidth * 6) + 1;
  pData->iFilterbpp  = 6;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGB16_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx1_ni   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX1_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx1;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx1;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 7;
  pData->iSamplediv  = 3;
  pData->iRowsize    = (pData->iRowsamples + 7) >> 3;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX1_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx1_i    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX1_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx1;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx1;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 7;
  pData->iSamplediv  = 3;
  pData->iRowsize    = (pData->iRowsamples + 7) >> 3;
  pData->iRowmax     = ((pData->iDatawidth + 7) >> 3) + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX1_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx2_ni   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX2_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx2;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx2;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 3;
  pData->iSamplediv  = 2;
  pData->iRowsize    = (pData->iRowsamples + 3) >> 2;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX2_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx2_i    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX2_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx2;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx2;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 3;
  pData->iSamplediv  = 2;
  pData->iRowsize    = (pData->iRowsamples + 3) >> 2;
  pData->iRowmax     = ((pData->iDatawidth + 3) >> 2) + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX2_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx4_ni   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX4_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx4;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx4;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 1;
  pData->iSamplediv  = 1;
  pData->iRowsize    = (pData->iRowsamples + 1) >> 1;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX4_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx4_i    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX4_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx4;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx4;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 1;
  pData->iSamplediv  = 1;
  pData->iRowsize    = (pData->iRowsamples + 1) >> 1;
  pData->iRowmax     = ((pData->iDatawidth + 1) >> 1) + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX4_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx8_ni   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX8_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx8;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX8_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_idx8_i    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX8_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_idx8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_idx8;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples;
  pData->iRowmax     = pData->iDatawidth + 1;
  pData->iFilterbpp  = 1;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_IDX8_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_ga8_ni    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA8_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_ga8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_ga8;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 2;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 1;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 2;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA8_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_ga8_i     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA8_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_ga8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_ga8;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 2;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 1;
  pData->iRowmax     = (pData->iDatawidth << 1) + 1;
  pData->iFilterbpp  = 2;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA8_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_ga16_ni   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA16_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_ga16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_ga16;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 4;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 2;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 4;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA16_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_ga16_i    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA16_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_ga16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_ga16;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 4;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 2;
  pData->iRowmax     = (pData->iDatawidth << 2) + 1;
  pData->iFilterbpp  = 4;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_GA16_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgba8_ni  (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA8_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgba8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgba8;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 4;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 2;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 4;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA8_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgba8_i   (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA8_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgba8;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgba8;

  pData->iPass       = 0;              /* from 0..6; is 1..7 in specifications */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 4;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 2;
  pData->iRowmax     = (pData->iDatawidth << 2) + 1;
  pData->iFilterbpp  = 4;
  pData->bIsRGBA16   = MNG_FALSE;      /* intermediate row is 8-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA8_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgba16_ni (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA16_NI, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgba16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgba16;

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 8;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 3;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 8;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA16_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_rgba16_i  (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA16_I, MNG_LC_START);
#endif

  pData->fProcessrow = (mng_ptr)process_rgba16;

  if (pData->pStoreobj)                /* store in object too ? */
    pData->fStorerow = (mng_ptr)store_rgba16;

  pData->iPass       = 0;              /* from 0..6; (1..7 in specification) */
  pData->iRow        = interlace_row     [0];
  pData->iRowinc     = interlace_rowskip [0];
  pData->iCol        = interlace_col     [0];
  pData->iColinc     = interlace_colskip [0];
  pData->iRowsamples = (pData->iDatawidth + interlace_roundoff [0]) >> interlace_divider [0];
  pData->iSamplemul  = 8;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 3;
  pData->iRowmax     = (pData->iDatawidth << 3) + 1;
  pData->iFilterbpp  = 8;
  pData->bIsRGBA16   = MNG_TRUE;       /* intermediate row is 16-bit deep */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_RGBA16_I, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Row processing initialization routines (JPEG) - set up the variables   * */
/* * needed to process uncompressed row-data                                * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG

/* ************************************************************************** */

mng_retcode init_jpeg_a1_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A1_NI, MNG_LC_START);
#endif

  if (pData->pStoreobj)                /* store in object too ? */
  {
    if (pData->iJHDRimgbitdepth == 8)
    {
      switch (pData->iJHDRcolortype)
      {
        case 12 : { pData->fStorerow = (mng_ptr)store_jpeg_g8_a1;   break; }
        case 14 : { pData->fStorerow = (mng_ptr)store_jpeg_rgb8_a1; break; }
      }
    }

    /* TODO: bitdepth 12 & 20 */
    
  }

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 7;
  pData->iSamplediv  = 3;
  pData->iRowsize    = (pData->iRowsamples + 7) >> 3;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A1_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_jpeg_a2_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A2_NI, MNG_LC_START);
#endif

  if (pData->pStoreobj)                /* store in object too ? */
  {
    if (pData->iJHDRimgbitdepth == 8)
    {
      switch (pData->iJHDRcolortype)
      {
        case 12 : { pData->fStorerow = (mng_ptr)store_jpeg_g8_a2;   break; }
        case 14 : { pData->fStorerow = (mng_ptr)store_jpeg_rgb8_a2; break; }
      }
    }

    /* TODO: bitdepth 12 & 20 */

  }

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 3;
  pData->iSamplediv  = 2;
  pData->iRowsize    = (pData->iRowsamples + 3) >> 2;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A2_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_jpeg_a4_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A4_NI, MNG_LC_START);
#endif

  if (pData->pStoreobj)                /* store in object too ? */
  {
    if (pData->iJHDRimgbitdepth == 8)
    {
      switch (pData->iJHDRcolortype)
      {
        case 12 : { pData->fStorerow = (mng_ptr)store_jpeg_g8_a4;   break; }
        case 14 : { pData->fStorerow = (mng_ptr)store_jpeg_rgb8_a4; break; }
      }
    }

    /* TODO: bitdepth 12 & 20 */

  }

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 1;
  pData->iSamplediv  = 1;
  pData->iRowsize    = (pData->iRowsamples + 1) >> 1;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A4_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_jpeg_a8_ni     (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A8_NI, MNG_LC_START);
#endif

  if (pData->pStoreobj)                /* store in object too ? */
  {
    if (pData->iJHDRimgbitdepth == 8)
    {
      switch (pData->iJHDRcolortype)
      {
        case 12 : { pData->fStorerow = (mng_ptr)store_jpeg_g8_a8;   break; }
        case 14 : { pData->fStorerow = (mng_ptr)store_jpeg_rgb8_a8; break; }
      }
    }

    /* TODO: bitdepth 12 & 20 */

  }

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 1;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 1;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A8_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

mng_retcode init_jpeg_a16_ni    (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A16_NI, MNG_LC_START);
#endif

  if (pData->pStoreobj)                /* store in object too ? */
  {
    if (pData->iJHDRimgbitdepth == 8)
    {
      switch (pData->iJHDRcolortype)
      {
        case 12 : { pData->fStorerow = (mng_ptr)store_jpeg_g8_a16;   break; }
        case 14 : { pData->fStorerow = (mng_ptr)store_jpeg_rgb8_a16; break; }
      }
    }

    /* TODO: bitdepth 12 & 20 */

  }

  pData->iPass       = -1;
  pData->iRow        = 0;
  pData->iRowinc     = 1;
  pData->iCol        = 0;
  pData->iColinc     = 1;
  pData->iRowsamples = pData->iDatawidth;
  pData->iSamplemul  = 2;
  pData->iSampleofs  = 0;
  pData->iSamplediv  = 0;
  pData->iRowsize    = pData->iRowsamples << 1;
  pData->iRowmax     = pData->iRowsize + 1;
  pData->iFilterbpp  = 2;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_JPEG_A16_NI, MNG_LC_END);
#endif

  return init_rowproc (pData);
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */
/* *                                                                        * */
/* * Generic row processing initialization & cleanup routines               * */
/* * - initialize the buffers used by the row processing routines           * */
/* * - cleanup the buffers used by the row processing routines              * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode init_rowproc (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_ROWPROC, MNG_LC_START);
#endif

  if (pData->pStoreobj)                /* storage object selected ? */
  {
    pData->pStorebuf = ((mng_imagep)pData->pStoreobj)->pImgbuf;
                                       /* and so it becomes viewable ! */
    ((mng_imagep)pData->pStoreobj)->bViewable     = MNG_TRUE;
    ((mng_imagedatap)pData->pStorebuf)->bViewable = MNG_TRUE;
  }

  /* allocate the buffers; the individual init routines have already
     calculated the required maximum size; except in the case of a JNG
     without alpha */
  if (pData->iRowmax)
  {
    MNG_ALLOC (pData, pData->pWorkrow, pData->iRowmax)
    MNG_ALLOC (pData, pData->pPrevrow, pData->iRowmax)
  }  

  /* allocate an RGBA16 row for intermediate processing */
  MNG_ALLOC (pData, pData->pRGBArow, (pData->iDatawidth << 3));

#ifndef MNG_NO_CMS
  if (pData->fDisplayrow)              /* display "on-the-fly" ? */
  {
#if defined(MNG_FULL_CMS)              /* determine color-management routine */
    mng_retcode iRetcode = init_full_cms   (pData);
#elif defined(MNG_GAMMA_ONLY)
    mng_retcode iRetcode = init_gamma_only (pData);
#elif defined(MNG_APP_CMS)
    mng_retcode iRetcode = init_app_cms    (pData);
#endif
    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
#endif /* !MNG_NO_CMS */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INIT_ROWPROC, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode next_row (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_NEXT_ROW, MNG_LC_START);
#endif

  pData->iRow += pData->iRowinc;       /* increase the row counter */

  if (pData->iPass >= 0)               /* interlaced ? */
  {
    while ((pData->iPass < 7) &&       /* went 'outside' the image ? */
           ((pData->iRow >= (mng_int32)pData->iDataheight) ||
            (pData->iCol >= (mng_int32)pData->iDatawidth )    ))
    {
      pData->iPass++;                  /* next pass ! */

      if (pData->iPass < 7)            /* there's only 7 passes ! */
      {
        pData->iRow        = interlace_row     [pData->iPass];
        pData->iRowinc     = interlace_rowskip [pData->iPass];
        pData->iCol        = interlace_col     [pData->iPass];
        pData->iColinc     = interlace_colskip [pData->iPass];
        pData->iRowsamples = (pData->iDatawidth - pData->iCol + interlace_roundoff [pData->iPass])
                                 >> interlace_divider [pData->iPass];

        if (pData->iSamplemul > 1)     /* recalculate row dimension */
          pData->iRowsize  = pData->iRowsamples * pData->iSamplemul;
        else
        if (pData->iSamplediv > 0)
          pData->iRowsize  = (pData->iRowsamples + pData->iSampleofs) >> pData->iSamplediv;
        else
          pData->iRowsize  = pData->iRowsamples;

      }
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_NEXT_ROW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode cleanup_rowproc (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLEANUP_ROWPROC, MNG_LC_START);
#endif

#ifdef MNG_INCLUDE_LCMS
  if (pData->hTrans)                   /* cleanup CMS transform */
    mnglcms_freetransform (pData->hTrans);

  pData->hTrans = 0;

  if (pData->hProf1)                   /* cleanup CMS image-profile */
    mnglcms_freeprofile (pData->hProf1);

  pData->hProf1 = 0;
#endif /* MNG_INCLUDE_LCMS */

  if (pData->pWorkrow)                 /* cleanup buffer for working row */
    MNG_FREE (pData, pData->pWorkrow, pData->iRowmax)

  if (pData->pPrevrow)                 /* cleanup buffer for previous row */
    MNG_FREE (pData, pData->pPrevrow, pData->iRowmax)

  if (pData->pRGBArow)                 /* cleanup buffer for intermediate row */
    MNG_FREE (pData, pData->pRGBArow, (pData->iDatawidth << 3))

  pData->pWorkrow = 0;                 /* propogate uninitialized buffers */
  pData->pPrevrow = 0;
  pData->pRGBArow = 0;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLEANUP_ROWPROC, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* woohiii */
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Generic row processing routines for JNG                                * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_JNG

/* ************************************************************************** */

mng_retcode display_jpeg_rows (mng_datap pData)
{
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_JPEG_ROWS, MNG_LC_START);
#endif
                                       /* any completed rows ? */
  if ((pData->iJPEGrow      > pData->iJPEGdisprow) &&
      (pData->iJPEGalpharow > pData->iJPEGdisprow)    )
  {
    mng_uint32 iX, iMax;
    mng_uint32 iSaverow = pData->iRow; /* save alpha decompression row-count */
                                       /* determine the highest complete(!) row */
    if (pData->iJPEGrow > pData->iJPEGalpharow)
      iMax = pData->iJPEGalpharow;
    else
      iMax = pData->iJPEGrow;
                                       /* display the rows */
    for (iX = pData->iJPEGdisprow; iX < iMax; iX++)
    {
      pData->iRow = iX;                /* make sure we all know which row to handle */
                                       /* makeup an intermediate row from the buffer */
      iRetcode = ((mng_retrieverow)pData->fRetrieverow) (pData);
                                       /* color-correct it if necessary */
      if ((!iRetcode) && (pData->fCorrectrow))
        iRetcode = ((mng_correctrow)pData->fCorrectrow) (pData);

      if (!iRetcode)                   /* and display it */
        iRetcode = ((mng_displayrow)pData->fDisplayrow) (pData);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }

    pData->iJPEGdisprow = iMax;        /* keep track of the last displayed row */
    pData->iRow         = iSaverow;    /* restore alpha decompression row-count */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DISPLAY_JPEG_ROWS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode next_jpeg_alpharow (mng_datap pData)
{
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_NEXT_JPEG_ALPHAROW, MNG_LC_START);
#endif

  pData->iJPEGalpharow++;              /* count the row */

  if (pData->fDisplayrow)              /* display "on-the-fly" ? */
  {                                    /* try to display what you can */
    iRetcode = display_jpeg_rows (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_NEXT_JPEG_ALPHAROW, MNG_LC_END);
#endif

  return MNG_NOERROR; 
}

/* ************************************************************************** */

mng_retcode next_jpeg_row (mng_datap pData)
{
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_NEXT_JPEG_ROW, MNG_LC_START);
#endif

  pData->iJPEGrow++;                   /* increase the row-counter */
  
  if (pData->fDisplayrow)              /* display "on-the-fly" ? */
  {                                    /* has alpha channel ? */
    if ((pData->iJHDRcolortype == MNG_COLORTYPE_JPEGGRAYA ) ||
        (pData->iJHDRcolortype == MNG_COLORTYPE_JPEGCOLORA)    )
    {                                  /* try to display what you can */
      iRetcode = display_jpeg_rows (pData);
    }
    else
    {                                  /* make sure we all know which row to handle */
      pData->iRow = pData->iJPEGrow - 1;
                                       /* makeup an intermediate row from the buffer */
      iRetcode = ((mng_retrieverow)pData->fRetrieverow) (pData);
                                       /* color-correct it if necessary */
      if ((!iRetcode) && (pData->fCorrectrow))
        iRetcode = ((mng_correctrow)pData->fCorrectrow) (pData);

      if (!iRetcode)                   /* and display it */
        iRetcode = ((mng_displayrow)pData->fDisplayrow) (pData);

    }

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }

                                       /* surpassed last filled row ? */
  if (pData->iJPEGrow > pData->iJPEGrgbrow)
    pData->iJPEGrgbrow = pData->iJPEGrow;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_NEXT_JPEG_ROW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_JNG */

/* ************************************************************************** */

#endif /* MNG_INCLUDE_DISPLAY_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

