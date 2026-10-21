// Compatibility stubs for Android ARM64 native runtime
#include <stddef.h>

struct UEBinkAudioDecodeInterface;

// C++ linkage for UnrealBinkAudioDecodeInterface
UEBinkAudioDecodeInterface* UnrealBinkAudioDecodeInterface()
{
    return nullptr;
}

extern "C" {

int unw_backtrace(void **buffer, int size)
{
    return 0;
}

int _ULaarch64_init_local2(void *cursor, void *context, int flag)
{
    return -1;
}

int _ULaarch64_get_reg(void *cursor, int reg, void *val)
{
    return -1;
}

int _ULaarch64_step(void *cursor)
{
    return 0;
}

int curl_multi_wakeup(void* multi)
{
    return 0; // CURLM_OK
}

int curl_multi_poll(void* multi, void* extra_fds, unsigned int extra_nfds, int timeout_ms, int* ret)
{
    if (ret)
    {
        *ret = 0;
    }
    return 0; // CURLM_OK
}

}
