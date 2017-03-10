#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

GLchar *file_contents(const char *filename, GLint *length) {
    char *buffer = 0;
    FILE *f = fopen(filename, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        *length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(*length);
        if (buffer) {
            fread(buffer, 1, *length, f);
        }
        fclose(f);
    }

    if (buffer) {
        return buffer;
    }

    return NULL;
}

static void show_info_log(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog) {
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

static GLuint make_shader(GLenum type, const char *filename) {
    GLint length;
    GLchar *source = file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source) {
        fprintf(stderr, "Failed to load file %s\n", filename);
        return 0;
    }

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader) {
    GLint program_ok;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

GLuint ogl_shader_loader_load(char *vert_file, char *frag_file) {
    GLuint vertex_shader_id, fragment_shader_id, program_id;

    vertex_shader_id = make_shader(GL_VERTEX_SHADER, vert_file);
    if (vertex_shader_id == 0)
        return 0;

    fragment_shader_id = make_shader(GL_FRAGMENT_SHADER, frag_file);
    if (vertex_shader_id == 0)
        return 0;

    program_id = make_program(vertex_shader_id, fragment_shader_id);
    if (program_id == 0)
        return 0;

    return program_id;
}
