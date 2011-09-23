/* Performance test for low-level graphics functions
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */

/* This code is based in part on GL3 usage exampe code from

   filip.wanstrom _at_ gmail.com
   http://www.cfwdesign.se/gl3-tutorials/tutorial-2/
*/
#include "perftest.h"

#ifdef __MINGW32__
/* Not all versions of MinGW define GLintptr yet */
typedef long GLintptr;
typedef long GLsizeiptr;
#endif

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

/* VAO function prototypes */
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
PFNGLBINDVERTEXARRAYPROC p_glBindVertexArray = NULL;
PFNGLGENVERTEXARRAYSPROC p_glGenVertexArrays = NULL;

/* APPLE_flush_buffer_range prototypes */
typedef void (APIENTRYP PFNGLBUFFERPARAMETERIAPPLEPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC) (GLenum target, GLintptr offset, GLsizeiptr size);
PFNGLBUFFERPARAMETERIAPPLEPROC p_glBufferParameteriAPPLE = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC p_glFlushMappedBufferRangeAPPLE = NULL;
#ifndef GL_APPLE_flush_buffer_range
#define GL_BUFFER_SERIALIZED_MODIFY_APPLE 0x8A12
#define GL_BUFFER_FLUSHING_UNMAP_APPLE    0x8A13
#endif

/* ARB_map_buffer_range */
typedef void *(APIENTRY * PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRY * PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);
PFNGLMAPBUFFERRANGEPROC p_glMapBufferRange = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC p_glFlushMappedBufferRange = NULL;
#ifndef GL_ARB_map_buffer_range
#define GL_MAP_READ_BIT                     0x0001
#define GL_MAP_WRITE_BIT                    0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT         0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT        0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT           0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT           0x0020
#endif

/* ARB_vertex_buffer_object prototypes */
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void * (APIENTRY * PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERARBPROC) (GLenum target);
PFNGLBINDBUFFERARBPROC p_glBindBuffer = NULL;
PFNGLGENBUFFERSARBPROC p_glGenBuffers = NULL;
PFNGLBUFFERSUBDATAARBPROC p_glBufferSubData = NULL;
PFNGLBUFFERDATAARBPROC p_glBufferData = NULL;
PFNGLMAPBUFFERARBPROC p_glMapBuffer = NULL;
PFNGLUNMAPBUFFERARBPROC p_glUnmapBuffer = NULL;

/* EXT_bindable_uniform */
typedef void (APIENTRY * PFNGLUNIFORMBUFFEREXTPROC) (GLuint program, GLint location, GLuint buffer);
typedef GLint (APIENTRY * PFNGLGETUNIFORMBUFFERSIZEEXTPROC) (GLuint program, GLint location);
typedef GLintptr (APIENTRY * PFNGLGETUNIFORMOFFSETEXTPROC) (GLuint program, GLint location);
PFNGLUNIFORMBUFFEREXTPROC p_glUniformBufferEXT = NULL;
PFNGLGETUNIFORMBUFFERSIZEEXTPROC p_glGetUniformBufferSizeEXT = NULL;
PFNGLGETUNIFORMOFFSETEXTPROC p_glGetUniformOffsetEXT = NULL;
#ifndef GL_EXT_bindable_uniform
#define GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT         0x8DE2
#define GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT       0x8DE3
#define GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT       0x8DE4
#define GL_MAX_BINDABLE_UNIFORM_SIZE_EXT            0x8DED
#define GL_UNIFORM_BUFFER_EXT                       0x8DEE
#define GL_UNIFORM_BUFFER_BINDING_EXT               0x8DEF
#endif

/* ARB_uniform_buffer_object */
typedef void (APIENTRYP PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar* *uniformNames, GLuint *uniformIndices);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMNAMEPROC) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
typedef GLuint (APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
typedef void (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void (APIENTRY * PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRY * PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
PFNGLGETUNIFORMINDICESPROC p_glGetUniformIndices = NULL;
PFNGLGETACTIVEUNIFORMSIVPROC p_glGetActiveUniformsiv = NULL;
PFNGLGETACTIVEUNIFORMNAMEPROC p_glGetActiveUniformName = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC p_glGetUniformBlockIndex = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC p_glGetActiveUniformBlockiv = NULL;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC p_glGetActiveUniformBlockName = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC p_glUniformBlockBinding = NULL;
PFNGLBINDBUFFERRANGEPROC p_glBindBufferRange = NULL;
PFNGLBINDBUFFERBASEPROC p_glBindBufferBase = NULL;
#ifndef GL_ARB_uniform_buffer_object
#define GL_UNIFORM_BUFFER                               0x8A11
#define GL_UNIFORM_BUFFER_BINDING                       0x8A28
#define GL_UNIFORM_BUFFER_START                         0x8A29
#define GL_UNIFORM_BUFFER_SIZE                          0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                    0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS                  0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                  0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                  0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                  0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                       0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS       0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS     0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS     0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT              0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH         0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS                        0x8A36
#define GL_UNIFORM_TYPE                                 0x8A37
#define GL_UNIFORM_SIZE                                 0x8A38
#define GL_UNIFORM_NAME_LENGTH                          0x8A39
#define GL_UNIFORM_BLOCK_INDEX                          0x8A3A
#define GL_UNIFORM_OFFSET                               0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                         0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                        0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                         0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                        0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                      0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH                    0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES         0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER    0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER  0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER  0x8A46
#define GL_INVALID_INDEX                                0xFFFFFFFFu
#endif

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


#define BUFFER_USAGE_FLAG GL_STATIC_DRAW
#define UBO_USAGE_FLAG    GL_STREAM_DRAW


/* OpenGL Globals */
int     gHaveVAO = 0;
int     gHaveARBPrograms = 0;
int     gWindowID = 0;
int     gHaveGPUProgramParameters = 0;
int     gHaveFlushBufferRange = 0;
GLuint  gVBO;
GLuint  gEBO;
GLuint  gBindableBuffer = 0;
GLuint  gUBOBuffer = 0;
GLuint  gUBOSize = 0;
GLint   gUBOOffset = 0;
GLint   gUBOOffsetIncrement = 0;
GLuint  gUBOBlockIndex = 0;
GLuint  gVAO;
GLuint  gVAO2;
GLuint  gFBO;
int     gHaveMapBufferRange = 0;
GLuint  gFBOColorTexture;
GLuint  gFBOColorTexture1;
GLuint  gFBODepthTexture;
GLuint  gDummyTex;
GLuint  gShader;
GLuint  gShaderBindable;
GLuint  gShaderUBO;
GLuint  gPostShader;
GLuint  gVertexProgram;
GLuint  gFragmentProgram;
GLuint  gQuadVBO;
GLuint  gQuadVAO;
GLuint  gPositionLoc = -1;
GLuint  gQuadPositionLoc = -1;
GLuint  gProjMatrixLoc = -1;
GLuint  gMvMatrixLoc = -1;
GLuint  gProjMatrixBindableLoc = -1;
GLuint  gMvMatrixBindableLoc = -1;
GLuint  gProjMatrixUBOLoc = -1;
GLuint  gWindowWidth;
GLuint  gWindowHeight;
GLuint  gWindowHasBeenResized = 0;
char    gGLMajor = 0;
char    gGLMinor = 0;
char    gGLSubminor = 0;

/* Shader source */
/******************************************************************************/

const GLchar* basicVertexShaderSourceBindable =
    "#extension GL_EXT_bindable_uniform : enable\n"
    "#if __VERSION__ >= 140\n"
    "in vec4                inPosition;\n"
    "#else\n"
    "attribute vec4         inPosition;\n"
    "#endif\n"
    "uniform mat4           inProjectionMatrix;\n"
    "bindable uniform mat4  inModelViewMatrix;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = inProjectionMatrix * inModelViewMatrix* inPosition;\n"
    "   gl_Position.z = 0.0;\n"
    "}\n";

const GLchar* basicVertexShaderSourceUBO =
    "#extension GL_ARB_uniform_buffer_object : enable\n"
    "#if __VERSION__ >= 140\n"
    "in vec4                inPosition;\n"
    "#else\n"
    "attribute vec4         inPosition;\n"
    "#endif\n"
    "uniform mat4           inProjectionMatrix;\n"
    "layout(std140) uniform UBO\n"
    "    { mat4 inModelViewMatrix; };\n"
    "void main()\n"
    "{\n"
    "   gl_Position = inProjectionMatrix * inModelViewMatrix * inPosition;\n"
    "   gl_Position.z = 0.0;\n"
    "}\n";

const GLchar* basicVertexShaderSource =
    "#if __VERSION__ >= 140\n"
    "in vec4         inPosition;\n"
    "#else\n"
    "attribute vec4  inPosition;\n"
    "#endif\n"
    "uniform mat4    inProjectionMatrix;\n"
    "uniform mat4    inModelViewMatrix;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = inProjectionMatrix * inModelViewMatrix* inPosition;\n"
    "   gl_Position.z = 0.0;\n"
    "}\n";

const GLchar* basicFragmentShaderSource =
    "#if __VERSION__ >= 140\n"
    "  out vec4 fragColor;\n"
    "#else\n"
    "  #define fragColor gl_FragColor\n"
    "#endif\n"
    "void main()\n"
    "{\n"
    "   fragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    /* TODO: "   gl_FragData[1] = vec4(0.0, 0.0, 1.0, 1.0);\n" */
    "}\n";

/* Post processing shader */
const GLchar* postVertexShaderSource =
    "#if __VERSION__ >= 140\n"
    "in vec4 inPosition;\n"
    "in vec4 inTexCoords;\n"
    "out vec4 texCoords;\n"
    "#else\n"
    "attribute vec4 inPosition;\n"
    "attribute vec4 inTexCoords;\n"
    "varying vec4 texCoords;\n"
    "#endif\n"
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
    "#if __VERSION__ >= 140\n"
    "in vec4 texCoords;\n"
    "out vec4 fragColor;\n"
    "#else\n"
    "varying vec4 texCoords;\n"
    "#define fragColor gl_FragColor\n"
    "#endif\n"
    "void main()\n"
    "{\n"
    "   vec4 texcoordsColor = vec4(texCoords.xyz, 1);\n"
    "   #if __VERSION__ >= 140\n"
    "     vec4 texColor = texture(tex, texCoords.xy);\n"
    "   #else\n"
    "     vec4 texColor = texture2D(tex, texCoords.xy);\n"
    "   #endif\n"
    "   fragColor = mix(vec4(texColor.rgb, 1.0), texcoordsColor, 0.5);\n"
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
    GLchar *modified_source = malloc(strlen(source) + 60);

    if (!modified_source) {
        fprintf(stderr, "%s:%u, Out of memory!\n", __FILE__, __LINE__);
        return 0;
    }
    if (gUseCoreContext) {
        /* For core context shaders, we'll use GLSL 1.40 across the board.
         * We can modify this later when necessary. */
        sprintf(modified_source, "#version 140\n");
    } else {
        sprintf(modified_source, "#ifndef __VERSION__\n#define __VERSION__ 100\n#endif\n");
    }
    strcat(modified_source, source);

    p_glShaderSource(shader, 1, (const GLchar**)&modified_source, NULL);
    p_glCompileShader(shader);
    GLint status;
    p_glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        fprintf(stderr, "shader compile error:\n%s ", modified_source );
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

static void utilGetMatrixLocations(GLuint prog,
                                   GLuint *projMatrix,
                                   GLuint *mvMatrix)
{
    *projMatrix = p_glGetUniformLocation(prog, "inProjectionMatrix");
    if (mvMatrix != NULL)
        *mvMatrix = p_glGetUniformLocation(prog, "inModelViewMatrix");
}

static void utilCreateBindableUniformBuffer()
{
    GLuint bufSize;

    p_glGenBuffers(1, &gBindableBuffer);
    if (!gBindableBuffer) {
        fprintf(stderr, "Unable to create bindable uniform VBO\n");
        gHaveBindableUniform = 0;
        gUseBindableUniform = 0;
        return;
    }
    p_glBindBuffer(GL_UNIFORM_BUFFER_EXT, gBindableBuffer);
    p_glBufferData(GL_UNIFORM_BUFFER_EXT,
                   sizeof(gModelViewMatrixf),
                   NULL,
                   BUFFER_USAGE_FLAG);

    toggleFlushBufferRange(gBindableUpdateMethod >= BINDABLE_UPDATE_FLUSH_BUFFER_RANGE);

    /* Sanity size check */
    bufSize = p_glGetUniformBufferSizeEXT(gShaderBindable, gMvMatrixBindableLoc);
    if (bufSize != sizeof(gModelViewMatrixf)) {
        fprintf(stderr, "bufSize (%d) != matrix size (%zd)\n",
                bufSize,
                sizeof(gModelViewMatrixf));
        gHaveBindableUniform = 0;
        gUseBindableUniform = 0;
        return;
    }
    /* Bind the buffer to the bindable modelview matrix */
    p_glUniformBufferEXT(gShaderBindable, gMvMatrixBindableLoc, gBindableBuffer);
    p_glBufferSubData(GL_UNIFORM_BUFFER_EXT, 0, sizeof(gModelViewMatrixf), gModelViewMatrixf);
}

/* Create the buffer object for ARB_uniform_buffer_object.
 * Create the buffer with size gUBOSize and update it using a ring-buffer
 * approach. */
static void utilCreateUniformBufferObject()
{
    GLint alignSize, blockSize, matSize, offset;
    GLuint index;
    /* WTF? */
#ifdef __APPLE__
    const GLchar *varName = "UBO.inModelViewMatrix";
#else
    const GLchar *varName = "inModelViewMatrix";
#endif

    /* Sanity checks - make sure the offset is 0, the blocksize is the size of
     * the modelview matrix, and the matrix size is 1 element. */
    gUBOBlockIndex = p_glGetUniformBlockIndex(gShaderUBO, "UBO");
    p_glGetActiveUniformBlockiv(gShaderUBO, gUBOBlockIndex,
                                GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    p_glGetUniformIndices(gShaderUBO, 1, &varName, &index);
    p_glGetActiveUniformsiv(gShaderUBO, 1, &index, GL_UNIFORM_OFFSET, &offset);
    p_glGetActiveUniformsiv(gShaderUBO, 1, &index, GL_UNIFORM_SIZE, &matSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignSize);
    CHECK_GL_ERROR;

    if ((blockSize != sizeof(gModelViewMatrixf)) ||
        (matSize != 1) ||
        (offset != 0)) {
        fprintf(stderr, "UBO sanity checks fail: blockSize=%d, matSize=%d,"
                        "gUBOBlockIndex=%d, index=%d, offset=%d\n",
                blockSize, matSize, gUBOBlockIndex, index, offset);
        gHaveUniformBufferObject = 0;
        gUseUniformBufferObject = 0;
        return;
    }

    /* Generate the backing store buffer */
    p_glGenBuffers(1, &gUBOBuffer);
    if (!gUBOBuffer) {
        fprintf(stderr, "Unable to create ARB_uniform_buffer_object VBO\n");
        gHaveUniformBufferObject = 0;
        gUseUniformBufferObject = 0;
        return;
    }

    /* NOTE: GL_UNIFORM_BUFFER is not the same as GL_UNIFORM_BUFFER_EXT! */
    p_glBindBuffer(GL_UNIFORM_BUFFER, gUBOBuffer);

    /* Allocate our buffer space as 500 * the minimum alignment/block size */
    gUBOOffsetIncrement = max(alignSize, sizeof(gModelViewMatrixf));
    gUBOSize = gUBOOffsetIncrement * 500;

    p_glBufferData(GL_UNIFORM_BUFFER,
                   gUBOSize,
                   NULL,
                   UBO_USAGE_FLAG);

    /* Upload the initial data */
    p_glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(gModelViewMatrixf), gModelViewMatrixf);

    /* Bind the buffer to the UBO block index. */
    p_glBindBufferRange(GL_UNIFORM_BUFFER, gUBOBlockIndex, gUBOBuffer, 0, sizeof(gModelViewMatrixf));

    CHECK_GL_ERROR;
}

static void createShaders()
{
    GLint status;

    /* The basic shader for the geometry */
    gShader = p_glCreateProgram();
    GLuint vertexShaderBindable = 0;
    GLuint vertexShaderUBO = 0;
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
    utilGetMatrixLocations(gShader, &gProjMatrixLoc, &gMvMatrixLoc);

    /* Vertex shader that uses EXT_bindable_uniform */
    if (gHaveBindableUniform) {
        gShaderBindable = p_glCreateProgram();
        vertexShaderBindable = utilCompileShader(GL_VERTEX_SHADER,
                                                 basicVertexShaderSourceBindable);

        /* Use the same fragment shader */
        p_glAttachShader(gShaderBindable, vertexShaderBindable);
        p_glAttachShader(gShaderBindable, fragmentShader);

        p_glLinkProgram(gShaderBindable);

        p_glGetProgramiv(gShaderBindable, GL_LINK_STATUS, &status);
        if (!status) {
            printf("program link error from bindable shader\n");
            utilPrintInfoLog(0, gShaderBindable);
        }
        utilGetMatrixLocations(gShaderBindable,
                               &gProjMatrixBindableLoc,
                               &gMvMatrixBindableLoc);
        utilCreateBindableUniformBuffer();
    }

    /* Vertex shader that uses ARB_uniform_buffer_object */
    if (gHaveUniformBufferObject) {
        gShaderUBO = p_glCreateProgram();
        vertexShaderUBO = utilCompileShader(GL_VERTEX_SHADER,
                                            basicVertexShaderSourceUBO);

        /* Use the same fragment shader */
        p_glAttachShader(gShaderUBO, vertexShaderUBO);
        p_glAttachShader(gShaderUBO, fragmentShader);

        p_glLinkProgram(gShaderUBO);

        p_glGetProgramiv(gShaderUBO, GL_LINK_STATUS, &status);
        if (!status) {
            printf("program link error from UBO shader\n");
            utilPrintInfoLog(0, gShaderUBO);
        }
        utilGetMatrixLocations(gShaderUBO,
                               &gProjMatrixUBOLoc,
                               NULL);
        utilCreateUniformBufferObject();
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
    p_glBufferData(GL_ARRAY_BUFFER, sizeInBytes, NULL, BUFFER_USAGE_FLAG);
    p_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, icosahedronVertices);

    gPositionLoc = p_glGetAttribLocation(gShader, "inPosition");
    p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    p_glEnableVertexAttribArray(gPositionLoc);

    sizeInBytes = sizeof(unsigned short)*3*NUM_INDICES;
    p_glGenBuffers(1, &gEBO);
    p_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    p_glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes,NULL, BUFFER_USAGE_FLAG);
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
    p_glBufferData(GL_ARRAY_BUFFER, sizeInBytes*2, NULL, BUFFER_USAGE_FLAG);
    p_glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, quadVertices);
    p_glBufferSubData(GL_ARRAY_BUFFER, sizeInBytes, sizeInBytes, quadTexcoords);

    gQuadPositionLoc = p_glGetAttribLocation(gPostShader, "inPosition");
    p_glVertexAttribPointer(gQuadPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    p_glEnableVertexAttribArray(gQuadPositionLoc);

    GLuint texcoordLoc = p_glGetAttribLocation(gPostShader, "inTexCoords");
    p_glVertexAttribPointer(texcoordLoc, 3, GL_FLOAT, GL_FALSE, 0,
                            BUFFER_OFFSET(48));
    p_glEnableVertexAttribArray(texcoordLoc);

    p_glUseProgram(gPostShader);

    GLuint texLoc  = p_glGetUniformLocation(gPostShader, "tex");
    p_glUniform1i(texLoc, 0);

    GLuint texLoc2  = p_glGetUniformLocation(gPostShader, "texDepth");
    p_glUniform1i(texLoc2, 1);

    p_glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture);

    if (gHaveVAO)
        p_glBindVertexArray(0);

    p_glUseProgram(0);
}

/******************************************************************************/

/* Updates the modelview matrix with the given float params.  mtxLoc is only
 * used in the GLSL case and is the handle to the matrix uniform */
static inline void update_modelview_constants(const float* params, GLuint mtxLoc)
{
    if (gUseGLSL) {

        /* Using EXT_bindable_uniform */
        if (gUseBindableUniform) {
            /* TODO: Add other update mechanisms here for profiling:
             *  - glUniform4fv() (would need to rework the shader a bit)
             *  - ARB_map_buffer_range
             *  - Ring buffer approach using multiple buffer objects
             */
            switch (gBindableUpdateMethod) {

                case BINDABLE_UPDATE_GLUNIFORM_WITH_DISCARD:
                    p_glBufferData(GL_UNIFORM_BUFFER_EXT,
                                   sizeof(gModelViewMatrixf),
                                   NULL, BUFFER_USAGE_FLAG);
                    /* Fall-through */
                case BINDABLE_UPDATE_GLUNIFORM:
                    /* Just use the same glUniformMatrix command to update
                     * shader uniforms */
                    p_glUniformMatrix4fv(mtxLoc, 1, GL_FALSE, params);
                    break;

                case BINDABLE_UPDATE_BUFFERDATA:
                    /* Replace uniform buffer contents with glBufferData */
                    p_glBufferData(GL_UNIFORM_BUFFER_EXT,
                                   sizeof(gModelViewMatrixf),
                                   params, BUFFER_USAGE_FLAG);
                    break;

                case BINDABLE_UPDATE_BUFFERSUBDATA_WITH_DISCARD:
                    p_glBufferData(GL_UNIFORM_BUFFER_EXT,
                                   sizeof(gModelViewMatrixf),
                                   NULL, BUFFER_USAGE_FLAG);
                    /* Fall-through */
                case BINDABLE_UPDATE_BUFFERSUBDATA:
                    /* Update uniform buffer contents with glBufferSubData */
                    p_glBufferSubData(GL_UNIFORM_BUFFER_EXT,
                                      0, sizeof(gModelViewMatrixf), params);
                    break;

                case BINDABLE_UPDATE_MAPBUFFER_WITH_DISCARD:
                    p_glBufferData(GL_UNIFORM_BUFFER_EXT,
                                   sizeof(gModelViewMatrixf),
                                   NULL, BUFFER_USAGE_FLAG);
                    /* Fall-through */
                case BINDABLE_UPDATE_MAPBUFFER:
                {
                    /* Update uniform buffer contents with glMapBuffer */
                    void * ptr = p_glMapBuffer(GL_UNIFORM_BUFFER_EXT,
                                               GL_WRITE_ONLY);
                    if (ptr) {
                        memcpy(ptr, params, sizeof(gModelViewMatrixf));
                        p_glUnmapBuffer(GL_UNIFORM_BUFFER_EXT);
                    } else
                        fprintf(stderr, "ERROR: Unable to map buffer!\n");
                    break;
                }
                case BINDABLE_UPDATE_FLUSH_BUFFER_RANGE_WITH_DISCARD:
                    p_glBufferData(GL_UNIFORM_BUFFER_EXT,
                                   sizeof(gModelViewMatrixf),
                                   NULL, BUFFER_USAGE_FLAG);
                    /* Fall-through */
                case BINDABLE_UPDATE_FLUSH_BUFFER_RANGE:
                {
                    /* Update uniform buffer contents with glMapBuffer */
                    void * ptr = p_glMapBuffer(GL_UNIFORM_BUFFER_EXT,
                                               GL_WRITE_ONLY);
                    if (ptr) {
                        memcpy(ptr, params, sizeof(gModelViewMatrixf));
                        p_glFlushMappedBufferRangeAPPLE(GL_UNIFORM_BUFFER_EXT, 0, sizeof(gModelViewMatrixf));
                        p_glUnmapBuffer(GL_UNIFORM_BUFFER_EXT);
                    } else
                        fprintf(stderr, "ERROR: Unable to map buffer!\n");
                    break;
                }

                default:
                    fprintf(stderr, "ERROR: Unknown buffer update method: %d\n",
                            gBindableUpdateMethod);
                    break;
            }

        /* Using ARB_uniform_buffer_object */
        } else if (gUseUniformBufferObject) {

            GLbitfield access = (GL_MAP_WRITE_BIT |
                                 GL_MAP_FLUSH_EXPLICIT_BIT |
                                 GL_MAP_UNSYNCHRONIZED_BIT);

            /* Start the ring buffer over at 0. */
            if (gUBOOffset >= gUBOSize) {
                /* Orphan and invalidate the previous buffer contents */
                p_glBufferData(GL_UNIFORM_BUFFER,
                               gUBOSize,
                               NULL, UBO_USAGE_FLAG);
                access |= GL_MAP_INVALIDATE_BUFFER_BIT;
                gUBOOffset = 0;
            }

            switch (gUBOUpdateMethod) {

                case UBO_UPDATE_BUFFERSUBDATA:
                    /* Update uniform buffer contents with glBufferSubData */
                    p_glBufferSubData(GL_UNIFORM_BUFFER, gUBOOffset,
                                      sizeof(gModelViewMatrixf), params);
                    break;
                case UBO_UPDATE_MAPBUFFER_RANGE:
                {
                    /* Update uniform buffer contents with glMapBufferRange */
                    void * ptr = p_glMapBufferRange(GL_UNIFORM_BUFFER, gUBOOffset,
                                                    sizeof(gModelViewMatrixf),
                                                    access);
                    if (ptr) {
                        memcpy(ptr, params, sizeof(gModelViewMatrixf));
                        p_glFlushMappedBufferRange(GL_UNIFORM_BUFFER, gUBOOffset,
                                                   sizeof(gModelViewMatrixf));
                        p_glUnmapBuffer(GL_UNIFORM_BUFFER);
                    } else
                        fprintf(stderr, "ERROR: Unable to map buffer!\n");
                    break;
                }
                default:
                    fprintf(stderr, "ERROR: Unknown buffer update method: %d\n",
                            gBindableUpdateMethod);
                    break;
            }

            /* Bind the new starting UBO offset location */
            p_glBindBufferRange(GL_UNIFORM_BUFFER, gUBOBlockIndex,
                                gUBOBuffer, gUBOOffset,
                                sizeof(gModelViewMatrixf));
            gUBOOffset += gUBOOffsetIncrement;

        /* Just using regular glUniform commands */
        } else {
            p_glUniformMatrix4fv(mtxLoc, 1, GL_FALSE, params);
        }
    }
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
    GLuint mvMtxLoc = -1;

    if (gUseGLSL) {
        GLuint prog;
        GLuint projMtxLoc;

        if (gUseBindableUniform) {
            prog = gShaderBindable;
            projMtxLoc = gProjMatrixBindableLoc;
            mvMtxLoc = gMvMatrixBindableLoc;
        } else if (gUseUniformBufferObject) {
            prog = gShaderUBO;
            projMtxLoc = gProjMatrixUBOLoc;
            /* With UBO, there isn't a regular uniform location for mvMtxLoc.
             * The UBO constant update function uses the buffer solely. */
        } else {
            prog = gShader;
            projMtxLoc = gProjMatrixLoc;
            mvMtxLoc = gMvMatrixLoc;
        }

        /* Enable GLSL and disable any ARB programs */
        p_glUseProgram(prog);
        if (gHaveARBPrograms) {
            glDisable(GL_VERTEX_PROGRAM_ARB);
            glDisable(GL_FRAGMENT_PROGRAM_ARB);
        }

        /* Upload new projection matrix */
        p_glUniformMatrix4fv(projMtxLoc, 1, GL_FALSE, gProjectionMatrixf);

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
                update_modelview_constants(gModelViewMatrixf, mvMtxLoc);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

            if (gResetVertexPointers)
                p_glBindVertexArray(gVAO2);

            if (gResetConstants || i == 0)
                update_modelview_constants(gModelViewMatrixf2, mvMtxLoc);

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
                update_modelview_constants(gModelViewMatrixf, mvMtxLoc);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

            if (gResetVertexPointers)
                p_glVertexAttribPointer(gPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(36));

            if (gResetConstants || i == 0)
                update_modelview_constants(gModelViewMatrixf2, mvMtxLoc);

            glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT,BUFFER_OFFSET(0));
        }
    }
}

/* General OpenGL drawing routine */
void displayOpenGL()
{
    update();

    p_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    utilReshapeOrtho(gWindowWidth, gWindowHeight);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene();
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
    return (void*)glXGetProcAddressARB((const GLubyte*)fncString);
#endif
}

static void checkGLExtensions()
{
    const char* ver = (const char*)glGetString(GL_VERSION);
    const char* exts = NULL;

    /* Parse GL version */
    if (ver && *ver) {
        /* Skip any leading text and grab major version */
        while (!(*ver) >= '0' && *ver <= '9')
            ver++;
        gGLMajor = atoi(ver);
        /* Grab minor version */
        while ((*ver != 0) && (*ver != ' ') && (*ver != '.'))
            ver++;
        if (*ver == '.') {
            ver++;
            gGLMinor = atoi(ver);
            /* Grab subminor version */
            while ((*ver != 0) && (*ver != ' ') && (*ver != '.'))
                ver++;
            if (*ver == '.') {
                ver++;
                gGLSubminor = atoi(ver);
            }
        }
    }

#define GET_PROC_ADDRESS(x, y) \
    x = (void*)getGLProcAddress(#y); \
    if (!x) { \
        fprintf(stderr, "Cannot load " #x "! Bailing out!\n"); \
        killProcess(1); \
    }
/* Shortcut for core function pointers */
#define GET_CORE_PROC_ADDRESS(x)  GET_PROC_ADDRESS(p_##x, x)

/* TODO: Support GL3 extensions via glGetStringi() */
#define HAVE_EXTENSION(x) \
    ((gUseCoreContext) ? \
     (0) : \
     (strstr(exts, x) != NULL))

    if (!gUseCoreContext)
        exts = (const char*)glGetString(GL_EXTENSIONS);

    /* Grab ARB_multitexture extension pointers */
    printf("version: %s\n", glGetString(GL_VERSION));
    if (HAVE_EXTENSION("GL_ARB_multitexture")) {
        GET_PROC_ADDRESS(p_glActiveTexture, glActiveTextureARB);
    } else if ((gGLMajor > 1) || ((gGLMajor == 1) && (gGLMinor >= 3))) {
        GET_PROC_ADDRESS(p_glActiveTexture, glActiveTexture);
    } else {
        fprintf(stderr, "ERROR: ARB_multitexture not supported!\n");
        killProcess(1);
    }

    /* Grab ARB_shader_objects and GL 2.0 pointers */
    if (gGLMajor >= 2) {
        GET_CORE_PROC_ADDRESS(glAttachShader);
        GET_CORE_PROC_ADDRESS(glCompileShader);
        GET_CORE_PROC_ADDRESS(glCreateProgram);
        GET_CORE_PROC_ADDRESS(glCreateShader);
        GET_CORE_PROC_ADDRESS(glGetShaderInfoLog);
        GET_CORE_PROC_ADDRESS(glGetShaderiv);
        GET_CORE_PROC_ADDRESS(glShaderSource);
        GET_CORE_PROC_ADDRESS(glGetProgramiv);
        GET_CORE_PROC_ADDRESS(glGetProgramInfoLog);
        GET_CORE_PROC_ADDRESS(glGetUniformLocation);
        GET_CORE_PROC_ADDRESS(glLinkProgram);
        GET_CORE_PROC_ADDRESS(glUseProgram);
        GET_CORE_PROC_ADDRESS(glUniform1i);
        GET_CORE_PROC_ADDRESS(glUniformMatrix4fv);
    } else if (HAVE_EXTENSION("GL_ARB_shader_objects")) {
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
    } else {
        fprintf(stderr, "ERROR: ARB_shader_objects not supported\n");
        killProcess(1);
    }

    if (gGLMajor >= 2) {
        GET_CORE_PROC_ADDRESS(glEnableVertexAttribArray);
        GET_CORE_PROC_ADDRESS(glGetAttribLocation);
        GET_CORE_PROC_ADDRESS(glVertexAttribPointer);
    } else if (HAVE_EXTENSION("GL_ARB_vertex_shader")) {
        GET_PROC_ADDRESS(p_glEnableVertexAttribArray, glEnableVertexAttribArrayARB);
        GET_PROC_ADDRESS(p_glGetAttribLocation, glGetAttribLocationARB);
        GET_PROC_ADDRESS(p_glVertexAttribPointer, glVertexAttribPointerARB);
    } else {
        /* TODO: Make this app run w/o GLSL support */
        fprintf(stderr, "ERROR: ARB_vertex_shader not supported\n");
        killProcess(1);
    }

    if (gGLMajor < 2 && !HAVE_EXTENSION("GL_ARB_fragment_shader")) {
        fprintf(stderr, "ERROR: ARB_fragment_shader not supported\n");
        killProcess(1);
    }

    if (HAVE_EXTENSION("GL_ARB_vertex_program") &&
        HAVE_EXTENSION("GL_ARB_fragment_program")) {
        gHaveARBPrograms = 1;
        GET_PROC_ADDRESS(p_glBindProgramARB, glBindProgramARB);
        GET_PROC_ADDRESS(p_glGenProgramsARB, glGenProgramsARB);
        GET_PROC_ADDRESS(p_glProgramLocalParameter4fvARB, glProgramLocalParameter4fvARB);
        GET_PROC_ADDRESS(p_glProgramEnvParameter4fvARB, glProgramEnvParameter4fvARB);
        GET_PROC_ADDRESS(p_glProgramStringARB, glProgramStringARB);
        GET_PROC_ADDRESS(p_glGetProgramivARB, glGetProgramivARB);
    }

    /* Grab FBO extension pointers */
    if (gGLMajor >= 2 || HAVE_EXTENSION("GL_ARB_framebuffer_object")) {
        GET_CORE_PROC_ADDRESS(glBindRenderbuffer);
        GET_CORE_PROC_ADDRESS(glGenRenderbuffers);
        GET_CORE_PROC_ADDRESS(glBindFramebuffer);
        GET_CORE_PROC_ADDRESS(glGenFramebuffers);
        GET_CORE_PROC_ADDRESS(glCheckFramebufferStatus);
        GET_CORE_PROC_ADDRESS(glFramebufferTexture2D);
        GET_CORE_PROC_ADDRESS(glFramebufferRenderbuffer);
    } else if (HAVE_EXTENSION("GL_EXT_framebuffer_object")) {
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
    if ((gGLMajor > 1) || ((gGLMajor == 1) && (gGLMinor >= 5))) {
        GET_CORE_PROC_ADDRESS(glBindBuffer);
        GET_CORE_PROC_ADDRESS(glGenBuffers);
        GET_CORE_PROC_ADDRESS(glBufferSubData);
        GET_CORE_PROC_ADDRESS(glBufferData);
        GET_CORE_PROC_ADDRESS(glMapBuffer);
        GET_CORE_PROC_ADDRESS(glUnmapBuffer);
    } else if (HAVE_EXTENSION("GL_ARB_vertex_buffer_object")) {
        GET_PROC_ADDRESS(p_glBindBuffer, glBindBufferARB);
        GET_PROC_ADDRESS(p_glGenBuffers, glGenBuffersARB);
        GET_PROC_ADDRESS(p_glBufferSubData, glBufferSubDataARB);
        GET_PROC_ADDRESS(p_glBufferData, glBufferDataARB);
        GET_PROC_ADDRESS(p_glMapBuffer, glMapBufferARB);
        GET_PROC_ADDRESS(p_glUnmapBuffer, glUnmapBufferARB);
    } else {
        fprintf(stderr, "ERROR: VBO not supported\n");
        killProcess(1);
    }

    /* Grab bindable uniform pointers */
    if (HAVE_EXTENSION("GL_EXT_bindable_uniform")) {
        gHaveBindableUniform = 1;
        GET_PROC_ADDRESS(p_glUniformBufferEXT, glUniformBufferEXT);
        GET_PROC_ADDRESS(p_glGetUniformBufferSizeEXT, glGetUniformBufferSizeEXT);
        GET_PROC_ADDRESS(p_glGetUniformOffsetEXT, glGetUniformOffsetEXT);
    } else {
        gHaveBindableUniform = 0;
        gUseBindableUniform = 0;
    }

    /* Grab uniform buffer object pointers */
    if ((gGLMajor > 3) || ((gGLMajor == 3) && (gGLMinor >= 1)) ||
        HAVE_EXTENSION("GL_ARB_uniform_buffer_object")) {
        gHaveUniformBufferObject = 1;
        GET_CORE_PROC_ADDRESS(glGetUniformIndices);
        GET_CORE_PROC_ADDRESS(glGetActiveUniformsiv);
        GET_CORE_PROC_ADDRESS(glGetActiveUniformName);
        GET_CORE_PROC_ADDRESS(glGetUniformBlockIndex);
        GET_CORE_PROC_ADDRESS(glGetActiveUniformBlockiv);
        GET_CORE_PROC_ADDRESS(glGetActiveUniformBlockName);
        GET_CORE_PROC_ADDRESS(glUniformBlockBinding);
        GET_CORE_PROC_ADDRESS(glBindBufferRange);
        GET_CORE_PROC_ADDRESS(glBindBufferBase);
    } else {
        gHaveUniformBufferObject = 0;
        gUseUniformBufferObject = 0;
    }

    /* Grab APPLE_flush_buffer_range pointers */
    if (HAVE_EXTENSION("GL_APPLE_flush_buffer_range")) {
        gHaveFlushBufferRange = 1;
        GET_PROC_ADDRESS(p_glBufferParameteriAPPLE, glBufferParameteriAPPLE);
        GET_PROC_ADDRESS(p_glFlushMappedBufferRangeAPPLE, glFlushMappedBufferRangeAPPLE);
    }

    /* Grab ARB_map_buffer_object pointers */
    if ((gGLMajor >= 3) || HAVE_EXTENSION("GL_ARB_map_buffer_object")) {
        gHaveMapBufferRange = 1;
        GET_CORE_PROC_ADDRESS(glMapBufferRange);
        GET_CORE_PROC_ADDRESS(glFlushMappedBufferRange);
    }

    /* Grab VAO extension pointers */
    if ((gGLMajor >= 3) || HAVE_EXTENSION("GL_ARB_vertex_array_object")) {
        GET_CORE_PROC_ADDRESS(glGenVertexArrays);
        GET_CORE_PROC_ADDRESS(glBindVertexArray);
        gHaveVAO = 1;
    } else if (HAVE_EXTENSION("GL_APPLE_vertex_array_object")) {
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
    if (HAVE_EXTENSION("GL_EXT_gpu_program_parameters")) {
        GET_PROC_ADDRESS(p_glProgramEnvParameters4fvEXT, glProgramEnvParameters4fvEXT);
        GET_PROC_ADDRESS(p_glProgramLocalParameters4fvEXT, glProgramLocalParameters4fvEXT);
        gHaveGPUProgramParameters  = 1;
    }

#ifdef __APPLE__
    /* Enable multi-threaded GL if enabled */
    if (gUseMultiThreadedGL)
        CGLEnable( CGLGetCurrentContext(), kCGLCEMPEngine);
#endif

#undef GET_PROC_ADDRESS
#undef GET_CORE_PROC_ADDRESS
}

void toggleFlushBufferRange(int enable)
{
    /* Disallow APPLE_flush_buffer_range unless we have the extension */
    if (!gHaveFlushBufferRange) {
        if (gBindableUpdateMethod >= BINDABLE_UPDATE_FLUSH_BUFFER_RANGE)
            gBindableUpdateMethod = 0;

    } else {
        /* If we want to enable the use of the extension, we need to disable these
         * two buffer parameters. */
        GLboolean val = (enable ? GL_FALSE : GL_TRUE);

        /* Toggle the flushing buffer parameters if necessary */
        p_glBufferParameteriAPPLE(GL_UNIFORM_BUFFER_EXT, GL_BUFFER_SERIALIZED_MODIFY_APPLE, val);
        p_glBufferParameteriAPPLE(GL_UNIFORM_BUFFER_EXT, GL_BUFFER_FLUSHING_UNMAP_APPLE, val);
    }
}

void setViewportOGL(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);
}

void shutdownOpenGL()
{
    /* TODO: Clean shutdown for the various platforms */
}

#ifndef GL_MULTISAMPLE_ARB
#define GL_MULTISAMPLE_ARB 0x809D
#endif
#ifndef GL_CLIP_VOLUME_CLIPPING_HINT_EXT
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT 0x80F0
#endif

void initOpenGLStates()
{
    checkGLExtensions();
    createDummyTex();
    createFBO();
    createShaders();
    if (gHaveARBPrograms)
        createARBPrograms();
    createVertexBuffers();

    CHECK_GL_ERROR;

    gWindowWidth = DEFAULT_WINDOW_WIDTH;
    gWindowHeight = DEFAULT_WINDOW_HEIGHT;

    /* Set to D3D defaults */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_MULTISAMPLE_ARB);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glDisable(GL_DITHER);

    if (gGLMajor < 3) {
        /* Deprecated states */
        glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }
    CHECK_GL_ERROR;
}

void initOpenGL(int argc, char **argv)
{
#ifdef __APPLE__
    initMacOS();
#elif defined(linux)
    initLinux();
#elif defined(__WIN32__)
    initOpenGLWin32();
#else
    fprint(stderr, "Platform not supported!\n");
#endif
}

