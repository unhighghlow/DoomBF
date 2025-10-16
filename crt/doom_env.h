

extern void *g_DoomHeapAddress;
extern unsigned int g_DoomHeapSize;
extern char *g_DoomWadAddress;
extern unsigned int g_DoomWadSize;

#define CRT_DOOM_KEY_ENTER 1
#define CRT_DOOM_KEY_LEFT_ARROW 2
#define CRT_DOOM_KEY_RIGHT_ARROW 3
#define CRT_DOOM_KEY_UP_ARROW 4
#define CRT_DOOM_KEY_DOWN_ARROW 5
#define CRT_DOOM_KEY_SPACE 6
#define CRT_DOOM_KEY_CTRL 7
#define CRT_DOOM_KEY_ESCAPE 8
#define CRT_DOOM_KEY_Y 9

struct DoomKey
{
    int key;
    int action;
};

struct DoomControlRegs
{
    char *pixels;
    int width;
    int height;
    int time_sec;
    int time_usec;
    struct DoomKey keys[16];
};

extern struct DoomControlRegs *g_DoomControlRegs;

void EnvPutChar(int c);

void CrtDoomInit();
void CrtDoomIteration();