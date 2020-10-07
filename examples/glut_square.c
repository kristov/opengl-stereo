#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <gl-simple.h>
#include <gl-matrix.h>
#include <opengl-stereo.h>

#define PI    3.141593f
#define TWOPI 6.283185f

opengl_stereo ostereo;

float model[16];
float view[16];
float projection[16];
float mv[16];
float mvp[16];

struct gl_simple_rcs render;
struct gl_simple_m matrix;
struct gl_simple_err err;

float verts[] = {
    -1.0f, -1.0f,  0.0f,
     1.0f, -1.0f,  0.0f,
    -1.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  0.0f
};

float norms[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f
};

uint16_t indexes[] = {0, 1, 2, 1, 2, 3};

void error_print(void* data, char* message, uint16_t len) {
    printf("ERROR: %s\n", message);
}

GLvoid reshape(int w, int h) {
    //
}

void motion(int x, int y) {
    float xpct = (float)x / (float)1152.0f;
    float ypct = (float)y / (float)648.0f;
    float xangle = TWOPI * xpct;
    float yangle = (PI * ypct) - (PI / 2);
    mat4_identity(ostereo.hmd_matrix);
    mat4_translatef(ostereo.hmd_matrix, 0, 0, -5.0f);
    mat4_rotateX(ostereo.hmd_matrix, yangle);
    mat4_rotateY(ostereo.hmd_matrix, xangle);
    gl_simple_matrix_update(&matrix);
}

void draw_scene(opengl_stereo* ostereo, void* data) {
    gl_simple_draw_rcs(&render, &matrix);
}

GLvoid display(GLvoid) {
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    opengl_stereo_display(&ostereo);
    glutSwapBuffers();
}

void do_timer(int timer_event) {
    glutPostRedisplay();
    glutTimerFunc(10, do_timer, 1);
}

void initWindowingSystem(int *argc, char **argv, int width, int height) {
    glutInit(argc, argv);
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("GL Simple GLUT Demo");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(motion);
    glutTimerFunc(10, do_timer, 1);
}

void init_gl_simple(uint16_t width, uint16_t height) {
    double physical_width = 1.347;
    opengl_stereo_init(&ostereo, width, height, physical_width, OSTEREO_MODE_STEREO);
    opengl_stereo_draw_scene_callback(&ostereo, draw_scene, NULL);

    render.vertex_id = gl_simple_load_float_buffer(verts, 12);
    render.normal_id = gl_simple_load_float_buffer(norms, 12);
    render.index_id = gl_simple_load_integer_buffer(indexes, 6);
    render.nr_indexes = 6;
    render.r = 0.0f;
    render.g = 0.7f;
    render.b = 0.7f;
    render.a = 1.0f;

    err.data = NULL;
    err.callback = error_print;
    render.shader_id = gl_simple_shader_rcs(&err);

    render.err = &err;

    matrix.m = &model[0];
    matrix.v = &ostereo.view_matrix[0];
    matrix.p = &ostereo.projection_matrix[0];
    matrix.mv = &mv[0];
    matrix.mvp = &mvp[0];

    mat4_identity(matrix.m);
    mat4_translatef(matrix.m, 0, 0, -5.0f);
    gl_simple_matrix_update(&matrix);
}

void init(int *argc, char **argv) {
    uint16_t width = 1152;
    uint16_t height = 648;
    initWindowingSystem(argc, argv, width, height);
    init_gl_simple(width, height);
}

int32_t main(int argc, char **argv) {
    init(&argc, argv);
    glutMainLoop();
    return 0;
}
