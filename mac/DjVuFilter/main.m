/*
 DjVuFilter
 NSUnixStdio type filter for DjVu files.
 Author:  Jeff Sickel
 
 2002-01-22 Written for Cocoa
 2004-05-24 Upgraded to Panther and DjVuLibre
 
 Copyright (c) 2002, 2004 by Corpus Callosum Corporation, all rights reserved.
 */

#import <Foundation/Foundation.h>

extern NSBitmapImageRep *bitmapImageRepFromDjVu(NSString *djvuPath, int pagenum, int dpi, NSSize size);

void
copyright(NSBundle *mainBundle)
{
	NSString *right = [mainBundle localizedStringForKey:@"DjVuFilterCopyright"
												  value:nil
												  table:nil];
	
    fprintf(stderr, "%s\n\n", [right cString]);
}

int usage(NSBundle *mainBundle)
{
	NSString *usage = [mainBundle localizedStringForKey:@"DjVuFilterUsage"
												  value:nil
												  table:nil];
	
	fprintf(stderr, "%s\n", [usage cString]);
    return(1);
}

int main (int argc, const char *argv[]) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    NSString *djvuPath = nil;
    NSData *data;
    NSBitmapImageRep *tiff = nil;
	int pagenum = 0;
	float dpi = 72.0;
	NSSize size = NSMakeSize(0.0, 0.0);		// default 100%
    BOOL success = NO;
	NSBundle *mainBundle = nil;
	
	/* go ahead and get the actual bundle for the service */
	{
		NSString *exe = [[NSString pathWithComponents:
			[NSArray arrayWithObjects:
				[[[NSProcessInfo processInfo] arguments] objectAtIndex:0], nil]]
			stringByResolvingSymlinksInPath];
		NSString *service = [[[exe stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
		mainBundle = [NSBundle bundleWithPath:service];
	}
	
    copyright(mainBundle);
    
	if (args == nil || [args count] == 1) {
		int ret = usage(mainBundle);
		[pool release];
		exit(ret);
	}
	
	/* arg sifting */
	/*
	 NSLog(@"args: %@", args);
	 NSLog(@"env: %@", [[NSProcessInfo processInfo] environment]);
	if (len > 1) {
        for (i = 1; i < len; i++) {
            if ([[args objectAtIndex:i] isEqual:@"-c"])
                server = [args objectAtIndex:++i];
            else if (([[args objectAtIndex:i] isEqual:@"--help"]) || ([[args objectAtIndex:i] isEqual:@"-?"])) {
                [pool release];
                exit(usage());
            } else {
                doHost = [args objectAtIndex:i];
            }
        }
    }
	*/
	
    if (args && ([args count] >= 2) &&
		(djvuPath = [args objectAtIndex:1]) &&
		([[NSFileManager defaultManager] fileExistsAtPath:djvuPath]) &&
		(tiff = bitmapImageRepFromDjVu(djvuPath, pagenum, dpi, size)) &&
		(data = [tiff TIFFRepresentation]))
    {		
		NSFileHandle *stdOutput = [NSFileHandle fileHandleWithStandardOutput];
		[stdOutput writeData:data];
		success = YES;
    }
	
    [pool release];
    exit(success ? 0 : 1); 
    return 0;      // ...and make main fit the ANSI spec.
}
