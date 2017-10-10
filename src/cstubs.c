#include <sys/stat.h>
#include <uart.h>

/* Increase program data space. As malloc and related functions depend on this,
   it is useful to have a working implementation. The following suffices for a
   standalone system; it exploits the symbol _end automatically defined by the
   GNU linker. */
caddr_t _sbrk(int incr) {
    extern char __end;
    static char* heap_end = 0;
    char* prev_heap_end;

    if (heap_end == 0)
        heap_end = &__end;

    prev_heap_end = heap_end;

    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

/* There's currently no implementation of a file system because there's no
   file system! */
int _close(int file) {
    return -1;
}

/* Status of an open file. For consistency with other minimal implementations
   in these examples, all files are regarded as character special devices. The
   sys/stat.h header file required is distributed in the include subdirectory
   for this C library. */
int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

/* Query whether output stream is a terminal. For consistency with the other
   minimal implementations, which only support output to stdout, this minimal
   implementation is suggested: */
int _isatty(int file) {
    return 1;
}

/* Set position in a file. Minimal implementation: */
int _lseek(int file, int ptr, int dir) {
    return 0;
}

/* Read from a file. Minimal implementation: */
int _read(int file, char *ptr, int len) {
    return 0;
}

void outbyte(char b) {
    uart_putc(b);
}

/* Write to a file. libc subroutines will use this system routine for output to
   all files, including stdout—so if you need to generate any output, for
   example to a serial port for debugging, you should make your minimal write
   capable of doing this. The following minimal implementation is an
   incomplete example; it relies on a outbyte subroutine (not shown; typically,
   you must write this in assembler from examples provided by your hardware
   manufacturer) to actually perform the output. */
int _write(int file, char *ptr, int len) {
    int todo;
    for (todo=0 ; todo<len ; todo++)
        outbyte(*ptr++);
    return len;
}
