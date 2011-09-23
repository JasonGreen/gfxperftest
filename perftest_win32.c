/* D3D9 Performance test
 *
 * Windows-specific functions
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */
#include "perftest.h"

#ifdef __WIN32__
BOOL        gIsQuitting = FALSE;
HDC         gHdc = 0;

LRESULT CALLBACK WndProcPerfTest(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
        case WM_KEYDOWN:
            switch (wParam) {
                case 'q':
                case 'Q':
                    /* Quit */
                    gIsQuitting = TRUE;
                    break;
                default:
                    if (handleKeyPress(wParam))
                        return 0;
                    else
                        return DefWindowProc(hwnd, msg, wParam, lParam);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            gIsQuitting = TRUE;
            break;
        case WM_DESTROY:
            gIsQuitting = TRUE;
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND createWindowWin32(void)
{
    WNDCLASS wc = {0};
    HWND hwnd;
    wc.lpfnWndProc = WndProcPerfTest;
    wc.lpszClassName = "GfxPerfTest";
    RegisterClass(&wc);

    hwnd = CreateWindow("GfxPerfTest", "GfxPerfTest",
                        WS_MAXIMIZE | WS_VISIBLE | WS_CAPTION , 0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0, 0, 0, 0);
    return hwnd;
}

void initOpenGLWin32()
{
    PIXELFORMATDESCRIPTOR pfd;
    int pixelFormat;
    HGLRC hrc;
    HWND hwnd;

    hwnd = createWindowWin32();

    /* Create the GL context */
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    gHdc = GetDC(hwnd);
    pixelFormat = ChoosePixelFormat(gHdc, &pfd);

    SetPixelFormat(gHdc, pixelFormat, &pfd);
    hrc = wglCreateContext(gHdc);
    wglMakeCurrent(gHdc, hrc);

    ShowWindow(hwnd, SW_SHOWNORMAL);

    /* Disable VSync */
    typedef int (APIENTRYP PFNWGLSWAPINTERVALEXTPROC) (int interval);
    PFNWGLSWAPINTERVALEXTPROC p_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (p_wglSwapIntervalEXT) {
        if (!p_wglSwapIntervalEXT(0)) {
            fprintf(stderr, "WARNING: Cannot disable VSync!\n");
        }
    }

#ifndef WGL_ARB_create_context
#define WGL_CONTEXT_MAJOR_VERSION_ARB               0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB               0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB                 0x2093
#define WGL_CONTEXT_FLAGS_ARB                       0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB                0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                   0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB      0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x00000002
#endif

    /* Create a core 3.2 context if requested */
    if (gUseCoreContext) {
        typedef HGLRC (APIENTRYP PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attriblist);
        PFNWGLCREATECONTEXTATTRIBSARBPROC p_wglCreateContextAttribsARB =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        if (!p_wglCreateContextAttribsARB) {
            fprintf(stderr, "Core context not available\n");
            gUseCoreContext = 0;
        } else {
            const int attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                0
            };
            HGLRC newRC = p_wglCreateContextAttribsARB(gHdc, 0, attribs);

            /* Switch to new context and delete the old one. */
            wglMakeCurrent(0, 0);
            wglDeleteContext(hrc);
            hrc = newRC;
            wglMakeCurrent(gHdc, hrc);
            CHECK_GL_ERROR;
        }

    }

    initOpenGLStates();

    /* Run loop */
    while (!gIsQuitting) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        displayOpenGL();
        SwapBuffers(gHdc);
    }
}

#endif
