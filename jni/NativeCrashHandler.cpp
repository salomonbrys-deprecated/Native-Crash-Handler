
#include "NativeCrashHandler.h"

#include <jni.h>
#include <android/log.h>

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>


struct StackEntry {
	const char *mtd;
	const char *file;
	int line;
};

struct Stack {
	int capacity;
	int size;
	StackEntry *entries;
	const char *nextFile;
	int nextLine;

	Stack() : capacity(64), size(0), nextFile(NULL), nextLine(0) {
		entries = (StackEntry*)malloc(capacity * sizeof(StackEntry));
	}

	virtual ~Stack() {
		free(entries);
	}

	void push(const char *mtd, const char *file, int line) {
		if (size == capacity) {
			capacity += 64;
			entries = (StackEntry*)realloc(entries, capacity);
		}

		entries[size].mtd = mtd;
		entries[size].file = nextFile ? nextFile : file;
		entries[size].line = nextFile ? nextLine : line;

		nextFile = NULL;

		++size;
	}

	void pop() {
		nextFile = NULL;
		if (size == 0)
			return ;
		--size;
	}

	void next(const char *file, int line) {
		nextFile = file;
		nextLine = line;
	}
};

class Stacks {
	static pthread_key_t key;

public:
	static void destroy(void *arg) {
		Stack *stack = (Stack*)arg;
		delete stack;
	}

	static Stack *get() {
		if (key == INT_MIN)
			pthread_key_create(&key, Stacks::destroy);
		Stack *stack = (Stack*)pthread_getspecific(key);
		if (!stack) {
			stack = new Stack;
			pthread_setspecific(key, stack);
		}
		return stack;
	}

	static Stack *peak() {
		if (key == INT_MIN)
			return NULL;
		return (Stack*)pthread_getspecific(key);
	}
};

pthread_key_t Stacks::key = INT_MIN;


extern "C" {

static struct sigaction old_sa[NSIG];

static jclass applicationClass = 0;
static jmethodID makeCrashReportMethod;
static jobject applicationObject = 0;

static jclass stackTraceElementClass = 0;
static jmethodID stackTraceElementMethod;

static JavaVM *javaVM;

void __nativeCrashHandler_stack_call(const char *file, int line) {
	Stacks::get()->next(file, line);
}

void __nativeCrashHandler_stack_push(const char *mtd, const char *file, int line) {
	Stacks::get()->push(mtd, file, line);
}

void __nativeCrashHandler_stack_pop() {
	Stacks::get()->pop();
}

void makeNativeCrashReport(const char *reason) {
	JNIEnv *env = 0;

	int result = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

	if (result == JNI_EDETACHED) {
		__android_log_print(ANDROID_LOG_WARN, "NativeCrashHandler", "Native crash occured in a non jvm-attached thread");
		result = javaVM->AttachCurrentThread(&env, NULL);
	}

	jobjectArray elements = NULL;
	Stack *stack = Stacks::peak();
	if (stack && stack->size > 0) {
		__android_log_print(ANDROID_LOG_DEBUG, "NativeCrashHandler", "There is a native stack");
		elements = env->NewObjectArray(stack->size, stackTraceElementClass, NULL);
		int pos = 0;
		jstring jni = env->NewStringUTF("<JNI>");
		for (int i = stack->size - 1; i >= 0; --i) {
			jobject element = env->NewObject(stackTraceElementClass, stackTraceElementMethod,
					jni,
					env->NewStringUTF(stack->entries[i].mtd),
					env->NewStringUTF(stack->entries[i].file),
					stack->entries[i].line
			);
			env->SetObjectArrayElement(elements, pos++, element);
		}
	}

	if (result != JNI_OK)
		__android_log_print(ANDROID_LOG_ERROR, "NativeCrashHandler",
				"Could not attach thread to Java VM for crash reporting.\n"
				"Crash was: %s",
				reason
		);
	else if (env && applicationObject)
		env->CallVoidMethod(applicationObject, makeCrashReportMethod, env->NewStringUTF(reason), elements, (jint)gettid());
	else
		__android_log_print(ANDROID_LOG_ERROR, "NativeCrashHandler",
				"Could not create native crash report as registerForNativeCrash was not called in JAVA context.\n"
				"Crash was: %s",
				reason
		);
}

void nativeCrashHandler_sigaction(int signal, struct siginfo*, void*) {

	makeNativeCrashReport(strsignal(signal));

	if (old_sa[signal].sa_handler)
		old_sa[signal].sa_handler(signal);
}

JNIEXPORT jboolean JNICALL Java_com_github_nativehandler_NativeCrashHandler_nRegisterForNativeCrash(JNIEnv * env, jobject obj) {
	if (applicationClass) {
		applicationObject = (jobject)env->NewGlobalRef(obj);
		return 1;
	}

	return 0;
}

JNIEXPORT void JNICALL Java_com_github_nativehandler_NativeCrashHandler_nUnregisterForNativeCrash(JNIEnv *env, jobject) {
	if (applicationObject) {
		env->DeleteGlobalRef(applicationObject);
	}
}

void nativeCrashHandler_onLoad(JavaVM *jvm) {
	javaVM = jvm;

	JNIEnv *env = 0;
	int result = jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
	assert(result == JNI_OK);

	applicationClass = env->FindClass("com/github/nativehandler/NativeCrashHandler");
	applicationClass = (jclass)env->NewGlobalRef(applicationClass);
	makeCrashReportMethod = env->GetMethodID(applicationClass, "makeCrashReport", "(Ljava/lang/String;[Ljava/lang/StackTraceElement;I)V");

	stackTraceElementClass = env->FindClass("java/lang/StackTraceElement");
	stackTraceElementClass = (jclass)env->NewGlobalRef(stackTraceElementClass);
	stackTraceElementMethod = env->GetMethodID(stackTraceElementClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

	struct sigaction handler;
	memset(&handler, 0, sizeof(handler));
	handler.sa_sigaction = nativeCrashHandler_sigaction;
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
