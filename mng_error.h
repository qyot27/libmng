/* ************************************************************************** */
/* *             For conditions of distribution and use,                    * */
/* *                see copyright notice in libmng.h                        * */
/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mng_error.h               copyright (c) 2000 G.Juyn        * */
/* * version   : 0.5.1                                                      * */
/* *                                                                        * */
/* * purpose   : Error functions (definition)                               * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : Definition of the generic error-codes and functions        * */
/* *                                                                        * */
/* * changes   : 0.5.1 - 05/06/2000 - G.Juyn                                * */
/* *             - added some errorcodes                                    * */
/* *             0.5.1 - 05/08/2000 - G.Juyn                                * */
/* *             - added some errorcodes                                    * */
/* *             - changed strict-ANSI stuff                                * */
/* *             0.5.1 - 05/11/2000 - G.Juyn                                * */
/* *             - added application errorcodes (used with callbacks)       * */
/* *             - moved chunk-access errorcodes to severity 5              * */
/* *                                                                        * */
/* ************************************************************************** */

#if defined(__BORLANDC__) && defined(MNG_STRICT_ANSI)
#pragma option -A                      /* force ANSI-C */
#endif

#ifndef _mng_error_h_
#define _mng_error_h_

/* ************************************************************************** */
/* *                                                                        * */
/* * Error-code structure                                                   * */
/* *                                                                        * */
/* * 0b0000 00xx xxxx xxxx - basic errors; severity 9 (environment)         * */
/* * 0b0000 01xx xxxx xxxx - chunk errors; severity 9 (image induced)       * */
/* * 0b0000 10xx xxxx xxxx - severity 5 errors (application induced)        * */
/* * 0b0001 00xx xxxx xxxx - severity 2 warnings (recoverable)              * */
/* * 0b0010 00xx xxxx xxxx - severity 1 warnings (recoverable)              * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_NOERROR          (mng_retcode)0    /* er.. indicates all's well   */

#define MNG_OUTOFMEMORY      (mng_retcode)1    /* oops, buy some megabytes!   */
#define MNG_INVALIDHANDLE    (mng_retcode)2    /* call mng_initialize first   */
#define MNG_NOCALLBACK       (mng_retcode)3    /* set the callbacks please    */
#define MNG_UNEXPECTEDEOF    (mng_retcode)4    /* what'd ya do with the data? */
#define MNG_ZLIBERROR        (mng_retcode)5    /* zlib burped                 */
#define MNG_JPEGERROR        (mng_retcode)6    /* jpglib complained           */
#define MNG_LCMSERROR        (mng_retcode)7    /* little cms stressed out     */
#define MNG_NOOUTPUTPROFILE  (mng_retcode)8    /* no output-profile defined   */
#define MNG_NOSRGBPROFILE    (mng_retcode)9    /* no sRGB-profile defined     */
#define MNG_BUFOVERFLOW      (mng_retcode)10   /* zlib output-buffer overflow */
#define MNG_FUNCTIONINVALID  (mng_retcode)11   /* ay, totally inappropriate   */
#define MNG_OUTPUTERROR      (mng_retcode)12   /* disk full ?                 */

#define MNG_APPIOERROR       (mng_retcode)901  /* application I/O error       */
#define MNG_APPTIMERERROR    (mng_retcode)902  /* application timing error    */
#define MNG_APPCMSERROR      (mng_retcode)903  /* application CMS error       */
#define MNG_APPMISCERROR     (mng_retcode)904  /* application other error     */
#define MNG_APPTRACEABORT    (mng_retcode)905  /* application aborts on trace */

#define MNG_INTERNALERROR    (mng_retcode)999  /* internal inconsistancy      */

#define MNG_INVALIDSIG       (mng_retcode)1025 /* invalid graphics file       */
#define MNG_INVALIDCRC       (mng_retcode)1027 /* crc check failed            */
#define MNG_INVALIDLENGTH    (mng_retcode)1028 /* chunklength mystifies me    */
#define MNG_SEQUENCEERROR    (mng_retcode)1029 /* invalid chunk sequence      */
#define MNG_CHUNKNOTALLOWED  (mng_retcode)1030 /* completely out-of-place     */
#define MNG_MULTIPLEERROR    (mng_retcode)1031 /* only one occurence allowed  */
#define MNG_PLTEMISSING      (mng_retcode)1032 /* indexed-color requires PLTE */
#define MNG_IDATMISSING      (mng_retcode)1033 /* IHDR-block requires IDAT    */
#define MNG_CANNOTBEEMPTY    (mng_retcode)1034 /* must contain some data      */
#define MNG_GLOBALLENGTHERR  (mng_retcode)1035 /* global data incorrect       */
#define MNG_INVALIDBITDEPTH  (mng_retcode)1036 /* bitdepth out-of-range       */
#define MNG_INVALIDCOLORTYPE (mng_retcode)1037 /* colortype out-of-range      */
#define MNG_INVALIDCOMPRESS  (mng_retcode)1038 /* compression method invalid  */
#define MNG_INVALIDFILTER    (mng_retcode)1039 /* filter method invalid       */
#define MNG_INVALIDINTERLACE (mng_retcode)1040 /* interlace method invalid    */
#define MNG_NOTENOUGHIDAT    (mng_retcode)1041 /* ran out of compressed data  */
#define MNG_PLTEINDEXERROR   (mng_retcode)1042 /* palette-index out-of-range  */
#define MNG_NULLNOTFOUND     (mng_retcode)1043 /* couldn't find null-separator*/
#define MNG_KEYWORDNULL      (mng_retcode)1044 /* keyword cannot be empty     */
#define MNG_OBJECTUNKNOWN    (mng_retcode)1045 /* the object can't be found   */
#define MNG_OBJECTEXISTS     (mng_retcode)1046 /* the object already exists   */
#define MNG_TOOMUCHIDAT      (mng_retcode)1047 /* got too much compressed data*/
#define MNG_INVSAMPLEDEPTH   (mng_retcode)1048 /* sampledepth out-of-range    */
#define MNG_INVOFFSETSIZE    (mng_retcode)1049 /* invalid offset-size         */
#define MNG_INVENTRYTYPE     (mng_retcode)1050 /* invalid entry-type          */
#define MNG_ENDWITHNULL      (mng_retcode)1051 /* may not end with NULL       */
#define MNG_INVIMAGETYPE     (mng_retcode)1052 /* invalid image_type          */
#define MNG_INVDELTATYPE     (mng_retcode)1053 /* invalid delta_type          */
#define MNG_INVALIDINDEX     (mng_retcode)1054 /* index-value invalid         */

#define MNG_INVALIDCNVSTYLE  (mng_retcode)2049 /* can't make anything of this */
#define MNG_WRONGCHUNK       (mng_retcode)2050 /* accessing the wrong chunk   */
#define MNG_INVALIDENTRYIX   (mng_retcode)2051 /* accessing the wrong entry   */
#define MNG_NOHEADER         (mng_retcode)2052 /* must have had header first  */
#define MNG_NOCORRCHUNK      (mng_retcode)2053 /* can't find parent chunk     */

#define MNG_IMAGETOOLARGE    (mng_retcode)4097 /* input-image way too big     */
#define MNG_NOTANANIMATION   (mng_retcode)4098 /* file not a MNG              */
#define MNG_FRAMENRTOOHIGH   (mng_retcode)4099 /* frame-nr out-of-range       */
#define MNG_LAYERNRTOOHIGH   (mng_retcode)4100 /* layer-nr out-of-range       */
#define MNG_PLAYTIMETOOHIGH  (mng_retcode)4101 /* playtime out-of-range       */
#define MNG_FNNOTIMPLEMENTED (mng_retcode)4102 /* function not yet available  */

#define MNG_IMAGEFROZEN      (mng_retcode)8193 /* stopped displaying          */

#define MNG_LCMS_NOHANDLE    1                 /* LCMS returned NULL handle   */
#define MNG_LCMS_NOMEM       2                 /* LCMS returned NULL gammatab */
#define MNG_LCMS_NOTRANS     3                 /* LCMS returned NULL transform*/

/* ************************************************************************** */
/* *                                                                        * */
/* * Default error routines                                                 * */
/* *                                                                        * */
/* ************************************************************************** */

mng_bool mng_process_error (mng_datap   pData,
                            mng_retcode iError,
                            mng_retcode iExtra1,
                            mng_retcode iExtra2);

/* ************************************************************************** */
/* *                                                                        * */
/* * Error handling macros                                                  * */
/* *                                                                        * */
/* ************************************************************************** */

#define MNG_ERROR(D,C)      { mng_process_error (D, C, 0, 0); return C; }
#define MNG_ERRORZ(D,Z)     { mng_process_error (D, MNG_ZLIBERROR, Z, 0); return MNG_ZLIBERROR; }
#define MNG_ERRORJ(D,J)     { mng_process_error (D, MNG_JPEGERROR, J, 0); return MNG_JPEGERROR; }
#define MNG_ERRORL(D,L)     { mng_process_error (D, MNG_LCMSERROR, L, 0); return MNG_LCMSERROR; }
#define MNG_WARNING(D,C)    { if (!mng_process_error (D, C, 0, 0)) return C; }

#define MNG_VALIDHANDLE(H)  { if ((H == 0) || (((mng_datap)H)->iMagic != MNG_MAGIC)) \
                                return MNG_INVALIDHANDLE; }
#define MNG_VALIDHANDLEX(H) { if ((H == 0) || (((mng_datap)H)->iMagic != MNG_MAGIC)) \
                                return 0; }
#define MNG_VALIDCB(D,C)    { if (!((mng_datap)D)->C) \
                                MNG_ERROR (((mng_datap)D), MNG_NOCALLBACK) }

/* ************************************************************************** */

#endif /* _mng_error_h_ */

/* ************************************************************************** */
/* * end of file                                                            * */
/* ************************************************************************** */
