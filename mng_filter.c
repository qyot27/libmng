/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_filter.c              copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Filtering routines (implementation)                        * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : implementation of the filtering routines                   * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#include "libmng.h"
#include "mng_data.h"
#include "mng_error.h"
#include "mng_trace.h"
#include "mng_filter.h"

/* ************************************************************************** */

#ifdef MNG_INCLUDE_FILTERS

/* ************************************************************************** */

mng_retcode filter_a_row (mng_datap pData)
{
#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_A_ROW, MNG_LC_START);
#endif

  switch (pData->pWorkrow[0])
  {
    case 1  : {
                filter_sub     (pData);
                break;
              }
    case 2  : {
                filter_up      (pData);
                break;
              }
    case 3  : {
                filter_average (pData);
                break;
              }
    case 4  : {
                filter_paeth   (pData);
                break;
              }

    default : return MNG_INVALIDFILTER; 
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_A_ROW, MNG_LC_END);
#endif

  return MNG_NOERROR;
}

/* ************************************************************************** */

void filter_sub (mng_datap pData)
{
  mng_uint32 iBpp;
  mng_uint8p pRawx;
  mng_uint8p pRawx_prev;
  mng_int32  iX;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_SUB, MNG_LC_START);
#endif

  iBpp       = pData->iFilterbpp;
  pRawx      = pData->pWorkrow + 1 + iBpp;
  pRawx_prev = pData->pWorkrow + 1;

  for (iX = iBpp; iX < pData->iRowsize; iX++)
  {
    pRawx [0] = (mng_uint8)(pRawx [0] + pRawx_prev [0]);
    pRawx++;
    pRawx_prev++;
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_SUB, MNG_LC_END);
#endif

  return;
}

/* ************************************************************************** */

void filter_up (mng_datap pData)
{
  mng_uint8p pRawx;
  mng_uint8p pPriorx;
  mng_int32  iX;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_UP, MNG_LC_START);
#endif

  pRawx   = pData->pWorkrow + 1;
  pPriorx = pData->pPrevrow + 1;

  for (iX = 0; iX < pData->iRowsize; iX++)
  {
    pRawx [0] = (mng_uint8)(pRawx [0] + pPriorx [0]);
    pRawx++;
    pPriorx++;
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_UP, MNG_LC_END);
#endif

  return;
}

/* ************************************************************************** */

void filter_average (mng_datap pData)
{
  mng_int32  iBpp;
  mng_uint8p pRawx;
  mng_uint8p pRawx_prev;
  mng_uint8p pPriorx;
  mng_int32  iX;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_AVERAGE, MNG_LC_START);
#endif

  iBpp       = pData->iFilterbpp;
  pRawx      = pData->pWorkrow + 1;
  pPriorx    = pData->pPrevrow + 1;
  pRawx_prev = pData->pWorkrow + 1;

  for (iX = 0; iX < iBpp; iX++)
  {
    pRawx [0] = (mng_uint8)(pRawx [0] + (pPriorx [0] >> 1));
    pRawx++;
    pPriorx++;
  }

  for (iX = iBpp; iX < pData->iRowsize; iX++)
  {
    pRawx [0] = (mng_uint8)(pRawx [0] + ((pRawx_prev [0] + pPriorx [0]) >> 1));
    pRawx++;
    pPriorx++;
    pRawx_prev++;
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_AVERAGE, MNG_LC_END);
#endif

  return;
}

/* ************************************************************************** */

void filter_paeth (mng_datap pData)
{
  mng_int32  iBpp;
  mng_uint8p pRawx;
  mng_uint8p pRawx_prev;
  mng_uint8p pPriorx;
  mng_uint8p pPriorx_prev;
  mng_int32  iX;
  mng_uint32 iA, iB, iC;
  mng_uint32 iP;
  mng_uint32 iPa, iPb, iPc;

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_PAETH, MNG_LC_START);
#endif

  iBpp         = pData->iFilterbpp;
  pRawx        = pData->pWorkrow + 1;
  pPriorx      = pData->pPrevrow + 1;
  pRawx_prev   = pData->pWorkrow + 1;
  pPriorx_prev = pData->pPrevrow + 1;

  for (iX = 0; iX < iBpp; iX++)
  {
    pRawx [0] = (mng_uint8)(pRawx [0] + pPriorx [0]);

    pRawx++;
    pPriorx++;
  }

  for (iX = iBpp; iX < pData->iRowsize; iX++)
  {
    iA  = (mng_uint32)pRawx_prev   [0];
    iB  = (mng_uint32)pPriorx      [0];
    iC  = (mng_uint32)pPriorx_prev [0];
    iP  = iA + iB - iC;
    iPa = abs (iP - iA);
    iPb = abs (iP - iB);
    iPc = abs (iP - iC);

    if ((iPa <= iPb) && (iPa <= iPc))
      pRawx [0] = (mng_uint8)(pRawx [0] + iA);
    else
      if (iPb <= iPc)
        pRawx [0] = (mng_uint8)(pRawx [0] + iB);
      else
        pRawx [0] = (mng_uint8)(pRawx [0] + iC);

    pRawx++;
    pPriorx++;
    pRawx_prev++;
    pPriorx_prev++;
  }

#ifdef MNG_SUPPORT_TRACE
  mng_trace (pData, MNG_FN_FILTER_PAETH, MNG_LC_END);
#endif

  return;
}

/* ************************************************************************** */

#endif /* MNG_INCLUDE_FILTERS */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */

