#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BYTES_PER_PIXEL 4

unsigned char read_byte();
void read_until_image();
short read_short();
void full_read(char*, int);

int col = 0;
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

static int parse_key(int key) {
    int k = 0;
    switch (key) {
        case XK_Return:   k = KEY_ENTER; break;
        case XK_Left:     k = KEY_LEFT; break;
        case XK_Right:    k = KEY_RIGHT; break;
        case XK_Up:       k = KEY_UP; break;
        case XK_Down:     k = KEY_DOWN; break;
        case XK_space:    k = KEY_SPACE; break;
        case XK_Control_L:
        case XK_Control_R: k = KEY_CTRL; break;
        case XK_Escape:   k = KEY_ESC; break;
        case XK_y:
        case XK_Y:        k = KEY_Y; break;
        default: break;
    }
    return k;
}

static void send_keyaction(int k, int action) {
    if (!k) return;

    if (action == 2) k |= KEYUP;

    putc(k, stdout);
}

int set_stdin_nonblocking()
{
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
        return -1;
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1)
        return -1;
    return 0;
}

#define STATUSBAR_HEIGHT 10

int main(int argc, char *argv[]) {
        int cur_width;
        int cur_height;
        int new_width;
        int new_height;
        char *cur_buf = 0;
        char *cur_temp_buf = 0;
        signed char cur_pressed[10]; /* 0 - not pressed; 1 - pressed for a frame; 2 - held down */
        signed char new_pressed[10];
        memset(cur_pressed, 0, 10);
        memset(new_pressed, 0, 10);

        // X11 init
        Display *dpy = XOpenDisplay(NULL);
        if (!dpy) {
                fprintf(stderr, "Error: cannot open X display\n");
                return 1;
        }
        int screen = DefaultScreen(dpy);
        Window root = RootWindow(dpy, screen);

        XSetWindowAttributes attrs;
        attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;

        Window win;
        XImage *img;
        GC gc;

        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stdin, NULL, _IONBF, 0);
        if (set_stdin_nonblocking())
            exit(2);

#define READ_UNTIL_IMAGE \
                    char chr = 0; \
                    if (read(STDIN_FILENO, &chr, 1) < 1) { \
                        if (errno == EAGAIN || errno == EWOULDBLOCK) { \
                            continue; \
                        } \
                        perror("error reading stdin"); \
                        exit(1); \
                    } \
                    if (!chr) break; \
                    fprintf(stderr, "%c", chr);

#define UPDATE XPutImage(dpy, win, gc, img, 0, 0, 0, 0, cur_width, cur_height); XFlush(dpy);

        int running = 1;
        while (1) {
            READ_UNTIL_IMAGE
        }
        putc(0, stdout);
        while (running) {
                fprintf(stderr, "[frnt] Image begin\n");
                new_width = read_short();
                new_height = read_short();
                fprintf(stderr, "[frnt] %dx%d\n", new_width, new_height);
                int img_size = new_width * new_height * BYTES_PER_PIXEL;
                int read_img_size = new_width * new_height * 3;

                if (!cur_buf) {
                        cur_width = new_width;
                        cur_height = new_height;
                        cur_buf = (char*)calloc(1, img_size);
                        cur_temp_buf = (char*)calloc(1, read_img_size);
                        if (!cur_buf || !cur_temp_buf) {
                                fprintf(stderr, "Error: cannot allocate framebuffer\n");
                                exit(1);
                        }

                        win = XCreateWindow(
                                dpy, root,
                                0, 0, cur_width, cur_height+STATUSBAR_HEIGHT,
                                0,
                                DefaultDepth(dpy, screen),
                                InputOutput,
                                DefaultVisual(dpy, screen),
                                CWEventMask, &attrs
                        );
                        XStoreName(dpy, win, "Doom window (Brainfuck)");
                        XMapWindow(dpy, win);

                        img = XCreateImage(
                                dpy, DefaultVisual(dpy, screen),
                                24,               
                                ZPixmap,
                                0,                
                                cur_buf,           
                                cur_width,
                                cur_height+STATUSBAR_HEIGHT,
                                32,               // bitmap_pad
                                cur_width * BYTES_PER_PIXEL // bytes_per_line
                        );
                        if (!img) {
                                fprintf(stderr, "Error: XCreateImage failed\n");
                                exit(1);
                        }

                        gc = XCreateGC(dpy, win, 0, NULL);
                }
                if (new_width != cur_width || new_height != cur_height) {
                        fprintf(stderr, "Error: image dimensions changed\n");
                        fprintf(stderr, "width: %d -> %d\n", cur_width, new_width);
                        fprintf(stderr, "height: %d -> %d\n", cur_height, new_height);
                        exit(1);
                }

                fprintf(stderr, "[frnt] Reading %d bytes\n", read_img_size);
                full_read(cur_temp_buf+(cur_width*STATUSBAR_HEIGHT*3), read_img_size);
                fprintf(stderr, "[frnt] Read done\n");

                int j = 0;
                for (int i = 0; i < cur_width * cur_height * BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL) {
                        cur_buf[i] = cur_temp_buf[j++];
                        cur_buf[i+1] = cur_temp_buf[j++];
                        cur_buf[i+2] = cur_temp_buf[j++];
                        /* skip alpha */
                        cur_buf[i+3] = 0xff;
                }

                UPDATE

                for (int i = 0; i < 10; i++) {
                        new_pressed[i] = cur_pressed[i];
                }
                while (1) {
                    char br;
                    char bg;
                    char bb;
                    for (int b = 0; b < 11; b++) {
                            if (b == 0) {
                                    br = 0x00;
                                    bg = 0x00;
                                    bb = 0x00;
                                    switch (col) {
                                            case 0: br = 0xff; break;
                                            case 1: bg = 0xff; break;
                                            case 2: bb = 0xff; break;
                                    }
                            } else {
                                    br = 0x00;
                                    bg = 0x00;
                                    bb = 0x00;
                                    if (!new_pressed[b] && cur_pressed[b]) {
                                            br = 0xff;
                                    }
                                    if (new_pressed[b] && cur_pressed[b]) {
                                            br = 0xff;
                                            bg = 0xff;
                                            bb = 0xff;
                                    }
                                    if (new_pressed[b] && !cur_pressed[b]) {
                                            bb = 0xff;
                                    }
                            }
                            for (int x = 0; x < STATUSBAR_HEIGHT; x++) {
                                    for (int y = 0; y < STATUSBAR_HEIGHT; y++) {
                                            int a = BYTES_PER_PIXEL*(x+(b*STATUSBAR_HEIGHT)+y*cur_width);
                                            cur_buf[a+0] = br;
                                            cur_buf[a+1] = bg;
                                            cur_buf[a+2] = bb;
                                            cur_buf[a+3] = 0xff;
                                    }
                            }
                    }
                    UPDATE
                    while (XPending(dpy)) {
                        XEvent ev;
                        int kv;
                        XNextEvent(dpy, &ev);
                        switch (ev.type) {
                            case Expose:
                                break;
                            case ConfigureNotify:
                                break;
                            case KeyPress: {
                                KeySym ks = XLookupKeysym(&ev.xkey, 0);
                                if (ks == XK_q) { running = 0; }
                                kv = parse_key(ks);
                                new_pressed[kv] = !new_pressed[kv];
                            } break;
                            case ClientMessage:
                            default:
                                break;
                        }
                    }
                    READ_UNTIL_IMAGE
                }

                for (int i = 0; i < 10; i++) {
                        if (!cur_pressed[i] && new_pressed[i]) {
                                cur_pressed[i] = 1;
                                send_keyaction(i, 1);
                                continue;
                        }
                        if (cur_pressed[i] && !new_pressed[i]) {
                                cur_pressed[i] = 0;
                                send_keyaction(i, 2);
                                continue;
                        }
                }
                putc(0, stdout);
                fflush(stdout);
                col++;
                col%=3;
        }

        if (cur_buf) {
                XDestroyImage(img);
                XFreeGC(dpy, gc);
                XDestroyWindow(dpy, win);
        }
        XCloseDisplay(dpy);
        return 0;
}

void full_read(char *buf, int size) {
        int bytes_read = 0;
        int bytes_read_i;
        while (1) {
                bytes_read_i = read(STDIN_FILENO, buf, size - bytes_read);
                if (bytes_read_i > 0) {
                        bytes_read += bytes_read_i;
                        buf += bytes_read_i;
                        if (bytes_read >= size) {
                                break;
                        }
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue;
                }

                perror("error reading pipe");
                exit(1);
        }
}

unsigned char read_byte() {
        char buf[1];
        if (!read(STDIN_FILENO, buf, 1)) {
                perror("error reading pipe");
                exit(1);
        }
        return buf[0];
}

void read_until_image() {
        char chr;
        while (1) {
                chr = read_byte();
                if (chr)
                        fprintf(stderr, "%c", chr);
                else
                        break;
        }
}

short read_short() {
        unsigned short i = 0;
        i += read_byte();
        i <<= 8;
        i += read_byte();
        return i;
}
