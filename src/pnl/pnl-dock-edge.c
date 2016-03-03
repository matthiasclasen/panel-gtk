/* pnl-dock-edge.c
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

#include "pnl-dock-edge.h"
#include "pnl-dock-edge-child.h"

struct _PnlDockEdge
{
  GtkRevealer     parent;
  GtkPositionType edge : 3;
};

G_DEFINE_TYPE (PnlDockEdge, pnl_dock_edge, GTK_TYPE_REVEALER)

enum {
  PROP_0,
  PROP_EDGE,
  PROP_POSITION,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

static void
pnl_dock_edge_update_edge (PnlDockEdge *self)
{
  GtkStyleContext *style_context;
  GtkRevealerTransitionType reveal_type = GTK_REVEALER_TRANSITION_TYPE_NONE;
  const gchar *class_name = NULL;

  g_assert (PNL_IS_DOCK_EDGE (self));

  style_context = gtk_widget_get_style_context (GTK_WIDGET (self));

  gtk_style_context_remove_class (style_context, "left");
  gtk_style_context_remove_class (style_context, "right");
  gtk_style_context_remove_class (style_context, "top");
  gtk_style_context_remove_class (style_context, "bottom");

  if (self->edge == GTK_POS_LEFT)
    {
      reveal_type = GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT;
      class_name = "left";
    }
  else if (self->edge == GTK_POS_RIGHT)
    {
      reveal_type = GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT;
      class_name = "right";
    }
  else if (self->edge == GTK_POS_TOP)
    {
      reveal_type = GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN;
      class_name = "top";
    }
  else if (self->edge == GTK_POS_BOTTOM)
    {
      reveal_type = GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP;
      class_name = "bottom";
    }
  else
    {
      g_assert_not_reached ();
      return;
    }

  gtk_style_context_add_class (style_context, class_name);
  gtk_revealer_set_transition_type (GTK_REVEALER (self), reveal_type);
}

GtkPositionType
pnl_dock_edge_get_edge (PnlDockEdge *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_EDGE (self), 0);

  return self->edge;
}

void
pnl_dock_edge_set_edge (PnlDockEdge     *self,
                        GtkPositionType  edge)
{
  g_return_if_fail (PNL_IS_DOCK_EDGE (self));

  if (edge != self->edge)
    {
      self->edge = edge;
      pnl_dock_edge_update_edge (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_EDGE]);
    }
}

gint
pnl_dock_edge_get_position (PnlDockEdge *self)
{
  GtkWidget *child;

  g_return_val_if_fail (PNL_IS_DOCK_EDGE (self), 0);

  child = gtk_bin_get_child (GTK_BIN (self));
  g_assert (PNL_IS_DOCK_EDGE_CHILD (child));

  return pnl_dock_edge_child_get_position (PNL_DOCK_EDGE_CHILD (child));
}

void
pnl_dock_edge_set_position (PnlDockEdge *self,
                            gint         position)
{
  GtkWidget *child;

  g_return_if_fail (PNL_IS_DOCK_EDGE (self));
  g_return_if_fail (position >= 0);

  child = gtk_bin_get_child (GTK_BIN (self));
  g_assert (PNL_IS_DOCK_EDGE_CHILD (child));

  pnl_dock_edge_child_set_position (PNL_DOCK_EDGE_CHILD (child), position);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
}

static void
pnl_dock_edge_add (GtkContainer *container,
                   GtkWidget    *widget)
{
  PnlDockEdge *self = (PnlDockEdge *)container;
  GtkWidget *child;

  g_assert (PNL_IS_DOCK_EDGE (self));
  g_assert (GTK_IS_WIDGET (widget));

  child = gtk_bin_get_child (GTK_BIN (self));
  g_assert (PNL_IS_DOCK_EDGE_CHILD (child));

  gtk_container_add (GTK_CONTAINER (child), widget);
}

static void
pnl_dock_edge_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
  PnlDockEdge *self = (PnlDockEdge *)widget;
  gint position;

  g_assert (PNL_IS_DOCK_EDGE (self));
  g_assert (allocation != NULL);

  if (self->edge == GTK_POS_LEFT || self->edge == GTK_POS_RIGHT)
    position = allocation->width;
  else
    position = allocation->height;

  if (gtk_revealer_get_reveal_child (GTK_REVEALER (self)) &&
      gtk_revealer_get_child_revealed (GTK_REVEALER (self)) &&
      (position < pnl_dock_edge_get_position (self)))
    pnl_dock_edge_set_position (self, position);

  GTK_WIDGET_CLASS (pnl_dock_edge_parent_class)->size_allocate (widget, allocation);
}

static void
pnl_dock_edge_constructed (GObject *object)
{
  PnlDockEdge *self = (PnlDockEdge *)object;

  G_OBJECT_CLASS (pnl_dock_edge_parent_class)->constructed (object);

  pnl_dock_edge_update_edge (self);
}

static void
pnl_dock_edge_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  PnlDockEdge *self = PNL_DOCK_EDGE (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      g_value_set_enum (value, pnl_dock_edge_get_edge (self));
      break;

    case PROP_POSITION:
      g_value_set_int (value, pnl_dock_edge_get_position (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_edge_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PnlDockEdge *self = PNL_DOCK_EDGE (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      pnl_dock_edge_set_edge (self, g_value_get_enum (value));
      break;

    case PROP_POSITION:
      pnl_dock_edge_set_position (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_edge_class_init (PnlDockEdgeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->constructed = pnl_dock_edge_constructed;
  object_class->get_property = pnl_dock_edge_get_property;
  object_class->set_property = pnl_dock_edge_set_property;

  widget_class->size_allocate = pnl_dock_edge_size_allocate;

  container_class->add = pnl_dock_edge_add;

  properties [PROP_EDGE] =
    g_param_spec_enum ("edge",
                       "Edge",
                       "The edge of the dock this widget is attached to",
                       GTK_TYPE_POSITION_TYPE,
                       GTK_POS_LEFT,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_POSITION] =
    g_param_spec_int ("position",
                      "Position",
                      "The size of the dock edge when displayed",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_css_name (widget_class, "dockedge");
}

static void
pnl_dock_edge_init (PnlDockEdge *self)
{
  GtkWidget *child;

  self->edge = GTK_POS_LEFT;

  child = g_object_new (PNL_TYPE_DOCK_EDGE_CHILD,
                        "visible", TRUE,
                        "position", 0,
                        NULL);
  GTK_CONTAINER_CLASS (pnl_dock_edge_parent_class)->add (GTK_CONTAINER (self), child);
}
