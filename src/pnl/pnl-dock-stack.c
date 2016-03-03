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

typedef struct
{
  PnlDockManager *manager;
} PnlDockStackPrivate;

static void pnl_dock_stack_init_dock_group_iface (PnlDockGroupInterface *iface);

G_DEFINE_TYPE_EXTENDED (PnlDockStack, pnl_dock_stack, GTK_TYPE_CONTAINER, 0,
                        G_ADD_PRIVATE (PnlDockStack)
                        G_IMPLEMENT_INTERFACE (PNL_TYPE_DOCK_MANAGER,
                                               pnl_dock_stack_init_dock_group_iface))

enum {
  PROP_0,
  PROP_MANAGER,
  N_PROPS
};

static void
pnl_dock_stack_finalize (GObject *object)
{
  PnlDockStack *self = (PnlDockStack *)object;
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);

  g_clear_object (&priv->manager);

  G_OBJECT_CLASS (pnl_dock_stack_parent_class)->finalize (object);
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
    case PROP_MANAGER:
      g_value_set_object (value, pnl_dock_group_get_manager (PNL_DOCK_GROUP (self)));
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
    case PROP_MANAGER:
      pnl_dock_group_set_manager (PNL_DOCK_GROUP (self), g_value_get_object (value));
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

  object_class->finalize = pnl_dock_stack_finalize;
  object_class->get_property = pnl_dock_stack_get_property;
  object_class->set_property = pnl_dock_stack_set_property;

  g_object_class_override_property (object_class, PROP_MANAGER, "manager");

  gtk_widget_class_set_css_name (widget_class, "dockstack");
}

static void
pnl_dock_stack_init (PnlDockStack *self)
{
}

GtkWidget *
pnl_dock_stack_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_STACK, NULL);
}

static PnlDockManager *
pnl_dock_stack_get_manager (PnlDockGroup *group)
{
  PnlDockStack *self = (PnlDockStack *)group;
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);

  g_return_val_if_fail (PNL_IS_DOCK_STACK (self), NULL);

  return priv->manager;
}

static void
pnl_dock_stack_set_manager (PnlDockGroup   *group,
                            PnlDockManager *manager)
{
  PnlDockStack *self = (PnlDockStack *)group;
  PnlDockStackPrivate *priv = pnl_dock_stack_get_instance_private (self);

  g_return_if_fail (PNL_IS_DOCK_STACK (self));
  g_return_if_fail (!manager || PNL_IS_DOCK_MANAGER (manager));

  if (manager != priv->manager)
    {
      if (priv->manager)
        {
          /* todo: ask manager to adopt children */
          g_clear_object (&priv->manager);
        }

      if (manager)
        priv->manager = g_object_ref (manager);

      g_object_notify (G_OBJECT (self), "manager");
    }
}

static void
pnl_dock_stack_init_dock_group_iface (PnlDockGroupInterface *iface)
{
  iface->get_manager = pnl_dock_stack_get_manager;
  iface->set_manager = pnl_dock_stack_set_manager;
}
