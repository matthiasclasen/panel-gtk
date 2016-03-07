/* pnl-dock-edge-child-private.h
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#ifndef PNL_DOCK_BIN_EDGE_CHILD_PRIVATE_H
#define PNL_DOCK_BIN_EDGE_CHILD_PRIVATE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_DOCK_BIN_EDGE_CHILD (pnl_dock_bin_edge_child_get_type())

G_DECLARE_FINAL_TYPE (PnlDockBinEdgeChild, pnl_dock_bin_edge_child, PNL, DOCK_BIN_EDGE_CHILD, GtkBin)

gint pnl_dock_bin_edge_child_get_position (PnlDockBinEdgeChild *self);
void pnl_dock_bin_edge_child_set_position (PnlDockBinEdgeChild *self,
                                           gint                  position);

G_END_DECLS

#endif /* PNL_DOCK_BIN_EDGE_CHILD_PRIVATE_H */
