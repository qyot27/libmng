dnl autoconf macros for detecting libmng
dnl add this to your aclocal or acinclude to make use of it
dnl
dnl (c) 2000 Ralph Giles <giles@ashlu.bc.ca>
dnl

dnl A basic check: looks for libmng and it's dependencies
dnl and adds the required bits to CFLAGS and LIBS

# check for libmng
AC_DEFUN(LIBMNG_CHECK, [
  dnl prerequisites first
  AC_CHECK_LIB(jpeg, jpeg_set_defaults)
  dnl now the library
  AC_CHECK_LIB(mng, mng_readdisplay, [], [
	dnl try it with lcms -- optional link dependency
	AC_CHECK_LIB(lcms, cmsCreateRGBProfile, [
		AC_CHECK_HEADER(lcms/lcms.h)
		AC_CHECK_LIB(mng, mng_readdisplay)
	])
  ])
  AC_CHECK_HEADER(libmng.h)
])

dnl end LIBMNG macros
