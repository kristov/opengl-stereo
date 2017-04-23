#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef RASPBERRYPI
#include <GLES2/gl2.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */

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

static void show_shader_info_log(const char* filename, GLuint object) {
    GLint log_length;
    char *log;
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGetShaderInfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s: %s\n", filename, log);
    free(log);
}

static void show_program_info_log(GLuint object) {
    GLint log_length;
    char *log;
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGetProgramInfoLog(object, log_length, NULL, log);
    fprintf(stderr, "PROGRAM: %s\n", log);
    free(log);
}

static GLuint make_shader(GLenum type, const char* filename) {
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
        show_shader_info_log(filename, shader);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint make_program(char* filename, GLuint vertex_shader, GLuint fragment_shader) {
    GLint program_ok;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program [%s]:\n", filename);
        show_program_info_log(program);
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

    program_id = make_program(vert_file, vertex_shader_id, fragment_shader_id);
    if (program_id == 0)
        return 0;

    return program_id;
}
