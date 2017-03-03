test: src/test.c lib/opengl_stereo.o lib/ogl_objecttree.o lib/ogl_shader_loader.o
	gcc -Wall -Werror -Iinclude -ggdb -lpthread -lm -lGL -lGLU -lglut -o test lib/opengl_stereo.o lib/ogl_objecttree.o lib/ogl_shader_loader.o src/test.c

lib/opengl_stereo.o: src/opengl_stereo.c
	gcc -Wall -Werror -Iinclude -ggdb -c -o lib/opengl_stereo.o src/opengl_stereo.c

lib/ogl_objecttree.o: src/ogl_objecttree.c
	gcc -Wall -Werror -Iinclude -ggdb -c -o lib/ogl_objecttree.o src/ogl_objecttree.c

lib/ogl_shader_loader.o: src/ogl_shader_loader.c
	gcc -Wall -Werror -Iinclude -ggdb -c -o lib/ogl_shader_loader.o src/ogl_shader_loader.c

all: test

clean:
	rm test
	rm lib/*
