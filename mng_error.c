/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_error.c               copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.1                                                      * */
/* *                                                                        * */
/* * purpose   : Error routines (implementation)                            * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the general error handling routines      * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - changed strict-ANSI stuff                                * */
/* *                                                                        * */
/* ************************************************************************** */

#include "libmng.h"
#include "mng_data.h"
#include "mng_error.h"
#include "mng_trace.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

/* ************************************************************************** */

mng_bool mng_process_error (mng_datap   pData,
                            mng_retcode iError,
                            mng_retcode iExtra1,
                            mng_retcode iExtra2)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_ERROR, MNG_LC_START);
#endif

  if (pData != 0)
  {
    pData->iErrorcode = iError;        /* save also for getlasterror */
    pData->iErrorx1   = iExtra1;
    pData->iErrorx2   = iExtra2;

#ifdef MNG_ERROR_TELLTALE
    pData->zErrortext = 0;             /* something to do still !!!! */
#else
    pData->zErrortext = 0;
#endif /* mng_error_telltale */

    if (iError == 0)                   /* no error is not severe ! */
    {
      pData->iSeverity = 0;
    }
    else
    {
      switch (iError&0x3C00)           /* determine the severity */
      {
        case 0x0800 : { pData->iSeverity = 5; break; };
        case 0x1000 : { pData->iSeverity = 2; break; };
        case 0x2000 : { pData->iSeverity = 1; break; };
        default     : { pData->iSeverity = 9; }
      };
    };

    if (pData->fErrorproc)             /* callback defined ? */
    {
      return pData->fErrorproc (((mng_handle)pData), iError, pData->iSeverity,
                                pData->iChunkname, pData->iChunkseq,
                                pData->iErrorx1, pData->iErrorx2, pData->zErrortext);
    };
  };

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_PROCESS_ERROR, MNG_LC_END);
#endif

  return MNG_FALSE;                    /* automatic failure */
};

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

