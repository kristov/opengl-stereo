#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#define DTR 0.0174532925
#include "opengl_stereo.h"
#include "ogl_shader_loader.h"

#define PRINT_GL_ERRORS

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define printOpenGLError() printGlError(__FILE__, __LINE__)

void discardGlError() {
    glGetError();
}

int printGlError(char *file, int line) {
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    if (glErr != GL_NO_ERROR) {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
    }
    return retCode;
}

/*
Eye separation is typically kept at 1/30th of the convergence distance and objects
closer than half the convergence distance are avoided in the scene.
*/

void opengl_stereo_load_screen_shader(opengl_stereo* ostereo) {
    ostereo->screen_shader_program_id = ogl_shader_loader_load("vert_screen.glsl", "frag_screen.glsl");
    ostereo->default_scene_shader_program_id = ogl_shader_loader_load("vert_buffer.glsl", "frag_buffer.glsl");
}

void opengl_stereo_store_screen_plane(opengl_stereo* ostereo) {
    GLuint glVertexB, glTextB;
    GLuint vertex_index_buffer;
    GLuint vertex_data_buffer;
    GLfloat* verts;
    GLfloat* uvs;
    GLuint* indicies;
    int voff;
    GLuint buff_size, vert_size, text_size, indi_size;

    vert_size = sizeof(GLfloat) * 12;
    text_size = sizeof(GLfloat) * 8;
    indi_size = sizeof(GLuint) * 6;
    buff_size = vert_size + text_size;

    verts = malloc(vert_size);
    uvs = malloc(text_size);
    indicies = malloc(indi_size);

    voff = 0;

    verts[voff + 0] = 0.0f;
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 2.0f;
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 0.0f;
    verts[voff + 1] = 2.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 2.0f;
    verts[voff + 1] = 2.0f;
    verts[voff + 2] = 0.0f;

    voff = 0;
    indicies[voff + 0] = 0;
    indicies[voff + 1] = 1;
    indicies[voff + 2] = 2;
    indicies[voff + 3] = 1;
    indicies[voff + 4] = 2;
    indicies[voff + 5] = 3;

    voff = 0;
    uvs[voff + 0] = 0.0f;
    uvs[voff + 1] = 0.0f;
    voff += 2;
    uvs[voff + 0] = 1.0f;
    uvs[voff + 1] = 0.0f;
    voff += 2;
    uvs[voff + 0] = 0.0f;
    uvs[voff + 1] = 1.0f;
    voff += 2;
    uvs[voff + 0] = 1.0f;
    uvs[voff + 1] = 1.0f;

    glGenVertexArrays(1, &ostereo->screen_plane_vao);
    glBindVertexArray(ostereo->screen_plane_vao);

    glGenBuffers(1, &vertex_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, buff_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vert_size, verts);
    glBufferSubData(GL_ARRAY_BUFFER, vert_size, text_size, uvs);

    glGenBuffers(1, &vertex_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indi_size, indicies, GL_STATIC_DRAW);

    glVertexB = glGetAttribLocation(ostereo->screen_shader_program_id, "glVertexB" );
    glVertexAttribPointer(glVertexB, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glVertexB);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glTextB = glGetAttribLocation(ostereo->screen_shader_program_id, "glTextB" );
    glVertexAttribPointer(glTextB, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vert_size));
    glEnableVertexAttribArray(glTextB);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    free(verts);
    free(uvs);
    free(indicies);

    glBindVertexArray(0);
}

void opengl_stereo_render_screen_plane(opengl_stereo* ostereo) {
    glBindVertexArray(ostereo->screen_plane_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

void opengl_stereo_create_render_textures(opengl_stereo* ostereo) {
    GLuint depthRenderBufferLeft;
    GLuint depthRenderBufferRight;
    float borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };

    glGenFramebuffers(1, &ostereo->screen_buffers->left_buffer);
    glGenTextures(1, &ostereo->screen_buffers->rendered_texture_left);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_left);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ostereo->width / 2, ostereo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBufferLeft);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferLeft);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ostereo->width / 2, ostereo->height);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->left_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_left, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferLeft);


    glGenFramebuffers(1, &ostereo->screen_buffers->right_buffer);
    glGenTextures(1, &ostereo->screen_buffers->rendered_texture_right);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_right);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ostereo->width / 2, ostereo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBufferRight);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferRight);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ostereo->width / 2, ostereo->height);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->right_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_right, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferRight);

    return;
}

void opengl_stereo_set_frustum(opengl_stereo* ostereo) {
    double top = ostereo->nearZ * tan(DTR * ostereo->fovy / 2);

    double right = ostereo->aspect * top;
    double frustumshift = (ostereo->IOD / 2) * ostereo->nearZ / ostereo->screenZ;

    ostereo->left_camera->top_frustum = top;
    ostereo->left_camera->bottom_frustum = -top;
    ostereo->left_camera->left_frustum = -right + frustumshift;
    ostereo->left_camera->right_frustum = right + frustumshift;
    ostereo->left_camera->model_translation = ostereo->IOD / 2;

    ostereo->right_camera->top_frustum = top;
    ostereo->right_camera->bottom_frustum = -top;
    ostereo->right_camera->left_frustum = -right - frustumshift;
    ostereo->right_camera->right_frustum = right - frustumshift;
    ostereo->right_camera->model_translation = -ostereo->IOD / 2;
}

void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h) {
    if (h == 0) {
        h = 1;
    }
    ostereo->width = w;
    ostereo->height = h;
    ostereo->aspect = ( (double)w / 2 ) / (double)h;
    opengl_stereo_set_frustum(ostereo);
}

void opengl_stereo_render_scene_to_left_buffer(opengl_stereo* ostereo) {
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->left_buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    glFrustum(ostereo->left_camera->left_frustum, ostereo->left_camera->right_frustum,
              ostereo->left_camera->bottom_frustum, ostereo->left_camera->top_frustum,
              ostereo->nearZ, ostereo->farZ);
    glTranslatef(ostereo->left_camera->model_translation, 0.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, ostereo->depthZ);
        ostereo->draw_scene_function();
    }
    glPopMatrix();
}

void opengl_stereo_render_scene_to_right_buffer(opengl_stereo* ostereo) {
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->right_buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    glFrustum(ostereo->right_camera->left_frustum, ostereo->right_camera->right_frustum,
              ostereo->right_camera->bottom_frustum, ostereo->right_camera->top_frustum,
              ostereo->nearZ, ostereo->farZ);
    glTranslatef(ostereo->right_camera->model_translation, 0.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, ostereo->depthZ);
        ostereo->draw_scene_function();
    }
    glPopMatrix();
}

void opengl_stereo_render_scene_to_buffers(opengl_stereo* ostereo) {
    glUseProgram(ostereo->default_scene_shader_program_id);

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    opengl_stereo_render_scene_to_left_buffer(ostereo);
    opengl_stereo_render_scene_to_right_buffer(ostereo);
}

void opengl_stereo_render_left_buffer_to_window(opengl_stereo* ostereo) {
    GLint texLoc;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, ostereo->width / 2, ostereo->height);
    glFrustum(-1, 1, -1, 1, 0.0, 40.0);
    discardGlError();

    texLoc = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_left);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, 0.0);
        opengl_stereo_render_screen_plane(ostereo);
    }
    glPopMatrix();

#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
}

void opengl_stereo_render_right_buffer_to_window(opengl_stereo* ostereo) {
    GLint texLoc;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(ostereo->width / 2, 0, ostereo->width / 2, ostereo->height);
    glFrustum(-1, 1, -1, 1, 0.0, 40.0);
    discardGlError();

    texLoc = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_right);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, 0.0);
        opengl_stereo_render_screen_plane(ostereo);
    }
    glPopMatrix();

#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
}

void opengl_stereo_render_buffers_to_window(opengl_stereo* ostereo) {
    glUseProgram(ostereo->screen_shader_program_id);

    ostereo->barrel_power_id = glGetUniformLocation(ostereo->screen_shader_program_id, "barrel_power");
    glUniform1f(ostereo->barrel_power_id, 1.1f);

#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glClearColor(0.0f,0.8f,0.8f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    opengl_stereo_render_left_buffer_to_window(ostereo);
    opengl_stereo_render_right_buffer_to_window(ostereo);
}

/*
    display():
        opengl_stereo_render_scene_to_buffers():
            glUseProgram(buffer) <-- Rendering a 3d scene
            opengl_stereo_render_scene_to_left_buffer()
            opengl_stereo_render_scene_to_right_buffer()
        opengl_stereo_render_buffers_to_window():
            glUseProgram(screen) <-- Rendering a texture
            opengl_stereo_render_left_buffer_to_window()
            opengl_stereo_render_right_buffer_to_window()
*/
void opengl_stereo_display(opengl_stereo* ostereo) {
    if (ostereo->draw_scene_function == NULL) {
        fprintf(stderr, "opengl_stereo_ERROR: draw_scene_function not attached\n");
        return;
    }
    opengl_stereo_render_scene_to_buffers(ostereo);
    opengl_stereo_render_buffers_to_window(ostereo);
}

void initGL(opengl_stereo* ostereo) {
    GLfloat lightpos[] = { 0.5f, 1.0f, 1.0f, 0.0f };
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
}

void opengl_stereo_init_screen_info(opengl_stereo* ostereo) {
    ostereo->aspect = ostereo->width / ostereo->height;
    ostereo->depthZ = -10.0;       // depth of the object drawing
    ostereo->fovy = 45;            // field of view in y-axis
    ostereo->nearZ = 3.0;          // near clipping plane
    ostereo->farZ = 30.0;          // far clipping plane
    ostereo->screenZ = 10.0;       // screen projection plane
    ostereo->IOD = 0.5;            // intraocular distance
}

void opengl_stereo_init_system(opengl_stereo* ostereo) {
    initGL(ostereo);
    opengl_stereo_load_screen_shader(ostereo);
    opengl_stereo_set_frustum(ostereo);
    opengl_stereo_create_render_textures(ostereo);
    opengl_stereo_store_screen_plane(ostereo);
}

void opengl_stereo_init(opengl_stereo* ostereo) {
    opengl_stereo_init_screen_info(ostereo);
    opengl_stereo_init_system(ostereo);
}

opengl_stereo_camera* opengl_stereo_camera_new() {
    opengl_stereo_camera* camera = malloc(sizeof(opengl_stereo_camera));
    camera->left_frustum = 0;
    camera->right_frustum = 0;
    camera->bottom_frustum = 0;
    camera->top_frustum = 0;
    camera->model_translation = 0.0f;
    return camera;
}

opengl_stereo_buffer_store* opengl_stereo_buffer_store_new() {
    opengl_stereo_buffer_store* buffer = malloc(sizeof(opengl_stereo_buffer_store));
    buffer->left_buffer = 0;
    buffer->right_buffer = 0;
    buffer->rendered_texture_left = 0;
    buffer->rendered_texture_right = 0;
    return buffer;
}

opengl_stereo* opengl_stereo_new() {
    opengl_stereo* ostereo = malloc(sizeof(opengl_stereo));
    ostereo->width = 0;
    ostereo->height = 0;
    ostereo->left_camera = NULL;
    ostereo->right_camera = NULL;
    ostereo->screen_buffers = NULL;
    ostereo->screen_plane_vao = 0;
    ostereo->draw_scene_function = NULL;
    return ostereo;
}

opengl_stereo* opengl_stereo_create(int width, int height) {
    opengl_stereo* ostereo = opengl_stereo_new();
    ostereo->width = width;
    ostereo->height = height;
    ostereo->left_camera = opengl_stereo_camera_new();
    ostereo->right_camera = opengl_stereo_camera_new();
    ostereo->screen_buffers = opengl_stereo_buffer_store_new();
    opengl_stereo_init(ostereo);
    return ostereo;
}
