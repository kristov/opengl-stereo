CC := gcc
CFLAGS := -Wall -Werror -DGL_SIMPLE_PLAT_GLUT
LINK := -lGL -lGLU -lglut

GLUT :=
GLUT += glut_square

all: $(GLUT)

debug: CFLAGS += -ggdb
debug: $(GLUT)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(shell pkg-bee --cflags --libs gl-matrix gl-simple opengl-stereo) $(LINK)

clean:
	rm -f $(GLUT)
