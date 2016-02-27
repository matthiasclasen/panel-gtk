/* pnl-dock.c
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

#include <stdlib.h>

#include "pnl-dock.h"
#include "pnl-dock-edge.h"

#define HANDLE_WIDTH  10
#define HANDLE_HEIGHT 10

typedef enum
{
  PNL_DOCK_CHILD_LEFT   = GTK_POS_LEFT,
  PNL_DOCK_CHILD_RIGHT  = GTK_POS_RIGHT,
  PNL_DOCK_CHILD_TOP    = GTK_POS_TOP,
  PNL_DOCK_CHILD_BOTTOM = GTK_POS_BOTTOM,
  PNL_DOCK_CHILD_CENTER = 4,
  LAST_PNL_DOCK_CHILD   = 5
} PnlDockChildType;

typedef struct
{
  /*
   * The child widget in question.
   * Typically this is a PnlDockEdge, but the
   * center widget can be whatever.
   */
  GtkWidget *widget;

  /*
   * The type of child. The PNL_DOCK_CHILD_CENTER is always
   * the last child, and our sort function ensures that.
   */
  PnlDockChildType type;

  /*
   * The GdkWindow for the handle to resize the edge.
   * This is an input only window, the pane handle is drawn
   * with CSS by whatever styling the application has chose.
   */
  GdkWindow *handle;

  /*
   * When dragging, we need to know our offset relative to the
   * grab position to alter preferred size requests.
   */
  gint drag_offset;

  /*
   * This is the position of the child before the drag started.
   * We use this, combined with @drag_offset to determine the
   * size the child should be in the drag operation.
   */
  gint drag_begin_position;

  /*
   * Priority child property used to alter which child is
   * dominant in each slice stage. See
   * pnl_dock_get_children_preferred_width() for more information
   * on how the slicing is performed.
   */
  gint priority;

  /*
   * Cached size request used during size allocation.
   */
  GtkRequisition min_req;
  GtkRequisition nat_req;
} PnlDockChild;

typedef struct
{
  /*
   * All of our dock children, including edges and center child.
   */
  PnlDockChild children[LAST_PNL_DOCK_CHILD];

  /*
   * Actions used to toggle edge visibility.
   */
  GSimpleActionGroup *actions;

  /*
   * The pan gesture is used to resize edges.
   */
  GtkGesturePan *pan_gesture;

  /*
   * While in a pan gesture, we need to drag the current edge
   * being dragged. This is left, right, top, or bottom only.
   */
  PnlDockChild *drag_child;
} PnlDockPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (PnlDock, pnl_dock, GTK_TYPE_CONTAINER)

enum {
  CHILD_PROP_0,
  CHILD_PROP_POSITION,
  CHILD_PROP_PRIORITY,
  LAST_CHILD_PROP
};

static GParamSpec *child_properties [LAST_CHILD_PROP];

static gboolean
map_boolean_to_variant (GBinding     *binding,
                        const GValue *from_value,
                        GValue       *to_value,
                        gpointer      user_data)
{
  g_assert (G_IS_BINDING (binding));

  if (g_value_get_boolean (from_value))
    g_value_set_variant (to_value, g_variant_new_boolean (TRUE));
  else
    g_value_set_variant (to_value, g_variant_new_boolean (FALSE));

  return TRUE;
}

static PnlDockChild *
pnl_dock_get_child (PnlDock   *self,
                    GtkWidget *widget)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < G_N_ELEMENTS (priv->children); i++)
    {
      PnlDockChild *child = &priv->children [i];

      if ((GtkWidget *)child->widget == widget)
        return child;
    }

  g_assert_not_reached ();

  return NULL;
}

static PnlDockChild *
pnl_dock_get_child_typed (PnlDock          *self,
                          PnlDockChildType  type)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));
  g_assert (type >= PNL_DOCK_CHILD_LEFT);
  g_assert (type < LAST_PNL_DOCK_CHILD);

  for (i = 0; i < G_N_ELEMENTS (priv->children); i++)
    {
      PnlDockChild *child = &priv->children [i];

      if (child->type == type)
        return child;
    }

  g_assert_not_reached ();

  return NULL;
}

static GAction *
pnl_dock_get_action_for_type (PnlDock          *self,
                              PnlDockChildType  type)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  const gchar *name = NULL;

  g_assert (PNL_IS_DOCK (self));

  if (type == PNL_DOCK_CHILD_LEFT)
    name = "left-visible";
  else if (type == PNL_DOCK_CHILD_RIGHT)
    name = "right-visible";
  else if (type == PNL_DOCK_CHILD_TOP)
    name = "top-visible";
  else if (type == PNL_DOCK_CHILD_BOTTOM)
    name = "bottom-visible";
  else
    g_assert_not_reached ();

  return g_action_map_lookup_action (G_ACTION_MAP (priv->actions), name);
}

static void
pnl_dock_add (GtkContainer *container,
              GtkWidget    *widget)
{
  PnlDock *self = (PnlDock *)container;
  PnlDockChild *child;

  g_assert (PNL_IS_DOCK (self));
  g_assert (GTK_IS_WIDGET (widget));

  child = pnl_dock_get_child_typed (self, PNL_DOCK_CHILD_CENTER);

  if (child->widget != NULL)
    {
      g_warning ("Attempt to add a %s to a %s, but it already has a child of type %s",
                 G_OBJECT_TYPE_NAME (widget),
                 G_OBJECT_TYPE_NAME (self),
                 G_OBJECT_TYPE_NAME (child->widget));
      return;
    }

  child->widget = g_object_ref_sink (widget);
  gtk_widget_set_parent (widget, GTK_WIDGET (self));

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
pnl_dock_remove (GtkContainer *container,
                 GtkWidget    *widget)
{
  PnlDock *self = (PnlDock *)container;
  PnlDockChild *child;

  g_return_if_fail (PNL_IS_DOCK (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  child = pnl_dock_get_child (self, widget);
  gtk_widget_unparent (child->widget);
  g_clear_object (&child->widget);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
pnl_dock_forall (GtkContainer *container,
                 gboolean      include_internal,
                 GtkCallback   callback,
                 gpointer      user_data)
{
  PnlDock *self = (PnlDock *)container;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));
  g_assert (callback != NULL);

  for (i = G_N_ELEMENTS (priv->children); i > 0; i--)
    {
      PnlDockChild *child = &priv->children [i - 1];

      if (child->widget != NULL)
        callback (GTK_WIDGET (child->widget), user_data);
    }
}

static void
pnl_dock_get_children_preferred_width (PnlDock      *self,
                                       PnlDockChild *children,
                                       gint          n_children,
                                       gint         *min_width,
                                       gint         *nat_width)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  PnlDockChild *child = children;
  gint child_min_width = 0;
  gint child_nat_width = 0;
  gint neighbor_min_width = 0;
  gint neighbor_nat_width = 0;

  g_assert (PNL_IS_DOCK (self));
  g_assert (children != NULL);
  g_assert (n_children > 0);
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  *min_width = 0;
  *nat_width = 0;

  /*
   * We have a fairly simple rule for deducing the size request of
   * the children layout. Since children edges can have any priority,
   * we need to know how to slice them into areas that allow us to
   * combine (additive) or negotiate (maximum) widths with the
   * neighboring widgets.
   *
   *          .
   *          .
   *     +----+---------------------------------+
   *     |    |              2                  |
   *     |    +=================================+.....
   *     |    |                            |    |
   *     |    |                            |    |
   *     | 1  |              5             |    |
   *     |    |                            | 3  |
   *     |    +==.==.==.==.==.==.==.==.==.=+    |
   *     |    |              4             |    |
   *     +----+----------------------------+----+
   *          .                            .
   *          .                            .
   *
   * The children are sorted in their weighting order. Each child
   * will dominate the leftover allocation, in the orientation that
   * matters.
   *
   * 1 and 3 in the diagram above will always be additive with their
   * neighbors horizontal neighbors. See the guide does for how this
   * gets sliced. Even if 3 were dominant (instead of 2), it would still
   * be additive to its neighbors. Same for 1.
   *
   * Both 2 and 4, will always negotiate their widths with the next
   * child.
   *
   * This allows us to make a fairly simple recursive function to
   * size ourselves and then call again with the next child, working our
   * way down to 5 (the center widget).
   *
   * At this point, we walk back up the recursive-stack and do our
   * adding or negotiating.
   */

  if (child->widget != NULL)
    gtk_widget_get_preferred_width (child->widget, &child_min_width, &child_nat_width);

  if (child == priv->drag_child)
    child_nat_width = MAX (child_min_width,
                           child->drag_begin_position + child->drag_offset);

  if (n_children > 1)
    pnl_dock_get_children_preferred_width (self,
                                           &children [1],
                                           n_children - 1,
                                           &neighbor_min_width,
                                           &neighbor_nat_width);

  switch (child->type)
    {
    case PNL_DOCK_CHILD_LEFT:
    case PNL_DOCK_CHILD_RIGHT:
      *min_width = (child_min_width + neighbor_min_width);
      *nat_width = (child_nat_width + neighbor_nat_width);
      break;

    case PNL_DOCK_CHILD_TOP:
    case PNL_DOCK_CHILD_BOTTOM:
      *min_width = MAX (child_min_width, neighbor_min_width);
      *nat_width = MAX (child_nat_width, neighbor_nat_width);
      break;

    case PNL_DOCK_CHILD_CENTER:
      *min_width = child_min_width;
      *nat_width = child_min_width;
      break;

    case LAST_PNL_DOCK_CHILD:
    default:
      g_assert_not_reached ();
    }

  child->min_req.width = *min_width;
  child->nat_req.width = *nat_width;
}

static void
pnl_dock_get_preferred_width (GtkWidget *widget,
                              gint      *min_width,
                              gint      *nat_width)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);

  g_assert (PNL_IS_DOCK (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  pnl_dock_get_children_preferred_width (self,
                                         priv->children,
                                         G_N_ELEMENTS (priv->children),
                                         min_width,
                                         nat_width);
}

static void
pnl_dock_get_children_preferred_height (PnlDock      *self,
                                        PnlDockChild *children,
                                        gint          n_children,
                                        gint         *min_height,
                                        gint         *nat_height)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  PnlDockChild *child = children;
  gint child_min_height = 0;
  gint child_nat_height = 0;
  gint neighbor_min_height = 0;
  gint neighbor_nat_height = 0;

  g_assert (PNL_IS_DOCK (self));
  g_assert (children != NULL);
  g_assert (n_children > 0);
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  *min_height = 0;
  *nat_height = 0;

  /*
   * See pnl_dock_get_children_preferred_width() for more information on
   * how this works. This works just like that but the negotiated/additive
   * operations are switched between the left/right and top/bottom.
   */

  if (child->widget != NULL)
    gtk_widget_get_preferred_height (child->widget, &child_min_height, &child_nat_height);

  if (child == priv->drag_child)
    child_nat_height = MAX (child_min_height,
                            child->drag_begin_position + child->drag_offset);

  if (n_children > 1)
    pnl_dock_get_children_preferred_height (self,
                                            &children [1],
                                            n_children - 1,
                                            &neighbor_min_height,
                                            &neighbor_nat_height);

  switch (child->type)
    {
    case PNL_DOCK_CHILD_LEFT:
    case PNL_DOCK_CHILD_RIGHT:
      *min_height = MAX (child_min_height, neighbor_min_height);
      *nat_height = MAX (child_nat_height, neighbor_nat_height);
      break;

    case PNL_DOCK_CHILD_TOP:
    case PNL_DOCK_CHILD_BOTTOM:
      *min_height = (child_min_height + neighbor_min_height);
      *nat_height = (child_nat_height + neighbor_nat_height);
      break;

    case PNL_DOCK_CHILD_CENTER:
      *min_height = child_min_height;
      *nat_height = child_min_height;
      break;

    case LAST_PNL_DOCK_CHILD:
    default:
      g_assert_not_reached ();
    }

  child->min_req.height = *min_height;
  child->nat_req.height = *nat_height;
}

static void
pnl_dock_get_preferred_height (GtkWidget *widget,
                               gint      *min_height,
                               gint      *nat_height)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);

  g_assert (PNL_IS_DOCK (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  pnl_dock_get_children_preferred_height (self,
                                          priv->children,
                                          G_N_ELEMENTS (priv->children),
                                          min_height,
                                          nat_height);
}

static void
pnl_dock_negotiate_size (PnlDock              *self,
                         const GtkAllocation  *allocation,
                         const GtkRequisition *child_min,
                         const GtkRequisition *child_nat,
                         const GtkRequisition *neighbor_min,
                         const GtkRequisition *neighbor_nat,
                         GtkAllocation        *child_alloc)
{
  g_assert (PNL_IS_DOCK (self));
  g_assert (allocation != NULL);
  g_assert (child_min != NULL);
  g_assert (child_nat != NULL);
  g_assert (neighbor_min != NULL);
  g_assert (neighbor_nat != NULL);
  g_assert (child_alloc != NULL);

  if (allocation->width - child_nat->width < neighbor_min->width)
    child_alloc->width = allocation->width - neighbor_min->width;
  else
    child_alloc->width = child_nat->width;

  if (allocation->height - child_nat->height < neighbor_min->height)
    child_alloc->height = allocation->height - neighbor_min->height;
  else
    child_alloc->height = child_nat->height;
}

static void
pnl_dock_child_size_allocate (PnlDock       *self,
                              PnlDockChild  *children,
                              gint           n_children,
                              GtkAllocation *allocation)
{
  PnlDockChild *child = children;

  g_assert (PNL_IS_DOCK (self));
  g_assert (children != NULL);
  g_assert (n_children >= 1);
  g_assert (allocation != NULL);

  if (n_children == 1)
    {
      g_assert (child->type == PNL_DOCK_CHILD_CENTER);

      if (child->widget != NULL && gtk_widget_get_visible (child->widget))
        gtk_widget_size_allocate (child->widget, allocation);

      return;
    }

  if (child->widget != NULL && gtk_widget_get_visible (child->widget))
    {
      GtkAllocation child_alloc = { 0 };
      GtkAllocation handle_alloc = { 0 };
      GtkRequisition neighbor_min = { 0 };
      GtkRequisition neighbor_nat = { 0 };

      pnl_dock_get_children_preferred_height (self, child, 1,
                                              &child->min_req.height,
                                              &child->nat_req.height);
      pnl_dock_get_children_preferred_width (self, child, 1,
                                             &child->min_req.width,
                                             &child->nat_req.width);

      pnl_dock_get_children_preferred_height (self,
                                              &children [1],
                                              n_children - 1,
                                              &neighbor_min.height,
                                              &neighbor_nat.height);

      pnl_dock_get_children_preferred_width (self,
                                             &children [1],
                                             n_children - 1,
                                             &neighbor_min.width,
                                             &neighbor_nat.width);

      pnl_dock_negotiate_size (self,
                               allocation,
                               &child->min_req,
                               &child->nat_req,
                               &neighbor_min,
                               &neighbor_nat,
                               &child_alloc);

      switch (child->type)
        {
        case PNL_DOCK_CHILD_LEFT:
          child_alloc.x = allocation->x;
          child_alloc.y = allocation->y;
          child_alloc.height = allocation->height;
          allocation->x += child_alloc.width;
          allocation->width -= child_alloc.width;
          break;

        case PNL_DOCK_CHILD_RIGHT:
          child_alloc.x = allocation->x + allocation->width - child_alloc.width;
          child_alloc.y = allocation->y;
          child_alloc.height = allocation->height;
          allocation->width -= child_alloc.width;
          break;

        case PNL_DOCK_CHILD_TOP:
          child_alloc.x = allocation->x;
          child_alloc.y = allocation->y;
          child_alloc.width = allocation->width;
          allocation->y += child_alloc.height;
          allocation->height -= child_alloc.height;
          break;

        case PNL_DOCK_CHILD_BOTTOM:
          child_alloc.x = allocation->x;
          child_alloc.y = allocation->y + allocation->height - child_alloc.height;
          child_alloc.width = allocation->width;
          allocation->height -= child_alloc.height;
          break;

        case PNL_DOCK_CHILD_CENTER:
        case LAST_PNL_DOCK_CHILD:
        default:
          g_assert_not_reached ();
          break;
        }

      handle_alloc = child_alloc;

      switch (child->type)
        {
        case PNL_DOCK_CHILD_LEFT:
          handle_alloc.x += handle_alloc.width - HANDLE_WIDTH;
          handle_alloc.width = HANDLE_WIDTH;

        case PNL_DOCK_CHILD_RIGHT:
          handle_alloc.width = HANDLE_WIDTH;
          break;

        case PNL_DOCK_CHILD_BOTTOM:
          handle_alloc.height = HANDLE_HEIGHT;
          break;

        case PNL_DOCK_CHILD_TOP:
          handle_alloc.y += handle_alloc.height - HANDLE_HEIGHT;
          handle_alloc.height = HANDLE_HEIGHT;
          break;

        case PNL_DOCK_CHILD_CENTER:
        case LAST_PNL_DOCK_CHILD:
        default:
          break;
        }

      if (child_alloc.width > 0 && child_alloc.height > 0)
        gdk_window_move_resize (child->handle,
                                handle_alloc.x, handle_alloc.y,
                                handle_alloc.width, handle_alloc.height);

      gtk_widget_size_allocate (child->widget, &child_alloc);
    }

  pnl_dock_child_size_allocate (self, &children [1], n_children - 1, allocation);
}

static void
pnl_dock_size_allocate (GtkWidget     *widget,
                        GtkAllocation *allocation)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);

  g_assert (PNL_IS_DOCK (self));
  g_assert (allocation != NULL);

  gtk_widget_set_allocation (widget, allocation);
  pnl_dock_child_size_allocate (self, priv->children, G_N_ELEMENTS (priv->children), allocation);
}

static void
pnl_dock_visible_action (GSimpleAction *action,
                         GVariant      *state,
                         gpointer       user_data)
{
  PnlDock *self = user_data;
  PnlDockChild *child;
  PnlDockChildType type;
  const gchar *action_name;
  gboolean reveal_child;

  g_assert (PNL_IS_DOCK (self));
  g_assert (G_IS_SIMPLE_ACTION (action));
  g_assert (state != NULL);
  g_assert (g_variant_is_of_type (state, G_VARIANT_TYPE_BOOLEAN));

  action_name = g_action_get_name (G_ACTION (action));
  reveal_child = g_variant_get_boolean (state);

  if (g_str_has_prefix (action_name, "left"))
    type = PNL_DOCK_CHILD_LEFT;
  else if (g_str_has_prefix (action_name, "right"))
    type = PNL_DOCK_CHILD_RIGHT;
  else if (g_str_has_prefix (action_name, "top"))
    type = PNL_DOCK_CHILD_TOP;
  else if (g_str_has_prefix (action_name, "bottom"))
    type = PNL_DOCK_CHILD_BOTTOM;
  else
    return;

  child = pnl_dock_get_child_typed (self, type);

  gtk_revealer_set_reveal_child (GTK_REVEALER (child->widget), reveal_child);
}

static gint
pnl_dock_child_compare (gconstpointer a,
                        gconstpointer b)
{
  const PnlDockChild *child_a = a;
  const PnlDockChild *child_b = b;

  if (child_a->type == PNL_DOCK_CHILD_CENTER)
    return 1;
  else if (child_b->type == PNL_DOCK_CHILD_CENTER)
    return -1;

  return child_a->priority - child_b->priority;
}

static void
pnl_dock_set_child_priority (PnlDock   *self,
                             GtkWidget *widget,
                             gint       priority)
{
  PnlDockChild *child;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);

  g_assert (PNL_IS_DOCK (self));
  g_assert (GTK_IS_WIDGET (widget));

  child = pnl_dock_get_child (self, widget);
  child->priority = priority;

  g_qsort_with_data (&priv->children[0],
                     PNL_DOCK_CHILD_CENTER,
                     sizeof (PnlDockChild),
                     (GCompareDataFunc)pnl_dock_child_compare,
                     NULL);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
pnl_dock_create_child_handle (PnlDock      *self,
                              PnlDockChild *child)
{
  GdkWindowAttr attributes = { 0 };
  GdkDisplay *display;
  GdkWindow *parent;
  GdkCursorType cursor_type;

  g_assert (PNL_IS_DOCK (self));
  g_assert (child != NULL);
  g_assert (child->type < PNL_DOCK_CHILD_CENTER);
  g_assert (child->handle == NULL);

  display = gtk_widget_get_display (GTK_WIDGET (self));
  parent = gtk_widget_get_window (GTK_WIDGET (self));

  cursor_type = (child->type == PNL_DOCK_CHILD_LEFT || child->type == PNL_DOCK_CHILD_RIGHT)
              ? GDK_SB_H_DOUBLE_ARROW
              : GDK_SB_V_DOUBLE_ARROW;

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_ONLY;
  attributes.x = -1;
  attributes.x = -1;
  attributes.width = 1;
  attributes.height = 1;
  attributes.visual = gtk_widget_get_visual (GTK_WIDGET (self));
  attributes.event_mask = (GDK_BUTTON_PRESS_MASK |
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_ENTER_NOTIFY_MASK |
                           GDK_LEAVE_NOTIFY_MASK |
                           GDK_POINTER_MOTION_MASK);
  attributes.cursor = gdk_cursor_new_for_display (display, cursor_type);

  child->handle = gdk_window_new (parent, &attributes, GDK_WA_CURSOR);
  gtk_widget_register_window (GTK_WIDGET (self), child->handle);

  g_clear_object (&attributes.cursor);
}

static void
pnl_dock_destroy_child_handle (PnlDock      *self,
                               PnlDockChild *child)
{
  g_assert (PNL_IS_DOCK (self));
  g_assert (child != NULL);
  g_assert (child->type < PNL_DOCK_CHILD_CENTER);

  if (child->handle != NULL)
    {
      gdk_window_destroy (child->handle);
      child->handle = NULL;
    }
}

static void
pnl_dock_realize (GtkWidget *widget)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));

  GTK_WIDGET_CLASS (pnl_dock_parent_class)->realize (widget);

  for (i = 0; i < PNL_DOCK_CHILD_CENTER; i++)
    {
      PnlDockChild *child = &priv->children [i];

      pnl_dock_create_child_handle (self, child);
    }
}

static void
pnl_dock_unrealize (GtkWidget *widget)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));

  GTK_WIDGET_CLASS (pnl_dock_parent_class)->unrealize (widget);

  for (i = 0; i < PNL_DOCK_CHILD_CENTER; i++)
    {
      PnlDockChild *child = &priv->children [i];

      pnl_dock_destroy_child_handle (self, child);
    }
}

static void
pnl_dock_map (GtkWidget *widget)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));

  GTK_WIDGET_CLASS (pnl_dock_parent_class)->map (widget);

  for (i = 0; i < PNL_DOCK_CHILD_CENTER; i++)
    {
      PnlDockChild *child = &priv->children [i];

      if (child->handle != NULL)
        gdk_window_show (child->handle);
    }
}

static void
pnl_dock_unmap (GtkWidget *widget)
{
  PnlDock *self = (PnlDock *)widget;
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  guint i;

  g_assert (PNL_IS_DOCK (self));

  for (i = 0; i < PNL_DOCK_CHILD_CENTER; i++)
    {
      PnlDockChild *child = &priv->children [i];

      if (child->handle != NULL)
        gdk_window_hide (child->handle);
    }

  GTK_WIDGET_CLASS (pnl_dock_parent_class)->unmap (widget);
}

static void
pnl_dock_pan_gesture_drag_begin (PnlDock       *self,
                                 gdouble        x,
                                 gdouble        y,
                                 GtkGesturePan *gesture)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  GdkEventSequence *sequence;
  PnlDockChild *child = NULL;
  GtkAllocation child_alloc;
  const GdkEvent *event;
  guint i;

  g_assert (PNL_IS_DOCK (self));
  g_assert (GTK_IS_GESTURE_PAN (gesture));

  sequence = gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  event = gtk_gesture_get_last_event (GTK_GESTURE (gesture), sequence);

  for (i = 0; i < G_N_ELEMENTS (priv->children); i++)
    {
      if (priv->children [i].handle == event->any.window)
        {
          child = &priv->children [i];
          break;
        }
    }

  if (child == NULL || child->type >= PNL_DOCK_CHILD_CENTER)
    {
      gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

  gtk_widget_get_allocation (child->widget, &child_alloc);

  priv->drag_child = child;
  priv->drag_child->drag_offset = 0;

  if (child->type == PNL_DOCK_CHILD_LEFT || child->type == PNL_DOCK_CHILD_RIGHT)
    {
      gtk_gesture_pan_set_orientation (gesture, GTK_ORIENTATION_HORIZONTAL);
      priv->drag_child->drag_begin_position = child_alloc.width;
    }
  else
    {
      gtk_gesture_pan_set_orientation (gesture, GTK_ORIENTATION_VERTICAL);
      priv->drag_child->drag_begin_position = child_alloc.height;
    }

  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
pnl_dock_pan_gesture_drag_end (PnlDock       *self,
                               gdouble        x,
                               gdouble        y,
                               GtkGesturePan *gesture)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  GdkEventSequence *sequence;
  GtkEventSequenceState state;
  GtkAllocation child_alloc;
  gint position;

  g_assert (PNL_IS_DOCK (self));
  g_assert (GTK_IS_GESTURE_PAN (gesture));

  sequence = gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  state = gtk_gesture_get_sequence_state (GTK_GESTURE (gesture), sequence);

  if (state == GTK_EVENT_SEQUENCE_DENIED)
    goto cleanup;

  g_assert (priv->drag_child != NULL);
  g_assert (PNL_IS_DOCK_EDGE (priv->drag_child->widget));

  gtk_widget_get_allocation (priv->drag_child->widget, &child_alloc);

  if ((priv->drag_child->type == PNL_DOCK_CHILD_LEFT) ||
      (priv->drag_child->type == PNL_DOCK_CHILD_RIGHT))
    position = child_alloc.width;
  else
    position = child_alloc.height;

  pnl_dock_edge_set_position (PNL_DOCK_EDGE (priv->drag_child->widget), position);

cleanup:
  if (priv->drag_child != NULL)
    {
      priv->drag_child->drag_offset = 0;
      priv->drag_child->drag_begin_position = 0;
      priv->drag_child = NULL;
    }
}

static void
pnl_dock_pan_gesture_pan (PnlDock         *self,
                          GtkPanDirection  direction,
                          gdouble          offset,
                          GtkGesturePan   *gesture)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  gint position;

  g_assert (PNL_IS_DOCK (self));
  g_assert (GTK_IS_GESTURE_PAN (gesture));
  g_assert (priv->drag_child != NULL);
  g_assert (priv->drag_child->type < PNL_DOCK_CHILD_CENTER);

  /*
   * This callback is used to adjust the size allocation of the edge in
   * question (denoted by priv->drag_child). It is always one of the
   * left, right, top, or bottom children. Never the center child.
   *
   * Because of how GtkRevealer works, we are doing a bit of a workaround
   * here. We need the revealer (the PnlDockEdge) child to have a size
   * request that matches the visible area, otherwise animating out the
   * revealer will not look right.
   */

  if (direction == GTK_PAN_DIRECTION_UP)
    {
      if (priv->drag_child->type == PNL_DOCK_CHILD_TOP)
        offset = -offset;
    }
  else if (direction == GTK_PAN_DIRECTION_DOWN)
    {
      if (priv->drag_child->type == PNL_DOCK_CHILD_BOTTOM)
        offset = -offset;
    }
  else if (direction == GTK_PAN_DIRECTION_LEFT)
    {
      if (priv->drag_child->type == PNL_DOCK_CHILD_LEFT)
        offset = -offset;
    }
  else if (direction == GTK_PAN_DIRECTION_RIGHT)
    {
      if (priv->drag_child->type == PNL_DOCK_CHILD_RIGHT)
        offset = -offset;
    }

  priv->drag_child->drag_offset = (gint)offset;

  position = priv->drag_child->drag_offset + priv->drag_child->drag_begin_position;
  if (position >= 0)
    pnl_dock_edge_set_position (PNL_DOCK_EDGE (priv->drag_child->widget), position);
}

static void
pnl_dock_create_pan_gesture (PnlDock *self)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  GtkGesture *gesture;

  g_assert (PNL_IS_DOCK (self));
  g_assert (priv->pan_gesture == NULL);

  gesture = gtk_gesture_pan_new (GTK_WIDGET (self), GTK_ORIENTATION_HORIZONTAL);
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (gesture), FALSE);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture), GTK_PHASE_CAPTURE);

  g_signal_connect_object (gesture,
                           "drag-begin",
                           G_CALLBACK (pnl_dock_pan_gesture_drag_begin),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (gesture,
                           "drag-end",
                           G_CALLBACK (pnl_dock_pan_gesture_drag_end),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (gesture,
                           "pan",
                           G_CALLBACK (pnl_dock_pan_gesture_pan),
                           self,
                           G_CONNECT_SWAPPED);

  priv->pan_gesture = GTK_GESTURE_PAN (gesture);
}

static void
pnl_dock_destroy (GtkWidget *widget)
{
  PnlDock *self = PNL_DOCK (widget);
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);

  g_clear_object (&priv->actions);
  g_clear_object (&priv->pan_gesture);

  GTK_WIDGET_CLASS (pnl_dock_parent_class)->destroy (widget);
}

static void
pnl_dock_get_child_property (GtkContainer *container,
                             GtkWidget    *widget,
                             guint         prop_id,
                             GValue       *value,
                             GParamSpec   *pspec)
{
  PnlDock *self = PNL_DOCK (container);
  PnlDockChild *child = pnl_dock_get_child (self, widget);

  switch (prop_id)
    {
    case CHILD_PROP_PRIORITY:
      g_value_set_int (value, child->priority);
      break;

    case CHILD_PROP_POSITION:
      g_value_set_enum (value, (GtkPositionType)child->type);
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
pnl_dock_set_child_property (GtkContainer *container,
                             GtkWidget    *widget,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PnlDock *self = PNL_DOCK (container);

  switch (prop_id)
    {
    case CHILD_PROP_PRIORITY:
      pnl_dock_set_child_priority (self, widget, g_value_get_int (value));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
pnl_dock_class_init (PnlDockClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  widget_class->destroy = pnl_dock_destroy;
  widget_class->get_preferred_height = pnl_dock_get_preferred_height;
  widget_class->get_preferred_width = pnl_dock_get_preferred_width;
  widget_class->size_allocate = pnl_dock_size_allocate;
  widget_class->realize = pnl_dock_realize;
  widget_class->unrealize = pnl_dock_unrealize;
  widget_class->map = pnl_dock_map;
  widget_class->unmap = pnl_dock_unmap;

  container_class->add = pnl_dock_add;
  container_class->forall = pnl_dock_forall;
  container_class->get_child_property = pnl_dock_get_child_property;
  container_class->remove = pnl_dock_remove;
  container_class->set_child_property = pnl_dock_set_child_property;

  child_properties [CHILD_PROP_POSITION] =
    g_param_spec_enum ("position",
                       "Position",
                       "The position of the dock edge",
                       GTK_TYPE_POSITION_TYPE,
                       GTK_POS_LEFT,
                       (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  child_properties [CHILD_PROP_PRIORITY] =
    g_param_spec_int ("priority",
                      "Priority",
                      "The priority of the dock edge",
                      G_MININT,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gtk_container_class_install_child_properties (container_class, LAST_CHILD_PROP, child_properties);
}

static void
pnl_dock_init_child (PnlDock          *self,
                     PnlDockChild     *child,
                     PnlDockChildType  type)
{
  GAction *action;

  g_assert (PNL_IS_DOCK (self));
  g_assert (child != NULL);
  g_assert (type >= PNL_DOCK_CHILD_LEFT);
  g_assert (type < LAST_PNL_DOCK_CHILD);

  child->type = type;
  child->priority = (int)type * 100;

  if (type == PNL_DOCK_CHILD_CENTER)
    return;

  child->widget = g_object_new (PNL_TYPE_DOCK_EDGE,
                                "edge", (GtkPositionType)type,
                                "reveal-child", FALSE,
                                "visible", TRUE,
                                NULL);
  gtk_widget_set_parent (g_object_ref_sink (child->widget), GTK_WIDGET (self));

  action = pnl_dock_get_action_for_type (self, type);
  g_object_bind_property_full (child->widget, "reveal-child",
                               action, "state",
                               G_BINDING_SYNC_CREATE,
                               map_boolean_to_variant,
                               NULL, NULL, NULL);
}

static void
pnl_dock_init (PnlDock *self)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  static const GActionEntry entries[] = {
    { "left-visible", NULL, NULL, "false", pnl_dock_visible_action },
    { "right-visible", NULL, NULL, "false", pnl_dock_visible_action },
    { "top-visible", NULL, NULL, "false", pnl_dock_visible_action },
    { "bottom-visible", NULL, NULL, "false", pnl_dock_visible_action },
  };

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  priv->actions = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (priv->actions),
                                   entries, G_N_ELEMENTS (entries),
                                   self);
  gtk_widget_insert_action_group (GTK_WIDGET (self), "dock", G_ACTION_GROUP (priv->actions));

  pnl_dock_init_child (self, &priv->children [0], PNL_DOCK_CHILD_LEFT);
  pnl_dock_init_child (self, &priv->children [1], PNL_DOCK_CHILD_RIGHT);
  pnl_dock_init_child (self, &priv->children [2], PNL_DOCK_CHILD_BOTTOM);
  pnl_dock_init_child (self, &priv->children [3], PNL_DOCK_CHILD_TOP);
  pnl_dock_init_child (self, &priv->children [4], PNL_DOCK_CHILD_CENTER);

  pnl_dock_create_pan_gesture (self);
}

GtkWidget *
pnl_dock_new (void)
{
  return g_object_new (PNL_TYPE_DOCK, NULL);
}

/**
 * pnl_dock_get_center_widget:
 * @self: A #PnlDock
 *
 * Gets the center widget for the docking region.
 *
 * Returns: (transfer none) (nullable): A #GtkWidget or %NULL.
 */
GtkWidget *
pnl_dock_get_center_widget (PnlDock *self)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  PnlDockChild *child;

  g_return_val_if_fail (PNL_IS_DOCK (self), NULL);

  child = &priv->children [PNL_DOCK_CHILD_CENTER];

  return child->widget;
}

void
pnl_dock_set_center_widget (PnlDock   *self,
                            GtkWidget *widget)
{
  PnlDockPrivate *priv = pnl_dock_get_instance_private (self);
  PnlDockChild *child;

  g_return_if_fail (PNL_IS_DOCK (self));
  g_return_if_fail (!widget || GTK_IS_WIDGET (widget));

  child = &priv->children [PNL_DOCK_CHILD_CENTER];

  if (widget != child->widget)
    {
      if (child->widget != NULL)
        {
          gtk_widget_unparent (child->widget);
          g_clear_object (&child->widget);
        }

      if (widget != NULL)
        {
          child->widget = g_object_ref_sink (widget);
          gtk_widget_set_parent (widget, GTK_WIDGET (self));
        }

      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
}

GtkWidget *
pnl_dock_get_top_edge (PnlDock *self)
{
  g_assert (PNL_IS_DOCK (self));

  return pnl_dock_get_child_typed (self, PNL_DOCK_CHILD_TOP)->widget;
}

GtkWidget *
pnl_dock_get_left_edge (PnlDock *self)
{
  g_assert (PNL_IS_DOCK (self));

  return pnl_dock_get_child_typed (self, PNL_DOCK_CHILD_LEFT)->widget;
}

GtkWidget *
pnl_dock_get_bottom_edge (PnlDock *self)
{
  g_assert (PNL_IS_DOCK (self));

  return pnl_dock_get_child_typed (self, PNL_DOCK_CHILD_BOTTOM)->widget;
}

GtkWidget *
pnl_dock_get_right_edge (PnlDock *self)
{
  g_assert (PNL_IS_DOCK (self));

  return pnl_dock_get_child_typed (self, PNL_DOCK_CHILD_RIGHT)->widget;
}
