/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_display.h             copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Display management (definition)                            * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of the display managament routines              * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                        **nobody**  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _mng_display_h_
#define _mng_display_h_

/* ************************************************************************** */

mng_retcode display_image (mng_datap  pData,
                           mng_imagep pImage);
                           
/* ************************************************************************** */

mng_retcode process_display_ihdr  (mng_datap  pData);

mng_retcode process_display_idat  (mng_datap  pData,
                                   mng_uint32 iRawlen,
                                   mng_uint8p pRawdata);

mng_retcode process_display_iend  (mng_datap  pData);

mng_retcode process_display_mend  (mng_datap  pData);

mng_retcode process_display_defi  (mng_datap  pData);

mng_retcode process_display_basi  (mng_datap  pData,
                                   mng_uint16 iRed,
                                   mng_uint16 iGreen,
                                   mng_uint16 iBlue,
                                   mng_bool   bHasalpha,
                                   mng_uint16 iAlpha,
                                   mng_uint8  iViewable);

mng_retcode process_display_clon  (mng_datap  pData,
                                   mng_uint16 iSourceid,
                                   mng_uint16 iCloneid,
                                   mng_uint8  iClonetype,
                                   mng_bool   bHasdonotshow,
                                   mng_uint8  iDonotshow,
                                   mng_uint8  iConcrete,
                                   mng_bool   bHasloca,
                                   mng_uint8  iLocationtype,
                                   mng_int32  iLocationx,
                                   mng_int32  iLocationy);
mng_retcode process_display_clon2 (mng_datap pData);

mng_retcode process_display_disc  (mng_datap   pData,
                                   mng_uint32  iCount,
                                   mng_uint16p pIds);

mng_retcode process_display_fram  (mng_datap  pData,
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
                                   mng_int32  iClipb);
mng_retcode process_display_fram2 (mng_datap  pData);

mng_retcode process_display_move  (mng_datap  pData,
                                   mng_uint16 iFromid,
                                   mng_uint16 iToid,
                                   mng_uint8  iMovetype,
                                   mng_int32  iMovex,
                                   mng_int32  iMovey);

mng_retcode process_display_clip  (mng_datap  pData,
                                   mng_uint16 iFromid,
                                   mng_uint16 iToid,
                                   mng_uint8  iCliptype,
                                   mng_int32  iClipl,
                                   mng_int32  iClipr,
                                   mng_int32  iClipt,
                                   mng_int32  iClipb);

mng_retcode process_display_show  (mng_datap  pData);
                                  
/* ************************************************************************** */

#endif /* _mng_display_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
