extern void _entry(void);
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Forward prototypes.  */
// int _system (const char *){}
// int _rename (const char *, const char *){}
// int _isatty (int){}
// clock_t _times (struct tms *){}
// int _gettimeofday (struct timeval *, void *){}
// int _unlink (const char *){}
// int _link (void){}
// int _stat (const char *, struct stat *){}
// int _swistat (int fd, struct stat * st){}
// int _getpid (int){}
// clock_t _clock (void){}
// int _swiclose (int){}
// int _open (const char *, int, ...){}
// int _swiopen (const char *, int){}
// int _swiwrite (int, char *, int){}
// int _lseek (int, int, int){}
// int _swilseek (int, int, int){}
// int _read (int, char *, int){}
// int _swiread (int, char *, int){}
// void initialise_monitor_handles (void){}
int _fstat (int, struct stat * stat){
    (void)stat;
}
caddr_t _sbrk (int){}
int _close (int){}
int _write (int fd, char *ptr, int len){
    (void)fd;
    (void)ptr;
    (void)len;
    asm(".word 0");
    return len;
}


int main(int argc, char const *argv[])
{
    return printf("hello risc-v\n");
}