#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include "ogl_objecttree.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

ogl_node* ogl_node_create(ogl_node_type type) {
    ogl_node* node = malloc(sizeof(ogl_node));
    node->type = type;
    return node;
}

ogl_object_mesh* ogl_object_mesh_create() {
    ogl_object_mesh* mesh = malloc(sizeof(ogl_object_mesh));
    mesh->vertex_array_object_id = 0;
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
    GLuint glVertexB, glNormalB, glColorB, glTextB;
    GLuint vertex_index_buffer;
    GLuint vertex_data_buffer;
    GLuint buff_size, vert_size, norm_size, colo_size, text_size, indi_size;

    glGenVertexArrays(1, &mesh->vertex_array_object_id);
    glBindVertexArray(mesh->vertex_array_object_id);

    vert_size = mesh->nr_verticies * 3 * sizeof(GLfloat);
    norm_size = vert_size;
    colo_size = mesh->nr_verticies * 4 * sizeof(GLfloat);
    text_size = mesh->nr_verticies * 2 * sizeof(GLfloat);
    indi_size = mesh->nr_indicies * sizeof(GLuint);

    buff_size = vert_size + norm_size + colo_size;
    if (mesh->uvs != NULL) {
        buff_size += text_size;
    }

    glGenBuffers(1, &vertex_data_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, buff_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vert_size, mesh->verts);
    glBufferSubData(GL_ARRAY_BUFFER, vert_size, norm_size, mesh->norms);
    glBufferSubData(GL_ARRAY_BUFFER, vert_size + norm_size, colo_size, mesh->colors);
    if (mesh->uvs != NULL) {
        glBufferSubData(GL_ARRAY_BUFFER, vert_size + norm_size + colo_size, text_size, mesh->uvs);
    }

    glGenBuffers(1, &vertex_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indi_size, mesh->indicies, GL_STATIC_DRAW);

    glVertexB = glGetAttribLocation(program, "glVertexB" );
    glVertexAttribPointer(glVertexB, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glVertexB);

    glNormalB = glGetAttribLocation(program, "glNormalB" );
    glVertexAttribPointer(glNormalB, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vert_size));
    glEnableVertexAttribArray(glNormalB);

    if (mesh->uvs != NULL) {
        glTextB = glGetAttribLocation(program, "glTextB" );
        glVertexAttribPointer(glTextB, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vert_size + norm_size + colo_size));
        glEnableVertexAttribArray(glTextB);
    }
    else {
        glColorB = glGetAttribLocation(program, "glColorB" );
        glVertexAttribPointer(glColorB, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vert_size + norm_size));
        glEnableVertexAttribArray(glColorB);
    }

    glBindVertexArray(0);
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

void ogl_object_rotate_render(ogl_object_rotate* rotate) {
    if (rotate->x != 0) {
        glRotatef(rotate->x, 1, 0, 0);
    }
    if (rotate->y != 0) {
        glRotatef(rotate->z, 0, 1, 0);
    }
    if (rotate->z != 0) {
        glRotatef(rotate->z, 0, 0, 1);
    }
    if (rotate->node != NULL) {
        ogl_node_render(rotate->node);
    }
    if (rotate->z != 0) {
        glRotatef(0 - rotate->z, 0, 0, 1);
    }
    if (rotate->y != 0) {
        glRotatef(0 - rotate->z, 0, 1, 0);
    }
    if (rotate->x != 0) {
        glRotatef(0 - rotate->x, 1, 0, 0);
    }
}

void ogl_object_trans_render(ogl_object_trans* trans) {
    glTranslatef(trans->x, trans->y, trans->z);
    if (trans->node != NULL) {
        ogl_node_render(trans->node);
    }
    glTranslatef(0 - trans->x, 0 - trans->y, 0 - trans->z);
}

void ogl_object_mesh_render(ogl_object_mesh* mesh) {
    if (mesh->vertex_array_object_id == 0) {
        return;
    }
    if (mesh->visible == 0) {
        return;
    }
    glBindVertexArray(mesh->vertex_array_object_id);
    glDrawElements(GL_TRIANGLES, mesh->nr_indicies, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

void ogl_node_render(ogl_node* node) {
    switch(node->type) {
        case OGL_OBJECT_MESH:
            ogl_object_mesh_render(node->object_pointer.object_mesh);
            break;
        case OGL_OBJECT_TRANS:
            ogl_object_trans_render(node->object_pointer.object_trans);
            break;
        case OGL_OBJECT_ROTATE:
            ogl_object_rotate_render(node->object_pointer.object_rotate);
            break;
    }
}

void ogl_object_cube_generate_geometry(ogl_object_mesh* mesh, GLfloat x, GLfloat y, GLfloat z) {
    int nr_vert_floats, nr_color_floats;
    GLfloat* verts;
    GLfloat* norms;
    GLuint* indicies;
    GLfloat* colors;
    int ioff, voff, coff;
    GLfloat r, g, b;

    mesh->nr_verticies = 4 * 6;
    nr_vert_floats = 3 * mesh->nr_verticies;
    verts = malloc(sizeof(GLfloat) * nr_vert_floats);
    norms = malloc(sizeof(GLfloat) * nr_vert_floats);

    mesh->nr_indicies = 6 * 6;
    indicies = malloc(sizeof(GLuint) * mesh->nr_indicies);

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
