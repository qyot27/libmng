/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_cms.c                 copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : color management routines (implementation)                 * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the color management routines            * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#include "libmng.h"
#include "mng_data.h"
#include "mng_objects.h"
#include "mng_error.h"
#include "mng_trace.h"
#include "mng_cms.h"

#if defined(MNG_FULL_CMS) || defined(MNG_GAMMA_ONLY)
#include <math.h>
#endif

/* ************************************************************************** */

#ifdef MNG_INCLUDE_DISPLAY_PROCS

/* ************************************************************************** */
/* *                                                                        * */
/* * Little CMS helper routines                                             * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_LCMS

/* ************************************************************************** */

void mnglcms_initlibrary ()
{
  cmsErrorAction (LCMS_ERROR_IGNORE);  /* LCMS should ignore errors! */
}

/* ************************************************************************** */

mng_cmsprof mnglcms_createfileprofile (mng_pchar zFilename)
{
  return cmsOpenProfileFromFile (zFilename, "r");
}

/* ************************************************************************** */

void mnglcms_freeprofile (mng_cmsprof hProf)
{
  cmsCloseProfile (hProf);
  return;
}

/* ************************************************************************** */

void mnglcms_freetransform (mng_cmstrans hTrans)
{
  cmsCloseProfile (hTrans);
  return;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_LCMS */

/* ************************************************************************** */
/* *                                                                        * */
/* * Color-management initialization & correction routines                  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef MNG_INCLUDE_LCMS

#define MNG_CMS_FLAGS 0

mng_retcode init_full_cms (mng_datap pData)
{
  mng_cmsprof    hProf;
  mng_cmstrans   hTrans;
  mng_imagedatap pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_FULL_CMS, MNG_LC_START);
#endif

  if ((pBuf->bHasICCP) || (pData->bHasglobalICCP))
  {
    if (!pData->hProf2)                /* output profile defined ? */
      MNG_ERROR (pData, MNG_NOOUTPUTPROFILE);

    if (pBuf->bHasICCP)                /* generate a profile handle */
      hProf = cmsOpenProfileFromMem (pBuf->pProfile, pBuf->iProfilesize);
    else
      hProf = cmsOpenProfileFromMem (pData->pGlobalProfile, pData->iGlobalProfilesize);

    pData->hProf1 = hProf;             /* save for future use */

    if (!hProf)                        /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOHANDLE)

    if (pData->bIsRGBA16)              /* 16-bit intermediates ? */
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_16_SE,
                                   pData->hProf2, TYPE_RGBA_16_SE,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);
    else
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_8,
                                   pData->hProf2, TYPE_RGBA_8,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);

    pData->hTrans = hTrans;            /* save for future use */

    if (!hTrans)                       /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOTRANS)
                                       /* load color-correction routine */
    pData->fCorrectrow = (mng_ptr)correct_full_cms;

    return MNG_NOERROR;                /* and done */
  }

  if ((pBuf->bHasSRGB) || (pData->bHasglobalSRGB))
  {
    mng_uint8 iIntent;

    if (pData->bIssRGB)                /* sRGB system ? */
      return MNG_NOERROR;              /* no conversion required */

    if (!pData->hProf3)                /* sRGB profile defined ? */
      MNG_ERROR (pData, MNG_NOSRGBPROFILE)

    hProf = pData->hProf3;             /* convert from sRGB profile */

    if (pBuf->bHasSRGB)                /* determine rendering intent */
      iIntent = pBuf->iRenderingintent;
    else
      iIntent = pData->iGlobalRendintent;

    if (pData->bIsRGBA16)              /* 16-bit intermediates ? */
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_16_SE,
                                   pData->hProf2, TYPE_RGBA_16_SE,
                                   iIntent, MNG_CMS_FLAGS);
    else
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_8,
                                   pData->hProf2, TYPE_RGBA_8,
                                   iIntent, MNG_CMS_FLAGS);

    pData->hTrans = hTrans;            /* save for future use */

    if (!hTrans)                       /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOTRANS)
                                       /* load color-correction routine */
    pData->fCorrectrow = (mng_ptr)correct_full_cms;

    return MNG_NOERROR;                /* and done */
  }

  if ( ((pBuf->bHasCHRM) || (pData->bHasglobalCHRM)) &&
       ((pBuf->bHasGAMA) || (pData->bHasglobalGAMA))    )
  {
    mng_CIExyY       sWhitepoint;
    mng_CIExyYTRIPLE sPrimaries;
    mng_gammatabp    pGammatable[3];
    mng_float        dGamma;

    if (pBuf->bHasCHRM)                /* local cHRM ? */
    {
      sWhitepoint.x      = (mng_float)pBuf->iWhitepointx   / 100000;
      sWhitepoint.y      = (mng_float)pBuf->iWhitepointy   / 100000;
      sPrimaries.Red.x   = (mng_float)pBuf->iPrimaryredx   / 100000;
      sPrimaries.Red.y   = (mng_float)pBuf->iPrimaryredy   / 100000;
      sPrimaries.Green.x = (mng_float)pBuf->iPrimarygreenx / 100000;
      sPrimaries.Green.y = (mng_float)pBuf->iPrimarygreeny / 100000;
      sPrimaries.Blue.x  = (mng_float)pBuf->iPrimarybluex  / 100000;
      sPrimaries.Blue.y  = (mng_float)pBuf->iPrimarybluey  / 100000;
    }
    else
    {
      sWhitepoint.x      = (mng_float)pData->iGlobalWhitepointx   / 100000;
      sWhitepoint.y      = (mng_float)pData->iGlobalWhitepointy   / 100000;
      sPrimaries.Red.x   = (mng_float)pData->iGlobalPrimaryredx   / 100000;
      sPrimaries.Red.y   = (mng_float)pData->iGlobalPrimaryredy   / 100000;
      sPrimaries.Green.x = (mng_float)pData->iGlobalPrimarygreenx / 100000;
      sPrimaries.Green.y = (mng_float)pData->iGlobalPrimarygreeny / 100000;
      sPrimaries.Blue.x  = (mng_float)pData->iGlobalPrimarybluex  / 100000;
      sPrimaries.Blue.y  = (mng_float)pData->iGlobalPrimarybluey  / 100000;
    }

    sWhitepoint.Y      =               /* Y component is always 1.0 */
    sPrimaries.Red.Y   =
    sPrimaries.Green.Y =
    sPrimaries.Blue.Y  = 1.0;

    if (pBuf->bHasGAMA)                /* get the gamma value */
      dGamma = (mng_float)pBuf->iGamma / 100000;
    else
      dGamma = (mng_float)pData->iGlobalGamma / 100000;

/*    dGamma = pData->dViewgamma / (dGamma * pData->dDisplaygamma); ??? */
    dGamma = pData->dViewgamma / dGamma;

    pGammatable [0] =                  /* and build the lookup tables */
    pGammatable [1] =
    pGammatable [2] = cmsBuildGamma (256, dGamma);

    if (!pGammatable [0] || !pGammatable [1] || !pGammatable [2])
      MNG_ERRORL (pData, MNG_LCMS_NOMEM)
                                       /* create the profile */
    hProf = cmsCreateRGBProfile (&sWhitepoint, &sPrimaries, pGammatable);

/*    cmsFreeGamma (pGammatable [0]); */   /* free the temporary gamma tables ??? */
/*    cmsFreeGamma (pGammatable [1]); */   /* appearantly not !?! */
/*    cmsFreeGamma (pGammatable [2]); */

    pData->hProf1 = hProf;             /* save for future use */

    if (!hProf)                        /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOHANDLE)

    if (pData->bIsRGBA16)              /* 16-bit intermediates ? */
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_16_SE,
                                   pData->hProf2, TYPE_RGBA_16_SE,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);
    else
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_8,
                                   pData->hProf2, TYPE_RGBA_8,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);

    pData->hTrans = hTrans;            /* save for future use */

    if (!hTrans)                       /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOTRANS)
                                       /* load color-correction routine */
    pData->fCorrectrow = (mng_ptr)correct_full_cms;

    return MNG_NOERROR;                /* and done */
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_FULL_CMS, MNG_LC_END);
#endif

  return init_gamma_only (pData);      /* if we get here, we'll only do gamma */
}
#endif /* MNG_INCLUDE_LCMS */

/* ************************************************************************** */

#ifdef MNG_INCLUDE_LCMS

#define MNG_CMS_FLAGS 0

mng_retcode init_full_cms_object (mng_datap pData)
{
  mng_cmsprof    hProf;
  mng_cmstrans   hTrans;
  mng_imagedatap pBuf;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_FULL_CMS_OBJ, MNG_LC_START);
#endif
                                       /* address the object-buffer */
  pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;

  if (pBuf->bHasICCP)
  {
    if (!pData->hProf2)                /* output profile defined ? */
      MNG_ERROR (pData, MNG_NOOUTPUTPROFILE);

                                       /* generate a profile handle */
    hProf = cmsOpenProfileFromMem (pBuf->pProfile, pBuf->iProfilesize);

    pData->hProf1 = hProf;             /* save for future use */

    if (!hProf)                        /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOHANDLE)

    if (pData->bIsRGBA16)              /* 16-bit intermediates ? */
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_16_SE,
                                   pData->hProf2, TYPE_RGBA_16_SE,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);
    else
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_8,
                                   pData->hProf2, TYPE_RGBA_8,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);

    pData->hTrans = hTrans;            /* save for future use */

    if (!hTrans)                       /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOTRANS)
                                       /* load color-correction routine */
    pData->fCorrectrow = (mng_ptr)correct_full_cms;

    return MNG_NOERROR;                /* and done */
  }

  if (pBuf->bHasSRGB)
  {
    if (pData->bIssRGB)                /* sRGB system ? */
      return MNG_NOERROR;              /* no conversion required */

    if (!pData->hProf3)                /* sRGB profile defined ? */
      MNG_ERROR (pData, MNG_NOSRGBPROFILE)

    hProf = pData->hProf3;             /* convert from sRGB profile */

    if (pData->bIsRGBA16)              /* 16-bit intermediates ? */
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_16_SE,
                                   pData->hProf2, TYPE_RGBA_16_SE,
                                   pBuf->iRenderingintent, MNG_CMS_FLAGS);
    else
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_8,
                                   pData->hProf2, TYPE_RGBA_8,
                                   pBuf->iRenderingintent, MNG_CMS_FLAGS);

    pData->hTrans = hTrans;            /* save for future use */

    if (!hTrans)                       /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOTRANS)
                                       /* load color-correction routine */
    pData->fCorrectrow = (mng_ptr)correct_full_cms;

    return MNG_NOERROR;                /* and done */
  }

  if ((pBuf->bHasCHRM) && (pBuf->bHasGAMA))
  {
    mng_CIExyY       sWhitepoint;
    mng_CIExyYTRIPLE sPrimaries;
    mng_gammatabp    pGammatable[3];
    mng_float        dGamma;

    sWhitepoint.x      = (mng_float)pBuf->iWhitepointx   / 100000;
    sWhitepoint.y      = (mng_float)pBuf->iWhitepointy   / 100000;
    sPrimaries.Red.x   = (mng_float)pBuf->iPrimaryredx   / 100000;
    sPrimaries.Red.y   = (mng_float)pBuf->iPrimaryredy   / 100000;
    sPrimaries.Green.x = (mng_float)pBuf->iPrimarygreenx / 100000;
    sPrimaries.Green.y = (mng_float)pBuf->iPrimarygreeny / 100000;
    sPrimaries.Blue.x  = (mng_float)pBuf->iPrimarybluex  / 100000;
    sPrimaries.Blue.y  = (mng_float)pBuf->iPrimarybluey  / 100000;

    sWhitepoint.Y      =               /* Y component is always 1.0 */
    sPrimaries.Red.Y   =
    sPrimaries.Green.Y =
    sPrimaries.Blue.Y  = 1.0;

    dGamma = (mng_float)pBuf->iGamma / 100000;

/*    dGamma = pData->dViewgamma / (dGamma * pData->dDisplaygamma); ??? */
    dGamma = pData->dViewgamma / dGamma;

    pGammatable [0] =                  /* and build the lookup tables */
    pGammatable [1] =
    pGammatable [2] = cmsBuildGamma (256, dGamma);

    if (!pGammatable [0] || !pGammatable [1] || !pGammatable [2])
      MNG_ERRORL (pData, MNG_LCMS_NOMEM)

                                       /* create the profile */
    hProf = cmsCreateRGBProfile (&sWhitepoint, &sPrimaries, pGammatable);

/*    cmsFreeGamma (pGammatable [0]); */   /* free the temporary gamma tables ??? */
/*    cmsFreeGamma (pGammatable [1]); */   /* appearantly not !?! */
/*    cmsFreeGamma (pGammatable [2]); */

    pData->hProf1 = hProf;             /* save for future use */

    if (!hProf)                        /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOHANDLE)

    if (pData->bIsRGBA16)              /* 16-bit intermediates ? */
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_16_SE,
                                   pData->hProf2, TYPE_RGBA_16_SE,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);
    else
      hTrans = cmsCreateTransform (hProf,         TYPE_RGBA_8,
                                   pData->hProf2, TYPE_RGBA_8,
                                   INTENT_PERCEPTUAL, MNG_CMS_FLAGS);

    pData->hTrans = hTrans;            /* save for future use */

    if (!hTrans)                       /* handle error ? */
      MNG_ERRORL (pData, MNG_LCMS_NOTRANS)
                                       /* load color-correction routine */
    pData->fCorrectrow = (mng_ptr)correct_full_cms;

    return MNG_NOERROR;                /* and done */
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_FULL_CMS_OBJ, MNG_LC_END);
#endif
                                       /* if we get here, we'll only do gamma */
  return init_gamma_only_object (pData);
}
#endif /* MNG_INCLUDE_LCMS */

/* ************************************************************************** */

#ifdef MNG_INCLUDE_LCMS
mng_retcode correct_full_cms (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_CORRECT_FULL_CMS, MNG_LC_START);
#endif

  cmsDoTransform (pData->hTrans, pData->pRGBArow, pData->pRGBArow, pData->iRowsamples);

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_CORRECT_FULL_CMS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_INCLUDE_LCMS */

/* ************************************************************************** */

#if defined(MNG_GAMMA_ONLY) || defined(MNG_FULL_CMS)
mng_retcode init_gamma_only (mng_datap pData)
{
  mng_float      dGamma;
  mng_imagedatap pBuf = ((mng_imagep)pData->pObjzero)->pImgbuf;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_GAMMA_ONLY, MNG_LC_START);
#endif

  if (pBuf->bHasGAMA)                  /* get the gamma value */
    dGamma = (mng_float)pBuf->iGamma / 100000;
  else
  if (pData->bHasglobalGAMA)
    dGamma = (mng_float)pData->iGlobalGamma / 100000;
  else
    dGamma = pData->dDfltimggamma;

  if (dGamma)                          /* lets not divide by zero, shall we... */
    dGamma = pData->dViewgamma / (dGamma * pData->dDisplaygamma);

  if (dGamma != pData->dLastgamma)     /* lookup table needs to be computed ? */
  {
    mng_int32 iX;

    pData->aGammatab [0] = 0;

    for (iX = 1; iX <= 255; iX++)
      pData->aGammatab [iX] = pow (iX / 255.0, dGamma) * 255 + 0.5;

    pData->dLastgamma = dGamma;        /* keep for next time */
  }
                                       /* load color-correction routine */
  pData->fCorrectrow = (mng_ptr)correct_gamma_only;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_GAMMA_ONLY, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_GAMMA_ONLY || MNG_FULL_CMS */

/* ************************************************************************** */

#if defined(MNG_GAMMA_ONLY) || defined(MNG_FULL_CMS)
mng_retcode init_gamma_only_object (mng_datap pData)
{
  mng_float      dGamma;
  mng_imagedatap pBuf;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_GAMMA_ONLY_OBJ, MNG_LC_START);
#endif
                                       /* address the object-buffer */
  pBuf = ((mng_imagep)pData->pRetrieveobj)->pImgbuf;

  if (pBuf->bHasGAMA)                  /* get the gamma value */
    dGamma = (mng_float)pBuf->iGamma / 100000;
  else
    dGamma = pData->dDfltimggamma;

  if (dGamma)                          /* lets not divide by zero, shall we... */
    dGamma = pData->dViewgamma / (dGamma * pData->dDisplaygamma);

  if (dGamma != pData->dLastgamma)     /* lookup table needs to be computed ? */
  {
    mng_int32 iX;

    pData->aGammatab [0] = 0;

    for (iX = 1; iX <= 255; iX++)
      pData->aGammatab [iX] = pow (iX / 255.0, dGamma) * 255 + 0.5;

    pData->dLastgamma = dGamma;        /* keep for next time */
  }
                                       /* load color-correction routine */
  pData->fCorrectrow = (mng_ptr)correct_gamma_only;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_GAMMA_ONLY_OBJ, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_GAMMA_ONLY || MNG_FULL_CMS */

/* ************************************************************************** */

#if defined(MNG_GAMMA_ONLY) || defined(MNG_FULL_CMS)
mng_retcode correct_gamma_only (mng_datap pData)
{
  mng_uint8p pWork;
  mng_int32  iX;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_CORRECT_GAMMA_ONLY, MNG_LC_START);
#endif

  pWork = pData->pRGBArow;             /* address intermediate row */

  if (pData->bIsRGBA16)                /* 16-bit intermediate row ? */
  {

  
     /* TODO: 16-bit precision gamma processing */
     /* we'll just do the high-order byte for now */

     
                                       /* convert all samples in the row */
     for (iX = 0; iX < pData->iRowsamples; iX++)
     {                                 /* using the precalculated gamma lookup table */
       *pWork     = pData->aGammatab [*pWork];
       *(pWork+2) = pData->aGammatab [*(pWork+2)];
       *(pWork+4) = pData->aGammatab [*(pWork+4)];

       pWork += 8;
     }
  }
  else
  {                                    /* convert all samples in the row */
     for (iX = 0; iX < pData->iRowsamples; iX++)
     {                                 /* using the precalculated gamma lookup table */
       *pWork     = pData->aGammatab [*pWork];
       *(pWork+1) = pData->aGammatab [*(pWork+1)];
       *(pWork+2) = pData->aGammatab [*(pWork+2)];

       pWork += 4;
     }
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_CORRECT_GAMMA_ONLY, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_GAMMA_ONLY || MNG_FULL_CMS */

/* ************************************************************************** */

#ifdef MNG_APP_CMS
mng_retcode init_app_cms (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_APP_CMS, MNG_LC_START);
#endif


  /* TODO: something */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_APP_CMS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_APP_CMS */

/* ************************************************************************** */

#ifdef MNG_APP_CMS
mng_retcode init_app_cms_object (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_APP_CMS_OBJ, MNG_LC_START);
#endif


  /* TODO: something */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_INIT_APP_CMS_OBJ, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_APP_CMS */

/* ************************************************************************** */

#ifdef MNG_APP_CMS
mng_retcode correct_app_cms (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_CORRECT_APP_CMS, MNG_LC_START);
#endif


  /* TODO: something */


#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_CORRECT_APP_CMS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_APP_CMS */

/* ************************************************************************** */

#endif /* MNG_INCLUDE_DISPLAY_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */



