/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_object_prc.c          copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.1                                                      * */
/* *                                                                        * */
/* * purpose   : Object processing routines (implementation)                * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the internal object processing routines  * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/12/2000 - G.Juyn                                * */
/* *             - changed trace to macro for callback error-reporting      * */
/* *                                                                        * */
/* *             0.5.2 - 05/20/2000 - G.Juyn                                * */
/* *             - fixed to support JNG objects                             * */
/* *                                                                        * */
/* ************************************************************************** */

#include "libmng.h"
#include "mng_data.h"
#include "mng_error.h"
#include "mng_trace.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "mng_memory.h"
#include "mng_objects.h"
#include "mng_display.h"
#include "mng_object_prc.h"

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

/* ************************************************************************** */

#ifdef MNG_INCLUDE_DISPLAY_PROCS

/* ************************************************************************** */
/* *                                                                        * */
/* * Image-data-object routines                                             * */
/* *                                                                        * */
/* * these handle the "object buffer" as defined by the MNG specification   * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode create_imagedataobject (mng_datap      pData,
                                    mng_bool       bConcrete,
                                    mng_bool       bViewable,
                                    mng_uint32     iWidth,
                                    mng_uint32     iHeight,
                                    mng_uint8      iBitdepth,
                                    mng_uint8      iColortype,
                                    mng_imagedatap *ppObject)
{
  mng_imagedatap pImagedata;
  mng_uint32 iSamplesize = 0;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_IMGDATAOBJECT, MNG_LC_START)
#endif
                                       /* get a buffer */
  MNG_ALLOC (pData, pImagedata, sizeof (mng_imagedata))
                                       /* fill the appropriate fields */
  pImagedata->sHeader.fCleanup   = (mng_cleanupobject)free_imagedataobject;
  pImagedata->sHeader.fProcess   = 0;
  pImagedata->iRefcount          = 1;
  pImagedata->bFrozen            = MNG_FALSE;
  pImagedata->bConcrete          = bConcrete;
  pImagedata->bViewable          = bViewable;
  pImagedata->iWidth             = iWidth;
  pImagedata->iHeight            = iHeight;
  pImagedata->iBitdepth          = iBitdepth;
  pImagedata->iColortype         = iColortype;
                                       /* determine samplesize from color_type/bit_depth */
  switch (iColortype)                  /* for < 8-bit samples we just reserve 8 bits */
  {
    case  0  : ;                       /* gray */
    case  8  : {                       /* JPEG gray */
                 if (iBitdepth > 8)
                   iSamplesize = 2;
                 else
                   iSamplesize = 1;

                 break;
               }
    case  2  : ;                       /* rgb */
    case 10  : {                       /* JPEG rgb */
                 if (iBitdepth > 8)
                   iSamplesize = 6;
                 else
                   iSamplesize = 3;

                 break;
               }
    case  3  : {                       /* indexed */
                 iSamplesize = 1;
                 break;
               }
    case  4  : ;                       /* gray+alpha */
    case 12  : {                       /* JPEG gray+alpha */
                 if (iBitdepth > 8)
                   iSamplesize = 4;
                 else
                   iSamplesize = 2;

                 break;
               }
    case  6  : ;                       /* rgb+alpha */
    case 14  : {                       /* JPEG rgb+alpha */
                 if (iBitdepth > 8)
                   iSamplesize = 8;
                 else
                   iSamplesize = 4;

                 break;
               }
  }
                                       /* make sure we remember all this */
  pImagedata->iSamplesize  = iSamplesize;
  pImagedata->iRowsize     = iSamplesize * iWidth;
  pImagedata->iImgdatasize = pImagedata->iRowsize * iHeight;

  if (pImagedata->iImgdatasize)        /* need a buffer ? */
  {                                    /* so allocate it */
    MNG_ALLOCX (pData, pImagedata->pImgdata, pImagedata->iImgdatasize)

    if (!pImagedata->pImgdata)         /* enough memory ? */
    {
      MNG_FREEX (pData, pImagedata, sizeof (mng_imagedata))
      MNG_ERROR (pData, MNG_OUTOFMEMORY)
    }
  }
                                       /* check global stuff */
  pImagedata->bHasGAMA           = pData->bHasglobalGAMA;
  pImagedata->bHasCHRM           = pData->bHasglobalCHRM;
  pImagedata->bHasSRGB           = pData->bHasglobalSRGB;
  pImagedata->bHasICCP           = pData->bHasglobalICCP;
  pImagedata->bHasBKGD           = pData->bHasglobalBKGD;

  if (pData->bHasglobalGAMA)           /* global gAMA present ? */
    pImagedata->iGamma           = pData->iGlobalGamma;

  if (pData->bHasglobalCHRM)           /* global cHRM present ? */
  {
    pImagedata->iWhitepointx     = pData->iGlobalWhitepointx;
    pImagedata->iWhitepointy     = pData->iGlobalWhitepointy;
    pImagedata->iPrimaryredx     = pData->iGlobalPrimaryredx;
    pImagedata->iPrimaryredy     = pData->iGlobalPrimaryredy;
    pImagedata->iPrimarygreenx   = pData->iGlobalPrimarygreenx;
    pImagedata->iPrimarygreeny   = pData->iGlobalPrimarygreeny;
    pImagedata->iPrimarybluex    = pData->iGlobalPrimarybluex;
    pImagedata->iPrimarybluey    = pData->iGlobalPrimarybluey;
  }

  if (pData->bHasglobalSRGB)           /* glbal sRGB present ? */
    pImagedata->iRenderingintent = pData->iGlobalRendintent;

  if (pData->bHasglobalICCP)           /* glbal iCCP present ? */
  {
    pImagedata->iProfilesize     = pData->iGlobalProfilesize;

    if (pImagedata->iProfilesize)
    {
      MNG_ALLOCX (pData, pImagedata->pProfile, pImagedata->iProfilesize)

      if (!pImagedata->pProfile)       /* enough memory ? */
      {
        MNG_FREEX (pData, pImagedata->pImgdata, pImagedata->iImgdatasize)
        MNG_FREEX (pData, pImagedata, sizeof (mng_imagedata))
        MNG_ERROR (pData, MNG_OUTOFMEMORY)
      }

      MNG_COPY  (pImagedata->pProfile, pData->pGlobalProfile, pImagedata->iProfilesize)
    }
  }

  if (pData->bHasglobalBKGD)           /* global bKGD present ? */
  {
    pImagedata->iBKGDred         = pData->iGlobalBKGDred;
    pImagedata->iBKGDgreen       = pData->iGlobalBKGDgreen;
    pImagedata->iBKGDblue        = pData->iGlobalBKGDblue;
  }

  *ppObject = pImagedata;              /* return it */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_IMGDATAOBJECT, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_imagedataobject   (mng_datap      pData,
                                    mng_imagedatap pImagedata)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_IMGDATAOBJECT, MNG_LC_START)
#endif

  if (pImagedata->iRefcount)           /* decrease reference count */
    pImagedata->iRefcount--;

  if (!pImagedata->iRefcount)          /* reached zero ? */
  {
    if (pImagedata->iProfilesize)      /* stored an iCCP profile ? */
      MNG_FREEX (pData, pImagedata->pProfile, pImagedata->iProfilesize)

    if (pImagedata->iImgdatasize)      /* sample-buffer present ? */
      MNG_FREEX (pData, pImagedata->pImgdata, pImagedata->iImgdatasize)
                                       /* drop the buffer */
    MNG_FREEX (pData, pImagedata, sizeof (mng_imagedata))
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_IMGDATAOBJECT, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode clone_imagedataobject  (mng_datap      pData,
                                    mng_bool       bConcrete,
                                    mng_imagedatap pSource,
                                    mng_imagedatap *ppClone)
{
  mng_imagedatap pNewdata;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLONE_IMGDATAOBJECT, MNG_LC_START)
#endif
                                       /* get a buffer */
  MNG_ALLOC (pData, pNewdata, sizeof (mng_imagedata))
                                       /* blatently copy the original buffer */
  MNG_COPY (pNewdata, pSource, sizeof (mng_imagedata))

  pNewdata->iRefcount = 1;             /* only the reference count */
  pNewdata->bConcrete = bConcrete;     /* and concrete-flag are different */

  if (pSource->iImgdatasize)           /* sample buffer present ? */
  {
    MNG_ALLOCX (pData, pNewdata->pImgdata, pNewdata->iImgdatasize)

    if (!pNewdata->pImgdata)           /* not enough memory ? */
    {
      MNG_FREEX (pData, pNewdata, sizeof (mng_imagedata))
      MNG_ERROR (pData, MNG_OUTOFMEMORY)
    }
                                       /* make a copy */
    MNG_COPY (pNewdata->pImgdata, pSource->pImgdata, pNewdata->iImgdatasize)
  }

  if (pNewdata->iProfilesize)          /* iCCP profile present ? */
  {
    MNG_ALLOCX (pData, pNewdata->pProfile, pNewdata->iProfilesize)

    if (!pNewdata->pProfile)           /* enough memory ? */
    {
      MNG_FREEX (pData, pNewdata, sizeof (mng_imagedata))
      MNG_ERROR (pData, MNG_OUTOFMEMORY)
    }
                                       /* make a copy */
    MNG_COPY (pNewdata->pProfile, pSource->pProfile, pNewdata->iProfilesize)
  }

  *ppClone = pNewdata;                 /* return the clone */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLONE_IMGDATAOBJECT, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Image-object routines                                                  * */
/* *                                                                        * */
/* * these handle the "object" as defined by the MNG specification          * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode create_imageobject (mng_datap  pData,
                                mng_uint16 iId,
                                mng_bool   bConcrete,
                                mng_bool   bVisible,
                                mng_bool   bViewable,
                                mng_uint32 iWidth,
                                mng_uint32 iHeight,
                                mng_uint8  iBitdepth,
                                mng_uint8  iColortype,
                                mng_int32  iPosx,
                                mng_int32  iPosy,
                                mng_bool   bClipped,
                                mng_int32  iClipl,
                                mng_int32  iClipr,
                                mng_int32  iClipt,
                                mng_int32  iClipb,
                                mng_imagep *ppObject)
{
  mng_imagep     pImage;
  mng_imagep     pPrev, pNext;
  mng_retcode    iRetcode;
  mng_imagedatap pImgbuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_IMGOBJECT, MNG_LC_START)
#endif
                                       /* get a buffer */
  MNG_ALLOC (pData, pImage, sizeof (mng_image))
                                       /* now get a new "object buffer" */
  iRetcode = create_imagedataobject (pData, bConcrete, bViewable,
                                     iWidth, iHeight, iBitdepth, iColortype,
                                     &pImgbuf);

  if (iRetcode)                        /* on error bail out */
  {
    MNG_FREEX (pData, pImage, sizeof (mng_image))
    return iRetcode;
  }
                                       /* fill the appropriate fields */
  pImage->sHeader.fCleanup = (mng_cleanupobject)free_imageobject;
  pImage->sHeader.fProcess = 0;
  pImage->iId              = iId;
  pImage->bFrozen          = MNG_FALSE;
  pImage->bVisible         = bVisible;
  pImage->bViewable        = bViewable;
  pImage->iPosx            = iPosx;
  pImage->iPosy            = iPosy;
  pImage->bClipped         = bClipped;
  pImage->iClipl           = iClipl;
  pImage->iClipr           = iClipr;
  pImage->iClipt           = iClipt;
  pImage->iClipb           = iClipb;
  pImage->pImgbuf          = pImgbuf;

  if (iId)                             /* only if not object 0 ! */
  {                                    /* find previous lower object-id */
    pPrev = (mng_imagep)pData->pLastimgobj;

    while ((pPrev) && (pPrev->iId > iId))
      pPrev = (mng_imagep)pPrev->sHeader.pPrev;

    if (pPrev)                         /* found it ? */
    {
      pImage->sHeader.pPrev = pPrev;   /* than link it in place */
      pImage->sHeader.pNext = pPrev->sHeader.pNext;
      pPrev->sHeader.pNext  = pImage;
    }
    else                               /* if not found, it becomes the first ! */
    {
      pImage->sHeader.pNext = pData->pFirstimgobj;
      pData->pFirstimgobj   = pImage;
    }

    pNext                   = (mng_imagep)pImage->sHeader.pNext;

    if (pNext)
      pNext->sHeader.pPrev  = pImage;
    else
      pData->pLastimgobj    = pImage;
    
  }  

  *ppObject = pImage;                  /* and return the new buffer */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_IMGOBJECT, MNG_LC_END)
#endif

  return MNG_NOERROR;                  /* okido */
}

/* ************************************************************************** */

mng_retcode free_imageobject (mng_datap  pData,
                              mng_imagep pImage)
{
  mng_retcode    iRetcode;
  mng_imagep     pPrev   = pImage->sHeader.pPrev;
  mng_imagep     pNext   = pImage->sHeader.pNext;
  mng_imagedatap pImgbuf = pImage->pImgbuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_IMGOBJECT, MNG_LC_START)
#endif

  if (pImage->iId)                     /* not for object 0 */
  {
    if (pPrev)                         /* unlink from the list first ! */
      pPrev->sHeader.pNext = pImage->sHeader.pNext;
    else
      pData->pFirstimgobj  = pImage->sHeader.pNext;

    if (pNext)
      pNext->sHeader.pPrev = pImage->sHeader.pPrev;
    else
      pData->pLastimgobj   = pImage->sHeader.pPrev;

  }
                                       /* unlink the image-data buffer */
  iRetcode = free_imagedataobject (pData, pImgbuf);
                                       /* drop it's own buffer */
  MNG_FREEX (pData, pImage, sizeof (mng_image))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_IMGOBJECT, MNG_LC_END)
#endif

  return iRetcode;
}

/* ************************************************************************** */

mng_imagep find_imageobject (mng_datap  pData,
                             mng_uint16 iId)
{
  mng_imagep pImage = (mng_imagep)pData->pFirstimgobj;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACEX (pData, MNG_FN_FIND_IMGOBJECT, MNG_LC_START)
#endif
                                       /* look up the right id */
  while ((pImage) && (pImage->iId != iId))
    pImage = (mng_imagep)pImage->sHeader.pNext;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACEX (pData, MNG_FN_FIND_IMGOBJECT, MNG_LC_END)
#endif

  return pImage;
}

/* ************************************************************************** */

mng_retcode clone_imageobject (mng_datap  pData,
                               mng_uint16 iId,
                               mng_bool   bPartial,
                               mng_bool   bVisible,
                               mng_bool   bAbstract,
                               mng_bool   bHasloca,
                               mng_uint8  iLocationtype,
                               mng_int32  iLocationx,
                               mng_int32  iLocationy,
                               mng_imagep pSource,
                               mng_imagep *ppClone)
{
  mng_imagep     pNew;
  mng_imagep     pPrev, pNext;
  mng_retcode    iRetcode;
  mng_imagedatap pImgbuf;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLONE_IMGOBJECT, MNG_LC_START)
#endif
                                       /* get a buffer */
  MNG_ALLOC (pData, pNew, sizeof (mng_image))
                                       /* fill or copy the appropriate fields */
  pNew->sHeader.fCleanup = (mng_cleanupobject)free_imageobject;
  pNew->sHeader.fProcess = 0;
  pNew->iId              = iId;
  pNew->bFrozen          = MNG_FALSE;
  pNew->bVisible         = bVisible;
  pNew->bViewable        = pSource->bViewable;

  if (bHasloca)                        /* location info available ? */
  {
    if (iLocationtype == 0)            /* absolute position ? */
    {
      pNew->iPosx        = iLocationx;
      pNew->iPosy        = iLocationy;
    }
    else                               /* relative */
    {
      pNew->iPosx        = pSource->iPosx + iLocationx;
      pNew->iPosy        = pSource->iPosy + iLocationy;
    }
  }
  else                                 /* copy from source */
  {
    pNew->iPosx          = pSource->iPosx;
    pNew->iPosy          = pSource->iPosy;
  }
                                       /* copy clipping info */
  pNew->bClipped         = pSource->bClipped;
  pNew->iClipl           = pSource->iClipl;
  pNew->iClipr           = pSource->iClipr;
  pNew->iClipt           = pSource->iClipt;
  pNew->iClipb           = pSource->iClipb;

  if (iId)                             /* not for object 0 */
  {                                    /* find previous lower object-id */
    pPrev = (mng_imagep)pData->pLastimgobj;
    while ((pPrev) && (pPrev->iId > iId))
      pPrev = (mng_imagep)pPrev->sHeader.pPrev;

    if (pPrev)                         /* found it ? */
    {
      pNew->sHeader.pPrev  = pPrev;    /* than link it in place */
      pNew->sHeader.pNext  = pPrev->sHeader.pNext;
      pPrev->sHeader.pNext = pNew;
    }
    else                               /* if not found, it becomes the first ! */
    {
      pNew->sHeader.pNext  = pData->pFirstimgobj;
      pData->pFirstimgobj  = pNew;
    }

    pNext                  = (mng_imagep)pNew->sHeader.pNext;
    
    if (pNext)
      pNext->sHeader.pPrev = pNew;
    else
      pData->pLastimgobj   = pNew;

  }

  if (bPartial)                        /* partial clone ? */
  {
    pNew->pImgbuf = pSource->pImgbuf;  /* use the same object buffer */
    pNew->pImgbuf->iRefcount++;        /* and increase the reference count */
  }
  else                                 /* create a full clone ! */
  {
    mng_bool bConcrete = MNG_FALSE;    /* it's abstract by default (?) */

    if (!bAbstract)                    /* determine concreteness from source ? */
      bConcrete = pSource->pImgbuf->bConcrete;
                                       /* create a full clone ! */
    iRetcode = clone_imagedataobject (pData, bConcrete, pSource->pImgbuf, &pImgbuf);

    if (iRetcode)                      /* on error bail out */
    {
      MNG_FREEX (pData, pNew, sizeof (mng_image))
      return iRetcode;
    }

    pNew->pImgbuf = pImgbuf;           /* and remember it */
  }

  *ppClone = pNew;                     /* return it */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLONE_IMGOBJECT, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode renum_imageobject (mng_datap  pData,
                               mng_imagep pSource,
                               mng_uint16 iId,
                               mng_bool   bVisible,
                               mng_bool   bAbstract,
                               mng_bool   bHasloca,
                               mng_uint8  iLocationtype,
                               mng_int32  iLocationx,
                               mng_int32  iLocationy)
{
  mng_imagep pPrev, pNext;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RENUM_IMGOBJECT, MNG_LC_START)
#endif

  pSource->bVisible  = bVisible;       /* store the new visibility */

  if (bHasloca)                        /* location info available ? */
  {
    if (iLocationtype == 0)            /* absolute position ? */
    {
      pSource->iPosx = iLocationx;
      pSource->iPosy = iLocationy;
    }
    else                               /* relative */
    {
      pSource->iPosx = pSource->iPosx + iLocationx;
      pSource->iPosy = pSource->iPosy + iLocationy;
    }
  }

  if (iId)                             /* not for object 0 */
  {                                    /* find previous lower object-id */
    pPrev = (mng_imagep)pData->pLastimgobj;
    while ((pPrev) && (pPrev->iId > iId))
      pPrev = (mng_imagep)pPrev->sHeader.pPrev;
                                       /* different from current ? */
    if (pPrev != pSource->sHeader.pPrev)
    {
      if (pSource->sHeader.pPrev)      /* unlink from current position !! */
        ((mng_imagep)pSource->sHeader.pPrev)->sHeader.pNext = pSource->sHeader.pNext;
      else
        pData->pFirstimgobj                                 = pSource->sHeader.pNext;

      if (pSource->sHeader.pNext)
        ((mng_imagep)pSource->sHeader.pNext)->sHeader.pPrev = pSource->sHeader.pPrev;
      else
        pData->pLastimgobj                                  = pSource->sHeader.pPrev;

      if (pPrev)                       /* found the previous ? */
      {                                /* than link it in place */
        pSource->sHeader.pPrev = pPrev;
        pSource->sHeader.pNext = pPrev->sHeader.pNext;
        pPrev->sHeader.pNext   = pSource;
      }
      else                             /* if not found, it becomes the first ! */
      {
        pSource->sHeader.pNext = pData->pFirstimgobj;
        pData->pFirstimgobj    = pSource;
      }
      
      pNext                    = (mng_imagep)pSource->sHeader.pNext;

      if (pNext)
        pNext->sHeader.pPrev   = pSource;
      else
        pData->pLastimgobj     = pSource;

    }
  }

  pSource->iId = iId;                  /* now set the new id! */

  if (bAbstract)                       /* force it to abstract ? */
    pSource->pImgbuf->bConcrete = MNG_FALSE;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RENUM_IMGOBJECT, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode reset_object_details (mng_datap  pData,
                                  mng_imagep pImage,
                                  mng_uint32 iWidth,
                                  mng_uint32 iHeight,
                                  mng_uint8  iBitdepth,
                                  mng_uint8  iColortype,
                                  mng_bool   bResetall)
{
  mng_imagedatap pBuf  = pImage->pImgbuf;
  mng_uint32     iSamplesize = 0;
  mng_uint32     iRowsize;
  mng_uint32     iImgdatasize;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESET_OBJECTDETAILS, MNG_LC_START)
#endif

  pBuf->iWidth     = iWidth;           /* set buffer characteristics */
  pBuf->iHeight    = iHeight;
  pBuf->iBitdepth  = iBitdepth;
  pBuf->iColortype = iColortype;
                                       /* determine samplesize from color_type/bit_depth */
  switch (iColortype)                  /* for < 8-bit samples we just reserve 8 bits */
  {
    case  0  : ;                       /* gray */
    case  8  : {                       /* JPEG gray */
                 if (iBitdepth > 8)
                   iSamplesize = 2;
                 else
                   iSamplesize = 1;

                 break;
               }
    case  2  : ;                       /* rgb */
    case 10  : {                       /* JPEG rgb */
                 if (iBitdepth > 8)
                   iSamplesize = 6;
                 else
                   iSamplesize = 3;

                 break;
               }
    case  3  : {                       /* indexed */
                 iSamplesize = 1;
                 break;
               }
    case  4  : ;                       /* gray+alpha */
    case 12  : {                       /* JPEG gray+alpha */
                 if (iBitdepth > 8)
                   iSamplesize = 4;
                 else
                   iSamplesize = 2;

                 break;
               }
    case  6  : ;                       /* rgb+alpha */
    case 14  : {                       /* JPEG rgb+alpha */
                 if (iBitdepth > 8)
                   iSamplesize = 8;
                 else
                   iSamplesize = 4;

                 break;
               }
  }

  iRowsize     = iSamplesize * iWidth;
  iImgdatasize = iRowsize    * iHeight;
                                       /* buffer size changed ? */
  if (iImgdatasize != pBuf->iImgdatasize)
  {                                    /* drop the old one */
    MNG_FREE (pData, pBuf->pImgdata, pBuf->iImgdatasize)

    if (iImgdatasize)                  /* allocate new sample-buffer ? */
      MNG_ALLOC (pData, pBuf->pImgdata, iImgdatasize)
  }

  pBuf->iSamplesize  = iSamplesize;    /* remember new sizes */
  pBuf->iRowsize     = iRowsize;
  pBuf->iImgdatasize = iImgdatasize;
                                       /* dimension set and clipping not ? */
  if ((iWidth) && (iHeight) && (!pImage->bClipped))
  {
    pImage->iClipl   = 0;              /* set clipping to dimension by default */
    pImage->iClipr   = iWidth;
    pImage->iClipt   = 0;
    pImage->iClipb   = iHeight;
  }

  if (bResetall)                       /* reset the other characteristics ? */
  {
    pBuf->bHasPLTE = MNG_FALSE;
    pBuf->bHasTRNS = MNG_FALSE;
    pBuf->bHasGAMA = pData->bHasglobalGAMA;
    pBuf->bHasCHRM = pData->bHasglobalCHRM;
    pBuf->bHasSRGB = pData->bHasglobalSRGB;
    pBuf->bHasICCP = pData->bHasglobalICCP;
    pBuf->bHasBKGD = pData->bHasglobalBKGD;

    if (pBuf->iProfilesize)            /* drop possibly old ICC profile */
      MNG_FREE (pData, pBuf->pProfile, pBuf->iProfilesize)

    if (pData->bHasglobalGAMA)         /* global gAMA present ? */
      pBuf->iGamma           = pData->iGlobalGamma;

    if (pData->bHasglobalCHRM)         /* global cHRM present ? */
    {
      pBuf->iWhitepointx     = pData->iGlobalWhitepointx;
      pBuf->iWhitepointy     = pData->iGlobalWhitepointy;
      pBuf->iPrimaryredx     = pData->iGlobalPrimaryredx;
      pBuf->iPrimaryredy     = pData->iGlobalPrimaryredy;
      pBuf->iPrimarygreenx   = pData->iGlobalPrimarygreenx;
      pBuf->iPrimarygreeny   = pData->iGlobalPrimarygreeny;
      pBuf->iPrimarybluex    = pData->iGlobalPrimarybluex;
      pBuf->iPrimarybluey    = pData->iGlobalPrimarybluey;
    }

    if (pData->bHasglobalSRGB)           /* glbal sRGB present ? */
      pBuf->iRenderingintent = pData->iGlobalRendintent;

    if (pData->bHasglobalICCP)           /* glbal iCCP present ? */
    {
      if (pData->iGlobalProfilesize)
      {
        MNG_ALLOC (pData, pBuf->pProfile, pData->iGlobalProfilesize)
        MNG_COPY  (pBuf->pProfile, pData->pGlobalProfile, pData->iGlobalProfilesize)
      }

      pBuf->iProfilesize     = pData->iGlobalProfilesize;
    }

    if (pData->bHasglobalBKGD)           /* global bKGD present ? */
    {
      pBuf->iBKGDred         = pData->iGlobalBKGDred;
      pBuf->iBKGDgreen       = pData->iGlobalBKGDgreen;
      pBuf->iBKGDblue        = pData->iGlobalBKGDblue;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_RESET_OBJECTDETAILS, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Animation-object routines                                              * */
/* *                                                                        * */
/* * these handle the animation objects used to re-run parts of a MNG.      * */
/* * eg. during LOOP or TERM processing                                     * */
/* *                                                                        * */
/* ************************************************************************** */

void add_ani_object (mng_datap          pData,
                     mng_object_headerp pObject)
{
  mng_object_headerp pLast = (mng_object_headerp)pData->pLastaniobj;

  if (pLast)
  {
    pObject->pPrev      = pLast;
    pLast->pNext        = pObject;
  }
  else
  {
    pData->pFirstaniobj = pObject;
  }

  pData->pLastaniobj    = pObject;

  return;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_image (mng_datap      pData,
                              mng_ani_imagep *ppObject)
{
  mng_ani_imagep pImage;
  mng_imagep     pCurrent = (mng_imagep)pData->pCurrentobj;
  mng_retcode    iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_IMAGE, MNG_LC_START)
#endif

  if (!pCurrent)                       /* otherwise object 0 */
    pCurrent = (mng_imagep)pData->pObjzero;
                                       /* just clone the object !!! */
  iRetcode  = clone_imageobject (pData, 0, MNG_FALSE, pCurrent->bVisible,
                                 MNG_TRUE, MNG_FALSE, 0, 0, 0, pCurrent, &pImage);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;                         

  pImage->sHeader.fCleanup = free_ani_image;
  pImage->sHeader.fProcess = process_ani_image;

  add_ani_object (pData, (mng_object_headerp)pImage);

  *ppObject = pImage;                  /* and return the new buffer */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_IMAGE, MNG_LC_END)
#endif

  return MNG_NOERROR;                  /* okido */
}

/* ************************************************************************** */

mng_retcode free_ani_image (mng_datap   pData,
                            mng_objectp pObject)
{
  mng_ani_imagep pImage = (mng_ani_imagep)pObject;
  mng_imagedatap pImgbuf = pImage->pImgbuf;
  mng_retcode    iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_IMAGE, MNG_LC_START)
#endif
                                       /* unlink the image-data buffer */
  iRetcode = free_imagedataobject (pData, pImgbuf);
                                       /* drop it's own buffer */
  MNG_FREEX (pData, pImage, sizeof (mng_ani_image))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_IMAGE, MNG_LC_END)
#endif

  return iRetcode;
}

/* ************************************************************************** */

mng_retcode process_ani_image (mng_datap   pData,
                               mng_objectp pObject)
{
  mng_retcode    iRetcode = MNG_NOERROR;
  mng_ani_imagep pImage   = (mng_imagep)pObject;
  mng_imagep     pCurrent = (mng_imagep)pData->pCurrentobj;
  mng_imagep     pObjzero = (mng_imagep)pData->pObjzero;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_IMAGE, MNG_LC_START)
#endif

  if (pCurrent)                        /* active object ? */
  {
    mng_imagedatap pBuf = pCurrent->pImgbuf;

    if (!pData->iBreakpoint)           /* don't copy it again ! */
    {
      if (pBuf->iImgdatasize)          /* buffer present in active object ? */
                                       /* then drop it */
        MNG_FREE (pData, pBuf->pImgdata, pBuf->iImgdatasize)
                                       /* now blatently copy the animation buffer */
      MNG_COPY (pBuf, pImage->pImgbuf, sizeof (mng_imagedata))

      if (pBuf->iImgdatasize)          /* sample buffer present ? */
      {                                /* then make a copy */
        MNG_ALLOC (pData, pBuf->pImgdata, pBuf->iImgdatasize)
        MNG_COPY (pBuf->pImgdata, pImage->pImgbuf->pImgdata, pBuf->iImgdatasize)
      }

      if (pBuf->iProfilesize)          /* iCCP profile present ? */
      {                                /* then make a copy */
        MNG_ALLOC (pData, pBuf->pProfile, pBuf->iProfilesize)
        MNG_COPY (pBuf->pProfile, pImage->pImgbuf->pProfile, pBuf->iProfilesize)
      }
    }  
                                       /* now go and shoot it off (if required) */
    if ((pImage->bVisible) && (pImage->bViewable))
      iRetcode = display_image (pData, pCurrent, MNG_FALSE);
  }
  else                                 /* overlay with object 0 status */
  {
    pImage->bVisible   = pObjzero->bVisible;
    pImage->bViewable  = pObjzero->bViewable;
    pImage->iPosx      = pObjzero->iPosx;
    pImage->iPosy      = pObjzero->iPosy;
    pImage->bClipped   = pObjzero->bClipped;
    pImage->iClipl     = pObjzero->iClipl;
    pImage->iClipr     = pObjzero->iClipr;
    pImage->iClipt     = pObjzero->iClipt;
    pImage->iClipb     = pObjzero->iClipb;
                                       /* so this should do the trick */
    if ((pImage->bVisible) && (pImage->bViewable))
      iRetcode = display_image (pData, pImage, MNG_FALSE);
  }

  if (!iRetcode)                       /* all's well ? */
  {
    if (pData->bTimerset)              /* timer break ? */
      pData->iBreakpoint = 99;         /* fictive number ! */
    else
      pData->iBreakpoint = 0;          /* else clear it */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_IMAGE, MNG_LC_END)
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_loop (mng_datap     pData,
                             mng_uint8     iLevel,
                             mng_uint32    iRepeatcount,
                             mng_uint8     iTermcond,
                             mng_uint32    iItermin,
                             mng_uint32    iItermax,
                             mng_uint32    iCount,
                             mng_uint32p   pSignals,
                             mng_ani_loopp *ppObject)
{
  mng_ani_loopp pLOOP;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_LOOP, MNG_LC_START)
#endif

  MNG_ALLOC (pData, pLOOP, sizeof (mng_ani_loop))

  pLOOP->sHeader.fCleanup = free_ani_loop;
  pLOOP->sHeader.fProcess = process_ani_loop;

  add_ani_object (pData, (mng_object_headerp)pLOOP);

  pLOOP->iLevel           = iLevel;
  pLOOP->iRepeatcount     = iRepeatcount;
  pLOOP->iTermcond        = iTermcond;
  pLOOP->iItermin         = iItermin;
  pLOOP->iItermax         = iItermax;
  pLOOP->iCount           = iCount;
                                       /* running counter starts with repeat_count */
  pLOOP->iRunningcount    = iRepeatcount;

  if (iCount)
  {
    MNG_ALLOC (pData, pLOOP->pSignals, (iCount << 1))
    MNG_COPY (pLOOP->pSignals, pSignals, (iCount << 1))
  }  

  *ppObject = pLOOP;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_LOOP, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_loop (mng_datap   pData,
                           mng_objectp pObject)
{
  mng_ani_loopp pLOOP = (mng_ani_loopp)pObject;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_LOOP, MNG_LC_START)
#endif

  if (pLOOP->iCount)                   /* drop signal buffer ? */
    MNG_FREEX (pData, pLOOP->pSignals, (pLOOP->iCount << 1))

  MNG_FREEX (pData, pObject, sizeof (mng_ani_loop))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_LOOP, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_loop (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_loopp pLOOP = (mng_ani_loopp)pObject;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_LOOP, MNG_LC_START)
#endif
                                       /* just reset the running counter */
  pLOOP->iRunningcount = pLOOP->iRepeatcount;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_LOOP, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_endl (mng_datap     pData,
                             mng_uint8     iLevel,
                             mng_ani_endlp *ppObject)
{
  mng_ani_endlp pENDL;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_ENDL, MNG_LC_START)
#endif

  MNG_ALLOC (pData, pENDL, sizeof (mng_ani_endl))

  pENDL->sHeader.fCleanup = free_ani_endl;
  pENDL->sHeader.fProcess = process_ani_endl;

  add_ani_object (pData, (mng_object_headerp)pENDL);

  pENDL->iLevel           = iLevel;

  *ppObject               = pENDL;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_ENDL, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_endl (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_ENDL, MNG_LC_START)
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_endl))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_ENDL, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_endl (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_endlp pENDL = (mng_ani_endlp)pObject;
  mng_ani_loopp pLOOP;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_ENDL, MNG_LC_START)
#endif

  pLOOP = pENDL->pLOOP;                /* determine matching LOOP */

  if (!pLOOP)                          /* haven't got it yet ? */
  {                                    /* go and look back in the list */
    pLOOP = (mng_ani_loopp)pENDL->sHeader.pPrev;

    while ((pLOOP) &&
           ((pLOOP->sHeader.fCleanup != free_ani_loop) ||
            (pLOOP->iLevel           != pENDL->iLevel)    ))
      pLOOP = (mng_ani_loopp)pLOOP->sHeader.pPrev;
  }
                                       /* got it now ? */
  if ((pLOOP) && (pLOOP->iLevel == pENDL->iLevel))
  {
    pENDL->pLOOP = pLOOP;              /* save for next time ! */
                                       /* decrease running counter ? */
    if ((pLOOP->iRunningcount) && (pLOOP->iRunningcount < 0x7fffffffL))
      pLOOP->iRunningcount--;

    /* TODO: we're cheating out on the termination_condition,
       iteration_min, iteration_max and possible signals;
       it's just not ready for that can of worms.... */  

    if (!pLOOP->iRunningcount)         /* reached zero ? */
    {                                  /* was this the outer LOOP ? */
      if (pLOOP == pData->pFirstaniobj)
      {                                /* then we can clean up all ani objects */
        mng_objectp       pObject = pData->pFirstaniobj;
        mng_objectp       pNext;
        mng_cleanupobject fCleanup;

        while (pObject)                /* more objects to discard ? */
        {
          pNext = ((mng_object_headerp)pObject)->pNext;
                                       /* call appropriate cleanup */
          fCleanup = ((mng_object_headerp)pObject)->fCleanup;
          fCleanup (pData, pObject);

          pObject = pNext;             /* neeeext */
        }
                                       /* indicate we left the outmost loop */
        pData->bHasLOOP     = MNG_FALSE;
        pData->pFirstaniobj = 0;       /* leave no trace !!! */
        pData->pLastaniobj  = 0;
        pData->pCurraniobj  = 0;
      }
    }
    else
    {
      if (pData->pCurraniobj)          /* was we processing objects ? */
        pData->pCurraniobj = pLOOP;    /* then restart with LOOP */
      else                             /* else restart behind LOOP !!! */
        pData->pCurraniobj = pLOOP->sHeader.pNext;
    }
  }
  else
  {

    /* TODO: error abort ??? */

  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_ENDL, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_defi (mng_datap     pData,
                             mng_ani_defip *ppObject)
{
  mng_ani_defip pDEFI;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_DEFI, MNG_LC_START)
#endif

  MNG_ALLOC (pData, pDEFI, sizeof (mng_ani_defi))

  pDEFI->sHeader.fCleanup = free_ani_defi;
  pDEFI->sHeader.fProcess = process_ani_defi;

  add_ani_object (pData, (mng_object_headerp)pDEFI);

  pDEFI->iId              = pData->iDEFIobjectid;
  pDEFI->iDonotshow       = pData->iDEFIdonotshow;
  pDEFI->iConcrete        = pData->iDEFIconcrete;
  pDEFI->bHasloca         = pData->bDEFIhasloca;
  pDEFI->iLocax           = pData->iDEFIlocax;
  pDEFI->iLocay           = pData->iDEFIlocay;
  pDEFI->bHasclip         = pData->bDEFIhasclip;
  pDEFI->iClipl           = pData->iDEFIclipl;
  pDEFI->iClipr           = pData->iDEFIclipr;
  pDEFI->iClipt           = pData->iDEFIclipt;
  pDEFI->iClipb           = pData->iDEFIclipb;

  *ppObject               = pDEFI;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_DEFI, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_defi (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_DEFI, MNG_LC_START)
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_defi))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_DEFI, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_defi (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_defip pDEFI = (mng_ani_defip)pObject;
  mng_retcode   iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_DEFI, MNG_LC_START)
#endif

  pData->iDEFIobjectid  = pDEFI->iId;
  pData->iDEFIdonotshow = pDEFI->iDonotshow;
  pData->iDEFIconcrete  = pDEFI->iConcrete;
  pData->bDEFIhasloca   = pDEFI->bHasloca;
  pData->iDEFIlocax     = pDEFI->iLocax;
  pData->iDEFIlocay     = pDEFI->iLocay;
  pData->bDEFIhasclip   = pDEFI->bHasclip;
  pData->iDEFIclipl     = pDEFI->iClipl;
  pData->iDEFIclipr     = pDEFI->iClipr;
  pData->iDEFIclipt     = pDEFI->iClipt;
  pData->iDEFIclipb     = pDEFI->iClipb;

  iRetcode = process_display_defi (pData);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_DEFI, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_basi (mng_datap     pData,
                             mng_uint16    iRed,
                             mng_uint16    iGreen,
                             mng_uint16    iBlue,
                             mng_bool      bHasalpha,
                             mng_uint16    iAlpha,
                             mng_uint8     iViewable,
                             mng_ani_basip *ppObject)
{
  mng_ani_basip pBASI;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_BASI, MNG_LC_START)
#endif

  MNG_ALLOC (pData, pBASI, sizeof (mng_ani_basi))

  pBASI->sHeader.fCleanup = free_ani_basi;
  pBASI->sHeader.fProcess = process_ani_basi;

  add_ani_object (pData, (mng_object_headerp)pBASI);

  pBASI->iRed             = iRed;
  pBASI->iGreen           = iGreen;
  pBASI->iBlue            = iBlue;
  pBASI->bHasalpha        = bHasalpha;
  pBASI->iAlpha           = iAlpha;
  pBASI->iViewable        = iViewable;

  *ppObject               = pBASI;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_BASI, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_basi (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_BASI, MNG_LC_START)
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_basi))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_BASI, MNG_LC_END)
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_basi (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_basip pBASI = (mng_ani_basip)pObject;
  mng_retcode   iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_BASI, MNG_LC_START)
#endif

  iRetcode = process_display_basi (pData, pBASI->iRed, pBASI->iGreen, pBASI->iBlue,
                                   pBASI->bHasalpha, pBASI->iAlpha, pBASI->iViewable);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_BASI, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_clon (mng_datap     pData,
                             mng_uint16    iCloneid,
                             mng_uint16    iSourceid,
                             mng_uint8     iClonetype,
                             mng_bool      bHasdonotshow,
                             mng_uint8     iDonotshow,
                             mng_uint8     iConcrete,
                             mng_bool      bHasloca,
                             mng_uint8     iLocatype,
                             mng_int32     iLocax,
                             mng_int32     iLocay,
                             mng_ani_clonp *ppObject)
{
  mng_ani_clonp pCLON;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_CLON, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pCLON, sizeof (mng_ani_clon))

  pCLON->sHeader.fCleanup = free_ani_clon;
  pCLON->sHeader.fProcess = process_ani_clon;

  add_ani_object (pData, (mng_object_headerp)pCLON);

  pCLON->iCloneid         = iCloneid;
  pCLON->iSourceid        = iSourceid;
  pCLON->iClonetype       = iClonetype;
  pCLON->bHasdonotshow    = bHasdonotshow;
  pCLON->iDonotshow       = iDonotshow;
  pCLON->iConcrete        = iConcrete;
  pCLON->bHasloca         = bHasloca;
  pCLON->iLocatype        = iLocatype;
  pCLON->iLocax           = iLocax;
  pCLON->iLocay           = iLocay;

  *ppObject               = pCLON;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_CLON, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_clon (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_CLON, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_clon))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_CLON, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_clon (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_clonp pCLON = (mng_ani_clonp)pObject;
  mng_retcode   iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_CLON, MNG_LC_START);
#endif

  iRetcode = process_display_clon (pData, pCLON->iCloneid, pCLON->iSourceid,
                                   pCLON->iClonetype, pCLON->bHasdonotshow,
                                   pCLON->iDonotshow, pCLON->iConcrete,
                                   pCLON->bHasloca, pCLON->iLocatype,
                                   pCLON->iLocax, pCLON->iLocay);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_CLON, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_back (mng_datap     pData,
                             mng_uint16    iRed,
                             mng_uint16    iGreen,
                             mng_uint16    iBlue,
                             mng_uint8     iMandatory,
                             mng_uint16    iImageid,
                             mng_uint8     iTile,
                             mng_ani_backp *ppObject)
{
  mng_ani_backp pBACK;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_BACK, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pBACK, sizeof (mng_ani_back))

  pBACK->sHeader.fCleanup = free_ani_back;
  pBACK->sHeader.fProcess = process_ani_back;

  add_ani_object (pData, (mng_object_headerp)pBACK);

  pBACK->iRed             = iRed;
  pBACK->iGreen           = iGreen;
  pBACK->iBlue            = iBlue;
  pBACK->iMandatory       = iMandatory;
  pBACK->iImageid         = iImageid;
  pBACK->iTile            = iTile;

  *ppObject               = pBACK;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_BACK, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_back (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_BACK, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_back))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_BACK, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_back (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_backp pBACK = (mng_ani_backp)pObject;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_BACK, MNG_LC_START);
#endif

  pData->iNextBACKred       = pBACK->iRed;
  pData->iNextBACKgreen     = pBACK->iGreen;
  pData->iNextBACKblue      = pBACK->iBlue;
  pData->iNextBACKmandatory = pBACK->iMandatory;
  pData->iNextBACKimageid   = pBACK->iImageid;
  pData->iNextBACKtile      = pBACK->iTile;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_BACK, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_fram (mng_datap     pData,
                             mng_uint8     iFramemode,
                             mng_uint8     iChangedelay,
                             mng_uint32    iDelay,
                             mng_uint8     iChangetimeout,
                             mng_uint32    iTimeout,
                             mng_uint8     iChangeclipping,
                             mng_uint8     iCliptype,
                             mng_int32     iClipl,
                             mng_int32     iClipr,
                             mng_int32     iClipt,
                             mng_int32     iClipb,
                             mng_ani_framp *ppObject)
{
  mng_ani_framp pFRAM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_FRAM, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pFRAM, sizeof (mng_ani_fram))

  pFRAM->sHeader.fCleanup = free_ani_fram;
  pFRAM->sHeader.fProcess = process_ani_fram;

  add_ani_object (pData, (mng_object_headerp)pFRAM);

  pFRAM->iFramemode       = iFramemode;
  pFRAM->iChangedelay     = iChangedelay;
  pFRAM->iDelay           = iDelay;
  pFRAM->iChangetimeout   = iChangetimeout;
  pFRAM->iTimeout         = iTimeout;
  pFRAM->iChangeclipping  = iChangeclipping;
  pFRAM->iCliptype        = iCliptype;
  pFRAM->iClipl           = iClipl;
  pFRAM->iClipr           = iClipr;
  pFRAM->iClipt           = iClipt;
  pFRAM->iClipb           = iClipb;

  *ppObject               = pFRAM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_FRAM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_fram (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_FRAM, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_fram))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_FRAM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_fram (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_ani_framp pFRAM = (mng_ani_framp)pObject;
  mng_retcode   iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_FRAM, MNG_LC_START);
#endif

  if (pData->iBreakpoint)              /* previously broken ? */
  {
    iRetcode           = process_display_fram2 (pData);
    pData->iBreakpoint = 0;            /* not again */
  }
  else
    iRetcode = process_display_fram (pData, pFRAM->iFramemode,
                                     pFRAM->iChangedelay, pFRAM->iDelay,
                                     pFRAM->iChangetimeout, pFRAM->iTimeout,
                                     pFRAM->iChangeclipping, pFRAM->iCliptype,
                                     pFRAM->iClipl, pFRAM->iClipr,
                                     pFRAM->iClipt, pFRAM->iClipb);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_FRAM, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_move (mng_datap     pData,
                             mng_uint16    iFirstid,
                             mng_uint16    iLastid,
                             mng_uint8     iType,
                             mng_int32     iLocax,
                             mng_int32     iLocay,
                             mng_ani_movep *ppObject)
{
  mng_ani_movep pMOVE;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_MOVE, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pMOVE, sizeof (mng_ani_move))

  pMOVE->sHeader.fCleanup = free_ani_move;
  pMOVE->sHeader.fProcess = process_ani_move;

  add_ani_object (pData, (mng_object_headerp)pMOVE);

  pMOVE->iFirstid         = iFirstid;
  pMOVE->iLastid          = iLastid;
  pMOVE->iType            = iType;
  pMOVE->iLocax           = iLocax;
  pMOVE->iLocay           = iLocay;

  *ppObject               = pMOVE;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_MOVE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_move (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_MOVE, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_move))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_MOVE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_move (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_retcode   iRetcode;
  mng_ani_movep pMOVE = (mng_ani_movep)pObject;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_MOVE, MNG_LC_START);
#endif
                                       /* re-process the MOVE chunk */
  iRetcode = process_display_move (pData, pMOVE->iFirstid, pMOVE->iLastid,
                                          pMOVE->iType,
                                          pMOVE->iLocax, pMOVE->iLocay);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_MOVE, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_clip (mng_datap     pData,
                             mng_uint16    iFirstid,
                             mng_uint16    iLastid,
                             mng_uint8     iType,
                             mng_int32     iClipl,
                             mng_int32     iClipr,
                             mng_int32     iClipt,
                             mng_int32     iClipb,
                             mng_ani_clipp *ppObject)
{
  mng_ani_clipp pCLIP;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_CLIP, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pCLIP, sizeof (mng_ani_clip))

  pCLIP->sHeader.fCleanup = free_ani_clip;
  pCLIP->sHeader.fProcess = process_ani_clip;

  add_ani_object (pData, (mng_object_headerp)pCLIP);

  pCLIP->iFirstid         = iFirstid;
  pCLIP->iLastid          = iLastid;
  pCLIP->iType            = iType;
  pCLIP->iClipl           = iClipl;
  pCLIP->iClipr           = iClipr;
  pCLIP->iClipt           = iClipt;
  pCLIP->iClipb           = iClipb;

  *ppObject               = pCLIP;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_CLIP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_clip (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_CLIP, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_clip))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_CLIP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_clip (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_retcode   iRetcode;
  mng_ani_clipp pCLIP = (mng_ani_clipp)pObject;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_CLIP, MNG_LC_START);
#endif
                                       /* re-process the CLIP chunk */
  iRetcode = process_display_clip (pData, pCLIP->iFirstid, pCLIP->iLastid,
                                          pCLIP->iType,
                                          pCLIP->iClipl, pCLIP->iClipr,
                                          pCLIP->iClipt, pCLIP->iClipb);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_CLIP, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_show (mng_datap     pData,
                             mng_uint16    iFirstid,
                             mng_uint16    iLastid,
                             mng_uint8     iMode,
                             mng_ani_showp *ppObject)
{
  mng_ani_showp pSHOW;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_SHOW, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pSHOW, sizeof (mng_ani_show))

  pSHOW->sHeader.fCleanup = free_ani_show;
  pSHOW->sHeader.fProcess = process_ani_show;

  add_ani_object (pData, (mng_object_headerp)pSHOW);

  pSHOW->iFirstid         = iFirstid;
  pSHOW->iLastid          = iLastid;
  pSHOW->iMode            = iMode;

  *ppObject               = pSHOW;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_SHOW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_show (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_SHOW, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_show))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_SHOW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_show (mng_datap   pData,
                              mng_objectp pObject)
{
  mng_retcode   iRetcode;
  mng_ani_showp pSHOW = (mng_ani_showp)pObject;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_SHOW, MNG_LC_START);
#endif

  if (pData->iBreakpoint)              /* returning from breakpoint ? */
  {
    iRetcode           = process_display_show (pData);
  }
  else
  {                                    /* "re-run" SHOW chunk */
    pData->iSHOWmode   = pSHOW->iMode;
    pData->iSHOWfromid = pSHOW->iFirstid;
    pData->iSHOWtoid   = pSHOW->iLastid;

    iRetcode           = process_display_show (pData);
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_SHOW, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */
/* ************************************************************************** */

mng_retcode create_ani_term (mng_datap     pData,
                             mng_uint8     iTermaction,
                             mng_uint8     iIteraction,
                             mng_uint32    iDelay,
                             mng_uint32    iItermax,
                             mng_ani_termp *ppObject)
{
  mng_ani_termp pTERM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_TERM, MNG_LC_START);
#endif

  MNG_ALLOC (pData, pTERM, sizeof (mng_ani_term))

  pTERM->sHeader.fCleanup = free_ani_term;
  pTERM->sHeader.fProcess = process_ani_term;

  add_ani_object (pData, (mng_object_headerp)pTERM);

  pTERM->iTermaction      = iTermaction;
  pTERM->iIteraction      = iIteraction;
  pTERM->iDelay           = iDelay;
  pTERM->iItermax         = iItermax;

  *ppObject               = pTERM;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CREATE_ANI_TERM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode free_ani_term (mng_datap   pData,
                           mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_TERM, MNG_LC_START);
#endif

  MNG_FREEX (pData, pObject, sizeof (mng_ani_term))

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_FREE_ANI_TERM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_ani_term (mng_datap   pData,
                              mng_objectp pObject)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_TERM, MNG_LC_START);
#endif

  /* dummy: no action required! */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_PROCESS_ANI_TERM, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_DISPLAY_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

