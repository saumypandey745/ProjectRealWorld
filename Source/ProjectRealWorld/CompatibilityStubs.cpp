// Compatibility stubs for Android ARM64 native runtime
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>

struct UEBinkAudioDecodeInterface;

// C++ linkage for UnrealBinkAudioDecodeInterface
__attribute__((weak)) UEBinkAudioDecodeInterface* UnrealBinkAudioDecodeInterface()
{
    return nullptr;
}

extern "C" {

__attribute__((weak)) int unw_backtrace(void **buffer, int size)
{
    return 0;
}

__attribute__((weak)) int _ULaarch64_init_local2(void *cursor, void *context, int flag)
{
    return -1;
}

__attribute__((weak)) int _ULaarch64_get_reg(void *cursor, int reg, void *val)
{
    return -1;
}

__attribute__((weak)) int _ULaarch64_step(void *cursor)
{
    return 0;
}

// ── Bionic <-> Glibc bridge stubs for third-party prebuilt libraries ───────────

int* __errno_location(void)
{
    return __errno();
}

int bcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

char* __strtok_r(char *str, const char *delim, char **saveptr)
{
    return strtok_r(str, delim, saveptr);
}

void* __rawmemchr(const void *s, int c)
{
    const unsigned char *p = (const unsigned char *)s;
    while (*p != (unsigned char)c)
    {
        p++;
    }
    return (void*)p;
}

int __isoc99_sscanf(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vsscanf(str, format, args);
    va_end(args);
    return res;
}

char* __xpg_basename(char *path)
{
    return basename(path);
}

int __xpg_strerror_r(int errnum, char *buf, size_t buflen)
{
    char* msg = strerror_r(errnum, buf, buflen);
    if (msg && msg != buf && buflen > 0)
    {
        strncpy(buf, msg, buflen - 1);
        buf[buflen - 1] = '\0';
    }
    return 0;
}

int __fxstat64(int ver, int fildes, void *stat_buf)
{
    return fstat(fildes, (struct stat*)stat_buf);
}

int __xstat64(int ver, const char *path, void *stat_buf)
{
    return stat(path, (struct stat*)stat_buf);
}

}
