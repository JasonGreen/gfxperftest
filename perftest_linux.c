/* Performance test for low-level graphics functions
 *
 * Linux-specific functions
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */
#ifdef linux

#include <sys/time.h>
#include <signal.h>
#include "perftest.h"

typedef struct {
    Display *MainDisplay;
    Window MainWindow;
} PlatformContext;

void initLinux()
{
    int attrib[] = {
        GLX_RENDER_TYPE,    GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER,   True,
        GLX_RED_SIZE,       1,
        GLX_GREEN_SIZE,     1,
        GLX_BLUE_SIZE,      1,
        None
    };

    PlatformContext context;
    context.MainDisplay = XOpenDisplay(NULL);
    int screen = DefaultScreen(context.MainDisplay);
    Window root = RootWindow(context.MainDisplay, screen);

    int fbcount = 0;
    PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig =
        (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddress((GLubyte*)"glXChooseFBConfig");
    GLXFBConfig *fbc = glXChooseFBConfig(context.MainDisplay, screen, attrib, &fbcount);
    if (!fbc) {
        fprintf(stderr, "Failed to retrieve a framebuffer config!\n");
        exit(1);
    }

    PFNGLXGETVISUALFROMFBCONFIGPROC glXGetVisualFromFBConfig =
        (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddress((GLubyte*)"glXGetVisualFromFBConfig");
    XVisualInfo *visinfo = glXGetVisualFromFBConfig(context.MainDisplay, fbc[0]);
    if (!visinfo) {
        fprintf(stderr, "Fail to open window with pixel format!\n");
        exit(1);
    }

    XSetWindowAttributes attr;
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(context.MainDisplay, root, visinfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask |
                      PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

    context.MainWindow = XCreateWindow(context.MainDisplay,
                                       root,
                                       0, 0,
                                       DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0,
                                       visinfo->depth,
                                       InputOutput,
                                       visinfo->visual,
                                       CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
                                       &attr);
    XMapWindow(context.MainDisplay, context.MainWindow);

    GLXContext glcontext;
    if (gUseCoreContext) {
        GLXContext tempContext = glXCreateContext(context.MainDisplay, visinfo, NULL, True);
        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribs =
            (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
        if (!glXCreateContextAttribs) {
            fprintf(stderr, "Could not create Core 3.0 context!\n");
            exit(1);
        }

        fbcount = 0;
        GLXFBConfig *framebufferConfig = glXChooseFBConfig(context.MainDisplay, screen, 0, &fbcount);
        if (!framebufferConfig) {
            fprintf(stderr, "Can't create framebuffer for core context!\n");
            exit(1);
        } else {
            int coreAttribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                0
            };
            glcontext = glXCreateContextAttribs(context.MainDisplay, framebufferConfig[0], NULL, True, coreAttribs);
            glXMakeCurrent(context.MainDisplay, 0, 0);
            glXDestroyContext(context.MainDisplay, tempContext);
        }
    } else {
        glcontext = glXCreateContext(context.MainDisplay, visinfo, NULL, True);
    }

    glXMakeCurrent(context.MainDisplay, context.MainWindow, glcontext);

    /* Clear any GL errors */
    while (glGetError() != GL_NO_ERROR) { }

    XStoreName(context.MainDisplay, context.MainWindow, "GfxPerfTest");

    initOpenGLStates();

    /* Run loop */
    int done = 0;
    while (!done) {

        if (XPending(context.MainDisplay)) {
            XEvent event;
            XNextEvent(context.MainDisplay, &event);

            switch (event.type) {
                case KeyPress:
                {
                    XComposeStatus composeStatus;
                    char asciiCode[32];
                    KeySym keysym;

                    XLookupString(&event.xkey, asciiCode,
                                  sizeof(asciiCode),
                                  &keysym, &composeStatus);

                    if ((asciiCode[0] == 'q') || (asciiCode[0] == 'Q'))
                        done = 1;
                    else
                        handleKeyPress(asciiCode[0]);
                    break;
                }
            }
        }

        displayOpenGL();

        glXSwapBuffers(context.MainDisplay, context.MainWindow);
    }
}

#endif
