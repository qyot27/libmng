/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_pixels.h              copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.0                                                      * */
/* *                                                                        * */
/* * purpose   : Pixel-row management routines (definition)                 * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of the pixel-row management routines            * */
/* *                                                                        * */
/* * changes   : 0.5.0 ../../.. **none**                        **nobody**  * */
/* *                                                                        * */
/* ************************************************************************** */

#ifdef __BORLANDC__
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _mng_pixels_h_
#define _mng_pixels_h_

/* ************************************************************************** */
/* *                                                                        * */
/* * Display routines - convert rowdata (which is already color-corrected)  * */
/* * to the output canvas, respecting any transparency information          * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode display_rgb8      (mng_datap pData);
mng_retcode display_rgba8     (mng_datap pData);
mng_retcode display_argb8     (mng_datap pData);
mng_retcode display_bgr8      (mng_datap pData);
mng_retcode display_bgra8     (mng_datap pData);
mng_retcode display_abgr8     (mng_datap pData);

/* ************************************************************************** */
/* *                                                                        * */
/* * Background restore routines - restore the background with info from    * */
/* * the BACK and/or bKGD chunk and/or the app's background canvas          * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode restore_bkgd_backimage (mng_datap pData);
mng_retcode restore_bkgd_backcolor (mng_datap pData);
mng_retcode restore_bkgd_bgcolor   (mng_datap pData);
mng_retcode restore_bkgd_rgb8      (mng_datap pData);
mng_retcode restore_bkgd_bgr8      (mng_datap pData);

/* ************************************************************************** */
/* *                                                                        * */
/* * Row retrieval routines - retrieve processed & uncompressed row-data    * */
/* * from the current "object"                                              * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode retrieve_g8       (mng_datap pData);
mng_retcode retrieve_g16      (mng_datap pData);
mng_retcode retrieve_rgb8     (mng_datap pData);
mng_retcode retrieve_rgb16    (mng_datap pData);
mng_retcode retrieve_idx8     (mng_datap pData);
mng_retcode retrieve_ga8      (mng_datap pData);
mng_retcode retrieve_ga16     (mng_datap pData);
mng_retcode retrieve_rgba8    (mng_datap pData);
mng_retcode retrieve_rgba16   (mng_datap pData);

/* ************************************************************************** */
/* *                                                                        * */
/* * Row storage routines - store processed & uncompressed row-data         * */
/* * into the current "object"                                              * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode store_g1          (mng_datap pData);
mng_retcode store_g2          (mng_datap pData);
mng_retcode store_g4          (mng_datap pData);
mng_retcode store_g8          (mng_datap pData);
mng_retcode store_g16         (mng_datap pData);
mng_retcode store_rgb8        (mng_datap pData);
mng_retcode store_rgb16       (mng_datap pData);
mng_retcode store_idx1        (mng_datap pData);
mng_retcode store_idx2        (mng_datap pData);
mng_retcode store_idx4        (mng_datap pData);
mng_retcode store_idx8        (mng_datap pData);
mng_retcode store_ga8         (mng_datap pData);
mng_retcode store_ga16        (mng_datap pData);
mng_retcode store_rgba8       (mng_datap pData);
mng_retcode store_rgba16      (mng_datap pData);

/* ************************************************************************** */
/* *                                                                        * */
/* * Row processing routines - convert uncompressed data from zlib to       * */
/* * managable row-data which serves as input to the color-management       * */
/* * routines                                                               * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode process_g1        (mng_datap pData);
mng_retcode process_g2        (mng_datap pData);
mng_retcode process_g4        (mng_datap pData);
mng_retcode process_g8        (mng_datap pData);
mng_retcode process_g16       (mng_datap pData);
mng_retcode process_rgb8      (mng_datap pData);
mng_retcode process_rgb16     (mng_datap pData);
mng_retcode process_idx1      (mng_datap pData);
mng_retcode process_idx2      (mng_datap pData);
mng_retcode process_idx4      (mng_datap pData);
mng_retcode process_idx8      (mng_datap pData);
mng_retcode process_ga8       (mng_datap pData);
mng_retcode process_ga16      (mng_datap pData);
mng_retcode process_rgba8     (mng_datap pData);
mng_retcode process_rgba16    (mng_datap pData);

/* ************************************************************************** */
/* *                                                                        * */
/* * Row processing initialization routines - set up the variables needed   * */
/* * to process uncompressed row-data                                       * */
/* *                                                                        * */
/* ************************************************************************** */

mng_retcode init_g1_ni        (mng_datap pData);
mng_retcode init_g1_i         (mng_datap pData);
mng_retcode init_g2_ni        (mng_datap pData);
mng_retcode init_g2_i         (mng_datap pData);
mng_retcode init_g4_ni        (mng_datap pData);
mng_retcode init_g4_i         (mng_datap pData);
mng_retcode init_g8_ni        (mng_datap pData);
mng_retcode init_g8_i         (mng_datap pData);
mng_retcode init_g16_ni       (mng_datap pData);
mng_retcode init_g16_i        (mng_datap pData);
mng_retcode init_rgb8_ni      (mng_datap pData);
mng_retcode init_rgb8_i       (mng_datap pData);
mng_retcode init_rgb16_ni     (mng_datap pData);
mng_retcode init_rgb16_i      (mng_datap pData);
mng_retcode init_idx1_ni      (mng_datap pData);
mng_retcode init_idx1_i       (mng_datap pData);
mng_retcode init_idx2_ni      (mng_datap pData);
mng_retcode init_idx2_i       (mng_datap pData);
mng_retcode init_idx4_ni      (mng_datap pData);
mng_retcode init_idx4_i       (mng_datap pData);
mng_retcode init_idx8_ni      (mng_datap pData);
mng_retcode init_idx8_i       (mng_datap pData);
mng_retcode init_ga8_ni       (mng_datap pData);
mng_retcode init_ga8_i        (mng_datap pData);
mng_retcode init_ga16_ni      (mng_datap pData);
mng_retcode init_ga16_i       (mng_datap pData);
mng_retcode init_rgba8_ni     (mng_datap pData);
mng_retcode init_rgba8_i      (mng_datap pData);
mng_retcode init_rgba16_ni    (mng_datap pData);
mng_retcode init_rgba16_i     (mng_datap pData);
            
/* ************************************************************************** */

mng_retcode init_rowproc      (mng_datap pData);
void        next_row          (mng_datap pData);
mng_retcode cleanup_rowproc   (mng_datap pData);

/* ************************************************************************** */

#endif /* _mng_pixels_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
