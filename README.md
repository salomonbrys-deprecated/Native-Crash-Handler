# Native-Crash-Handler

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
    #include "NativeCrashHandler.h"
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



# Advanced usage

## Activity Context

Sometimes, you need to register the *NativeCrashACRAHandler* with an *Activity Context* instead of an *Application Context*.
Please remember that this should not be done if you can use the *Application Context*.

When you use *NativeCrashACRAHandler.registerForNativeCrash* with an *Activity Context*, the activity is **retained** by the NativeCrashACRAHandler JNI code.
Therefore, you need to call *NativeCrashACRAHandler.unregisterForNativeCrash* when the activity is paused, otherwise the application will never be relased by the system.
Remember that only ONE context needs to be registered at a time, so you need to unregister an activity **before** register a new one.

