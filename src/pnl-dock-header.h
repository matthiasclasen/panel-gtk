/* pnl-dock-header.h
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

#ifndef PNL_DOCK_HEADER_H
#define PNL_DOCK_HEADER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_DOCK_HEADER (pnl_dock_header_get_type())

G_DECLARE_DERIVABLE_TYPE (PnlDockHeader, pnl_dock_header, PNL, DOCK_HEADER, GtkBox)

struct _PnlDockHeaderClass
{
  GtkBoxClass parent;
};

GtkWidget   *pnl_dock_header_new                   (void);
const gchar *pnl_dock_header_get_title             (PnlDockHeader *self);
void         pnl_dock_header_set_title             (PnlDockHeader *self,
                                                    const gchar   *title);
gboolean     pnl_dock_header_get_show_close_button (PnlDockHeader *self);
void         pnl_dock_header_set_show_close_button (PnlDockHeader *self,
                                                    gboolean       show_close_button);

G_END_DECLS

#endif /* PNL_DOCK_HEADER_H */
