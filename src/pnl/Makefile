all: test-panel

PKGS = gtk+-3.0

OBJECTS = \
	pnl-dock-bin.o \
	pnl-dock-group.o \
	pnl-dock.o \
	pnl-dock-edge.o \
	pnl-dock-edge-child.o \
	pnl-dock-manager.o \
	pnl-dock-overlay.o \
	pnl-dock-paned.o \
	pnl-dock-stack.o \
	pnl-dock-widget.o \
	pnl-dock-window.o \
	pnl-multi-paned.o \
	$(NULL)

%.o: %.c %.h
	$(CC) -o $@ -fPIC -c -ggdb -O0 -fno-omit-frame-pointer $*.c $(shell pkg-config --cflags $(PKGS))

test-panel: test-panel.c $(OBJECTS)
	$(CC) -o $@ -ggdb -O0 -fno-omit-frame-pointer test-panel.c $(OBJECTS) $(shell pkg-config --cflags --libs $(PKGS))

clean:
	rm -f $(OBJECTS) test-panel
