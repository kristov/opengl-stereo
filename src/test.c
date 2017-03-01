#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#define DTR 0.0174532925
#include <unistd.h>
#include "object3d.h"
#include "ogl_objecttree.h"

#define PRINT_GL_ERRORS

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define printOpenGLError() printGlError(__FILE__, __LINE__)

void discardGlError() {
    glGetError();
}

int printGlError(char *file, int line) {
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    if (glErr != GL_NO_ERROR) {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
    }
    return retCode;
}

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
    GLuint leftBuffer;
    GLuint rightBuffer;
    GLuint renderedTextureLeft;
    GLuint renderedTextureRight;
} buffers;

static struct {
    GLuint vertex_shader_buffer;
    GLuint fragment_shader_buffer;
    GLuint program_buffer;
    GLuint vertex_shader_screen;
    GLuint fragment_shader_screen;
    GLuint program_screen;
    GLuint barrel_power_addr;
} g_resources;

ogl_node* oglObjectRoot;
object3d* screenCanvas;

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

    glUseProgram(g_resources.program_buffer);

    return 1;
}

void renderObject(object3d* obj, GLuint program) {
    glBindVertexArray(obj->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, obj->numIndicies, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

void drawScene(GLuint program) {
    ogl_node_render(oglObjectRoot);
}

void storeObject(object3d* obj, GLuint program) {
    GLuint glVertexB, glColorB, glTextB;
    GLuint vertexIndexBuffer;
    GLuint vertexDataBuffer;
    GLuint vertexArrayObject;
    GLuint buffSize, vertSize, coloSize, textSize, indiSize;

    glGenVertexArrays(1, &obj->vertexArrayObject);
    glBindVertexArray(obj->vertexArrayObject);

    vertSize = obj->numVerticies * 3 * sizeof(GLfloat);
    coloSize = obj->numVerticies * 4 * sizeof(GLfloat);
    textSize = obj->numVerticies * 2 * sizeof(GLfloat);
    indiSize = obj->numIndicies * sizeof(GLuint);

    buffSize = vertSize + coloSize;
    if (obj->uvs != NULL) {
        buffSize += textSize;
    }

    glGenBuffers(1, &vertexDataBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexDataBuffer);
    glBufferData(GL_ARRAY_BUFFER, buffSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, obj->verts);
    glBufferSubData(GL_ARRAY_BUFFER, vertSize, coloSize, obj->colors);
    if (obj->uvs != NULL) {
        glBufferSubData(GL_ARRAY_BUFFER, vertSize + coloSize, textSize, obj->uvs);
    }

    glGenBuffers(1, &vertexIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiSize, obj->indicies, GL_STATIC_DRAW);

    glVertexB = glGetAttribLocation(program, "glVertexB" );
    glVertexAttribPointer(glVertexB, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(glVertexB);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    if (obj->uvs != NULL) {
        glTextB = glGetAttribLocation(program, "glTextB" );
        glVertexAttribPointer(glTextB, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertSize + coloSize));
        glEnableVertexAttribArray(glTextB);
#ifdef PRINT_GL_ERRORS
        printOpenGLError(); // ERROR
#endif
    }
    else {
        glColorB = glGetAttribLocation(program, "glColorB" );
        glVertexAttribPointer(glColorB, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertSize));
        glEnableVertexAttribArray(glColorB);
#ifdef PRINT_GL_ERRORS
        printOpenGLError(); // ERROR
#endif
    }

    glBindVertexArray(0);
}

void storeSquare(GLuint program) {
    GLuint glVertexB, glColorB;
    GLuint vertexIndexBuffer;
    GLuint vertexDataBuffer;
    GLuint buffSize, vertSize, coloSize, textSize;
    GLfloat* uvs;
    int voff;

    object3d* obj = square(2.0f, 2.0f);
    screenCanvas = obj;
    uvs = malloc(obj->numVerticies * 2 * sizeof(GLfloat));

    voff = 0;
    // front
    uvs[voff + 0] = 0.0f; // 0
    uvs[voff + 1] = 0.0f;
    voff += 2;
    uvs[voff + 0] = 1.0f; // 1
    uvs[voff + 1] = 0.0f;
    voff += 2;
    uvs[voff + 0] = 0.0f; // 2
    uvs[voff + 1] = 1.0f;
    voff += 2;
    uvs[voff + 0] = 1.0f; // 3
    uvs[voff + 1] = 1.0f;
    voff += 2;

    obj->uvs = uvs;

    storeObject(obj, program);
}

void createObjectTree(GLuint program) {
    ogl_node* cube1 = ogl_node_cube_create(2.0f, 2.0f, 2.0f);
    ogl_node* cube2 = ogl_node_cube_create(2.0f, 2.0f, 2.0f);
    ogl_node_color(cube2, 0.3f, 0.1f, 0.9f);
    ogl_node* trans1 = ogl_node_trans_create(1.0f, 1.0f, 1.0f, cube2);
    ogl_node* rotate1 = ogl_node_rotate_create(20.0f, 20.0f, 20.0f, trans1);
    //ogl_node* root = ogl_node_rotate_create(10.0f, 10.0f, 10.0f, cube1);
    ogl_node* root = rotate1;
    ogl_node_realize(root, program);
    oglObjectRoot = root;
}

void storeObjects() {
    createObjectTree(g_resources.program_buffer);
    storeSquare(g_resources.program_screen);
}

void createRenderTextures() {
    GLuint depthRenderBufferLeft;
    GLuint depthRenderBufferRight;
    float borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };

    glGenFramebuffers(1, &buffers.leftBuffer);
    glGenTextures(1, &buffers.renderedTextureLeft);
    glBindTexture(GL_TEXTURE_2D, buffers.renderedTextureLeft);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenInfo.screenwidth / 2, screenInfo.screenheight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBufferLeft);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferLeft);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screenInfo.screenwidth / 2, screenInfo.screenheight);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.leftBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers.renderedTextureLeft, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferLeft);


    glGenFramebuffers(1, &buffers.rightBuffer);
    glGenTextures(1, &buffers.renderedTextureRight);
    glBindTexture(GL_TEXTURE_2D, buffers.renderedTextureRight);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenInfo.screenwidth / 2, screenInfo.screenheight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBufferRight);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferRight);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screenInfo.screenwidth / 2, screenInfo.screenheight);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.rightBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers.renderedTextureRight, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferRight);

    return;
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
    // recreate the buffers in a new size
    setFrustum();
}

void renderSceneToLeftBuffer(GLuint program) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.leftBuffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glViewport(0, 0, screenInfo.screenwidth / 2, screenInfo.screenheight);

    glFrustum(leftCam.leftfrustum, leftCam.rightfrustum,
              leftCam.bottomfrustum, leftCam.topfrustum,
              screenInfo.nearZ, screenInfo.farZ);
    glTranslatef(leftCam.modeltranslation, 0.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, screenInfo.depthZ);
        drawScene(program);
    }
    glPopMatrix();
}

void renderSceneToRightBuffer(GLuint program) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.rightBuffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glViewport(0, 0, screenInfo.screenwidth / 2, screenInfo.screenheight);

    glFrustum(rightCam.leftfrustum, rightCam.rightfrustum,
              rightCam.bottomfrustum, rightCam.topfrustum,
              screenInfo.nearZ, screenInfo.farZ);
    glTranslatef(rightCam.modeltranslation, 0.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, screenInfo.depthZ);
        drawScene(program);
    }
    glPopMatrix();
}

void renderSceneToBuffers() {
    GLuint program = g_resources.program_buffer;
    glUseProgram(program);

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    renderSceneToLeftBuffer(program);
    renderSceneToRightBuffer(program);
}

void renderLeftBufferToWindow(GLuint program) {
    GLint texLoc;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, screenInfo.screenwidth / 2, screenInfo.screenheight);
    glFrustum(-1, 1, -1, 1, 0.0, 40.0);
    discardGlError();

    texLoc = glGetUniformLocation(program, "tex0");
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, buffers.renderedTextureLeft);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, 0.0);
        renderObject(screenCanvas, program);
    }
    glPopMatrix();

#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
}

void renderRightBufferToWindow(GLuint program) {
    GLint texLoc;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(screenInfo.screenwidth / 2, 0, screenInfo.screenwidth / 2, screenInfo.screenheight);
    glFrustum(-1, 1, -1, 1, 0.0, 40.0);
    discardGlError();

    texLoc = glGetUniformLocation(program, "tex0");
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, buffers.renderedTextureRight);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glPushMatrix();
    {
        glTranslatef(-1.0, -1.0, 0.0);
        renderObject(screenCanvas, program);
    }
    glPopMatrix();

#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
}

void renderBuffersToWindow() {
    GLuint program = g_resources.program_screen;
    glUseProgram(program);

    g_resources.barrel_power_addr = glGetUniformLocation(program, "barrel_power");
    glUniform1f(g_resources.barrel_power_addr, 1.1f);

#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif
    glClearColor(0.0f,0.8f,0.8f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#ifdef PRINT_GL_ERRORS
    printOpenGLError(); // ERROR
#endif

    renderLeftBufferToWindow(program);
    renderRightBufferToWindow(program);
    //renderSceneToLeftBuffer(program);
    //renderSceneToRightBuffer(program);
}

/*
    display():
        renderSceneToBuffers():
            glUseProgram(buffer) <-- Rendering a 3d scene
            renderSceneToLeftBuffer()
            renderSceneToRightBuffer()
        renderBuffersToWindow():
            glUseProgram(screen) <-- Rendering a texture
            renderLeftBufferToWindow()
            renderRightBufferToWindow()
        glutSwapBuffers();
*/
GLvoid display(GLvoid) {
    renderSceneToBuffers();
    renderBuffersToWindow();
    glutSwapBuffers();
}

void initGL(void) {
    GLfloat lightpos[] = { 0.5f, 1.0f, 1.0f, 0.0f };
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
}

void initScreenInfo() {
    screenInfo.screenwidth = 1024;
    screenInfo.screenheight = 600;
    screenInfo.aspect = screenInfo.screenwidth / screenInfo.screenheight;
    screenInfo.depthZ = -10.0;       // depth of the object drawing
    screenInfo.fovy = 45;            // field of view in y-axis
    screenInfo.nearZ = 3.0;          // near clipping plane
    screenInfo.farZ = 30.0;          // far clipping plane
    screenInfo.screenZ = 10.0;       // screen projection plane
    screenInfo.IOD = 0.5;            // intraocular distance
}

void initWindowingSystem(int *argc, char **argv) {
    glutInit(argc, argv);
    glutInitWindowSize(screenInfo.screenwidth, screenInfo.screenheight);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("Stereo Test");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(rotate_delay, rotate, 0);
}

void init3dSystem() {
    makeResources();
    setFrustum();
    createRenderTextures();
    storeObjects();
}

void init(int *argc, char **argv) {
    initScreenInfo();
    initWindowingSystem(argc, argv);
    initGL();
    init3dSystem();
}

void main(int argc, char **argv) {
    init(&argc, argv);
    glutMainLoop();
}
