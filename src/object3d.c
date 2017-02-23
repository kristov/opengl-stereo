#include "object3d.h"
#include <stdio.h>

void generatePlane(int uSize, int vSize, int z, object3d* obj, enum planeType pt, int vert_off, int ind_off, int col_off) {
    int i, ti, vi;
    int x, y;
    GLfloat xf, yf, zf;
    GLfloat r, g, b;
    int nrVertFloats;
    int nrColorFloats;
    int UM = 0;
    int VM = 1;
    int ZM = 2;

    switch (pt) {
        case F_TOP_BOTTOM:
            printf("F_TOP_BOTTOM\n");
            UM = 0;
            VM = 2;
            ZM = 1;
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
            break;
        case F_LEFT_RIGHT:
            printf("F_LEFT_RIGHT\n");
            UM = 2;
            VM = 1;
            ZM = 0;
            r = 0.0f;
            g = 1.0f;
            b = 0.0f;
            break;
        case F_FRONT_BACK:
            printf("F_FRONT_BACK\n");
            UM = 0;
            VM = 1;
            ZM = 2;
            r = 0.0f;
            g = 0.0f;
            b = 1.0f;
            break;
    }

    nrColorFloats = 0;
    for (y = 0; y <= vSize; y++) {
        for (x = 0; x <= uSize; x++, vert_off += 3) {
            xf = (GLfloat)x; // / 10;
            yf = (GLfloat)y; // / 10;
            zf = (GLfloat)z; // / 10;
            obj->verts[vert_off + UM] = xf;
            obj->verts[vert_off + VM] = yf;
            obj->verts[vert_off + ZM] = zf;
            nrColorFloats++;
        }
    }

    for (vi = 0, y = 0; y < vSize; y++, vi++) {
        for (x = 0; x < uSize; x++, ind_off += 6, vi++) {
            obj->indicies[ind_off] = vi;
            obj->indicies[ind_off + 3] = obj->indicies[ind_off + 2] = vi + 1;
            obj->indicies[ind_off + 4] = obj->indicies[ind_off + 1] = vi + uSize + 1;
            obj->indicies[ind_off + 5] = vi + uSize + 2;
        }
    }

    for (i = 0; i < nrColorFloats; i++, col_off += 4) {
        obj->colors[col_off + 0] = r;
        obj->colors[col_off + 1] = g;
        obj->colors[col_off + 2] = b;
        obj->colors[col_off + 3] = 1.0f;
    }
}

object3d* cube(GLfloat x, GLfloat y, GLfloat z) {
    int nrVertFloats, nrColorFloats;
    GLfloat* verts;
    GLuint* indicies;
    GLfloat* colors;
    int ioff, voff, coff;
    GLfloat rf, gf, bf, rl, gl, bl, rr, gr, br, rt, gt, bt, rb, gb, bb, rk, gk, bk;

    object3d* obj = malloc(sizeof(object3d));

    obj->numVerticies = 4 * 6;
    nrVertFloats = 3 * obj->numVerticies;
    verts = malloc(sizeof(GLfloat) * nrVertFloats);

    obj->numIndicies = 6 * 6;
    indicies = malloc(sizeof(GLuint) * obj->numIndicies);

    nrColorFloats = 4 * obj->numVerticies;
    colors = malloc(sizeof(GLfloat) * nrColorFloats);

    rf = 1.0f;
    gf = 0.0f;
    bf = 0.0f;

    rl = 0.0f;
    gl = 1.0f;
    bl = 0.0f;

    rr = 0.0f;
    gr = 0.0f;
    br = 1.0f;

    rt = 1.0f;
    gt = 0.0f;
    bt = 1.0f;

    rb = 1.0f;
    gb = 1.0f;
    bb = 0.0f;

    rk = 0.5f;
    gk = 0.5f;
    bk = 0.5f;

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

    // front
    colors[coff + 0] = rf; // 0
    colors[coff + 1] = gf;
    colors[coff + 2] = bf;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rf; // 1
    colors[coff + 1] = gf;
    colors[coff + 2] = bf;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rf; // 2
    colors[coff + 1] = gf;
    colors[coff + 2] = bf;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rf; // 3
    colors[coff + 1] = gf;
    colors[coff + 2] = bf;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // left
    colors[coff + 0] = rl; // 4
    colors[coff + 1] = gl;
    colors[coff + 2] = bl;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rl; // 5
    colors[coff + 1] = gl;
    colors[coff + 2] = bl;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rl; // 6
    colors[coff + 1] = gl;
    colors[coff + 2] = bl;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rl; // 7
    colors[coff + 1] = gl;
    colors[coff + 2] = bl;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // right
    colors[coff + 0] = rr; // 8
    colors[coff + 1] = gr;
    colors[coff + 2] = br;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rr; // 9
    colors[coff + 1] = gr;
    colors[coff + 2] = br;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rr; // 10
    colors[coff + 1] = gr;
    colors[coff + 2] = br;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rr; // 11
    colors[coff + 1] = gr;
    colors[coff + 2] = br;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // top
    colors[coff + 0] = rt; // 12
    colors[coff + 1] = gt;
    colors[coff + 2] = bt;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rt; // 13
    colors[coff + 1] = gt;
    colors[coff + 2] = bt;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rt; // 14
    colors[coff + 1] = gt;
    colors[coff + 2] = bt;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rt; // 15
    colors[coff + 1] = gt;
    colors[coff + 2] = bt;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // bottom
    colors[coff + 0] = rb; // 16
    colors[coff + 1] = gb;
    colors[coff + 2] = bb;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rb; // 17
    colors[coff + 1] = gb;
    colors[coff + 2] = bb;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rb; // 18
    colors[coff + 1] = gb;
    colors[coff + 2] = bb;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rb; // 19
    colors[coff + 1] = gb;
    colors[coff + 2] = bb;
    colors[coff + 3] = 1.0f;
    coff += 4;

    // back
    colors[coff + 0] = rk; // 20
    colors[coff + 1] = gk;
    colors[coff + 2] = bk;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rk; // 21
    colors[coff + 1] = gk;
    colors[coff + 2] = bk;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rk; // 22
    colors[coff + 1] = gk;
    colors[coff + 2] = bk;
    colors[coff + 3] = 1.0f;
    coff += 4;
    colors[coff + 0] = rk; // 23
    colors[coff + 1] = gk;
    colors[coff + 2] = bk;
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

    obj->verts = verts;
    obj->indicies = indicies;
    obj->colors = colors;

    return obj;
}

/*
void checkerFloor() {
    int gridSizeX = 10;
    int gridSizeY = 10;
    int x = 0;
    int y = 0;
    float depth = -3.0f;
 
    unsigned int SizeX = 1;
    unsigned int SizeY = 1;

    glBegin(GL_QUADS);
    for (x = 0; x < gridSizeX; ++x) {
        for (y = 0; y < gridSizeY; ++y) {
            if ((x+y) & 0x00000001)
                glColor3f(1.0f,1.0f,1.0f);
            else
                glColor3f(0.0f,0.0f,0.0f);
            glVertex3f(    x*SizeX,depth,    y*SizeY);
            glVertex3f((x+1)*SizeX,depth,    y*SizeY);
            glVertex3f((x+1)*SizeX,depth,(y+1)*SizeY);
            glVertex3f(    x*SizeX,depth,(y+1)*SizeY);
 
        }
    }
    glEnd();
}
*/
