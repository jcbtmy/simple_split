Split

Description:
Split.c is a simple command line tool that splits a file into multiple pieces. This tool takes in a [-s <split number>| -g]  command in order to split up or retrieve a file. Once a file is split into multiple pieces it is placed in a directory under the current dir labeled with the same name as the file without the extension. In order to retrieve a file, the user can use the -g command outside the directory of the directory.

Usage:

./split [-s(split) | -g(get)] <filename> <partitions_number>


