/* pnl-dock-bin-edge-child.c
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

#include "pnl-dock-bin-edge-private.h"
#include "pnl-dock-bin-edge-child-private.h"
#include "pnl-dock-paned.h"
#include "pnl-util-private.h"

/*
 * This class has a simple purpose. It is to help ease the process of
 * size allocations in the PnlDockBinEdge. Since the dock edge is a revealer,
 * we need a child with predictable size requests during the animation
 * sequences (which are tied to the size of the dock edge based on
 * resizing from the handle).
 */

struct _PnlDockBinEdgeChild
{
  GtkBin parent;
  gint   position;
};

G_DEFINE_TYPE (PnlDockBinEdgeChild, pnl_dock_bin_edge_child, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_POSITION,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
pnl_dock_bin_edge_child_add (GtkContainer *container,
                             GtkWidget    *widget)
{
  PnlDockBinEdgeChild *self = (PnlDockBinEdgeChild *)container;
  GtkWidget *prev_child;

  g_assert (PNL_IS_DOCK_BIN_EDGE_CHILD (self));
  g_assert (GTK_IS_WIDGET (widget));

  prev_child = gtk_bin_get_child (GTK_BIN (self));

  if (!prev_child || !PNL_IS_DOCK_PANED (prev_child))
    {
      GtkWidget *paned;

      paned = g_object_new (PNL_TYPE_DOCK_PANED,
                            "orientation", GTK_ORIENTATION_VERTICAL, // FIXME
                             "visible", TRUE,
                             NULL);

      if (prev_child)
        {
          g_object_ref (prev_child);
          gtk_container_remove (GTK_CONTAINER (self), prev_child);
          gtk_container_add (GTK_CONTAINER (paned), prev_child);
          g_object_unref (prev_child);
        }

      GTK_CONTAINER_CLASS (pnl_dock_bin_edge_child_parent_class)->add (GTK_CONTAINER (self), paned);

      prev_child = paned;
    }

  gtk_container_add (GTK_CONTAINER (prev_child), widget);
}

static void
pnl_dock_bin_edge_child_get_preferred_height (GtkWidget *widget,
                                              gint      *min_height,
                                              gint      *nat_height)
{
  PnlDockBinEdgeChild *self = (PnlDockBinEdgeChild *)widget;
  GtkWidget *parent;

  g_assert (PNL_IS_DOCK_BIN_EDGE_CHILD (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  GTK_WIDGET_CLASS (pnl_dock_bin_edge_child_parent_class)->get_preferred_height (widget, min_height, nat_height);

  parent = gtk_widget_get_parent (widget);

  if (self->position != 0 && PNL_IS_DOCK_BIN_EDGE (parent))
    {
      GtkPositionType edge;

      edge = pnl_dock_bin_edge_get_edge (PNL_DOCK_BIN_EDGE (parent));

      if (edge == GTK_POS_TOP || edge == GTK_POS_BOTTOM)
        *nat_height = MAX (*min_height, self->position);
    }
}

static void
pnl_dock_bin_edge_child_get_preferred_width (GtkWidget *widget,
                                             gint      *min_width,
                                             gint      *nat_width)
{
  PnlDockBinEdgeChild *self = (PnlDockBinEdgeChild *)widget;
  GtkWidget *parent;

  g_assert (PNL_IS_DOCK_BIN_EDGE_CHILD (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  GTK_WIDGET_CLASS (pnl_dock_bin_edge_child_parent_class)->get_preferred_width (widget, min_width, nat_width);

  parent = gtk_widget_get_parent (widget);

  if (self->position != 0 && PNL_IS_DOCK_BIN_EDGE (parent))
    {
      GtkPositionType edge;

      edge = pnl_dock_bin_edge_get_edge (PNL_DOCK_BIN_EDGE (parent));

      if (edge == GTK_POS_LEFT || edge == GTK_POS_RIGHT)
        *nat_width = MAX (*min_width, self->position);
    }
}

static GtkSizeRequestMode
pnl_dock_bin_edge_child_get_request_mode (GtkWidget *widget)
{
  PnlDockBinEdgeChild *self = (PnlDockBinEdgeChild *)widget;
  GtkWidget *parent;

  g_assert (PNL_IS_DOCK_BIN_EDGE_CHILD (self));

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (PNL_IS_DOCK_BIN_EDGE (parent))
    {
      GtkPositionType edge = pnl_dock_bin_edge_get_edge (PNL_DOCK_BIN_EDGE (parent));

      if (edge == GTK_POS_LEFT || edge == GTK_POS_RIGHT)
         return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
      else
        return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
    }

  return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
pnl_dock_bin_edge_child_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PnlDockBinEdgeChild *self = PNL_DOCK_BIN_EDGE_CHILD (object);

  switch (prop_id)
    {
    case PROP_POSITION:
      g_value_set_int (value, pnl_dock_bin_edge_child_get_position (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_bin_edge_child_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PnlDockBinEdgeChild *self = PNL_DOCK_BIN_EDGE_CHILD (object);

  switch (prop_id)
    {
    case PROP_POSITION:
      pnl_dock_bin_edge_child_set_position (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_bin_edge_child_class_init (PnlDockBinEdgeChildClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = pnl_dock_bin_edge_child_get_property;
  object_class->set_property = pnl_dock_bin_edge_child_set_property;

  widget_class->get_preferred_height = pnl_dock_bin_edge_child_get_preferred_height;
  widget_class->get_preferred_width = pnl_dock_bin_edge_child_get_preferred_width;
  widget_class->get_request_mode = pnl_dock_bin_edge_child_get_request_mode;
  widget_class->draw = pnl_gtk_bin_draw;
  widget_class->size_allocate = pnl_gtk_bin_size_allocate;

  container_class->add = pnl_dock_bin_edge_child_add;

  properties [PROP_POSITION] =
    g_param_spec_int ("position",
                      "Position",
                      "Position",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_css_name (widget_class, "dockbinedgechild");
}

static void
pnl_dock_bin_edge_child_init (PnlDockBinEdgeChild *self)
{
}

gint
pnl_dock_bin_edge_child_get_position (PnlDockBinEdgeChild *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_BIN_EDGE_CHILD (self), 0);

  return self->position;
}

void
pnl_dock_bin_edge_child_set_position (PnlDockBinEdgeChild *self,
                                      gint                 position)
{
  g_return_if_fail (PNL_IS_DOCK_BIN_EDGE_CHILD (self));

  if (position != self->position)
    {
      self->position = position;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}
