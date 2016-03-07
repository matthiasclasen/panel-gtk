/* pnl-dock-item.c
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
#include "pnl-dock-manager.h"

G_DEFINE_INTERFACE (PnlDockItem, pnl_dock_item, GTK_TYPE_WIDGET)

enum {
  MANAGER_SET,
  N_SIGNALS
};

static guint signals [N_SIGNALS];

static void
pnl_dock_item_real_set_manager (PnlDockItem    *self,
                                PnlDockManager *manager)
{
  PnlDockManager *old_manager;

  g_assert (PNL_IS_DOCK_ITEM (self));
  g_assert (!manager || PNL_IS_DOCK_MANAGER (manager));

  if (NULL != (old_manager = pnl_dock_item_get_manager (self)))
    {
      if (PNL_IS_DOCK (self))
        pnl_dock_manager_unregister_dock (old_manager, PNL_DOCK (self));
    }

  if (manager != NULL)
    {
      g_object_set_data_full (G_OBJECT (self),
                              "PNL_DOCK_MANAGER",
                              g_object_ref (manager),
                              g_object_unref);
      if (PNL_IS_DOCK (self))
        pnl_dock_manager_register_dock (manager, PNL_DOCK (self));
    }
  else
    g_object_set_data (G_OBJECT (self), "PNL_DOCK_MANAGER", NULL);

  g_object_notify (G_OBJECT (self), "manager");

  g_signal_emit (self, signals [MANAGER_SET], 0, old_manager);
}

static PnlDockManager *
pnl_dock_item_real_get_manager (PnlDockItem *self)
{
  g_assert (PNL_IS_DOCK_ITEM (self));

  return g_object_get_data (G_OBJECT (self), "PNL_DOCK_MANAGER");
}

static void
pnl_dock_item_default_init (PnlDockItemInterface *iface)
{
  iface->get_manager = pnl_dock_item_real_get_manager;
  iface->set_manager = pnl_dock_item_real_set_manager;

  signals [MANAGER_SET] =
    g_signal_new ("manager-set",
                  G_TYPE_FROM_INTERFACE (iface),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PnlDockItemInterface, manager_set),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, PNL_TYPE_DOCK_MANAGER);
}

/**
 * pnl_dock_item_get_manager:
 * @self: A #PnlDockItem
 *
 * Gets the dock manager for this dock item.
 *
 * Returns: (nullable) (transfer none): A #PnlDockmanager.
 */
PnlDockManager *
pnl_dock_item_get_manager (PnlDockItem *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_ITEM (self), NULL);

  return PNL_DOCK_ITEM_GET_IFACE (self)->get_manager (self);
}

/**
 * pnl_dock_item_set_manager:
 * @self: A #PnlDockItem
 * @manager: (nullable): A #PnlDockManager
 *
 * Sets the dock manager for this #PnlDockItem.
 */
void
pnl_dock_item_set_manager (PnlDockItem    *self,
                           PnlDockManager *manager)
{
  g_return_if_fail (PNL_IS_DOCK_ITEM (self));
  g_return_if_fail (!manager || PNL_IS_DOCK_MANAGER (manager));

  PNL_DOCK_ITEM_GET_IFACE (self)->set_manager (self, manager);
}

gboolean
pnl_dock_item_adopt (PnlDockItem *self,
                     PnlDockItem *child)
{
  PnlDockManager *manager;
  PnlDockManager *child_manager;

  g_return_val_if_fail (PNL_IS_DOCK_ITEM (self), FALSE);
  g_return_val_if_fail (PNL_IS_DOCK_ITEM (child), FALSE);

  manager = pnl_dock_item_get_manager (self);
  child_manager = pnl_dock_item_get_manager (child);

  if (manager == child_manager)
    return TRUE;

  if (child_manager == NULL)
    {
      pnl_dock_item_set_manager (child, manager);
      return TRUE;
    }

  return FALSE;
}

void
pnl_dock_item_present_child (PnlDockItem *self,
                             PnlDockItem *child)
{
  g_assert (PNL_IS_DOCK_ITEM (self));
  g_assert (PNL_IS_DOCK_ITEM (child));

#if 0
  g_print ("present_child (%s, %s)\n",
           G_OBJECT_TYPE_NAME (self),
           G_OBJECT_TYPE_NAME (child));
#endif

  if (PNL_DOCK_ITEM_GET_IFACE (self)->present_child)
    PNL_DOCK_ITEM_GET_IFACE (self)->present_child (self, child);
}

/**
 * pnl_dock_item_present:
 * @self: A #PnlDockItem
 *
 * This widget will walk the widget hierarchy to ensure that the
 * dock item is visible to the user.
 */
void
pnl_dock_item_present (PnlDockItem *self)
{
  GtkWidget *parent;

  g_return_if_fail (PNL_IS_DOCK_ITEM (self));

  for (parent = gtk_widget_get_parent (GTK_WIDGET (self));
       parent != NULL;
       parent = gtk_widget_get_parent (parent))
    {
      if (PNL_IS_DOCK_ITEM (parent))
        {
          pnl_dock_item_present_child (PNL_DOCK_ITEM (parent), self);
          pnl_dock_item_present (PNL_DOCK_ITEM (parent));
          return;
        }
    }
}
