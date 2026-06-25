#pragma once
/** Optional crash/debug diagnostics for driver/test binaries (not public ABI). */
#ifdef __cplusplus
extern "C" {
#endif
void nwchemc_debug_install_handlers(void);
void nwchemc_debug_print_backtrace(const char *context);
#ifdef __cplusplus
}
#endif
