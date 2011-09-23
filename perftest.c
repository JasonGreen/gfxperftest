/* Performance test for low-level graphics functions
 *
 * Copyright 2010-2011, TransGaming, Inc.
 *
 * To test:
 * Run from the command line. An FPS counter will be displayed every 1 seconds,
 * unless the "ignore_keyboard=1" command line option was given.  In that case,
 * the average FPS will show at the end of the run.
 */

/* This code is based in part on GL3 usage exampe code from

   filip.wanstrom _at_ gmail.com
   http://www.cfwdesign.se/gl3-tutorials/tutorial-2/
*/
#include "perftest.h"
#include <stdint.h>

/* Globals */
int     gCommandLineOptionsValid = 0;
int     gUseD3D9 = 0;
uint64_t gLastFrameTime;
uint64_t gLastFPSDrawTime;
int     gLastFPSDrawFrameCount;
int     gFPSFramesToSkip = 100; /* Skip the first 100 frames when averaging FPS (TODO: make configurable?) */
int     gIgnoreKeyboard = 0;
int     gUseGLSL = 0; /* Default to ARB_vp/fp */
int     gUseBindableUniform = 0;
int     gHaveBindableUniform = 0;
int     gHaveUniformBufferObject = 0;
int     gUseUniformBufferObject = 0;
int     gBindableUpdateMethod = 0; /* Defaults to BINDABLE_UPDATE_GLUNIFORM */
int     gUBOUpdateMethod = 0; /* DEFAULTS to UBO_UPDATE_BUFFERDATA */
int     gUseVAO = 0;
int     gResetVertexPointers = 0;
int     gResetConstants = 0;
int     gNumDrawCalls = 5000;
int     gNumFrames = 5000;
int     gUseMultiThreadedGL = 0;
int     gUseCoreContext = 0;
double  gProjectionMatrix[16];
float   gProjectionMatrixf[16];
float   gOrthoProjectionMatrixf[16];
double  gModelViewMatrix[16];
float   gModelViewMatrixf[16];
float   gModelViewMatrixf2[16];
GLuint  gBufferWidth = 1024;
GLuint  gBufferHeight = 1024;
float   gRotation;

/* Geometry data */
/******************************************************************************/

unsigned short icosahedronIndices[NUM_INDICES][3]=
{
{0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
{8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
{7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
{6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
};

float icosahedronVertices[NUM_VERTICES][3]=
{
{-_A,0.0,_B},{_A,0.0,_B},{-_A,0.0,-_B},{_A,0.0,-_B},
{0.0,_B,_A},{0.0,_B,-_A},{0.0,-_B,_A},{0.0,-_B,-_A},
{_B,_A,0.0},{-_B,_A,0.0},{_B,-_A,0.0},{-_B,-_A,0.0},

/* 36 */

{-_A,0.0,_B},{_A,0.0,_B},{-_A,0.0,-_B},{_A,0.0,-_B},
{0.0,_B,_A},{0.0,_B,-_A},{0.0,-_B,_A},{0.0,-_B,-_A},
{_B,_A,0.0},{-_B,_A,0.0},{_B,-_A,0.0},{-_B,-_A,0.0}
};

float quadVertices[4][3] =
{
{0, 0, 0.0f},
{0, QUAD_H, 0.0f},
{QUAD_W, 0, 0.0f},
{QUAD_W, QUAD_H, 0.0f}
};
float quadTexcoords[4][3] =
{
{0, 0, 0.0f},
{0, 1, 0.0f},
{1, 0, 0.0f},
{1, 1, 0.0f}
};

/******************************************************************************/

void matrixTranslate(double A[16], double tx, double ty, double tz)
{
    memset(A, 0, 16*sizeof(double));
    A[0] = A[5] = A[10] = A[15] = 1.0;
    A[12] = tx;
    A[13] = ty;
    A[14] = tz;

}

void matrixRotateZ(double A[16], double angle)
{
    double angled = DEG2RAD(angle);
    memset(A, 0, 16*sizeof(double));
    A[10] = A[15] = 1.0;

    A[0] = cos(angled);
    A[1] = sin(angled);
    A[4] = -sin(angled);
    A[5] = cos(angled);

}

void matrixMultiply(double A[16], double B[16], double C[16])
{
    int i, j, k;
        for ( i=0;i<4;i++ ) {
                for ( j=0;j<4;j++ ) {
                        C[MAT4_INDEX(i,j)] =  A[MAT4_INDEX(i,0)] * B[MAT4_INDEX(0,j)];
                        for (k=1;k<4;k++) {
                                C[MAT4_INDEX(i,j)] += A[MAT4_INDEX(i,k)] * B[MAT4_INDEX(k,j)];
                        }

                }
        }
}

/******************************************************************************/

void utilCreateFrustum( GLdouble     left,
                        GLdouble     right,
                        GLdouble     bottom,
                        GLdouble     top,
                        GLdouble     nearVal,
                        GLdouble     farVal,
                        GLdouble     *mat)
{
    double A = (right+left)/(right-left);
    double B = (top+bottom)/(top-bottom);
    double C = (farVal+nearVal)/(farVal-nearVal);
    double D = (2.0*farVal*nearVal)/(farVal-nearVal);
    int i;
    for (i=0;i<16;i++) { mat[i] = 0.0; }

    mat[0] = (2.0*nearVal)/(right-left);
    mat[5] = (2.0*nearVal)/(top-bottom);
    mat[8] = A;
    mat[9] = B;
    mat[10] = C;
    mat[11] = -1.0;
    mat[14] = D;
}

void utilCreateOrtho(float w, float h, GLdouble mat[16])
{
    GLdouble        left = 0;
    GLdouble        right = w;
    GLdouble        bottom = 0;
    GLdouble        top = h;
    GLdouble        dNear = -1.0;
    GLdouble        dFar = 1.0;

    GLdouble tx = -(right+left)/(right-left);
    GLdouble ty = -(top+bottom)/(top-bottom);
    GLdouble tz = -(dFar+dNear)/(dFar-dNear);

    int i=0;
    for (i=0;i<16;i++) { mat[i] = 0.0;}

    mat[0] = (2.0/(right-left));
    mat[5] = (2.0/(top-bottom));
    mat[10] = -(2.0/(dFar-dNear));
    mat[12] = tx;
    mat[13] = ty;
    mat[14] = tz;
    mat[15] = 1.0;

}

void utilReshape(GLuint width, GLuint height)
{
    int i;
    if (width ==0)
        width = 1;

    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    GLfloat h = (GLfloat) height / (GLfloat) width;
    setViewport(0, 0, width, height);

    utilCreateFrustum(-1.0f*nearPlane,
                      nearPlane,
                      -h*nearPlane,
                      h*nearPlane,
                      nearPlane,
                      farPlane,
                      gProjectionMatrix);

    for(i=0;i<16;i++) { gProjectionMatrixf[i] = gProjectionMatrix[i];}

}

/* Debugging function to print out the name of the bindable update method */
static inline const char* utilBindableString(int val)
{
    switch (val) {
        case BINDABLE_UPDATE_GLUNIFORM:                       return "GLUNIFORM";
        case BINDABLE_UPDATE_GLUNIFORM_WITH_DISCARD:          return "GLUNIFORM_WITH_DISCARD";
        case BINDABLE_UPDATE_BUFFERDATA:                      return "BUFFERDATA";
        case BINDABLE_UPDATE_BUFFERSUBDATA:                   return "BUFFERSUBDATA";
        case BINDABLE_UPDATE_BUFFERSUBDATA_WITH_DISCARD:      return "BUFFERSUBDATA_WITH_DISCARD";
        case BINDABLE_UPDATE_MAPBUFFER:                       return "MAPBUFFER";
        case BINDABLE_UPDATE_MAPBUFFER_WITH_DISCARD:          return "MAPBUFFER_WITH_DISCARD";
        case BINDABLE_UPDATE_FLUSH_BUFFER_RANGE:              return "FLUSH_BUFFER_RANGE";
        case BINDABLE_UPDATE_FLUSH_BUFFER_RANGE_WITH_DISCARD: return "FLUSH_BUFFER_RANGE_WITH_DISCARD";
        default:
            return "ERROR";
    }
}

/* Debugging function to print out the name of the UBO update method */
static inline const char* utilUBOString(int val)
{
    switch (val) {
        case UBO_UPDATE_BUFFERDATA:                 return "BUFFERDATA";
        case UBO_UPDATE_BUFFERSUBDATA:              return "BUFFERSUBDATA";
        case UBO_UPDATE_BUFFERSUBDATA_WITH_DISCARD: return "BUFFERSUBDATA_WITH_DISCARD";
        case UBO_UPDATE_MAPBUFFER:                  return "MAPBUFFER";
        case UBO_UPDATE_MAPBUFFER_WITH_DISCARD:     return "MAPBUFFER_WITH_DISCARD";
        case UBO_UPDATE_MAPBUFFER_RANGE:            return "MAPBUFFER_RANGE";
        case UBO_UPDATE_MAPBUFFER_RANGE_WITH_DISCARD: return "MAPBUFFER_RANGE_WITH_DISCARD";
        default:
        return "ERROR";
    }
}

/******************************************************************************/

void utilReshapeOrtho(GLuint width, GLuint height)
{
    if (width ==0)
        width = 1;

    double mat[16];
    int i;
    setViewport(0, 0, (GLint) width, (GLint) height);
    utilCreateOrtho(width, height, mat);
    for (i=0;i<16;i++) {gOrthoProjectionMatrixf[i] = mat[i]; }
}

/******************************************************************************/

void killProcess(int result)
{
    if (gUseD3D9)
        shutdownD3D9();
    else
        shutdownOpenGL();

    exit(result);
}

static void init()
{
    utilReshape(gBufferWidth, gBufferHeight);
}

static uint64_t getTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void update()
{
    double T[16];
    double R[16];
    int i;
    static int frame = 1;

    if (frame==1) {
        init();
    }
    frame++;

    /* Update */
    if (gLastFrameTime == 0)
    {
        gLastFPSDrawTime = gLastFrameTime = getTime();
        gLastFPSDrawFrameCount = 0;
    }
    else
        gLastFPSDrawFrameCount++;

    uint64_t now = getTime();
    uint64_t elapsedMilliseconds = now - gLastFrameTime;
    float elapsedTime = elapsedMilliseconds / 1000.0f;
    gLastFrameTime = now;

    /* Show FPS once and quit automatically if run with command line options.
     * Also, skip the first few frames to allow the GL to finish all of its 
     * setup routines, so startup time doesn't factor into average FPS. */
    if (gIgnoreKeyboard) {
        /* For command line ,  */
        if (frame == gFPSFramesToSkip) {
            gLastFPSDrawTime = gLastFrameTime = getTime();
            gLastFPSDrawFrameCount = 0;
        } else if (frame == gNumFrames) {
            printf("FPS: %f\n", (float)gLastFPSDrawFrameCount * 1000.0f/ (now - gLastFPSDrawTime));
            killProcess(0);
        }

    /* Otherwise, print FPS every 1 second */
    } else {
        if ((now - gLastFPSDrawTime) >= 1000)
        {
            printf("FPS: %f\n", (float)gLastFPSDrawFrameCount * 1000.0f/ (now - gLastFPSDrawTime));
            gLastFPSDrawTime = now;
            gLastFPSDrawFrameCount = 0;
        }
    }

    gRotation+=elapsedTime*30.0f;    

    /* transform the geometry */
    matrixTranslate(T , 0, 0, -10);
    matrixRotateZ(R, gRotation);
    matrixMultiply(T, R, gModelViewMatrix);

    /* copy doubles to float */
    for(i=0;i<16;i++) { gModelViewMatrixf[i] = gModelViewMatrix[i];}


    /* And again for the secondary viewmatrix */
    matrixTranslate(T , 0, 0, -10);
    matrixRotateZ(R, gRotation*10);
    matrixMultiply(T, R, gModelViewMatrix);
    for(i=0;i<16;i++) { gModelViewMatrixf2[i] = gModelViewMatrix[i];}

}

/******************************************************************************/
/* Graphics abstraction functions */
void setViewport(int x, int y, int width, int height)
{
    if (gUseD3D9)
        setViewportD3D9(x, y, width, height);
    else
        setViewportOGL(x, y, width, height);
}

static void displayHelp()
{
    printf("\nValid command line options for perftest:\n");
    printf("  -ignore_keyboard=[0,1]       1 = Ignore keyboard toggle commands\n");
    printf("  -use_d3d9=[0,1]              1 = Use D3D9 engine (Windows only)\n");
    printf("  -use_multithreaded_gl=[0,1]  1 = Use MultiThreaded GL (Mac only) (M)\n");
    printf("  -use_glsl=[0,1]              1 = Use GLSL, 0 = Use ARB_vp/fp (G)\n");
    printf("  -use_bindable_uniform=[0,1]  1 = Use EXT_bindable_uniform with GLSL (B)\n");
    printf("  -use_ubo=[0,1]               1 = Use ARB_uniform_buffer_object with GLSL (O)\n");
    printf("  -bindable_update_method=[0..%d] Which bindable uniform update method (U)\n", BINDABLE_UPDATE_NUM_METHODS);
    printf("  -ubo_update_method=[0..%d]      Which UBO uniform update method (U)\n", UBO_UPDATE_NUM_METHODS);
    printf("  -use_vao=[0,1]               1 = Use VAO, 0 = Use general vertex attrib calls (V)\n");
    printf("  -reset_constants=[0,1]       1 = Reset constants before each draw call (C)\n");
    printf("  -reset_vertex_pointers=[0,1] 1 = Reset vertex pointers before each draw call (P)\n");
    printf("  -num_draw_calls_per_frame=[]     Defaults to 5000 ('+' or '-')\n");
    printf("  -num_frames=[]                   Number of frames to draw, defaults to 5000\n");
    printf("  -use_core_context=[0,1]          1 = Use core GL 3.2 context (Mac only)\n");
    printf("\nTo toggle an option at runtime, run with no parameters, and press the key in\n");
    printf("parentheses at the end of the option description (not all are available).\n");
    printf("\nPress 'Q' to quit.\n\n");
}

static void parseIntArgument(const char* inputStr, const char* compareStr, int *outVal)
{
    int len = strlen(compareStr);
    if (!strncasecmp(inputStr + 1, compareStr, len)) {
        /* Grab integer value:
         * (string start + string length + 2, one for the '-' and one for the '=') */
        const char *ptr = inputStr + len + 2;
        int tmp = atoi(ptr);
        printf("Setting %s to %d\n", compareStr, tmp);
        *outVal = tmp;
        gCommandLineOptionsValid = 1;
    }
}

/* Handles the given input key command.  Returns 1 if the keypress was
 * handled, and 0 if not. 'q' and 'Q' are handled from caller functions to
 * accommodate differences in runloops. */
int handleKeyPress(unsigned char key)
{
#define TOGGLE_OPTION(x) { \
    x = !x; \
    printf(#x " = %d\n", x); \
    break; }

    /* Ignore keyboard entry if command line argument was passed */
    if (gIgnoreKeyboard)
        return 0;

    /* Some commands are only handled when using OpenGL */
    switch(key) {
        case 'b':
        case 'B':
            if (!gUseD3D9 && gHaveBindableUniform && gUseGLSL) {
                TOGGLE_OPTION(gUseBindableUniform);
                /* The UBO and BU options are mutually exclusive */
                if (gUseBindableUniform)
                    gUseUniformBufferObject = 0;
            }
            break;
        case 'c':
        case 'C':
            TOGGLE_OPTION(gResetConstants)
        case 'g':
        case 'G':
            if (!gUseD3D9 && !gUseCoreContext)
                TOGGLE_OPTION(gUseGLSL)
            break;
        case 'm':
        case 'M':
#ifdef __APPLE__
            if (!gUseD3D9) {
                gUseMultiThreadedGL = !gUseMultiThreadedGL;
                printf("gUseMultiThreadedGL = %d\n", gUseMultiThreadedGL);
                if (gUseMultiThreadedGL)
                    CGLEnable(CGLGetCurrentContext(), kCGLCEMPEngine);
                else
                    CGLDisable(CGLGetCurrentContext(), kCGLCEMPEngine);
            }
#endif
            break;
        case 'o':
        case 'O':
            if (!gUseD3D9 && gHaveUniformBufferObject && gUseGLSL) {
                TOGGLE_OPTION(gUseUniformBufferObject);
                /* The UBO and BU options are mutually exclusive */
                if (gUseUniformBufferObject)
                    gUseBindableUniform = 0;
            }
            break;
        case 'p':
        case 'P':
            TOGGLE_OPTION(gResetVertexPointers)
        case 'v':
        case 'V':
            if (!gUseD3D9 && gHaveVAO && !gUseCoreContext)
                TOGGLE_OPTION(gUseVAO)
            break;
        case '+':
            gNumDrawCalls += 1000;
            printf("gNumDrawCalls = %d\n", gNumDrawCalls);
            break;
        case '-':
            if (gNumDrawCalls <= 1000)
                gNumDrawCalls = 1;
            else
                gNumDrawCalls -= 1000;
            printf("gNumDrawCalls = %d\n", gNumDrawCalls);
            break;
        case 'u':
        case 'U':
            /* This key handles both UBO and bindable_uniform updates since
             * they are mutually exclusive options. */
            if (gUseBindableUniform) {
                gBindableUpdateMethod = (gBindableUpdateMethod + 1) %
                                        BINDABLE_UPDATE_NUM_METHODS;

                toggleFlushBufferRange(gBindableUpdateMethod >= BINDABLE_UPDATE_FLUSH_BUFFER_RANGE);

                printf("gBindableUpdateMethod: %s\n", utilBindableString(gBindableUpdateMethod));

            } else if (gUseUniformBufferObject) {
                gUBOUpdateMethod = (gUBOUpdateMethod + 1) %
                                   UBO_UPDATE_NUM_METHODS;

                /* We need to have ARB_map_buffer_range for the last 2 update
                 * methods */
                if (gUBOUpdateMethod >= UBO_UPDATE_MAPBUFFER_RANGE && !gHaveMapBufferRange)
                    gUBOUpdateMethod = 0;

                printf("gUBOUpdateMethod: %s\n", utilUBOString(gUBOUpdateMethod));
            }
            break;
        default:
            return 0;
    }
    return 1;
#undef TOGGLE_OPTION
}

static void parseCommandLineOptions(int argc, char** argv)
{
    int i;

    for (i=1; i<argc; i++) {
        /* Command line argument must begin with '-' */
        if (argv[i][0] != '-')
            break;

        /* Ignore the process ID when launching from a Mac OS bundle */
        if (strstr(argv[i], "psn"))
            continue;

        parseIntArgument(argv[i], "use_d3d9", &gUseD3D9);
        parseIntArgument(argv[i], "use_multithreaded_gl", &gUseMultiThreadedGL);
        parseIntArgument(argv[i], "use_glsl", &gUseGLSL);
        parseIntArgument(argv[i], "use_bindable_uniform", &gUseBindableUniform);
        parseIntArgument(argv[i], "use_ubo", &gUseUniformBufferObject);
        parseIntArgument(argv[i], "bindable_update_method", &gBindableUpdateMethod);
        parseIntArgument(argv[i], "use_vao", &gUseVAO);
        parseIntArgument(argv[i], "reset_constants", &gResetConstants);
        parseIntArgument(argv[i], "reset_vertex_pointers", &gResetVertexPointers);
        parseIntArgument(argv[i], "num_draw_calls_per_frame", &gNumDrawCalls);
        parseIntArgument(argv[i], "num_frames", &gNumFrames);
        parseIntArgument(argv[i], "ignore_keyboard", &gIgnoreKeyboard);
        parseIntArgument(argv[i], "use_core_context", &gUseCoreContext);

        /* The UBO and BU options are mutually exclusive */
        if (gUseBindableUniform)
            gUseUniformBufferObject = 0;

        /* If we're using a core GL context, we have to use GLSL and VAO */
        if (gUseCoreContext) {
            gUseGLSL = 1;
            gUseVAO = 1;
        }

        /* If none of the valid options were found, display Help and quit */
        if (!gCommandLineOptionsValid) {
            displayHelp();
            killProcess(1);
        }
    }
}

int main(int argc, char** argv)
{
    parseCommandLineOptions(argc, argv);

    /* Determine which graphics API to use */
    if (gUseD3D9) {
        initD3D9();
        shutdownD3D9();
    } else {
        initOpenGL(argc, argv);
        shutdownOpenGL();
    }
    return EXIT_SUCCESS;
}
/******************************************************************************/
