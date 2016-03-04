/* pnl-dock-manager.c
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

#include "pnl-dock-manager.h"

typedef struct
{
  void *foo;
} PnlDockManagerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (PnlDockManager, pnl_dock_manager, G_TYPE_OBJECT)

enum {
  PROP_0,
  N_PROPS
};

enum {
  REGISTER_DOCK,
  UNREGISTER_DOCK,
  N_SIGNALS
};

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static void
pnl_dock_manager_finalize (GObject *object)
{
  PnlDockManager *self = (PnlDockManager *)object;
  PnlDockManagerPrivate *priv = pnl_dock_manager_get_instance_private (self);

  G_OBJECT_CLASS (pnl_dock_manager_parent_class)->finalize (object);
}

static void
pnl_dock_manager_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PnlDockManager *self = PNL_DOCK_MANAGER (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_manager_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PnlDockManager *self = PNL_DOCK_MANAGER (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_manager_class_init (PnlDockManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = pnl_dock_manager_finalize;
  object_class->get_property = pnl_dock_manager_get_property;
  object_class->set_property = pnl_dock_manager_set_property;

  signals [REGISTER_DOCK] =
    g_signal_new ("register-dock",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PnlDockManagerClass, register_dock),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, PNL_TYPE_DOCK);

  signals [UNREGISTER_DOCK] =
    g_signal_new ("unregister-dock",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PnlDockManagerClass, unregister_dock),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, PNL_TYPE_DOCK);
}

static void
pnl_dock_manager_init (PnlDockManager *self)
{
}

PnlDockManager *
pnl_dock_manager_new (void)
{
  return g_object_new (PNL_TYPE_DOCK_MANAGER, NULL);
}

void
pnl_dock_manager_register_dock (PnlDockManager *self,
                                PnlDock        *dock)
{
  g_return_if_fail (PNL_IS_DOCK_MANAGER (self));
  g_return_if_fail (PNL_IS_DOCK (dock));

  g_signal_emit (self, signals [REGISTER_DOCK], 0, dock);
}

void
pnl_dock_manager_unregister_dock (PnlDockManager *self,
                                  PnlDock        *dock)
{
  g_return_if_fail (PNL_IS_DOCK_MANAGER (self));
  g_return_if_fail (PNL_IS_DOCK (dock));

  g_signal_emit (self, signals [UNREGISTER_DOCK], 0, dock);
}
