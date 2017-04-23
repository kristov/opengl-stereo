CC := gcc
OBJS := lib/opengl_stereo.o lib/ogl_objecttree.o lib/ogl_shader_loader.o lib/esm.o
CFLAGS := -Wall -Werror -ggdb

include desktop.mk

all: test

lib/opengl_stereo.o: src/opengl_stereo.c
	$(CC) $(CFLAGS) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/ogl_objecttree.o: src/ogl_objecttree.c
	$(CC) $(CFLAGS) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/ogl_shader_loader.o: src/ogl_shader_loader.c
	$(CC) $(CFLAGS) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/esm.o: src/esm.c
	$(CC) $(CFLAGS) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

test: $(MAINSRC) $(OBJS)
	$(CC) $(CFLAGS) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) -lm $(EXTERNAL) -o $@ $(OBJS) $<

clean:
	rm test lib/*
