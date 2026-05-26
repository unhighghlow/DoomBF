#ifndef _BF
#  define _GNU_SOURCE
#  include <sys/mman.h>
#  include <time.h>
#endif

#include <stddef.h>
#include <stdio.h>
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
#ifdef _BF
    char cc = c;

    register long a0 __asm__("a0") = 1; // stdout
    register const char *a1 __asm__("a1") = &cc;
    register long a2 __asm__("a2") = 1;
    register long a7 __asm__("a7") = 64; // write
    __asm__ volatile ("ecall"
        : "+r"(a0)
        : "r"(a1), "r"(a2), "r"(a7)
        : "memory");
#else
    putc(c, stdout);
    if (c == '\n') {
        fflush(stdout);
    }
#endif
}

char EnvGetCharBlock() {
    char buf[1];
#ifdef _BF
    register long a0 __asm__("a0") = 0; // stdin
    register const char *a1 __asm__("a1") = buf;
    register long a2 __asm__("a2") = 1;
    register long a7 __asm__("a7") = 63; // read

    __asm__ volatile("ecall"
        : "+r"(a0)
        : "r"(a1), "r"(a2), "r"(a7)
        : "memory");
#else
    read(STDIN_FILENO, buf, 1);
#endif
    return buf[0];
}

#define DOOM_WIDTH 640
#define DOOM_HEIGHT 480
#define PIXEL_WIDTH 4
#define IMAGE_SIZE (DOOM_WIDTH * DOOM_HEIGHT * PIXEL_WIDTH)

#define KEY_        0x0f
#define KEY_UP      0x01
#define KEY_DOWN    0x02
#define KEY_LEFT    0x03
#define KEY_RIGHT   0x04
#define KEY_ENTER   0x05
#define KEY_SPACE   0x06
#define KEY_CTRL    0x07
#define KEY_ESC     0x08
#define KEY_Y       0x09
#define KEYUP       0xf0

static int  g_DoomWinWidth  = DOOM_WIDTH;
static int  g_DoomWinHeight = DOOM_HEIGHT;

void output_image(char*);
void step_clock(int);
void process_keyevent(char);

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
    output_image(pixels);
    output_image(pixels);

    char ev;
    while (1) {
        while (1) {
            ev = EnvGetCharBlock();
            if (!ev) break;
            process_keyevent(ev);
        }
        g_BrainfuckDoomControlRegs.pixels = pixels;
        g_BrainfuckDoomControlRegs.width  = g_DoomWinWidth;
        g_BrainfuckDoomControlRegs.height = g_DoomWinHeight;
        CrtDoomIteration();
        output_image(pixels);
#ifndef _BF
        usleep(10000);
#endif
        step_clock(20000);
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
}

void process_keyevent(char event) {
    int key = event & KEY_;
    int k = 0;
    switch (key) {
        case KEY_ENTER: k = CRT_DOOM_KEY_ENTER; break;
        case KEY_LEFT:  k = CRT_DOOM_KEY_LEFT_ARROW; break;
        case KEY_RIGHT: k = CRT_DOOM_KEY_RIGHT_ARROW; break;
        case KEY_UP:    k = CRT_DOOM_KEY_UP_ARROW; break;
        case KEY_DOWN:  k = CRT_DOOM_KEY_DOWN_ARROW; break;
        case KEY_SPACE: k = CRT_DOOM_KEY_SPACE; break;
        case KEY_CTRL:  k = CRT_DOOM_KEY_CTRL; break;
        case KEY_ESC:   k = CRT_DOOM_KEY_ESCAPE; break;
        case KEY_Y:     k = CRT_DOOM_KEY_Y; break;
        default: break;
    }
    if (!k) return;

    int action = 1;
    if (event & KEYUP) {
        action = 2;
    }

    for (size_t i = 0; i < sizeof(g_BrainfuckDoomControlRegs.keys) / sizeof(g_BrainfuckDoomControlRegs.keys[0]); i++) {
        if (!g_BrainfuckDoomControlRegs.keys[i].action) {
            g_BrainfuckDoomControlRegs.keys[i].action = action; // 1=down, 2=up
            g_BrainfuckDoomControlRegs.keys[i].key    = k;
            break;
        }
    }
}

#ifdef _BF
void _start() {
    main(0, NULL);
}
#endif
