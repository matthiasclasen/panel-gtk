/* pnl-tab-strip.h
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

#ifndef PNL_TAB_STRIP_H
#define PNL_TAB_STRIP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_TAB_STRIP (pnl_tab_strip_get_type())

G_DECLARE_DERIVABLE_TYPE (PnlTabStrip, pnl_tab_strip, PNL, TAB_STRIP, GtkBox)

struct _PnlTabStripClass
{
  GtkBoxClass parent;
};

GtkWidget       *pnl_tab_strip_new             (void);
GtkStack        *pnl_tab_strip_get_stack       (PnlTabStrip     *self);
void             pnl_tab_strip_set_stack       (PnlTabStrip     *self,
                                                GtkStack        *stack);
GtkPositionType  pnl_tab_strip_get_edge        (PnlTabStrip     *self);
void             pnl_tab_strip_set_edge        (PnlTabStrip     *self,
                                                GtkPositionType  edge);
gboolean         pnl_tab_strip_get_show_labels (PnlTabStrip     *self);
void             pnl_tab_strip_set_show_labels (PnlTabStrip     *self,
                                                gboolean         show_labels);

G_END_DECLS

#endif /* PNL_TAB_STRIP_H */
