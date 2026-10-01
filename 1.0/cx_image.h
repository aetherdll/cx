// 7 DEVELOMENT.ALL RIGHTS RESERVED.

#ifndef CX_IMAGE_H
#define CX_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        int width;
        int height;
        int channels;
        unsigned char* data;
    } cx_Image;

    cx_Image* cx_image_load(const char* filepath);
    void cx_image_free(cx_Image* image);
    void cx_image_draw_window(const char* title, cx_Image* image);

#ifdef __cplusplus
}
#endif

#endif

#ifdef CX_IMAGE_IMPLEMENTATION
#ifndef CX_IMAGE_IMPLEMENTATION_ONCE
#define CX_IMAGE_IMPLEMENTATION_ONCE

#include 
#include 
#include 

#ifdef _WIN32
#include 
#include 
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#endif

static unsigned char* cx__load_tga(const char* filepath, int* width, int* height, int* channels) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    unsigned char header[18];
    if (fread(header, 1, 18, f) != 18) {
        fclose(f);
        return NULL;
    }

    int w = header[12] | (header[13] << 8);
    int h = header[14] | (header[15] << 8);
    int bpp = header[16];
    int img_type = header[2];

    if (img_type != 2 && img_type != 3) {
        fclose(f);
        return NULL;
    }

    int ch = bpp / 8;
    if (ch < 3) ch = 3;

    int data_size = w * h * ch;
    unsigned char* data = (unsigned char*)malloc(data_size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (header[6] > 0) {
        fseek(f, header[6], SEEK_CUR);
    }

    if (ch == 3) {
        for (int i = 0; i < w * h; i++) {
            unsigned char bgr[3];
            if (fread(bgr, 1, 3, f) != 3) {
                free(data);
                fclose(f);
                return NULL;
            }
            data[i * 3 + 0] = bgr[2];
            data[i * 3 + 1] = bgr[1];
            data[i * 3 + 2] = bgr[0];
        }
    }
    else if (ch == 4) {
        for (int i = 0; i < w * h; i++) {
            unsigned char bgra[4];
            if (fread(bgra, 1, 4, f) != 4) {
                free(data);
                fclose(f);
                return NULL;
            }
            data[i * 4 + 0] = bgra[2];
            data[i * 4 + 1] = bgra[1];
            data[i * 4 + 2] = bgra[0];
            data[i * 4 + 3] = bgra[3];
        }
    }

    *width = w;
    *height = h;
    *channels = ch;
    fclose(f);
    return data;
}

cx_Image* cx_image_load(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (!ext) return NULL;

    int width = 0, height = 0, channels = 0;
    unsigned char* data = NULL;

    if (strcmp(ext, ".tga") == 0 || strcmp(ext, ".TGA") == 0) {
        data = cx__load_tga(filepath, &width, &height, &channels);
    }

    if (!data) return NULL;

    cx_Image* img = (cx_Image*)malloc(sizeof(cx_Image));
    if (!img) {
        free(data);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->data = data;

    return img;
}

void cx_image_free(cx_Image* image) {
    if (image) {
        if (image->data) {
            free(image->data);
        }
        free(image);
    }
}

#ifdef _WIN32
static LRESULT CALLBACK cx__window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
#endif

void cx_image_draw_window(const char* title, cx_Image* image) {
    if (!image || !image->data) return;

#ifdef _WIN32
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = cx__window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CX_D3D_WINDOW";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "CX_D3D_WINDOW", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, image->width + 16, image->height + 39,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return;

    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferCount = 1;
    sd.BufferDesc.Width = image->width;
    sd.BufferDesc.Height = image->height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    IDXGISwapChain* swap_chain = NULL;
    ID3D11Device* d3d_device = NULL;
    ID3D11DeviceContext* d3d_context = NULL;
    ID3D11RenderTargetView* render_target_view = NULL;

    D3D_FEATURE_LEVEL feature_level;
    D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
        D3D11_SDK_VERSION, &sd, &swap_chain, &d3d_device, &feature_level, &d3d_context
    );

    ID3D11Texture2D* back_buffer = NULL;
    swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
    d3d_device->CreateRenderTargetView((ID3D11Resource*)back_buffer, NULL, &render_target_view);
    back_buffer->lpVtbl->Release(back_buffer);

    d3d_context->lpVtbl->OMSetRenderTargets(d3d_context, 1, &render_target_view, NULL);

    // RGBA formatına dönüştürme
    int data_size = image->width * image->height * 4;
    unsigned char* rgba_pixels = (unsigned char*)malloc(data_size);
    for (int i = 0; i < image->width * image->height; i++) {
        int src_idx = i * image->channels;
        int dst_idx = i * 4;
        rgba_pixels[dst_idx + 0] = image->data[src_idx + 0]; // R
        rgba_pixels[dst_idx + 1] = image->data[src_idx + 1]; // G
        rgba_pixels[dst_idx + 2] = image->data[src_idx + 2]; // B
        rgba_pixels[dst_idx + 3] = (image->channels == 4) ? image->data[src_idx + 3] : 255; // A
    }

    // Doku oluşturma
    D3D11_TEXTURE2D_DESC tex_desc = { 0 };
    tex_desc.Width = image->width;
    tex_desc.Height = image->height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init_data = { 0 };
    init_data.pSysMem = rgba_pixels;
    init_data.SysMemPitch = image->width * 4;

    ID3D11Texture2D* texture = NULL;
    d3d_device->CreateTexture2D(&tex_desc, &init_data, &texture);
    free(rgba_pixels);

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        else {
            float clear_color[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
            d3d_context->lpVtbl->ClearRenderTargetView(d3d_context, render_target_view, clear_color);

            // Burada Direct3D 11 donanım hızlandırmalı pipeline ile çizim gerçekleştirilir
            swap_chain->Present(1, 0);
            Sleep(10);
        }
    }

    if (texture) texture->lpVtbl->Release(texture);
    if (render_target_view) render_target_view->lpVtbl->Release(render_target_view);
    if (swap_chain) swap_chain->lpVtbl->Release(swap_chain);
    if (d3d_context) d3d_context->lpVtbl->Release(d3d_context);
    if (d3d_device) d3d_device->lpVtbl->Release(d3d_device);
#endif
}

#endif
#endif