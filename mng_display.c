/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_display.c             copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Display management (implementation)                        * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the display management routines          * */
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
#include "mng_memory.h"
#include "mng_error.h"
#include "mng_trace.h"
#include "mng_zlib.h"
#include "mng_cms.h"
#include "mng_pixels.h"
#include "mng_display.h"

/* ************************************************************************** */

#ifdef MNG_INCLUDE_DISPLAY_PROCS

/* ************************************************************************** */
/* *                                                                        * */
/* * Generic display routines                                               * */
/* *                                                                        * */
/* ************************************************************************** */

void set_display_routine (mng_datap pData)
{
  switch (pData->iCanvasstyle)         /* determine display routine */
  {
    case MNG_CANVAS_RGB8    : { pData->fDisplayrow = (mng_ptr)display_rgb8;    break; }
    case MNG_CANVAS_RGBA8   : { pData->fDisplayrow = (mng_ptr)display_rgba8;   break; }
    case MNG_CANVAS_ARGB8   : { pData->fDisplayrow = (mng_ptr)display_argb8;   break; }
    case MNG_CANVAS_BGR8    : { pData->fDisplayrow = (mng_ptr)display_bgr8;    break; }
    case MNG_CANVAS_BGRA8   : { pData->fDisplayrow = (mng_ptr)display_bgra8;   break; }
    case MNG_CANVAS_ABGR8   : { pData->fDisplayrow = (mng_ptr)display_abgr8;   break; }
/*    case MNG_CANVAS_RGB16   : { pData->fDisplayrow = (mng_ptr)display_rgb16;   break; } */
/*    case MNG_CANVAS_RGBA16  : { pData->fDisplayrow = (mng_ptr)display_rgba16;  break; } */
/*    case MNG_CANVAS_ARGB16  : { pData->fDisplayrow = (mng_ptr)display_argb16;  break; } */
/*    case MNG_CANVAS_BGR16   : { pData->fDisplayrow = (mng_ptr)display_bgr16;   break; } */
/*    case MNG_CANVAS_BGRA16  : { pData->fDisplayrow = (mng_ptr)display_bgra16;  break; } */
/*    case MNG_CANVAS_ABGR16  : { pData->fDisplayrow = (mng_ptr)display_abgr16;  break; } */
/*    case MNG_CANVAS_INDEX8  : { pData->fDisplayrow = (mng_ptr)display_index8;  break; } */
/*    case MNG_CANVAS_INDEXA8 : { pData->fDisplayrow = (mng_ptr)display_indexa8; break; } */
/*    case MNG_CANVAS_AINDEX8 : { pData->fDisplayrow = (mng_ptr)display_aindex8; break; } */
/*    case MNG_CANVAS_GRAY8   : { pData->fDisplayrow = (mng_ptr)display_gray8;   break; } */
/*    case MNG_CANVAS_GRAY16  : { pData->fDisplayrow = (mng_ptr)display_gray16;  break; } */
/*    case MNG_CANVAS_GRAYA8  : { pData->fDisplayrow = (mng_ptr)display_graya8;  break; } */
/*    case MNG_CANVAS_GRAYA16 : { pData->fDisplayrow = (mng_ptr)display_graya16; break; } */
/*    case MNG_CANVAS_AGRAY8  : { pData->fDisplayrow = (mng_ptr)display_agray8;  break; } */
/*    case MNG_CANVAS_AGRAY16 : { pData->fDisplayrow = (mng_ptr)display_agray16; break; } */
/*    case MNG_CANVAS_DX15    : { pData->fDisplayrow = (mng_ptr)display_dx15;    break; } */
/*    case MNG_CANVAS_DX16    : { pData->fDisplayrow = (mng_ptr)display_dx16;    break; } */
  }

  return;
}

/* ************************************************************************** */

mng_retcode interframe_delay (mng_datap pData)
{
  mng_uint32 iWaitfor;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INTERFRAME_DELAY, MNG_LC_START);
#endif
                                       /* let the app refresh first ! */
  pData->fRefresh (((mng_handle)pData), 0, 0,
                   pData->iDataheight - 1, pData->iDatawidth - 1);
                                       /* get current tickcount */
  pData->iRuntime   = pData->fGettickcount ((mng_handle)pData);
                                       /* tickcount wrapped around ? */
  if (pData->iRuntime < pData->iStarttime)
    pData->iRuntime = pData->iRuntime + ~pData->iStarttime + 1;
  else
    pData->iRuntime = pData->iRuntime - pData->iStarttime;

  /* NOTE that a second wraparound of the tickcount will seriously f**k things
     up here! (but that's somewhere past 49 days on Windoze...) */
  /* TODO: yeah, what to do ??? */

#ifndef MNG_NEVER_DELAY                /* THIS IS FOR TESTING ONLY !!!! */
  if (pData->iTicks)                   /* what are we aiming for */
    iWaitfor        = pData->iFrametime +
                        ((1000 / pData->iTicks) * pData->iFramedelay);
  else
#endif
    iWaitfor        = pData->iFrametime;

#ifndef MNG_ALWAYS_DELAY               /* THIS IS FOR TESTING ONLY !!!! */
  if (pData->iRuntime < iWaitfor)      /* delay necessary ? */
  {                                    /* then set the timer */
    pData->fSettimer ((mng_handle)pData, iWaitfor - pData->iRuntime);
#else
    pData->fSettimer ((mng_handle)pData, 1);
#endif    
    pData->bTimerset  = MNG_TRUE;      /* and indicate so */
#ifndef MNG_ALWAYS_DELAY               /* THIS IS FOR TESTING ONLY !!!! */
  }
#endif

  pData->iFrametime = iWaitfor;        /* increase frametime in advance */

  /* note that slow systems may never catch up !!! */

  /* TODO: some provision to compensate during slow file-access (such as
     on low-bandwidth network connections);
     only necessary during read-processing! */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INTERFRAME_DELAY, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode load_bkgdlayer (mng_datap pData)
{
  mng_int32   iY;
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_LOAD_BKGDLAYER, MNG_LC_START);
#endif

  pData->iDestl   = 0;                 /* determine clipping region */
  pData->iDestt   = 0;
  pData->iDestr   = pData->iWidth;
  pData->iDestb   = pData->iHeight;

  if (pData->bFrameclipping)           /* frame clipping specified ? */
  {
    pData->iDestl = MAX_COORD (pData->iDestl,  pData->iFrameclipl);
    pData->iDestt = MAX_COORD (pData->iDestt,  pData->iFrameclipt);
    pData->iDestr = MIN_COORD (pData->iDestr,  pData->iFrameclipr);
    pData->iDestb = MIN_COORD (pData->iDestb,  pData->iFrameclipb);
  }
                                       /* anything to clear ? */
  if ((pData->iDestr >= pData->iDestl) && (pData->iDestb >= pData->iDestt))
  {
    pData->iPass       = -1;           /* these are the object's dimensions now */
    pData->iRow        = 0;
    pData->iRowinc     = 1;
    pData->iCol        = 0;
    pData->iColinc     = 1;
    pData->iRowsamples = pData->iWidth;
    pData->iRowsize    = pData->iRowsamples << 2;
    pData->bIsRGBA16   = MNG_FALSE;    /* let's keep it simple ! */
    pData->bIsOpaque   = MNG_TRUE;

    pData->iSourcel    = 0;            /* source relative to destination */
    pData->iSourcer    = pData->iDestr - pData->iDestl;
    pData->iSourcet    = 0;
    pData->iSourceb    = pData->iDestb - pData->iDestt;

    set_display_routine (pData);       /* determine display routine */
                                       /* default restore using preset BG color */
    pData->fRestbkgdrow = restore_bkgd_bgcolor;

    if (pData->fGetbkgdline)           /* background-canvas-access callback set ? */
    {
      switch (pData->iBkgdstyle)
      {
        case MNG_CANVAS_RGB8    : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_rgb8;    break; }
        case MNG_CANVAS_BGR8    : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_bgr8;    break; }
/*        case MNG_CANVAS_RGB16   : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_rgb16;   break; } */
/*        case MNG_CANVAS_BGR16   : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_bgr16;   break; } */
/*        case MNG_CANVAS_INDEX8  : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_index8;  break; } */
/*        case MNG_CANVAS_GRAY8   : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_gray8;   break; } */
/*        case MNG_CANVAS_GRAY16  : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_gray16;  break; } */
/*        case MNG_CANVAS_DX15    : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_dx15;    break; } */
/*        case MNG_CANVAS_DX16    : { pData->fRestbkgdrow = (mng_ptr)restore_bkgd_dx16;    break; } */
      }
    }

    if (pData->bHasBACK)
    {                                  /* background image ? */
      if ((pData->iBACKmandatory & 0x02) && (pData->iBACKimageid))
        pData->fRestbkgdrow = restore_bkgd_backimage;
      else                             /* background color ? */
      if (pData->iBACKmandatory & 0x01)
        pData->fRestbkgdrow = restore_bkgd_backcolor;

    }

    pData->fCorrectrow = 0;            /* default no color-correction */


    /* TODO: determine color correction; this is tricky;
       the BACK color is treated differently as the image;
       it probably requires a rewrite of the logic here... */


                                       /* get a temporary row-buffer */
    MNG_ALLOC (pData, pData->pRGBArow, pData->iRowsize)

    iY       = pData->iDestt;          /* this is where we start */
    iRetcode = MNG_NOERROR;            /* so far, so good */

    while ((!iRetcode) && (iY < pData->iDestb))
    {                                  /* restore a background row */
      iRetcode = ((mng_restbkgdrow)pData->fRestbkgdrow) (pData);
                                       /* color correction ? */
      if ((!iRetcode) && (pData->fCorrectrow))
        iRetcode = ((mng_correctrow)pData->fCorrectrow) (pData);

      if (!iRetcode)                   /* so... display it */
        iRetcode = ((mng_displayrow)pData->fDisplayrow) (pData);

      if (!iRetcode)
        next_row (pData);              /* adjust variables for next row */

      iY++;                            /* and next line */
    }
                                       /* drop the temporary row-buffer */
    MNG_FREE (pData, pData->pRGBArow, pData->iRowsize)

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_LOAD_BKGDLAYER, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode next_frame (mng_datap  pData,
                        mng_uint8  iFramemode,
                        mng_uint8  iChangedelay,
                        mng_uint32 iDelay,
                        mng_uint8  iChangetimeout,
                        mng_uint32 iTimeout,
                        mng_uint8  iChangeclipping,
                        mng_uint8  iCliptype,
                        mng_int32  iClipl,
                        mng_int32  iClipr,
                        mng_int32  iClipt,
                        mng_int32  iClipb)
{
  mng_retcode iRetcode = MNG_NOERROR;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_NEXT_FRAME, MNG_LC_START);
#endif

  if (!pData->iBreakpoint)             /* no previous break here ? */
  {                                    /* interframe delay required ? */
    if ((pData->iFrameseq) &&
        ((pData->iFramemode == 2) || (pData->iFramemode == 4)))
      iRetcode = interframe_delay (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
                                       /* now we'll assume we're in the next frame! */
    if (iFramemode)                    /* save the new framing mode ? */
    {
      pData->iFRAMmode  = iFramemode;
      pData->iFramemode = iFramemode;
    }
    else                               /* reload default */
      pData->iFramemode = pData->iFRAMmode;

    if (iChangedelay)                  /* delay changed ? */
    {                                  /* for next subframe */
      pData->iFramedelay = iDelay;

      if (iChangedelay == 2)           /* also overall ? */
        pData->iFRAMdelay = iDelay;
    }
    else                               /* reload default */
      pData->iFramedelay = pData->iFRAMdelay;

    if (iChangetimeout)                /* timeout changed ? */
    {                                  /* for next subframe */
      pData->iFrametimeout = iTimeout;

      if ((iChangetimeout == 2) ||     /* also overall ? */
          (iChangetimeout == 4) ||
          (iChangetimeout == 6) ||
          (iChangetimeout == 8))
        pData->iFRAMtimeout = iTimeout;
    }
    else                               /* reload default */
      pData->iFrametimeout = pData->iFRAMtimeout;

    if (iChangeclipping)               /* clipping changed ? */
    {
      pData->bFrameclipping = MNG_TRUE;

      if (!iCliptype)                  /* absolute ? */
      {
        pData->iFrameclipl = iClipl;
        pData->iFrameclipr = iClipr;
        pData->iFrameclipt = iClipt;
        pData->iFrameclipb = iClipb;
      }
      else                             /* relative */
      {
        pData->iFrameclipl = pData->iFrameclipl + iClipl;
        pData->iFrameclipr = pData->iFrameclipr + iClipr;
        pData->iFrameclipt = pData->iFrameclipt + iClipt;
        pData->iFrameclipb = pData->iFrameclipb + iClipb;
      }

      if (iChangeclipping == 2)        /* also overall ? */
      {
        pData->bFRAMclipping = MNG_TRUE;

        if (!iCliptype)                /* absolute ? */
        {
          pData->iFRAMclipl = iClipl;
          pData->iFRAMclipr = iClipr;
          pData->iFRAMclipt = iClipt;
          pData->iFRAMclipb = iClipb;
        }
        else                           /* relative */
        {
          pData->iFRAMclipl = pData->iFRAMclipl + iClipl;
          pData->iFRAMclipr = pData->iFRAMclipr + iClipr;
          pData->iFRAMclipt = pData->iFRAMclipt + iClipt;
          pData->iFRAMclipb = pData->iFRAMclipb + iClipb;
        }
      }
    }
    else
    {                                  /* reload defaults */
      pData->bFrameclipping = pData->bFRAMclipping;
      pData->iFrameclipl    = pData->iFRAMclipl;
      pData->iFrameclipr    = pData->iFRAMclipr;
      pData->iFrameclipt    = pData->iFRAMclipt;
      pData->iFrameclipb    = pData->iFRAMclipb;
    }
  }

  if (!pData->bTimerset)               /* timer still off ? */
  {
    if ((pData->iFramemode == 4) ||    /* insert background layer after a new frame */
        (!pData->iLayerseq))           /* and certainly before the very first layer */
      iRetcode = load_bkgdlayer (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    pData->iFrameseq++;                /* count the frame ! */
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_NEXT_FRAME, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode next_layer (mng_datap pData)
{
  mng_imagep  pImage   = (mng_imagep)pData->pCurrentobj;
  mng_retcode iRetcode = MNG_NOERROR;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_NEXT_LAYER, MNG_LC_START);
#endif

  if (!pData->iBreakpoint)             /* no previous break here ? */
  {
    if ((pData->iLayerseq) &&          /* interframe delay required ? */
        ((pData->iFramemode == 1) || (pData->iFramemode == 3)))
      iRetcode = interframe_delay (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }

  if (!pData->bTimerset)               /* timer still off ? */
  {
    if (!pData->iLayerseq)             /* restore background for the very first layer ? */
    {
      iRetcode = load_bkgdlayer (pData);
      pData->iLayerseq++;              /* and it counts as a layer then ! */
    }
    else
    if (pData->iFramemode == 3)        /* restore background for each layer ? */
      iRetcode = load_bkgdlayer (pData);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    if (!pImage)                       /* not an active object ? */
      pImage = (mng_imagep)pData->pObjzero;
                                       /* determine display rectangle */
    pData->iDestl   = MAX_COORD ((mng_int32)0,   pImage->iPosx);
    pData->iDestt   = MAX_COORD ((mng_int32)0,   pImage->iPosy);
                                       /* is it a valid buffer ? */
    if ((pImage->pImgbuf->iWidth) && (pImage->pImgbuf->iHeight))
    {
      pData->iDestr = MIN_COORD ((mng_int32)pData->iWidth,
                                 pImage->iPosx + (mng_int32)pImage->pImgbuf->iWidth );
      pData->iDestb = MIN_COORD ((mng_int32)pData->iHeight,
                                 pImage->iPosy + (mng_int32)pImage->pImgbuf->iHeight);
    }
    else                               /* it's a single image ! */
    {
      pData->iDestr = MIN_COORD ((mng_int32)pData->iWidth,
                                 (mng_int32)pData->iDatawidth );
      pData->iDestb = MIN_COORD ((mng_int32)pData->iHeight,
                                 (mng_int32)pData->iDataheight);
    }

    if (pData->bFrameclipping)         /* frame clipping specified ? */
    {
      pData->iDestl = MAX_COORD (pData->iDestl,  pData->iFrameclipl);
      pData->iDestt = MAX_COORD (pData->iDestt,  pData->iFrameclipt);
      pData->iDestr = MIN_COORD (pData->iDestr,  pData->iFrameclipr);
      pData->iDestb = MIN_COORD (pData->iDestb,  pData->iFrameclipb);
    }

    if (pImage->bClipped)              /* is the image clipped itself ? */
    {
      pData->iDestl = MAX_COORD (pData->iDestl,  pImage->iClipl);
      pData->iDestt = MAX_COORD (pData->iDestt,  pImage->iClipt);
      pData->iDestr = MIN_COORD (pData->iDestr,  pImage->iClipr);
      pData->iDestb = MIN_COORD (pData->iDestb,  pImage->iClipb);
    }
                                       /* determine source starting point */
    pData->iSourcel = MAX_COORD ((mng_int32)0,   pData->iDestl - pImage->iPosx);
    pData->iSourcet = MAX_COORD ((mng_int32)0,   pData->iDestt - pImage->iPosy);

    if ((pImage->pImgbuf->iWidth) && (pImage->pImgbuf->iHeight))
    {                                  /* and maximum size  */
      pData->iSourcer = MIN_COORD ((mng_int32)pImage->pImgbuf->iWidth,
                                   pData->iSourcel + pData->iDestr - pData->iDestl);
      pData->iSourceb = MIN_COORD ((mng_int32)pImage->pImgbuf->iHeight,
                                   pData->iSourcet + pData->iDestb - pData->iDestt);
    }
    else                               /* it's a single image ! */
    {
      pData->iSourcer = pData->iSourcel + pData->iDestr - pData->iDestl;
      pData->iSourceb = pData->iSourcet + pData->iDestb - pData->iDestt;
    }

    pData->iLayerseq++;                /* count the layer ! */
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_NEXT_LAYER, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode display_image (mng_datap  pData,
                           mng_imagep pImage)
{
  mng_imagep pSave;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_DISPLAY_IMAGE, MNG_LC_START);
#endif

  pData->pRetrieveobj = pImage;        /* so retrieve-row and color-correction can find it */

  pSave               = pData->pCurrentobj;
  pData->pCurrentobj  = pImage;
  next_layer (pData);                  /* advance to next layer */
  pData->pCurrentobj  = pSave;

  if (!pData->bTimerset)               /* all systems still go ? */
  {
    pData->iBreakpoint = 0;            /* let's make absolutely sure... */
                                       /* anything to display ? */
    if ((pData->iDestr >= pData->iDestl) && (pData->iDestb >= pData->iDestt))
    {
      mng_int32   iY;
      mng_retcode iRetcode;

      set_display_routine (pData);     /* determine display routine */
                                       /* and image-buffer retrieval routine */
      switch (pImage->pImgbuf->iColortype)
      {
        case 0 : { if (pImage->pImgbuf->iBitdepth > 8)
                     pData->fRetrieverow = (mng_ptr)retrieve_g16;
                   else
                     pData->fRetrieverow = (mng_ptr)retrieve_g8;

                   pData->bIsOpaque      = (mng_bool)(!pImage->pImgbuf->bHasTRNS);
                   break;
                 }

        case 2 : { if (pImage->pImgbuf->iBitdepth > 8)
                     pData->fRetrieverow = (mng_ptr)retrieve_rgb16;
                   else
                     pData->fRetrieverow = (mng_ptr)retrieve_rgb8;

                   pData->bIsOpaque      = (mng_bool)(!pImage->pImgbuf->bHasTRNS);
                   break;
                 }


        case 3 : { pData->fRetrieverow   = (mng_ptr)retrieve_idx8;
                   pData->bIsOpaque      = (mng_bool)(!pImage->pImgbuf->bHasTRNS);
                   break;
                 }


        case 4 : { if (pImage->pImgbuf->iBitdepth > 8)
                     pData->fRetrieverow = (mng_ptr)retrieve_ga16;
                   else
                     pData->fRetrieverow = (mng_ptr)retrieve_ga8;

                   pData->bIsOpaque      = MNG_FALSE;
                   break;
                 }


        case 6 : { if (pImage->pImgbuf->iBitdepth > 8)
                     pData->fRetrieverow = (mng_ptr)retrieve_rgba16;
                   else
                     pData->fRetrieverow = (mng_ptr)retrieve_rgba8;

                   pData->bIsOpaque      = MNG_FALSE;
                   break;
                 }

      }

      pData->iPass       = -1;         /* these are the object's dimensions now */
      pData->iRow        = pData->iSourcet;
      pData->iRowinc     = 1;
      pData->iCol        = 0;
      pData->iColinc     = 1;
      pData->iRowsamples = pImage->pImgbuf->iWidth;
      pData->iRowsize    = pData->iRowsamples << 2;
      pData->bIsRGBA16   = MNG_FALSE;
                                       /* adjust for 16-bit object ? */
      if (pImage->pImgbuf->iBitdepth > 8)
      {
        pData->bIsRGBA16 = MNG_TRUE;
        pData->iRowsize  = pData->iRowsamples << 3;
      }

      pData->fCorrectrow = 0;          /* default no color-correction */

#ifndef MNG_NO_CMS
#if defined(MNG_FULL_CMS)              /* determine color-management routine */
      iRetcode = init_full_cms_object   (pData);
#elif defined(MNG_GAMMA_ONLY)
       iRetcode = init_gamma_only_object (pData);
#elif defined(MNG_APP_CMS)
      iRetcode = init_app_cms_object    (pData);
#endif
      if (iRetcode)                    /* on error bail out */
        return iRetcode;
#endif /* !MNG_NO_CMS */
                                       /* get a temporary row-buffer */
      MNG_ALLOC (pData, pData->pRGBArow, pData->iRowsize)

      iY = pData->iSourcet;            /* this is where we start */

      while ((!iRetcode) && (iY < pData->iSourceb))
      {                                /* get a row */
        iRetcode = ((mng_retrieverow)pData->fRetrieverow) (pData);
                                       /* color correction ? */
        if ((!iRetcode) && (pData->fCorrectrow))
          iRetcode = ((mng_correctrow)pData->fCorrectrow) (pData);

        if (!iRetcode)                 /* so... display it */
          iRetcode = ((mng_displayrow)pData->fDisplayrow) (pData);

        if (!iRetcode)
          next_row (pData);            /* adjust variables for next row */

        iY++;                          /* and next line */
      }
                                       /* drop the temporary row-buffer */
      MNG_FREE (pData, pData->pRGBArow, pData->iRowsize)

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_DISPLAY_IMAGE, MNG_LC_END);
#endif

  return MNG_NOERROR;                  /* whehehe, this is good ! */
}

/* ************************************************************************** */
/* *                                                                        * */
/* * Chunk display processing routines                                      * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode process_display_ihdr (mng_datap pData)
{                                      /* address the current "object" if any */
  mng_imagep pImage = (mng_imagep)pData->pCurrentobj;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_IHDR, MNG_LC_START);
#endif
                                       /* sequence checks */
  pData->fDisplayrow = 0;              /* do nothing by default */
  pData->fCorrectrow = 0;
  pData->fStorerow   = 0;
  pData->fProcessrow = 0;
  pData->pStoreobj   = 0;

  if (!pData->iBreakpoint)             /* not previously broken ? */
  {
    mng_retcode iRetcode = MNG_NOERROR;

    if (pImage)                        /* update object buffer ? */
      iRetcode = reset_object_details (pData, pImage,
                                       pData->iDatawidth, pData->iDataheight,
                                       pData->iBitdepth, pData->iColortype,
                                       MNG_TRUE);
    else                               /* update object 0 ? */
    if (pData->eImagetype != mng_it_png)
      iRetcode = reset_object_details (pData, (mng_imagep)pData->pObjzero,
                                       pData->iDatawidth, pData->iDataheight,
                                       pData->iBitdepth, pData->iColortype,
                                       MNG_TRUE);

    if (iRetcode)                      /* on error bail out */
      return iRetcode;
  }
                                       /* do we need to store it ? */
  if (pData->eImagetype != mng_it_png)
  {
    if (pImage)                        /* real object ? */
      pData->pStoreobj = pImage;       /* tell the row routines */
    else                               /* otherwise use object 0 */
      pData->pStoreobj = pData->pObjzero;
  }
                                       /* display "on-the-fly" ? */
  if ( (pData->eImagetype == mng_it_png                       ) ||
       ((!pImage) && (((mng_imagep)pData->pObjzero)->bVisible)) ||
       ((pImage ) && (pImage->bVisible                       ))    )
  {
    next_layer (pData);                /* that's a new layer then ! */

    if (pData->bTimerset)              /* timer break ? */
      pData->iBreakpoint = 2;
    else
    {
      pData->iBreakpoint = 0;
                                       /* anything to display ? */
      if ((pData->iDestr > pData->iDestl) && (pData->iDestb > pData->iDestt))
        set_display_routine (pData);   /* then determine display routine */
    }
  }

  if (!pData->bTimerset)               /* no timer break ? */
  {
    switch (pData->iColortype)         /* determine row initialization routine */
    {
      case 0 : {                       /* gray */
                 switch (pData->iBitdepth)
                 {
                   case  1 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_g1_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_g1_i;

                               break;
                             }
                   case  2 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_g2_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_g2_i;

                               break;
                             }
                   case  4 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_g4_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_g4_i;

                               break;
                             }
                   case  8 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_g8_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_g8_i;

                               break;
                             }
                   case 16 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_g16_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_g16_i;

                               break;
                             }
                 }

                 break;
               }
      case 2 : {                       /* rgb */
                 switch (pData->iBitdepth)
                 {
                   case  8 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_rgb8_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_rgb8_i;

                               break;
                             }
                   case 16 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_rgb16_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_rgb16_i;

                               break;
                             }
                 }

                 break;
               }
      case 3 : {                       /* indexed */
                 switch (pData->iBitdepth)
                 {
                   case  1 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_idx1_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_idx1_i;

                               break;
                             }
                   case  2 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_idx2_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_idx2_i;

                               break;
                             }
                   case  4 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_idx4_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_idx4_i;

                               break;
                             }
                   case  8 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_idx8_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_idx8_i;

                               break;
                             }
                 }

                 break;
               }
      case 4 : {                       /* gray+alpha */
                 switch (pData->iBitdepth)
                 {
                   case  8 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_ga8_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_ga8_i;

                               break;
                             }
                   case 16 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_ga16_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_ga16_i;

                               break;
                             }
                 }

                 break;
               }
      case 6 : {                       /* rgb+alpha */
                 switch (pData->iBitdepth)
                 {
                   case  8 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_rgba8_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_rgba8_i;

                               break;
                             }
                   case 16 : {
                               if (!pData->iInterlace)
                                 pData->fInitrowproc = (mng_ptr)init_rgba16_ni;
                               else
                                 pData->fInitrowproc = (mng_ptr)init_rgba16_i;

                               break;
                             }
                 }

                 break;
               }
    }
  }  

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_IHDR, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_idat (mng_datap  pData,
                                  mng_uint32 iRawlen,
                                  mng_uint8p pRawdata)
{
  mng_retcode iRetcode = MNG_NOERROR;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_IDAT, MNG_LC_START);
#endif

  if (!pData->bInflating)              /* if we're not inflating already */
  {                                    /* initialize row-processing */
    iRetcode = ((mng_initrowproc)pData->fInitrowproc) (pData);

    if (!iRetcode)                     /* initialize inflate */
      iRetcode = mngzlib_inflateinit (pData);
  }

  if (!iRetcode)                       /* all ok? then inflate, my man */
    iRetcode = mngzlib_inflaterows (pData, iRawlen, pRawdata);

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_IDAT, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */

mng_retcode process_display_iend (mng_datap pData)
{
  mng_retcode iRetcode, iRetcode2;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_IEND, MNG_LC_START);
#endif

  if ((pData->bHasBASI) ||             /* was it a BASI stream */
      (pData->iBreakpoint))            /* or did we get broken last time ? */
  {
    mng_imagep pImage = (mng_imagep)pData->pCurrentobj;

    if (!pImage)                       /* or was it an "on-the-fly" image ? */
      pImage = (mng_imagep)pData->pObjzero;
                                       /* display it on the fly then ? */
    if ((pImage->bVisible) && (pImage->bViewable))
    {                                  /* so do it */
      iRetcode = display_image (pData, pImage);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;

      if (pData->bTimerset)            /* timer break ? */
        pData->iBreakpoint = 6;
    }
  }

  if (!pData->bTimerset)               /* can we continue ? */
  {
    pData->iBreakpoint = 0;            /* clear this flag now ! */
                                       /* cleanup object 0 */
    reset_object_details (pData, (mng_imagep)pData->pObjzero, 0, 0, 0, 0, MNG_TRUE);

    if (pData->bInflating)             /* if we've been inflating */
    {                                  /* cleanup row-processing, */
      iRetcode  = cleanup_rowproc (pData);
                                       /* also cleanup inflate! */
      iRetcode2 = mngzlib_inflatefree (pData);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
      if (iRetcode2)
        return iRetcode2;
    }
                                       /* if the image was displayed on the fly, */
    if (pData->fDisplayrow)            /* we'll have to make the app refresh */
      pData->fRefresh (((mng_handle)pData), 0, 0,
                       pData->iDataheight - 1, pData->iDatawidth - 1);

  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_IEND, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_mend (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_MEND, MNG_LC_START);
#endif

  if (pData->bHasTERM)                 /* TERM processed ? */
  {                                    /* gotta be the first animation object ! */
    mng_ani_termp pTERM = (mng_ani_termp)pData->pFirstaniobj;

    switch (pTERM->iTermaction)        /* determine what to do! */
    {
      case 0 : {                       /* show last frame indefinitly */
                 break;                /* piece of cake, that is... */
               }

      case 1 : {                       /* cease displaying anything */
                 pData->bFrameclipping = MNG_FALSE;
                 load_bkgdlayer (pData);
                 break;
               }

      case 2 : {                       /* show first image after TERM */

                 /* TODO: something */

                 break;
               }

      case 3 : {                       /* repeat */
                 if ((pTERM->iItermax) && (pTERM->iItermax < 0x7FFFFFFF))
                   pTERM->iItermax--;

                 if (pTERM->iItermax)  /* go back to TERM ? */
                   pData->pCurraniobj = pTERM;
                 else
                 {
                   switch (pTERM->iIteraction)
                   {
                     case 0 : {        /* show last frame indefinitly */
                                break; /* piece of cake, that is... */
                              }

                     case 1 : {        /* cease displaying anything */
                                pData->bFrameclipping = MNG_FALSE;
                                load_bkgdlayer (pData);
                                break;
                              }

                     case 2 : {        /* show first image after TERM */

                                /* TODO: something */

                                break;
                              }

                   }
                 }

                 break;
               }

    }
  }

  if (!pData->pCurraniobj)             /* always let the app refresh at the end ! */
    pData->fRefresh (((mng_handle)pData), 0, 0,
                     pData->iDataheight - 1, pData->iDatawidth - 1);

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_MEND, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_defi (mng_datap pData)
{
  mng_imagep pImage;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_DEFI, MNG_LC_START);
#endif

  if (!pData->iDEFIobjectid)           /* object id=0 ? */
  {
    pImage             = (mng_imagep)pData->pObjzero;
    pImage->bVisible   = (mng_bool)(pData->iDEFIdonotshow == 0);

    if (pData->bDEFIhasloca)           /* location info in chunk ? */
    {
      pImage->iPosx    = pData->iDEFIlocax;
      pImage->iPosy    = pData->iDEFIlocay;
    }

    if (pData->bDEFIhasclip)           /* clipping info in chunk ? */
    {
      pImage->bClipped = MNG_TRUE;
      pImage->iClipl   = pData->iDEFIclipl;
      pImage->iClipr   = pData->iDEFIclipr;
      pImage->iClipt   = pData->iDEFIclipt;
      pImage->iClipb   = pData->iDEFIclipb;
    }

    pData->pCurrentobj = 0;            /* not a real object ! */
  }
  else
  {                                    /* already exists ? */
    pImage = (mng_imagep)find_imageobject (pData, pData->iDEFIobjectid);

    if (!pImage)                       /* if not; create new */
    {
      mng_retcode iRetcode = create_imageobject (pData, pData->iDEFIobjectid,
                                                 (mng_bool)(pData->iDEFIconcrete == 1),
                                                 (mng_bool)(pData->iDEFIdonotshow == 0),
                                                 MNG_FALSE, 0, 0, 0, 0,
                                                 pData->iDEFIlocax, pData->iDEFIlocay,
                                                 pData->bDEFIhasclip,
                                                 pData->iDEFIclipl, pData->iDEFIclipr,
                                                 pData->iDEFIclipt, pData->iDEFIclipb,
                                                 &pImage);

      if (iRetcode)                    /* on error bail out */
        return iRetcode;
    }
    else
    {                                  /* exists; then set new info */
      pImage->bVisible  = (mng_bool)(pData->iDEFIdonotshow == 0);
      pImage->bViewable = MNG_FALSE;
      pImage->iPosx     = pData->iDEFIlocax;
      pImage->iPosy     = pData->iDEFIlocay;
      pImage->bClipped  = pData->bDEFIhasclip;
      pImage->iClipl    = pData->iDEFIclipl;
      pImage->iClipr    = pData->iDEFIclipr;
      pImage->iClipt    = pData->iDEFIclipt;
      pImage->iClipb    = pData->iDEFIclipb;

      pImage->pImgbuf->bConcrete = (mng_bool)(pData->iDEFIconcrete == 1);
    }

    pData->pCurrentobj = pImage;       /* others may want to know this */
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_DEFI, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_basi (mng_datap  pData,
                                  mng_uint16 iRed,
                                  mng_uint16 iGreen,
                                  mng_uint16 iBlue,
                                  mng_bool   bHasalpha,
                                  mng_uint16 iAlpha,
                                  mng_uint8  iViewable)
{                                      /* address the current "object" if any */
  mng_imagep     pImage = (mng_imagep)pData->pCurrentobj;
  mng_uint8p     pWork;
  mng_uint32     iX;
  mng_imagedatap pBuf = pImage->pImgbuf;
  mng_retcode    iRetcode;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_BASI, MNG_LC_START);
#endif

  if (!pImage)                         /* or is it an "on-the-fly" image ? */
    pImage = (mng_imagep)pData->pObjzero;

  pData->fDisplayrow = 0;              /* do nothing by default */
  pData->fCorrectrow = 0;
  pData->fStorerow   = 0;
  pData->fProcessrow = 0;
                                       /* set parms now that they're known */
  iRetcode = reset_object_details (pData, pImage, pData->iDatawidth,
                                   pData->iDataheight, pData->iBitdepth,
                                   pData->iColortype, MNG_FALSE);
  if (iRetcode)                        /* on error bail out */
    return iRetcode;
                                       /* save the viewable flag */
  pImage->bViewable = (mng_bool)(iViewable == 1);
  pImage->pImgbuf->bViewable = pImage->bViewable;
  pData->pStoreobj  = pImage;          /* let row-routines know which object */

  pWork = pBuf->pImgdata;              /* fill the object-buffer with the specified
                                          "color" sample */
  switch (pData->iColortype)           /* depending on color_type & bit_depth */
  {
    case 0 : {                         /* gray */
               if (pData->iBitdepth == 16)
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   mng_put_uint16 (pWork, iRed);
                   pWork += 2;
                 }
               }
               else
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   *pWork = (mng_uint8)iRed;
                   pWork++;
                 }
               }
                                       /* force tRNS ? */
               if ((bHasalpha) && (!iAlpha))
               {
                 pBuf->bHasTRNS  = MNG_TRUE;
                 pBuf->iTRNSgray = iRed;
               }

               break;
             }

    case 2 : {                         /* rgb */
               if (pData->iBitdepth == 16)
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   mng_put_uint16 (pWork,   iRed  );
                   mng_put_uint16 (pWork+2, iGreen);
                   mng_put_uint16 (pWork+4, iBlue );
                   pWork += 6;
                 }
               }
               else
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   *pWork     = (mng_uint8)iRed;
                   *(pWork+1) = (mng_uint8)iGreen;
                   *(pWork+2) = (mng_uint8)iBlue;
                   pWork += 3;
                 }
               }
                                       /* force tRNS ? */
               if ((bHasalpha) && (!iAlpha))
               {
                 pBuf->bHasTRNS   = MNG_TRUE;
                 pBuf->iTRNSred   = iRed;
                 pBuf->iTRNSgreen = iGreen;
                 pBuf->iTRNSblue  = iBlue;
               }

               break;
             }

    case 3 : {                         /* indexed */
               pImage->pImgbuf->bHasPLTE = MNG_TRUE;

               switch (pData->iBitdepth)
               {
                 case 1  : { pBuf->iPLTEcount =   2; break; }
                 case 2  : { pBuf->iPLTEcount =   4; break; }
                 case 4  : { pBuf->iPLTEcount =  16; break; }
                 case 8  : { pBuf->iPLTEcount = 256; break; }
                 default : { pBuf->iPLTEcount =   1; break; }
               }

               pBuf->aPLTEentries [0].iRed   = (mng_uint8)iRed;
               pBuf->aPLTEentries [0].iGreen = (mng_uint8)iGreen;
               pBuf->aPLTEentries [0].iBlue  = (mng_uint8)iBlue;

               for (iX = 1; iX < pBuf->iPLTEcount; iX++)
               {
                 pBuf->aPLTEentries [iX].iRed   = 0;
                 pBuf->aPLTEentries [iX].iGreen = 0;
                 pBuf->aPLTEentries [iX].iBlue  = 0;
               }
                                       /* force tRNS ? */
               if ((bHasalpha) && (iAlpha < 255))
               {
                 pBuf->bHasTRNS         = MNG_TRUE;
                 pBuf->iTRNScount       = 1;
                 pBuf->aTRNSentries [0] = iAlpha;
               }

               break;
             }

    case 4 : {                         /* gray+alpha */
               if (pData->iBitdepth == 16)
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   mng_put_uint16 (pWork,   iRed);
                   mng_put_uint16 (pWork+2, iAlpha);
                   pWork += 4;
                 }
               }
               else
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   *pWork     = (mng_uint8)iRed;
                   *(pWork+1) = (mng_uint8)iAlpha;
                   pWork += 2;
                 }
               }

               break;
             }

    case 6 : {                         /* rgb+alpha */
               if (pData->iBitdepth == 16)
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   mng_put_uint16 (pWork,   iRed);
                   mng_put_uint16 (pWork+2, iGreen);
                   mng_put_uint16 (pWork+4, iBlue);
                   mng_put_uint16 (pWork+6, iAlpha);
                   pWork += 8;
                 }
               }
               else
               {
                 for (iX = 0; iX < pData->iDatawidth * pData->iDataheight; iX++)
                 {
                   *pWork     = (mng_uint8)iRed;
                   *(pWork+1) = (mng_uint8)iGreen;
                   *(pWork+2) = (mng_uint8)iBlue;
                   *(pWork+3) = (mng_uint8)iAlpha;
                   pWork += 4;
                 }
               }

               break;
             }

  }

  switch (pData->iColortype)           /* determine row initialization routine */
  {                                    /* just to accomodate IDAT if it arrives */
    case 0 : {                         /* gray */
               switch (pData->iBitdepth)
               {
                 case  1 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_g1_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_g1_i;

                             break;
                           }
                 case  2 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_g2_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_g2_i;

                             break;
                           }
                 case  4 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_g4_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_g4_i;

                             break;
                           }
                 case  8 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_g8_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_g8_i;

                             break;
                           }
                 case 16 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_g16_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_g16_i;

                             break;
                           }
               }

               break;
             }
    case 2 : {                         /* rgb */
               switch (pData->iBitdepth)
               {
                 case  8 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_rgb8_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_rgb8_i;

                             break;
                           }
                 case 16 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_rgb16_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_rgb16_i;

                             break;
                           }
               }

               break;
             }
    case 3 : {                         /* indexed */
               switch (pData->iBitdepth)
               {
                 case  1 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_idx1_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_idx1_i;

                             break;
                           }
                 case  2 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_idx2_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_idx2_i;

                             break;
                           }
                 case  4 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_idx4_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_idx4_i;

                             break;
                           }
                 case  8 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_idx8_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_idx8_i;

                             break;
                           }
               }

               break;
             }
    case 4 : {                         /* gray+alpha */
               switch (pData->iBitdepth)
               {
                 case  8 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_ga8_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_ga8_i;

                             break;
                           }
                 case 16 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_ga16_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_ga16_i;

                             break;
                           }
               }

               break;
             }
    case 6 : {                         /* rgb+alpha */
               switch (pData->iBitdepth)
               {
                 case  8 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_rgba8_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_rgba8_i;

                             break;
                           }
                 case 16 : {
                             if (!pData->iInterlace)
                               pData->fInitrowproc = (mng_ptr)init_rgba16_ni;
                             else
                               pData->fInitrowproc = (mng_ptr)init_rgba16_i;

                             break;
                           }
               }

               break;
             }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_BASI, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_clon (mng_datap  pData,
                                  mng_uint16 iSourceid,
                                  mng_uint16 iCloneid,
                                  mng_uint8  iClonetype,
                                  mng_bool   bHasdonotshow,
                                  mng_uint8  iDonotshow,
                                  mng_uint8  iConcrete,
                                  mng_bool   bHasloca,
                                  mng_uint8  iLocationtype,
                                  mng_int32  iLocationx,
                                  mng_int32  iLocationy)
{
  mng_imagep  pSource, pClone;
  mng_bool    bVisible, bAbstract;
  mng_retcode iRetcode = MNG_NOERROR;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_CLON, MNG_LC_START);
#endif
                                       /* locate the source object first */
  pSource = find_imageobject (pData, iSourceid);
                                       /* check if the clone exists */
  pClone  = find_imageobject (pData, iCloneid);

  if (!pSource)                        /* source must exist ! */
    MNG_ERROR (pData, MNG_OBJECTUNKNOWN);

  if (pClone)                          /* clone must not exist ! */
    MNG_ERROR (pData, MNG_OBJECTEXISTS);

  if (bHasdonotshow)                   /* DoNotShow flag filled ? */
    bVisible = (mng_bool)(iDonotshow == 0);
  else
    bVisible = pSource->bVisible;

  bAbstract  = (mng_bool)(iConcrete == 1);

  switch (iClonetype)                  /* determine action to take */
  {
    case 0 : {                         /* full clone */
               iRetcode = clone_imageobject (pData, iCloneid, MNG_FALSE,
                                             bVisible, bAbstract, bHasloca,
                                             iLocationtype, iLocationx, iLocationy,
                                             pSource, &pClone);
               break;
             }

    case 1 : {                         /* partial clone */
               iRetcode = clone_imageobject (pData, iCloneid, MNG_TRUE,
                                             bVisible, bAbstract, bHasloca,
                                             iLocationtype, iLocationx, iLocationy,
                                             pSource, &pClone);
               break;
             }

    case 2 : {                         /* renumber object */
               iRetcode = renum_imageobject (pData, pSource, iCloneid,
                                             bVisible, bAbstract, bHasloca,
                                             iLocationtype, iLocationx, iLocationy);
               pClone   = pSource;
               break;
             }

  }

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

                                       /* display on the fly ? */
  if ((pClone->bViewable) && (pClone->bVisible))
  {
    pData->pLastclone = pClone;        /* remember in case of timer break ! */
    display_image (pData, pClone);     /* display it */

    if (pData->bTimerset)              /* timer break ? */
      pData->iBreakpoint = 5;
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_CLON, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_clon2 (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_CLON, MNG_LC_START);
#endif
                                       /* only called after timer break ! */
  display_image (pData, (mng_imagep)pData->pLastclone);
  pData->iBreakpoint = 0;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_CLON, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_disc (mng_datap   pData,
                                  mng_uint32  iCount,
                                  mng_uint16p pIds)
{
  mng_uint32 iX;
  mng_imagep pImage;
  mng_uint32 iRetcode;
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_DISC, MNG_LC_START);
#endif

  if (iCount)                          /* specific list ? */
  {
    mng_uint16p pWork = pIds;

    for (iX = 0; iX < iCount; iX++)    /* iterate the list */
    {
      pImage = find_imageobject (pData, *pWork++);

      if (pImage)                      /* found the object ? */
      {                                /* then drop it */
        iRetcode = free_imageobject (pData, pImage);

        if (iRetcode)                  /* on error bail out */
          return iRetcode;
      }
    }
  }
  else                                 /* empty: drop all un-frozen objects */
  {
    mng_imagep pNext = (mng_imagep)pData->pFirstimgobj;

    while (pNext)                      /* any left ? */
    {
      pImage = pNext;
      pNext  = pImage->sHeader.pNext;

      if (!pImage->bFrozen)            /* not frozen ? */
      {                                /* then drop it */
        iRetcode = free_imageobject (pData, pImage);

        if (iRetcode)                  /* on error bail out */
          return iRetcode;
      }
    }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_DISC, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_fram (mng_datap  pData,
                                  mng_uint8  iFramemode,
                                  mng_uint8  iChangedelay,
                                  mng_uint32 iDelay,
                                  mng_uint8  iChangetimeout,
                                  mng_uint32 iTimeout,
                                  mng_uint8  iChangeclipping,
                                  mng_uint8  iCliptype,
                                  mng_int32  iClipl,
                                  mng_int32  iClipr,
                                  mng_int32  iClipt,
                                  mng_int32  iClipb)
{
  mng_uint32 iRetcode;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_FRAM, MNG_LC_START);
#endif
                                       /* advance a frame then */
  iRetcode = next_frame (pData, iFramemode, iChangedelay, iDelay,
                         iChangetimeout, iTimeout, iChangeclipping,
                         iCliptype, iClipl, iClipr, iClipt, iClipb);

  if (pData->bTimerset)                /* timer break ? */
    pData->iBreakpoint = 1;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_FRAM, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */

mng_retcode process_display_fram2 (mng_datap  pData)
{
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_FRAM, MNG_LC_START);
#endif
                                       /* again; after the break */
  iRetcode = next_frame (pData, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  pData->iBreakpoint = 0;              /* not again! */

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_FRAM, MNG_LC_END);
#endif

  return iRetcode;
}

/* ************************************************************************** */

mng_retcode process_display_move (mng_datap  pData,
                                  mng_uint16 iFromid,
                                  mng_uint16 iToid,
                                  mng_uint8  iMovetype,
                                  mng_int32  iMovex,
                                  mng_int32  iMovey)
{
  mng_uint16 iX;
  mng_imagep pImage;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_MOVE, MNG_LC_START);
#endif
                                       /* iterate the list */
  for (iX = iFromid; iX <= iToid; iX++)
  {
    if (!iX)                           /* object id=0 ? */
      pImage = (mng_imagep)pData->pObjzero;
    else
      pImage = find_imageobject (pData, iX);

    if (pImage)                        /* object exists ? */
    {
      switch (iMovetype)
      {
        case 0 : {                     /* absolute */
                   pImage->iPosx = iMovex;
                   pImage->iPosy = iMovey;
                   break;
                 }
        case 1 : {                     /* relative */
                   pImage->iPosx = pImage->iPosx + iMovex;
                   pImage->iPosy = pImage->iPosy + iMovey;
                   break;
                 }
      }
    }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_MOVE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_clip (mng_datap  pData,
                                  mng_uint16 iFromid,
                                  mng_uint16 iToid,
                                  mng_uint8  iCliptype,
                                  mng_int32  iClipl,
                                  mng_int32  iClipr,
                                  mng_int32  iClipt,
                                  mng_int32  iClipb)
{
  mng_uint16 iX;
  mng_imagep pImage;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_CLIP, MNG_LC_START);
#endif
                                       /* iterate the list */
  for (iX = iFromid; iX <= iToid; iX++)
  {
    if (!iX)                           /* object id=0 ? */
      pImage = (mng_imagep)pData->pObjzero;
    else
      pImage = find_imageobject (pData, iX);

    if (pImage)                        /* object exists ? */
    {
      switch (iCliptype)
      {
        case 0 : {                     /* absolute */
                   pImage->bClipped = MNG_TRUE;
                   pImage->iClipl   = iClipl;
                   pImage->iClipr   = iClipr;
                   pImage->iClipt   = iClipt;
                   pImage->iClipb   = iClipb;
                   break;
                 }
        case 1 : {                    /* relative */
                   pImage->bClipped = MNG_TRUE;
                   pImage->iClipl   = pImage->iClipl + iClipl;
                   pImage->iClipr   = pImage->iClipr + iClipr;
                   pImage->iClipt   = pImage->iClipt + iClipt;
                   pImage->iClipb   = pImage->iClipb + iClipb;
                   break;
                 }
      }
    }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_CLIP, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode process_display_show (mng_datap pData)
{
  mng_int16  iX, iS, iFrom, iTo;
  mng_imagep pImage;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_SHOW, MNG_LC_START);
#endif

  /* TODO: optimization for the cases where "abs (iTo - iFrom)" is rather high;
     especially where ((iFrom==1) && (iTo==65535)); eg. an empty SHOW !!! */

  if (pData->iBreakpoint == 3)         /* previously broken during cycle-mode ? */
  {
    pImage = find_imageobject (pData, pData->iSHOWnextid);

    if (pImage)                        /* still there ? */
      display_image (pData, pImage);

    pData->iBreakpoint = 0;            /* let's not go through this again! */
  }
  else
  {
    if (pData->iBreakpoint)            /* previously broken at other point ? */
    {                                  /* restore last parms */
      iFrom = (mng_int16)pData->iSHOWfromid;
      iTo   = (mng_int16)pData->iSHOWtoid;
      iX    = (mng_int16)pData->iSHOWnextid;
      iS    = (mng_int16)pData->iSHOWskip;
    }
    else
    {                                  /* regular sequence ? */
      if (pData->iSHOWtoid >= pData->iSHOWfromid)
        iS  = 1;
      else                             /* reverse sequence ! */
        iS  = -1;

      iFrom = (mng_int16)pData->iSHOWfromid;
      iTo   = (mng_int16)pData->iSHOWtoid;
      iX    = iFrom;
      
      pData->iSHOWfromid = (mng_uint16)iFrom;
      pData->iSHOWtoid   = (mng_uint16)iTo;
      pData->iSHOWskip   = iS;
    }
                                       /* cycle mode ? */
    if ((pData->iSHOWmode == 6) || (pData->iSHOWmode == 7))
    {
      mng_uint16 iTrigger = 0;
      mng_uint16 iFound   = 0;
      mng_uint16 iPass    = 0;
      mng_imagep pFound   = 0;

      do
      {
        iPass++;                       /* lets prevent endless loops when there
                                          are no potential candidates in the list! */

        if (iS > 0)                    /* forward ? */
        {
          for (iX = iFrom; iX <= iTo; iX += iS)
          {
            pImage = find_imageobject (pData, (mng_uint16)iX);

            if (pImage)                  /* object exists ? */
            {
              if (iFound)                /* already found a candidate ? */
                pImage->bVisible = MNG_FALSE;
              else
              if (iTrigger)              /* found the trigger ? */
              {
                pImage->bVisible = MNG_TRUE;
                iFound           = iX;
                pFound           = pImage;
              }
              else
              if (pImage->bVisible)      /* ok, this is the trigger */
              {
                pImage->bVisible = MNG_FALSE;
                iTrigger         = iX;
              }
            }
          }
        }
        else
        {
          for (iX = iFrom; iX >= iTo; iX += iS)
          {
            pImage = find_imageobject (pData, (mng_uint16)iX);

            if (pImage)                  /* object exists ? */
            {
              if (iFound)                /* already found a candidate ? */
                pImage->bVisible = MNG_FALSE;
              else
              if (iTrigger)              /* found the trigger ? */
              {
                pImage->bVisible = MNG_TRUE;
                iFound           = iX;
                pFound           = pImage;
              }
              else
              if (pImage->bVisible)      /* ok, this is the trigger */
              {
                pImage->bVisible = MNG_FALSE;
                iTrigger         = iX;
              }
            }
          }
        }

        if (!iTrigger)                 /* did not find a trigger ? */
          iTrigger = 1;                /* then fake it so the first image
                                          gets nominated */
      }                                /* cycle back to beginning ? */
      while ((iPass < 2) && (iTrigger) && (!iFound));

      pData->iBreakpoint = 0;          /* just a sanity precaution */
                                       /* display it ? */
      if ((pData->iSHOWmode == 6) && (pFound))
      {
        display_image (pData, pFound);

        if (pData->bTimerset)          /* timer set ? */
        {
          pData->iBreakpoint = 3;
          pData->iSHOWnextid = iFound; /* save it for after the break */
        }
      }
    }
    else
    {
      do
      {
        pImage = find_imageobject (pData, iX);

        if (pImage)                    /* object exists ? */
        {
          if (pData->iBreakpoint)      /* did we get broken last time ? */
          {                            /* could only happen in the display routine */
            display_image (pData, pImage);
            pData->iBreakpoint = 0;    /* only once inside this loop please ! */
          }
          else
          {
            switch (pData->iSHOWmode)  /* do what ? */
            {
              case 0 : {
                         pImage->bVisible = MNG_TRUE;
                         display_image (pData, pImage);
                         break;
                       }
              case 1 : {
                         pImage->bVisible = MNG_FALSE;
                         break;
                       }
              case 2 : {
                         if (pImage->bVisible)
                           display_image (pData, pImage);
                         break;
                       }
              case 3 : {
                         pImage->bVisible = MNG_TRUE;
                         break;
                       }
              case 4 : {
                         pImage->bVisible = (mng_bool)(!pImage->bVisible);
                         if (pImage->bVisible)
                           display_image (pData, pImage);
                         break;
                       }
              case 5 : {
                         pImage->bVisible = (mng_bool)(!pImage->bVisible);
                       }
            }
          }
        }

        if (!pData->bTimerset)         /* next ? */
          iX += iS;

      }                                /* continue ? */
      while ((!pData->bTimerset) && (((iS > 0) && (iX <= iTo)) ||
                                     ((iS < 0) && (iX >= iTo))    ));

      if (pData->bTimerset)            /* timer set ? */
      {
        pData->iBreakpoint = 4;
        pData->iSHOWnextid = iX;       /* save for next time */
      }
      else
        pData->iBreakpoint = 0;
        
    }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_DISPLAY_SHOW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_DISPLAY_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

