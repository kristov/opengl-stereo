test: src/test.c src/object3d.c
	gcc -Iinclude -ggdb -lpthread -lm -lGL -lGLU -lglut -o test src/object3d.c src/test.c

all: test
clean:
	rm test
