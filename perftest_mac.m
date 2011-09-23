/* Performance test for low-level graphics functions
 *
 * Copyright 2011, TransGaming, Inc.
 */
#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <unistd.h>
#import "perftest.h"

/* CGL / NSOpenGL variables */
static NSOpenGLContext *context = NULL;

static void loop_callback(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info)
{
    CFRunLoopWakeUp(CFRunLoopGetCurrent());

    displayOpenGL();

    /* Yield control back to the OS briefly to prevent full-system slowdown. */
    usleep(0);

    [context flushBuffer];

    fflush(NULL);
}

@interface PerfTestWindow : NSWindow
@end

@implementation PerfTestWindow
-(void)onKey: (unichar)key downEvent: (BOOL)flag
{
    if ((key == 'q') || (key == 'Q'))
        killProcess(0);

    handleKeyPress(key);
}

-(void)keyDown: (NSEvent*) event
{
    NSString *characters;
    unsigned int characterIndex, characterCount;

    characters = [event charactersIgnoringModifiers];
    characterCount = [characters length];

    for (characterIndex = 0; characterIndex < characterCount; characterIndex++)
        [self onKey:[characters characterAtIndex:characterIndex] downEvent:YES];
}
@end


@interface AppDelegate : NSObject<NSApplicationDelegate> {
    PerfTestWindow* window;
    NSOpenGLView* view;
}
@end

@implementation AppDelegate
-(void)applicationDidFinishLaunching:(NSNotification*)notification
{
    NSRect rect;
    rect.origin.x    = 0;
    rect.origin.y    = 0;
    rect.size.width  = DEFAULT_WINDOW_WIDTH;
    rect.size.height = DEFAULT_WINDOW_HEIGHT;
    window =
    [
        [PerfTestWindow alloc]
        initWithContentRect: rect
        styleMask:           NSTitledWindowMask |
                             NSClosableWindowMask |
                             NSMiniaturizableWindowMask
        backing:             NSBackingStoreBuffered
        defer:               NO
    ];
    [window setTitle: @"gfxperftest"];

    NSOpenGLPixelFormatAttribute attr[8];
    unsigned int i = 0;

    attr[i++] = NSOpenGLPFADoubleBuffer;
    attr[i++] = NSOpenGLPFAAccelerated;
    attr[i++] = NSOpenGLPFANoRecovery;
    attr[i++] = NSOpenGLPFADepthSize;
    attr[i++] = 24;

#ifndef AVAILABLE_MAC_OS_X_VERSION_10_7_AND_LATER
  #define NSOpenGLPFAOpenGLProfile          99
  #define NSOpenGLProfileVersion3_2Core 0x3200
#endif
    if (gUseCoreContext) {
        attr[i++] = NSOpenGLPFAOpenGLProfile;
        attr[i++] = NSOpenGLProfileVersion3_2Core;
    }

    attr[i++] = 0;

    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
    view = [[NSOpenGLView alloc] initWithFrame:rect pixelFormat:format];
    [window setContentView:view];

    context = [view openGLContext];

    /* Disable VSync */
    GLint vBlank = 0;
    [context setValues:&vBlank forParameter:NSOpenGLCPSwapInterval];

    CFRunLoopObserverRef observer = CFRunLoopObserverCreate(kCFAllocatorDefault, kCFRunLoopBeforeWaiting, true, 0, &loop_callback, NULL);
    CFRunLoopAddObserver(CFRunLoopGetCurrent(), observer, kCFRunLoopCommonModes);
    CFRelease(observer);

    initOpenGLStates();

    [window makeFirstResponder:window];
    [window makeKeyAndOrderFront:window];
    [format dealloc];
}

-(void)dealloc
{
    [window dealloc];
    [view dealloc];
    [super dealloc];
}

@end

void initMacOS()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    AppDelegate *delegate = [[AppDelegate alloc] init];
    NSApplication *application = [NSApplication sharedApplication];
    [application setDelegate: delegate];
    [pool drain];
    [application run];

    /* Cleanup */
    [delegate release];
    [pool release];
}

#endif
