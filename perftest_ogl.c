/* Performance test for low-level graphics functions
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */

/* This code is based in part on GL3 usage exampe code from

   filip.wanstrom _at_ gmail.com
   http://www.cfwdesign.se/gl3-tutorials/tutorial-2/
*/
#include "perftest.h"


/* VAO function prototypes */
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
PFNGLBINDVERTEXARRAYPROC p_glBindVertexArray = NULL;
PFNGLGENVERTEXARRAYSPROC p_glGenVertexArrays = NULL;

/* ARB_vertex_buffer_object prototypes */
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
PFNGLBINDBUFFERARBPROC p_glBindBuffer = NULL;
PFNGLGENBUFFERSARBPROC p_glGenBuffers = NULL;
PFNGLBUFFERSUBDATAARBPROC p_glBufferSubData = NULL;
PFNGLBUFFERDATAARBPROC p_glBufferData = NULL;

/* ARB_multitexture prototypes */
typedef void (APIENTRYP PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
PFNGLACTIVETEXTUREARBPROC p_glActiveTexture = NULL;

/* ARB_vertex/fragment_program prototypes */
typedef void (APIENTRY * PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (APIENTRY * PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (APIENTRY * PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint *params);
PFNGLBINDPROGRAMARBPROC p_glBindProgramARB = NULL;
PFNGLGENPROGRAMSARBPROC p_glGenProgramsARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC p_glProgramLocalParameter4fvARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC p_glProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMSTRINGARBPROC p_glProgramStringARB = NULL;
PFNGLGETPROGRAMIVARBPROC p_glGetProgramivARB = NULL;

/* ARB_shader_objects / GL 2.0 prototypes */
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
typedef GLint (APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint (APIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
PFNGLATTACHSHADERPROC p_glAttachShader = NULL;
PFNGLCOMPILESHADERPROC p_glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC p_glCreateProgram = NULL;
PFNGLCREATESHADERPROC p_glCreateShader = NULL;
PFNGLGETSHADERINFOLOGPROC p_glGetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC p_glGetShaderiv = NULL;
PFNGLSHADERSOURCEPROC p_glShaderSource = NULL;
PFNGLGETATTRIBLOCATIONPROC p_glGetAttribLocation = NULL;
PFNGLGETPROGRAMIVPROC p_glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC p_glGetProgramInfoLog = NULL;
PFNGLGETUNIFORMLOCATIONPROC p_glGetUniformLocation = NULL;
PFNGLLINKPROGRAMPROC p_glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC p_glUseProgram = NULL;
PFNGLVERTEXATTRIBPOINTERPROC p_glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC p_glEnableVertexAttribArray = NULL;
PFNGLUNIFORM1IPROC p_glUniform1i = NULL;
PFNGLUNIFORMMATRIX4FVPROC p_glUniformMatrix4fv = NULL;

/* FBO function prototypes */
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
PFNGLBINDRENDERBUFFEREXTPROC p_glBindRenderbuffer = NULL;
PFNGLGENRENDERBUFFERSEXTPROC p_glGenRenderbuffers = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC p_glBindFramebuffer = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC p_glGenFramebuffers = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC p_glCheckFramebufferStatus = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC p_glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC p_glFramebufferRenderbuffer = NULL;

/* GPU_program_parameters functions */
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETERS4FVEXTPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
typedef void (APIENTRYP PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
PFNGLPROGRAMENVPARAMETERS4FVEXTPROC p_glProgramEnvParameters4fvEXT = NULL;
PFNGLPROGRAMENVPARAMETERS4FVEXTPROC p_glProgramLocalParameters4fvEXT = NULL;

/* Windows VSync command */
typedef int (APIENTRYP PFNWGLSWAPINTERVALEXTPROC) (int interval);
PFNWGLSWAPINTERVALEXTPROC p_wglSwapIntervalEXT = NULL;


#define CHECK_GL_ERROR do { GLint err = glGetError(); if (err != GL_NO_ERROR) fprintf(stderr, "GL Error: %x line: %d\n", err, __LINE__ ); } while (0);


/* OpenGL Globals */
int     gHaveVAO = 0;
int     gWindowID = 0;
int     gHaveGPUProgramParameters = 0;
GLuint  gVBO;
GLuint  gEBO;
GLuint  gVAO;
GLuint  gVAO2;
GLuint  gFBO;
GLuint  gFBOColorTexture;
GLuint  gFBOColorTexture1;
GLuint  gFBODepthTexture;
GLuint  gDummyTex;
GLuint  gShader;
GLuint  gPostShader;
GLuint  gVertexProgram;
GLuint  gFragmentProgram;
GLuint  gQuadVBO;
GLuint  gQuadVAO;
GLuint  gPositionLoc = -1;
GLuint  gQuadPositionLoc = -1;
GLuint  gProjMatrixLoc = -1;
GLuint  gMvMatrixLoc = -1;
GLuint  gWindowWidth;
GLuint  gWindowHeight;
GLuint  gWindowHasBeenResized = 0;


/* Shader source */
/******************************************************************************/

const GLchar* basicVertexShaderSource =
    "attribute vec4  inPosition;\n"
    "uniform mat4    inProjectionMatrix;\n"
    "uniform mat4    inModelViewMatrix;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = inProjectionMatrix * inModelViewMatrix* inPosition;\n"
    "   gl_Position.z = 0.0;\n"
    "}\n";

const GLchar* basicFragmentShaderSource =
    "void main()\n"
    "{\n"
    "   gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    /* TODO: "   gl_FragData[1] = vec4(0.0, 0.0, 1.0, 1.0);\n" */
    "}\n";

/* Post processing shader */
const GLchar* postVertexShaderSource =
    "attribute vec4 inPosition;\n"
    "attribute vec4 inTexCoords;\n"
    "varying vec4 texCoords;\n"
    "uniform mat4    inProjectionMatrix;\n"
    "uniform mat4    inModelViewMatrix;\n"
    "void main()\n"
    "{\n"
    "   texCoords = inTexCoords;\n"
    "   gl_Position = inProjectionMatrix * inPosition;\n"
    "}\n";

const GLchar* postFragmentShaderSource =
    "uniform sampler2D tex;\n"
    "uniform sampler2D texDepth;\n"
    "varying vec4 texCoords;\n"
    "varying vec4 outColor;\n"
    "void main()\n"
    "{\n"
    "   vec4 texcoordsColor = vec4(texCoords.xyz, 1);\n"
    "   gl_FragColor = mix(vec4(texture2D(tex, texCoords.xy).rgb,\n"
    "                      1.0),texcoordsColor,0.5);\n"
    "}\n";


/* ARB_vertex_program version of the basic shaders.
 *  - Converted using "cgc -oglsl -profile arbvp1"
 *  - inPosition = vertex.attrib[0]
 *  - inProjectionMatrix program.local[0..3]
 *  - inModelViewMatrix program.local[4..7]
 */
const GLchar* basicVertexProgramSource =
    "!!ARBvp1.0\n"
    "PARAM c[9] = { program.local[0..8] };\n"
    "TEMP R0, R1, R2, R3, R4, R5;\n"
    "MOV R0, c[1];\n"
    "MOV R1, c[0];\n"
    "MUL R3, R0, c[4].y;\n"
    "MUL R2, R0, c[5].y;\n"
    "MAD R5, R1, c[4].x, R3;\n"
    "MAD R3, R1, c[5].x, R2;\n"
    "MOV R2, c[2];\n"
    "MAD R4, R2, c[5].z, R3;\n"
    "MOV R3, c[3];\n"
    "MAD R4, R3, c[5].w, R4;\n"
    "MAD R5, R2, c[4].z, R5;\n"
    "MUL R4, vertex.attrib[0].y, R4;\n"
    "MAD R5, R3, c[4].w, R5;\n"
    "MAD R5, vertex.attrib[0].x, R5, R4;\n"
    "MUL R4, R0, c[6].y;\n"
    "MUL R0, R0, c[7].y;\n"
    "MAD R0, R1, c[7].x, R0;\n"
    "MAD R4, R1, c[6].x, R4;\n"
    "MAD R1, R2, c[6].z, R4;\n"
    "MAD R0, R2, c[7].z, R0;\n"
    "MAD R1, R3, c[6].w, R1;\n"
    "MAD R1, vertex.attrib[0].z, R1, R5;\n"
    "MAD R0, R3, c[7].w, R0;\n"
    "MAD result.position, vertex.attrib[0].w, R0, R1;\n"
    "MOV result.position.z, {0.0, 0.0, 0.0, 0.0};\n"
    "END\n";

const GLchar* basicFragmentProgramSource =
    "!!ARBfp1.0\n"
    /* TODO "OPTION ARB_draw_buffers;\n" */
    "PARAM c[1] = { { 0, 1 } };\n"
    "MOV result.color, c[0].xyxy;\n"
    /* "MOV result.color[1], c[0].xxyy;\n" */
    "END\n";


/******************************************************************************/

static void utilPrintInfoLog(GLenum objType, GLuint obj)
{
    int infologLength = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    if ((objType == GL_VERTEX_SHADER) || (objType == GL_FRAGMENT_SHADER))
        p_glGetShaderiv(obj, GL_INFO_LOG_LENGTH, (GLint*)&infologLength);
    else
        p_glGetProgramiv(obj, GL_INFO_LOG_LENGTH, (GLint*)&infologLength);

    if (infologLength > 1) {

        infoLog = (GLchar*)malloc(infologLength);
        if (infoLog == NULL) {
            fprintf(stderr, "Allocation error\n");
            killProcess(1);
        }

        if ((objType == GL_VERTEX_SHADER) || (objType == GL_FRAGMENT_SHADER))
            p_glGetShaderInfoLog(obj, infologLength, (GLsizei*)&charsWritten, infoLog);
        else
            p_glGetProgramInfoLog(obj, infologLength, (GLsizei*)&charsWritten, infoLog);

        fprintf(stderr, "InfoLog:\n");
        fprintf(stderr, "%s\n", infoLog);
        free(infoLog);
    }
}

static GLuint utilCompileShader(GLenum shaderType, const GLchar* source)
{
    GLuint shader = p_glCreateShader(shaderType);
    p_glShaderSource(shader, 1, &source, NULL);
    p_glCompileShader(shader);
    GLint status;
    p_glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        fprintf(stderr, "shader compile error:\n%s ", source );
        utilPrintInfoLog(shaderType, shader);
        return 0;
    }
    return shader;
}

static GLuint utilCompileProgram(GLenum shaderType, const GLchar* source)
{
    GLuint shader;
    GLint errpos;
    const unsigned char *errstr;
    p_glGenProgramsARB(1, &shader);
    p_glBindProgramARB(shaderType, shader);
    CHECK_GL_ERROR;
    p_glProgramStringARB(shaderType, GL_PROGRAM_FORMAT_ASCII_ARB,
                         strlen(source), source);
    errstr = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

    CHECK_GL_ERROR;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);
    if( errpos < 0 ) {
        GLint isNative;
        p_glGetProgramivARB(shaderType, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
        if( isNative != 1 )
          printf("ARB: fragment program NOT NATIVE\n");

        glEnable(shaderType);
        CHECK_GL_ERROR;
    } else {

        printf("ARB: error string='%s'\n", errstr);
        if( errpos < strlen(source) ) {
          printf("ARB: syntax error in fragment program at offset=%d\n", errpos);
          printf("%s", (const char*)&source[errpos]);
        } else {
          printf("ARB: semantic error in fragment program errpos=%d\n", errpos);
        }
    }
    return shader;
}

/******************************************************************************/

static void utilCheckFramebufferStatus()
{
    GLenum status;
    status = (GLenum) p_glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
            break;
        default:
            printf("Unknown error %d\n", status);
    }
}

/******************************************************************************/

static void createDummyTex()
{
    unsigned char data[256*256*4];
    int x, y;
    for(y=0; y<256; y++)
    {
        for(x=0; x<256; x++)
        {
            int c = 127 + ((((y*8)/256)+((x*8)/256))%2) * 128;

            data[4*(y*256+x) + 0] = c;
            data[4*(y*256+x) + 1] = c;
            data[4*(y*256+x) + 2] = c;
            data[4*(y*256+x) + 3] = 255;
        }
    }

    glGenTextures(1, &gDummyTex);
    p_glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gDummyTex);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  256, 256, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);

}

/******************************************************************************/

static void createShaders()
{
    GLint status;

    /* The basic shader for the geometry */
    gShader = p_glCreateProgram();
    GLuint vertexShader= utilCompileShader(GL_VERTEX_SHADER,
                                           basicVertexShaderSource);
    GLuint fragmentShader = utilCompileShader(GL_FRAGMENT_SHADER,
                                              basicFragmentShaderSource);

    p_glAttachShader(gShader, vertexShader);
    p_glAttachShader(gShader, fragmentShader);

    p_glLinkProgram(gShader);

    p_glGetProgramiv(gShader, GL_LINK_STATUS, &status);
    if (!status) {
        printf("program link error\n");
        utilPrintInfoLog(0, gShader);
    }

    /* The post processing shader */
    gPostShader = p_glCreateProgram();
    vertexShader= utilCompileShader(GL_VERTEX_SHADER, postVertexShaderSource);
    fragmentShader = utilCompileShader(GL_FRAGMENT_SHADER,
                                       postFragmentShaderSource);

    p_glAttachShader(gPostShader, vertexShader);
    p_glAttachShader(gPostShader, fragmentShader);
    p_glLinkProgram(gPostShader);

    p_glGetProgramiv(gPostShader, GL_LINK_STATUS, &status);
    if (!status) {
        printf("program link error\n");
        utilPrintInfoLog(0, gPostShader);
    }
}

static void createARBPrograms()
{
    gVertexProgram = utilCompileProgram(GL_VERTEX_PROGRAM_ARB,
                                        basicVertexProgramSource);

    gFragmentProgram = utilCompileProgram(GL_FRAGMENT_PROGRAM_ARB,
                                          basicFragmentProgramSource);
}

/******************************************************************************/
/* TODO: Currently not used for anything useful */
static void createFBO()
{
    GLuint w = gBufferWidth;
    GLuint h = gBufferHeight;
    p_glGenFramebuffers(1, &gFBO);
    glGenTextures(1, &gFBOColorTexture);
    glGenTextures(1, &gFBOColorTexture1);
    p_glBindFramebuffer(GL_FRAMEBUFFER_EXT, gFBO);

    /* Depth */
    glBindTexture(GL_TEXTURE_2D, gFBODepthTexture);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_INT, NULL);

    p_glFramebufferTexture2D( GL_FRAMEBUFFER_EXT,
                              GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, gFBODepthTexture, 0);

    /* Color 0 */
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_INT, NULL);

    p_glFramebufferTexture2D( GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, gFBOColorTexture, 0);

    /* Color 1 */
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture1);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_INT, NULL);

    /* TODO: ARB_draw_buffers support:
     * p_glFramebufferTexture2D( GL_FRAMEBUFFER_EXT,
     *                           GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, gFBOColorTexture1, 0);
     */

    utilCheckFramebufferStatus();

    p_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

/******************************************************************************/

static void createVertexBuffers()
{
    int sizeInBytes = 0;

    /* Indexed icosaderon buffers */

    if (gHaveVAO) {
        p_glGenVertexArrays(1, &gVAO);
        p_glBindVertexArray(gVAO);
    }

    sizeInBytes = sizeof(float)*3*NUM_VERTICES;
    p_glGenBuffers(1, &gVBO);
    p_glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    p_glBufferData(GL_ARRAY_BUFFER, sizeInBytes, NULL, GL_STATIC_DRAW);
    p_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, icosahedronVertices);

    gPositionLoc = p_glGetAttribLocation(gShader, "inPosition");
    p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    p_glEnableVertexAttribArray(gPositionLoc);

    sizeInBytes = sizeof(unsigned short)*3*NUM_INDICES;
    p_glGenBuffers(1, &gEBO);
    p_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    p_glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes,NULL, GL_STATIC_DRAW);
    p_glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeInBytes, icosahedronIndices);

    if (gHaveVAO) {
        p_glBindVertexArray(0);

        p_glGenVertexArrays(1, &gVAO2);
        p_glBindVertexArray(gVAO2);
    }

    p_glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(36));
    p_glEnableVertexAttribArray(gPositionLoc);

    p_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);

    if (gHaveVAO) {
        p_glBindVertexArray(0);

        /* Quad buffers (Using tri strips) */
        p_glGenVertexArrays(1, &gQuadVAO);
        p_glBindVertexArray(gQuadVAO);
    }

    sizeInBytes = sizeof(float)*3*4;
    p_glGenBuffers(1, &gQuadVBO);
    p_glBindBuffer(GL_ARRAY_BUFFER, gQuadVBO);
    p_glBufferData(GL_ARRAY_BUFFER, sizeInBytes*2, NULL, GL_STATIC_DRAW);
    p_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, quadVertices);
    p_glBufferSubData(GL_ARRAY_BUFFER, sizeInBytes, sizeInBytes, quadTexcoords);

    gQuadPositionLoc = p_glGetAttribLocation(gPostShader, "inPosition");
    p_glVertexAttribPointer(gQuadPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    p_glEnableVertexAttribArray(gQuadPositionLoc);

    GLuint texcoordLoc = p_glGetAttribLocation(gPostShader, "inTexCoords");
    p_glVertexAttribPointer(texcoordLoc, 3, GL_FLOAT, GL_FALSE, 0,
                            BUFFER_OFFSET(48));
    p_glEnableVertexAttribArray(texcoordLoc);

    GLuint texLoc  = p_glGetAttribLocation(gPostShader, "tex");
    p_glUniform1i(texLoc, 0);

    GLuint texLoc2  = p_glGetAttribLocation(gPostShader, "texDepth");
    p_glUniform1i(texLoc2, 1);

    p_glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture);

    if (gHaveVAO)
        p_glBindVertexArray(0);
}

/******************************************************************************/

static inline void update_modelview_constants(const float* params)
{
    if (gUseGLSL)
        p_glUniformMatrix4fv(gMvMatrixLoc, 1, GL_FALSE, params);
    else {
        if (gHaveGPUProgramParameters)
            p_glProgramLocalParameters4fvEXT(GL_VERTEX_PROGRAM_ARB, 4, 4, params);
        else {
            int i;
            for (i=0; i<4; i++)
                p_glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, i, params);
        }
    }
}

static void drawScene()
{
    int i;

    if (gUseGLSL) {
        p_glUseProgram(gShader);
        glDisable(GL_VERTEX_PROGRAM_ARB);
        p_glBindProgramARB(GL_VERTEX_PROGRAM_ARB, gVertexProgram);
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
        p_glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gFragmentProgram);

        /* Upload new projection matrix */
        if (gProjMatrixLoc == -1)
            gProjMatrixLoc = p_glGetUniformLocation(gShader, "inProjectionMatrix");
        p_glUniformMatrix4fv(gProjMatrixLoc, 1, GL_FALSE, gProjectionMatrixf);

        /* Grab modelview uniform pointer if needed */
        if (gMvMatrixLoc == -1)
            gMvMatrixLoc = p_glGetUniformLocation(gShader, "inModelViewMatrix");

    } else {
        p_glUseProgram(0);
        glEnable(GL_VERTEX_PROGRAM_ARB);
        p_glBindProgramARB(GL_VERTEX_PROGRAM_ARB, gVertexProgram);
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
        p_glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gFragmentProgram);

        /* Upload new projection matrix */
        if (gHaveGPUProgramParameters)
            p_glProgramLocalParameters4fvEXT(GL_VERTEX_PROGRAM_ARB, 0, 4, gProjectionMatrixf);
        else {
            int i;
            for (i=0; i<4; i++)
                p_glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, i, &gProjectionMatrixf[i*4]);
        }
    }

    if (gUseVAO) {
        /* Path 1  - VAO */
        p_glBindVertexArray(gVAO);
        for (i = 0; i < gNumDrawCalls; i++)
        {
            if (gResetVertexPointers)
                p_glBindVertexArray(gVAO);

            if (gResetConstants || i == 0)
                update_modelview_constants(gModelViewMatrixf);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

            if (gResetVertexPointers)
                p_glBindVertexArray(gVAO2);

            if (gResetConstants || i == 0)
                update_modelview_constants(gModelViewMatrixf2);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT,BUFFER_OFFSET(0));
        }

    } else {
        /* Path 2  - no VAO */
        if (gHaveVAO)
            p_glBindVertexArray(0);
        p_glBindBuffer(GL_ARRAY_BUFFER, gVBO);
        p_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
        p_glEnableVertexAttribArray(gPositionLoc);
        p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (i = 0; i < gNumDrawCalls; i++)
        {
            if (gResetVertexPointers)
                p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

            if (gResetConstants || i == 0)
                update_modelview_constants(gModelViewMatrixf);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

            if (gResetVertexPointers)
                p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(36));

            if (gResetConstants || i == 0)
                update_modelview_constants(gModelViewMatrixf2);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT,BUFFER_OFFSET(0));
        }
    }
}

/******************************************************************************/

static void cbKeyboard(unsigned char key, int x, int y)
{
    /* Special handling for quitting app early */
    switch(key) {
        case 'q':
        case 'Q':
            /* Quit */
            killProcess(0);
    }

    /* Ignore keyboard entry if command line argument was passed */
    if (gIgnoreKeyboard)
        return;

    handleKeyPress(key);
}

static void cbDisplay(void)
{
    update();

    p_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    utilReshapeOrtho(gWindowWidth, gWindowHeight);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene();

    glutSwapBuffers();
}

/******************************************************************************/

static void cbReshape(int width, int height)
{
    gWindowWidth = width;
    gWindowHeight = height;
    gWindowHasBeenResized = 1;

}

/******************************************************************************/

static void cbIdle(void)
{
    glutPostRedisplay();
}

/******************************************************************************/

static void* getGLProcAddress(const char *fncString)
{
#ifdef __APPLE__
    static CFBundleRef bundle = NULL;
    static CFURLRef bundleURL = NULL;
    void *function = NULL;

    /* Cache the bundleURLRef */
    if (!bundleURL)
        bundleURL = CFURLCreateWithFileSystemPath (kCFAllocatorDefault,
                                                   CFSTR("/System/Library/Frameworks/OpenGL.framework"), kCFURLPOSIXPathStyle, true);
    if (!bundle)
        bundle = CFBundleCreate (kCFAllocatorDefault, bundleURL);

    CFStringRef functionName = CFStringCreateWithCString(kCFAllocatorDefault, fncString, kCFStringEncodingASCII);

    if (bundle)
        function = CFBundleGetFunctionPointerForName (bundle, functionName);

    CFRelease ( functionName );

    return function;
#elif defined(__WIN32__)
    return (void*)wglGetProcAddress(fncString);
#elif defined(linux)
    return (void*)glXGetProcAddressARB(fncString);
#endif
}

static void checkGLExtensions()
{
    const char* ver = (const char*)glGetString(GL_VERSION);
    char major=0, minor=0, subminor=0;

    /* Parse GL version */
    if (ver && *ver) {
        /* Skip any leading text and grab major version */
        while (!(*ver) >= '0' && *ver <= '9')
            ver++;
        major = atoi(ver);
        /* Grab minor version */
        while ((*ver != 0) && (*ver != ' ') && (*ver != '.'))
            ver++;
        if (*ver == '.') {
            ver++;
            minor = atoi(ver);
            /* Grab subminor version */
            while ((*ver != 0) && (*ver != ' ') && (*ver != '.'))
                ver++;
            if (*ver == '.') {
                ver++;
                subminor = atoi(ver);
            }
        }
    }

#define GET_PROC_ADDRESS(x, y) \
    x = (void*)getGLProcAddress(#y); \
    if (!x) { \
        fprintf(stderr, "Cannot load " #x "! Bailing out!\n"); \
        killProcess(1); \
    }

    /* Grab ARB_multitexture extension pointers */
    if (glutExtensionSupported("GL_ARB_multitexture")) {
        GET_PROC_ADDRESS(p_glActiveTexture, glActiveTextureARB);
    } else if ((major > 1) || ((major == 1) && (minor >= 3))) {
        GET_PROC_ADDRESS(p_glActiveTexture, glActiveTexture);
    } else {
        fprintf(stderr, "ERROR: ARB_multitexture not supported!\n");
        killProcess(1);
    }

    /* Grab ARB_shader_objects and GL 2.0 pointers */
    if (glutExtensionSupported("GL_ARB_shader_objects")) {
        GET_PROC_ADDRESS(p_glAttachShader, glAttachObjectARB);
        GET_PROC_ADDRESS(p_glCompileShader, glCompileShaderARB);
        GET_PROC_ADDRESS(p_glCreateProgram, glCreateProgramObjectARB);
        GET_PROC_ADDRESS(p_glCreateShader, glCreateShaderObjectARB);
        GET_PROC_ADDRESS(p_glGetShaderInfoLog, glGetInfoLogARB);
        GET_PROC_ADDRESS(p_glGetShaderiv, glGetObjectParameterivARB);
        GET_PROC_ADDRESS(p_glShaderSource, glShaderSourceARB);
        GET_PROC_ADDRESS(p_glGetProgramiv, glGetObjectParameterivARB);
        GET_PROC_ADDRESS(p_glGetProgramInfoLog, glGetInfoLogARB);
        GET_PROC_ADDRESS(p_glGetUniformLocation, glGetUniformLocationARB);
        GET_PROC_ADDRESS(p_glLinkProgram, glLinkProgramARB);
        GET_PROC_ADDRESS(p_glUseProgram, glUseProgramObjectARB);
        GET_PROC_ADDRESS(p_glUniform1i, glUniform1iARB);
        GET_PROC_ADDRESS(p_glUniformMatrix4fv, glUniformMatrix4fvARB);

    /* TODO: Grab 2.0 core functions if extension not present. */
    } else {
        fprintf(stderr, "ERROR: ARB_shader_objects not supported\n");
        killProcess(1);
    }

    if (glutExtensionSupported("GL_ARB_vertex_shader")) {
        GET_PROC_ADDRESS(p_glEnableVertexAttribArray, glEnableVertexAttribArrayARB);
        GET_PROC_ADDRESS(p_glGetAttribLocation, glGetAttribLocationARB);
        GET_PROC_ADDRESS(p_glVertexAttribPointer, glVertexAttribPointerARB);
    } else {
        /* TODO: Make this app run w/o GLSL support */
        fprintf(stderr, "ERROR: ARB_vertex_shader not supported\n");
        killProcess(1);
    }

    if (!glutExtensionSupported("GL_ARB_fragment_shader")) {
        fprintf(stderr, "ERROR: ARB_fragment_shader not supported\n");
        killProcess(1);
    }

    if (glutExtensionSupported("GL_ARB_vertex_program") &&
        glutExtensionSupported("GL_ARB_fragment_program")) {
        GET_PROC_ADDRESS(p_glBindProgramARB, glBindProgramARB);
        GET_PROC_ADDRESS(p_glGenProgramsARB, glGenProgramsARB);
        GET_PROC_ADDRESS(p_glProgramLocalParameter4fvARB, glProgramLocalParameter4fvARB);
        GET_PROC_ADDRESS(p_glProgramEnvParameter4fvARB, glProgramEnvParameter4fvARB);
        GET_PROC_ADDRESS(p_glProgramStringARB, glProgramStringARB);
        GET_PROC_ADDRESS(p_glGetProgramivARB, glGetProgramivARB);
    } else {
        /* TODO: Make this app run w/o ARB_vp/fp support */
        fprintf(stderr, "ERROR: ARB_vertex/fragment_program not supported\n");
        killProcess(1);
    }

    /* Grab FBO extension pointers */
    if (glutExtensionSupported("GL_ARB_framebuffer_object")) {
        GET_PROC_ADDRESS(p_glBindRenderbuffer, glBindRenderbuffer);
        GET_PROC_ADDRESS(p_glGenRenderbuffers, glGenRenderbuffers);
        GET_PROC_ADDRESS(p_glBindFramebuffer, glBindFramebuffer);
        GET_PROC_ADDRESS(p_glGenFramebuffers, glGenFramebuffers);
        GET_PROC_ADDRESS(p_glCheckFramebufferStatus, glCheckFramebufferStatus);
        GET_PROC_ADDRESS(p_glFramebufferTexture2D, glFramebufferTexture2D);
        GET_PROC_ADDRESS(p_glFramebufferRenderbuffer, glFramebufferRenderbuffer);
    } else if (glutExtensionSupported("GL_EXT_framebuffer_object")) {
        GET_PROC_ADDRESS(p_glBindRenderbuffer, glBindRenderbufferEXT);
        GET_PROC_ADDRESS(p_glGenRenderbuffers, glGenRenderbuffersEXT);
        GET_PROC_ADDRESS(p_glBindFramebuffer, glBindFramebufferEXT);
        GET_PROC_ADDRESS(p_glGenFramebuffers, glGenFramebuffersEXT);
        GET_PROC_ADDRESS(p_glCheckFramebufferStatus, glCheckFramebufferStatusEXT);
        GET_PROC_ADDRESS(p_glFramebufferTexture2D, glFramebufferTexture2DEXT);
        GET_PROC_ADDRESS(p_glFramebufferRenderbuffer, glFramebufferRenderbufferEXT);
    } else {
        /* FBO not supported - bail */
        fprintf(stderr, "ERROR: FBO not supported\n");
        killProcess(1);
    }

    /* Grab VBO extension pointers */
    if (glutExtensionSupported("GL_ARB_vertex_buffer_object")) {
        GET_PROC_ADDRESS(p_glBindBuffer, glBindBufferARB);
        GET_PROC_ADDRESS(p_glGenBuffers, glGenBuffersARB);
        GET_PROC_ADDRESS(p_glBufferSubData, glBufferSubDataARB);
        GET_PROC_ADDRESS(p_glBufferData, glBufferDataARB);

    /* TODO: Grab core 2.0 pointers if available w/o extension */
    } else {
        fprintf(stderr, "ERROR: VBO not supported\n");
        killProcess(1);
    }

    /* Grab VAO extension pointers */
    if (glutExtensionSupported("GL_ARB_vertex_array_object")) {
        GET_PROC_ADDRESS(p_glGenVertexArrays, glGenVertexArrays);
        GET_PROC_ADDRESS(p_glBindVertexArray, glBindVertexArray);
        gHaveVAO = 1;
    } else if (glutExtensionSupported("GL_APPLE_vertex_array_object")) {
        GET_PROC_ADDRESS(p_glGenVertexArrays, glGenVertexArraysAPPLE);
        GET_PROC_ADDRESS(p_glBindVertexArray, glBindVertexArrayAPPLE);
        gHaveVAO = 1;
    } else if (gUseVAO) {
        /* VAO not supported - force it off */
        fprintf(stderr, "WARNING: VAO not supported\n");
        gUseVAO = 0;
    }

    /* Setting more than 1 vec4 at a time is preferred, so check for this
     * extension.  */
    if (glutExtensionSupported("GL_EXT_gpu_program_parameters")) {
        GET_PROC_ADDRESS(p_glProgramEnvParameters4fvEXT, glProgramEnvParameters4fvEXT);
        GET_PROC_ADDRESS(p_glProgramLocalParameters4fvEXT, glProgramLocalParameters4fvEXT);
        gHaveGPUProgramParameters  = 1;
    }

    /* Disable VSync on Windows if the wgl functions is available */
#ifdef __WIN32__
    GET_PROC_ADDRESS(p_wglSwapIntervalEXT, wglSwapIntervalEXT);
    if (p_wglSwapIntervalEXT) {
        if (!p_wglSwapIntervalEXT(0)) {
            fprintf(stderr, "WARNING: Cannot disable VSync!\n");
        }
    }
#endif

#ifdef __APPLE__
    /* Enable multi-threaded GL if enabled */
    if (gUseMultiThreadedGL)
        CGLEnable( CGLGetCurrentContext(), kCGLCEMPEngine);
#endif

#undef GET_PROC_ADDRESS
}

void setViewportOGL(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}

void shutdownOpenGL()
{
    if (gWindowID)
        glutDestroyWindow(gWindowID);
}

void initOpenGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

    gWindowID = glutCreateWindow("GL3 Tutorial 2");
    checkGLExtensions();

    createDummyTex();
    createFBO();
    createShaders();
    createARBPrograms();
    createVertexBuffers();

#ifndef GL_MULTISAMPLE_ARB
#define GL_MULTISAMPLE_ARB 0x809D
#endif
#ifndef GL_CLIP_VOLUME_CLIPPING_HINT_EXT
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT 0x80F0
#endif

    /* Set to D3D defaults */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_MULTISAMPLE_ARB);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glDisable(GL_DITHER);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);

    glutDisplayFunc(cbDisplay);
    glutKeyboardFunc(cbKeyboard);
    glutReshapeFunc(cbReshape);
    glutIdleFunc(cbIdle);

    glutMainLoop();
}

