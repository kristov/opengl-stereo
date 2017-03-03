
typedef struct opengl_stereo_camera {
    GLdouble left_frustum;
    GLdouble right_frustum;
    GLdouble bottom_frustum;
    GLdouble top_frustum;
    GLfloat model_translation;
} opengl_stereo_camera;

typedef struct opengl_stereo_buffer_store {
    GLuint left_buffer;
    GLuint right_buffer;
    GLuint rendered_texture_left;
    GLuint rendered_texture_right;
} opengl_stereo_buffer_store;

typedef struct opengl_stereo {
    double width;
    double height;
    float depthZ;
    double fovy;
    double aspect;
    double nearZ;
    double farZ;
    double screenZ;
    double IOD;
    GLuint screen_plane_vao;
    GLuint screen_shader_program_id;
    GLuint default_scene_shader_program_id;
    void (*draw_scene_function)(GLuint program_id);
    GLuint barrel_power_id;
    opengl_stereo_camera* left_camera;
    opengl_stereo_camera* right_camera;
    opengl_stereo_buffer_store* screen_buffers;
} opengl_stereo;

void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h);
void opengl_stereo_display(opengl_stereo* ostereo);
opengl_stereo* opengl_stereo_create(int width, int height);
