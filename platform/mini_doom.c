#include "mod_doom.h"
#include "PureDOOM.h"

char* g_MiniDoomArgvw[3] = { "doom", "-file", NULL };
char g_MiniDoomHome[MAX_PATH];

VOID* MiniDoomOpen(const char* File, const char* Mode)
{
    return fopen(File, Mode);
}

VOID MiniDoomClose(VOID* Handle)
{
    if (Handle == NULL)
        return;
    fclose(Handle);
}

INT MiniDoomRead(VOID* Handle, void* Buf, int Count)
{
    if (Handle == NULL)
        return 0;
    return fread(Buf, 1, Count, (FILE*)Handle);
}

INT MiniDoomTell(VOID* Handle)
{
    if (Handle == NULL)
        return 0;

    return ftell((FILE*)Handle);
}

INT MiniDoomEof(VOID* Handle)
{
    if (Handle == NULL)
        return -1;
    return feof((FILE*)Handle);
}

INT MiniDoomSeek(VOID* Handle, int Offset, doom_seek_t SeekType)
{
    if (Handle == NULL)
        return -1;
    //DOOM_SEEK_CUR
    return fseek((FILE*)Handle, Offset, SeekType);
}

void* MiniDoomMalloc(int size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void MiniDoomFree(void* Mem)
{
    HeapFree(GetProcessHeap(), 0, Mem);
    return;
}

void MiniDoomPrint(const char* String)
{
    printf("[DOOM]: %s\n", String);
}

void MiniDoomExit(int code)
{
    printf("[DOOM]: Exiting with code %d\n", code);
    //Running = FALSE;
}

char* MiniDoomGetEnv(const char* Name) 
{
    if (!strcmp(Name, "HOME"))
    {
        return g_MiniDoomHome;
    }
    return NULL;
}

void MiniDoomGetTime(int* sec, int* usec)
{
    ULONGLONG time64 = GetTickCount();
    *sec = (int)(time64 / 1000);
    *usec = (int)((time64 % 1000) * 1000);
}

int MiniDoomKeyTranslate(int key)
{
    switch (key)
    {
    case VK_RETURN:  return DOOM_KEY_ENTER;
    case VK_LEFT:    return DOOM_KEY_LEFT_ARROW;
    case VK_RIGHT:   return DOOM_KEY_RIGHT_ARROW;
    case VK_UP:      return DOOM_KEY_UP_ARROW;
    case VK_DOWN:    return DOOM_KEY_DOWN_ARROW;
    case VK_SPACE:   return DOOM_KEY_SPACE;
    case VK_CONTROL: return DOOM_KEY_CTRL;
    case VK_ESCAPE:  return DOOM_KEY_ESCAPE;
    case 'Y':        return DOOM_KEY_Y;
    }
    return DOOM_KEY_UNKNOWN;
}

void MiniDoomKeyUp(int key)
{
    switch (key)
    {
    case VK_RETURN:  doom_key_up(DOOM_KEY_ENTER); break;
    case VK_LEFT:    doom_key_up(DOOM_KEY_LEFT_ARROW); break;
    case VK_RIGHT:   doom_key_up(DOOM_KEY_RIGHT_ARROW); break;
    case VK_UP:      doom_key_up(DOOM_KEY_UP_ARROW); break;
    case VK_DOWN:    doom_key_up(DOOM_KEY_DOWN_ARROW); break;
    case VK_SPACE:   doom_key_up(DOOM_KEY_SPACE); break;
    case VK_CONTROL: doom_key_up(DOOM_KEY_CTRL); break;
    case VK_ESCAPE:  doom_key_up(DOOM_KEY_ESCAPE); break;
    case 'Y':        doom_key_up(DOOM_KEY_Y); break;
    }
}

void MiniDoomKeyDown(int key)
{
    switch (key)
    {
    case VK_RETURN:  doom_key_down(DOOM_KEY_ENTER); break;
    case VK_LEFT:    doom_key_down(DOOM_KEY_LEFT_ARROW); break;
    case VK_RIGHT:   doom_key_down(DOOM_KEY_RIGHT_ARROW); break;
    case VK_UP:      doom_key_down(DOOM_KEY_UP_ARROW); break;
    case VK_DOWN:    doom_key_down(DOOM_KEY_DOWN_ARROW); break;
    case VK_SPACE:   doom_key_down(DOOM_KEY_SPACE); break;
    case VK_CONTROL: doom_key_down(DOOM_KEY_CTRL); break;
    case VK_ESCAPE:  doom_key_down(DOOM_KEY_ESCAPE); break;
    case 'Y':        doom_key_down(DOOM_KEY_Y); break;
    }
}

void MiniDoomPutPixel(char* p_imgBuf, int x, int y, int imgWidth, int imgHeight, int color)
{
    int* p_pixels = (int*)p_imgBuf;
    if (p_imgBuf == NULL)
    {
        return;
    }
    if (x < 0 || x >= imgWidth)
    {
        return;
    }
    if (y < 0 || y >= imgHeight)
    {
        return;
    }
    p_pixels[y * imgWidth + x] = color;
}

void MiniDoomIteration(char* p_imgBuf, int _x, int _y, int imgWidth, int imgHeight)
{
    int* framebuffer = NULL;
    int x = 0;
    int y = 0;
    doom_update();
    framebuffer = (int*)doom_get_framebuffer(4);

    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 320; x++)
        {
            MiniDoomPutPixel(p_imgBuf, _x + x, _y + y, imgWidth, imgHeight, framebuffer[x + y * 320]);
        }
    }
}

void MiniDoomStart(const char* homeDir, const char *fileName)
{
    if (homeDir)
    {
        _snprintf(g_MiniDoomHome, sizeof(g_MiniDoomHome) - 1, "%s", homeDir);
    }
    else
    {
        _snprintf(g_MiniDoomHome, sizeof(g_MiniDoomHome) - 1, "D:\\");
    }

    doom_set_file_io(MiniDoomOpen, MiniDoomClose, MiniDoomRead, NULL,
        MiniDoomSeek, MiniDoomTell, MiniDoomEof);
    doom_set_malloc(MiniDoomMalloc, MiniDoomFree);
    doom_set_exit(MiniDoomExit);
    doom_set_getenv(MiniDoomGetEnv);
    doom_set_gettime(MiniDoomGetTime);
    doom_set_print(MiniDoomPrint);

    g_MiniDoomArgvw[2] = fileName;
    doom_init(3, g_MiniDoomArgvw, 0);
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

void Win_DoDraw(char* p_pixels, int width, int height, int time)
{
    if (p_pixels)
    {
        memset(p_pixels, 0, width * height * 4);
    }

    MiniDoomIteration(p_pixels, g_DoomX, g_DoomY, width, height);
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
        Win_DoDraw(g_pDoomWinPixelsBuffer, g_DoomWinWidth, g_DoomWinHeight, (int)GetTickCount());

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

int main(int argc, char *argv[])
{
    WNDCLASSEX wcex;
    HRESULT hres = 0;
    HWND hWnd;
   
    printf("Hello!"); fflush(stdout);
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


    MiniDoomStart("", "doom.wad");

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
