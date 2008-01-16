/*
 *  ddjvuRef.c
 *  QuickLookDjVu
 *
 *  Created by Jeff Sickel on 11/27/07.
 *  Copyright 2007 Corpus Callosum Corporation. All rights reserved.
 *
 */

#include "ddjvuRef.h"

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

#include <sys/param.h> /* for MAXPATHLEN */
#include <sys/types.h>
#include <sys/stat.h>

CFURLRef
ddjvuURL(CFBundleRef bundle)
{
	CFURLRef ddjvuRef = NULL;
	
	if (bundle) {
		ddjvuRef = CFBundleCopyResourceURL(bundle, CFSTR("ddjvu"), NULL, NULL);
		if (ddjvuRef == NULL) {
			CFURLRef url = CFBundleCopyBundleURL(bundle);
			CFMutableStringRef path = NULL;
			CFStringRef bpath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			path = CFStringCreateMutableCopy(kCFAllocatorDefault, 1024, bpath);
			CFStringTrim(path, CFSTR("Library/QuickLook/djvu.qlgenerator"));
			CFStringAppend(path, CFSTR("bin/ddjvu"));
			CFShow(path);
			CFRelease(bpath);
		}
	}
	if (ddjvuRef == NULL) {
		CFBundleRef djviewBundle = NULL;
		djviewBundle = CFBundleGetBundleWithIdentifier(CFSTR("org.djvu.DjView"));
		if (djviewBundle == NULL) {
			OSStatus stat;
			FSRef djviewPath;
			CFURLRef refURL;
			stat = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("org.djvu.DjView"), NULL, &djviewPath, &refURL);
			if (stat == noErr) {
				djviewBundle = CFBundleCreate(kCFAllocatorDefault, refURL);
			}
		}
		
		if (djviewBundle != NULL) {
			CFURLRef url = CFBundleCopyBundleURL(djviewBundle);
			ddjvuRef = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault,
															 url,
															 CFSTR("Contents/bin/ddjvu"),
															 false);
			CFRelease(url);
			CFRelease(djviewBundle);
		}		
	}
	return ddjvuRef;
}

char *
ddjvuPath(CFBundleRef bundle)
{
	static char *ddjvuRef = NULL;
	if (ddjvuRef == NULL) {
		CFURLRef url = ddjvuURL(bundle);
		if (url != NULL) {
			char buf[MAXPATHLEN];
			if (CFURLGetFileSystemRepresentation(url, true, (UInt8 *)buf, MAXPATHLEN))
			{
				ddjvuRef = (char *)malloc(strlen(buf));
				strcpy(ddjvuRef, buf);
			}
			CFRelease(url);
		}
	} else {
		struct stat fstat;
		if (stat(ddjvuRef, &fstat) != 0){
			free(ddjvuRef);
			ddjvuRef = NULL;
			return ddjvuPath(bundle);
		}
	}
	return ddjvuRef;
}
