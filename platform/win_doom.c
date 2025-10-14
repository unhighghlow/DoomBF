
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "doom_env.h"

struct DoomControlRegs g_WinDoomControlRegs;
void *g_DoomHeapAddress = NULL;
unsigned int g_DoomHeapSize = 0;
char *g_DoomWadAddress = NULL;
unsigned int g_DoomWadSize = 0;
struct DoomControlRegs *g_DoomControlRegs = & g_WinDoomControlRegs;

void EnvPutChar(int c)
{
    printf("%c", c);
}

void WinDoomUpdateTime()
{
    ULONGLONG time64 = GetTickCount();
    g_WinDoomControlRegs.time_sec = (int)(time64 / 1000);
    g_WinDoomControlRegs.time_usec = (int)((time64 % 1000) * 1000);
}

void MiniDoomKeyAction(int key, int action)
{
    int k = 0;
    int i = 0;

    switch (key)
    {
    case VK_RETURN:  k = CRT_DOOM_KEY_ENTER; break;
    case VK_LEFT:    k = CRT_DOOM_KEY_LEFT_ARROW; break;
    case VK_RIGHT:   k = CRT_DOOM_KEY_RIGHT_ARROW; break;
    case VK_UP:      k = CRT_DOOM_KEY_UP_ARROW; break;
    case VK_DOWN:    k = CRT_DOOM_KEY_DOWN_ARROW; break;
    case VK_SPACE:   k = CRT_DOOM_KEY_SPACE; break;
    case VK_CONTROL: k = CRT_DOOM_KEY_CTRL; break;
    case VK_ESCAPE:  k = CRT_DOOM_KEY_ESCAPE; break;
    case 'Y':        k = CRT_DOOM_KEY_Y; break;
    }
    if (!k)
    {
        return;
    }

    for (i = 0; i < (sizeof(g_WinDoomControlRegs.keys)/ sizeof(g_WinDoomControlRegs.keys[0])); i++)
    {
        if (!g_WinDoomControlRegs.keys[i].action)
        {
            g_WinDoomControlRegs.keys[i].action = action;
            g_WinDoomControlRegs.keys[i].key = k;
            break;
        }
    }
}

void MiniDoomKeyUp(int key)
{
    MiniDoomKeyAction(key, 2);
}

void MiniDoomKeyDown(int key)
{
    MiniDoomKeyAction(key, 1);
}

#define DOOMWIN_WND_CLASS "DoomWinWndClass"

int g_DoomX = 0;
int g_DoomY = 0;
int g_DoomWinWidth = 640;
int g_DoomWinHeight = 480;
HBITMAP g_DoomWinBitmap;
char* g_pDoomWinPixelsBuffer = NULL;
BITMAPINFO g_DoomWinBitmapInfo;
HWND g_DoomWinMain = NULL;

void Win_PutPixel(int x, int y, int r, int g, int b)
{
    if (x < 0 || x >= g_DoomWinWidth)
    {
        return;
    }
    if (y < 0 || y >= g_DoomWinHeight)
    {
        return;
    }

    g_pDoomWinPixelsBuffer[y * g_DoomWinWidth * 4 + x * 4 + 0] = b;
    g_pDoomWinPixelsBuffer[y * g_DoomWinWidth * 4 + x * 4 + 1] = g;
    g_pDoomWinPixelsBuffer[y * g_DoomWinWidth * 4 + x * 4 + 2] = r;
    g_pDoomWinPixelsBuffer[y * g_DoomWinWidth * 4 + x * 4 + 3] = 0xFF;
}

int Win_Repaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HDC rdc;

    if (g_pDoomWinPixelsBuffer != NULL)
    {
        hdc = BeginPaint(hWnd, &ps);
        rdc = CreateCompatibleDC(hdc);
        
        WinDoomUpdateTime();
        g_WinDoomControlRegs.pixels = g_pDoomWinPixelsBuffer;
        g_WinDoomControlRegs.height = g_DoomWinHeight;
        g_WinDoomControlRegs.width = g_DoomWinWidth;
        
        CrtDoomIteration();

        SelectObject(rdc, g_DoomWinBitmap);
        BitBlt(hdc, 0, 0, g_DoomWinWidth, g_DoomWinHeight, rdc, 0, 0, SRCCOPY);
        DeleteDC(rdc);
        EndPaint(hWnd, &ps);
    }
    return 0;
}

LRESULT CALLBACK Win_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        Win_Repaint(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        exit(0);
        break;

    case WM_MOUSEMOVE:
        //g_DoomX = (unsigned int)LOWORD(lParam);
        //g_DoomY = (unsigned int)HIWORD(lParam);
        break;
    case WM_KEYDOWN:
        MiniDoomKeyDown(wParam);
        break;
    case WM_KEYUP:
        MiniDoomKeyUp(wParam);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int Win_LoadFile(const char *p_file_path) {
    FILE *file;
    long file_size;
    void *buffer;
    
    // Open the file in binary read mode
    file = fopen(p_file_path, "rb");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", p_file_path);
        return -1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        printf("Error: File is empty or invalid size\n");
        fclose(file);
        return -1;
    }
    
    // Allocate memory for the file content
    buffer = malloc(file_size);
    if (buffer == NULL) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return -1;
    }
    
    // Read the entire file into memory
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        printf("Error: Could not read entire file (read %zu of %ld bytes)\n", 
               bytes_read, file_size);
        free(buffer);
        fclose(file);
        return -1;
    }
    
    // Close the file
    fclose(file);
    
    // Set the global variables
    g_DoomWadAddress = buffer;
    g_DoomWadSize = (unsigned int)file_size;
    
    unsigned long long  summ = 0;
   for (unsigned  ii = 0; ii < g_DoomWadSize; ii++)
   {
    summ += 1 + ((unsigned char *)g_DoomWadAddress)[ii];
   }


    printf("Successfully loaded file: %s (%u bytes) %lld code\n", p_file_path, g_DoomWadSize, summ);
    return 0;
}

int main(int argc, char *argv[])
{
    WNDCLASSEX wcex;
    HRESULT hres = 0;
    HWND hWnd;
   
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Win_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wcex.lpszClassName = DOOMWIN_WND_CLASS;

    // Register Class  <- WndProc
    hres = RegisterClassEx(&wcex);
    
    // HWND <- Create Windows (Class)
    hWnd = CreateWindow(DOOMWIN_WND_CLASS, "Doom window", WS_OVERLAPPEDWINDOW,
        0, 0, g_DoomWinWidth, g_DoomWinHeight, NULL, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    g_DoomWinMain = hWnd;

    memset(&g_DoomWinBitmapInfo, 0, sizeof(g_DoomWinBitmapInfo));
    g_DoomWinBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_DoomWinBitmapInfo.bmiHeader.biWidth = g_DoomWinWidth;
    g_DoomWinBitmapInfo.bmiHeader.biHeight = -((int)g_DoomWinHeight);
    g_DoomWinBitmapInfo.bmiHeader.biPlanes = 1;
    g_DoomWinBitmapInfo.bmiHeader.biBitCount = 32;
    g_DoomWinBitmapInfo.bmiHeader.biCompression = BI_RGB;
    g_DoomWinBitmapInfo.bmiHeader.biSizeImage = (g_DoomWinWidth * g_DoomWinHeight)
        * (g_DoomWinBitmapInfo.bmiHeader.biBitCount / 8);

    g_DoomWinBitmap = CreateDIBSection(NULL, &g_DoomWinBitmapInfo, DIB_RGB_COLORS,
        &g_pDoomWinPixelsBuffer, 0, 0);
    memset(g_pDoomWinPixelsBuffer, 0, g_DoomWinBitmapInfo.bmiHeader.biSizeImage);

    g_DoomHeapSize =0x4000000;
    g_DoomHeapAddress = HeapAlloc(GetProcessHeap(), 0, g_DoomHeapSize);
    Win_LoadFile("doom.wad");
    CrtDoomInit();
    printf("INITED!!");
    // Get Messages for window -> Process messages for windwos <- HWND
    while (1)
    {
        MSG msg;
        int res = 0;
        // is there a message to process?
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
    return 0;
}
