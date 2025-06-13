/* These stub functions are provided to prevent linker warnings related to
 * unimplemented system calls, such as `_close`, `_fstat`, etc., which are
 * typically required by the C standard library (newlib) in environments
 * with full operating system support.
 *
 * In embedded systems, where a full file system or operating system is
 * not present, these system calls are not needed. However, the newlib
 * library may still reference them, leading to linker warnings.
 *
 * These stub implementations provide minimal, do-nothing versions of
 * these functions, which return error codes where appropriate. This
 * allows the code to compile and link successfully without introducing
 * unnecessary dependencies or causing runtime errors.
 */
#include <stdlib.h>

struct stat
{
    int st_mode;  // Mode of file
    int st_size;  // Size of file
};

int _close(int file)
{
    return -1; // Indicate error
}

int _fstat(int         file,
           struct stat *st)
{
    return -1; // Indicate error
}

int _isatty(int file)
{
    return 0; // Not a terminal
}

_off_t _lseek(int    file,
              _off_t offset,
              int    whence)
{
    return -1; // Indicate error
}

int _kill(int pid,
          int sig)
{
    return -1; // Indicate error
}

int _getpid(void)
{
    return 1; // Return a dummy PID
}
