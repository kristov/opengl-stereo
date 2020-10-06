#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define DTR 0.0174532925
#include "opengl_stereo.h"
#include "gl-matrix.h"

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

#define printOpenGLError() printGlError(__FILE__, __LINE__)

static void shader_err(GLuint obj) {
    int32_t length;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, (GLint*)&length);
    char* log = malloc(length);
    if (!log) {
        fprintf(stderr, "LOG MALLOC ERROR\n");
    }
    glGetShaderInfoLog(obj, length, NULL, log);
    fprintf(stderr, "shader_err: %s\n", log);
    free(log);
}

static void program_err(GLuint obj) {
    int32_t length;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, (GLint*)&length);
    char* log = malloc(length);
    if (!log) {
        fprintf(stderr, "LOG MALLOC ERROR\n");
    }
    glGetProgramInfoLog(obj, length, NULL, log);
    fprintf(stderr, "program_err: %s\n", log);
    free(log);
}

static uint32_t compile_shader(GLenum type, const char* src) {
    GLint length = strlen(src);
    GLint ok = 0;
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&src, &length);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        shader_err(shader);
        glDeleteShader(shader);
        return 0;
    }
    return (uint32_t)shader;
}

static uint32_t compile_program(const char* vert, const char* frag) {
    GLint ok = 0;
    GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert);
    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag);
    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        program_err(program);
        glDeleteProgram(program);
        return 0;
    }
    return (uint32_t)program;
}

static uint32_t ostereo_distortion_shader() {
    const char vert[] = "#version 120\n"
        "attribute vec3 b_vertex;\n"
        "attribute vec2 b_text;\n"
        "uniform mat4 m_projection;\n"
        "varying vec2 v_texcoord;\n"
        "void main() {\n"
        "    v_texcoord = b_text;\n"
        "    gl_Position = m_projection * vec4(b_vertex, 1);\n"
        "}\n";
    const char frag[] = "#version 120\n"
        "const float PI = 3.1415926535;\n"
        "uniform float barrel_power;\n"
        "uniform sampler2D tex0;\n"
        "varying vec2 v_texcoord;\n"
        "vec2 Distort(vec2 p) {\n"
        "    float theta = atan(p.y, p.x);\n"
        "    float radius = length(p);\n"
        "    radius = pow(radius, barrel_power);\n"
        "    p.x = radius * cos(theta);\n"
        "    p.y = radius * sin(theta);\n"
        "    return 0.5 * (p + 1.0);\n"
        "}\n"
        "void main() {\n"
        "    vec2 xy = 2.0 * v_texcoord - 1.0;\n"
        "    vec2 uv;\n"
        "    vec4 color;\n"
        "    float d = length(xy);\n"
        "    if (d < 1.15) {\n"
        "        uv = Distort(xy);\n"
        "        color = texture2D(tex0, uv);\n"
        "    }\n"
        "    else {\n"
        "        color = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "    gl_FragColor = color;\n"
        "}\n";
    return compile_program(vert, frag);
}

void opengl_stereo_store_screen_plane(opengl_stereo* ostereo) {
    float verts[] = {
        0.0f, 0.0f, 0.0f,
        2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f,
        2.0f, 2.0f, 0.0f
    };
    float uvs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    glGenBuffers(1, &ostereo->screen_plane_vdb);
    glBindBuffer(GL_ARRAY_BUFFER, ostereo->screen_plane_vdb);
    glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * sizeof(float), verts);
    glBufferSubData(GL_ARRAY_BUFFER, 12 * sizeof(float), 8 * sizeof(float), uvs);

    uint16_t indexes[] = {0, 1, 2, 1, 2, 3};
    glGenBuffers(1, &ostereo->screen_plane_idb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ostereo->screen_plane_idb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint16_t), indexes, GL_STATIC_DRAW);

    ostereo->screen_text_offset = 12 * sizeof(float); // can be just a separate buffer...
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

void opengl_stereo_create_render_texture(opengl_stereo* ostereo) {
    GLuint depthRenderBuffer;
    GLenum status;

    glGenFramebuffers(1, &ostereo->screen_buffer);
    glGenTextures(1, &ostereo->screen_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffer);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ostereo->width, ostereo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ostereo->width, ostereo->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ostereo->screen_texture, 0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FRAMEBUFFER incomplete: %d\n", (int)status);
    }

    return;
}

void opengl_stereo_camera_frustrum_L(opengl_stereo_camera* left_camera, double IODh, double top, double right, double frustumshift, double nearZ, double farZ) {
    left_camera->model_translation = IODh;
    mat4_frustum(
        left_camera->projection_matrix,
        -right + frustumshift,
        right + frustumshift,
        -top,
        top,
        nearZ,
        farZ
    );
}

void opengl_stereo_camera_frustrum_R(opengl_stereo_camera* right_camera, double IODh, double top, double right, double frustumshift, double nearZ, double farZ) {
    right_camera->model_translation = -IODh;
    mat4_frustum(
        right_camera->projection_matrix,
        -right - frustumshift,
        right - frustumshift,
        -top,
        top,
        nearZ,
        farZ
    );
}

void opengl_stereo_camera_frustrum_I(opengl_stereo_camera* camera, double IODh, double top, double right, double frustumshift, double nearZ, double farZ) {
    camera->model_translation = 0.0;
    mat4_frustum(
        camera->projection_matrix,
        -right,
        right,
        -top,
        top,
        nearZ,
        farZ
    );
}

void opengl_stereo_set_frustum(opengl_stereo* ostereo) {
    double IODh;
    double top;
    double right;
    double frustumshift;

    IODh = ostereo->IOD / 2;
    top = ostereo->nearZ * tan(DTR * ostereo->fovy / 2);
    right = ostereo->aspect * top;
    frustumshift = IODh * ostereo->nearZ / ostereo->screenZ;

    opengl_stereo_camera_frustrum_L(&ostereo->left_camera, IODh, top, right, frustumshift, ostereo->nearZ, ostereo->farZ);
    opengl_stereo_camera_frustrum_R(&ostereo->right_camera, IODh, top, right, frustumshift, ostereo->nearZ, ostereo->farZ);
    opengl_stereo_camera_frustrum_I(&ostereo->skybox_camera, IODh, top, right, frustumshift, ostereo->nearZ, ostereo->farZ);
}

void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h) {
    if (h == 0) {
        h = 1;
    }
    if (ostereo->mode == OSTEREO_MODE_STEREO) {
        ostereo->width = w / 2;
    }
    else {
        ostereo->width = w;
    }
    ostereo->height = h;
    ostereo->aspect = ostereo->width / ostereo->height;
    opengl_stereo_set_frustum(ostereo);
}

void opengl_stereo_render_left_scene(opengl_stereo* ostereo) {
    GLint tex0;
    GLuint m_projection;

    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width, ostereo->height);

    mat4_copy(ostereo->projection_matrix, ostereo->left_camera.projection_matrix);
    mat4_identity(ostereo->view_matrix);
    mat4_multiply(ostereo->view_matrix, ostereo->hmd_matrix);
    mat4_translatef(ostereo->view_matrix, ostereo->left_camera.model_translation, 0.0, ostereo->depthZ);

    ostereo->draw_scene_callback(ostereo, ostereo->draw_scene_callback_data);

    glUseProgram(ostereo->screen_shader_program_id);

    ostereo->barrel_power_id = glGetUniformLocation(ostereo->screen_shader_program_id, "barrel_power");
    glUniform1f(ostereo->barrel_power_id, 1.1f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    mat4_identity(ostereo->screen_matrix);
    mat4_translatef(ostereo->screen_matrix, -1.0 + ostereo->texture_shift, -1.0, 0.0);

    m_projection = glGetUniformLocation(ostereo->screen_shader_program_id, "m_projection");
    glUniformMatrix4fv(m_projection, 1, GL_FALSE, ostereo->screen_matrix);

    glViewport(0, 0, ostereo->width, ostereo->height);

    tex0 = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex0, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_texture);

    opengl_stereo_render_screen_plane(ostereo);
}

void opengl_stereo_render_right_scene(opengl_stereo* ostereo) {
    GLint texLoc;
    GLuint m_projection;

    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width, ostereo->height);

    mat4_copy(ostereo->projection_matrix, ostereo->right_camera.projection_matrix);
    mat4_identity(ostereo->view_matrix);
    mat4_multiply(ostereo->view_matrix, ostereo->hmd_matrix);
    mat4_translatef(ostereo->view_matrix, ostereo->right_camera.model_translation, 0.0, ostereo->depthZ);

    ostereo->draw_scene_callback(ostereo, ostereo->draw_scene_callback_data);

    glUseProgram(ostereo->screen_shader_program_id);

    ostereo->barrel_power_id = glGetUniformLocation(ostereo->screen_shader_program_id, "barrel_power");
    glUniform1f(ostereo->barrel_power_id, 1.1f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    mat4_identity(ostereo->screen_matrix);
    mat4_translatef(ostereo->screen_matrix, -1.0 - ostereo->texture_shift, -1.0, 0.0);

    m_projection = glGetUniformLocation(ostereo->screen_shader_program_id, "m_projection");
    glUniformMatrix4fv(m_projection, 1, GL_FALSE, ostereo->screen_matrix);

    glViewport(ostereo->width, 0, ostereo->width, ostereo->height);

    texLoc = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_texture);

    opengl_stereo_render_screen_plane(ostereo);
}

void opengl_stereo_render_mono_scene(opengl_stereo* ostereo) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width, ostereo->height);

    mat4_identity(ostereo->view_matrix);
    mat4_multiply(ostereo->view_matrix, ostereo->hmd_matrix);
    mat4_translatef(ostereo->view_matrix, ostereo->skybox_camera.model_translation, 0.0, ostereo->depthZ);

    ostereo->draw_scene_callback(ostereo, ostereo->draw_scene_callback_data);
}

void opengl_stereo_render_stereo_scene(opengl_stereo* ostereo) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    opengl_stereo_render_left_scene(ostereo);
    opengl_stereo_render_right_scene(ostereo);
}

/*
    display(): (one buffer)
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
    if (!ostereo->draw_scene_callback) {
        fprintf(stderr, "opengl_stereo_ERROR: draw_scene_callback not attached\n");
        return;
    }
    ostereo->scene_renderer(ostereo);
}

void opengl_stereo_draw_scene_callback(opengl_stereo* ostereo, ostereo_draw_scene_callback_t callback, void* callback_data) {
    if (!callback) {
        fprintf(stderr, "opengl_stereo_ERROR: draw_scene_callback is NULL\n");
        return;
    }

    ostereo->draw_scene_callback = callback;
    ostereo->draw_scene_callback_data = callback_data;
}

void initGL(opengl_stereo* ostereo) {
    glEnable(GL_DEPTH_TEST);
    //glMatrixMode(GL_PROJECTION);
    //glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_CUBE_MAP);
    //glLoadIdentity();
}

void opengl_stereo_camera_init(opengl_stereo_camera* camera) {
    memset(camera, 0, sizeof(opengl_stereo_camera));
    camera->model_translation = 0.0f;
}

void opengl_stereo_init_system(opengl_stereo* ostereo) {
    initGL(ostereo);

    ostereo->aspect = ostereo->width / ostereo->height;
    double quart_screen_width = ostereo->physical_width / 4;
    double half_iod = ostereo->IOD / 2;

    ostereo->texture_shift = quart_screen_width - half_iod;

    //fprintf(stderr, "           Physical width (dm): %0.2f\n", ostereo->physical_width);
    //fprintf(stderr, "              Correction ratio: %0.2f\n", correction_ratio);
    //fprintf(stderr, "(half_screen_width - half_iod): %0.2f\n", (half_screen_width - half_iod));
    //fprintf(stderr, "                 Texture shift: %0.2f\n", ostereo->texture_shift);

    ostereo->screen_shader_program_id = ostereo_distortion_shader();
    opengl_stereo_set_frustum(ostereo);
    opengl_stereo_create_render_texture(ostereo);
    opengl_stereo_store_screen_plane(ostereo);
}

void opengl_stereo_load_defaults(opengl_stereo* ostereo) {
    ostereo->IOD = 0.6;
    ostereo->depthZ = 0.0;
    ostereo->fovy = 45;
    ostereo->nearZ = 0.1;
    ostereo->farZ = 300.0;
    ostereo->screenZ = 100.0;
}

void opengl_stereo_init(opengl_stereo* ostereo, int width, int height, double physical_width, opengl_stereo_mode_t mode) {
    memset(ostereo, 0, sizeof(opengl_stereo));
    ostereo->mode = mode;
    if (mode == OSTEREO_MODE_STEREO) {
        ostereo->width = width / 2;
        ostereo->scene_renderer = opengl_stereo_render_stereo_scene;
    }
    else if (mode == OSTEREO_MODE_MONO) {
        ostereo->width = width;
        ostereo->scene_renderer = opengl_stereo_render_mono_scene;
    }
    else {
        fprintf(stderr, "INVALID MODE! (OSTEREO_MODE_STEREO or OSTEREO_MODE_MONO)\n");
        return;
    }
    ostereo->height = height;
    ostereo->physical_width = physical_width;
    opengl_stereo_camera_init(&ostereo->left_camera);
    opengl_stereo_camera_init(&ostereo->right_camera);
    opengl_stereo_camera_init(&ostereo->skybox_camera);
    mat4_identity(ostereo->screen_matrix);
    mat4_identity(ostereo->view_matrix);
    mat4_identity(ostereo->hmd_matrix);
    opengl_stereo_load_defaults(ostereo);
    opengl_stereo_init_system(ostereo);
}
