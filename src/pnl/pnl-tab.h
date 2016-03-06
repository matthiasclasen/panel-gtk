/* pnl-tab.h
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

#ifndef PNL_TAB_H
#define PNL_TAB_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_TAB (pnl_tab_get_type())

G_DECLARE_FINAL_TYPE (PnlTab, pnl_tab, PNL, TAB, GtkToggleButton)

const gchar     *pnl_tab_get_title  (PnlTab          *self);
void             pnl_tab_set_title  (PnlTab          *self,
                                     const gchar     *title);
GtkPositionType  pnl_tab_get_edge   (PnlTab          *self);
void             pnl_tab_set_edge   (PnlTab          *self,
                                     GtkPositionType  edge);
GtkWidget       *pnl_tab_get_widget (PnlTab          *self);
void             pnl_tab_set_widget (PnlTab          *self,
                                     GtkWidget       *widget);

G_END_DECLS

#endif /* PNL_TAB_H */
