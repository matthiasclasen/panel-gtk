#include "pnl.h"

gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GActionGroup *group;
  GtkWidget *dockbin;
  GError *error = NULL;

  gtk_init (&argc, &argv);

  g_type_ensure (PNL_TYPE_DOCK_BIN);
  g_type_ensure (PNL_TYPE_DOCK_OVERLAY);
  g_type_ensure (PNL_TYPE_DOCK_PANED);
  g_type_ensure (PNL_TYPE_DOCK_STACK);
  g_type_ensure (PNL_TYPE_DOCK_WIDGET);

  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, "test-panel.ui", &error);
  g_assert_no_error (error);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  dockbin = GTK_WIDGET (gtk_builder_get_object (builder, "dockbin"));
  group = gtk_widget_get_action_group (dockbin, "dockbin");
  gtk_widget_insert_action_group (GTK_WIDGET (window), "dockbin", group);

  gtk_window_present (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
