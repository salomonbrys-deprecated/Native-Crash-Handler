Native-Crash-Handler
====================

A simple set of code to enable JNI Native crashes to be caught by java and throw a meaningfull exception


## Why it exists

While java exceptions are a tremendous tool for Android JAVA developers to investigate crashes, JNI C / C++ crashes terminate the process and are not catchable by the system.


## Can it work with crash reporters like [ACRA](https://github.com/ACRA/acra) ?

Yes ! I'm glad you ask : That's the reason why it was developped in the first place !
As this throws an exception, a crash handler can intercept it and handle it the way it needs.


## How it works

On the JNI side, it registers sigaction callbacks to be called when the program receives an error signal (such as SIGSEGV: segmentation fault).
It then generates an exception to capture the stack trace and then launches a new activity **in a new process** (because the current is to be killed by the signal) which will throw an exception.

This tool has been largely inspired by [Chris Boyle's great SO answer](http://stackoverflow.com/a/1789879/1269640).


## How to use it

1.  [Download the archive](https://github.com/SalomonBrys/Native-Crash-Handler/blob/master/NativeCrashHandler.tgz?raw=true) and extract it

2.  Put *NativeCrashHandler-vX.Y.jar* in the libs/ directory of your project

3.  Put *NativeCrashHandler.h* and *NativeCrashHandler.cpp* in the jni/ directory of your project

4.  Add *NativeCrashHandler.cpp* to the LOCAL_SRC_FILES variable in your *jni/Android.mk* file

5.  In your application class, in *onCreate*, add:
```java
    new NativeCrashHandler().registerForNativeCrash(this);
```

6.  In The JNI_OnLoad JNI C function, add:
```java
    #include "nativeCrashHandler.h"
    /*...*/
    nativeCrashHandler_onLoad(jvm);
```

7.  In your AndroidManifest.xml, add:
```xml
    <activity
        android:name="com.github.nativehandler.NativeCrashActivity"
        android:configChanges="keyboard|keyboardHidden|orientation"
        android:exported="false"
        android:process=":CrashHandler"
        android:stateNotNeeded="true"
        android:theme="@android:style/Theme.Translucent.NoTitleBar" />
```

8.  ***You're done***


## Stack Trace

***DISCLAIMER:*** *I have not found an automated way to get a native stack trace from a signal handler. While the solution proposed here is really sub-optimal, this is the only one I could think of.
If you know a better way, please please* ***PLEASE answer this [SO question](http://stackoverflow.com/questions/18017222/android-unwind-backtrace-inside-sigaction)*** *!*

***THIS IS OPTIONAL AND NOT REQUIRED***

Having a stack trace in an exception is always very useful. Unfortunately, in the native context, you'll have to manually create / fill it. Doing so is both simple and complicated.

The solution provided in this library implies overhead for each function call. For this reason, it is completely disabled when compiling in release mode (NDEBUG is defined).
Native stack traces will only work on projects that are built on debug configuration. You can leave the stack lines of code in release as they are defined empty in release and will then not make any overhead.

#### The simple part:

First, all of your functions / methods should start with:

```C++
    native_stack_start;
```

Then, all of your functions should end with:

```C++
    native_stack_end;
```

Be careful as this should be put before ANY return, including of course middle function returns;

Entering a function with `native_stack_start` and NOT leaving it with `native_stack_end` or vice versa **will corrupt the stacktrace** and give you very strange stack to analyse...

These two defines are enough to get a stack trace. However, in the generated stack trace, you will get the file and line of the first line of the function, not the line that called the function.
To get the reference of the calling line, put:

```C++
	native_stack_call;
```

on the same line, just before the call, which gives:

```C++
	native_stack_call; myFunction(42);
```

#### The complicated part:

Remember to NOT put an expression inside a return clause, as, if the expression were to crash, you would have already called `native_stack_end` to declare the end of the function. So, these lines:

```C++
	native_stack_end;
	return functioncall();

	native_stack_end;
	return obj->property;
```

Should be replaced by:

```C++
	int ret = functioncall();
	native_stack_end;
	return ret;

	int ret = obj->property;
	native_stack_end;
	return ret;
```

#### The trick that makes things more both readable and complicated:

If you wish, you can put this at the top of your .c / .cpp source file:

```C++
	#define _start  native_stack_start;
	#define _end    native_stack_end;
	#define _call   native_stack_call;
```

This makes code more readable by making stack calls easier to read and adding semicolon directly in the define enables the possibility to prefix lines and make it more readable:

```C++
	int myFunction() {
		_start;
		/* ... */
		_call TestClass::Method();
		/* ... */
		_call int ret = anotherFunction();
		_end return ret;
	}
```

When you are using this, always keep in mind that `_start`, `_end` and `_call` are in fact semicolon terminated function calls. Which means that non-braces single line block will NOT work and result in a very strange behaviour.

```C++
	if (whatever)
		_call testFunc(); // NEVER DO THIS !!!
```

is in fact:

```C++
	if (whatever)
		native_stack_call;
	testFunc(); // OOPS !
```

and should be replaced with:

```C++
	if (whatever) { // NOTE THE BRACES
		_call testFunc();
	}
```

## Advanced use: Activity Context

Sometimes, you need to register the *NativeCrashACRAHandler* with an *Activity Context* instead of an *Application Context*.
Please remember that this should not be done if you can use the *Application Context*.

When you use *NativeCrashACRAHandler.registerForNativeCrash* with an *Activity Context*, the activity is **retained** by the NativeCrashACRAHandler JNI code.
Therefore, you need to call *NativeCrashACRAHandler.unregisterForNativeCrash* when the activity is paused, otherwise the application will never be relased by the system.
Remember that only ONE context needs to be registered at a time, so you need to unregister an activity **before** register a new one.

