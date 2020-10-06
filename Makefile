CC := gcc
CFLAGS := -Wall -Werror
INCD := -I. $(shell pkg-bee --cflags gl-matrix)

#EXTCOM := -lm -lconfig

OBJECTS :=
OBJECTS += opengl_stereo.o

HEADERS := $(OBJECTS:.o=.h)

all: opengl-stereo.a

opengl-stereo.h:
	cat $(HEADERS) > $@

%.o: %.c %.h opengl-stereo.h
	$(CC) $(CFLAGS) $(INCD) -c -o $@ $<

opengl-stereo.a: $(OBJECTS)
	ar -crs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) opengl-stereo.h opengl-stereo.a
