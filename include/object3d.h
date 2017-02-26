#include <stdlib.h>
#include <GL/glut.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

enum planeType {
    F_TOP_BOTTOM,
    F_LEFT_RIGHT,
    F_FRONT_BACK
};

typedef struct {
    GLuint vertexArrayObject;
    int numVerticies;
    int numIndicies;
    GLfloat* verts;
    GLuint* indicies;
    GLfloat* colors;
    GLfloat* uvs;
} object3d;

void generatePlane(int xSize, int ySize, int z, object3d* obj, enum planeType pt, int vert_off, int ind_off, int col_off);
object3d* square(GLfloat x, GLfloat y);
object3d* cube(GLfloat x, GLfloat y, GLfloat z);
