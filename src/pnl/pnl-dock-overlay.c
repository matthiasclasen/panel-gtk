/* pnl-dock-overlay.c
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

#include "pnl-dock-edge-private.h"
#include "pnl-dock-item.h"
#include "pnl-dock-overlay.h"

typedef struct
{
  PnlDockEdge *bottom;
  PnlDockEdge *left;
  PnlDockEdge *right;
  PnlDockEdge *top;

  guint alt_pressed : 1;
} PnlDockOverlayPrivate;

static void pnl_dock_overlay_init_dock_iface      (PnlDockInterface *iface);
static void pnl_dock_overlay_init_buildable_iface (GtkBuildableIface *iface);

G_DEFINE_TYPE_EXTENDED (PnlDockOverlay, pnl_dock_overlay, GTK_TYPE_OVERLAY, 0,
                        G_ADD_PRIVATE (PnlDockOverlay)
                        G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                               pnl_dock_overlay_init_buildable_iface)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK_ITEM, NULL)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK, pnl_dock_overlay_init_dock_iface))

enum {
  PROP_0,
  PROP_MANAGER,
  N_PROPS
};

static gboolean
pnl_dock_overlay_get_child_position (GtkOverlay    *overlay,
                                     GtkWidget     *widget,
                                     GtkAllocation *allocation)
{
  PnlDockOverlay *self = (PnlDockOverlay *)overlay;
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);
  PnlDockEdge *edge;
  gint nat_width;
  gint nat_height;

  g_assert (GTK_IS_OVERLAY (overlay));
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (allocation != NULL);

  if (!PNL_IS_DOCK_EDGE (widget))
    return FALSE;

  gtk_widget_get_allocation (GTK_WIDGET (self), allocation);

  allocation->x = 0;
  allocation->y = 0;

  edge = PNL_DOCK_EDGE (widget);

  if (edge == priv->left || edge == priv->right)
    gtk_widget_get_preferred_width_for_height (widget,
                                               MAX (allocation->height, 1),
                                               NULL,
                                               &nat_width);
  else
    gtk_widget_get_preferred_height_for_width (widget,
                                               MAX (allocation->width, 1),
                                               NULL,
                                               &nat_height);

  if (edge == priv->left)
    {
      allocation->width = nat_width;
    }
  else if (edge == priv->right)
    {
      allocation->x = allocation->x + allocation->width - nat_width;
      allocation->width - nat_width;
    }
  else if (edge == priv->bottom)
    {
      allocation->y = allocation->y + allocation->height - nat_height;
      allocation->height = nat_height;
    }
  else if (edge == priv->top)
    {
      allocation->height = nat_height;
    }

  return TRUE;
}

static void
pnl_dock_overlay_add (GtkContainer *container,
                      GtkWidget    *widget)
{
  PnlDockOverlay *self = (PnlDockOverlay *)container;

  g_assert (PNL_IS_DOCK_OVERLAY (self));
  g_assert (GTK_IS_WIDGET (widget));

  if (PNL_IS_DOCK_ITEM (widget) &&
      !pnl_dock_item_adopt (PNL_DOCK_ITEM (self), PNL_DOCK_ITEM (widget)))
    {
      g_warning ("Child of type %s has a different PnlDockManager than %s",
                 G_OBJECT_TYPE_NAME (widget), G_OBJECT_TYPE_NAME (self));
      return;
    }

  GTK_CONTAINER_CLASS (pnl_dock_overlay_parent_class)->add (container, widget);
}

static void
pnl_dock_overlay_toplevel_mnemonics (PnlDockOverlay *self,
                                     GParamSpec     *pspec,
                                     GtkWindow      *toplevel)
{
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);
  gboolean reveal_child;

  g_assert (PNL_IS_DOCK_OVERLAY (self));
  g_assert (pspec != NULL);
  g_assert (GTK_IS_WINDOW (toplevel));

  reveal_child = gtk_window_get_mnemonics_visible (toplevel);

  /* TODO: really we just want to show the floating tabs here */

  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->top), reveal_child);
  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->bottom), reveal_child);
  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->right), reveal_child);
  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->left), reveal_child);
}

static void
pnl_dock_overlay_hierarchy_changed (GtkWidget *widget,
                                    GtkWidget *old_toplevel)
{
  PnlDockOverlay *self = (PnlDockOverlay *)widget;
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);
  GtkWidget *toplevel;

  g_assert (PNL_IS_DOCK_OVERLAY (self));
  g_assert (!old_toplevel || GTK_IS_WIDGET (old_toplevel));

  if (old_toplevel != NULL)
    {
      g_signal_handlers_disconnect_by_func (old_toplevel,
                                            G_CALLBACK (pnl_dock_overlay_toplevel_mnemonics),
                                            self);
    }

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

  if (toplevel != NULL)
    {
      g_signal_connect_object (toplevel,
                               "notify::mnemonics-visible",
                               G_CALLBACK (pnl_dock_overlay_toplevel_mnemonics),
                               self,
                               G_CONNECT_SWAPPED);
    }
}

static void
pnl_dock_overlay_finalize (GObject *object)
{
  G_OBJECT_CLASS (pnl_dock_overlay_parent_class)->finalize (object);
}

static void
pnl_dock_overlay_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PnlDockOverlay *self = PNL_DOCK_OVERLAY (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      g_value_set_object (value, pnl_dock_item_get_manager (PNL_DOCK_ITEM (self)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_overlay_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PnlDockOverlay *self = PNL_DOCK_OVERLAY (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      pnl_dock_item_set_manager (PNL_DOCK_ITEM (self), g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_overlay_class_init (PnlDockOverlayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GtkOverlayClass *overlay_class = GTK_OVERLAY_CLASS (klass);

  object_class->finalize = pnl_dock_overlay_finalize;
  object_class->get_property = pnl_dock_overlay_get_property;
  object_class->set_property = pnl_dock_overlay_set_property;

  widget_class->hierarchy_changed = pnl_dock_overlay_hierarchy_changed;

  container_class->add = pnl_dock_overlay_add;

  overlay_class->get_child_position = pnl_dock_overlay_get_child_position;

  g_object_class_override_property (object_class, PROP_MANAGER, "manager");

  gtk_widget_class_set_css_name (widget_class, "dockoverlay");
}

static void
pnl_dock_overlay_init_child (PnlDockOverlay   *self,
                             PnlDockEdge     **edge,
                             GtkPositionType   type)
{
  g_assert (PNL_IS_DOCK_OVERLAY (self));
  g_assert (edge != NULL);

  *edge = g_object_new (PNL_TYPE_DOCK_EDGE,
                        "edge", type,
                        "reveal-child", FALSE,
                        "visible", TRUE,
                        NULL);

  gtk_overlay_add_overlay (GTK_OVERLAY (self), GTK_WIDGET (*edge));
}

static void
pnl_dock_overlay_init (PnlDockOverlay *self)
{
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);

  pnl_dock_overlay_init_child (self, &priv->bottom, GTK_POS_BOTTOM);
  pnl_dock_overlay_init_child (self, &priv->left, GTK_POS_LEFT);
  pnl_dock_overlay_init_child (self, &priv->right, GTK_POS_RIGHT);
  pnl_dock_overlay_init_child (self, &priv->top, GTK_POS_TOP);
}

static void
pnl_dock_overlay_init_dock_iface (PnlDockInterface *iface)
{
}

GtkWidget *
pnl_dock_overlay_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_OVERLAY, NULL);
}

static void
pnl_dock_overlay_add_child (GtkBuildable *buildable,
                            GtkBuilder   *builder,
                            GObject      *child,
                            const gchar  *type)
{
  PnlDockOverlay *self = (PnlDockOverlay *)buildable;
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);
  PnlDockEdge *parent = NULL;

  g_assert (PNL_IS_DOCK_OVERLAY (self));
  g_assert (GTK_IS_BUILDER (builder));
  g_assert (G_IS_OBJECT (child));

  if (!GTK_IS_WIDGET (child))
    {
      g_warning ("Attempt to add a child of type \"%s\" to a \"%s\"",
                 G_OBJECT_TYPE_NAME (child), G_OBJECT_TYPE_NAME (self));
      return;
    }

  if (PNL_IS_DOCK_ITEM (child) &&
      !pnl_dock_item_adopt (PNL_DOCK_ITEM (self), PNL_DOCK_ITEM (child)))
    {
      g_warning ("Child of type %s has a different PnlDockManager than %s",
                 G_OBJECT_TYPE_NAME (child), G_OBJECT_TYPE_NAME (self));
      return;
    }

  if (!type || (g_strcmp0 ("center", type) == 0))
    {
      gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (child));
      return;
    }

  if (g_strcmp0 ("top", type) == 0)
    parent = priv->top;
  else if (g_strcmp0 ("bottom", type) == 0)
    parent = priv->bottom;
  else if (g_strcmp0 ("right", type) == 0)
    parent = priv->right;
  else
    parent = priv->left;

  gtk_container_add (GTK_CONTAINER (parent), GTK_WIDGET (child));
}

static void
pnl_dock_overlay_init_buildable_iface (GtkBuildableIface *iface)
{
  iface->add_child = pnl_dock_overlay_add_child;
}
