LIBMNG_PREFIX = /opt/gnome-1.2

CC = gcc

CFLAGS = -g -O2 -Wall \
         `gtk-config --cflags` \
         -I$(LIBMNG_PREFIX)/include

LIBS = `gtk-config --libs` \
      -L$(LIBMNG_PREFIX)/lib -lmng -ljpeg -lz

OBJ = dummy.o \
      gtk-mng-view.o

all: dummy

dummy:$(OBJ)
	$(CC) -o dummy $(OBJ) $(LIBS)

.c.o: gtk-mng-viewer.h
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f core dummy $(OBJ)
