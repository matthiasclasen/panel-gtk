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

#include "pnl-dock-overlay.h"

typedef struct
{
  PnlDockManager *manager;
} PnlDockOverlayPrivate;

static void pnl_dock_overlay_init_dock_iface (PnlDockInterface *iface);

G_DEFINE_TYPE_EXTENDED (PnlDockOverlay, pnl_dock_overlay, GTK_TYPE_OVERLAY, 0,
                        G_ADD_PRIVATE (PnlDockOverlay)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK, pnl_dock_overlay_init_dock_iface))

enum {
  PROP_0,
  PROP_MANAGER,
  N_PROPS
};

static void
pnl_dock_overlay_finalize (GObject *object)
{
  PnlDockOverlay *self = (PnlDockOverlay *)object;
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);

  g_clear_object (&priv->manager);

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
      g_value_set_object (value, pnl_dock_get_manager (PNL_DOCK (self)));
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
      pnl_dock_set_manager (PNL_DOCK (self), g_value_get_object (value));
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

  object_class->finalize = pnl_dock_overlay_finalize;
  object_class->get_property = pnl_dock_overlay_get_property;
  object_class->set_property = pnl_dock_overlay_set_property;

  g_object_class_override_property (object_class, PROP_MANAGER, "manager");

  gtk_widget_class_set_css_name (widget_class, "dockoverlay");
}

static void
pnl_dock_overlay_init (PnlDockOverlay *self)
{
}

static PnlDockManager *
pnl_dock_overlay_get_manager (PnlDock *dock)
{
  PnlDockOverlay *self = (PnlDockOverlay *)dock;
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);

  g_assert (PNL_IS_DOCK_OVERLAY (self));

  return priv->manager;
}

static void
pnl_dock_overlay_set_manager (PnlDock        *dock,
                              PnlDockManager *manager)
{
  PnlDockOverlay *self = (PnlDockOverlay *)dock;
  PnlDockOverlayPrivate *priv = pnl_dock_overlay_get_instance_private (self);

  g_assert (PNL_IS_DOCK_OVERLAY (self));
  g_assert (!manager || PNL_IS_DOCK_MANAGER (manager));

  if (manager != priv->manager)
    {
      if (priv->manager)
        {
          /* TODO: have old manager adopt all children. */
          g_clear_object (&priv->manager);
        }

      if (manager)
        priv->manager = g_object_ref (manager);

      g_object_notify (G_OBJECT (self), "manager");
    }
}

static void
pnl_dock_overlay_init_dock_iface (PnlDockInterface *iface)
{
  iface->get_manager = pnl_dock_overlay_get_manager;
  iface->set_manager = pnl_dock_overlay_set_manager;
}

GtkWidget *
pnl_dock_overlay_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_OVERLAY, NULL);
}
