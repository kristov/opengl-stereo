#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#endif /* RASPBERRYPI */
#define DTR 0.0174532925
#include "opengl_stereo.h"
#include "ogl_shader_loader.h"
#include "esm.h"

#ifdef RASPBERRYPI
static char vert_screen[] = "shaders/100/vert_screen.glsl";
static char frag_screen[] = "shaders/100/frag_screen.glsl";
static char vert_buffer[] = "shaders/100/vert_buffer.glsl";
static char frag_buffer[] = "shaders/100/frag_buffer.glsl";
#else /* not RASPBERRYPI */
static char vert_screen[] = "shaders/120/vert_screen.glsl";
static char frag_screen[] = "shaders/120/frag_screen.glsl";
static char vert_buffer[] = "shaders/120/vert_buffer.glsl";
static char frag_buffer[] = "shaders/120/frag_buffer.glsl";
#endif /* RASPBERRYPI */

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

int printGlError(char *file, int line) {
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    switch (glErr) {
        case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_VALUE:
            printf("GL_INVALID_VALUE in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_STACK_OVERFLOW:
            printf("GL_STACK_OVERFLOW in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_STACK_UNDERFLOW:
            printf("GL_STACK_UNDERFLOW in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_OUT_OF_MEMORY:
            printf("GL_OUT_OF_MEMORY in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
    }
    return retCode;
}

/*
Eye separation is typically kept at 1/30th of the convergence distance and objects
closer than half the convergence distance are avoided in the scene.
*/

void opengl_stereo_load_screen_shader(opengl_stereo* ostereo) {
    ostereo->screen_shader_program_id = ogl_shader_loader_load(vert_screen, frag_screen);
    ostereo->default_scene_shader_program_id = ogl_shader_loader_load(vert_buffer, frag_buffer);
}

void opengl_stereo_store_screen_plane(opengl_stereo* ostereo) {
    GLfloat* verts;
    GLfloat* uvs;
    GLushort* indicies;
    int voff;
    GLuint buff_size, vert_size, text_size, indi_size;

    vert_size = sizeof(GLfloat) * 12;
    text_size = sizeof(GLfloat) * 8;
    indi_size = sizeof(GLuint) * 6;
    buff_size = vert_size + text_size;

    ostereo->screen_text_offset = vert_size;

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

    glGenBuffers(1, &ostereo->screen_plane_vdb);
    glBindBuffer(GL_ARRAY_BUFFER, ostereo->screen_plane_vdb);
    glBufferData(GL_ARRAY_BUFFER, buff_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vert_size, verts);
    glBufferSubData(GL_ARRAY_BUFFER, vert_size, text_size, uvs);

    glGenBuffers(1, &ostereo->screen_plane_idb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ostereo->screen_plane_idb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indi_size, indicies, GL_STATIC_DRAW);

    free(verts);
    free(uvs);
    free(indicies);
}

void opengl_stereo_render_screen_plane(opengl_stereo* ostereo) {
    GLuint b_vertex, b_text;

    glBindBuffer(GL_ARRAY_BUFFER, ostereo->screen_plane_vdb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ostereo->screen_plane_idb);

    b_vertex = glGetAttribLocation(ostereo->screen_shader_program_id, "b_vertex" );
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    b_text = glGetAttribLocation(ostereo->screen_shader_program_id, "b_text" );
    glVertexAttribPointer(b_text, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(ostereo->screen_text_offset));
    glEnableVertexAttribArray(b_text);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

void opengl_stereo_create_render_textures(opengl_stereo* ostereo) {
    GLuint depthRenderBufferLeft;
    GLuint depthRenderBufferRight;
    GLenum status;

    glGenFramebuffers(1, &ostereo->screen_buffers->left_buffer);
    glGenTextures(1, &ostereo->screen_buffers->rendered_texture_left);
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->left_buffer);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_left);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ostereo->width / 2, ostereo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBufferLeft);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferLeft);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ostereo->width / 2, ostereo->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferLeft);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_left, 0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FRAMEBUFFER [left] incomplete: %d\n", (int)status);
    }

    glGenFramebuffers(1, &ostereo->screen_buffers->right_buffer);
    glGenTextures(1, &ostereo->screen_buffers->rendered_texture_right);
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->right_buffer);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_right);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ostereo->width / 2, ostereo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBufferRight);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferRight);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ostereo->width / 2, ostereo->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferRight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_right, 0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FRAMEBUFFER [right] incomplete: %d\n", (int)status);
    }

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
    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    esmLoadIdentity(ostereo->view_matrix);
    esmLoadIdentity(ostereo->model_matrix);

    esmFrustumf(ostereo->projection_matrix, ostereo->left_camera->left_frustum, ostereo->left_camera->right_frustum,
                ostereo->left_camera->bottom_frustum, ostereo->left_camera->top_frustum,
                ostereo->nearZ, ostereo->farZ);
    esmTranslatef(ostereo->view_matrix, ostereo->left_camera->model_translation, 0.0, ostereo->depthZ);
    ostereo->draw_scene_function();
}

void opengl_stereo_render_scene_to_right_buffer(opengl_stereo* ostereo) {
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->right_buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    esmLoadIdentity(ostereo->view_matrix);
    esmLoadIdentity(ostereo->model_matrix);

    esmFrustumf(ostereo->projection_matrix, ostereo->right_camera->left_frustum, ostereo->right_camera->right_frustum,
                ostereo->right_camera->bottom_frustum, ostereo->right_camera->top_frustum,
                ostereo->nearZ, ostereo->farZ);
    esmTranslatef(ostereo->view_matrix, ostereo->right_camera->model_translation, 0.0, ostereo->depthZ);
    ostereo->draw_scene_function();
}

void opengl_stereo_render_scene_to_buffers(opengl_stereo* ostereo) {
    glUseProgram(ostereo->default_scene_shader_program_id);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    opengl_stereo_render_scene_to_left_buffer(ostereo);
    opengl_stereo_render_scene_to_right_buffer(ostereo);
}

void opengl_stereo_render_left_buffer_to_window(opengl_stereo* ostereo) {
    GLint tex0;
    GLuint m_projection;

    esmLoadIdentity(ostereo->screen_matrix);
    esmTranslatef(ostereo->screen_matrix, -1.0 + ostereo->texture_shift, -1.0, 0.0);

    m_projection = glGetUniformLocation(ostereo->screen_shader_program_id, "m_projection");
    glUniformMatrix4fv(m_projection, 1, GL_FALSE, ostereo->screen_matrix);

    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    tex0 = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glUniform1i(tex0, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_left);

    opengl_stereo_render_screen_plane(ostereo);
}

void opengl_stereo_render_right_buffer_to_window(opengl_stereo* ostereo) {
    GLint texLoc;
    GLuint m_projection;

    esmLoadIdentity(ostereo->screen_matrix);
    esmTranslatef(ostereo->screen_matrix, -1.0 - ostereo->texture_shift, -1.0, 0.0);

    m_projection = glGetUniformLocation(ostereo->screen_shader_program_id, "m_projection");
    glUniformMatrix4fv(m_projection, 1, GL_FALSE, ostereo->screen_matrix);

    glViewport(ostereo->width / 2, 0, ostereo->width / 2, ostereo->height);

    texLoc = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture_right);

    opengl_stereo_render_screen_plane(ostereo);
}

void opengl_stereo_render_buffers_to_window(opengl_stereo* ostereo) {

    glUseProgram(ostereo->screen_shader_program_id);

    ostereo->barrel_power_id = glGetUniformLocation(ostereo->screen_shader_program_id, "barrel_power");
    glUniform1f(ostereo->barrel_power_id, 1.1f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

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

    alt_display(): (one buffer)
        opengl_stereo_render_left_scene():
            glUseProgram(buffer) <-- Rendering a 3d scene
            opengl_stereo_render_left_scene_to_buffer()
            glUseProgram(screen) <-- Rendering a texture
            opengl_stereo_render_buffer_to_window()
        opengl_stereo_render_right_scene():
            glUseProgram(buffer) <-- Rendering a 3d scene
            opengl_stereo_render_right_scene_to_buffer()
            glUseProgram(screen) <-- Rendering a texture
            opengl_stereo_render_buffer_to_window()
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
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

double opengl_stereo_get_config_value(opengl_stereo* ostereo, char* name) {
    double value;
    if (config_lookup_float(ostereo->config, name, &value) == CONFIG_TRUE) {
        return value;
    }
    return 0.0;
}

void opengl_stereo_set_config_value_int(opengl_stereo* ostereo, char* name, double value) {
    config_setting_t* setting = config_lookup(ostereo->config, name);
    if (!setting)
        return;
    config_setting_set_float(setting, value);
}

void opengl_stereo_set_config_value(opengl_stereo* ostereo, char* name, double value) {
    int needs_save = 0;

    printf("%s: %0.4f\n", name, value);
    opengl_stereo_set_config_value_int(ostereo, name, value);

    if (!strcmp(name, "IOD")) {
        ostereo->IOD = value;
        needs_save = 1;
    }

    if (!strcmp(name, "depthZ")) {
        ostereo->depthZ = value;
        needs_save = 1;
    }

    if (!strcmp(name, "fovy")) {
        ostereo->fovy = value;
        needs_save = 1;
    }

    if (!strcmp(name, "nearZ")) {
        ostereo->nearZ = value;
        needs_save = 1;
    }

    if (!strcmp(name, "farZ")) {
        ostereo->farZ = value;
        needs_save = 1;
    }

    if (!strcmp(name, "screenZ")) {
        ostereo->screenZ = value;
        needs_save = 1;
    }

    if (needs_save) {
        char* filename = malloc(sizeof(char) * 200);
        int written = snprintf(filename, 200, "/home/%s/.openglstereorc", getenv("USER"));
        if (written == 200) {
            fprintf(stderr, "path to .openglstereorc is too long!\n");
            return;
        }
        config_write_file(ostereo->config, filename);
        opengl_stereo_set_frustum(ostereo);
    }
}

int opengl_stereo_load_config_value(config_t* config, config_setting_t* root, char* name, double* location, double def) {
    double value;
    if (config_lookup_float(config, name, &value) == CONFIG_TRUE) {
        *location = value;
        return 0;
    }
    else {
        *location = def;
        config_setting_t* new = config_setting_add(root, name, CONFIG_TYPE_FLOAT);
        config_setting_set_float(new, def);
        return 1;
    }
}

void opengl_stereo_load_defaults(opengl_stereo* ostereo) {
    int needs_save = 0;

    config_setting_t* root = config_root_setting(ostereo->config);

    if (opengl_stereo_load_config_value(ostereo->config, root, "IOD", &ostereo->IOD, 0.5))
        needs_save = 1;

    if (opengl_stereo_load_config_value(ostereo->config, root, "depthZ", &ostereo->depthZ, -10.0))
        needs_save = 1;

    if (opengl_stereo_load_config_value(ostereo->config, root, "fovy", &ostereo->fovy, 45))
        needs_save = 1;

    if (opengl_stereo_load_config_value(ostereo->config, root, "nearZ", &ostereo->nearZ, 3.0))
        needs_save = 1;

    if (opengl_stereo_load_config_value(ostereo->config, root, "farZ", &ostereo->farZ, 30.0))
        needs_save = 1;

    if (opengl_stereo_load_config_value(ostereo->config, root, "screenZ", &ostereo->screenZ, 10.0))
        needs_save = 1;

    if (needs_save) {
        char* filename = malloc(sizeof(char) * 200);
        int written = snprintf(filename, 200, "/home/%s/.openglstereorc", getenv("USER"));
        if (written == 200) {
            fprintf(stderr, "path to .openglstereorc is too long!\n");
            return;
        }
        config_write_file(ostereo->config, filename);
    }
}

void opengl_stereo_load_config(opengl_stereo* ostereo) {
    config_t* config;
    config = malloc(sizeof(config_t));
    config_init(config);

    char* filename = malloc(sizeof(char) * 200);
    int written = snprintf(filename, 200, "/home/%s/.openglstereorc", getenv("USER"));
    if (written == 200) {
        fprintf(stderr, "path to .openglstereorc is too long %d!\n", written);
        return;
    }

    config_read_file(config, filename);
    ostereo->config = config;

    opengl_stereo_load_defaults(ostereo);
}

void opengl_stereo_init_system(opengl_stereo* ostereo) {
    initGL(ostereo);

    ostereo->aspect = ( ostereo->width / 2 ) / ostereo->height;
    double half_screen_width = ostereo->physical_width / 2;
    double half_iod = ostereo->IOD / 2;
    double correction_ratio = 2 / ostereo->physical_width;
    ostereo->texture_shift = (half_screen_width - half_iod) * correction_ratio;

    //fprintf(stderr, "           Physical width (dm): %0.2f\n", ostereo->physical_width);
    //fprintf(stderr, "              Correction ratio: %0.2f\n", correction_ratio);
    //fprintf(stderr, "(half_screen_width - half_iod): %0.2f\n", (half_screen_width - half_iod));
    //fprintf(stderr, "                 Texture shift: %0.2f\n", ostereo->texture_shift);

    opengl_stereo_load_screen_shader(ostereo);
    opengl_stereo_set_frustum(ostereo);
    opengl_stereo_create_render_textures(ostereo);
    opengl_stereo_store_screen_plane(ostereo);

}

void opengl_stereo_init(opengl_stereo* ostereo) {
    opengl_stereo_load_config(ostereo);
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
    ostereo->draw_scene_function = NULL;
    ostereo->screen_matrix = NULL;
    ostereo->model_matrix = NULL;
    ostereo->view_matrix = NULL;
    ostereo->projection_matrix = NULL;
    return ostereo;
}

opengl_stereo* opengl_stereo_create(int width, int height, double physical_width) {
    opengl_stereo* ostereo = opengl_stereo_new();
    ostereo->width = width;
    ostereo->height = height;
    ostereo->physical_width = physical_width;
    ostereo->left_camera = opengl_stereo_camera_new();
    ostereo->right_camera = opengl_stereo_camera_new();
    ostereo->screen_buffers = opengl_stereo_buffer_store_new();
    ostereo->screen_matrix = esmCreate();
    ostereo->model_matrix = esmCreate();
    ostereo->view_matrix = esmCreate();
    ostereo->projection_matrix = esmCreate();
    opengl_stereo_init(ostereo);
    return ostereo;
}
