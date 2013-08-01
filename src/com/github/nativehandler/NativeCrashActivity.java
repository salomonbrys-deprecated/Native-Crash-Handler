package com.github.nativehandler;

import android.app.Activity;
import android.os.Bundle;

public class NativeCrashActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		NativeError e = (NativeError)getIntent().getSerializableExtra("error");

		throw e;
	}
	
}
