#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "crt/doom_env.h"
#include "doom_wad.h" /* data_doom_wad */

void *g_DoomHeapAddress = (void*)0x4000000;
unsigned int g_DoomHeapSize = 0x4000000;
char *g_DoomWadAddress = NULL;
unsigned int g_DoomWadSize = 0;
struct DoomControlRegs g_BrainfuckDoomControlRegs;
struct DoomControlRegs *g_DoomControlRegs = &g_BrainfuckDoomControlRegs;

unsigned int cur_time_us;
unsigned int cur_time_sec;

void EnvPutChar(int c) {
    putc(c, stdout);
#ifndef _BF
    if (c == '\n') {
        fflush(stdout);
    }
#endif
}

#define DOOM_WIDTH 640
#define DOOM_HEIGHT 480
#define PIXEL_WIDTH 4
#define IMAGE_SIZE (DOOM_WIDTH * DOOM_HEIGHT * PIXEL_WIDTH)

static int  g_DoomWinWidth  = DOOM_WIDTH;
static int  g_DoomWinHeight = DOOM_HEIGHT;

void output_image(char*);
void step_clock(int);

int main(int argc, char *argv[]) {
    char pixels[IMAGE_SIZE];
    for (int i = 0; i < IMAGE_SIZE; i++) {
        pixels[i] = 0;
    }

    g_DoomWadAddress = data_doom_wad;
    g_DoomWadSize = data_doom_wad_len;
    cur_time_us = 0;
    cur_time_sec = 0;

#ifndef _BF
    /* Linux shim for testing purposes */
    size_t pagesize = sysconf(_SC_PAGESIZE);
    void *ptr = mmap(
	g_DoomHeapAddress,
	g_DoomHeapSize,
	PROT_READ | PROT_WRITE,
	MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
	-1,
	0
    );

    if (ptr == MAP_FAILED) {
        perror("mmap failed");
	return 1;
    }
#endif

    g_BrainfuckDoomControlRegs.pixels = pixels;
    g_BrainfuckDoomControlRegs.width  = DOOM_WIDTH;
    g_BrainfuckDoomControlRegs.height = DOOM_HEIGHT;

    CrtDoomInit();
    while (1) {
        step_clock(1000000);
        g_BrainfuckDoomControlRegs.pixels = pixels;
        g_BrainfuckDoomControlRegs.width  = DOOM_WIDTH;
        g_BrainfuckDoomControlRegs.height = DOOM_HEIGHT;
        CrtDoomIteration();
        output_image(pixels);
    }
}

void step_clock(int step_us) {
    cur_time_us += step_us;
    if (cur_time_us >= 1000000) {
        cur_time_sec += 1;
        cur_time_us -= 1000000;
    }

    g_BrainfuckDoomControlRegs.time_sec  = cur_time_sec;
    g_BrainfuckDoomControlRegs.time_usec = cur_time_us;
}

void output_image(char *pixels) {
    int i;

    EnvPutChar(0);
    EnvPutChar(DOOM_WIDTH >> 8);
    EnvPutChar(DOOM_WIDTH);
    EnvPutChar(DOOM_HEIGHT >> 8);
    EnvPutChar(DOOM_HEIGHT);
    unsigned char a = 0x01;
    for (i = 0; i < IMAGE_SIZE; i+=4) {
        EnvPutChar(pixels[i]);
        EnvPutChar(pixels[i+1]);
        EnvPutChar(pixels[i+2]);
        a++;
        if (!a) a = 1;
        /* skip alpha */
    }
    EnvPutChar('\n');
}
