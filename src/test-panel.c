#include "pnl.h"

typedef enum
{
   COMMAND_SHOW,
   COMMAND_HIDE,
   COMMAND_EXIT
} Command;

#define INTERVAL 500

static struct {
  gint interval;
  Command command;
} commands[] = {
  { 1000, COMMAND_SHOW },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { INTERVAL, COMMAND_SHOW },
  { INTERVAL, COMMAND_HIDE },
  { 300, COMMAND_EXIT },
};
static gint current_command;
static GtkWidget *dockbin;
static GtkWidget *class_entry;
static GTimer *timer;

static void
toggle_all (gboolean show)
{
  PnlDockRevealer *edge;
  PnlDockBin *dock = PNL_DOCK_BIN (dockbin);

  edge = PNL_DOCK_REVEALER (pnl_dock_bin_get_left_edge (dock));
  pnl_dock_revealer_set_reveal_child (edge, show);

  edge = PNL_DOCK_REVEALER (pnl_dock_bin_get_right_edge (dock));
  pnl_dock_revealer_set_reveal_child (edge, show);

  edge = PNL_DOCK_REVEALER (pnl_dock_bin_get_bottom_edge (dock));
  pnl_dock_revealer_set_reveal_child (edge, show);
}

static void
adjust_sizes (void)
{
  PnlDockBin *dock = PNL_DOCK_BIN (dockbin);
  GtkWidget *edge;

  edge = pnl_dock_bin_get_left_edge (dock);
  pnl_dock_revealer_set_position (PNL_DOCK_REVEALER (edge), 300);

  edge = pnl_dock_bin_get_right_edge (dock);
  pnl_dock_revealer_set_position (PNL_DOCK_REVEALER (edge), 300);

  edge = pnl_dock_bin_get_bottom_edge (dock);
  pnl_dock_revealer_set_position (PNL_DOCK_REVEALER (edge), 300);
}

static void
grab_class_entry (GtkWidget *button)
{
  gtk_widget_grab_focus (class_entry);
}

static gboolean
process_command (gpointer data)
{
  switch (commands [current_command].command)
    {
    case COMMAND_SHOW:
      toggle_all (TRUE);
      break;

    case COMMAND_HIDE:
      toggle_all (FALSE);
      break;

    case COMMAND_EXIT:
    default:
      gtk_main_quit ();
      return G_SOURCE_REMOVE;
    }

  g_timeout_add (commands [++current_command].interval,
                 process_command,
                 NULL);

  return G_SOURCE_REMOVE;
}

static void
log_handler (const gchar    *domain,
             GLogLevelFlags  flags,
             const gchar    *message,
             gpointer        user_data)
{
  gdouble t = g_timer_elapsed (timer, NULL);

  g_print ("%s: time=%0.5lf %s\n", domain, t, message);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GActionGroup *group;
  GtkWidget *dockoverlay;
  GtkCssProvider *provider;
  GError *error = NULL;

  gtk_init (&argc, &argv);

  timer = g_timer_new ();

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_path (provider, "test-panel.css", &error);
  g_assert_no_error (error);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, "test-panel.ui", &error);
  g_assert_no_error (error);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  dockoverlay = GTK_WIDGET (gtk_builder_get_object (builder, "dockoverlay"));
  _pnl_dock_item_printf (PNL_DOCK_ITEM (dockoverlay));

  dockbin = GTK_WIDGET (gtk_builder_get_object (builder, "dockbin"));
  group = gtk_widget_get_action_group (dockbin, "dockbin");
  gtk_widget_insert_action_group (GTK_WIDGET (window), "dockbin", group);

  adjust_sizes ();

  class_entry = GTK_WIDGET (gtk_builder_get_object (builder, "class_entry"));

#if 0
  gtk_builder_add_callback_symbol (builder, "toggle_all", G_CALLBACK (toggle_all));
#endif
  gtk_builder_add_callback_symbol (builder, "grab_class_entry", G_CALLBACK (grab_class_entry));
  gtk_builder_connect_signals (builder, dockbin);

  if (0)
  g_timeout_add (commands [0].interval,
                 process_command,
                 NULL);

  g_log_set_default_handler (log_handler, NULL);

  gtk_window_present (window);
  gtk_window_maximize (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
