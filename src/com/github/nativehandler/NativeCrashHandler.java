package com.github.nativehandler;

import android.content.Context;
import android.content.Intent;

public class NativeCrashHandler {

	Context ctx;
	
	private void makeCrashReport(String reason, StackTraceElement[] stack, int threadID) {
		
		if (stack != null)
			NativeError.natSt = stack;
		NativeError e = new NativeError(reason, threadID);
		Intent intent = new Intent(ctx, NativeCrashActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.putExtra("error", e);
		ctx.startActivity(intent);
	}
	
	public void registerForNativeCrash(Context ctx) {
		this.ctx = ctx;
		if (!nRegisterForNativeCrash())
			throw new RuntimeException("Could not register for native crash as nativeCrashHandler_onLoad was not called in JNI context");
	}

	public void unregisterForNativeCrash() {
		this.ctx = null;
		nUnregisterForNativeCrash();
	}

	private native boolean nRegisterForNativeCrash();
	private native void nUnregisterForNativeCrash();

}
