#!/bin/sh -x

gcc -bundle -arch x86_64 -arch i386 -o DjVu main.c GetMetadataForFile.m -ldjvulibre -framework System -framework CoreFoundation -framework CoreServices -framework Foundation

