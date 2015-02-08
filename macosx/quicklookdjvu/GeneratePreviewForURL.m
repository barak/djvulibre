#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#import <Cocoa/Cocoa.h>

#include "ddjvuRef.h"

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus
GeneratePreviewForURL(void *thisInterface,
					  QLPreviewRequestRef preview,
					  CFURLRef url,
					  CFStringRef contentTypeUTI,
					  CFDictionaryRef options)
{
	CFBundleRef bundle = QLPreviewRequestGetGeneratorBundle(preview);
	char *ddjvu = ddjvuPath(bundle);
	if (ddjvu != NULL) {
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];		
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSDictionary *domain = [defaults persistentDomainForName:@"org.djvu.qlgenerator"];
		CFUUIDRef uuid = NULL;
		CFStringRef uuidString;
		BOOL debug = FALSE;
		NSString *tmpPath = nil;
		
		if (domain && [domain objectForKey:@"previewpages"])
			if ([[domain objectForKey:@"previewpages"] intValue] == 0)
				goto poppool;

		if (domain && [domain objectForKey:@"debug"]) {
			debug = [[domain objectForKey:@"debug"] boolValue];
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
			const char *cmd = NULL;
			CFStringRef cmdRef;
			int page = 0;
			int pages = 1;
			int maxpages = 5;
			int width = 612;	// default 8.5x11 in points
			int height = 792;
			CGSize size;
			MDItemRef mditem = NULL;
			NSString *source = (NSString *)CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			NSString *dest = nil;
			NSFileManager *fmgr = [NSFileManager defaultManager];
			[fmgr createDirectoryAtPath:tmpPath attributes:nil];
			
			mditem = MDItemCreate(kCFAllocatorDefault, (CFStringRef)source);
			if (mditem) {
				CFTypeRef ref = NULL;
				ref = MDItemCopyAttribute(mditem, kMDItemNumberOfPages);
				if (ref) {
					CFNumberGetValue(ref, kCFNumberIntType, &pages);
					CFRelease(ref);
				}
				ref = MDItemCopyAttribute(mditem, kMDItemPageWidth);
				if (ref) {
					CFNumberGetValue(ref, kCFNumberIntType, &width);
					CFRelease(ref);
				}
				ref = MDItemCopyAttribute(mditem, kMDItemPageHeight);
				if (ref) {
					CFNumberGetValue(ref, kCFNumberIntType, &height);
					CFRelease(ref);
				}
				CFRelease(mditem);				
			}
			
			if (domain && [domain objectForKey:@"previewpages"])
				maxpages = [[domain objectForKey:@"previewpages"] intValue];
			
			if (debug) {
				NSLog(@"metadata: pages=%d, width=%d, height=%d", pages, width, height);
				NSLog(@"maxpages=%d", maxpages);				
			}
						
			CGRect rect = CGRectMake(0, 0, width, height);
			CGContextRef c;
			c = QLPreviewRequestCreatePDFContext(preview, &rect, NULL, NULL);
			
			page = 0;
			do {
				page++;
				dest = [tmpPath stringByAppendingPathComponent:[[source lastPathComponent] stringByAppendingFormat:@"_t_p%04d.tiff", page]];
				size = CGSizeMake(width, height);
				cmdRef = CFStringCreateWithFormat(NULL, NULL, CFSTR("\"%s\" -format=tiff -page=%d -size=%dx%d \"%s\" \"%s\""), ddjvu, page, width, height, [source fileSystemRepresentation], [dest fileSystemRepresentation]);
				cmd = CFStringGetCStringPtr(cmdRef, CFStringGetSystemEncoding());
				if (cmd != NULL) {
					if (debug)
						NSLog(@"%s", cmd);
					if ((system(cmd) == 0) && ([fmgr fileExistsAtPath:dest])) {
						NSURL *durl = [NSURL fileURLWithPath:dest];
						CGImageSourceRef  sourceRef;
						
						sourceRef = CGImageSourceCreateWithURL((CFURLRef)durl, NULL);
						if(sourceRef) {
							CGImageRef imageRef = NULL;
							imageRef = CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL);
							if (imageRef) {
								CGFloat offset = height * (page-1);
								CGRectOffset(rect, 0.0, offset);
								CGPDFContextBeginPage(c, NULL);
								CGContextSaveGState(c);
								CGContextDrawImage(c, rect, imageRef);
								if (page == maxpages && maxpages < pages) {
									CGAffineTransform m;
									CFURLRef more = CFBundleCopyResourceURL(bundle, CFSTR("more_pages"), CFSTR("pdf"), NULL);
									CGPDFDocumentRef doc = CGPDFDocumentCreateWithURL(more);
									CGPDFPageRef pdf = CGPDFDocumentGetPage(doc, 1);
									CGFloat offset = height * page;
									CGRectOffset(rect, 0.0, offset);
									CGContextSaveGState(c);
									m = CGPDFPageGetDrawingTransform(pdf, kCGPDFMediaBox, rect, 0, true);
									CGContextConcatCTM(c, m);
									CGContextDrawPDFPage(c, pdf);
									CGContextRestoreGState(c);
									CFRelease(doc);
								}
								CGContextRestoreGState(c);
								CGPDFContextEndPage(c);
								CFRelease(imageRef);
							}
							CFRelease(sourceRef);
						}
					}
				}
				CFRelease(cmdRef);
			} while (page < pages && page < maxpages);

			CGPDFContextClose(c);
			QLPreviewRequestFlushContext(preview, c);
			CFRelease(c);
			[fmgr removeFileAtPath:tmpPath handler:nil];
			[source release];
		}
	poppool:
		[pool release];
	}
    return noErr;
}

void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview)
{
    // implement only if supported
}
