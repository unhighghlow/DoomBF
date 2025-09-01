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
    ULONGLONG time64 = GetTickCount64();
    *sec = (int)(time64 / 1000);
    *usec = (int)((time64 % 1000) * 1000);
}

__declspec(dllexport) int MiniDoomKeyTranslate(int key)
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

__declspec(dllexport) void MiniDoomKeyUp(int key)
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

__declspec(dllexport) void MiniDoomKeyDown(int key)
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

__declspec(dllexport) void MiniDoomIteration(char* p_imgBuf, int _x, int _y, int imgWidth, int imgHeight)
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

__declspec(dllexport) void MiniDoomStart(const char* homeDir, const char *fileName)
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

