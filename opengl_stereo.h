#ifndef OPENGL_STEREO_H
#define OPENGL_STEREO_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GL/glut.h>

typedef struct opengl_stereo_camera {
    float projection_matrix[16];
    float model_translation;
} opengl_stereo_camera;

typedef struct opengl_stereo opengl_stereo;

typedef void (*ostereo_draw_scene_callback_t)(opengl_stereo* ostereo, void* data);

typedef enum opengl_stereo_mode {
    OSTEREO_MODE_STEREO = 0x00,
    OSTEREO_MODE_MONO = 0x01
} opengl_stereo_mode_t;

typedef struct opengl_stereo {
    opengl_stereo_mode_t mode;
    double width;
    double height;
    double depthZ;
    double fovy;
    double aspect;
    double nearZ;
    double farZ;
    double screenZ;
    double IOD;
    double physical_width;
    double texture_shift;
    GLuint screen_plane_vdb;
    GLuint screen_plane_idb;
    GLuint screen_text_offset;
    float screen_matrix[16];
    GLuint screen_shader_program_id;
    GLuint color_shader_id;
    GLuint texture_shader_id;
    GLuint cubemap_shader_id;
    float view_matrix[16];
    float hmd_matrix[16];
    float projection_matrix[16];
    ostereo_draw_scene_callback_t draw_scene_callback;
    void (*scene_renderer)(opengl_stereo* ostereo);
    void* draw_scene_callback_data;
    GLuint barrel_power_id;
    opengl_stereo_camera left_camera;
    opengl_stereo_camera right_camera;
    opengl_stereo_camera skybox_camera;
    GLuint screen_buffer;
    GLuint screen_texture;
} opengl_stereo;

void opengl_stereo_draw_scene_callback(opengl_stereo* ostereo, ostereo_draw_scene_callback_t callback, void* callback_data);
void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h);
void opengl_stereo_display(opengl_stereo* ostereo);
void opengl_stereo_init(opengl_stereo* ostereo, int width, int height, double physical_width, opengl_stereo_mode_t mode);

#endif
