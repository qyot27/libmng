#ifndef _mozlibmng_conf_h_
#define _mozlibmng_conf_h_

/* Mozilla defines */

#if 1
/* Perform footprint optimizations */
#define MNG_OPTIMIZE_FOOTPRINT_COMPOSE
#define MNG_OPTIMIZE_FOOTPRINT_DIV
#define MNG_OPTIMIZE_FOOTPRINT_SWITCH
#define MNG_DECREMENT_LOOPS
#define MNG_USE_ZLIB_CRC
#define MNG_OPTIMIZE_FOOTPRINT_INIT
#endif

#if 1
/* eliminate unused features from libmng */
#define MNG_NO_OLD_VERSIONS
#define MNG_SKIPCANVAS_ABGR8
#define MNG_SKIPCANVAS_ARGB8
#if defined(XP_WIN) || defined(XP_OS2) || defined(XP_BEOS) || defined(MOZ_WIDGET_PHOTON)
#define MNG_SKIPCANVAS_RGB8
#define MNG_SKIPCANVAS_RGBX8
#else
#define MNG_SKIPCANVAS_BGR8
#define MNG_SKIPCANVAS_BGRX8
#endif
#define MNG_SKIPCANVAS_BGRA8
#define MNG_SKIPCANVAS_BGRA8_PM
#define MNG_SKIPCANVAS_RGBA8
#define MNG_SKIP_MAXCANVAS
#define MNG_SKIPCHUNK_tEXt
#define MNG_SKIPCHUNK_zTXt
#define MNG_SKIPCHUNK_iTXt
#define MNG_SKIPCHUNK_bKGD
#define MNG_SKIPCHUNK_cHRM
#define MNG_SKIPCHUNK_hIST
#define MNG_SKIPCHUNK_iCCP
#define MNG_SKIPCHUNK_pHYs
#define MNG_SKIPCHUNK_sBIT
#define MNG_SKIPCHUNK_sPLT
#define MNG_SKIPCHUNK_tIME
#define MNG_SKIPCHUNK_evNT
#define MNG_SKIPCHUNK_eXPI
#define MNG_SKIPCHUNK_fPRI
#define MNG_SKIPCHUNK_nEED
#define MNG_SKIPCHUNK_pHYg
/* Eliminate "critical" but safe-to-ignore chunks (see mng_read_unknown()) */
#define MNG_SKIPCHUNK_SAVE
#define MNG_SKIPCHUNK_SEEK
#define MNG_SKIPCHUNK_DBYK
#define MNG_SKIPCHUNK_ORDR
/* Eliminate unused zlib and jpeg "get" and "set" accessors */
#define MNG_NO_ACCESS_ZLIB
#define MNG_NO_ACCESS_JPEG
/* Eliminate other unused features */
#define MNG_NO_SUPPORT_FUNCQUERY
#define MNG_NO_DISPLAY_GO_SUPPORTED
#define MNG_NO_CURRENT_INFO
#define MNG_NO_DFLT_INFO
#define MNG_NO_LOOP_SIGNALS_SUPPORTED
#endif

#if 1
/* Do all MAGN operations in RGBA8 space */
#define MNG_OPTIMIZE_FOOTPRINT_MAGN
#endif

#if 1
/* eliminate 16-bit support from libmng */
#define MNG_NO_16BIT_SUPPORT
#endif

#if 1
#define MNG_SKIPCHUNK_PAST
/* eliminate PAST feature from libmng */
#endif

#if 1
/* eliminate Delta-PNG feature from libmng */
#define MNG_NO_DELTA_PNG
#endif

#if 1
/* If you change this you should also manually remove or restore
   jng-recognition in mozilla/modules/libpr0n/src/imgLoader.cpp */
#define MNG_NO_INCLUDE_JNG
#endif

#if 1
/* Recognize MNG-VLC chunks only */
#define MNG_SKIPCHUNK_BASI
#define MNG_SKIPCHUNK_CLIP
#define MNG_SKIPCHUNK_CLON
#define MNG_SKIPCHUNK_DISC
#define MNG_SKIPCHUNK_DEFI
#define MNG_SKIPCHUNK_FRAM
#define MNG_SKIPCHUNK_LOOP
#define MNG_SKIPCHUNK_MAGN
#define MNG_SKIPCHUNK_MOVE
#define MNG_SKIPCHUNK_SHOW
#endif

#endif /* _mozlibmng_conf_h */
