/* pnl-panel.c
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

#include "pnl-panel.h"
#include "pnl-dock-widget.h"

typedef struct
{
} PnlPanelPrivate;

G_DEFINE_TYPE (PnlPanel, pnl_panel, PNL_TYPE_MULTI_PANED)

static void
pnl_panel_resize_drag_begin (PnlMultiPaned *multi_paned,
                             GtkWidget     *child)
{
  G_GNUC_UNUSED PnlPanel *self = (PnlPanel *)multi_paned;

  g_assert (PNL_IS_PANEL (self));
  g_assert (GTK_IS_WIDGET (child));

  if (PNL_IS_DOCK_WIDGET (child))
    pnl_dock_widget_set_reveal_child (PNL_DOCK_WIDGET (child), TRUE);

  PNL_MULTI_PANED_CLASS (pnl_panel_parent_class)->resize_drag_end (multi_paned, child);
}

static void
pnl_panel_resize_drag_end (PnlMultiPaned *multi_paned,
                           GtkWidget     *child)
{
  G_GNUC_UNUSED PnlPanel *self = (PnlPanel *)multi_paned;

  g_assert (PNL_IS_PANEL (self));
  g_assert (GTK_IS_WIDGET (child));

  PNL_MULTI_PANED_CLASS (pnl_panel_parent_class)->resize_drag_begin (multi_paned, child);
}

static void
pnl_panel_class_init (PnlPanelClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  PnlMultiPanedClass *multi_paned_class = PNL_MULTI_PANED_CLASS (klass);

  multi_paned_class->resize_drag_begin = pnl_panel_resize_drag_begin;
  multi_paned_class->resize_drag_end = pnl_panel_resize_drag_end;

  gtk_widget_class_set_css_name (widget_class, "panel");
}

static void
pnl_panel_init (PnlPanel *self)
{
}
