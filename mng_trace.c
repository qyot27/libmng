/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_trace.c               copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.1                                                      * */
/* *                                                                        * */
/* * purpose   : Trace functions (implementation)                           * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the trace functions                      * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/12/2000 - G.Juyn                                * */
/* *             - added callback error-reporting support                   * */
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

#ifdef MNG_INCLUDE_TRACE_PROCS

/* ************************************************************************** */

mng_retcode mng_trace (mng_datap  pData,
                       mng_uint32 iFunction,
                       mng_uint32 iLocation)
{
  mng_pchar zName = 0;                 /* bufferptr for tracestring */

  if ((pData == 0) || (pData->iMagic != MNG_MAGIC))
    return MNG_INVALIDHANDLE;          /* no good if the handle is corrupt */

  if (pData->fTraceproc)               /* report back to user ? */
  {
#ifdef MNG_TRACE_TELLTALE


    /* TODO: get trace string */


#endif
                                       /* oke, now tell */
    if (!pData->fTraceproc (((mng_handle)pData), iFunction, iLocation, zName))
      return MNG_APPTRACEABORT;

  }

  return MNG_NOERROR;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_TRACE_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

