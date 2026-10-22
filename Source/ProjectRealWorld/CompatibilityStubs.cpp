// Compatibility stubs for Android ARM64 native runtime
#include <stddef.h>

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

}
