/* pnl-dock-group.c
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

#include "pnl-dock-group.h"

G_DEFINE_INTERFACE (PnlDockGroup, pnl_dock_group, GTK_TYPE_CONTAINER)

static void
pnl_dock_group_default_init (PnlDockGroupInterface *iface)
{
  g_object_interface_install_property (iface,
                                       g_param_spec_object ("manager",
                                                            "Manager",
                                                            "The manager of the dock group",
                                                            PNL_TYPE_DOCK_MANAGER,
                                                            (G_PARAM_READWRITE |
                                                             G_PARAM_STATIC_STRINGS)));
}

/**
 * pnl_dock_group_get_manager:
 * @self: A #PnlDockGroup
 *
 * Gets the #PnlDockManager that is managing this group.
 *
 * Returns: (transfer none): A #PnlDockManager.
 */
PnlDockManager *
pnl_dock_group_get_manager (PnlDockGroup *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_GROUP (self), NULL);

  return PNL_DOCK_GROUP_GET_IFACE (self)->get_manager (self);
}

void
pnl_dock_group_set_manager (PnlDockGroup   *self,
                            PnlDockManager *manager)
{
  g_return_if_fail (PNL_IS_DOCK_GROUP (self));
  g_return_if_fail (!manager || PNL_IS_DOCK_MANAGER (manager));

  PNL_DOCK_GROUP_GET_IFACE (self)->set_manager (self, manager);
}
