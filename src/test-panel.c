#include "pnl-dock.h"

static void
add_child (GtkWidget *parent)
{
  GtkWidget *child;

  child = g_object_new (GTK_TYPE_LABEL,
                        "label", "This is the label",
                        "visible", TRUE,
                        NULL);
  gtk_container_add (GTK_CONTAINER (parent), child);
}

static void
toggle_all (GtkButton *button,
            PnlDock   *dock)
{
  GtkRevealer *edge;

  edge = GTK_REVEALER (pnl_dock_get_left_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));

  edge = GTK_REVEALER (pnl_dock_get_right_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));

  edge = GTK_REVEALER (pnl_dock_get_top_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));

  edge = GTK_REVEALER (pnl_dock_get_bottom_edge (dock));
  gtk_revealer_set_reveal_child (edge, !gtk_revealer_get_reveal_child (edge));
}

gint
main (gint   argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *left;
  GtkWidget *right;
  GtkWidget *bottom;
  GtkWidget *top;
  PnlDock *dock;
  GtkWidget *scroller;
  GtkTextView *textview;
  GtkHeaderBar *header;
  GtkCssProvider *provider;
  GtkBox *box;
  GtkWidget *button;
  GActionGroup *group;
  GError *error = NULL;

  gtk_init (&argc, &argv);

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_path (provider, "test-panel.css", &error);
  g_assert_no_error (error);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 800,
                         "default-height", 600,
                         NULL);

  header = g_object_new (GTK_TYPE_HEADER_BAR,
                         "title", "Panel-Gtk Test App",
                         "show-close-button", TRUE,
                         "visible", TRUE,
                         NULL);
  gtk_window_set_titlebar (GTK_WINDOW (window), GTK_WIDGET (header));

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);
  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (box)), "linked");
  gtk_container_add_with_properties (GTK_CONTAINER (header), GTK_WIDGET (box),
                                     "pack-type", GTK_PACK_END,
                                     "position", 0,
                                     NULL);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "action-name", "dock.left-visible",
                         "visible", TRUE,
                         "label", "Left",
                         NULL);
  gtk_container_add (GTK_CONTAINER (box), button);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "action-name", "dock.right-visible",
                         "visible", TRUE,
                         "label", "Right",
                         NULL);
  gtk_container_add (GTK_CONTAINER (box), button);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "action-name", "dock.top-visible",
                         "visible", TRUE,
                         "label", "Top",
                         NULL);
  gtk_container_add (GTK_CONTAINER (box), button);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "action-name", "dock.bottom-visible",
                         "visible", TRUE,
                         "label", "Bottom",
                         NULL);
  gtk_container_add (GTK_CONTAINER (box), button);

  dock = g_object_new (PNL_TYPE_DOCK,
                       "visible", TRUE,
                       NULL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (dock));

  group = gtk_widget_get_action_group (GTK_WIDGET (dock), "dock");
  gtk_widget_insert_action_group (GTK_WIDGET (header), "dock", group);

  left = pnl_dock_get_left_edge (dock);
  add_child (left);

  right = pnl_dock_get_right_edge (dock);
  add_child (right);

  top = pnl_dock_get_top_edge (dock);
  add_child (top);

  bottom = pnl_dock_get_bottom_edge (dock);
  add_child (bottom);

  scroller = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "min-content-width", 300,
                           "min-content-height", 300,
                           "visible", TRUE,
                           NULL);
  gtk_container_add (GTK_CONTAINER (dock), scroller);

  textview = g_object_new (GTK_TYPE_TEXT_VIEW,
                           "visible", TRUE,
                           NULL);
  gtk_container_add (GTK_CONTAINER (scroller), GTK_WIDGET (textview));

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "visible", TRUE,
                         "label", "Toggle All",
                         NULL);
  g_signal_connect (button, "clicked", G_CALLBACK (toggle_all), dock);
  gtk_container_add (GTK_CONTAINER (header), button);

  gtk_window_present (GTK_WINDOW (window));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_main ();

  return 0;
}
