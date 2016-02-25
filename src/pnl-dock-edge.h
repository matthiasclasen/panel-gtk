/* pnl-dock-edge.h
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

#if !defined(PANEL_GTK_INSIDE) && !defined(PANEL_GTK_COMPILATION)
# error "Only <panel-gtk.h> can be included directly."
#endif

#ifndef PNL_DOCK_EDGE_H
#define PNL_DOCK_EDGE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_DOCK_EDGE (pnl_dock_edge_get_type())

G_DECLARE_FINAL_TYPE (PnlDockEdge, pnl_dock_edge, PNL, DOCK_EDGE, GtkRevealer)

GtkPositionType pnl_dock_edge_get_edge     (PnlDockEdge     *self);
void            pnl_dock_edge_set_edge     (PnlDockEdge     *self,
                                            GtkPositionType  edge);
gint            pnl_dock_edge_get_position (PnlDockEdge     *self);
void            pnl_dock_edge_set_position (PnlDockEdge     *self,
                                            gint             position);

G_END_DECLS

#endif /* PNL_DOCK_EDGE_H */
