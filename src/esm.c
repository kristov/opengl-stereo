#include <math.h>
#include <stdlib.h>
#include <memory.h>
#ifdef RASPBERRYPI
#include <GLES/gl.h>
#else /* not RASPBERRYPI */
#include <GL/gl.h>
#endif /* RASPBERRYPI */
#include "esm.h"

#define PI 3.1415926535f

void esm_set_identity(GLfloat* M) {
    M[0] = 1;
    M[1] = 0;
    M[2] = 0;
    M[3] = 0;
    M[4] = 0;
    M[5] = 1;
    M[6] = 0;
    M[7] = 0;
    M[8] = 0;
    M[9] = 0;
    M[10] = 1;
    M[11] = 0;
    M[12] = 0;
    M[13] = 0;
    M[14] = 0;
    M[15] = 1;
}

void esmMultiply(GLfloat* M, GLfloat* M2) {
    GLfloat a00 = M[0], a01 = M[1], a02 = M[2], a03 = M[3],
            a10 = M[4], a11 = M[5], a12 = M[6], a13 = M[7],
            a20 = M[8], a21 = M[9], a22 = M[10], a23 = M[11],
            a30 = M[12], a31 = M[13], a32 = M[14], a33 = M[15],
            b00 = M2[0], b01 = M2[1], b02 = M2[2], b03 = M2[3],
            b10 = M2[4], b11 = M2[5], b12 = M2[6], b13 = M2[7],
            b20 = M2[8], b21 = M2[9], b22 = M2[10], b23 = M2[11],
            b30 = M2[12], b31 = M2[13], b32 = M2[14], b33 = M2[15];

    M[0] = b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30;
    M[1] = b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31;
    M[2] = b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32;
    M[3] = b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33;
    M[4] = b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30;
    M[5] = b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31;
    M[6] = b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32;
    M[7] = b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33;
    M[8] = b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30;
    M[9] = b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31;
    M[10] = b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32;
    M[11] = b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33;
    M[12] = b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30;
    M[13] = b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31;
    M[14] = b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32;
    M[15] = b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33;
}

void esmScalef(GLfloat* M, GLfloat x, GLfloat y, GLfloat z) {
    M[0] *= x;
    M[1] *= x;
    M[2] *= x;
    M[3] *= x;
    M[4] *= y;
    M[5] *= y;
    M[6] *= y;
    M[7] *= y;
    M[8] *= z;
    M[9] *= z;
    M[10] *= z;
    M[11] *= z;
}

/*
void esmTranslatef(GLfloat* M, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat a00, a01, a02, a03,
            a10, a11, a12, a13,
            a20, a21, a22, a23;

    a00 = M[0]; a01 = M[1]; a02 = M[2]; a03 = M[3];
    a10 = M[4]; a11 = M[5]; a12 = M[6]; a13 = M[7];
    a20 = M[8]; a21 = M[9]; a22 = M[10]; a23 = M[11];

    M[0] = a00; M[1] = a01; M[2] = a02; M[3] = a03;
    M[4] = a10; M[5] = a11; M[6] = a12; M[7] = a13;
    M[8] = a20; M[9] = a21; M[10] = a22; M[11] = a23;

    M[12] = a00 * x + a10 * y + a20 * z + M[12];
    M[13] = a01 * x + a11 * y + a21 * z + M[13];
    M[14] = a02 * x + a12 * y + a22 * z + M[14];
    M[15] = a03 * x + a13 * y + a23 * z + M[15];
}
*/

void esmTranslatef(GLfloat* M, GLfloat x, GLfloat y, GLfloat z) {
    M[12] = M[0] * x + M[4] * y + M[8] * z + M[12];
    M[13] = M[1] * x + M[5] * y + M[9] * z + M[13];
    M[14] = M[2] * x + M[6] * y + M[10] * z + M[14];
    M[15] = M[3] * x + M[7] * y + M[11] * z + M[15];
}

void esmRotatef(GLfloat* M, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat len = sqrt(x * x + y * y + z * z),
            s, c, t,
            a00, a01, a02, a03,
            a10, a11, a12, a13,
            a20, a21, a22, a23,
            b00, b01, b02,
            b10, b11, b12,
            b20, b21, b22;

    if (!len) { return; }
    if (len != 1) {
        len = 1 / len;
        x *= len;
        y *= len;
        z *= len;
    }

    s = sin(angle);
    c = cos(angle);
    t = 1 - c;

    a00 = M[0]; a01 = M[1]; a02 = M[2]; a03 = M[3];
    a10 = M[4]; a11 = M[5]; a12 = M[6]; a13 = M[7];
    a20 = M[8]; a21 = M[9]; a22 = M[10]; a23 = M[11];

    b00 = x * x * t + c; b01 = y * x * t + z * s; b02 = z * x * t - y * s;
    b10 = x * y * t - z * s; b11 = y * y * t + c; b12 = z * y * t + x * s;
    b20 = x * z * t + y * s; b21 = y * z * t - x * s; b22 = z * z * t + c;

    M[0] = a00 * b00 + a10 * b01 + a20 * b02;
    M[1] = a01 * b00 + a11 * b01 + a21 * b02;
    M[2] = a02 * b00 + a12 * b01 + a22 * b02;
    M[3] = a03 * b00 + a13 * b01 + a23 * b02;

    M[4] = a00 * b10 + a10 * b11 + a20 * b12;
    M[5] = a01 * b10 + a11 * b11 + a21 * b12;
    M[6] = a02 * b10 + a12 * b11 + a22 * b12;
    M[7] = a03 * b10 + a13 * b11 + a23 * b12;

    M[8] = a00 * b20 + a10 * b21 + a20 * b22;
    M[9] = a01 * b20 + a11 * b21 + a21 * b22;
    M[10] = a02 * b20 + a12 * b21 + a22 * b22;
    M[11] = a03 * b20 + a13 * b21 + a23 * b22;
}

void esmFrustumf(GLfloat* M, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    GLfloat rl = (right - left),
            tb = (top - bottom),
            fn = (far - near);

    M[0] = (near * 2) / rl;
    M[1] = 0;
    M[2] = 0;
    M[3] = 0;
    M[4] = 0;
    M[5] = (near * 2) / tb;
    M[6] = 0;
    M[7] = 0;
    M[8] = (right + left) / rl;
    M[9] = (top + bottom) / tb;
    M[10] = -(far + near) / fn;
    M[11] = -1;
    M[12] = 0;
    M[13] = 0;
    M[14] = -(far * near * 2) / fn;
    M[15] = 0;
}

void esmPerspectivef(GLfloat* M, GLfloat fovy, GLfloat aspect, GLfloat near, GLfloat far) {
    GLfloat frustumW, frustumH;

    frustumH = tanf(fovy / 360.0f * PI) * near;
    frustumW = frustumH * aspect;

    esmFrustumf(M, -frustumW, frustumW, -frustumH, frustumH, near, far);
}

void esmOrthof(GLfloat* M, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    GLfloat rl = (right - left),
            tb = (top - bottom),
            fn = (far - near);
    M[0] = 2 / rl;
    M[1] = 0;
    M[2] = 0;
    M[3] = 0;
    M[4] = 0;
    M[5] = 2 / tb;
    M[6] = 0;
    M[7] = 0;
    M[8] = 0;
    M[9] = 0;
    M[10] = -2 / fn;
    M[11] = 0;
    M[12] = -(left + right) / rl;
    M[13] = -(top + bottom) / tb;
    M[14] = -(far + near) / fn;
    M[15] = 1;
}

/*
    [ 0  4  8  12 ]
    [ 1  5  9  13 ]
    [ 2  6  10 14 ]
    [ 3  7  11 15 ]

    [ 0  3  6 ]
    [ 1  4  7 ]
    [ 2  5  8 ]
*/

GLfloat* esmNormalMatrixFromProjection(GLfloat* M) {
    GLfloat* tmp = malloc(sizeof(GLfloat) * 16);
    GLfloat* N = malloc(sizeof(GLfloat) * 9);

    GLfloat a00 = M[0], a01 = M[1], a02 = M[2], a03 = M[3],
            a10 = M[4], a11 = M[5], a12 = M[6], a13 = M[7],
            a20 = M[8], a21 = M[9], a22 = M[10], a23 = M[11],
            a30 = M[12], a31 = M[13], a32 = M[14], a33 = M[15],
            b00 = a00 * a11 - a01 * a10,
            b01 = a00 * a12 - a02 * a10,
            b02 = a00 * a13 - a03 * a10,
            b03 = a01 * a12 - a02 * a11,
            b04 = a01 * a13 - a03 * a11,
            b05 = a02 * a13 - a03 * a12,
            b06 = a20 * a31 - a21 * a30,
            b07 = a20 * a32 - a22 * a30,
            b08 = a20 * a33 - a23 * a30,
            b09 = a21 * a32 - a22 * a31,
            b10 = a21 * a33 - a23 * a31,
            b11 = a22 * a33 - a23 * a32,
            d = (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06),
            invDet;

    if (!d) { return NULL; }
    invDet = 1 / d;

    tmp[0] = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
    tmp[1] = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
    tmp[2] = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
    tmp[3] = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
    tmp[4] = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
    tmp[5] = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
    tmp[6] = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
    tmp[7] = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
    tmp[8] = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
    tmp[9] = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
    tmp[10] = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
    tmp[11] = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
    tmp[12] = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
    tmp[13] = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
    tmp[14] = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
    tmp[15] = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;

    a01 = tmp[1];
    a02 = tmp[2];
    a03 = tmp[3];
    a12 = tmp[6];
    a13 = tmp[7];
    a23 = tmp[11];

    tmp[1] = tmp[4];
    tmp[2] = tmp[8];
    tmp[3] = tmp[12];
    tmp[4] = a01;
    tmp[6] = tmp[9];
    tmp[7] = tmp[13];
    tmp[8] = a02;
    tmp[9] = a12;
    tmp[11] = tmp[14];
    tmp[12] = a03;
    tmp[13] = a13;
    tmp[14] = a23;

    N[0] = tmp[0];
    N[1] = tmp[4];
    N[2] = tmp[8];
    N[3] = tmp[1];
    N[4] = tmp[5];
    N[5] = tmp[9];
    N[6] = tmp[2];
    N[7] = tmp[6];
    N[8] = tmp[10];

    free(tmp);

    return N;
}

void esmLoadIdentity(GLfloat* M) {
    esm_set_identity(M);
}

GLfloat* esmCreate() {
    GLfloat* M = malloc(sizeof(GLfloat) * 16);
    esm_set_identity(M);
    return M;
}

GLfloat* esmCreateCopy(GLfloat* M) {
    GLfloat* N = malloc(sizeof(GLfloat) * 16);
    N[0] = M[0];
    N[1] = M[1];
    N[2] = M[2];
    N[3] = M[3];
    N[4] = M[4];
    N[5] = M[5];
    N[6] = M[6];
    N[7] = M[7];
    N[8] = M[8];
    N[9] = M[9];
    N[10] = M[10];
    N[11] = M[11];
    N[12] = M[12];
    N[13] = M[13];
    N[14] = M[14];
    N[15] = M[15];
    return N;
}

void esmDestroy(GLfloat* M) {
    free(M);
}
