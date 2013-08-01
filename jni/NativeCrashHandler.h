
#ifndef __NATIVE_CRASH_HANDLER__
#define __NATIVE_CRASH_HANDLER__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void nativeCrashHandler_onLoad(JavaVM* jvm);

void makeNativeCrashReport(const char *reason);

//extern char * (*crash_data_getter)();

void __nativeCrashHandler_stack_push(const char *mtd, const char *file, int line);
void __nativeCrashHandler_stack_pop();

#ifndef NDEBUG
#define native_stack_start()       __nativeCrashHandler_stack_push(__func__, __FILE__, __LINE__)
#define native_stack_start_r(cls)  __nativeCrashHandler_stack_push(mtd, __FILE__, __LINE__)
#define native_stack_end()         __nativeCrashHandler_stack_pop()
#define native_stack_return()      { __nativeCrashHandler_stack_pop(); return ; }
#define native_stack_return_v(v)   { __nativeCrashHandler_stack_pop(); return v; }
#else
#define native_stack_start()
#define native_stack_start_r(cls)
#define native_stack_end()
#define native_stack_return()      return ;
#define native_stack_return_v(v)   return v;
#endif

#ifdef __cplusplus
}
#endif

#endif // !__NATIVE_CRASH_HANDLER__
