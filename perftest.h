/* Performance test for low-level graphics functions
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __WIN32__
#define COBJMACROS
#include <GL/glut.h>
#include <GL/tg_extras.h> /* Extra GL stuff not provided by freeglut or mingw */
#include <d3d9.h>

#elif defined(linux)
#include <GL/glut.h>

#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#define APIENTRY
#endif

#ifndef linux
#define APIENTRYP APIENTRY *
#endif

#include <sys/time.h>
#include <stdio.h>


/* Matrix index macro */
#define MAT4_INDEX(i,j) (j*4+i)
#define DEG2RAD(a) (a*3.141592653589793/180.0)

/* buffer helper macro */
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/* Default values */
#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480

/* Geometry data */
#define _A 0.525731112119133606f
#define _B 0.850650808352039932f
#define NUM_INDICES 20
#define NUM_VERTICES 24
#define QUAD_W 200.0f
#define QUAD_H 200.0f
extern unsigned short icosahedronIndices[NUM_INDICES][3];
extern float icosahedronVertices[NUM_VERTICES][3];
extern float quadVertices[4][3];
extern float quadTexcoords[4][3];
extern double gProjectionMatrix[16];
extern float gProjectionMatrixf[16];
extern float gOrthoProjectionMatrixf[16];
extern double gModelViewMatrix[16];
extern float gModelViewMatrixf[16];
extern float gModelViewMatrixf2[16];
extern GLuint gBufferWidth;
extern GLuint gBufferHeight;
extern float gRotation;

/* config options */
extern int gIgnoreKeyboard;
extern int gUseGLSL;
extern int gHaveBindableUniform;
extern int gUseBindableUniform;
extern int gHaveVAO;
extern int gUseVAO;
extern int gResetVertexPointers;
extern int gResetConstants;
extern int gNumDrawCalls;
extern int gNumFrames;
extern int gUseMultiThreadedGL;

/* perftest.c */
int handleKeyPress(unsigned char key);
void matrixTranslate(double A[16], double tx, double ty, double tz);
void matrixRotateZ(double A[16], double angle);
void matrixMultiply(double A[16], double B[16], double C[16]);
void killProcess(int result);
void setViewport(int x, int y, int width, int height);
void utilCreateFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal, GLdouble *mat);
void utilCreateOrtho(float w, float h, GLdouble mat[16]);
void utilReshapeOrtho(GLuint width, GLuint height);
void update();

/* perftest_d3d9.c */
extern void initD3D9();
extern void setViewportD3D9(int x, int y, int width, int height);
extern void shutdownD3D9();

/* perftest_ogl.c */
extern void initOpenGL(int argc, char **argv);
extern void setViewportOGL(int x, int y, int width, int height);
extern void shutdownOpenGL();

