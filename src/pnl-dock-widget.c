/* pnl-dock-widget.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pnl-dock-header.h"
#include "pnl-dock-widget.h"
#include "pnl-multi-paned.h"

#define SNAPSHOT_WIDTH 300
#define SNAPSHOT_HEIGHT 300

typedef struct
{
  GtkBox        *box;
  PnlDockHeader *title;
  GtkWidget     *custom_title;
  GtkRevealer   *revealer;
} PnlDockWidgetPrivate;

G_DEFINE_TYPE_EXTENDED (PnlDockWidget, pnl_dock_widget, GTK_TYPE_BIN, 0,
                        G_ADD_PRIVATE (PnlDockWidget)
                        G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_ORIENTATION,
  PROP_REVEAL_CHILD,
  PROP_SHOW_CLOSE_BUTTON,
  PROP_TITLE,
  N_PROPS
};

enum {
  BEGIN_DRAG,
  CLOSE,
  N_SIGNALS
};

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static void
pnl_dock_widget_notify_child_revealed (PnlDockWidget *self,
                                       GParamSpec    *pspec,
                                       GtkRevealer   *revealer)
{
  GtkWidget *child;

  g_assert (PNL_IS_DOCK_WIDGET (self));
  g_assert (pspec != NULL);
  g_assert (GTK_IS_REVEALER (revealer));

  if (!gtk_revealer_get_child_revealed (revealer) &&
      NULL != (child = gtk_bin_get_child (GTK_BIN (revealer))))
    gtk_widget_set_size_request (child, -1, -1);
}

static GtkOrientation
pnl_dock_widget_get_orientation (PnlDockWidget *self)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_WIDGET (self), GTK_ORIENTATION_HORIZONTAL);

  return gtk_orientable_get_orientation (GTK_ORIENTABLE (priv->box));
}

static void
pnl_dock_widget_set_orientation (PnlDockWidget  *self,
                                 GtkOrientation  orientation)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_WIDGET (self));

  gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->box), orientation);
  gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->title), !orientation);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_revealer_set_transition_type (priv->revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
  else
    gtk_revealer_set_transition_type (priv->revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);

  if (GTK_IS_ORIENTABLE (priv->custom_title))
    gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->custom_title), orientation);
}

static void
pnl_dock_widget_add (GtkContainer *container,
                     GtkWidget    *widget)
{
  PnlDockWidget *self = (PnlDockWidget *)container;
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_assert (PNL_IS_DOCK_WIDGET (self));
  g_assert (GTK_IS_WIDGET (widget));

  gtk_container_add (GTK_CONTAINER (priv->revealer), widget);
}

static gboolean
pnl_dock_widget_real_close (PnlDockWidget *self)
{
  g_assert (PNL_IS_DOCK_WIDGET (self));

  gtk_widget_destroy (GTK_WIDGET (self));

  return TRUE;
}

static cairo_surface_t *
pnl_dock_widget_snapshot (PnlDockWidget *self)
{
  cairo_surface_t *surface;
  cairo_t *cr;
  GtkAllocation alloc;

  g_assert (PNL_IS_DOCK_WIDGET (self));

  if (!gtk_widget_get_realized (GTK_WIDGET (self)) ||
      !gtk_widget_get_visible (GTK_WIDGET (self)))
    return NULL;

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  /* TODO: Scale size we want to render */

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, SNAPSHOT_WIDTH, SNAPSHOT_HEIGHT);

  cr = cairo_create (surface);

  /*
   * Draw the widget clipped to our source surface.
   */
  gtk_widget_draw (GTK_WIDGET (self), cr);

  /*
   * Change the alpha of the destination.
   */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba (cr, 0, 0, 0, 0);
  cairo_paint_with_alpha (cr, 0.25);

  /*
   * Cleanup after everything.
   */
  cairo_destroy (cr);

  return surface;
}

static void
pnl_dock_widget_real_begin_drag (PnlDockWidget *self,
                                 gint           button,
                                 GdkEvent      *event,
                                 gint           x,
                                 gint           y)
{
  GtkTargetList *target_list = NULL;
  GdkDragContext *drag_context = NULL;
  cairo_surface_t *snapshot;
  static const GtkTargetEntry entries[] = {
    { (gchar *)"PNL_DOCK_WIDGET", GTK_TARGET_SAME_APP, 0 },
  };

  g_assert (PNL_IS_DOCK_WIDGET (self));
  g_assert (event != NULL);

  target_list = gtk_target_list_new (entries, G_N_ELEMENTS (entries));

  drag_context = gtk_drag_begin_with_coordinates (GTK_WIDGET (self),
                                                  target_list,
                                                  GDK_ACTION_MOVE,
                                                  button,
                                                  event,
                                                  x, y);

  gdk_drag_motion (drag_context,
                   NULL,
                   GDK_DRAG_PROTO_LOCAL,
                   x, y,
                   GDK_ACTION_MOVE,
                   GDK_ACTION_MOVE,
                   GDK_CURRENT_TIME);

  snapshot = pnl_dock_widget_snapshot (self);

  if (snapshot != NULL)
    gtk_drag_set_icon_surface (drag_context, snapshot);

  g_clear_pointer (&target_list, gtk_target_list_unref);
  g_clear_pointer (&snapshot, cairo_surface_destroy);
}

static void
border_sum (GtkBorder *one,
            GtkBorder *two)
{
  one->top += two->top;
  one->right += two->right;
  one->bottom += two->bottom;
  one->left += two->left;
}

static gboolean
pnl_dock_widget_draw (GtkWidget *widget,
                      cairo_t   *cr)
{
  GtkStyleContext *style_context;
  GtkAllocation alloc;
  GtkStateFlags state;
  GtkBorder border;
  GtkBorder padding;

  g_assert (GTK_IS_WIDGET (widget));
  g_assert (cr != NULL);

  gtk_widget_get_allocation (widget, &alloc);

  style_context = gtk_widget_get_style_context (widget);
  state = gtk_style_context_get_state (style_context);
  gtk_style_context_get_border (style_context, state, &border);
  gtk_style_context_get_padding (style_context, state, &padding);

  border_sum (&border, &padding);

  gtk_render_background (gtk_widget_get_style_context (widget), cr,
                         border.left,
                         border.top,
                         alloc.width - border.left - border.right,
                         alloc.height - border.top - border.bottom);

  return GTK_WIDGET_CLASS (pnl_dock_widget_parent_class)->draw (widget, cr);
}

static void
pnl_dock_widget_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PnlDockWidget *self = PNL_DOCK_WIDGET (object);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, pnl_dock_widget_get_orientation (self));
      break;

    case PROP_REVEAL_CHILD:
      g_value_set_boolean (value, pnl_dock_widget_get_reveal_child (self));
      break;

    case PROP_SHOW_CLOSE_BUTTON:
      g_value_set_boolean (value, pnl_dock_widget_get_show_close_button (self));
      break;

    case PROP_TITLE:
      g_value_set_string (value, pnl_dock_widget_get_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_widget_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PnlDockWidget *self = PNL_DOCK_WIDGET (object);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      pnl_dock_widget_set_orientation (self, g_value_get_enum (value));
      break;

    case PROP_REVEAL_CHILD:
      pnl_dock_widget_set_reveal_child (self, g_value_get_boolean (value));
      break;

    case PROP_SHOW_CLOSE_BUTTON:
      pnl_dock_widget_set_show_close_button (self, g_value_get_boolean (value));
      break;

    case PROP_TITLE:
      pnl_dock_widget_set_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_widget_class_init (PnlDockWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = pnl_dock_widget_get_property;
  object_class->set_property = pnl_dock_widget_set_property;

  widget_class->draw = pnl_dock_widget_draw;

  container_class->add = pnl_dock_widget_add;

  klass->close = pnl_dock_widget_real_close;
  klass->begin_drag = pnl_dock_widget_real_begin_drag;

  properties [PROP_ORIENTATION] =
    g_param_spec_enum ("orientation",
                       "Orientation",
                       "Orientation",
                       GTK_TYPE_ORIENTATION,
                       GTK_ORIENTATION_HORIZONTAL,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_REVEAL_CHILD] =
    g_param_spec_boolean ("reveal-child",
                          "Reveal Child",
                          "Reveal Child",
                          TRUE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_CLOSE_BUTTON] =
    g_param_spec_boolean ("show-close-button",
                         "Show Close Button",
                         "If the close button should be shown on the widget header",
                         FALSE,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  signals [BEGIN_DRAG] =
    g_signal_new ("begin-drag",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PnlDockWidgetClass, begin_drag),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 4, G_TYPE_INT, GDK_TYPE_EVENT, G_TYPE_INT, G_TYPE_INT);

  signals [CLOSE] =
    g_signal_new ("close",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PnlDockWidgetClass, close),
                  g_signal_accumulator_true_handled, NULL, NULL,
                  G_TYPE_BOOLEAN, 0);

  gtk_widget_class_set_css_name (widget_class, "dockwidget");
}

static void
pnl_dock_widget_init (PnlDockWidget *self)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  priv->box = g_object_new (GTK_TYPE_BOX,
                            "orientation", GTK_ORIENTATION_HORIZONTAL,
                            "hexpand", TRUE,
                            "homogeneous", FALSE,
                            "visible", TRUE,
                            NULL);
  GTK_CONTAINER_CLASS (pnl_dock_widget_parent_class)->add (GTK_CONTAINER (self),
                                                           GTK_WIDGET (priv->box));

  priv->title = g_object_new (PNL_TYPE_DOCK_HEADER,
                              "expand", FALSE,
                              "visible", TRUE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (priv->box), GTK_WIDGET (priv->title));

  g_object_bind_property (self, "reveal-child", priv->title, "expanded",
                          G_BINDING_BIDIRECTIONAL);

  priv->revealer = g_object_new (GTK_TYPE_REVEALER,
                                 "expand", TRUE,
                                 "transition-type", GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT,
                                 "visible", TRUE,
                                 NULL);
  gtk_container_add (GTK_CONTAINER (priv->box), GTK_WIDGET (priv->revealer));

  g_signal_connect_object (priv->revealer,
                           "notify::child-revealed",
                           G_CALLBACK (pnl_dock_widget_notify_child_revealed),
                           self,
                           G_CONNECT_SWAPPED);
}

GtkWidget *
pnl_dock_widget_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_WIDGET, NULL);
}

gboolean
pnl_dock_widget_get_reveal_child (PnlDockWidget *self)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_WIDGET (self), FALSE);

  return gtk_revealer_get_reveal_child (priv->revealer);
}

void
pnl_dock_widget_set_reveal_child (PnlDockWidget *self,
                                  gboolean       reveal_child)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_assert (PNL_IS_DOCK_WIDGET (self));

  reveal_child = !!reveal_child;

  if (reveal_child != gtk_revealer_get_reveal_child (priv->revealer))
    {
      if (!reveal_child)
        {
          GtkWidget *child;

          /*
           * HACK:
           *
           * When the multi-paned has allocated more space to the child than it
           * requires due to a resize-drag, we need to mess with the
           * size-request during the animation cycle. Otherwise, the revealer
           * wont actually animate out the child widget.
           */

          if (NULL != (child = gtk_bin_get_child (GTK_BIN (priv->revealer))))
            {
              GtkAllocation child_alloc;
              GtkWidget *parent;

              gtk_widget_get_allocation (child, &child_alloc);

              if (pnl_dock_widget_get_orientation (self) == GTK_ORIENTATION_HORIZONTAL)
                gtk_widget_set_size_request (child, child_alloc.width, -1);
              else
                gtk_widget_set_size_request (child, -1, child_alloc.height);

              parent = gtk_widget_get_parent (GTK_WIDGET (self));

              if (PNL_IS_MULTI_PANED (parent))
                gtk_container_child_set (GTK_CONTAINER (parent), GTK_WIDGET (self),
                                         "position", 0,
                                         NULL);
            }
        }

      gtk_revealer_set_reveal_child (priv->revealer, reveal_child);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_REVEAL_CHILD]);
    }
}

/**
 * pnl_dock_widget_get_custom_title:
 * @self: A #PnlDockWidget.
 *
 * Returns: (transfer none) (nullable): A #GtkWidget or %NULL.
 */
GtkWidget *
pnl_dock_widget_get_custom_title (PnlDockWidget *self)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_WIDGET (self), NULL);

  return priv->custom_title;
}

void
pnl_dock_widget_set_custom_title (PnlDockWidget *self,
                                  GtkWidget     *custom_title)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_WIDGET (self));
  g_return_if_fail (!custom_title || GTK_IS_WIDGET (custom_title));

  if (priv->custom_title != NULL)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->custom_title),
                                    (gpointer *)&priv->custom_title);
      g_clear_pointer (&priv->custom_title, gtk_widget_destroy);
      gtk_widget_show (GTK_WIDGET (priv->title));
    }

  if (custom_title != NULL)
    {
      priv->custom_title = custom_title;
      g_object_add_weak_pointer (G_OBJECT (custom_title),
                                 (gpointer *)&priv->custom_title);
      gtk_container_add_with_properties (GTK_CONTAINER (priv->box), custom_title,
                                         "position", 1,
                                         NULL);
      gtk_widget_hide (GTK_WIDGET (priv->title));
    }
}

const gchar *
pnl_dock_widget_get_title (PnlDockWidget *self)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_WIDGET (self), NULL);

  return pnl_dock_header_get_title (priv->title);
}

void
pnl_dock_widget_set_title (PnlDockWidget *self,
                           const gchar   *title)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_WIDGET (self));

  if (g_strcmp0 (title, pnl_dock_widget_get_title (self)) != 0)
    {
      pnl_dock_header_set_title (priv->title, title);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TITLE]);
    }
}

gboolean
pnl_dock_widget_get_show_close_button (PnlDockWidget *self)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_WIDGET (self), FALSE);

  return pnl_dock_header_get_show_close_button (priv->title);
}

void
pnl_dock_widget_set_show_close_button (PnlDockWidget *self,
                                       gboolean       show_close_button)
{
  PnlDockWidgetPrivate *priv = pnl_dock_widget_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_WIDGET (self));

  show_close_button = !!show_close_button;

  if (show_close_button != pnl_dock_widget_get_show_close_button (self))
    {
      pnl_dock_header_set_show_close_button (priv->title, show_close_button);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SHOW_CLOSE_BUTTON]);
    }
}

void
pnl_dock_widget_close (PnlDockWidget *self)
{
  gboolean ret = FALSE;

  g_return_if_fail (PNL_IS_DOCK_WIDGET (self));

  g_signal_emit (self, signals [CLOSE], 0, &ret);
}

void
pnl_dock_widget_begin_drag (PnlDockWidget *self,
                            gint           button,
                            GdkEvent      *event,
                            gint           x,
                            gint           y)
{
  g_return_if_fail (PNL_IS_DOCK_WIDGET (self));
  g_return_if_fail (event != NULL);

  g_signal_emit (self, signals [BEGIN_DRAG], 0, button, event, x, y);
}
