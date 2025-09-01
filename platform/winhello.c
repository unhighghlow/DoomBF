#include <windows.h>
#include <stdio.h>
#include <mod_doom.h>

#define WINHELLO_WND_CLASS L"HelloWndClass"

//#define HELLOWIN_DOOM_NONE
#define HELLOWIN_DOOM_LIB
//#define HELLOWIN_DOOM_DLL
//#define HELLOWIN_DOOM_DRIVER
//#define HELLOWIN_DOOM_PIPE
#define HELLOWIN_DOOM_CONSOLE


#ifdef HELLOWIN_DOOM_CONSOLE
#include <doom_ioctl.h>
#endif

void MyDialogTest(HWND hWnd);

int g_DoomX = 0;
int g_DoomY = 0;
int g_HelloWidth = 800;
int g_HelloHeight = 600;
HBITMAP g_HelloBitmap;
char* g_pHelloPixelsBuffer = NULL;
BITMAPINFO BitmapInfo;
HWND g_HelloButton = NULL;
HWND g_HelloMain = NULL;
HANDLE h_driver = NULL;

typedef void (*FuncMiniDoomIteration)(char* p_imgBuf, int x, int y, int imgWidth, int imgHeight);
typedef void (*FuncMiniDoomKeyDown)(int vKey);
typedef void (*FuncMiniDoomKeyUp)(int vKey);
typedef void (*FuncMiniDoomStart)(const char* homeDir, const char* fileName);
typedef void (*FuncPipeDoomServerCreate)(DWORD timeout, LPCTSTR pipeName);
typedef void (*FuncConsoleDoomSetupConsole)(void);

FuncMiniDoomIteration pMiniDoomIteration;
FuncMiniDoomKeyDown pMiniDoomKeyDown;
FuncMiniDoomKeyUp pMiniDoomKeyUp;
FuncMiniDoomStart pMiniDoomStart;
FuncPipeDoomServerCreate pPipeDoomServerCreate;
FuncConsoleDoomSetupConsole pConsoleDoomSetupConsole;

void AAAConsoleDoomSetupConsole()
{
    int result = 1;
    FILE* fp = NULL;
    int console_buf_len = 1024;

    if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
    {
        result = 0;
    }
    else
    {
        setvbuf(stdin, NULL, _IONBF, 0);
    }
    if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
    {
        result = 0;
    }
    else
    {
        setvbuf(stdout, NULL, _IONBF, 0);
    }

    if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
    {
        result = 0;
    }
    else
    {
        setvbuf(stderr, NULL, _IONBF, 0);
    }
    FreeConsole();

    if (AllocConsole())
    {
        // Set the screen buffer to be big enough to scroll some text
        CONSOLE_SCREEN_BUFFER_INFO conInfo;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
        if (conInfo.dwSize.Y < console_buf_len)
        {
            conInfo.dwSize.Y = console_buf_len;
        }
        SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);

        if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
        {
            if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
            {
                result = 0;
            }
            else
            {
                setvbuf(stdin, NULL, _IONBF, 0);
            }
        }
        if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
        {
            if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
            {
                result = 0;
            }
            else
            {
                setvbuf(stdout, NULL, _IONBF, 0);
            }
        }
        if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
        {
            if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
            {
                result = 0;
            }
            else
            {
                setvbuf(stderr, NULL, _IONBF, 0);
            }
        }
    }
}


void HelloPutPixel(char* p_imgBuf, int x, int y, int imgWidth, int imgHeight, int color)
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



void HelloMiniDoomIteration(char* p_imgBuf, int x, int y, int imgWidth, int imgHeight)
{
#if defined(HELLOWIN_DOOM_DRIVER)
    static int framebuffer[320 * 200 * 4] = { 0 };
    BOOL success = 0;
    DWORD returned = 0;
    int _x, _y;
#endif


#if defined(HELLOWIN_DOOM_DRIVER)
if (h_driver != INVALID_HANDLE_VALUE && h_driver != NULL)
{
    memset(framebuffer, 0xFF, sizeof(framebuffer));
    success = DeviceIoControl(h_driver,
        IOCTL_DOOMDRV_SCREEN, // control code
        NULL, 0, // input buffer and length
        &framebuffer[0], sizeof(framebuffer), // output buffer and length
        &returned, NULL);
    printf("[Test Draw 0x%x]", success);
    //if (success)
    {
        for (_y = 0; _y < 200; _y++)
        {
            for (_x = 0; _x < 320; _x++)
            {
                HelloPutPixel(p_imgBuf, _x + x, _y + y, imgWidth, imgHeight, framebuffer[x + y * 320]);
            }
        }
    }
}
#elif defined(HELLOWIN_DOOM_LIB)
    MiniDoomIteration(p_imgBuf, x, y, imgWidth, imgHeight);
#elif defined(HELLOWIN_DOOM_DLL)
    if (pMiniDoomIteration)
    {
        pMiniDoomIteration(p_imgBuf, x, y, imgWidth, imgHeight);
    }
#endif
}

void HelloMiniDoomKeyDown(int vKey)
{
#if defined(HELLOWIN_DOOM_DRIVER)
    BOOL success = 0;
    DWORD returned = 0;
    int value = 0;
#endif

#if defined(HELLOWIN_DOOM_DRIVER)
if (h_driver != INVALID_HANDLE_VALUE && h_driver != NULL)
{
    value = MiniDoomKeyTranslate(vKey);
    success = DeviceIoControl(h_driver,
        IOCTL_DOOMDRV_KEY_DOWN, // control code
        &value, sizeof(value), // input buffer and length
        NULL, 0, // output buffer and length
        &returned, NULL);
}
#elif defined(HELLOWIN_DOOM_LIB)
    MiniDoomKeyDown(vKey);
#elif defined(HELLOWIN_DOOM_DLL)
    if (pMiniDoomKeyDown)
    {
        pMiniDoomKeyDown(vKey);
    }
#endif
}

void HelloMiniDoomKeyUp(int vKey)
{
#if defined(HELLOWIN_DOOM_DRIVER)
    BOOL success = 0;
    DWORD returned = 0;
    int value = 0;
#endif

#if defined(HELLOWIN_DOOM_DRIVER)
if (h_driver != INVALID_HANDLE_VALUE && h_driver != NULL)
{
    value = MiniDoomKeyTranslate(vKey);
    success = DeviceIoControl(h_driver,
        IOCTL_DOOMDRV_KEY_UP, // control code
        &value, sizeof(value), // input buffer and length
        NULL, 0, // output buffer and length
        &returned, NULL);
}
#elif defined(HELLOWIN_DOOM_LIB)
    MiniDoomKeyUp(vKey);
#elif defined(HELLOWIN_DOOM_DLL)
    if (pMiniDoomKeyUp)
    {
        pMiniDoomKeyUp(vKey);
    }
#endif
}

void HelloMiniDoomStart(const char* homeDir, const char* fileName)
{
    AAAConsoleDoomSetupConsole();

#if defined(HELLOWIN_DOOM_DRIVER)
h_driver = CreateFile(L"\\\\.\\" DOOMDRV, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
printf("[RESULT!!!! %llx]", h_driver);
#elif defined(HELLOWIN_DOOM_LIB)
    MiniDoomStart(homeDir, fileName);
#elif defined(HELLOWIN_DOOM_DLL)
    if (pMiniDoomStart)
    {
        pMiniDoomStart(homeDir, fileName);
    }
#endif

}

void HelloPipeDoomServerCreate(DWORD timeout, LPCTSTR pipeName)
{
#ifdef HELLOWIN_DOOM_LIB
    PipeDoomServerCreate(200, MINI_DOOM_PIPE_NAME);
#elif defined(HELLOWIN_DOOM_DLL)
    if (pPipeDoomServerCreate)
    {
        pPipeDoomServerCreate(200, MINI_DOOM_PIPE_NAME);
    }
#endif
}

void HelloConsoleDoomSetupConsole(void)
{
#ifndef HELLOWIN_DOOM_NONE
#ifdef HELLOWIN_DOOM_LIB
    ConsoleDoomSetupConsole();
#else
    if (pConsoleDoomSetupConsole)
    {
        pConsoleDoomSetupConsole();
    }
#endif
#endif
}



void Hello_PutPixel(int x, int y, int r, int g, int b)
{
    if (x < 0 || x >= g_HelloWidth)
    {
        return;
    }
    if (y < 0 || y >= g_HelloHeight)
    {
        return;
    }

    g_pHelloPixelsBuffer[y * g_HelloWidth * 4 + x * 4 + 0] = b;
    g_pHelloPixelsBuffer[y * g_HelloWidth * 4 + x * 4 + 1] = g;
    g_pHelloPixelsBuffer[y * g_HelloWidth * 4 + x * 4 + 2] = r;
    g_pHelloPixelsBuffer[y * g_HelloWidth * 4 + x * 4 + 3] = 0xFF;
}

void Hello_DoDraw(char* p_pixels, int width, int height, int time)
{
    if (p_pixels)
    {
        RtlZeroMemory(p_pixels, width * height * 4);
    }

    HelloMiniDoomIteration(p_pixels, g_DoomX, g_DoomY, width, height);
}

int Hello_Repaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HDC rdc;

    if (g_pHelloPixelsBuffer != NULL)
    {
        hdc = BeginPaint(hWnd, &ps);
        rdc = CreateCompatibleDC(hdc);
        Hello_DoDraw(g_pHelloPixelsBuffer, g_HelloWidth, g_HelloHeight, (int)GetTickCount());

        SelectObject(rdc, g_HelloBitmap);
        BitBlt(hdc, 0, 0, g_HelloWidth, g_HelloHeight, rdc, 0, 0, SRCCOPY);
        DeleteDC(rdc);
        EndPaint(hWnd, &ps);
    }
    return 0;
}

VOID WindowDoomSetupForWindow(HWND hwnd);
HWND WindowDoomGetMainWindow(void);

LRESULT CALLBACK Hello_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
#ifndef HELLOWIN_DOOM_NONE
        Hello_Repaint(hWnd);
#endif
        break;
    case WM_COMMAND:
        if (g_HelloButton == lParam)
        {
            MyDialogTest(hWnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        exit(0);
        break;

    case WM_MOUSEMOVE:
        g_DoomX = (unsigned int)LOWORD(lParam);
        g_DoomY = (unsigned int)HIWORD(lParam);
        break;
#ifndef HELLOWIN_DOOM_NONE
    case WM_KEYDOWN:
        HelloMiniDoomKeyDown(wParam);
        break;
    case WM_KEYUP:
        HelloMiniDoomKeyUp(wParam);
        break;
#endif
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd)
{
    WNDCLASSEX wcex;
    HRESULT hres = 0;
    HWND hWnd;
#ifndef HELLOWIN_DOOM_LIB
#ifndef HELLOWIN_DOOM_NONE
    HMODULE mod = LoadLibrary(TEXT("mod_doom.dll"));
    
    if (mod)
    {
        pMiniDoomIteration = (FuncMiniDoomIteration)GetProcAddress(mod, "MiniDoomIteration");
        pMiniDoomKeyDown = (FuncMiniDoomKeyDown)GetProcAddress(mod, "MiniDoomKeyDown");
        pMiniDoomKeyUp = (FuncMiniDoomKeyUp)GetProcAddress(mod, "MiniDoomKeyUp");

        pMiniDoomStart = (FuncMiniDoomStart)GetProcAddress(mod, "MiniDoomStart");
        pPipeDoomServerCreate = (FuncPipeDoomServerCreate)GetProcAddress(mod, "PipeDoomServerCreate");
        pConsoleDoomSetupConsole = (FuncConsoleDoomSetupConsole)GetProcAddress(mod, "ConsoleDoomSetupConsole");
    }
#endif
#endif

#ifndef HELLOWIN_CONSOLE
    //HelloConsoleDoomSetupConsole();
#endif
    
   
    printf("Hello!"); fflush(stdout);
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Hello_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wcex.lpszClassName = WINHELLO_WND_CLASS;

    // Register Class  <- WndProc
    hres = RegisterClassEx(&wcex);
    
    // HWND <- Create Windows (Class)
    hWnd = CreateWindow(WINHELLO_WND_CLASS, L"Hello window", WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);
    g_HelloMain = hWnd;

    memset(&BitmapInfo, 0, sizeof(BitmapInfo));
    BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth = g_HelloWidth;
    BitmapInfo.bmiHeader.biHeight = -((int)g_HelloHeight);
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    BitmapInfo.bmiHeader.biSizeImage = (g_HelloWidth * g_HelloHeight)
        * (BitmapInfo.bmiHeader.biBitCount / 8);

    g_HelloBitmap = CreateDIBSection(NULL, &BitmapInfo, DIB_RGB_COLORS,
        &g_pHelloPixelsBuffer, 0, 0);
    memset(g_pHelloPixelsBuffer, 0, BitmapInfo.bmiHeader.biSizeImage);


#ifndef HELLOWIN_DOOM_NONE
    HelloMiniDoomStart("D:\\", "Doom.wad");
#ifdef HELLOWIN_DOOM_PIPE
    HelloPipeDoomServerCreate(200, MINI_DOOM_PIPE_NAME);
#endif
#endif


    g_HelloButton = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Do magic",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10,         // y position 
        100,        // Button width
        100,        // Button height
        hWnd,     // Parent window
        NULL,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);      // Pointer not needed.


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
#ifndef HELLOWIN_DOOM_NONE
        else
        {
            InvalidateRect(hWnd, NULL, FALSE);
        }
#endif
    }
    return 0;
}

#include "resource.h"

INT_PTR CALLBACK DialogTest_Func(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        if (wParam == IDC_BUTTON1)
        {
            MessageBox(hWnd, "Hello Yes?", "IMPORTANT!", MB_OK);
        }
        break;
    default:
        return 0;
    }
    return 0;
}

void MyDialogTest(HWND hWnd)
{
    HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    DialogBox(hinst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, &DialogTest_Func);
    int i = 0;

}


