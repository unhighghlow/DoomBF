// macOS/AppKit
//clang -O2 -Wall -Wextra -fobjc-arc -framework Cocoa -o mac_doom mac_doom.m

#import <Cocoa/Cocoa.h>
#import <mach/mach_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "doom_env.h" 

struct DoomControlRegs g_MacDoomControlRegs;
struct DoomControlRegs *g_DoomControlRegs = &g_MacDoomControlRegs;

void *g_DoomHeapAddress = NULL;
unsigned int g_DoomHeapSize = 0;
char *g_DoomWadAddress = NULL;
unsigned int g_DoomWadSize = 0;

void EnvPutChar(int c) { putchar(c); fflush(stdout); }

static uint64_t timebase_num = 0, timebase_denom = 0;
static void MacDoomInitTime() {
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    timebase_num = info.numer;
    timebase_denom = info.denom;
}
static void MacDoomUpdateTime() {
    uint64_t t = mach_absolute_time();
    __uint128_t ns = (__uint128_t)t * timebase_num / timebase_denom;
    g_MacDoomControlRegs.time_sec  = (int)(ns / 1000000000ULL);
    g_MacDoomControlRegs.time_usec = (int)((ns % 1000000000ULL) / 1000ULL);
}

static int LoadFileToMemory(const char *path, char **out_buf, unsigned *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Error: cannot open %s\n", path); return -1; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0) { fclose(f); fprintf(stderr, "Error: invalid size\n"); return -1; }
    rewind(f);
    char *buf = (char*)malloc((size_t)sz);
    if (!buf) { fclose(f); fprintf(stderr, "Error: malloc failed\n"); return -1; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) { fprintf(stderr, "Error: fread mismatch\n"); free(buf); return -1; }

    *out_buf = buf;
    *out_size = (unsigned)sz;

    unsigned long long summ = 0;
    for (unsigned i = 0; i < *out_size; i++) summ += 1 + (unsigned char)buf[i];
    printf("Successfully loaded file: %s (%u bytes) %llu code\n", path, *out_size, summ);
    return 0;
}

static const int kWidth  = 640;
static const int kHeight = 480;

@interface DoomView : NSView
@property(nonatomic) uint8_t *pixels;  
@end

@implementation DoomView
+ (Class)layerClass { return [CALayer class]; }

- (BOOL)isFlipped { return YES; }

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    if (!self.pixels) return;

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider =
        CGDataProviderCreateWithData(NULL, self.pixels, (size_t)kWidth * (size_t)kHeight * 4, NULL);
    CGImageRef img = CGImageCreate(kWidth, kHeight,
                                   8, 32, kWidth * 4,
                                   cs, kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipLast,
                                   provider, NULL, false, kCGRenderingIntentDefault);

    CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
    CGRect dst = CGRectMake(0, 0, self.bounds.size.width, self.bounds.size.height);
    CGContextDrawImage(ctx, dst, img);

    CGImageRelease(img);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(cs);
}
@end

static void PushKeyEvent(int doomKey, int action) {
    if (!doomKey) return;
    size_t n = sizeof(g_MacDoomControlRegs.keys)/sizeof(g_MacDoomControlRegs.keys[0]);
    for (size_t i = 0; i < n; i++) {
        if (!g_MacDoomControlRegs.keys[i].action) {
            g_MacDoomControlRegs.keys[i].action = action; // 1=down, 2=up
            g_MacDoomControlRegs.keys[i].key    = doomKey;
            break;
        }
    }
}

static int MapKeyCodeToDoomKey(unsigned short keyCode, NSEvent *ev, BOOL keyDown) {
    switch (keyCode) {
        case 36:  return CRT_DOOM_KEY_ENTER;        
        case 123: return CRT_DOOM_KEY_LEFT_ARROW;   
        case 124: return CRT_DOOM_KEY_RIGHT_ARROW;  
        case 126: return CRT_DOOM_KEY_UP_ARROW;     
        case 125: return CRT_DOOM_KEY_DOWN_ARROW;   
        case 49:  return CRT_DOOM_KEY_SPACE;        
        case 53:  return CRT_DOOM_KEY_ESCAPE;       
        default:  break;
    }
    NSString *chs = [ev charactersIgnoringModifiers];
    if (chs.length > 0) {
        unichar c = [chs characterAtIndex:0];
        if (c == 'y' || c == 'Y') return CRT_DOOM_KEY_Y;
    }
    (void)keyDown;
    return 0;
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic,strong) NSWindow *window;
@property(nonatomic,strong) DoomView *view;
@property(nonatomic,strong) NSTimer *timer;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note {
    (void)note;

    NSRect rect = NSMakeRect(100, 100, kWidth, kHeight);
    self.window = [[NSWindow alloc] initWithContentRect:rect
                                              styleMask:(NSWindowStyleMaskTitled |
                                                         NSWindowStyleMaskClosable |
                                                         NSWindowStyleMaskMiniaturizable |
                                                         NSWindowStyleMaskResizable)
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    self.window.title = @"Doom window (macOS)";
    self.view = [[DoomView alloc] initWithFrame:rect];
    self.view.wantsLayer = YES;
    self.window.contentView = self.view;
    [self.window makeKeyAndOrderFront:nil];

    [self.window makeFirstResponder:self];

    g_DoomHeapSize    = 0x4000000;
    g_DoomHeapAddress = malloc(g_DoomHeapSize);
    if (!g_DoomHeapAddress) { NSAlert *a = [[NSAlert alloc] init]; a.messageText=@"Heap alloc failed"; [a runModal]; [NSApp terminate:nil]; return; }

    const char *wadPath = "doom.wad";
    NSArray<NSString*> *args = [[NSProcessInfo processInfo] arguments];
    if (args.count >= 2) wadPath = args[1].UTF8String;

    if (LoadFileToMemory(wadPath, &g_DoomWadAddress, &g_DoomWadSize) != 0) {
        NSAlert *a = [[NSAlert alloc] init]; a.messageText = [NSString stringWithFormat:@"Failed to load: %s", wadPath]; [a runModal];
        [NSApp terminate:nil]; return;
    }

    uint8_t *pixels = (uint8_t *)calloc(kWidth * kHeight, 4);
    if (!pixels) { NSAlert *a = [[NSAlert alloc] init]; a.messageText=@"Framebuffer alloc failed"; [a runModal]; [NSApp terminate:nil]; return; }
    self.view.pixels = pixels;

    MacDoomInitTime();
    CrtDoomInit();
    printf("INITED!!\n");

    self.timer = [NSTimer scheduledTimerWithTimeInterval:0.010
                                                  target:self
                                                selector:@selector(tick)
                                                userInfo:nil
                                                 repeats:YES];
}

- (void)tick {
    MacDoomUpdateTime();
    g_MacDoomControlRegs.pixels = (char*)self.view.pixels;
    g_MacDoomControlRegs.width  = kWidth;
    g_MacDoomControlRegs.height = kHeight;

    CrtDoomIteration();

    [self.view setNeedsDisplay:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender { return YES; }

#pragma mark - Клавиатура (KeyDown/KeyUp/Modifiers)

- (void)keyDown:(NSEvent *)event {
    int dk = MapKeyCodeToDoomKey(event.keyCode, event, YES);
    if (dk) PushKeyEvent(dk, 1);
    NSString *chs = event.charactersIgnoringModifiers;
    if (chs.length && ([chs characterAtIndex:0] == 'q' || [chs characterAtIndex:0] == 'Q')) {
        [NSApp terminate:nil];
    }
}
- (void)keyUp:(NSEvent *)event {
    int dk = MapKeyCodeToDoomKey(event.keyCode, event, NO);
    if (dk) PushKeyEvent(dk, 2);
}

- (void)flagsChanged:(NSEvent *)event {
    static bool prevCtrl = false;
    bool nowCtrl = (event.modifierFlags & NSEventModifierFlagControl) != 0;
    if (nowCtrl != prevCtrl) {
        PushKeyEvent(CRT_DOOM_KEY_CTRL, nowCtrl ? 1 : 2);
        prevCtrl = nowCtrl;
    }
}

@end

// ---------- main ----------
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [AppDelegate new];
        app.delegate = delegate;
        [app run];

        if (delegate.view.pixels) free(delegate.view.pixels);
        if (g_DoomHeapAddress) free(g_DoomHeapAddress);
        if (g_DoomWadAddress) free(g_DoomWadAddress);
    }
    return 0;
}
