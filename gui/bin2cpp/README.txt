bin2cpp is a program, which I'm using to compile graphics (button icons,
startup image) into the executable.

It reads the source file, compresses it using BSByteStream and stores results
into a CPP file. The compiled-in data is supposed to be accessed by the
program using CINData class (cin_data.*)
