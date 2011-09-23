/* D3D9 Performance test
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */
#include "perftest.h"

#ifdef __WIN32__
#include "tg_d3dx9.h"
/* D3D9 globals */
static IDirect3D9 *gD3D9Ptr = NULL;
static IDirect3DDevice9 *gDevicePtr = NULL;
static IDirect3DTexture9 *gTexturePtr = NULL;
static IDirect3DVertexShader9 *gVShaderPtr = NULL;
static IDirect3DPixelShader9 *gPShaderPtr = NULL;
static IDirect3DVertexBuffer9 *gVertexBufferPtr = NULL;
static IDirect3DIndexBuffer9 *gIndexBufferPtr = NULL;
static IDirect3DVertexDeclaration9 *gVertexDeclPtr = NULL;
static IDirect3DSurface9 *gDeviceBackBufferPtr = NULL;

/* D3D version of the shaders used in perftest_ogl.c
 *  - inPosition = v[0]
 *  - inProjectionMatrix c[0..3]
 *  - inModelViewMatrix c[4..7]
 */
static const char* basicVertexShaderSourceD3D9 =
    "vs.1.1\n"
    "dcl_position v0;\n"
    "def c10, 0.0, 0.0, 0.0, 0.0;\n"
    "mov r0, c1;\n"
    "mov r1, c0;\n"
    "mul r3, r0, c4.y;\n"
    "mul r2, r0, c5.y;\n"
    "mad r5, r1, c4.x, r3;\n"
    "mad r3, r1, c5.x, r2;\n"
    "mov r2, c2;\n"
    "mad r4, r2, c5.z, r3;\n"
    "mov r3, c3;\n"
    "mad r4, r3, c5.w, r4;\n"
    "mad r5, r2, c4.z, r5;\n"
    "mul r4, v0.y, r4;\n"
    "mad r5, r3, c4.w, r5;\n"
    "mad r5, v0.x, r5, r4;\n"
    "mul r4, r0, c6.y;\n"
    "mul r0, r0, c7.y;\n"
    "mad r0, r1, c7.x, r0;\n"
    "mad r4, r1, c6.x, r4;\n"
    "mad r1, r2, c6.z, r4;\n"
    "mad r0, r2, c7.z, r0;\n"
    "mad r1, r3, c6.w, r1;\n"
    "mad r1, v0.z, r1, r5;\n"
    "mad r0, r3, c7.w, r0;\n"
    "mad oPos, v0.w, r0, r1;\n"
    "mov oPos.z, c10;\n";

static const char* basicPixelShaderSourceD3D9 =
    "ps.1.1\n"
    "def c0, 0.0, 1.0, 0.0, 1.0;\n"
    "mov r0, c0;\n";

static void createDummyTexD3D9()
{
    HRESULT hr = IDirect3DDevice9_CreateTexture(gDevicePtr, 256, 256, 1, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &gTexturePtr, NULL);
    if (!gTexturePtr) {
        fprintf(stderr, "Unable to create texture, 0x%08lx\n", hr);
        killProcess(1);
    }
}

static void createShadersD3D9()
{
    static HMODULE d3dx9_handle = 0;
    LPD3DXBUFFER vShaderBuffer = NULL;
    LPD3DXBUFFER pShaderBuffer = NULL;
    LPD3DXBUFFER errBuf = NULL;
    HRESULT hr;
    HRESULT (__stdcall * d3dx_assemble_shader)(LPCSTR pSrcData, UINT SrcDataLen, CONST D3DXMACRO* pDefines,
                                               LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader,
                                               LPD3DXBUFFER* ppErrorMsgs);

    d3dx9_handle = LoadLibraryA("d3dx9_30.dll");
    if (!d3dx9_handle) {
        fprintf(stderr, "Could not load d3dx9_30.dll\n");
        return;
    }

    d3dx_assemble_shader = (void *)GetProcAddress(d3dx9_handle, "D3DXAssembleShader");
    if (!d3dx_assemble_shader) {
        fprintf(stderr, "Could not get address of D3DXAssembleShader\n");
        goto cleanup;
    }

    /* Assemble the vertex shader into bytecode and create it */
    hr = d3dx_assemble_shader(basicVertexShaderSourceD3D9, strlen(basicVertexShaderSourceD3D9), NULL, NULL, 0, &vShaderBuffer, &errBuf);
    if (FAILED(hr)) {
        fprintf(stderr, "Vertex shader assembly failed\n");
	if (errBuf) {
            fprintf(stderr, "error: %s\n", (const char*)ID3DXBuffer_GetBufferPointer(errBuf));
	    ID3DXBuffer_Release(errBuf);
	    errBuf = NULL;
	}
        goto cleanup;
    }
    hr = IDirect3DDevice9_CreateVertexShader(gDevicePtr, ID3DXBuffer_GetBufferPointer(vShaderBuffer), &gVShaderPtr);
    if (FAILED(hr)) {
        fprintf(stderr, "Vertex shader creation failed\n");
        goto cleanup;
    }

    /* Assemble the pixel shader into bytecode and create it */
    hr = d3dx_assemble_shader(basicPixelShaderSourceD3D9, strlen(basicPixelShaderSourceD3D9), NULL, NULL, 0, &pShaderBuffer, &errBuf);
    if (FAILED(hr)) {
        fprintf(stderr, "Pixel shader assembly failed\n");
	if (errBuf) {
            fprintf(stderr, "error: %s\n", (const char*)ID3DXBuffer_GetBufferPointer(errBuf));
	    ID3DXBuffer_Release(errBuf);
	    errBuf = NULL;
	}
        goto cleanup;
    }
    hr = IDirect3DDevice9_CreatePixelShader(gDevicePtr, ID3DXBuffer_GetBufferPointer(pShaderBuffer), &gPShaderPtr);
    if (FAILED(hr)) {
        fprintf(stderr, "Pixel shader creation failed\n");
        goto cleanup;
    }

    return;

cleanup:
    if (vShaderBuffer)
        ID3DXBuffer_Release(vShaderBuffer);
    if (pShaderBuffer)
        ID3DXBuffer_Release(pShaderBuffer);
    FreeLibrary(d3dx9_handle);
}

static void createVertexBuffersD3D9()
{
    HRESULT hr;
    LPVOID ptr = NULL;
    int sizeInBytes = sizeof(float)*3*NUM_VERTICES;
    D3DVERTEXELEMENT9 decl[] = {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        D3DDECL_END(),
    };

    hr = IDirect3DDevice9_CreateVertexBuffer(gDevicePtr, sizeInBytes, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &gVertexBufferPtr, NULL);
    if(FAILED(hr) || !gVertexBufferPtr) {
        fprintf(stderr, "Failed to create vertex buffer\n");
        return;
    }
    hr = IDirect3DVertexBuffer9_Lock(gVertexBufferPtr, 0, 0, &ptr, D3DLOCK_DISCARD);
    if (FAILED(hr) || !ptr) {
        fprintf(stderr, "Failed to lock vertex buffer\n");
        return;
    }
    memcpy(ptr, icosahedronVertices, sizeInBytes);
    hr = IDirect3DVertexBuffer9_Unlock(gVertexBufferPtr);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to unlock vertex buffer\n");
        return;
    }

    sizeInBytes = sizeof(unsigned short)*3*NUM_INDICES;
    hr = IDirect3DDevice9_CreateIndexBuffer(gDevicePtr, sizeInBytes, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &gIndexBufferPtr, NULL);
    if(FAILED(hr) || !gIndexBufferPtr) {
        fprintf(stderr, "Failed to create index buffer\n");
        return;
    }
    hr = IDirect3DIndexBuffer9_Lock(gIndexBufferPtr, 0, 0, &ptr, D3DLOCK_DISCARD);
    if (FAILED(hr) || !ptr) {
        fprintf(stderr, "Failed to lock index buffer\n");
        return;
    }
    memcpy(ptr, icosahedronIndices, sizeInBytes);
    hr = IDirect3DIndexBuffer9_Unlock(gIndexBufferPtr);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to unlock index buffer\n");
        return;
    }

    hr = IDirect3DDevice9_CreateVertexDeclaration(gDevicePtr, decl, &gVertexDeclPtr);
    if(FAILED(hr) || !gVertexDeclPtr) {
        fprintf(stderr, "Failed to create vertex declaration\n");
        return;
    }
}

static inline void update_modelview_constants_d3d9(const float* params)
{
    IDirect3DDevice9_SetVertexShaderConstantF(gDevicePtr, 4, params, 4);
}

static void displayD3D9()
{
    int i;

    update();

    IDirect3DDevice9_SetRenderState(gDevicePtr, D3DRS_LIGHTING, FALSE);
    IDirect3DDevice9_SetRenderState(gDevicePtr, D3DRS_CLIPPING, FALSE);
    IDirect3DDevice9_SetRenderTarget(gDevicePtr, 0, gDeviceBackBufferPtr);
    utilReshapeOrtho(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    IDirect3DDevice9_Clear(gDevicePtr, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF000000, 1.0f, 0);
    IDirect3DDevice9_SetVertexShader(gDevicePtr, gVShaderPtr);
    IDirect3DDevice9_SetPixelShader(gDevicePtr, gPShaderPtr);
    IDirect3DDevice9_SetVertexDeclaration(gDevicePtr, gVertexDeclPtr);
    IDirect3DDevice9_SetStreamSource(gDevicePtr, 0, gVertexBufferPtr, 0, sizeof(FLOAT) * 3);
    IDirect3DDevice9_SetIndices(gDevicePtr, gIndexBufferPtr);

    /* Send projection matrix constants */
    IDirect3DDevice9_SetVertexShaderConstantF(gDevicePtr, 0, gProjectionMatrixf, 4);

    IDirect3DDevice9_BeginScene(gDevicePtr);

    for (i = 0; i < gNumDrawCalls; i++) {
        if (gResetVertexPointers)
            IDirect3DDevice9_SetStreamSource(gDevicePtr, 0, gVertexBufferPtr, 0, sizeof(FLOAT) * 3);

        if (gResetConstants || i == 0)
            update_modelview_constants_d3d9(gModelViewMatrixf);

        IDirect3DDevice9_DrawIndexedPrimitive(gDevicePtr, D3DPT_TRIANGLELIST, 0, 0, NUM_VERTICES, 0, NUM_INDICES);

        if (gResetVertexPointers)
            IDirect3DDevice9_SetStreamSource(gDevicePtr, 0, gVertexBufferPtr, 36, sizeof(FLOAT) * 3);

        if (gResetConstants || i == 0)
            update_modelview_constants_d3d9(gModelViewMatrixf2);

        IDirect3DDevice9_DrawIndexedPrimitive(gDevicePtr, D3DPT_TRIANGLELIST, 0, 0, NUM_VERTICES, 0, NUM_INDICES);
    }

    IDirect3DDevice9_EndScene(gDevicePtr);

    IDirect3DDevice9_Present(gDevicePtr, NULL, NULL, 0, NULL);

    /* Reset state to default */
    IDirect3DDevice9_SetVertexShader(gDevicePtr, NULL);
    IDirect3DDevice9_SetPixelShader(gDevicePtr, NULL);
    IDirect3DDevice9_SetVertexDeclaration(gDevicePtr, NULL);
    IDirect3DDevice9_SetStreamSource(gDevicePtr, 0, NULL, 0, 0);
    IDirect3DDevice9_SetIndices(gDevicePtr, NULL);
}
#endif

void setViewportD3D9(int x, int y, int width, int height)
{
#ifdef __WIN32__
    D3DVIEWPORT9 vp = { x, y, width, height, 0.0f, 1.0f };
    IDirect3DDevice9_SetViewport(gDevicePtr, &vp);
#endif
}

void shutdownD3D9()
{
#ifdef __WIN32__
    /* Free all resources */
    if (gTexturePtr)
        IDirect3DTexture9_Release(gTexturePtr);
    gTexturePtr = NULL;
    if (gVShaderPtr)
        IDirect3DVertexShader9_Release(gVShaderPtr);
    gVShaderPtr = NULL;
    if (gPShaderPtr)
        IDirect3DPixelShader9_Release(gPShaderPtr);
    gPShaderPtr = NULL;
    if (gVertexBufferPtr)
        IDirect3DVertexBuffer9_Release(gVertexBufferPtr);
    gVertexBufferPtr = NULL;
    if (gIndexBufferPtr)
        IDirect3DIndexBuffer9_Release(gIndexBufferPtr);
    gIndexBufferPtr = NULL;
    if (gVertexDeclPtr)
        IDirect3DVertexDeclaration9_Release(gVertexDeclPtr);
    gVertexDeclPtr = NULL;
    if (gDevicePtr)
        IDirect3DDevice9_Release(gDevicePtr);
    gDevicePtr = NULL;
    if (gD3D9Ptr)
        IDirect3D9_Release(gD3D9Ptr);
    gD3D9Ptr = NULL;

    gDeviceBackBufferPtr = NULL;
#endif
}

void initD3D9()
{
#ifdef __WIN32__
    static HMODULE d3d9_handle = 0;
    IDirect3D9 *(__stdcall * d3d9_create)(UINT SDKVersion) = NULL;
    D3DPRESENT_PARAMETERS present_parameters;
    D3DADAPTER_IDENTIFIER9 identifier;
    HRESULT hr;
    HWND hwnd;

    d3d9_handle = LoadLibraryA("d3d9.dll");
    if (!d3d9_handle) {
        fprintf(stderr, "Could not load d3d9.dll\n");
        return;
    }

    d3d9_create = (void *)GetProcAddress(d3d9_handle, "Direct3DCreate9");
    if (!d3d9_create) {
        fprintf(stderr, "Could not get address of Direct3DCreate9\n");
        return;
    }

    gD3D9Ptr = d3d9_create(D3D_SDK_VERSION);
    if (!gD3D9Ptr) {
        fprintf(stderr, "could not create D3D9\n");
        return;
    }

    hwnd = createWindowWin32();

    ZeroMemory(&present_parameters, sizeof(present_parameters));
    present_parameters.Windowed = FALSE;
    present_parameters.hDeviceWindow = hwnd;
    present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present_parameters.BackBufferWidth = DEFAULT_WINDOW_WIDTH;;
    present_parameters.BackBufferHeight = DEFAULT_WINDOW_HEIGHT;
    present_parameters.BackBufferFormat = D3DFMT_A8R8G8B8;
    present_parameters.EnableAutoDepthStencil = TRUE;
    present_parameters.AutoDepthStencilFormat = D3DFMT_D24S8;
    present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    memset(&identifier, 0, sizeof(identifier));
    IDirect3D9_GetAdapterIdentifier(gD3D9Ptr, 0, 0, &identifier);
    printf("Driver string: \"%s\"\n", identifier.Driver);
    printf("Description string: \"%s\"\n", identifier.Description);
    printf("Device name string: \"%s\"\n", identifier.DeviceName);
    printf("Driver version %d.%d.%d.%d\n",
          HIWORD(identifier.DriverVersion.HighPart), LOWORD(identifier.DriverVersion.HighPart),
          HIWORD(identifier.DriverVersion.LowPart), LOWORD(identifier.DriverVersion.LowPart));

    hr = IDirect3D9_CreateDevice(gD3D9Ptr, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, present_parameters.hDeviceWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &present_parameters, &gDevicePtr);
    if(FAILED(hr)) {
        present_parameters.AutoDepthStencilFormat = D3DFMT_D16;
        hr = IDirect3D9_CreateDevice(gD3D9Ptr, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, present_parameters.hDeviceWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &present_parameters, &gDevicePtr);
        if(FAILED(hr)) {
            hr = IDirect3D9_CreateDevice(gD3D9Ptr, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, present_parameters.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &present_parameters, &gDevicePtr);
        }
    }

    if (FAILED(hr) || !gDevicePtr) {
        fprintf(stderr, "Creating the device failed\n");
        return;
    }

    /* Not yet being used */
    if (0)
        createDummyTexD3D9();

    createShadersD3D9();
    createVertexBuffersD3D9();

    IDirect3DDevice9_GetBackBuffer(gDevicePtr, 0, 0, D3DBACKBUFFER_TYPE_MONO, &gDeviceBackBufferPtr);

    ShowWindow(hwnd, TRUE);
    UpdateWindow(hwnd);

    while (!gIsQuitting) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        displayD3D9();
    }
#else
    fprintf(stderr, "ERROR: D3D9 not supported on this platform\n");
#endif
}

