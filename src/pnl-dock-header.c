/* pnl-dock-header.c
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

typedef struct
{
  GtkBox         *box;
  GtkImage       *arrow;
  GtkLabel       *title;
  GtkButton      *close;
  GtkWidget      *custom_title;
  guint           expanded : 1;
  GtkOrientation  orientation : 1;
} PnlDockHeaderPrivate;

G_DEFINE_TYPE_EXTENDED (PnlDockHeader, pnl_dock_header, GTK_TYPE_EVENT_BOX, 0,
                        G_ADD_PRIVATE (PnlDockHeader)
                        G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_EXPANDED,
  PROP_ORIENTATION,
  PROP_SHOW_CLOSE_BUTTON,
  PROP_TITLE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
pnl_dock_header_close_clicked (PnlDockHeader *self,
                               GtkButton     *button)
{
  GtkWidget *parent;

  g_assert (PNL_IS_DOCK_HEADER (self));
  g_assert (GTK_IS_BUTTON (button));

  parent = gtk_widget_get_ancestor (GTK_WIDGET (self), PNL_TYPE_DOCK_WIDGET);

  if (parent != NULL)
    pnl_dock_widget_close (PNL_DOCK_WIDGET (parent));
}

gboolean
pnl_dock_header_get_expanded (PnlDockHeader *self)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_HEADER (self), FALSE);

  return priv->expanded;
}

static void
pnl_dock_header_update_expanded (PnlDockHeader *self)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_assert (PNL_IS_DOCK_HEADER (self));

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      gtk_image_set_from_icon_name (priv->arrow,
                                    priv->expanded ? "pan-down-symbolic" : "pan-end-symbolic",
                                    GTK_ICON_SIZE_SMALL_TOOLBAR);
    }
  else
    {
      gtk_image_set_from_icon_name (priv->arrow,
                                    priv->expanded ? "pan-end-symbolic" : "pan-up-symbolic",
                                    GTK_ICON_SIZE_SMALL_TOOLBAR);
    }
}

void
pnl_dock_header_set_expanded (PnlDockHeader *self,
                              gboolean       expanded)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_HEADER (self));

  expanded = !!expanded;

  if (expanded != priv->expanded)
    {
      priv->expanded = expanded;
      pnl_dock_header_update_expanded (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_EXPANDED]);
    }
}

static gboolean
pnl_dock_header_button_release_event (GtkWidget      *widget,
                                      GdkEventButton *button)
{
  PnlDockHeader *self = (PnlDockHeader *)widget;

  g_assert (PNL_IS_DOCK_HEADER (self));
  g_assert (button != NULL);

  if (button->button == GDK_BUTTON_PRIMARY)
    {
      pnl_dock_header_set_expanded (self, !pnl_dock_header_get_expanded (self));
      return GDK_EVENT_STOP;
    }

  return GDK_EVENT_PROPAGATE;
}

static gboolean
pnl_dock_header_enter_notify_event (GtkWidget        *widget,
                                    GdkEventCrossing *crossing)
{
  PnlDockHeader *self = (PnlDockHeader *)widget;

  g_assert (PNL_IS_DOCK_HEADER (self));
  g_assert (crossing != NULL);

  gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_PRELIGHT, FALSE);

  return GDK_EVENT_PROPAGATE;
}

static gboolean
pnl_dock_header_leave_notify_event (GtkWidget        *widget,
                                    GdkEventCrossing *crossing)
{
  PnlDockHeader *self = (PnlDockHeader *)widget;

  g_assert (PNL_IS_DOCK_HEADER (self));
  g_assert (crossing != NULL);

  gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_PRELIGHT);

  return GDK_EVENT_PROPAGATE;
}

static void
swap_pack_type (GtkWidget *widget,
                gpointer   user_data)
{
  GtkWidget *parent = gtk_widget_get_parent (widget);
  GtkPackType *pack_type = user_data;

  g_assert (GTK_IS_BOX (parent));
  g_assert (pack_type != NULL);

  gtk_container_child_set (GTK_CONTAINER (parent), widget,
                           "pack-type", *pack_type,
                           NULL);
}

static void
pnl_dock_header_set_orientation (PnlDockHeader  *self,
                                 GtkOrientation  orientation)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);
  GtkPackType pack_type;

  g_assert (PNL_IS_DOCK_HEADER (self));

  priv->orientation = orientation;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (priv->box), orientation);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      pack_type = GTK_PACK_START;
      gtk_label_set_angle (priv->title, 0.0);
      gtk_widget_set_halign (GTK_WIDGET (self), GTK_ALIGN_FILL);
      gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_START);
    }
  else
    {
      pack_type = GTK_PACK_END;
      gtk_label_set_angle (priv->title, 90.0);
      gtk_widget_set_halign (GTK_WIDGET (self), GTK_ALIGN_START);
      gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_FILL);
    }

  gtk_container_foreach (GTK_CONTAINER (priv->box), swap_pack_type, &pack_type);

  pnl_dock_header_update_expanded (self);

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ORIENTATION]);
}

static void
pnl_dock_header_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PnlDockHeader *self = PNL_DOCK_HEADER (object);
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_EXPANDED:
      g_value_set_boolean (value, pnl_dock_header_get_expanded (self));
      break;

    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;

    case PROP_SHOW_CLOSE_BUTTON:
      g_value_set_boolean (value, pnl_dock_header_get_show_close_button (self));
      break;

    case PROP_TITLE:
      g_value_set_string (value, pnl_dock_header_get_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_header_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PnlDockHeader *self = PNL_DOCK_HEADER (object);

  switch (prop_id)
    {
    case PROP_EXPANDED:
      pnl_dock_header_set_expanded (self, g_value_get_boolean (value));
      break;

    case PROP_ORIENTATION:
      pnl_dock_header_set_orientation (self, g_value_get_enum (value));
      break;

    case PROP_SHOW_CLOSE_BUTTON:
      pnl_dock_header_set_show_close_button (self, g_value_get_boolean (value));
      break;

    case PROP_TITLE:
      pnl_dock_header_set_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_header_class_init (PnlDockHeaderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = pnl_dock_header_get_property;
  object_class->set_property = pnl_dock_header_set_property;

  widget_class->button_release_event = pnl_dock_header_button_release_event;
  widget_class->enter_notify_event = pnl_dock_header_enter_notify_event;
  widget_class->leave_notify_event = pnl_dock_header_leave_notify_event;

  properties [PROP_EXPANDED] =
    g_param_spec_boolean ("expanded",
                          "Expanded",
                          "If the child content is revealed",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE));

  properties [PROP_ORIENTATION] =
    g_param_spec_enum ("orientation",
                       "Orientation",
                       "Orientation",
                       GTK_TYPE_ORIENTATION,
                       GTK_ORIENTATION_HORIZONTAL,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_CLOSE_BUTTON] =
    g_param_spec_boolean ("show-close-button",
                          "Show Close Button",
                          "Show Close Button",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_css_name (widget_class, "dockheader");
}

static void
pnl_dock_header_init (PnlDockHeader *self)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  gtk_widget_add_events (GTK_WIDGET (self),
                         (GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK |
                          GDK_ENTER_NOTIFY_MASK |
                          GDK_LEAVE_NOTIFY_MASK));

  priv->box = g_object_new (GTK_TYPE_BOX,
                            "visible", TRUE,
                            NULL);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->box));

  priv->arrow = g_object_new (GTK_TYPE_IMAGE,
                              "icon-name", "pan-end-symbolic",
                              "visible", TRUE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (priv->box), GTK_WIDGET (priv->arrow));

  priv->title = g_object_new (GTK_TYPE_LABEL,
                              "hexpand", TRUE,
                              "visible", TRUE,
                              "xalign", 0.0f,
                              NULL);
  gtk_container_add (GTK_CONTAINER (priv->box), GTK_WIDGET (priv->title));

  priv->close = g_object_new (GTK_TYPE_BUTTON,
                              "child", g_object_new (GTK_TYPE_IMAGE,
                                                     "icon-name", "window-close-symbolic",
                                                     "visible", TRUE,
                                                     NULL),
                              "visible", FALSE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (priv->box), GTK_WIDGET (priv->close));

  g_signal_connect_object (priv->close,
                           "clicked",
                           G_CALLBACK (pnl_dock_header_close_clicked),
                           self,
                           G_CONNECT_SWAPPED);
}

GtkWidget *
pnl_dock_header_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_HEADER, NULL);
}

const gchar *
pnl_dock_header_get_title (PnlDockHeader *self)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_HEADER (self), NULL);

  return gtk_label_get_label (priv->title);
}

void
pnl_dock_header_set_title (PnlDockHeader *self,
                           const gchar   *title)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_HEADER (self));
  g_return_if_fail (title != NULL);

  if (g_strcmp0 (title, gtk_label_get_label (priv->title)) != 0)
    {
      gtk_label_set_label (priv->title, title);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TITLE]);
    }
}

gboolean
pnl_dock_header_get_show_close_button (PnlDockHeader *self)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_HEADER (self), FALSE);

  return gtk_widget_get_visible (GTK_WIDGET (priv->close));
}

void
pnl_dock_header_set_show_close_button (PnlDockHeader *self,
                                       gboolean       show_close_button)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_HEADER (self));

  show_close_button = !!show_close_button;

  if (show_close_button != pnl_dock_header_get_show_close_button (self))
    {
      gtk_widget_set_visible (GTK_WIDGET (priv->close), show_close_button);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SHOW_CLOSE_BUTTON]);
    }
}
