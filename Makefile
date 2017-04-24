CC := gcc
OBJS := lib/opengl_stereo.o lib/ogl_objecttree.o lib/ogl_shader_loader.o lib/esm.o
CFLAGS := -Wall -Werror -ggdb

x11_glut : EXTERNAL := -lpthread -lGL -lGLU -lglut
x11_glut : INCLUDEDIRS :=
x11_glut : LINKDIRS :=
x11_glut : PREPROC :=

rpi_egl : EXTERNAL := -lbcm_host -lEGL -lGLESv2
rpi_egl : INCLUDEDIRS := -I/opt/vc/include
rpi_egl : LINKDIRS := -L/opt/vc/lib
rpi_egl : PREPROC := -DRASPBERRYPI

all: x11_glut

lib/opengl_stereo.o: src/opengl_stereo.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/ogl_objecttree.o: src/ogl_objecttree.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/ogl_shader_loader.o: src/ogl_shader_loader.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/esm.o: src/esm.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

x11_glut: src/desktop_main.c $(OBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) -lm $(EXTERNAL) -o $@ $(OBJS) $<

rpi_egl: src/raspberrypi_main.c $(OBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) -lm $(EXTERNAL) -o $@ $(OBJS) $<

clean:
	rm -f lib/*
	rm -f x11_glut rpi_egl
