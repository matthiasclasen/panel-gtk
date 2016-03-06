all: test-panel

PKGS = gtk+-3.0

OBJECTS = \
	pnl-dock-bin.o \
	pnl-dock-bin-edge.o \
	pnl-dock-bin-edge-child.o \
	pnl-dock-group.o \
	pnl-dock-item.o \
	pnl-dock.o \
	pnl-dock-manager.o \
	pnl-dock-overlay.o \
	pnl-dock-overlay-edge.o \
	pnl-dock-paned.o \
	pnl-dock-stack.o \
	pnl-dock-widget.o \
	pnl-dock-window.o \
	pnl-multi-paned.o \
	pnl-tab.o \
	pnl-tab-strip.o \
	pnl-util.o \
	$(NULL)

%.o: %.c %-private.h
	$(CC) -o $@ -fPIC -c -ggdb -O0 -fno-omit-frame-pointer $*.c $(shell pkg-config --cflags $(PKGS))

%.o: %.c %.h
	$(CC) -o $@ -fPIC -c -ggdb -O0 -fno-omit-frame-pointer $*.c $(shell pkg-config --cflags $(PKGS))

test-panel: test-panel.c $(OBJECTS)
	$(CC) -o $@ -ggdb -O0 -fno-omit-frame-pointer test-panel.c $(OBJECTS) $(shell pkg-config --cflags --libs $(PKGS))

clean:
	rm -f $(OBJECTS) test-panel
