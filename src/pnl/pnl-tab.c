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
  GtkToggleButton parent;

  GtkLabel *title;
};

G_DEFINE_TYPE (PnlTab, pnl_tab, GTK_TYPE_TOGGLE_BUTTON)

enum {
  PROP_0,
  PROP_TITLE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
pnl_tab_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  PnlTab *self = PNL_TAB (object);

  switch (prop_id)
    {
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
  g_object_set (self, "draw-indicator", FALSE, NULL);

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
