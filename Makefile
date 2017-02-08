test: test.c
	gcc -ggdb -lpthread -lm -lGL -lGLU -lglut -o test test.c

all: test
clean:
	rm test
