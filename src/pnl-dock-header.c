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

typedef struct
{
  GtkImage  *arrow;
  GtkLabel  *title;
  GtkButton *close;
  GtkWidget *custom_title;
} PnlDockHeaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (PnlDockHeader, pnl_dock_header, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_TITLE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
pnl_dock_header_finalize (GObject *object)
{
  G_OBJECT_CLASS (pnl_dock_header_parent_class)->finalize (object);
}

static void
pnl_dock_header_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PnlDockHeader *self = PNL_DOCK_HEADER (object);

  switch (prop_id)
    {
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

  object_class->finalize = pnl_dock_header_finalize;
  object_class->get_property = pnl_dock_header_get_property;
  object_class->set_property = pnl_dock_header_set_property;

  gtk_widget_class_set_css_name (widget_class, "dockheader");
}

static void
pnl_dock_header_init (PnlDockHeader *self)
{
  PnlDockHeaderPrivate *priv = pnl_dock_header_get_instance_private (self);

  priv->arrow = g_object_new (GTK_TYPE_IMAGE,
                              "visible", TRUE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->arrow));

  priv->title = g_object_new (GTK_TYPE_LABEL,
                              "hexpand", TRUE,
                              "visible", TRUE,
                              "xalign", 0.0f,
                              NULL);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->title));

  priv->close = g_object_new (GTK_TYPE_BUTTON,
                              "child", g_object_new (GTK_TYPE_IMAGE,
                                                     "icon-name", "window-close-symbolic",
                                                     "visible", TRUE,
                                                     NULL),
                              "visible", TRUE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->close));
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
