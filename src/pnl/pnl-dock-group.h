/* pnl-dock-group.h
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

#ifndef PNL_DOCK_GROUP_H
#define PNL_DOCK_GROUP_H

#include "pnl-dock-types.h"

G_BEGIN_DECLS

struct _PnlDockGroupInterface
{
  GTypeInterface parent;

  PnlDockManager *(*get_manager) (PnlDockGroup   *self);
  void            (*set_manager) (PnlDockGroup   *self,
                                  PnlDockManager *manager);
};

PnlDockManager *pnl_dock_group_get_manager (PnlDockGroup   *self);
void            pnl_dock_group_set_manager (PnlDockGroup   *self,
                                            PnlDockManager *manager);

G_END_DECLS

#endif /* PNL_DOCK_GROUP_H */
