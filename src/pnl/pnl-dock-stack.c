/* pnl-dock-stack.c
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

#include "pnl-dock-stack.h"
#include "pnl-dock-widget.h"
#include "pnl-tab-strip.h"

typedef struct
{
  GtkStack         *stack;
  PnlTabStrip      *tab_strip;
  GtkPositionType   edge : 2;
} PnlDockStackPrivate;

static void pnl_dock_stack_init_dock_group_iface (PnlDockGroupInterface *iface);

G_DEFINE_TYPE_EXTENDED (PnlDockStack, pnl_dock_stack, GTK_TYPE_BOX, 0,
                        G_ADD_PRIVATE (PnlDockStack)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK_GROUP,
                                               pnl_dock_stack_init_dock_group_iface))

enum {
  PROP_0,
  PROP_EDGE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
pnl_dock_stack_add (GtkContainer *container,
                    GtkWidget    *widget)
{
  PnlDockStack *self = (PnlDockStack *)container;
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);
  const gchar *title = NULL;

  g_assert (PNL_IS_DOCK_STACK (self));

  if (PNL_IS_DOCK_WIDGET (widget))
    title = pnl_dock_widget_get_title (PNL_DOCK_WIDGET (widget));

  gtk_container_add_with_properties (GTK_CONTAINER (priv->stack), widget,
                                     "title", title,
                                     NULL);
}

static void
pnl_dock_stack_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PnlDockStack *self = PNL_DOCK_STACK (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      g_value_set_enum (value, pnl_dock_stack_get_edge (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_stack_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PnlDockStack *self = PNL_DOCK_STACK (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      pnl_dock_stack_set_edge (self, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_stack_class_init (PnlDockStackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = pnl_dock_stack_get_property;
  object_class->set_property = pnl_dock_stack_set_property;

  container_class->add = pnl_dock_stack_add;

  properties [PROP_EDGE] =
    g_param_spec_enum ("edge",
                       "Edge",
                       "The edge for the tab strip",
                       GTK_TYPE_POSITION_TYPE,
                       GTK_POS_TOP,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_css_name (widget_class, "dockstack");
}

static void
pnl_dock_stack_init (PnlDockStack *self)
{
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);

  priv->edge = GTK_POS_TOP;

  priv->stack = g_object_new (GTK_TYPE_STACK,
                              "homogeneous", FALSE,
                              "visible", TRUE,
                              NULL);

  priv->tab_strip = g_object_new (PNL_TYPE_TAB_STRIP,
                                  "edge", GTK_POS_TOP,
                                  "stack", priv->stack,
                                  "visible", TRUE,
                                  NULL);

  GTK_CONTAINER_CLASS (pnl_dock_stack_parent_class)->add (GTK_CONTAINER (self),
                                                          GTK_WIDGET (priv->tab_strip));
  GTK_CONTAINER_CLASS (pnl_dock_stack_parent_class)->add (GTK_CONTAINER (self),
                                                          GTK_WIDGET (priv->stack));
}

GtkWidget *
pnl_dock_stack_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_STACK, NULL);
}

static void
pnl_dock_stack_init_dock_group_iface (PnlDockGroupInterface *iface)
{
}

GtkPositionType
pnl_dock_stack_get_edge (PnlDockStack *self)
{
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_STACK (self), 0);

  return priv->edge;
}

void
pnl_dock_stack_set_edge (PnlDockStack    *self,
                         GtkPositionType  edge)
{
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_STACK (self));
  g_return_if_fail (edge >= 0);
  g_return_if_fail (edge <= 3);

  if (edge != priv->edge)
    {
      priv->edge = edge;

      pnl_tab_strip_set_edge (priv->tab_strip, edge);

      switch (edge)
        {
        case GTK_POS_TOP:
          gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
          gtk_container_child_set (GTK_CONTAINER (self), GTK_WIDGET (priv->tab_strip),
                                   "position", 0,
                                   NULL);
          break;

        case GTK_POS_BOTTOM:
          gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
          gtk_container_child_set (GTK_CONTAINER (self), GTK_WIDGET (priv->tab_strip),
                                   "position", 1,
                                   NULL);
          break;

        case GTK_POS_LEFT:
          gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
          gtk_container_child_set (GTK_CONTAINER (self), GTK_WIDGET (priv->tab_strip),
                                   "position", 0,
                                   NULL);
          break;

        case GTK_POS_RIGHT:
          gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
          gtk_container_child_set (GTK_CONTAINER (self), GTK_WIDGET (priv->tab_strip),
                                   "position", 1,
                                   NULL);
          break;

        default:
          g_assert_not_reached ();
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_EDGE]);
    }
}
