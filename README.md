Native-Crash-ACRA-Handler
=========================

A simple set of code to enable ACRA to catch JNI Native crashes


Why it exists
-------------

While [ACRA](https://github.com/ACRA/acra) is a tremendous tool for Android JAVA developers, it cannot catch JNI C / C++ crashes because those crashes terminate the process and are not catchable on the JAVA side


Do you need it ?
----------------

If you are using ACRA **and** are programming using the Android NDK, then yes.

Otherwise, no.


How it works
------------

On the JNI side, it registers sigaction callbacks to be called when the program receives an error signal (such as SIGSEGV: segmentation fault).
It then generates an exception to capture the stack trace and then launches a new activity **in a new process** (because the current is to be killed by the signal) which will triger ACRA reporting tool.

This tool has been largely inspired by [Chris Boyle's great SO answer](http://stackoverflow.com/a/1789879/1269640).


How to use it
-------------

Your application must already be configured to use [ACRA](https://github.com/ACRA/acra)

1.  [Download the archive](https://github.com/SalomonBrys/Native-Crash-ACRA-Handler/blob/master/NativeCrashACRAHandler.tgz?raw=true) and extract it

2.  Put *NativeCrashACRAHandler.jar* in the libs/ directory of your project

3.  Put *NativeCrashACRAHandler.h* and *NativeCrashACRAHandler.cpp* in the jni/ directory of your project

4.  Add *NativeCrashACRAHandler.cpp* to the LOCAL_SRC_FILES variable in your *jni/Android.mk* file

5.  In your application class, just after calling *ACRA.init(this);* in *onCreate*, add:

        new NativeCrashACRAHandler().registerForNativeCrash(this);

6.  In The JNI_OnLoad JNI C function, add:

        #include "nativeCrashACRAHandler.h"
        /*...*/
        nativeCrashACRAHandler_onLoad(jvm);

7.  In your AndroidManifest.xml, add:

        <activity
            android:name="com.github.nativeacra.NativeCrashACRAActivity"
            android:configChanges="keyboard|keyboardHidden|orientation"
            android:exported="false"
            android:process=":CrashHandler"
            android:stateNotNeeded="true"
            android:theme="@android:style/Theme.Translucent.NoTitleBar" />

8.  ***You're done***


Advanced use
------------

Sometimes, you need to register the *NativeCrashACRAHandler* with an *Activity Context* instead of an *Application Context*.
Please remember that this should not be done if you can use the *Application Context*.

When you use *NativeCrashACRAHandler.registerForNativeCrash* with an *Activity Context*, the activity is **retained** by the NativeCrashACRAHandler JNI code.
Therefore, you need to call *NativeCrashACRAHandler.unregisterForNativeCrash* when the activity is paused, otherwise the application will never be relased by the system.
Remember that only ONE context needs to be registered at a time, so you need to unregister an activity **before** register a new one.

