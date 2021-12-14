#pragma once
#include "resource.h"
#include "common.h"

inline auto CreateD3DDevice(D3D_DRIVER_TYPE type)
{
    winrt::com_ptr<ID3D11Device>   device{ nullptr };

    UINT flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    winrt::check_hresult(D3D11CreateDevice(nullptr, type, NULL, flag, nullptr, 0, D3D11_SDK_VERSION, device.put(), nullptr, nullptr));

    return device.as<ID3D11Device5>();
}

inline auto CreateD3DDeviceContext(ID3D11Device* device)
{
    winrt::com_ptr<ID3D11DeviceContext> context{ nullptr };
    device->GetImmediateContext(context.put());
    return context.as<ID3D11DeviceContext4>();
}

inline auto CreateSwapChainForHwnd(winrt::com_ptr<IDXGIDevice> device, HWND hwnd, int width, int height, DXGI_FORMAT format)
{
    winrt::com_ptr<IDXGIAdapter> adapter;
    winrt::check_hresult(device->GetParent(__uuidof(*adapter), adapter.put_void()));

    winrt::com_ptr<IDXGIFactory2> factory{ nullptr };
    winrt::check_hresult(adapter->GetParent(__uuidof(*factory), factory.put_void()));

    winrt::com_ptr<IDXGISwapChain1> swapChain{ nullptr };
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_NONE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    winrt::check_hresult(factory->CreateSwapChainForHwnd(device.get(), hwnd, &desc, nullptr, nullptr, swapChain.put()));

    return swapChain;
}

inline auto CreateSwapChainForComposition(winrt::com_ptr<IDXGIDevice> device, int width, int height, DXGI_FORMAT format)
{
    winrt::com_ptr<IDXGIAdapter> adapter;
    winrt::check_hresult(device->GetParent(__uuidof(*adapter), adapter.put_void()));

    winrt::com_ptr<IDXGIFactory2> factory{ nullptr };
    winrt::check_hresult(adapter->GetParent(__uuidof(*factory), factory.put_void()));

    winrt::com_ptr<IDXGISwapChain1> swapChain{ nullptr };
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = UseTransparentSwapChain ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_UNSPECIFIED;
    winrt::check_hresult(factory->CreateSwapChainForComposition(device.get(), &desc, nullptr, swapChain.put()));

    return swapChain;
}

inline auto CreateRenderTargetView(ID3D11Device* device, ID3D11Resource* backbuffer)
{
    winrt::com_ptr<ID3D11RenderTargetView> view{ nullptr };
    winrt::check_hresult(device->CreateRenderTargetView(backbuffer, nullptr, view.put()));
    return view;
}

inline auto CreateRenderTargetView(ID3D11Device* device, IDXGISwapChain* swapChain)
{
    winrt::com_ptr<ID3D11Texture2D> backbuffer{ nullptr };
    winrt::check_hresult(swapChain->GetBuffer(0, __uuidof(*backbuffer), backbuffer.put_void()));
    return CreateRenderTargetView(device, backbuffer.get());
}

inline auto CreateRasterizerState(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.DepthClipEnable = FALSE;

    winrt::com_ptr<ID3D11RasterizerState> state{ nullptr };
    winrt::check_hresult(device->CreateRasterizerState(&desc, state.put()));
    return state;
}

inline auto CreateViewport(int width, int height)
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    return viewport;
}

inline auto CreateBlendState(ID3D11Device* device)
{
    D3D11_BLEND_DESC desc = {};
    desc.AlphaToCoverageEnable = FALSE;
    desc.IndependentBlendEnable = FALSE;
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    winrt::com_ptr<ID3D11BlendState> state{ nullptr };
    winrt::check_hresult(device->CreateBlendState(&desc, state.put()));
    return state;
}

inline auto CreateSamplerState(ID3D11Device* device, D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_MODE address = D3D11_TEXTURE_ADDRESS_WRAP)
{
    D3D11_SAMPLER_DESC desc{};
    desc.Filter = filter;
    desc.AddressU = desc.AddressV = desc.AddressW = address;
    desc.MinLOD = -FLT_MAX;
    desc.MaxLOD = FLT_MAX;

    winrt::com_ptr<ID3D11SamplerState> state = { nullptr };
    winrt::check_hresult(device->CreateSamplerState(&desc, state.put()));
    return state;
}

inline auto CompileShaderFromFile(LPCTSTR path, LPCSTR entry, LPCSTR target)
{
    winrt::com_ptr<ID3DBlob> bin = { nullptr }, error = { nullptr };

    UINT flag = 0;
#ifdef _DEBUG
    flag |= D3DCOMPILE_DEBUG;
#endif
    auto hr = D3DCompileFromFile(path, nullptr, nullptr, entry, target, flag, 0, bin.put(), error.put());
    if (FAILED(hr)) {
        DLOG(L"Failed to Compile Shader\n%s(%S : %S)\n%S\n", path, entry, target, error->GetBufferPointer());
        winrt::check_hresult(hr);
    }

    return bin;
}

inline auto CompileShaderFromResource(LPCTSTR name, LPCSTR entry, LPCSTR target)
{
    auto [src, size] = LoadResourcePtr<const char*>(name, TEXT("SHADER"));
    winrt::check_pointer(src);

    winrt::com_ptr<ID3DBlob> bin = { nullptr }, error = { nullptr };

    UINT flag = 0;
    auto hr = D3DCompile(src, size, entry, NULL, NULL, entry, target, flag, 0, bin.put(), error.put());
    if (FAILED(hr)) {
        DLOG(L"Failed to Compile Shader\n(%S : %S)\n%S\n", entry, target, error->GetBufferPointer());
        winrt::check_hresult(hr);
    }

    return bin;
}

inline auto CreateVertexShader(ID3D11Device* device, int resourceId, LPCSTR entry, const D3D11_INPUT_ELEMENT_DESC* elements = nullptr, UINT numElements = 0, ID3D11InputLayout** inputLayout = nullptr)
{
    auto bin = CompileShaderFromResource(MAKEINTRESOURCE(resourceId), entry, "vs_5_0");
    winrt::com_ptr<ID3D11VertexShader> vs{ nullptr };
    winrt::check_hresult(device->CreateVertexShader(bin->GetBufferPointer(), bin->GetBufferSize(), nullptr, vs.put()));

    if (inputLayout) {
        winrt::check_hresult(device->CreateInputLayout(elements, numElements, bin->GetBufferPointer(), bin->GetBufferSize(), inputLayout));
    }

    return vs;
}

inline auto CreatePixelShader(ID3D11Device* device, int resourceId, LPCSTR entry)
{
    auto bin = CompileShaderFromResource(MAKEINTRESOURCE(resourceId), entry, "ps_5_0");
    winrt::com_ptr<ID3D11PixelShader> ps{ nullptr };
    winrt::check_hresult(device->CreatePixelShader(bin->GetBufferPointer(), bin->GetBufferSize(), nullptr, ps.put()));
    return ps;
}

inline auto CreateComputeShader(ID3D11Device* device, int resourceId, LPCSTR entry)
{
    auto bin = CompileShaderFromResource(MAKEINTRESOURCE(resourceId), entry, "cs_5_0");
    winrt::com_ptr<ID3D11ComputeShader> cs{ nullptr };
    winrt::check_hresult(device->CreateComputeShader(bin->GetBufferPointer(), bin->GetBufferSize(), nullptr, cs.put()));
    return cs;
}

inline auto CreateBuffer(ID3D11Device* device, int size, int stride, D3D11_USAGE usage, UINT bindFlag, UINT cpuAccess, const void* initial)
{
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = size;
    desc.Usage = usage;
    desc.BindFlags = bindFlag;
    desc.CPUAccessFlags = cpuAccess;
    desc.StructureByteStride = stride;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = initial;
    data.SysMemPitch = size;
    data.SysMemSlicePitch = size;

    winrt::com_ptr<ID3D11Buffer> buffer{ nullptr };
    winrt::check_hresult(device->CreateBuffer(&desc, &data, buffer.put()));
    return buffer;
}

inline auto CreateVertexBuffer(ID3D11Device* device, int num, int stride, const void* src)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = stride * num;
    desc.Usage = src ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = src ? 0 : D3D11_CPU_ACCESS_WRITE;
    desc.StructureByteStride = stride;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = src;
    data.SysMemPitch = data.SysMemSlicePitch = stride * num;

    winrt::com_ptr<ID3D11Buffer> buff{ nullptr };
    winrt::check_hresult(device->CreateBuffer(&desc, src ? &data : nullptr, buff.put()));
    return buff;
}

inline auto CreateConstantBuffer(ID3D11Device* device, int stride, int num = 1)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = stride;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.StructureByteStride = stride * num;
    
    winrt::com_ptr<ID3D11Buffer> buff{ nullptr };
    winrt::check_hresult(device->CreateBuffer(&desc, nullptr, buff.put()));
    return buff;
}

inline auto CreateUnorderedAccessBuffer(ID3D11Device* device, int stride, int num, UINT exBindFlag = 0, UINT misc = 0)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = stride * num;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | exBindFlag;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = misc;
    desc.StructureByteStride = stride;

    winrt::com_ptr<ID3D11Buffer> buff{ nullptr };
    winrt::check_hresult(device->CreateBuffer(&desc, nullptr, buff.put()));
    return buff;
}

inline auto CreateTexture2D(ID3D11Device* device, int width, int height, DXGI_FORMAT format, D3D11_USAGE usage, UINT bindFlag, UINT cpuAccess = 0, const void* src = nullptr, int pitch = 0)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = usage;
    desc.BindFlags = bindFlag;
    desc.CPUAccessFlags = cpuAccess;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = src;
    data.SysMemPitch = pitch;
    data.SysMemSlicePitch = pitch * height;

    winrt::com_ptr<ID3D11Texture2D> texture{ nullptr };
    winrt::check_hresult(device->CreateTexture2D(&desc, src ? &data : nullptr, texture.put()));
    return texture;
}

inline auto CreateShaderResourceView(ID3D11Device* device, winrt::com_ptr<ID3D11Buffer>& buffer, int elemSize, int elemNum, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
{
    D3D11_BUFFER_DESC desc = {};
    buffer->GetDesc(&desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC view{};
    view.Format = format;
    view.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    view.Buffer.ElementWidth = elemSize;
    view.Buffer.NumElements = elemNum;

    winrt::com_ptr<ID3D11ShaderResourceView> srv{};
    winrt::check_hresult(device->CreateShaderResourceView(buffer.get(), &view, srv.put()));
    return srv;
}

inline auto CreateShaderResourceView(ID3D11Device* device, winrt::com_ptr<ID3D11Texture2D>& texture)
{
    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC view{};
    view.Format = desc.Format;
    view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    view.Texture2D.MipLevels = desc.MipLevels;

    winrt::com_ptr<ID3D11ShaderResourceView> srv{};
    winrt::check_hresult(device->CreateShaderResourceView(texture.get(), &view, srv.put()));
    return srv;
}

inline auto CreateUnorderedAccessView(ID3D11Device* device, winrt::com_ptr<ID3D11Buffer>& buffer, int numElems, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
{
    D3D11_BUFFER_DESC desc = {};
    buffer->GetDesc(&desc);

    D3D11_UNORDERED_ACCESS_VIEW_DESC view = {};
    view.Format = format;
    view.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    view.Buffer.NumElements = numElems;

    winrt::com_ptr<ID3D11UnorderedAccessView> uav{};
    winrt::check_hresult(device->CreateUnorderedAccessView(buffer.get(), &view, uav.put()));
    return uav;
}

inline auto CreateUnorderedAccessView(ID3D11Device* device, winrt::com_ptr<ID3D11Texture2D>& tex)
{
    D3D11_TEXTURE2D_DESC desc = {};
    tex->GetDesc(&desc);

    D3D11_UNORDERED_ACCESS_VIEW_DESC view = {};
    view.Format = desc.Format;
    view.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

    winrt::com_ptr<ID3D11UnorderedAccessView> uav{};
    winrt::check_hresult(device->CreateUnorderedAccessView(tex.get(), &view, uav.put()));
    return uav;
}

inline void WriteResource(ID3D11DeviceContext* context, ID3D11Resource* resource, const void* src, const int size)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    winrt::check_hresult(context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

    memcpy_s(mapped.pData, size, src, size);
    context->Unmap(resource, 0);
}

inline void UpdateResource(ID3D11DeviceContext* context, winrt::com_ptr<ID3D11Buffer>& buffer, const void* src, const int size)
{
    context->UpdateSubresource(buffer.get(), 0, nullptr, src, size, size);
}

template<class DescTy, class ObjTy>
inline auto GetDesc(const ObjTy& obj)
{
    DescTy desc = {};
    obj->GetDesc(&desc);
    return desc;
}

template <typename T>
inline auto GetDXGIInterfaceFromObject(winrt::Windows::Foundation::IInspectable const& object)
{
    auto access = object.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
    winrt::com_ptr<T> result;
    winrt::check_hresult(access->GetInterface(__uuidof(*result), result.put_void()));
    return result;
}

#ifdef USE_DIRECTCOMPOSITION
//
// DirectComposition
//
inline auto CreateCompositionDevice(ID3D11Device* d3dDevice)
{
    auto dxgiDevice = d3dDevice.as<IDXGIDevice2>();

    winrt::com_ptr<IDCompositionDevice> device{ nullptr };
    winrt::check_hresult(DCompositionCreateDevice(dxgiDevice.get(), __uuidof(*device), device.put_void()));
    return device;
}

inline auto CreateCompositionTargetForHwnd(winrt::com_ptr<IDCompositionDevice>& device, HWND hwnd)
{
    winrt::com_ptr<IDCompositionTarget> target{ nullptr };
    winrt::check_hresult(device->CreateTargetForHwnd(hwnd, TRUE, target.put()));
    return target;
}

inline auto CreateCompositionVisual(winrt::com_ptr<IDCompositionDevice>& device)
{
    winrt::com_ptr<IDCompositionVisual> visual{ nullptr };
    winrt::check_hresult(device->CreateVisual(visual.put()));
    return visual;
}

#else defined USE_WINRTCOMPOSITION
//
// WinRT.UI.Composition
//
inline auto CreateDispatcherQueueController()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_STA
    };

    winrt::Windows::System::DispatcherQueueController controller{ nullptr };
    winrt::check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(winrt::put_abi(controller))));
    return controller;
}

inline auto CreateDesktopWindowTarget(winrt::Windows::UI::Composition::Compositor& compositor, HWND hwnd)
{
    namespace abi = ABI::Windows::UI::Composition::Desktop;
    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target{ nullptr };
    winrt::check_hresult(interop->CreateDesktopWindowTarget(hwnd, false, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))));
    return target;
}

inline auto CreateCompositionSurfaceForSwapChain(winrt::Windows::UI::Composition::Compositor& compositor, ::IUnknown* swapChain)
{
    using ICompositionSurface = winrt::Windows::UI::Composition::ICompositionSurface;
    ICompositionSurface surface{ nullptr };
    auto compositorInterop = compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>();
    winrt::com_ptr<ABI::Windows::UI::Composition::ICompositionSurface> surfaceInterop;
    winrt::check_hresult(compositorInterop->CreateCompositionSurfaceForSwapChain(swapChain, surfaceInterop.put()));
    winrt::check_hresult(surfaceInterop->QueryInterface(winrt::guid_of<ICompositionSurface>(), reinterpret_cast<void**>(winrt::put_abi(surface))));
    return surface;
}
#endif

//
// Direct2D
//
inline auto CreateD2DFactory(D2D1_FACTORY_TYPE type = D2D1_FACTORY_TYPE_SINGLE_THREADED)
{
    winrt::com_ptr<ID2D1Factory> factory{};
    winrt::check_hresult(D2D1CreateFactory(type, factory.put()));
    return factory.as<ID2D1Factory7>();
}

inline auto CreateD2DDevice(ID2D1Factory1* factory, IDXGIDevice* dxgiDevice)
{
    winrt::com_ptr<ID2D1Device> device{};
    winrt::check_hresult(factory->CreateDevice(dxgiDevice, device.put()));
    return device.as<ID2D1Device6>();
}

inline auto CreateD2DRenderTarget(ID2D1Factory* factory, IDXGISurface* surface, float dpiX = 96.f, float dpiY = 96.f)
{
    winrt::com_ptr<ID2D1RenderTarget> renderTarget{};
    auto props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);
    winrt::check_hresult(factory->CreateDxgiSurfaceRenderTarget(surface, props, renderTarget.put()));
    return renderTarget;
}

inline auto CreateD2DRenderTarget(ID2D1Factory* factory, IDXGISwapChain* swapChain, float dpiX = 96.f, float dpiY = 96.f)
{
    winrt::com_ptr<IDXGISurface> backbuffer{};
    winrt::check_hresult(swapChain->GetBuffer(0, __uuidof(*backbuffer), backbuffer.put_void()));
    return CreateD2DRenderTarget(factory, backbuffer.get(), dpiX, dpiY);
}

inline auto CreateD2DColorBrush(ID2D1RenderTarget* renderTarget, UINT32 color, float alpha = 1.f)
{
    winrt::com_ptr<ID2D1SolidColorBrush> brush{};
    renderTarget->CreateSolidColorBrush(D2D1::ColorF(color, alpha), brush.put());
    return brush;
}

//
// DirectWrite
//
inline auto CreateDWriteFactory(DWRITE_FACTORY_TYPE type = DWRITE_FACTORY_TYPE_SHARED)
{
    winrt::com_ptr<IDWriteFactory7> factory{};
    winrt::check_hresult(DWriteCreateFactory(type, __uuidof(*factory), reinterpret_cast<IUnknown**>(factory.put_void())));
    return factory;
}

inline auto CreateDWriteTextFormat(IDWriteFactory* factory, LPCWSTR fontName, float size = 24.f, DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, LPCWSTR location = nullptr)
{
    winrt::com_ptr<IDWriteTextFormat> textFormat;
    winrt::check_hresult(factory->CreateTextFormat(fontName, nullptr, weight,style,stretch, size, location ? location : L"ja-JP", textFormat.put()));
    return textFormat.as<IDWriteTextFormat3>();
}

inline int DivideAndRoundUp(int dividend, int divisor)
{
    return ((dividend + divisor - 1) / divisor);
}

inline int DivideAndRoundDown(int dividend, int divisor)
{
    return (dividend / divisor);
}
