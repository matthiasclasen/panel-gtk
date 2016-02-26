/* pnl-panel.h
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

#ifndef PNL_PANEL_H
#define PNL_PANEL_H

#include "pnl-multi-paned.h"

G_BEGIN_DECLS

#define PNL_TYPE_PANEL (pnl_panel_get_type())

G_DECLARE_DERIVABLE_TYPE (PnlPanel, pnl_panel, PNL, PANEL, PnlMultiPaned)

struct _PnlPanelClass
{
  PnlMultiPanedClass parent_class;
};

GtkWidget *pnl_panel_new (void);

G_END_DECLS

#endif /* PNL_PANEL_H */
