
#ifndef __NATIVE_CRASH_ACRA_HANDLER__
#define __NATIVE_CRASH_ACRA_HANDLER__

#ifdef __cplusplus
extern "C" {
#endif

void nativeCrashACRAHandler_onLoad(JavaVM* jvm);

void makeCrashReport(const char *reason);

#ifdef __cplusplus
}
#endif

#endif
