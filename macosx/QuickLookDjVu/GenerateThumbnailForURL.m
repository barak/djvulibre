#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#import <Cocoa/Cocoa.h>
#include <ApplicationServices/ApplicationServices.h>

#include "ddjvuRef.h"

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

OSStatus
GenerateThumbnailForURL(void *thisInterface,
						QLThumbnailRequestRef thumbnail,
						CFURLRef url,
						CFStringRef contentTypeUTI,
						CFDictionaryRef options,
						CGSize maxSize)
{
	char *ddjvu = ddjvuPath(QLThumbnailRequestGetGeneratorBundle(thumbnail));
	if (ddjvu != NULL) {
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSDictionary *domain = [defaults persistentDomainForName:@"org.djvu.qlgenerator"];
		CFUUIDRef uuid;
		CFStringRef uuidString;
		NSString *tmpPath = nil;
		BOOL debug = FALSE;
		
		if (domain && [domain objectForKey:@"debug"]) {
			debug = [[domain objectForKey:@"debug"] boolValue];
		}
		if (domain && [domain objectForKey:@"thumbnail"] &&
			([[domain objectForKey:@"thumbnail"] boolValue] == FALSE))
		{
			if (debug)
				NSLog(@"skip thumbnail for %@", url);
			CFStringRef ext = CFURLCopyPathExtension(url);
			if (ext) {
				NSImage *img = [[NSWorkspace sharedWorkspace] iconForFileType:(NSString*)ext];
				if (img) {
					NSData *data = [img TIFFRepresentation];
					QLThumbnailRequestSetImageWithData(thumbnail, (CFDataRef)data, NULL);
				}
				CFRelease(ext);
			}
		} else {
			if (debug) {
				CFShow(contentTypeUTI);
				CFShow(options);			
			}
			
			uuid = CFUUIDCreate(kCFAllocatorDefault);
			if (uuid) {
				uuidString = CFUUIDCreateString(kCFAllocatorDefault, uuid);
				if (uuidString) {
					tmpPath = [NSTemporaryDirectory() stringByAppendingPathComponent:
							   [NSString stringWithFormat:@"djvuql-%@", uuidString]];
					CFRelease(uuidString);
				}
				CFRelease(uuid);
			}
			if (tmpPath) {
				NSFileManager *fmgr = [NSFileManager defaultManager];
				const char *cmd = NULL;
				CFStringRef cmdRef;
				int page = 1;
				NSString *source = (NSString *)CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
				NSString *dest = [tmpPath stringByAppendingPathComponent:[[source lastPathComponent] stringByAppendingFormat:@"_t_p%d.tiff", page]];
				[fmgr createDirectoryAtPath:tmpPath attributes:nil];			
				cmdRef = CFStringCreateWithFormat(NULL, NULL, CFSTR("\"%s\" -format=tiff -page=%d -size=%dx%d \"%s\" \"%s\""), ddjvu, page, (int)maxSize.width, (int)maxSize.height, [source fileSystemRepresentation], [dest fileSystemRepresentation]);
				cmd = CFStringGetCStringPtr(cmdRef, CFStringGetSystemEncoding());
				if (cmd != NULL) {
					if (debug)
						NSLog(@"ddjvu: %s", cmd);
					if (system(cmd) == 0) {
						NSURL *durl = [NSURL fileURLWithPath:dest];
						CGImageRef imageRef = NULL;
						CGImageSourceRef  sourceRef;
						
						sourceRef = CGImageSourceCreateWithURL((CFURLRef)durl, NULL);
						if(sourceRef) {
							imageRef = CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL);
							if (imageRef)
								QLThumbnailRequestSetImage(thumbnail, imageRef, NULL);
							CFRelease(sourceRef);
						}
					}
				}
				[fmgr removeFileAtPath:tmpPath handler:nil];
				CFRelease(cmdRef);
				[source release];
			}
		}
		[pool release];
	}
    return noErr;
}

void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail)
{
    // implement only if supported
}
