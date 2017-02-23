#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#define DTR 0.0174532925
#include <unistd.h>
#include "object3d.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

int rotate_delay = 30;
double theta = 0;

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

struct screeninf {
    double screenwidth;
    double screenheight;
    float depthZ;
    double fovy;
    double aspect;
    double nearZ;
    double farZ;
    double screenZ;
    double IOD;
} screenInfo;

static struct {
    GLuint vertex_shader_buffer;
    GLuint fragment_shader_buffer;
    GLuint program_buffer;
    GLuint vertex_shader_screen;
    GLuint fragment_shader_screen;
    GLuint program_screen;
    GLuint barrel_power_addr;
} g_resources;

static object3d* objects[10];
int numObjects = 0;

GLchar *file_contents(const char *filename, GLint *length) {
    char *buffer = 0;
    FILE *f = fopen(filename, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        *length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(*length);
        if (buffer) {
            fread(buffer, 1, *length, f);
        }
        fclose (f);
    }

    if (buffer) {
        return buffer;
    }

    return NULL;
}

static void show_info_log( GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog ) {
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

static GLuint make_shader(GLenum type, const char *filename) {
    GLint length;
    GLchar *source = file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source) {
        fprintf(stderr, "Failed to load file %s\n", filename);
        return 0;
    }

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader) {
    GLint program_ok;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

static int makeResources(void) {
    g_resources.vertex_shader_buffer = make_shader(GL_VERTEX_SHADER, "vert_buffer.glsl");
    if (g_resources.vertex_shader_buffer == 0)
        return 0;

    g_resources.fragment_shader_buffer = make_shader(GL_FRAGMENT_SHADER, "frag_buffer.glsl");
    if (g_resources.fragment_shader_buffer == 0)
        return 0;

    g_resources.program_buffer = make_program(g_resources.vertex_shader_buffer, g_resources.fragment_shader_buffer);
    if (g_resources.program_buffer == 0)
        return 0;

    g_resources.vertex_shader_screen = make_shader(GL_VERTEX_SHADER, "vert_screen.glsl");
    if (g_resources.vertex_shader_screen == 0)
        return 0;

    g_resources.fragment_shader_screen = make_shader(GL_FRAGMENT_SHADER, "frag_screen.glsl");
    if (g_resources.fragment_shader_screen == 0)
        return 0;

    g_resources.program_screen = make_program(g_resources.vertex_shader_screen, g_resources.fragment_shader_screen);
    if (g_resources.program_screen == 0)
        return 0;

    //g_resources.barrel_power_addr = glGetUniformLocation(g_resources.program_screen, "barrel_power");

    glUseProgram(g_resources.program_buffer);

    //glUniform1f(g_resources.barrel_power_addr, 0.6f);
    return 1;
}

void drawFloor() {
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

void drawscene() {
    int objID = 0;
    for (objID = 0; objID < numObjects; objID++) {
        object3d* obj = objects[objID];
        glPushMatrix();
        {
            glRotatef(theta,1.0,1.0,1.0);
            glDrawElements(GL_TRIANGLES, obj->numIndicies, GL_UNSIGNED_INT, NULL);
        }
        glPopMatrix();
    }
}

void storeCube() {
    GLuint vertexIndexBuffer;
    GLuint vertexDataBuffer;
    GLuint glVertexB;
    GLuint glColorB;
    int vert_off, ind_off, col_off, i;

    object3d* obj = cube(1.0f, 1.0f, 1.0f);
    objects[numObjects] = obj;
    numObjects++;

    glGenVertexArrays(1, &obj->vertexArrayObject);
    glBindVertexArray(obj->vertexArrayObject);

    glGenBuffers(1, &vertexDataBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
    glBufferData(GL_ARRAY_BUFFER, obj->numVerticies * 7 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, obj->numVerticies * 3 * sizeof(GLfloat), obj->verts);
    glBufferSubData(GL_ARRAY_BUFFER, obj->numVerticies * 3 * sizeof(GLfloat), obj->numVerticies * 4 * sizeof(GLfloat), obj->colors);

    glVertexB = glGetAttribLocation(g_resources.program_buffer, "glVertexB" );
    glVertexAttribPointer(glVertexB, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glVertexB);

    glColorB = glGetAttribLocation(g_resources.program_buffer, "glColorB" );
    glVertexAttribPointer(glColorB, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(obj->numVerticies * 3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(glColorB);

    glGenBuffers(1, &vertexIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj->numIndicies * sizeof(GLuint), obj->indicies, GL_STATIC_DRAW);
}

void createRenderTexture() {
    GLuint frameBufferName = 0;
    glGenFramebuffers(1, &frameBufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);

    GLuint renderedTextureLeft;
    glGenTextures(1, &renderedTextureLeft);
    glBindTexture(GL_TEXTURE_2D, renderedTextureLeft);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 512, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    GLuint depthRenderBuffer;
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTextureLeft, 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
    glViewport(0, 0, 512, 600); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}

void setFrustum(void) {
    // sets top of frustum based on fovy and near clipping plane
    double top = screenInfo.nearZ * tan(DTR * screenInfo.fovy / 2);

    // sets right of frustum based on aspect ratio
    double right = screenInfo.aspect * top;
    double frustumshift = (screenInfo.IOD / 2) * screenInfo.nearZ / screenInfo.screenZ;

    leftCam.topfrustum = top;
    leftCam.bottomfrustum = -top;
    leftCam.leftfrustum = -right + frustumshift;
    leftCam.rightfrustum = right + frustumshift;
    leftCam.modeltranslation = screenInfo.IOD / 2;

    rightCam.topfrustum = top;
    rightCam.bottomfrustum = -top;
    rightCam.leftfrustum = -right - frustumshift;
    rightCam.rightfrustum = right - frustumshift;
    rightCam.modeltranslation = -screenInfo.IOD / 2;
}

void initGL(void) {
    //glClearColor(0.0f,0.0f,1.0f,0.0f);
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
    screenInfo.screenwidth = w;
    screenInfo.screenheight = h;
    screenInfo.aspect = ( (double)w / 2 ) / (double)h;
    //glViewport(0, 0, w, h);
    setFrustum();
}

GLvoid display(GLvoid) {
    glDrawBuffer(GL_BACK);                                   //draw into both back buffers
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);        //clear color and depth buffers

    glDrawBuffer(GL_BACK_LEFT);                              //draw into back left buffer
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                                        //reset projection matrix
    glViewport(0, 0, screenInfo.screenwidth / 2, screenInfo.screenheight);
    glFrustum(leftCam.leftfrustum, leftCam.rightfrustum,     //set left view frustum
              leftCam.bottomfrustum, leftCam.topfrustum,
              screenInfo.nearZ, screenInfo.farZ);
    glTranslatef(leftCam.modeltranslation, 0.0, 0.0);        //translate to cancel parallax
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    {
        glTranslatef(0.0, 0.0, screenInfo.depthZ);           //translate to screenplane
        drawscene();
    }
    glPopMatrix();

    glDrawBuffer(GL_BACK_RIGHT);                             //draw into back right buffer
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                                        //reset projection matrix
    glViewport(screenInfo.screenwidth / 2, 0, screenInfo.screenwidth / 2, screenInfo.screenheight);
    glFrustum(rightCam.leftfrustum, rightCam.rightfrustum,   //set left view frustum
              rightCam.bottomfrustum, rightCam.topfrustum,
              screenInfo.nearZ, screenInfo.farZ);
    glTranslatef(rightCam.modeltranslation, 0.0, 0.0);       //translate to cancel parallax
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    {
        glTranslatef(0.0, 0.0, screenInfo.depthZ);           //translate to screenplane
        drawscene();
    }
    glPopMatrix();
  
    glutSwapBuffers();
}

void initScreenInfo() {
    screenInfo.screenwidth = 1024;
    screenInfo.screenheight = 600;
    screenInfo.depthZ = -10.0;       // depth of the object drawing
    screenInfo.fovy = 45;            // field of view in y-axis
    screenInfo.aspect = 0;
    screenInfo.nearZ = 3.0;          // near clipping plane
    screenInfo.farZ = 30.0;          // far clipping plane
    screenInfo.screenZ = 10.0;       // screen projection plane
    screenInfo.IOD = 0.5;            // intraocular distance
}

void init(int *argc, char **argv) {
    initScreenInfo();
    glutInit(argc, argv);
    screenInfo.aspect = screenInfo.screenwidth / screenInfo.screenheight;
    glutInitWindowSize(screenInfo.screenwidth, screenInfo.screenheight);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("Stereo Test");
    glEnable(GL_DEPTH_TEST);
    setFrustum();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(rotate_delay, rotate, 0);
    makeResources();
    initGL();
    storeCube();
}

void main(int argc, char **argv) {
    init(&argc, argv);
    //main_loop();
    glutMainLoop();
    //cleanup();
}

