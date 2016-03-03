# Panel-Gtk Design

## PnlDock

This is the base interface for dock containers.
Dock containers are GtkContainer subclasses that are dock layouts.
They have direct children that are PnlDockGroups.

## PnlDockBin (a PnlDock)

This is a GtkWidget that is similar to bin in that there is a primary child.
The other children are the panel edges.
Top, left, right, bottom.

## PnlDockOverlay (a PnlDock)

This is a PnlDock that has a primary child (the GtkBin child).
It also has "hovering" panels at the edges that can animate in over the child.
The hovering panels show a small "tab-like" label.

## PnlDockWindow (a PnlDock)

This is a PnlDock that is a floating window.
It is used to tear out dock widgets from the normal toplevel.

## PnlDockWidget

The PnlDockWidget is the widget that is subclassed by applications.
This is what gets moved between docks.

## PnlDockGroup

This interface is implemented by groups.
It is important that groups can contain other groups.
This allows having a paned with stacks as paned children.

## PnlDockStack (a PnlDockGroup)

This allows adding PnlDockWidgets and shows a notebook like switcher.

## PnlDockPaned (a PnlDockGroup)

This allows adding PnlDockWidgets and shows them in a "multi-paned".
The paned sections can be resized.
