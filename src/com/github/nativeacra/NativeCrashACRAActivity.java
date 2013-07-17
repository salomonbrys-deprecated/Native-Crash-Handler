package com.github.nativeacra;

import org.acra.ACRA;

import android.app.Activity;
import android.os.Bundle;

public class NativeCrashACRAActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		Throwable e = (Throwable)getIntent().getSerializableExtra("error");
		
		ACRA.getErrorReporter().handleException(e, false);
		
		finish();
	}
	
}
