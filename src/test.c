#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <GL/glut.h>
#include "opengl_stereo.h"
#include "ogl_objecttree.h"

int rotate_delay = 30;
double theta = 0;

opengl_stereo* ostereo;
ogl_node* oglObjectRoot;

void createObjectTree(GLuint program) {
    //ogl_node* cube1 = ogl_node_cube_create(2.0f, 2.0f, 2.0f);
    ogl_node* cube2 = ogl_node_cube_create(2.0f, 2.0f, 2.0f);
    ogl_node_color(cube2, 0.3f, 0.1f, 0.9f);
    ogl_node* trans1 = ogl_node_trans_create(1.0f, 1.0f, 1.0f, cube2);
    ogl_node* rotate1 = ogl_node_rotate_create(20.0f, 20.0f, 20.0f, trans1);
    //ogl_node* root = ogl_node_rotate_create(10.0f, 10.0f, 10.0f, cube1);
    ogl_node* root = rotate1;
    ogl_node_realize(root, program);
    oglObjectRoot = root;
}

void draw_scene() {
    ogl_node_render(oglObjectRoot);
}

void rotate(int value) {
    theta += 1;
    glutPostRedisplay();
    glutTimerFunc(rotate_delay, rotate, 0);
}

GLvoid reshape(int w, int h) {
    opengl_stereo_reshape(ostereo, w, h);
}

GLvoid display(GLvoid) {
    opengl_stereo_display(ostereo);
    glutSwapBuffers();
}

void initWindowingSystem(int *argc, char **argv, int width, int height) {
    glutInit(argc, argv);
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("Stereo Test");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(rotate_delay, rotate, 0);
}

void init(int *argc, char **argv) {
    int width = 1024;
    int height = 768;

    initWindowingSystem(argc, argv, width, height);
    ostereo = opengl_stereo_create(width, height);
    ostereo->draw_scene_function = &draw_scene;
    createObjectTree(ostereo->default_scene_shader_program_id);
}

int main(int argc, char **argv) {
    init(&argc, argv);
    glutMainLoop();
    return 0;
}
