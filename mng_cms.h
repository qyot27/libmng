/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_cms.h                 copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : color management routines (definition)                     * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of color management routines                    * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                                    * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _mng_cms_h_
#define _mng_cms_h_

/* ************************************************************************** */

void        mnglcms_initlibrary       (void);
mng_cmsprof mnglcms_createfileprofile (mng_pchar    zFilename);
void        mnglcms_freeprofile       (mng_cmsprof  hProf    );
void        mnglcms_freetransform     (mng_cmstrans hTrans   );

/* ************************************************************************** */

mng_retcode init_full_cms          (mng_datap pData);
mng_retcode init_full_cms_object   (mng_datap pData);
mng_retcode correct_full_cms       (mng_datap pData);

mng_retcode init_gamma_only        (mng_datap pData);
mng_retcode init_gamma_only_object (mng_datap pData);
mng_retcode correct_gamma_only     (mng_datap pData);

mng_retcode init_app_cms           (mng_datap pData);
mng_retcode init_app_cms_object    (mng_datap pData);
mng_retcode correct_app_cms        (mng_datap pData);

/* ************************************************************************** */

#endif /* _mng_cms_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
