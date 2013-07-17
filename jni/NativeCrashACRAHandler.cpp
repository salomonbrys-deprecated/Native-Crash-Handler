
#include <jni.h>
#include <android/log.h>

#include <signal.h>
#include <string.h>

extern "C" {

static struct sigaction old_sa[NSIG];

static JNIEnv *env = 0;
static jclass applicationClass = 0;
static jmethodID makeCrashReportMethod;
static jobject applicationObject = 0;

void nativeCrashACRAHandler_sigaction(int signal, struct siginfo *info, void *reserved) {

	if (env && applicationObject)
		env->CallVoidMethod(applicationObject, makeCrashReportMethod, env->NewStringUTF(strsignal(signal)));
	else
		__android_log_print(ANDROID_LOG_ERROR, "NativeCrashACRAHandler", "Could not create native crash report as registerForNativeCrash was not called in JAVA context");

	if (old_sa[signal].sa_handler)
		old_sa[signal].sa_handler(signal);
}

JNIEXPORT jboolean JNICALL Java_com_github_nativeacra_NativeCrashACRAHandler_nRegisterForNativeCrash(JNIEnv * env, jobject obj) {
	if (env) {
		applicationObject = (jobject)env->NewGlobalRef(obj);
		return 1;
	}

	return 0;
}

JNIEXPORT void JNICALL Java_com_github_nativeacra_NativeCrashACRAHandler_nUnregisterForNativeCrash(JNIEnv * env, jobject obj) {
	if (env && applicationObject)
		env->DeleteGlobalRef(applicationObject);
}

void nativeCrashACRAHandler_onLoad(JavaVM* jvm) {
	jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

	applicationClass = env->FindClass("com/github/nativeacra/NativeCrashACRAHandler");
	applicationClass = (jclass)env->NewGlobalRef(applicationClass);
	makeCrashReportMethod = env->GetMethodID(applicationClass, "makeCrashReport", "(Ljava/lang/String;)V");

	struct sigaction handler;
	memset(&handler, 0, sizeof(handler));
	handler.sa_sigaction = nativeCrashACRAHandler_sigaction;
	handler.sa_flags = SA_RESETHAND;
	sigaction(SIGILL,    &handler, &old_sa[SIGILL]    );
	sigaction(SIGABRT,   &handler, &old_sa[SIGABRT]   );
	sigaction(SIGBUS,    &handler, &old_sa[SIGBUS]    );
	sigaction(SIGFPE,    &handler, &old_sa[SIGFPE]    );
	sigaction(SIGSEGV,   &handler, &old_sa[SIGSEGV]   );
	sigaction(SIGSTKFLT, &handler, &old_sa[SIGSTKFLT] );
	sigaction(SIGPIPE,   &handler, &old_sa[SIGPIPE]   );
}

}
