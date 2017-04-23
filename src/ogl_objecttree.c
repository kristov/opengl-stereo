#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */
#include <stdlib.h>
#include <stdio.h>
#include "ogl_objecttree.h"
#include "esm.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define PRINT_GL_ERRORS

#define printOpenGLError() OprintGlError(__FILE__, __LINE__)

int OprintGlError(char *file, int line) {
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
        //case GL_CONTEXT_LOST:
        //    printf("GL_CONTEXT_LOST in file %s @ line %d: %d\n", file, line, glErr);
        //    retCode = 1;
        //break;
        //case GL_TABLE_TOO_LARGE:
        //    printf("GL_TABLE_TOO_LARGE in file %s @ line %d: %d\n", file, line, glErr);
        //    retCode = 1;
        //break;
    }
    return retCode;
}

ogl_node* ogl_node_create(ogl_node_type type) {
    ogl_node* node = malloc(sizeof(ogl_node));
    node->type = type;
    return node;
}

ogl_object_mesh* ogl_object_mesh_create() {
    ogl_object_mesh* mesh = malloc(sizeof(ogl_object_mesh));
    mesh->shader_program_id = 0;
    mesh->vertex_data_buffer_id = 0;
    mesh->index_data_buffer_id = 0;
    mesh->norm_offset = 0;
    mesh->color_offset = 0;
    mesh->text_offset = 0;
    mesh->nr_verticies = 0;
    mesh->nr_indicies = 0;
    mesh->visible = 0;
    mesh->verts = NULL;
    mesh->indicies = NULL;
    mesh->colors = NULL;
    mesh->uvs = NULL;
    return mesh;
}

ogl_object_trans* ogl_object_trans_create() {
    ogl_object_trans* trans = malloc(sizeof(ogl_object_trans));
    trans->x = 0;
    trans->y = 0;
    trans->z = 0;
    trans->node = NULL;
    return trans;
}

ogl_object_rotate* ogl_object_rotate_create() {
    ogl_object_rotate* rotate = malloc(sizeof(ogl_object_rotate));
    rotate->x = 0;
    rotate->y = 0;
    rotate->z = 0;
    rotate->node = NULL;
    return rotate;
}

ogl_node* ogl_node_cube_create(GLfloat x, GLfloat y, GLfloat z) {
    ogl_node* node = ogl_node_create(OGL_OBJECT_MESH);
    ogl_object_mesh* mesh = ogl_object_mesh_create();
    ogl_object_cube_generate_geometry(mesh, x, y, z);
    node->object_pointer.object_mesh = mesh;
    return node;
}

void ogl_object_mesh_color(ogl_object_mesh* mesh, GLfloat r, GLfloat g, GLfloat b) {
    int nr_verts, i, coff;
    GLfloat* colors;

    coff = 0;
    nr_verts = mesh->nr_verticies;
    colors = mesh->colors;

    for (i = 0; i < nr_verts; i++) {
        colors[coff + 0] = r;
        colors[coff + 1] = g;
        colors[coff + 2] = b;
        colors[coff + 3] = 0.1f;
        coff += 4;
    }
}

void ogl_node_color(ogl_node* node, GLfloat r, GLfloat g, GLfloat b) {
    if (node->type != OGL_OBJECT_MESH) {
        return;
    }
    ogl_object_mesh_color(node->object_pointer.object_mesh, r, g, b);
    return;
}

ogl_node* ogl_node_trans_create(GLfloat x, GLfloat y, GLfloat z, ogl_node* child_node) {
    ogl_node* node = ogl_node_create(OGL_OBJECT_TRANS);
    ogl_object_trans* trans = ogl_object_trans_create();
    trans->x = x;
    trans->y = y;
    trans->z = z;
    trans->node = child_node;
    node->object_pointer.object_trans = trans;
    return node;
}

ogl_node* ogl_node_rotate_create(GLfloat x, GLfloat y, GLfloat z, ogl_node* child_node) {
    ogl_node* node = ogl_node_create(OGL_OBJECT_ROTATE);
    ogl_object_rotate* rotate = ogl_object_rotate_create();
    rotate->x = x;
    rotate->y = y;
    rotate->z = z;
    rotate->node = child_node;
    node->object_pointer.object_rotate = rotate;
    return node;
}

void ogl_object_mesh_realize(ogl_object_mesh* mesh, GLuint program) {
    GLuint vert_size, norm_size, colo_size, indi_size, buff_size;

    vert_size = mesh->nr_verticies * 3 * sizeof(GLfloat);
    norm_size = vert_size;
    colo_size = mesh->nr_verticies * 4 * sizeof(GLfloat);
    indi_size = mesh->nr_indicies * sizeof(GLuint);
    buff_size = vert_size + norm_size + colo_size;

    mesh->norm_offset = vert_size;
    mesh->color_offset = vert_size + norm_size;
    mesh->shader_program_id = program;

    glGenBuffers(1, &mesh->vertex_data_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_data_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, buff_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vert_size, mesh->verts);
    glBufferSubData(GL_ARRAY_BUFFER, mesh->norm_offset, norm_size, mesh->norms);
    glBufferSubData(GL_ARRAY_BUFFER, mesh->color_offset, colo_size, mesh->colors);

    glGenBuffers(1, &mesh->index_data_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_data_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indi_size, mesh->indicies, GL_STATIC_DRAW);
}

void ogl_object_trans_realize(ogl_object_trans* trans, GLuint program) {
    if (trans->node != NULL) {
        ogl_node_realize(trans->node, program);
    }
}

void ogl_object_rotate_realize(ogl_object_rotate* rotate, GLuint program) {
    if (rotate->node != NULL) {
        ogl_node_realize(rotate->node, program);
    }
}

void ogl_node_realize(ogl_node* node, GLuint program) {
    switch(node->type) {
        case OGL_OBJECT_MESH:
            ogl_object_mesh_realize(node->object_pointer.object_mesh, program);
            break;
        case OGL_OBJECT_TRANS:
            ogl_object_trans_realize(node->object_pointer.object_trans, program);
            break;
        case OGL_OBJECT_ROTATE:
            ogl_object_rotate_realize(node->object_pointer.object_rotate, program);
            break;
    }
}

void ogl_object_rotate_render(ogl_object_rotate* rotate, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    if (rotate->x != 0) {
        esmRotatef(model_matrix, rotate->x, 1, 0, 0);
    }
    if (rotate->y != 0) {
        esmRotatef(model_matrix, rotate->y, 0, 1, 0);
    }
    if (rotate->z != 0) {
        esmRotatef(model_matrix, rotate->z, 0, 0, 1);
    }
    if (rotate->node != NULL) {
        ogl_node_render(rotate->node, projection_matrix, view_matrix, model_matrix);
    }
    if (rotate->z != 0) {
        esmRotatef(model_matrix, 0 - rotate->z, 0, 0, 1);
    }
    if (rotate->y != 0) {
        esmRotatef(model_matrix, 0 - rotate->y, 0, 1, 0);
    }
    if (rotate->x != 0) {
        esmRotatef(model_matrix, 0 - rotate->x, 1, 0, 0);
    }
}

void ogl_object_trans_render(ogl_object_trans* trans, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    esmTranslatef(model_matrix, trans->x, trans->y, trans->z);
    if (trans->node != NULL) {
        ogl_node_render(trans->node, projection_matrix, view_matrix, model_matrix);
    }
    esmTranslatef(model_matrix, 0 - trans->x, 0 - trans->y, 0 - trans->z);
}

void ogl_object_mesh_render(ogl_object_mesh* mesh, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    GLuint b_vertex, b_normal, b_color, m_mvp, m_mv, m_normal;
    GLfloat* normal_matrix;
    GLfloat* mvp_matrix;
    GLfloat* mv_matrix;

    if (mesh->visible == 0) {
        return;
    }

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);
    normal_matrix = esmNormalMatrixFromProjection(mv_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_data_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_data_buffer_id);

    b_vertex = glGetAttribLocation(mesh->shader_program_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    b_normal = glGetAttribLocation(mesh->shader_program_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->norm_offset));
    glEnableVertexAttribArray(b_normal);

    b_color = glGetAttribLocation(mesh->shader_program_id, "b_color");
    glVertexAttribPointer(b_color, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->color_offset));
    glEnableVertexAttribArray(b_color);

    m_mvp = glGetUniformLocation(mesh->shader_program_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);
    glEnableVertexAttribArray(m_mvp);

    m_mv = glGetUniformLocation(mesh->shader_program_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);
    glEnableVertexAttribArray(m_mv);

    m_normal = glGetUniformLocation(mesh->shader_program_id, "m_normal");
    glUniformMatrix4fv(m_normal, 1, GL_FALSE, model_matrix);
    glEnableVertexAttribArray(m_normal);

    glDrawElements(GL_TRIANGLES, mesh->nr_indicies, GL_UNSIGNED_SHORT, NULL);

    esmDestroy(normal_matrix);
    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);
}

void ogl_node_render(ogl_node* node, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    switch(node->type) {
        case OGL_OBJECT_MESH:
            ogl_object_mesh_render(node->object_pointer.object_mesh, projection_matrix, view_matrix, model_matrix);
            break;
        case OGL_OBJECT_TRANS:
            ogl_object_trans_render(node->object_pointer.object_trans, projection_matrix, view_matrix, model_matrix);
            break;
        case OGL_OBJECT_ROTATE:
            ogl_object_rotate_render(node->object_pointer.object_rotate, projection_matrix, view_matrix, model_matrix);
            break;
    }
}

void ogl_object_rotate_change(ogl_object_rotate* rotate, GLfloat x, GLfloat y, GLfloat z) {
    if (x != 0.0) { rotate->x += x; }
    if (y != 0.0) { rotate->y += y; }
    if (z != 0.0) { rotate->z += z; }
}

void ogl_node_rotate_change(ogl_node* node, GLfloat x, GLfloat y, GLfloat z) {
    if (node->type == OGL_OBJECT_ROTATE) {
        ogl_object_rotate_change(node->object_pointer.object_rotate, x, y, z);
    }
}

void ogl_object_cube_generate_geometry(ogl_object_mesh* mesh, GLfloat x, GLfloat y, GLfloat z) {
    int nr_vert_floats, nr_color_floats;
    GLfloat* verts;
    GLfloat* norms;
    GLushort* indicies;
    GLfloat* colors;
    int ioff, voff, coff;
    GLfloat r, g, b;

    mesh->nr_verticies = 4 * 6;
    nr_vert_floats = 3 * mesh->nr_verticies;
    verts = malloc(sizeof(GLfloat) * nr_vert_floats);
    norms = malloc(sizeof(GLfloat) * nr_vert_floats);

    mesh->nr_indicies = 6 * 6;
    indicies = malloc(sizeof(GLushort) * mesh->nr_indicies);

    nr_color_floats = 4 * mesh->nr_verticies;
    colors = malloc(sizeof(GLfloat) * nr_color_floats);

    r = 0.4f;
    g = 0.0f;
    b = 0.4f;

    ioff = 0;
    voff = 0;
    coff = 0;

    // front
    verts[voff + 0] = 0.0f; // 0
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = x; // 1
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 0.0f; // 2
    verts[voff + 1] = y;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = x; // 3
    verts[voff + 1] = y;
    verts[voff + 2] = 0.0f;
    voff += 3;

    // left
    verts[voff + 0] = 0.0f; // 4
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 0.0f; // 5
    verts[voff + 1] = y;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = 0.0f; // 6
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = 0.0f; // 7
    verts[voff + 1] = y;
    verts[voff + 2] = 0.0f;
    voff += 3;

    // right
    verts[voff + 0] = x; // 8
    verts[voff + 1] = y;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = x; // 9
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = x; // 10
    verts[voff + 1] = y;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = x; // 11
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;

    // top
    verts[voff + 0] = x; // 12
    verts[voff + 1] = y;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 0.0f; // 13
    verts[voff + 1] = y;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = x; // 14
    verts[voff + 1] = y;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = 0.0f; // 15
    verts[voff + 1] = y;
    verts[voff + 2] = 0.0f;
    voff += 3;

    // bottom
    verts[voff + 0] = 0.0f; // 16
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = x; // 17
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = 0.0f; // 18
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = x; // 19
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;

    // back
    verts[voff + 0] = 0.0f; // 20
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = x; // 21
    verts[voff + 1] = y;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = 0.0f; // 22
    verts[voff + 1] = y;
    verts[voff + 2] = z;
    voff += 3;
    verts[voff + 0] = x; // 23
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = z;
    voff += 3;

    voff = 0;
    // front
    norms[voff + 0] = 0.0f; // 0
    norms[voff + 1] = 1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 1
    norms[voff + 1] = 1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 2
    norms[voff + 1] = 1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 3
    norms[voff + 1] = 1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;

    // left
    norms[voff + 0] = -1.0f; // 4
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = -1.0f; // 5
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = -1.0f; // 6
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = -1.0f; // 7
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;

    // right
    norms[voff + 0] = 1.0f; // 8
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 1.0f; // 9
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 1.0f; // 10
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 1.0f; // 11
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;

    // top
    norms[voff + 0] = 0.0f; // 12
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 1.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 13
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 1.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 14
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 1.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 15
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = 1.0f;
    voff += 3;

    // bottom
    norms[voff + 0] = 0.0f; // 16
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = -1.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 17
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = -1.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 18
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = -1.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 19
    norms[voff + 1] = 0.0f;
    norms[voff + 2] = -1.0f;
    voff += 3;

    // back
    norms[voff + 0] = 0.0f; // 20
    norms[voff + 1] = -1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 21
    norms[voff + 1] = -1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 22
    norms[voff + 1] = -1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;
    norms[voff + 0] = 0.0f; // 23
    norms[voff + 1] = -1.0f;
    norms[voff + 2] = 0.0f;
    voff += 3;

    // front
    colors[coff + 0] = r; // 0
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 1
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 2
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 3
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // left
    colors[coff + 0] = r; // 4
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 5
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 6
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 7
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // right
    colors[coff + 0] = r; // 8
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 9
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 10
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 11
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // top
    colors[coff + 0] = r; // 12
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 13
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 14
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 15
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // bottom
    colors[coff + 0] = r; // 16
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 17
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 18
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 19
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // back
    colors[coff + 0] = r; // 20
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 21
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 22
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = r; // 23
    colors[coff + 1] = g;
    colors[coff + 2] = b;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // front
    indicies[ioff + 0] = 0;
    indicies[ioff + 1] = 1;
    indicies[ioff + 2] = 2;
    indicies[ioff + 3] = 1;
    indicies[ioff + 4] = 2;
    indicies[ioff + 5] = 3;
    ioff += 6;

    // left
    indicies[ioff + 0] = 4;
    indicies[ioff + 1] = 5;
    indicies[ioff + 2] = 6;
    indicies[ioff + 3] = 4;
    indicies[ioff + 4] = 5;
    indicies[ioff + 5] = 7;
    ioff += 6;

    // right
    indicies[ioff + 0] = 8;
    indicies[ioff + 1] = 9;
    indicies[ioff + 2] = 10;
    indicies[ioff + 3] = 8;
    indicies[ioff + 4] = 9;
    indicies[ioff + 5] = 11;
    ioff += 6;

    // top
    indicies[ioff + 0] = 12;
    indicies[ioff + 1] = 13;
    indicies[ioff + 2] = 14;
    indicies[ioff + 3] = 12;
    indicies[ioff + 4] = 13;
    indicies[ioff + 5] = 15;
    ioff += 6;

    // bottom
    indicies[ioff + 0] = 16;
    indicies[ioff + 1] = 17;
    indicies[ioff + 2] = 18;
    indicies[ioff + 3] = 16;
    indicies[ioff + 4] = 17;
    indicies[ioff + 5] = 19;
    ioff += 6;

    // back
    indicies[ioff + 0] = 20;
    indicies[ioff + 1] = 21;
    indicies[ioff + 2] = 22;
    indicies[ioff + 3] = 20;
    indicies[ioff + 4] = 21;
    indicies[ioff + 5] = 23;
    ioff += 6;

    mesh->verts = verts;
    mesh->norms = norms;
    mesh->indicies = indicies;
    mesh->colors = colors;
    mesh->visible = 1;

    return;
}
