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

object3d* generateCube(int xSize, int ySize, int zSize) {
    int nvt_front_back, nvt_left_right, nvt_top_bottom;
    int nin_front_back, nin_left_right, nin_top_bottom;
    int vert_off = 0, ind_off = 0, col_off = 0;
    int nrVertFloats, nrColorFloats;

    object3d* obj = malloc(sizeof(object3d));

    nvt_front_back = (xSize + 1) * (ySize + 1);
    nvt_left_right = (zSize + 1) * (ySize + 1);
    nvt_top_bottom = (xSize + 1) * (zSize + 1);

    nin_front_back = xSize * ySize * 6;
    nin_left_right = zSize * ySize * 6;
    nin_top_bottom = xSize * zSize * 6;

    obj->numVerticies = (nvt_front_back * 2) + (nvt_left_right * 2) + (nvt_top_bottom * 2);
    nrVertFloats = 3 * obj->numVerticies;
    obj->verts = malloc(sizeof(GLfloat) * nrVertFloats);

    obj->numIndicies = (nin_front_back * 2) + (nin_left_right * 2) + (nin_top_bottom * 2);
    obj->indicies = malloc(sizeof(GLuint) * obj->numIndicies);

    nrColorFloats = 4 * obj->numVerticies;
    obj->colors = malloc(sizeof(GLfloat) * nrColorFloats);

    printf("nr verts    == %d\n", obj->numVerticies);
    printf("nr vert fl  == %d\n", nrVertFloats);
    printf("nr indicies == %d\n", obj->numIndicies);
    printf("nr colors   == %d\n", nrColorFloats);

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    generatePlane(xSize, ySize, 5, obj, F_FRONT_BACK, vert_off, ind_off, col_off);

    vert_off += nvt_front_back * 3;
    ind_off += nin_front_back;
    col_off += nvt_front_back * 4;

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    generatePlane(xSize, ySize, -5, obj, F_FRONT_BACK, vert_off, ind_off, col_off);

    vert_off += nvt_front_back * 3;
    ind_off += nin_front_back;
    col_off += nvt_front_back * 4;

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    generatePlane(zSize, ySize, 5, obj, F_LEFT_RIGHT, vert_off, ind_off, col_off);

    vert_off += nvt_left_right * 3;
    ind_off += nin_left_right;
    col_off += nvt_left_right * 4;

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    generatePlane(zSize, ySize, -5, obj, F_LEFT_RIGHT, vert_off, ind_off, col_off);

    vert_off += nvt_left_right * 3;
    ind_off += nin_left_right;
    col_off += nvt_left_right * 4;

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    generatePlane(xSize, zSize, 5, obj, F_TOP_BOTTOM, vert_off, ind_off, col_off);

    vert_off += nvt_top_bottom * 3;
    ind_off += nin_top_bottom;
    col_off += nvt_top_bottom * 4;

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    generatePlane(xSize, zSize, -5, obj, F_TOP_BOTTOM, vert_off, ind_off, col_off);

    vert_off += nvt_top_bottom * 3;
    ind_off += nin_top_bottom;
    col_off += nvt_top_bottom * 4;

    printf("vert_off         == %d\n", vert_off);
    printf("ind_off          == %d\n", ind_off);
    printf("col_off          == %d\n", col_off);

    return obj;
}
