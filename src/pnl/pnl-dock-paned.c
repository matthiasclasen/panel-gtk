/* pnl-dock-paned.c
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

#include "pnl-dock-item.h"
#include "pnl-dock-paned.h"

typedef struct
{
  void *foo;
} PnlDockPanedPrivate;

static void pnl_dock_paned_init_dock_group_iface (PnlDockGroupInterface *iface);

G_DEFINE_TYPE_EXTENDED (PnlDockPaned, pnl_dock_paned, PNL_TYPE_MULTI_PANED, 0,
                        G_ADD_PRIVATE (PnlDockPaned)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK_ITEM, NULL)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK_GROUP,
                                               pnl_dock_paned_init_dock_group_iface))

enum {
  PROP_0,
  N_PROPS
};

static void
pnl_dock_paned_add (GtkContainer *container,
                    GtkWidget    *widget)
{
  PnlDockPaned *self = (PnlDockPaned *)container;
  PnlDockPanedPrivate *priv = pnl_dock_paned_get_instance_private (self);

  g_assert (PNL_IS_DOCK_PANED (self));

  if (PNL_IS_DOCK_ITEM (widget) &&
      !pnl_dock_item_adopt (PNL_DOCK_ITEM (self), PNL_DOCK_ITEM (widget)))
    {
      g_warning ("%s failed to adopt child %x",
                 G_OBJECT_TYPE_NAME (self), G_OBJECT_TYPE_NAME (widget));
      return;
    }

  GTK_CONTAINER_CLASS (pnl_dock_paned_parent_class)->add (container, widget);
}

static void
pnl_dock_paned_finalize (GObject *object)
{
  PnlDockPaned *self = (PnlDockPaned *)object;
  PnlDockPanedPrivate *priv = pnl_dock_paned_get_instance_private (self);

  G_OBJECT_CLASS (pnl_dock_paned_parent_class)->finalize (object);
}

static void
pnl_dock_paned_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PnlDockPaned *self = PNL_DOCK_PANED (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_paned_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PnlDockPaned *self = PNL_DOCK_PANED (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_paned_class_init (PnlDockPanedClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = pnl_dock_paned_finalize;
  object_class->get_property = pnl_dock_paned_get_property;
  object_class->set_property = pnl_dock_paned_set_property;

  gtk_widget_class_set_css_name (widget_class, "dockpaned");
}

static void
pnl_dock_paned_init (PnlDockPaned *self)
{
}

GtkWidget *
pnl_dock_paned_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_PANED, NULL);
}

static void
pnl_dock_paned_init_dock_group_iface (PnlDockGroupInterface *iface)
{
}
