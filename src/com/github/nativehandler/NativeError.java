package com.github.nativehandler;


public class NativeError extends Error {

	private static final long serialVersionUID = 1L;

	static StackTraceElement[] natSt = new StackTraceElement[0];
	
	public NativeError(String reason, int threadID) {
		super(reason + " in thread " + threadID);
	}
	
	@Override
	public Throwable fillInStackTrace() {
		super.fillInStackTrace();
		StackTraceElement[] st = getStackTrace();
		
		StackTraceElement[] clst = new StackTraceElement[natSt.length + st.length - 1];

		int p = 0;
		
		for (int i = 0; i < natSt.length; ++i)
			clst[p++] = natSt[i];
		
		for (int i = 1; i < st.length; ++i)
			clst[p++] = st[i];
		
		setStackTrace(clst);
		return this;
	}
}
