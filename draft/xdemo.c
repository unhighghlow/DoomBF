// xdemo.c
#include <X11/Xlib.h>
#include <stdio.h>

int main(void) {
    Display *d = XOpenDisplay(NULL);
    if (!d) { fprintf(stderr, "no display\n"); return 1; }
    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10,10, 300,200, 1, BlackPixel(d,s), WhitePixel(d,s));
    XMapWindow(d, w);
    XFlush(d);
    getchar(); // ждём Enter
    XCloseDisplay(d);
    return 0;
}
