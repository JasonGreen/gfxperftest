/* D3D9 Performance test partial headers for D3DX9 functionality
 *
 * Copyright 2010-2011, TransGaming, Inc.
 */
#include <d3d9.h>


/*** ID3DXBuffer interface */
#undef INTERFACE
#define INTERFACE ID3DXBuffer
DECLARE_INTERFACE_(ID3DXBuffer,IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
    STDMETHOD_(LPVOID,GetBufferPointer)(THIS) PURE;
    STDMETHOD_(DWORD,GetBufferSize)(THIS) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define ID3DXBuffer_QueryInterface(p,a,b)     (p)->lpVtbl->QueryInterface(p,a,b)
#define ID3DXBuffer_AddRef(p)                 (p)->lpVtbl->AddRef(p)
#define ID3DXBuffer_Release(p)                (p)->lpVtbl->Release(p)
#define ID3DXBuffer_GetBufferPointer(p)       (p)->lpVtbl->GetBufferPointer(p)
#define ID3DXBuffer_GetBufferSize(p)          (p)->lpVtbl->GetBufferSize(p)
#else
#define ID3DXBuffer_QueryInterface(p,a,b)     (p)->QueryInterface(,a,b)
#define ID3DXBuffer_AddRef(p)                 (p)->AddRef()
#define ID3DXBuffer_Release(p)                (p)->Release()
#define ID3DXBuffer_GetBufferPointer(p)       (p)->GetBufferPointer()
#define ID3DXBuffer_GetBufferSize(p)          (p)->GetBufferSize()
#endif
#undef INTERFACE
typedef struct ID3DXBuffer *LPD3DXBUFFER;

typedef struct _D3DXMACRO
{
    LPCSTR Name;
    LPCSTR Definition;

} D3DXMACRO, *LPD3DXMACRO;

/* HACK: Actually fill out the ID3DXInclude class */
#define LPD3DXINCLUDE LPVOID

