#include <stdio.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "bcm_host.h"
#include "opengl_stereo.h"
#include "ogl_objecttree.h"

opengl_stereo* ostereo;
ogl_node* oglObjectRoot;
ogl_node* rotate;

void createObjectTree(GLuint program) {
    ogl_node* cube1 = ogl_node_cube_create(2.0f, 2.0f, 2.0f);
    ogl_node_color(cube1, 0.3f, 0.1f, 0.9f);
    ogl_node* trans1 = ogl_node_trans_create(-1.0f, -1.0f, -1.0f, cube1);
    ogl_node* rotate1 = ogl_node_rotate_create(2.0f, 0.0f, 0.0f, trans1);
    rotate = rotate1;
    //ogl_node* rotate1 = ogl_node_rotate_create(20.0f, 10.0f, 15.0f, trans1);
    ogl_node* trans2 = ogl_node_trans_create(0.0f, 0.0f, 0.0f, rotate1);
    ogl_node* root = trans2;
    ogl_node_realize(root, program);
    oglObjectRoot = root;
}

void drawScene() {
    ogl_node_rotate_change(rotate, 0.0, 0.01, 0.0);
    ogl_node_render(oglObjectRoot, ostereo->projection_matrix, ostereo->view_matrix, ostereo->model_matrix);
}

int main(int argc, char** argv) {
    int minor_v, major_v;
    uint32_t width, height;
    int32_t success;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
    EGL_DISPMANX_WINDOW_T dispman_window;

    EGLBoolean result;
    EGLDisplay display;
    EGLint num_configs;
    EGLConfig config;
    EGLSurface surface;
    EGLNativeWindowType window;
    EGLContext context;

    EGLint attr_context[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE
    };

    EGLint attr_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, EGL_DONT_CARE,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };

    bcm_host_init();
    success = graphics_get_display_size( 0, &width, &height);

    if (success < 0) {
        fprintf(stderr, "graphics_get_display_size failed (%d)\n", success);
        return 1;
    }

    fprintf(stderr, "size: [%dx%d]\n", width, height);

    vc_dispmanx_rect_set(&dst_rect, 0, 0, width, height);
    vc_dispmanx_rect_set(&src_rect, 0, 0, width, height);

    dispman_display = vc_dispmanx_display_open(0);
    dispman_update = vc_dispmanx_update_start(0);
    dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, 0, 0, 0);

    dispman_window.element = dispman_element;
    dispman_window.width = width;
    dispman_window.height = height;

    vc_dispmanx_update_submit_sync(dispman_update);

    window = &dispman_window;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (display == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay failed\n");
        return 1;
    }

    result = eglInitialize(display, &major_v, &minor_v);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglInitialize failed\n");
        return 1;
    }

    fprintf(stderr, "EGL version %d.%d\n", major_v, minor_v);

    result = eglGetConfigs(display, NULL, 0, &num_configs);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglGetConfigs failed (getting the number of configs)\n");
        return 1;
    }

    result = eglChooseConfig(display, attr_list, &config, 1, &num_configs);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglChooseConfig failed\n");
        return 1;
    }

    surface = eglCreateWindowSurface(display, config, window, NULL);

    if (surface == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface failed\n");
        return 1;
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attr_context);

    if (context == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed\n");
        return 1;
    }

    result = eglMakeCurrent(display, surface, surface, context);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        return 1;
    }

    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    fprintf(stderr, "extensions: %s\n", extensions);

    const GLubyte* shaderv = glGetString(GL_SHADING_LANGUAGE_VERSION);
    fprintf(stderr, "shaderv: %s\n", shaderv);

    ostereo = opengl_stereo_create((int)width, (int)height);
    ostereo->draw_scene_function = &drawScene;
    createObjectTree(ostereo->default_scene_shader_program_id);

    while (GL_TRUE) {
        opengl_stereo_display(ostereo);
        eglSwapBuffers(display, surface);
    }

    return 0;
}



/*
void dumpEGLConfigs(EGLDisplay display) {
    int i;
    EGLint num_configs;
    EGLBoolean result;
    EGLConfig* configs;
    EGLint r, g, b, depth, native;

    result = eglGetConfigs(display, NULL, 0, &num_configs);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglGetConfigs failed (getting the number of configs)\n");
        return;
    }

    fprintf(stderr, "EGL Number of configs: %d\n", num_configs);

    configs = malloc(sizeof(EGLConfig) * num_configs);

    result = eglGetConfigs(display, configs, num_configs, &num_configs);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglGetConfigs failed (fetching all configs)\n");
        return;
    }

    for (i = 0; i < num_configs; i++) {
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &r);
        eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &g);
        eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &b);
        eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(display, configs[i], EGL_NATIVE_RENDERABLE, &native);
        fprintf(stderr, "%d: [%d, %d, %d] depth: %d, native: %d\n", i, r, g, b, depth, native);
    }

    free(configs);
}
*/

