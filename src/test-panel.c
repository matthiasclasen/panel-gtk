#include "pnl.h"

static void
toggle_all (GtkButton  *button,
            PnlDockBin *dock)
{
  GtkRevealer *edge;

  edge = GTK_REVEALER (pnl_dock_bin_get_left_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));

  edge = GTK_REVEALER (pnl_dock_bin_get_right_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));

  edge = GTK_REVEALER (pnl_dock_bin_get_bottom_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));
}


gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GActionGroup *group;
  GtkWidget *dockbin;
  GtkCssProvider *provider;
  GError *error = NULL;

  gtk_init (&argc, &argv);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_path (provider, "test-panel.css", &error);
  g_assert_no_error (error);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  builder = gtk_builder_new ();
  gtk_builder_add_callback_symbol (builder, "toggle_all", G_CALLBACK (toggle_all));
  gtk_builder_add_from_file (builder, "test-panel.ui", &error);
  g_assert_no_error (error);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  dockbin = GTK_WIDGET (gtk_builder_get_object (builder, "dockbin"));
  group = gtk_widget_get_action_group (dockbin, "dockbin");
  gtk_widget_insert_action_group (GTK_WIDGET (window), "dockbin", group);

  gtk_builder_connect_signals (builder, dockbin);

  gtk_window_present (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
