/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_trace.c               copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Trace functions (implementation)                           * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the trace functions                      * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                        **nobody**  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __borlandc__
#pragma option -A
#endif

#include "libmng.h"
#include "mng_data.h"
#include "mng_trace.h"

/* ************************************************************************** */

#ifdef MNG_INCLUDE_TRACE_PROCS

/* ************************************************************************** */

void mng_trace (mng_datap  pData,
                mng_uint32 iFunction,
                mng_uint32 iLocation)
{
  mng_pchar zName = 0;                 /* bufferptr for tracestring */

  if ((pData == 0) || (pData->iMagic != MNG_MAGIC))
    return;                            /* no good if the handle is corrupt */

  if (pData->fTraceproc)               /* report back to user ? */
  {
#ifdef MNG_TRACE_TELLTALE


    /* TODO: get trace string */


#endif
                                       /* oke, now tell */
    pData->fTraceproc (((mng_handle)pData), iFunction, iLocation, zName);
  }
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_TRACE_PROCS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

