

#include "doom_env.h"
#include "doom_crt.h"

#include "PureDOOM.h"

void MiniDoomIteration(char *p_imgBuf, int x, int y, int imgWidth, int imgHeight);
void MiniDoomKeyDown(int vKey);
void MiniDoomKeyUp(int vKey);
void MiniDoomStart(const char* homeDir, const char* fileName);
int MiniDoomKeyTranslate(int key);
void EnvHeapSetup();
void EnvHeapFree(void* pv);
void* EnvHeapMalloc(size_t xWantedSize);

#define DOOM_WAD_FILE "doom.wad"
#define DOOM_WAD_FILE_HANDLE 0x123456

char* g_MiniDoomArgvw[3] = { "doom", "-file", DOOM_WAD_FILE };
char g_MiniDoomHome[128] = {0};
int g_MiniDoomWadDataOffset = 0;


void* MiniDoomOpen(const char* File, const char* Mode)
{
    printf("\n[Try open file] %s", File);	
    if (strcmp(File, "/" DOOM_WAD_FILE) == 0 || strcmp(File, DOOM_WAD_FILE) == 0)
    {
        return (void*)DOOM_WAD_FILE_HANDLE;
    }
    return NULL;
}

void MiniDoomClose(void* Handle)
{
}

int MiniDoomRead(void* Handle, void* Buf, int Count)
{
    int bytes = Count;
    if (Handle == NULL)
		return 0;
    if ((bytes + g_MiniDoomWadDataOffset) > g_DoomWadSize)
    {
      bytes = g_DoomWadSize - g_MiniDoomWadDataOffset;
    }
    memcpy(Buf, g_DoomWadAddress + g_MiniDoomWadDataOffset, bytes);
    return bytes;
}

int MiniDoomTell(void* Handle)
{
    if (Handle == NULL)
    		return 0;
    return g_MiniDoomWadDataOffset;
}

int MiniDoomEof(void* Handle)
{
    if (Handle == NULL)
		return -1;
    return g_MiniDoomWadDataOffset >= g_DoomWadSize;
}

int MiniDoomSeek(void* Handle, int Offset, doom_seek_t SeekType)
{
if (Handle == NULL)
		return -1;
    if (SeekType == DOOM_SEEK_SET)
		g_MiniDoomWadDataOffset = Offset;
    else if (SeekType == DOOM_SEEK_CUR)
		g_MiniDoomWadDataOffset = g_MiniDoomWadDataOffset + Offset;
		else if (SeekType == DOOM_SEEK_END)
  g_MiniDoomWadDataOffset = g_DoomWadSize - 1 + Offset;
    return 0;
}

void* MiniDoomMalloc(int size)
{
    return EnvHeapMalloc(size);
}

void MiniDoomFree(void* Mem)
{
    EnvHeapFree(Mem);
    return;
}

void MiniDoomPrint(const char* String)
{
    printf("[DOOM]: %s\n", String);
}

void MiniDoomExit(int code)
{
    printf("[DOOM]: Exiting with code %d\n", code);
    while (1);
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
    *sec = g_DoomControlRegs->time_sec;
    *usec = g_DoomControlRegs->time_usec;
}

int MiniDoomKeyTranslate(int key)
{
    switch (key)
    {
    case CRT_DOOM_KEY_ENTER:  return DOOM_KEY_ENTER;
    case CRT_DOOM_KEY_LEFT_ARROW:    return DOOM_KEY_LEFT_ARROW;
    case CRT_DOOM_KEY_RIGHT_ARROW:   return DOOM_KEY_RIGHT_ARROW;
    case CRT_DOOM_KEY_UP_ARROW:      return DOOM_KEY_UP_ARROW;
    case CRT_DOOM_KEY_DOWN_ARROW:    return DOOM_KEY_DOWN_ARROW;
    case CRT_DOOM_KEY_SPACE:   return DOOM_KEY_SPACE;
    case CRT_DOOM_KEY_CTRL: return DOOM_KEY_CTRL;
    case CRT_DOOM_KEY_ESCAPE:  return DOOM_KEY_ESCAPE;
    case CRT_DOOM_KEY_Y:        return DOOM_KEY_Y;
    }
    return 0;
}

void MiniDoomGetKeys()
{
    int i = 0;
    int k = 0;
    for (i = 0; i < (sizeof(g_DoomControlRegs->keys)/ sizeof(g_DoomControlRegs->keys[0])); i++)
    {
        if (g_DoomControlRegs->keys[i].action)
        {
            k = MiniDoomKeyTranslate(g_DoomControlRegs->keys[i].key);
            if (k)
            {
                if (g_DoomControlRegs->keys[i].action == 1)
                {
                    doom_key_down(k);
                }
                else
                {
                    doom_key_up(k);
                }
            }
            g_DoomControlRegs->keys[i].action = 0;
            g_DoomControlRegs->keys[i].key = 0;
        }
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


void CrtDoomInit()
{
    EnvHeapSetup();

    doom_set_file_io(MiniDoomOpen, MiniDoomClose, MiniDoomRead, NULL,
        MiniDoomSeek, MiniDoomTell, MiniDoomEof);
    doom_set_malloc(MiniDoomMalloc, MiniDoomFree);
    doom_set_exit(MiniDoomExit);
    doom_set_getenv(MiniDoomGetEnv);
    doom_set_gettime(MiniDoomGetTime);
    doom_set_print(MiniDoomPrint);

    doom_init(3, g_MiniDoomArgvw, 0);
}

void CrtDoomIteration()
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
            MiniDoomPutPixel(g_DoomControlRegs->pixels, x, y, g_DoomControlRegs->width, g_DoomControlRegs->height, framebuffer[x + y * 320]);
        }
    }
    MiniDoomGetKeys();
}

void *crtdoom_memset(void *addr, int val, int len)
{
	char *p;

	p = (char *)addr;
	while (len--)
		*p++ = val;
	return addr;
}

void *crtdoom_memcpy(void *dest, const void *src, int len)
{
	char *p, *q;

	p = (char *)dest;
	q = (char *)src;
	while (len--)
		*p++ = *q++;
	return dest;
}

int crtdoom_strcmp (const char *s1, const char *s2)
{
	int r, c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;
		r = c1 - c2;
	} while (!r && c1);
	return r;
}

int crtdoom_memcmp (const void *p1, const void *p2, int len)
{
	int r, i;
	char *q1, *q2;

	q1 = (char *)p1;
	q2 = (char *)p2;
	for (r = 0, i = 0; !r && i < len; i++)
		r = *q1++ - *q2++;
	return r;
}

int crtdoom_strlen (const char *p)
{
	int len = 0;

	while (*p++)
		len++;
	return len;
}

int crtdoom_strncmp (const char *s1, const char *s2, int len)
{
	int r, c1, c2;

	if (len <= 0)
		return 0;
	do {
		c1 = *s1++;
		c2 = *s2++;
		r = c1 - c2;
	} while (!r && c1 && --len > 0);
	return r;
}
