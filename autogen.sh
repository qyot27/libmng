# autogen.sh
#
# invoke the auto* tools to create the configureation system

# move out configure.in
if ! test -f configure.in; then
  echo "copying out configure.in"
  ln -s makefiles/configure.in
fi

# move out the macros and run aclocal
if ! test -f acinclude.m4; then
  echo "copying configure macros"
  ln -s makefiles/acinclude.m4 .
fi
aclocal

# set up libtool
libtoolize

# copy up our Makefile template and invoke automake
if ! test -f Makefile.am; then
  echo "copying automake template"
  ln -s makefiles/Makefile.am .
fi
automake --foreign --add-missing

# build the configure script
autoconf

# and invoke it
./configure $*

# end
