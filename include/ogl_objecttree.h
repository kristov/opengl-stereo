#include <stdlib.h>
#include <GL/gl.h>

typedef struct ogl_node ogl_node;

typedef enum ogl_node_type {
    OGL_OBJECT_MESH,
    OGL_OBJECT_TRANS,
    OGL_OBJECT_ROTATE
} ogl_node_type;

typedef struct ogl_object_mesh {
    GLuint vertex_array_object_id;
    int visible;
    int nr_verticies;
    int nr_indicies;
    GLfloat* verts;
    GLfloat* norms;
    GLuint* indicies;
    GLfloat* colors;
    GLfloat* uvs;
} ogl_object_mesh;

typedef struct ogl_object_trans {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    ogl_node* node;
} ogl_object_trans;

typedef struct ogl_object_rotate {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    ogl_node* node;
} ogl_object_rotate;

typedef struct ogl_node {
    union {
        ogl_object_mesh* object_mesh;
        ogl_object_trans* object_trans;
        ogl_object_rotate* object_rotate;
    } object_pointer;
    ogl_node_type type;
} ogl_node;

ogl_node* ogl_node_create(ogl_node_type type);

ogl_object_mesh* ogl_object_mesh_create();
ogl_object_trans* ogl_object_trans_create();
ogl_object_rotate* ogl_object_rotate_create();

void ogl_object_mesh_color(ogl_object_mesh* mesh, GLfloat r, GLfloat g, GLfloat b);
void ogl_node_color(ogl_node* node, GLfloat r, GLfloat g, GLfloat b);

ogl_node* ogl_node_cube_create(GLfloat x, GLfloat y, GLfloat z);
ogl_node* ogl_node_trans_create(GLfloat x, GLfloat y, GLfloat z, ogl_node* child_node);
ogl_node* ogl_node_rotate_create(GLfloat x, GLfloat y, GLfloat z, ogl_node* child_node);

void ogl_node_realize(ogl_node* node, GLuint program);
void ogl_node_render(ogl_node* node);
void ogl_object_cube_generate_geometry(ogl_object_mesh* mesh, GLfloat x, GLfloat y, GLfloat z);
