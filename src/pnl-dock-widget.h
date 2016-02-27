/* pnl-dock-widget.h
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

#ifndef PNL_DOCK_WIDGET_H
#define PNL_DOCK_WIDGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_DOCK_WIDGET (pnl_dock_widget_get_type())

G_DECLARE_DERIVABLE_TYPE (PnlDockWidget, pnl_dock_widget, PNL, DOCK_WIDGET, GtkBin)

struct _PnlDockWidgetClass
{
  GtkBinClass parent;
};

GtkWidget   *pnl_dock_widget_new (void);
gboolean     pnl_dock_widget_get_reveal_child (PnlDockWidget *self);
void         pnl_dock_widget_set_reveal_child (PnlDockWidget *self,
                                               gboolean       reveal_child);
const gchar *pnl_dock_widget_get_title        (PnlDockWidget *self);
void         pnl_dock_widget_set_title        (PnlDockWidget *self,
                                               const gchar   *title);
GtkWidget   *pnl_dock_widget_get_custom_title (PnlDockWidget *self);
void         pnl_dock_widget_set_custom_title (PnlDockWidget *self,
                                               GtkWidget     *custom_title);

G_END_DECLS

#endif /* PNL_DOCK_WIDGET_H */
