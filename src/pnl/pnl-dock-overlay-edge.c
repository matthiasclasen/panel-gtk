/* pnl-dock-overlay-edge.c
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

#include "pnl-dock-overlay-edge-private.h"
#include "pnl-util-private.h"

struct _PnlDockOverlayEdge
{
  GtkBin          parent;
  GtkPositionType edge : 2;
  gint            position;
};

G_DEFINE_TYPE (PnlDockOverlayEdge, pnl_dock_overlay_edge, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_EDGE,
  PROP_POSITION,
  N_PROPS
};

enum {
  STYLE_PROP_0,
  STYLE_PROP_OVERLAP_SIZE,
  STYLE_PROP_MNEMONIC_OVERLAP_SIZE,
  N_STYLE_PROPS
};

static GParamSpec *properties [N_PROPS];
static GParamSpec *style_properties [N_STYLE_PROPS];

static gboolean
pnl_dock_overlay_edge_draw (GtkWidget *widget,
                            cairo_t   *cr)
{
  g_assert (PNL_IS_DOCK_OVERLAY_EDGE (widget));
  g_assert (cr != NULL);

  pnl_gtk_render_background_simple (widget, cr);

  return GTK_WIDGET_CLASS (pnl_dock_overlay_edge_parent_class)->draw (widget, cr);
}

static void
pnl_dock_overlay_edge_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  PnlDockOverlayEdge *self = PNL_DOCK_OVERLAY_EDGE (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      g_value_set_enum (value, pnl_dock_overlay_edge_get_edge (self));
      break;

    case PROP_POSITION:
      g_value_set_int (value, pnl_dock_overlay_edge_get_position (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_overlay_edge_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PnlDockOverlayEdge *self = PNL_DOCK_OVERLAY_EDGE (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      pnl_dock_overlay_edge_set_edge (self, g_value_get_enum (value));
      break;

    case PROP_POSITION:
      pnl_dock_overlay_edge_set_position (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_dock_overlay_edge_class_init (PnlDockOverlayEdgeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = pnl_dock_overlay_edge_get_property;
  object_class->set_property = pnl_dock_overlay_edge_set_property;

  widget_class->draw = pnl_dock_overlay_edge_draw;

  properties [PROP_EDGE] =
    g_param_spec_enum ("edge",
                       "Edge",
                       "Edge",
                       GTK_TYPE_POSITION_TYPE,
                       GTK_POS_LEFT,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_POSITION] =
    g_param_spec_int ("position",
                      "Position",
                      "The size of the edge",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  style_properties [STYLE_PROP_MNEMONIC_OVERLAP_SIZE] =
    g_param_spec_int ("mnemonic-overlap-size",
                      "Mnemonic Overlap Size",
                      "The amount of pixels to overlap when mnemonics are visible",
                      0,
                      G_MAXINT,
                      20,
                      (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  gtk_widget_class_install_style_property (widget_class,
                                           style_properties [STYLE_PROP_MNEMONIC_OVERLAP_SIZE]);

  style_properties [STYLE_PROP_OVERLAP_SIZE] =
    g_param_spec_int ("overlap-size",
                      "Overlap Size",
                      "The amount of pixels to overlap when hidden",
                      0,
                      G_MAXINT,
                      3,
                      (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  gtk_widget_class_install_style_property (widget_class,
                                           style_properties [STYLE_PROP_OVERLAP_SIZE]);
}

static void
pnl_dock_overlay_edge_init (PnlDockOverlayEdge *self)
{
}

gint
pnl_dock_overlay_edge_get_position (PnlDockOverlayEdge *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_OVERLAY_EDGE (self), 0);

  return self->position;
}

void
pnl_dock_overlay_edge_set_position (PnlDockOverlayEdge *self,
                                    gint                position)
{
  g_return_if_fail (PNL_IS_DOCK_OVERLAY_EDGE (self));
  g_return_if_fail (position >= 0);

  if (position != self->position)
    {
      self->position = position;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
    }
}

GtkPositionType
pnl_dock_overlay_edge_get_edge (PnlDockOverlayEdge *self)
{
  g_return_val_if_fail (PNL_IS_DOCK_OVERLAY_EDGE (self), 0);

  return self->edge;
}

void
pnl_dock_overlay_edge_set_edge (PnlDockOverlayEdge *self,
                                GtkPositionType     edge)
{
  g_return_if_fail (PNL_IS_DOCK_OVERLAY_EDGE (self));
  g_return_if_fail (edge >= 0);
  g_return_if_fail (edge <= 3);

  if (edge != self->edge)
    {
      self->edge = edge;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_EDGE]);
    }
}