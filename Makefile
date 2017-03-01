test: src/test.c src/object3d.c lib/ogl_objecttree.o
	gcc -Iinclude -ggdb -lpthread -lm -lGL -lGLU -lglut -o test lib/ogl_objecttree.o src/object3d.c src/test.c

lib/ogl_objecttree.o: src/ogl_objecttree.c
	gcc -Iinclude -ggdb -c -o lib/ogl_objecttree.o src/ogl_objecttree.c

all: test

clean:
	rm test
	rm lib/*
