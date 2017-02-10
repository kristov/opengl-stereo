#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#define DTR 0.0174532925
#include <unistd.h>

int rotate_delay = 30;
double screenwidth = 1024;
double screenheight = 600;

/*
Eye separation is typically kept at 1/30th of the convergence distance and objects
closer than half the convergence distance are avoided in the scene.
*/

struct camera {
    GLdouble leftfrustum;
    GLdouble rightfrustum;
    GLdouble bottomfrustum;
    GLdouble topfrustum;
    GLfloat modeltranslation;
} leftCam, rightCam;

float depthZ = -10.0;                                      //depth of the object drawing

double fovy = 45;                                          //field of view in y-axis
double aspect = 0;
double nearZ = 3.0;                                        //near clipping plane
double farZ = 30.0;                                        //far clipping plane
double screenZ = 10.0;                                     //screen projection plane
double IOD = 0.5;                                          //intraocular distance
double theta = 0;

GLuint cube;

void drawFloor() {
    int gridSizeX = 10;
    int gridSizeY = 10;
    int x = 0;
    int y = 0;
    float depth = -3.0f;
 
    unsigned int SizeX = 8;
    unsigned int SizeY = 8;

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

void drawscene() {
    drawFloor();
    glPushMatrix();
    {
        glRotatef(theta,1.0,0.0,1.0);
        glCallList(cube);
    }
    glPopMatrix();
    glPushMatrix();
    {
        glTranslatef(1.0, 0.0, -2.0);
        glRotatef(theta+10,1.0,0.0,1.0);
        glCallList(cube);
    }
    glPopMatrix();
    glEnd();
}

void storeCube() {
    cube = glGenLists(1);
    glNewList(cube, GL_COMPILE);

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  1.0, 1.0 );
    glVertex3f(  0.5, -0.5, 0.5 );
    glVertex3f(  0.5,  0.5, 0.5 );
    glVertex3f( -0.5,  0.5, 0.5 );
    glVertex3f( -0.5, -0.5, 0.5 );
    glEnd();
     
    // Purple side - RIGHT
    glBegin(GL_POLYGON);
    glColor3f(  1.0,  0.0,  1.0 );
    glVertex3f( 0.5, -0.5, -0.5 );
    glVertex3f( 0.5,  0.5, -0.5 );
    glVertex3f( 0.5,  0.5,  0.5 );
    glVertex3f( 0.5, -0.5,  0.5 );
    glEnd();
     
    // Green side - LEFT
    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0,  0.0 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();
     
    // Blue side - TOP
    glBegin(GL_POLYGON);
    glColor3f(   0.0,  0.0,  1.0 );
    glVertex3f(  0.5,  0.5,  0.5 );
    glVertex3f(  0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glEnd();
     
    // Red side - BOTTOM
    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0,  0.0 );
    glVertex3f(  0.5, -0.5, -0.5 );
    glVertex3f(  0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();
    glEndList();
}

void setFrustum(void) {
    double top = nearZ*tan(DTR*fovy/2);                    //sets top of frustum based on fovy and near clipping plane
    double right = aspect*top;                             //sets right of frustum based on aspect ratio
    double frustumshift = (IOD/2)*nearZ/screenZ;

    leftCam.topfrustum = top;
    leftCam.bottomfrustum = -top;
    leftCam.leftfrustum = -right + frustumshift;
    leftCam.rightfrustum = right + frustumshift;
    leftCam.modeltranslation = IOD/2;

    rightCam.topfrustum = top;
    rightCam.bottomfrustum = -top;
    rightCam.leftfrustum = -right - frustumshift;
    rightCam.rightfrustum = right - frustumshift;
    rightCam.modeltranslation = -IOD/2;
}

void initGL(void) {
    //glViewport(0, 0, screenwidth, screenheight);            //sets drawing viewport
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void rotate(int value) {
    theta += 1;
    glutPostRedisplay();
    glutTimerFunc(rotate_delay, rotate, 0);
}

GLvoid reshape(int w, int h) {
    if (h==0) {
        h=1;                                               //prevent divide by 0
    }
    screenwidth = w;
    screenheight = h;
    aspect = ( (double)w / 2 ) / (double)h;
    //glViewport(0, 0, w, h);
    setFrustum();
}

GLvoid display(GLvoid) {
    glDrawBuffer(GL_BACK);                                   //draw into both back buffers
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);        //clear color and depth buffers

    glDrawBuffer(GL_BACK_LEFT);                              //draw into back left buffer
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                                        //reset projection matrix
    glViewport(0,0,screenwidth/2,screenheight);
    glFrustum(leftCam.leftfrustum, leftCam.rightfrustum,     //set left view frustum
              leftCam.bottomfrustum, leftCam.topfrustum,
              nearZ, farZ);
    glTranslatef(leftCam.modeltranslation, 0.0, 0.0);        //translate to cancel parallax
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    {
        glTranslatef(0.0, 0.0, depthZ);                        //translate to screenplane
        drawscene();
    }
    glPopMatrix();

    glDrawBuffer(GL_BACK_RIGHT);                             //draw into back right buffer
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                                        //reset projection matrix
    glViewport(screenwidth/2,0,screenwidth/2,screenheight);
    glFrustum(rightCam.leftfrustum, rightCam.rightfrustum,   //set left view frustum
              rightCam.bottomfrustum, rightCam.topfrustum,
              nearZ, farZ);
    glTranslatef(rightCam.modeltranslation, 0.0, 0.0);       //translate to cancel parallax
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    {
        glTranslatef(0.0, 0.0, depthZ);                        //translate to screenplane
        drawscene();
    }
    glPopMatrix();
  
    glutSwapBuffers();
}

void init(int *argc, char **argv) {
    glutInit(argc, argv);
    aspect = (double)screenwidth / (double)screenheight;
    glutInitWindowSize(screenwidth, screenheight);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("Stereo Test");
    glEnable(GL_DEPTH_TEST);
    setFrustum();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(rotate_delay, rotate, 0);
    initGL();
    storeCube();
}

void main(int argc, char **argv) {
    init(&argc, argv);
    //main_loop();
    glutMainLoop();
    //cleanup();
}

