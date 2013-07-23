package com.github.nativeacra;

public class NativeError extends Error {

	private static final long serialVersionUID = 1L;

	public NativeError(String reason) {
		super("JNI Native crash: " + reason);
	}
	
}
