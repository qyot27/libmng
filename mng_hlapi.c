/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_hlapi.c               copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.1                                                      * */
/* *                                                                        * */
/* * purpose   : high-level application API (implementation)                * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the high-level function interface        * */
/* *             for applications.                                          * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/06/2000 - G.Juyn                                * */
/* *             - added init of iPLTEcount                                 * */
/* *             0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - changed calling-convention definition                    * */
/* *             - changed status-handling of display-routines              * */
/* *             - added versioning-control routines                        * */
/* *             - filled the write routine                                 * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/11/2000 - G.Juyn                                * */
/* *             - added callback error-reporting support                   * */
/* *             0.5.1 - 05/12/2000 - G.Juyn                                * */
/* *             - changed trace to macro for callback error-reporting      * */
/* *             0.5.1 - 05/13/2000 - G.Juyn                                * */
/* *             - added eMNGma hack (will be removed in 1.0.0 !!!)         * */
/* *             - added TERM animation object pointer (easier reference)   * */
/* *             0.5.1 - 05/14/2000 - G.Juyn                                * */
/* *             - added cleanup of saved-data (SAVE/SEEK processing)       * */
/* *             0.5.1 - 05/16/2000 - G.Juyn                                * */
/* *             - moved the actual write_graphic functionality from here   * */
/* *               to it's appropriate function in the mng_write module     * */
/* *                                                                        * */
/* *             0.5.2 - 05/19/2000 - G.Juyn                                * */
/* *             - cleaned up some code regarding mixed support             * */
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
#include "mng_object_prc.h"
#include "mng_chunks.h"
#include "mng_memory.h"
#include "mng_read.h"
#include "mng_write.h"
#include "mng_display.h"
#include "mng_zlib.h"
#include "mng_jpeg.h"
#include "mng_cms.h"
#include "mng_pixels.h"

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

/* ************************************************************************** */
/* *                                                                        * */
/* * local routines                                                         * */
/* *                                                                        * */
/* ************************************************************************** */

#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_INCLUDE_LCMS)
mng_retcode mng_clear_cms (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLEAR_CMS, MNG_LC_START);
#endif

  if (pData->hTrans)                   /* transformation still active ? */
    mnglcms_freetransform (pData->hTrans);

  pData->hTrans = 0;

  if (pData->hProf1)                   /* file profile still active ? */
    mnglcms_freeprofile (pData->hProf1);

  pData->hProf1 = 0;  

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_CLEAR_CMS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY && MNG_INCLUDE_LCMS */

/* ************************************************************************** */

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
mng_retcode mng_drop_chunks (mng_datap pData)
{
  mng_chunkp       pChunk;
  mng_chunkp       pNext;
  mng_cleanupchunk fCleanup;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DROP_CHUNKS, MNG_LC_START);
#endif

  pChunk = pData->pFirstchunk;         /* and get first stored chunk (if any) */

  while (pChunk)                       /* more chunks to discard ? */
  {
    pNext = ((mng_chunk_headerp)pChunk)->pNext;
                                       /* call appropriate cleanup */
    fCleanup = ((mng_chunk_headerp)pChunk)->fCleanup;
    fCleanup (pData, pChunk);

    pChunk = pNext;                    /* neeeext */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DROP_CHUNKS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_READ || MNG_SUPPORT_WRITE */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode mng_drop_objects (mng_datap pData)
{
  mng_objectp       pObject;
  mng_objectp       pNext;
  mng_cleanupobject fCleanup;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DROP_OBJECTS, MNG_LC_START);
#endif

  pObject = pData->pFirstimgobj;       /* get first stored image-object (if any) */

  while (pObject)                      /* more objects to discard ? */
  {
    pNext = ((mng_object_headerp)pObject)->pNext;
                                       /* call appropriate cleanup */
    fCleanup = ((mng_object_headerp)pObject)->fCleanup;
    fCleanup (pData, pObject);

    pObject = pNext;                   /* neeeext */
  }

  pObject = pData->pFirstaniobj;       /* get first stored animation-object (if any) */

  while (pObject)                      /* more objects to discard ? */
  {
    pNext = ((mng_object_headerp)pObject)->pNext;
                                       /* call appropriate cleanup */
    fCleanup = ((mng_object_headerp)pObject)->fCleanup;
    fCleanup (pData, pObject);

    pObject = pNext;                   /* neeeext */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DROP_OBJECTS, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode mng_drop_savedata (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DROP_SAVEDATA, MNG_LC_START);
#endif

  if (pData->pSavedata)                /* sanity check */
  {                                    /* address it more directly */
    mng_savedatap pSave = pData->pSavedata;

    if (pSave->iGlobalProfilesize)     /* cleanup the profile ? */
      MNG_FREEX (pData, pSave->pGlobalProfile, pSave->iGlobalProfilesize)
                                       /* cleanup the save structure */
    MNG_FREE (pData, pData->pSavedata, sizeof (mng_savedata))
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_DROP_SAVEDATA, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */
/* *                                                                        * */
/* *  Versioning control                                                    * */
/* *                                                                        * */
/* ************************************************************************** */

mng_pchar MNG_DECL mng_version_text    (void)
{
  return MNG_VERSION_TEXT;
}

/* ************************************************************************** */

mng_uint8 MNG_DECL mng_version_so      (void)
{
  return MNG_VERSION_SO;
}

/* ************************************************************************** */

mng_uint8 MNG_DECL mng_version_dll     (void)
{
  return MNG_VERSION_DLL;
}

/* ************************************************************************** */

mng_uint8 MNG_DECL mng_version_major   (void)
{
  return MNG_VERSION_MAJOR;
}

/* ************************************************************************** */

mng_uint8 MNG_DECL mng_version_minor   (void)
{
  return MNG_VERSION_MINOR;
}

/* ************************************************************************** */

mng_uint8 MNG_DECL mng_version_release (void)
{
  return MNG_VERSION_RELEASE;
}

/* ************************************************************************** */
/* *                                                                        * */
/* * HLAPI routines                                                         * */
/* *                                                                        * */
/* ************************************************************************** */

mng_handle MNG_DECL mng_initialize (mng_int32     iUserdata,
                                    mng_memalloc  fMemalloc,
                                    mng_memfree   fMemfree,
                                    mng_traceproc fTraceproc)
{
  mng_datap   pData;
#ifdef MNG_SUPPORT_DISPLAY
  mng_retcode iRetcode;
  mng_imagep  pImage;
#endif

#ifdef MNG_INTERNAL_MEMMNGMT           /* allocate the main datastruc */
  pData = calloc (1, sizeof (mng_data));
#else
  pData = (mng_datap)fMemalloc (sizeof (mng_data));
#endif

  if (!pData)
    return 0;                          /* error: out of memory?? */
                                       /* validate the structure */
  pData->iMagic                = MNG_MAGIC;
                                       /* save userdata field */
  pData->iUserdata             = iUserdata;
                                       /* remember trace callback */
  pData->fTraceproc            = fTraceproc;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INITIALIZE, MNG_LC_INITIALIZE);
#endif
                                       /* default canvas styles are 8-bit RGB */
  pData->iCanvasstyle          = MNG_CANVAS_RGB8;
  pData->iBkgdstyle            = MNG_CANVAS_RGB8;

  pData->iBGred                = 0;  /* black */
  pData->iBGgreen              = 0;
  pData->iBGblue               = 0;

#ifdef MNG_FULL_CMS
  pData->bIssRGB               = MNG_TRUE;
  pData->hProf1                = 0;    /* no profiles yet */
  pData->hProf2                = 0;
  pData->hProf3                = 0;
  pData->hTrans                = 0;
#endif

  pData->dViewgamma            = 1.0;
  pData->dDisplaygamma         = 2.2;
  pData->dDfltimggamma         = 0.45455;

  pData->bStorechunks          = 1;    /* initially remember chunks */
                                       /* initial image limits */
  pData->iMaxwidth             = 1600;
  pData->iMaxheight            = 1200;

#ifdef MNG_INTERNAL_MEMMNGMT
  pData->fMemalloc             = 0;    /* internal management */
  pData->fMemfree              = 0;
#else                                  /* keep callbacks */
  pData->fMemalloc             = fMemalloc;
  pData->fMemfree              = fMemfree;
#endif

  pData->fOpenstream           = 0;    /* no value (yet) */
  pData->fClosestream          = 0;
  pData->fReaddata             = 0;
  pData->fWritedata            = 0;
  pData->fErrorproc            = 0;
  pData->fProcessheader        = 0;
  pData->fProcesstext          = 0;
  pData->fGetcanvasline        = 0;
  pData->fGetbkgdline          = 0;
  pData->fRefresh              = 0;
  pData->fGettickcount         = 0;
  pData->fSettimer             = 0;
  pData->fProcessgamma         = 0;
  pData->fProcesschroma        = 0;
  pData->fProcesssrgb          = 0;
  pData->fProcessiccp          = 0;
  pData->fProcessarow          = 0;

#if defined(MNG_SUPPORT_DISPLAY) && (defined(MNG_GAMMA_ONLY) || defined(MNG_FULL_CMS))
  pData->dLastgamma            = 0;    /* lookup table needs first-time calc */
#endif

#ifdef MNG_SUPPORT_DISPLAY             /* create object 0 */
  iRetcode = create_imageobject (pData, 0, MNG_TRUE, MNG_TRUE, MNG_TRUE,
                                 0, 0, 0, 0, 0, 0, MNG_FALSE, 0, 0, 0, 0,
                                 &pImage);

  if (iRetcode)                        /* on error drop out */
  {
    MNG_FREEX (pData, pData, sizeof (mng_data))
    return 0;
  }

  pData->pObjzero = pImage;  
#endif

#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_INCLUDE_LCMS)
  mnglcms_initlibrary ();              /* init lcms particulairs */
#endif

#ifdef MNG_INCLUDE_ZLIB
  mngzlib_initialize (pData);          /* initialize zlib structures and such */
#endif

  mng_reset ((mng_handle)pData);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (pData, MNG_FN_INITIALIZE, MNG_LC_END);
#endif

  return (mng_handle)pData;            /* if we get here, we're in business */
}

/* ************************************************************************** */

mng_retcode MNG_DECL mng_reset (mng_handle hHandle)
{
  mng_datap   pData;
#ifdef MNG_SUPPORT_DISPLAY
  mng_retcode iRetcode;
  mng_imagep  pImage;
#endif

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_RESET, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)(hHandle));      /* address main structure */

#ifdef MNG_SUPPORT_DISPLAY
  mng_drop_savedata (pData);           /* cleanup saved-data from SAVE/SEEK */
#endif

#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_FULL_CMS)
  mng_clear_cms (pData);               /* cleanup left-over cms stuff if any */
#endif

#ifdef MNG_INCLUDE_JNG
  mngjpeg_cleanup (pData);             /* cleanup jpeg stuff */
#endif

#ifdef MNG_INCLUDE_ZLIB
  if (pData->bInflating)               /* if we've been inflating */
  {
#ifdef MNG_INCLUDE_DISPLAY_PROCS
    cleanup_rowproc (pData);           /* cleanup row-processing, */
#endif
    mngzlib_inflatefree (pData);       /* cleanup inflate! */
  }
#endif /* MNG_INCLUDE_ZLIB */

#ifdef MNG_SUPPORT_READ                /* cleanup default read buffer */
  MNG_FREE (pData, pData->pReadbuf, pData->iReadbufsize)
#endif

#ifdef MNG_SUPPORT_WRITE               /* cleanup default write buffer */
  MNG_FREE (pData, pData->pWritebuf, pData->iWritebufsize)
#endif

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
  mng_drop_chunks  (pData);            /* drop stored chunks (if any) */
#endif

#ifdef MNG_SUPPORT_DISPLAY
  mng_drop_objects (pData);            /* drop stored objects (if any) */
#endif

  pData->eSigtype              = mng_it_unknown;
  pData->eImagetype            = mng_it_unknown;
  pData->iWidth                = 0;    /* these are unknown yet */
  pData->iHeight               = 0;
  pData->iTicks                = 0;
  pData->iLayercount           = 0;
  pData->iFramecount           = 0;
  pData->iPlaytime             = 0;

  pData->iMagnify              = 0;    /* 1-to-1 display */
  pData->iOffsetx              = 0;    /* no offsets */
  pData->iOffsety              = 0;
  pData->iCanvaswidth          = 0;    /* let the app decide during processheader */
  pData->iCanvasheight         = 0;

  pData->iErrorcode            = 0;    /* so far, so good */
  pData->iSeverity             = 0;
  pData->iErrorx1              = 0;
  pData->iErrorx2              = 0;
  pData->zErrortext            = 0;

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
                                       /* let's assume the best scenario */
  pData->bPreDraft48           = MNG_FALSE;
                                       /* the unknown chunk */
  pData->iChunkname            = MNG_UINT_HUH;
  pData->iChunkseq             = 0;
  pData->pFirstchunk           = 0;
  pData->pLastchunk            = 0;
                                       /* nothing processed yet */
  pData->bHasheader            = MNG_FALSE;
  pData->bHasMHDR              = MNG_FALSE;
  pData->bHasIHDR              = MNG_FALSE;
  pData->bHasBASI              = MNG_FALSE;
  pData->bHasDHDR              = MNG_FALSE;
#ifdef MNG_INCLUDE_JNG
  pData->bHasJHDR              = MNG_FALSE;
  pData->bHasJSEP              = MNG_FALSE;
  pData->bHasJDAT              = MNG_FALSE;
#endif
  pData->bHasPLTE              = MNG_FALSE;
  pData->bHasTRNS              = MNG_FALSE;
  pData->bHasGAMA              = MNG_FALSE;
  pData->bHasCHRM              = MNG_FALSE;
  pData->bHasSRGB              = MNG_FALSE;
  pData->bHasICCP              = MNG_FALSE;
  pData->bHasBKGD              = MNG_FALSE;
  pData->bHasIDAT              = MNG_FALSE;

  pData->bHasSAVE              = MNG_FALSE;
  pData->bHasBACK              = MNG_FALSE;
  pData->bHasFRAM              = MNG_FALSE;
  pData->bHasTERM              = MNG_FALSE;
  pData->bHasLOOP              = MNG_FALSE;
                                       /* there's no global stuff yet either */
  pData->bHasglobalPLTE        = MNG_FALSE;
  pData->bHasglobalTRNS        = MNG_FALSE;
  pData->bHasglobalGAMA        = MNG_FALSE;
  pData->bHasglobalCHRM        = MNG_FALSE;
  pData->bHasglobalSRGB        = MNG_FALSE;
  pData->bHasglobalICCP        = MNG_FALSE;

  pData->iDatawidth            = 0;    /* no IHDR/BASI/DHDR done yet */
  pData->iDataheight           = 0;
  pData->iBitdepth             = 0;
  pData->iColortype            = 0;
  pData->iCompression          = 0;
  pData->iFilter               = 0;
  pData->iInterlace            = 0;

#ifdef MNG_INCLUDE_JNG
  pData->iJHDRcolortype        = 0;    /* no JHDR data */
  pData->iJHDRimgbitdepth      = 0;
  pData->iJHDRimgcompression   = 0;
  pData->iJHDRimginterlace     = 0;
  pData->iJHDRalphabitdepth    = 0;
  pData->iJHDRalphacompression = 0;
  pData->iJHDRalphafilter      = 0;
  pData->iJHDRalphainterlace   = 0;
#endif

#endif /* MNG_SUPPORT_READ || MNG_SUPPORT_WRITE */

#ifdef MNG_SUPPORT_READ                /* no reading done */
  pData->bReading              = MNG_FALSE;
  pData->bHavesig              = MNG_FALSE;
  pData->bEOF                  = MNG_FALSE;
  pData->iReadbufsize          = 0;
  pData->pReadbuf              = 0;
#endif /* MNG_SUPPORT_READ */

#ifdef MNG_SUPPORT_WRITE               /* no creating/writing done */
  pData->bCreating             = MNG_FALSE;
  pData->bWriting              = MNG_FALSE;
  pData->iFirstchunkadded      = 0;
  pData->iWritebufsize         = 0;
  pData->pWritebuf             = 0;
#endif /* MNG_SUPPORT_WRITE */

#ifdef MNG_SUPPORT_DISPLAY             /* done nuttin' yet */
  pData->bDisplaying           = MNG_FALSE;
  pData->iFrameseq             = 0;
  pData->iLayerseq             = 0;
  pData->iFrametime            = 0;

  pData->iRuntime              = 0;
  pData->iStarttime            = 0;
  pData->iEndtime              = 0;
  pData->bRunning              = MNG_FALSE;
  pData->bTimerset             = MNG_FALSE;
  pData->iBreakpoint           = 0;

  pData->pCurrentobj           = 0;    /* these don't exist yet */
  pData->pCurraniobj           = 0;
  pData->pTermaniobj           = 0;
  pData->pLastclone            = 0;
  pData->pStoreobj             = 0;
  pData->pStorebuf             = 0;
  pData->pRetrieveobj          = 0;

  pData->pSavedata             = 0;    /* no saved data ! */
                                       /* TODO: remove in 1.0.0 !!! */
  pData->bEMNGMAhack           = MNG_FALSE;

  pData->iPass                 = 0;    /* interlacing stuff and temp buffers */
  pData->iRow                  = 0;
  pData->iRowinc               = 0;
  pData->iCol                  = 0;
  pData->iColinc               = 0;
  pData->iRowsamples           = 0;
  pData->iSamplemul            = 0;
  pData->iSampleofs            = 0;
  pData->iSamplediv            = 0;
  pData->iRowsize              = 0;
  pData->iRowmax               = 0;
  pData->pWorkrow              = 0;
  pData->pPrevrow              = 0;
  pData->pRGBArow              = 0;
  pData->bIsRGBA16             = MNG_TRUE;
  pData->bIsOpaque             = MNG_TRUE;
  pData->iFilterbpp            = 1;

  pData->iSourcel              = 0;    /* always initialized just before */
  pData->iSourcer              = 0;    /* compositing the next layer */
  pData->iSourcet              = 0;
  pData->iSourceb              = 0;
  pData->iDestl                = 0;
  pData->iDestr                = 0;
  pData->iDestt                = 0;
  pData->iDestb                = 0;

  pData->pFirstimgobj          = 0;    /* lists are empty */
  pData->pLastimgobj           = 0;
  pData->pFirstaniobj          = 0;
  pData->pLastaniobj           = 0;

  pData->fDisplayrow           = 0;    /* no processing callbacks */
  pData->fRestbkgdrow          = 0;
  pData->fCorrectrow           = 0;
  pData->fRetrieverow          = 0;
  pData->fStorerow             = 0;
  pData->fProcessrow           = 0;
  pData->fInitrowproc          = 0;

  pData->iPLTEcount            = 0;    /* no PLTE data */

  pData->iDEFIobjectid         = 0;    /* no DEFI data */
  pData->iDEFIdonotshow        = 0;
  pData->iDEFIconcrete         = 0;
  pData->bDEFIhasloca          = MNG_FALSE;
  pData->iDEFIlocax            = 0;
  pData->iDEFIlocay            = 0;
  pData->bDEFIhasclip          = MNG_FALSE;
  pData->iDEFIclipl            = 0;
  pData->iDEFIclipr            = 0;
  pData->iDEFIclipt            = 0;
  pData->iDEFIclipb            = 0;

  pData->iBACKred              = 0;    /* no BACK data */
  pData->iBACKgreen            = 0;
  pData->iBACKblue             = 0;
  pData->iBACKmandatory        = 0;
  pData->iBACKimageid          = 0;
  pData->iBACKtile             = 0;
                                       /* no next frame BACK data */
  pData->bHasnextBACK          = MNG_FALSE;
  pData->iNextBACKred          = 0;
  pData->iNextBACKgreen        = 0;
  pData->iNextBACKblue         = 0;
  pData->iNextBACKmandatory    = 0;
  pData->iNextBACKimageid      = 0;
  pData->iNextBACKtile         = 0;

  pData->iFRAMmode             = 1;     /* default global FRAM variables */
  pData->iFRAMdelay            = 1;
  pData->iFRAMtimeout          = 0x7fffffffl;
  pData->bFRAMclipping         = MNG_FALSE;
  pData->iFRAMclipl            = 0;
  pData->iFRAMclipr            = 0;
  pData->iFRAMclipt            = 0;
  pData->iFRAMclipb            = 0;

  pData->iFramemode            = 1;     /* again for the current frame */
  pData->iFramedelay           = 1;
  pData->iFrametimeout         = 0x7fffffffl;
  pData->bFrameclipping        = MNG_FALSE;
  pData->iFrameclipl           = 0;
  pData->iFrameclipr           = 0;
  pData->iFrameclipt           = 0;
  pData->iFrameclipb           = 0;

  pData->iNextdelay            = 1;

  pData->iSHOWmode             = 0;    /* no SHOW data */
  pData->iSHOWfromid           = 0;
  pData->iSHOWtoid             = 0;
  pData->iSHOWnextid           = 0;
  pData->iSHOWskip             = 0;

  pData->iGlobalPLTEcount      = 0;    /* no global PLTE data */

  pData->iGlobalTRNSrawlen     = 0;    /* no global tRNS data */

  pData->iGlobalGamma          = 0;    /* no global gAMA data */

  pData->iGlobalWhitepointx    = 0;    /* no global cHRM data */
  pData->iGlobalWhitepointy    = 0;
  pData->iGlobalPrimaryredx    = 0;
  pData->iGlobalPrimaryredy    = 0;
  pData->iGlobalPrimarygreenx  = 0;
  pData->iGlobalPrimarygreeny  = 0;
  pData->iGlobalPrimarybluex   = 0;
  pData->iGlobalPrimarybluey   = 0;

  pData->iGlobalRendintent     = 0;    /* no global sRGB data */

  pData->iGlobalProfilesize    = 0;    /* no global iCCP data */
  pData->pGlobalProfile        = 0;

  pData->iGlobalBKGDred        = 0;    /* no global bKGD data */
  pData->iGlobalBKGDgreen      = 0;
  pData->iGlobalBKGDblue       = 0;
#endif

#ifdef MNG_INCLUDE_ZLIB
  pData->bInflating            = 0;    /* no inflating or deflating */
  pData->bDeflating            = 0;    /* going on at the moment */
#endif

#ifdef MNG_SUPPORT_DISPLAY             /* reset object 0 */
  pImage   = (mng_imagep)pData->pObjzero;
  iRetcode = reset_object_details (pData, pImage, 0, 0, 0, 0, MNG_TRUE);

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

  pImage->bVisible             = MNG_TRUE;
  pImage->bViewable            = MNG_TRUE;
  pImage->iPosx                = 0;
  pImage->iPosy                = 0;
  pImage->bClipped             = MNG_FALSE;
  pImage->iClipl               = 0;
  pImage->iClipr               = 0;
  pImage->iClipt               = 0;
  pImage->iClipb               = 0;
#endif

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_RESET, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

mng_retcode MNG_DECL mng_cleanup (mng_handle* hHandle)
{
  mng_datap pData;                     /* local vars */
#ifndef MNG_INTERNAL_MEMMNGMT
  mng_memfree fFree;
#endif

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)*hHandle), MNG_FN_CLEANUP, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (*hHandle)           /* check validity handle */
  pData = ((mng_datap)(*hHandle));     /* and address main structure */

#ifdef MNG_SUPPORT_READ
  MNG_FREE (pData, pData->pReadbuf, pData->iReadbufsize)
#endif

#ifdef MNG_SUPPORT_WRITE
  MNG_FREE (pData, pData->pWritebuf, pData->iWritebufsize)
#endif

#ifdef MNG_SUPPORT_DISPLAY             /* drop object 0 */
  free_imageobject (pData, (mng_imagep)pData->pObjzero);
#endif

#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_INCLUDE_LCMS)
  mng_clear_cms (pData);               /* cleanup left-over cms stuff if any */

  if (pData->hProf2)                   /* output profile defined ? */
    mnglcms_freeprofile (pData->hProf2);

  if (pData->hProf3)                   /* sRGB profile defined ? */
    mnglcms_freeprofile (pData->hProf3);

#endif /* MNG_SUPPORT_DISPLAY && MNG_INCLUDE_LCMS */

#ifdef MNG_INCLUDE_JNG
  mngjpeg_cleanup (pData);             /* cleanup jpeg stuff */
#endif

#ifdef MNG_INCLUDE_ZLIB
  if (pData->bInflating)               /* if we've been inflating */
  {
#ifdef MNG_INCLUDE_DISPLAY_PROCS
    cleanup_rowproc (pData);           /* cleanup row-processing, */
#endif
    mngzlib_inflatefree (pData);       /* cleanup inflate! */
  }
#endif /* MNG_INCLUDE_ZLIB */

#ifdef MNG_INCLUDE_ZLIB
  mngzlib_cleanup (pData);             /* cleanup zlib stuff */
#endif

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
  mng_drop_chunks  (pData);            /* drop stored chunks (if any) */
#endif  

#ifdef MNG_SUPPORT_DISPLAY
  mng_drop_objects (pData);            /* drop stored objects (if any) */
#endif

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)*hHandle), MNG_FN_CLEANUP, MNG_LC_CLEANUP);
#endif

  pData->iMagic = 0;                   /* invalidate the actual memory */

#ifdef MNG_INTERNAL_MEMMNGMT
  free ((void *)*hHandle);             /* cleanup the data-structure */
#else
  fFree = ((mng_datap)*hHandle)->fMemfree;
  fFree ((mng_ptr)*hHandle, sizeof (mng_data));
#endif

  *hHandle = 0;                        /* wipe pointer to inhibit future use */

  return MNG_NOERROR;                  /* and we're done */
}

/* ************************************************************************** */

#ifdef MNG_SUPPORT_READ
mng_retcode MNG_DECL mng_read (mng_handle hHandle)
{
  mng_datap   pData;                   /* local vars */
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_READ, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle and callbacks */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

#ifndef MNG_INTERNAL_MEMMNGMT
  MNG_VALIDCB (hHandle, fMemalloc)
  MNG_VALIDCB (hHandle, fMemfree)
#endif

  MNG_VALIDCB (hHandle, fOpenstream)
  MNG_VALIDCB (hHandle, fClosestream)
  MNG_VALIDCB (hHandle, fReaddata)

#ifdef MNG_SUPPORT_DISPLAY             /* valid at this point ? */
  if ((pData->bReading) || (pData->bDisplaying))
#else
  if (pData->bReading)
#endif
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

#ifdef MNG_SUPPORT_WRITE
  if ((pData->bWriting) || (pData->bCreating))
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)
#endif

  pData->bReading = MNG_TRUE;          /* read only! */

  if (!pData->fOpenstream (hHandle))   /* open it and start reading */
    iRetcode = MNG_APPIOERROR;
  else
    iRetcode = read_graphic (pData);

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_READ, MNG_LC_END);
#endif

  return iRetcode;                     /* wow, that was easy */
}
#endif /* MNG_SUPPORT_READ */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_WRITE
mng_retcode MNG_DECL mng_write (mng_handle hHandle)
{
  mng_datap   pData;
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_WRITE, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle and callbacks */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

#ifndef MNG_INTERNAL_MEMMNGMT
  MNG_VALIDCB (hHandle, fMemalloc)
  MNG_VALIDCB (hHandle, fMemfree)
#endif

  MNG_VALIDCB (hHandle, fOpenstream)
  MNG_VALIDCB (hHandle, fClosestream)
  MNG_VALIDCB (hHandle, fWritedata)

#ifdef MNG_SUPPORT_READ
  if (pData->bReading)                 /* valid at this point ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)
#endif

  if (pData->bCreating)                /* can't write while it's still being made! */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  iRetcode = write_graphic (pData);    /* do the write */

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_WRITE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_WRITE */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_WRITE
mng_retcode MNG_DECL mng_create (mng_handle hHandle)
{
  mng_datap   pData;
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_CREATE, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle and callbacks */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

#ifndef MNG_INTERNAL_MEMMNGMT
  MNG_VALIDCB (hHandle, fMemalloc)
  MNG_VALIDCB (hHandle, fMemfree)
#endif

#ifdef MNG_SUPPORT_READ
  if (pData->bReading)                 /* valid at this point ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)
#endif

  if ((pData->bWriting) || (pData->bCreating))
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  iRetcode = mng_reset (hHandle);      /* clear any previous stuff */

  if (iRetcode)                        /* on error bail out */
    return iRetcode;

  pData->bCreating = MNG_TRUE;         /* indicate we're creating a new file */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_CREATE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_WRITE */

/* ************************************************************************** */

#if defined(MNG_SUPPORT_DISPLAY) && defined(MNG_SUPPORT_READ)
mng_retcode MNG_DECL mng_readdisplay (mng_handle hHandle)
{
  mng_datap   pData;                   /* local vars */
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_READDISPLAY, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle and callbacks */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

#ifndef MNG_INTERNAL_MEMMNGMT
  MNG_VALIDCB (hHandle, fMemalloc)
  MNG_VALIDCB (hHandle, fMemfree)
#endif

  MNG_VALIDCB (hHandle, fReaddata)
  MNG_VALIDCB (hHandle, fGetcanvasline)
  MNG_VALIDCB (hHandle, fRefresh)
  MNG_VALIDCB (hHandle, fGettickcount)
  MNG_VALIDCB (hHandle, fSettimer)
                                       /* valid at this point ? */
  if ((pData->bReading) || (pData->bDisplaying))
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

#ifdef MNG_SUPPORT_WRITE
  if ((pData->bWriting) || (pData->bCreating))
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)
#endif

  pData->bReading    = MNG_TRUE;       /* read & display! */
  pData->bDisplaying = MNG_TRUE;
  pData->bRunning    = MNG_TRUE;
  pData->iRuntime    = 0;
  pData->iStarttime  = pData->fGettickcount (hHandle);
  pData->iEndtime    = 0;

  if (!pData->fOpenstream (hHandle))   /* open it and start reading */
    iRetcode = MNG_APPIOERROR;
  else
    iRetcode = read_graphic (pData);

  if (pData->bEOF)                     /* already at EOF ? */
    pData->bReading  = MNG_FALSE;      /* then we're no longer reading */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_READDISPLAY, MNG_LC_END);
#endif

  return iRetcode;
}
#endif /* MNG_SUPPORT_DISPLAY && MNG_SUPPORT_READ */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display (mng_handle hHandle)
{
  mng_datap pData;                     /* local vars */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle and callbacks */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

#ifndef MNG_INTERNAL_MEMMNGMT
  MNG_VALIDCB (hHandle, fMemalloc)
  MNG_VALIDCB (hHandle, fMemfree)
#endif

  MNG_VALIDCB (hHandle, fGetcanvasline)
  MNG_VALIDCB (hHandle, fRefresh)
  MNG_VALIDCB (hHandle, fGettickcount)
  MNG_VALIDCB (hHandle, fSettimer)
                                       /* valid at this point ? */
  if ((pData->bReading) || (pData->bDisplaying))
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  pData->bDisplaying = MNG_TRUE;       /* display! */
  pData->bRunning    = MNG_TRUE;
  pData->iRuntime    = 0;
  pData->iStarttime  = pData->fGettickcount (hHandle);
  pData->iEndtime    = 0;


  /* TODO: display */
  MNG_WARNING (pData, MNG_FNNOTIMPLEMENTED)


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display_resume (mng_handle hHandle)
{
  mng_datap   pData;                   /* local vars */
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_RESUME, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  if (!pData->bDisplaying)             /* can we expect this call ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  if (pData->bRunning)                 /* was it running ? */
  {
    if (pData->bTimerset)              /* are we expecting this call ? */
    {
      pData->bTimerset = MNG_FALSE;    /* reset the flag */

      if (pData->bReading)             /* set during read&display ? */
      {
        iRetcode = read_graphic (pData);

        if (iRetcode)                  /* on error bail out */
          return iRetcode; 

        if (pData->bEOF)               /* already at EOF ? */
          pData->bReading = MNG_FALSE; /* then we're no longer reading */

      }
      else
     {


        /* TODO: display_resume for mng_display */
        MNG_WARNING (pData, MNG_FNNOTIMPLEMENTED)


      }
    }
    else
      MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  }
  else
  {

    /* TODO: resume animation after freeze or reset */
    MNG_WARNING (pData, MNG_FNNOTIMPLEMENTED)

  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_RESUME, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display_freeze (mng_handle hHandle)
{
  mng_datap   pData;
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_FREEZE, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  if (!pData->bDisplaying)             /* can we expect this call ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  pData->bRunning = MNG_FALSE;         /* indicate it's frozen */
                                       /* need to finish reading the stream ? */
  if ((pData->bReading) && (!pData->bEOF))
  {
    iRetcode = read_graphic (pData);   /* so read till EOF */

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    pData->bReading = MNG_FALSE;       /* gotta be finished now */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_FREEZE, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display_reset (mng_handle hHandle)
{
  mng_datap   pData;  
  mng_retcode iRetcode;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_RESET, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  if (!pData->bDisplaying)             /* can we expect this call ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  pData->bDisplaying = MNG_FALSE;      /* ok; stop displaying ! */
  pData->bRunning    = MNG_FALSE;
                                       /* need to finish reading the stream ? */
  if ((pData->bReading) && (!pData->bEOF))
  {
    iRetcode = read_graphic (pData);   /* so read till EOF */

    if (iRetcode)                      /* on error bail out */
      return iRetcode;

    pData->bReading = MNG_FALSE;       /* gotta be finished now */
  }

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_RESET, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display_goframe (mng_handle hHandle,
                                          mng_uint32 iFramenr)
{
  mng_datap pData;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_GOFRAME, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  if (pData->eImagetype != mng_it_mng) /* is it an animation ? */
    MNG_ERROR (pData, MNG_NOTANANIMATION);

  if (!pData->bDisplaying)             /* can we expect this call ? */
    MNG_ERROR ((mng_datap)hHandle, MNG_FUNCTIONINVALID)

  if (iFramenr > pData->iFramecount)   /* is the parameter within bounds ? */
    MNG_ERROR (pData, MNG_FRAMENRTOOHIGH);


  /* TODO: display_goframe */
  MNG_WARNING (pData, MNG_FNNOTIMPLEMENTED)


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_GOFRAME, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display_golayer (mng_handle hHandle,
                                          mng_uint32 iLayernr)
{
  mng_datap pData;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_GOLAYER, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  if (pData->eImagetype != mng_it_mng) /* is it an animation ? */
    MNG_ERROR (pData, MNG_NOTANANIMATION)

  if (!pData->bDisplaying)             /* can we expect this call ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  if (iLayernr > pData->iLayercount)   /* is the parameter within bounds ? */
    MNG_ERROR (pData, MNG_LAYERNRTOOHIGH)


  /* TODO: display_golayer */
  MNG_WARNING (pData, MNG_FNNOTIMPLEMENTED)


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_GOLAYER, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

#ifdef MNG_SUPPORT_DISPLAY
mng_retcode MNG_DECL mng_display_gotime (mng_handle hHandle,
                                         mng_uint32 iPlaytime)
{
  mng_datap pData;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_GOTIME, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  if (pData->eImagetype != mng_it_mng) /* is it an animation ? */
    MNG_ERROR (pData, MNG_NOTANANIMATION)

  if (!pData->bDisplaying)             /* can we expect this call ? */
    MNG_ERROR (pData, MNG_FUNCTIONINVALID)

  if (iPlaytime > pData->iPlaytime)    /* is the parameter within bounds ? */
    MNG_ERROR (pData, MNG_PLAYTIMETOOHIGH)


  /* TODO: display_gotime */
  MNG_WARNING (pData, MNG_FNNOTIMPLEMENTED)


#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_DISPLAY_GOTIME, MNG_LC_END);
#endif

  return MNG_NOERROR;
}
#endif /* MNG_SUPPORT_DISPLAY */

/* ************************************************************************** */

mng_retcode MNG_DECL mng_getlasterror (mng_handle   hHandle,
                                       mng_int8*    iSeverity,
                                       mng_chunkid* iChunkname,
                                       mng_uint32*  iChunkseq,
                                       mng_int32*   iExtra1,
                                       mng_int32*   iExtra2,
                                       mng_pchar*   zErrortext)
{
  mng_datap pData;                     /* local vars */

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_GETLASTERROR, MNG_LC_START);
#endif

  MNG_VALIDHANDLE (hHandle)            /* check validity handle */
  pData = ((mng_datap)hHandle);        /* and make it addressable */

  *iSeverity  = pData->iSeverity;      /* return the appropriate fields */

#if defined(MNG_SUPPORT_READ) || defined(MNG_SUPPORT_WRITE)
  *iChunkname = pData->iChunkname;
  *iChunkseq  = pData->iChunkseq;
#else
  *iChunkname = MNG_UINT_HUH;
  *iChunkseq  = 0;
#endif

  *iExtra1    = pData->iErrorx1;
  *iExtra2    = pData->iErrorx2;
  *zErrortext = pData->zErrortext;

#ifdef MNG_SUPPORT_TRACE
  MNG_TRACE (((mng_datap)hHandle), MNG_FN_GETLASTERROR, MNG_LC_END);
#endif

  return pData->iErrorcode;            /* and the errorcode */
}

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

