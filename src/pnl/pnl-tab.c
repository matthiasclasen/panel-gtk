/* pnl-tab.c
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

#include "pnl-tab.h"

struct _PnlTab
{
  GtkToggleButton  parent;
  GtkPositionType  edge : 2;
  GtkLabel        *title;
};

G_DEFINE_TYPE (PnlTab, pnl_tab, GTK_TYPE_TOGGLE_BUTTON)

enum {
  PROP_0,
  PROP_EDGE,
  PROP_TITLE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
pnl_tab_update_edge (PnlTab *self)
{
  g_assert (PNL_IS_TAB (self));

  switch (self->edge)
    {
    case GTK_POS_TOP:
      gtk_label_set_angle (self->title, 0.0);
      break;

    case GTK_POS_BOTTOM:
      gtk_label_set_angle (self->title, 0.0);
      break;

    case GTK_POS_LEFT:
      gtk_label_set_angle (self->title, 90.0);
      break;

    case GTK_POS_RIGHT:
      gtk_label_set_angle (self->title, -90.0);
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
pnl_tab_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  PnlTab *self = PNL_TAB (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      g_value_set_enum (value, pnl_tab_get_edge (self));
      break;

    case PROP_TITLE:
      g_value_set_string (value, pnl_tab_get_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_tab_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  PnlTab *self = PNL_TAB (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      pnl_tab_set_edge (self, g_value_get_enum (value));
      break;

    case PROP_TITLE:
      pnl_tab_set_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_tab_class_init (PnlTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = pnl_tab_get_property;
  object_class->set_property = pnl_tab_set_property;

  gtk_widget_class_set_css_name (widget_class, "tab");

  properties [PROP_EDGE] =
    g_param_spec_enum ("edge",
                       "Edge",
                       "Edge",
                       GTK_TYPE_POSITION_TYPE,
                       GTK_POS_TOP,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
pnl_tab_init (PnlTab *self)
{
  self->edge = GTK_POS_TOP;

  self->title = g_object_new (GTK_TYPE_LABEL,
                              "ellipsize", PANGO_ELLIPSIZE_END,
                              "visible", TRUE,
                              NULL);

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->title));
}

const gchar *
pnl_tab_get_title (PnlTab *self)
{
  g_return_val_if_fail (PNL_IS_TAB (self), NULL);

  return gtk_label_get_label (self->title);
}

void
pnl_tab_set_title (PnlTab      *self,
                   const gchar *title)
{
  g_return_if_fail (PNL_IS_TAB (self));

  gtk_label_set_label (self->title, title);
}

GtkPositionType
pnl_tab_get_edge (PnlTab *self)
{
  g_return_val_if_fail (PNL_IS_TAB (self), 0);

  return self->edge;
}

void
pnl_tab_set_edge (PnlTab          *self,
                  GtkPositionType  edge)
{
  g_return_if_fail (PNL_IS_TAB (self));
  g_return_if_fail (edge >= 0);
  g_return_if_fail (edge <= 3);

  if (self->edge != edge)
    {
      self->edge = edge;
      pnl_tab_update_edge (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_EDGE]);
    }
}
