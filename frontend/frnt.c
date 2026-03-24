#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define BYTES_PER_PIXEL 4

unsigned char read_byte();
void read_until_image();
short read_short();
void full_read(char*, int);

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

static void send_keyaction(int key, int action) {
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
    if (!k) return;

    if (action == 2) k |= KEYUP;

    putc(k, stdout);
}

int main(int argc, char *argv[]) {
        int cur_width;
        int cur_height;
        int new_width;
        int new_height;
        char *cur_buf = 0;
        char *cur_temp_buf = 0;

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

        int running = 1;
        while (running) {
                read_until_image();
                new_width = read_short();
                new_height = read_short();
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
                                0, 0, cur_width, cur_height,
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
                                cur_height,
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

                full_read(cur_temp_buf, read_img_size);

                int j = 0;
                for (int i = 0; i < cur_width * cur_height * BYTES_PER_PIXEL; i+=BYTES_PER_PIXEL) {
                        cur_buf[i] = cur_temp_buf[j++];
                        cur_buf[i+1] = cur_temp_buf[j++];
                        cur_buf[i+2] = cur_temp_buf[j++];
                        /* skip alpha */
                        cur_buf[i+3] = 0xff;
                }

                XPutImage(dpy, win, gc, img, 0, 0, 0, 0, cur_width, cur_height);
                XFlush(dpy);

                while (XPending(dpy)) {
                    XEvent ev;
                    XNextEvent(dpy, &ev);
                    switch (ev.type) {
                        case Expose:
                            break;
                        case ConfigureNotify:
                            break;
                        case KeyPress: {
                            KeySym ks = XLookupKeysym(&ev.xkey, 0);
                            if (ks == XK_q) { running = 0; }
                            send_keyaction(ks, 1);
                        } break;
                        case KeyRelease: {
                            KeySym ks = XLookupKeysym(&ev.xkey, 0);
                            send_keyaction(ks, 2);
                        } break;
                        case ClientMessage:
                        default:
                            break;
                    }
                }
                putc(0, stdout);
                fflush(stdout);
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
                bytes_read += bytes_read_i;
                buf += bytes_read_i;
                if (bytes_read >= size) {
                        break;
                }
                if (errno == EAGAIN) {
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
