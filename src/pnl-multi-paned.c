/* pnl-multi-paned.c
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

#include "pnl-multi-paned.h"

typedef struct
{
  GtkWidget *widget;
  gdouble    fraction;
} PnlMultiPanedChild;

typedef struct
{
  GArray         *children;
  GtkOrientation  orientation;
} PnlMultiPanedPrivate;

G_DEFINE_TYPE_EXTENDED (PnlMultiPaned, pnl_multi_paned, GTK_TYPE_CONTAINER, 0,
                        G_ADD_PRIVATE (PnlMultiPaned)
                        G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_ORIENTATION,
  LAST_PROP
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_FRACTION,
  LAST_CHILD_PROP
};

static GParamSpec *properties [LAST_CHILD_PROP];
static GParamSpec *child_properties [LAST_CHILD_PROP];

static void
pnl_multi_paned_reset_fractions (PnlMultiPaned *self)
{
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  gdouble each;
  guint i;

  g_assert (PNL_IS_MULTI_PANED (self));

  if (priv->children->len == 0)
    return;

  each = 1.0 / (gdouble)priv->children->len;

  for (i = 0; i < priv->children->len; i++)
    {
      PnlMultiPanedChild *child = &g_array_index (priv->children, PnlMultiPanedChild, i);

      child->fraction = each;
      gtk_container_child_notify_by_pspec (GTK_CONTAINER (self),
                                           child->widget,
                                           child_properties [CHILD_PROP_FRACTION]);
    }
}

static PnlMultiPanedChild *
pnl_multi_paned_get_child (PnlMultiPaned *self,
                           GtkWidget     *widget)
{
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      PnlMultiPanedChild *child = &g_array_index (priv->children, PnlMultiPanedChild, i);

      if (child->widget == widget)
        return child;
    }

  g_assert_not_reached ();

  return NULL;
}

static gdouble
pnl_multi_paned_get_child_fraction (PnlMultiPaned *self,
                                    GtkWidget     *widget)
{
  PnlMultiPanedChild *child;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (GTK_IS_WIDGET (widget));

  child = pnl_multi_paned_get_child (self, widget);

  return child->fraction;
}

static void
pnl_multi_paned_set_child_fraction (PnlMultiPaned *self,
                                    GtkWidget     *widget,
                                    gdouble        fraction)
{
  PnlMultiPanedChild *child;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (fraction >= 0.0);
  g_assert (fraction <= 1.0);

  child = pnl_multi_paned_get_child (self, widget);

  if (child->fraction != fraction)
    {
      child->fraction = fraction;
      gtk_container_child_notify_by_pspec (GTK_CONTAINER (self), widget,
                                           child_properties [CHILD_PROP_FRACTION]);
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}

static void
pnl_multi_paned_add (GtkContainer *container,
                     GtkWidget    *widget)
{
  PnlMultiPaned *self = (PnlMultiPaned *)container;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  PnlMultiPanedChild child = { 0 };

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (GTK_IS_WIDGET (widget));

  child.widget = g_object_ref_sink (widget);
  child.fraction = 0.0;

  g_array_append_val (priv->children, child);

  gtk_widget_set_parent (widget, GTK_WIDGET (self));
  pnl_multi_paned_reset_fractions (self);
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
pnl_multi_paned_remove (GtkContainer *container,
                        GtkWidget    *widget)
{
  PnlMultiPaned *self = (PnlMultiPaned *)container;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      PnlMultiPanedChild *child = &g_array_index (priv->children, PnlMultiPanedChild, i);

      if (child->widget == widget)
        {
          g_array_remove_index (priv->children, i);
          gtk_widget_unparent (widget);
          g_object_unref (widget);
          pnl_multi_paned_reset_fractions (self);
          gtk_widget_queue_resize (GTK_WIDGET (self));
          break;
        }
    }
}

static void
pnl_multi_paned_forall (GtkContainer *container,
                        gboolean      include_internals,
                        GtkCallback   callback,
                        gpointer      user_data)
{
  PnlMultiPaned *self = (PnlMultiPaned *)container;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  gint i;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (callback != NULL);

  for (i = priv->children->len; i > 0; i--)
    {
      PnlMultiPanedChild *child = &g_array_index (priv->children, PnlMultiPanedChild, i - 1);

      callback (child->widget, user_data);
    }
}

static GtkSizeRequestMode
pnl_multi_paned_get_request_mode (GtkWidget *widget)
{
  PnlMultiPaned *self = (PnlMultiPaned *)widget;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  g_assert (PNL_IS_MULTI_PANED (self));

  return (priv->orientation == GTK_ORIENTATION_HORIZONTAL) ? GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT
                                                           : GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
pnl_multi_paned_get_preferred_height (GtkWidget *widget,
                                      gint      *min_height,
                                      gint      *nat_height)
{
  PnlMultiPaned *self = (PnlMultiPaned *)widget;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  guint i;
  gint real_min_height = 0;
  gint real_nat_height = 0;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  for (i = 0; i < priv->children->len; i++)
    {
      PnlMultiPanedChild *child = &g_array_index (priv->children, PnlMultiPanedChild, i);
      gint child_min_height = 0;
      gint child_nat_height = 0;

      if (gtk_widget_get_visible (child->widget))
        {
          gtk_widget_get_preferred_height (child->widget, &child_min_height, &child_nat_height);

          if (priv->orientation == GTK_ORIENTATION_VERTICAL)
            {
              real_min_height += child_min_height;
              real_nat_height += child_nat_height;
            }
          else
            {
              real_min_height = MAX (real_min_height, child_min_height);
              real_nat_height = MAX (real_nat_height, child_nat_height);
            }
        }
    }

  *min_height = real_min_height;
  *nat_height = real_nat_height;
}

static void
pnl_multi_paned_get_child_preferred_height_for_width (PnlMultiPaned      *self,
                                                      PnlMultiPanedChild *children,
                                                      gint                n_children,
                                                      gint                width,
                                                      gint               *min_height,
                                                      gint               *nat_height)
{
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  PnlMultiPanedChild *child = children;
  gint child_min_height = 0;
  gint child_nat_height = 0;
  gint neighbor_min_height = 0;
  gint neighbor_nat_height = 0;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (n_children == 0 || children != NULL);
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  *min_height = 0;
  *nat_height = 0;

  if (n_children == 0)
    return;

  if (gtk_widget_get_visible (child->widget))
    gtk_widget_get_preferred_height_for_width (child->widget,
                                               width,
                                               &child_min_height,
                                               &child_nat_height);

  pnl_multi_paned_get_child_preferred_height_for_width (self,
                                                        children + 1,
                                                        n_children - 1,
                                                        width,
                                                        &neighbor_min_height,
                                                        &neighbor_nat_height);

  if (priv->orientation == GTK_ORIENTATION_VERTICAL)
    {
      *min_height = child_min_height + neighbor_min_height;
      *nat_height = child_nat_height + neighbor_nat_height;
    }
  else
    {
      *min_height = MAX (child_min_height, neighbor_min_height);
      *nat_height = MAX (child_nat_height, neighbor_nat_height);
    }
}

static void
pnl_multi_paned_get_preferred_height_for_width (GtkWidget *widget,
                                                gint       width,
                                                gint      *min_height,
                                                gint      *nat_height)
{
  PnlMultiPaned *self = (PnlMultiPaned *)widget;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  *min_height = 0;
  *nat_height = 0;

  pnl_multi_paned_get_child_preferred_height_for_width (self,
                                                        (PnlMultiPanedChild *)(gpointer)priv->children->data,
                                                        priv->children->len,
                                                        width,
                                                        min_height,
                                                        nat_height);
}

static void
pnl_multi_paned_get_preferred_width (GtkWidget *widget,
                                     gint      *min_width,
                                     gint      *nat_width)
{
  PnlMultiPaned *self = (PnlMultiPaned *)widget;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  guint i;
  gint real_min_width = 0;
  gint real_nat_width = 0;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  for (i = 0; i < priv->children->len; i++)
    {
      PnlMultiPanedChild *child = &g_array_index (priv->children, PnlMultiPanedChild, i);
      gint child_min_width = 0;
      gint child_nat_width = 0;

      if (gtk_widget_get_visible (child->widget))
        {
          gtk_widget_get_preferred_width (child->widget, &child_min_width, &child_nat_width);

          if (priv->orientation == GTK_ORIENTATION_VERTICAL)
            {
              real_min_width = MAX (real_min_width, child_min_width);
              real_nat_width = MAX (real_nat_width, child_nat_width);
            }
          else
            {
              real_min_width += child_min_width;
              real_nat_width += child_nat_width;
            }
        }
    }

  *min_width = real_min_width;
  *nat_width = real_nat_width;
}

static void
pnl_multi_paned_get_child_preferred_width_for_height (PnlMultiPaned      *self,
                                                      PnlMultiPanedChild *children,
                                                      gint                n_children,
                                                      gint                height,
                                                      gint               *min_width,
                                                      gint               *nat_width)
{
  PnlMultiPanedChild *child = children;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  gint child_min_width = 0;
  gint child_nat_width = 0;
  gint neighbor_min_width = 0;
  gint neighbor_nat_width = 0;

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (n_children == 0 || children != NULL);
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  *min_width = 0;
  *nat_width = 0;

  if (n_children == 0)
    return;

  if (gtk_widget_get_visible (child->widget))
    gtk_widget_get_preferred_height_for_width (child->widget,
                                               height,
                                               &child_min_width,
                                               &child_nat_width);

  pnl_multi_paned_get_child_preferred_width_for_height (self,
                                                        children + 1,
                                                        n_children - 1,
                                                        height,
                                                        &neighbor_min_width,
                                                        &neighbor_nat_width);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      *min_width = child_min_width + neighbor_min_width;
      *nat_width = child_nat_width + neighbor_nat_width;
    }
  else
    {
      *min_width = MAX (child_min_width, neighbor_min_width);
      *nat_width = MAX (child_nat_width, neighbor_nat_width);
    }
}

static void
pnl_multi_paned_get_preferred_width_for_height (GtkWidget *widget,
                                                gint       height,
                                                gint      *min_width,
                                                gint      *nat_width)
{
  PnlMultiPaned *self = (PnlMultiPaned *)widget;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  pnl_multi_paned_get_child_preferred_width_for_height (self,
                                                        (PnlMultiPanedChild *)(gpointer)priv->children->data,
                                                        priv->children->len,
                                                        height,
                                                        min_width,
                                                        nat_width);
}

static void
pnl_multi_paned_child_size_allocate (PnlMultiPaned      *self,
                                     PnlMultiPanedChild *children,
                                     gint                n_children,
                                     GtkAllocation      *allocation,
                                     gint                total_width,
                                     gint                total_height)
{
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  PnlMultiPanedChild *child = children;
  GtkAllocation child_alloc = { 0 };

  g_assert (PNL_IS_MULTI_PANED (self));
  g_assert (n_children == 0 || children != NULL);
  g_assert (allocation != NULL);

  if (n_children == 0)
    return;

  /*
   * Size allocation is performed by determining the size request of
   * all children towards the end of the allocation, and then comparing
   * that to the "position" of this child. If minimum allocations
   * require, we might fudge our position to allow for following children
   * to fit.
   */

  if (!gtk_widget_get_visible (child->widget))
    goto next_child;

  if (n_children == 1)
    {
      gtk_widget_size_allocate (child->widget, allocation);
      return;
    }

  child_alloc.x = allocation->x;
  child_alloc.y = allocation->y;

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      gint neighbor_min_width = 0;
      gint neighbor_nat_width = 0;
      gint child_min_width = 0;
      gint child_nat_width = 0;

      gtk_widget_get_preferred_width_for_height (child->widget,
                                                 allocation->height,
                                                 &child_min_width,
                                                 &child_nat_width);

      pnl_multi_paned_get_child_preferred_width_for_height (self,
                                                            children + 1,
                                                            n_children - 1,
                                                            allocation->height,
                                                            &neighbor_min_width,
                                                            &neighbor_nat_width);

      child_alloc.height = allocation->height;

      if ((child->fraction * total_width) <= (allocation->width - neighbor_min_width))
        child_alloc.width = child->fraction * total_width;
      else
        child_alloc.width = allocation->width - neighbor_min_width;

      allocation->x += child_alloc.width;
      allocation->width -= child_alloc.width;
    }
  else
    {
      gint neighbor_min_height = 0;
      gint neighbor_nat_height = 0;
      gint child_min_height = 0;
      gint child_nat_height = 0;

      gtk_widget_get_preferred_height_for_width (child->widget,
                                                 allocation->width,
                                                 &child_min_height,
                                                 &child_nat_height);

      pnl_multi_paned_get_child_preferred_height_for_width (self,
                                                            children + 1,
                                                            n_children - 1,
                                                            allocation->width,
                                                            &neighbor_min_height,
                                                            &neighbor_nat_height);

      child_alloc.width = allocation->width;

      if ((child->fraction * total_height) <= (allocation->height - neighbor_min_height))
        child_alloc.height = child->fraction * total_height;
      else
        child_alloc.height = allocation->height - neighbor_min_height;

      allocation->y += child_alloc.height;
      allocation->height -= child_alloc.height;
    }

  gtk_widget_size_allocate (child->widget, &child_alloc);

next_child:
  pnl_multi_paned_child_size_allocate (self,
                                       children + 1,
                                       n_children - 1,
                                       allocation,
                                       total_width,
                                       total_height);
}

static void
pnl_multi_paned_size_allocate (GtkWidget     *widget,
                               GtkAllocation *allocation)
{
  PnlMultiPaned *self = (PnlMultiPaned *)widget;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);
  GtkAllocation old_allocation;

  g_assert (PNL_IS_MULTI_PANED (self));

  gtk_widget_get_allocation (widget, &old_allocation);
  gtk_widget_set_allocation (widget, allocation);

  if (priv->children->len > 0)
    pnl_multi_paned_child_size_allocate (self,
                                         (PnlMultiPanedChild *)(gpointer)priv->children->data,
                                         priv->children->len,
                                         allocation,
                                         allocation->width,
                                         allocation->height);
}

static void
pnl_multi_paned_get_child_property (GtkContainer *container,
                                    GtkWidget    *widget,
                                    guint         prop_id,
                                    GValue       *value,
                                    GParamSpec   *pspec)
{
  PnlMultiPaned *self = PNL_MULTI_PANED (container);

  switch (prop_id)
    {
    case CHILD_PROP_FRACTION:
      g_value_set_double (value, pnl_multi_paned_get_child_fraction (self, widget));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
pnl_multi_paned_set_child_property (GtkContainer *container,
                                    GtkWidget    *widget,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PnlMultiPaned *self = PNL_MULTI_PANED (container);

  switch (prop_id)
    {
    case CHILD_PROP_FRACTION:
      pnl_multi_paned_set_child_fraction (self, widget, g_value_get_double (value));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
pnl_multi_paned_finalize (GObject *object)
{
  PnlMultiPaned *self = (PnlMultiPaned *)object;
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  g_clear_pointer (&priv->children, g_array_unref);

  G_OBJECT_CLASS (pnl_multi_paned_parent_class)->finalize (object);
}

static void
pnl_multi_paned_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PnlMultiPaned *self = PNL_MULTI_PANED (object);
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_multi_paned_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PnlMultiPaned *self = PNL_MULTI_PANED (object);
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      priv->orientation = g_value_get_enum (value);
      gtk_widget_queue_resize (GTK_WIDGET (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
pnl_multi_paned_class_init (PnlMultiPanedClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = pnl_multi_paned_get_property;
  object_class->set_property = pnl_multi_paned_set_property;
  object_class->finalize = pnl_multi_paned_finalize;

  widget_class->get_request_mode = pnl_multi_paned_get_request_mode;
  widget_class->get_preferred_width = pnl_multi_paned_get_preferred_width;
  widget_class->get_preferred_height = pnl_multi_paned_get_preferred_height;
  widget_class->get_preferred_width_for_height = pnl_multi_paned_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = pnl_multi_paned_get_preferred_height_for_width;
  widget_class->size_allocate = pnl_multi_paned_size_allocate;

  container_class->add = pnl_multi_paned_add;
  container_class->remove = pnl_multi_paned_remove;
  container_class->get_child_property = pnl_multi_paned_get_child_property;
  container_class->set_child_property = pnl_multi_paned_set_child_property;
  container_class->forall = pnl_multi_paned_forall;

  properties [PROP_ORIENTATION] =
    g_param_spec_enum ("orientation",
                       "Orientation",
                       "Orientation",
                       GTK_TYPE_ORIENTATION,
                       GTK_ORIENTATION_VERTICAL,
                       (G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  child_properties [CHILD_PROP_FRACTION] =
    g_param_spec_double ("fraction",
                         "Fraction",
                         "Fraction",
                         0,
                         1.0,
                         1.0,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gtk_container_class_install_child_properties (container_class, LAST_CHILD_PROP, child_properties);
}

static void
pnl_multi_paned_init (PnlMultiPaned *self)
{
  PnlMultiPanedPrivate *priv = pnl_multi_paned_get_instance_private (self);

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  priv->children = g_array_new (FALSE, TRUE, sizeof (PnlMultiPanedChild));
}

GtkWidget *
pnl_multi_paned_new (void)
{
  return g_object_new (PNL_TYPE_MULTI_PANED, NULL);
}
