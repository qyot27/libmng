/*
 *  Very simple program, that has been used by us
 *  (V. Babin <babin@users.sourceforge.net> and
 *   S. Kondrat <valiska@users.sourceforge.net>)
 *  during toying with libmng (http://www.limng.com)
 *
 *  License: GPL :-))
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "gtk-mng-view.h"

#define BLOCK_SIZE  4096

static guint
read_file (const gchar* file_name, guchar ** ptr)
{
  gint fd;
  guint size = 0;
  guint bytes_read = 0;

  if ((fd = open (file_name, O_RDONLY)) == -1)
    {
      perror (file_name);
      * ptr = NULL; 
      return 0;
    }

  * ptr = g_new (guchar, BLOCK_SIZE);
  while ((bytes_read = read (fd, * ptr + size, BLOCK_SIZE)))
    {
      size += bytes_read;
      * ptr = (guchar *) g_realloc (* ptr, size + BLOCK_SIZE);
    }
  close (fd);

  * ptr = (guchar *) g_realloc (* ptr, size);
  return size;
}

int
main (int argc, char ** argv)
{
  GtkMngView * mng_view;
  GtkWidget * window;
  GtkWidget * align;
  GtkWidget * frame;
  guchar * mng_data = NULL;
  guint mng_data_size;

  if (argc < 2)
    {
      g_print ("Usage: dummy <file.mng>\n");
      return 1;
    }

  mng_data_size = read_file (* (argv + 1), &mng_data);

  if (mng_data == NULL)
    return 1;

  gtk_init (&argc, &argv);
  gdk_rgb_init ();
  gdk_rgb_set_verbose (TRUE);
  gtk_widget_set_default_visual (gdk_rgb_get_visual ());
  gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());

  window = gtk_widget_new (GTK_TYPE_WINDOW,
			   "GtkWindow::type", GTK_WINDOW_TOPLEVEL,
			   "GtkWindow::title", "MNG animation",
			   "GtkContainer::border_width", 5,
			   NULL);
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (window), align);
  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (align), frame);

  /* actually it */
  mng_view = GTK_MNG_VIEW (gtk_mng_view_new ());
  gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (mng_view));

  gtk_mng_view_load_mng_from_memory (mng_view, mng_data, mng_data_size);
  g_free (mng_data);

  /* rest in piece */
  gtk_widget_show_all (window);
  gtk_main ();

  return 0;
}