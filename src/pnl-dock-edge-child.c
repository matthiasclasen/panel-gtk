/* pnl-dock-edge-child.c
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

#include "pnl-dock-edge.h"
#include "pnl-dock-edge-child.h"

/*
 * This class has a simple purpose. It is to help ease the process of
 * size allocations in the PnlDockEdge. Since the dock edge is a revealer,
 * we need a child with predictable size requests during the animation
 * sequences (which are tied to the size of the dock edge based on
 * resizing from the handle).
 */

struct _PnlDockEdgeChild
{
  GtkBin parent;
  gint   position;
};

G_DEFINE_TYPE (PnlDockEdgeChild, pnl_dock_edge_child, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_POSITION,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static gboolean
pnl_dock_edge_child_draw (GtkWidget *widget,
                          cairo_t   *cr)
{
  GtkStyleContext *style_context;
  GtkAllocation alloc;

  g_assert (PNL_IS_DOCK_EDGE_CHILD (widget));
  g_assert (cr != NULL);

  style_context = gtk_widget_get_style_context (widget);
  gtk_widget_get_allocation (widget, &alloc);
  gtk_render_background (style_context, cr, alloc.x, alloc.y, alloc.width, alloc.height);

  return GTK_WIDGET_CLASS (pnl_dock_edge_child_parent_class)->draw (widget, cr);
}

static void
pnl_dock_edge_child_get_preferred_height (GtkWidget *widget,
                                          gint      *min_height,
                                          gint      *nat_height)
{
  PnlDockEdgeChild *self = (PnlDockEdgeChild *)widget;
  GtkWidget *parent;

  g_assert (PNL_IS_DOCK_EDGE_CHILD (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  GTK_WIDGET_CLASS (pnl_dock_edge_child_parent_class)->get_preferred_height (widget, min_height, nat_height);

  parent = gtk_widget_get_parent (widget);

  if (self->position != 0 && PNL_IS_DOCK_EDGE (parent))
    {
      GtkPositionType edge;

      edge = pnl_dock_edge_get_edge (PNL_DOCK_EDGE (parent));

      if (edge == GTK_POS_TOP || edge == GTK_POS_BOTTOM)
        *nat_height = MAX (*min_height, self->position);
    }
}

static void
pnl_dock_edge_child_get_preferred_width (GtkWidget *widget,
                                         gint      *min_width,
                                         gint      *nat_width)
{
  PnlDockEdgeChild *self = (PnlDockEdgeChild *)widget;
  GtkWidget *parent;

  g_assert (PNL_IS_DOCK_EDGE_CHILD (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  GTK_WIDGET_CLASS (pnl_dock_edge_child_parent_class)->get_preferred_width (widget, min_width, nat_width);

  parent = gtk_widget_get_parent (widget);

  if (self->position != 0 && PNL_IS_DOCK_EDGE (parent))
    {
      GtkPositionType edge;

      edge = pnl_dock_edge_get_edge (PNL_DOCK_EDGE (parent));

      if (edge == GTK_POS_LEFT || edge == GTK_POS_RIGHT)
        *nat_width = MAX (*min_width, self->position);
    }
}

static void
pnl_dock_edge_child_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PnlDockEdgeChild *self = PNL_DOCK_EDGE_CHILD (object);

  switch (prop_id)
    {
    case PROP_POSITION:
      g_value_set_int (value, pnl_dock_edge_child_get_position (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_edge_child_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PnlDockEdgeChild *self = PNL_DOCK_EDGE_CHILD (object);

  switch (prop_id)
    {
    case PROP_POSITION:
      pnl_dock_edge_child_set_position (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_edge_child_class_init (PnlDockEdgeChildClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = pnl_dock_edge_child_get_property;
  object_class->set_property = pnl_dock_edge_child_set_property;

  widget_class->get_preferred_height = pnl_dock_edge_child_get_preferred_height;
  widget_class->get_preferred_width = pnl_dock_edge_child_get_preferred_width;
  widget_class->draw = pnl_dock_edge_child_draw;

  properties [PROP_POSITION] =
    g_param_spec_int ("position",
                      "Position",
                      "Position",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_css_name (widget_class, "dockedgechild");
}

static void
pnl_dock_edge_child_init (PnlDockEdgeChild *self)
{
}

gint
pnl_dock_edge_child_get_position (PnlDockEdgeChild *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_EDGE_CHILD (self), 0);

  return self->position;
}

void
pnl_dock_edge_child_set_position (PnlDockEdgeChild *self,
                                  gint              position)
{
  g_return_if_fail (PNL_IS_DOCK_EDGE_CHILD (self));

  if (position != self->position)
    {
      self->position = position;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}
