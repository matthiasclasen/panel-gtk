#include "pnl.h"

gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GError *error = NULL;

  gtk_init (&argc, &argv);

  g_type_ensure (PNL_TYPE_DOCK_BIN);
  g_type_ensure (PNL_TYPE_DOCK_OVERLAY);
  g_type_ensure (PNL_TYPE_DOCK_PANED);

  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, "test-panel.ui", &error);
  g_assert_no_error (error);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_window_present (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
