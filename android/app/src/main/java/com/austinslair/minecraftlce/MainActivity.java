package com.austinslair.minecraftlce;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;

public final class MainActivity extends Activity {
    private GLSurfaceView glView;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        );
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        NativeBridge.initPlatform(getAssets());

        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(3);
        glView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        glView.setPreserveEGLContextOnPause(true);
        glView.setRenderer(new MinecraftRenderer());
        glView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        glView.setOnTouchListener((view, event) -> {
            forwardTouchEvent(event);
            return true;
        });

        // Android owns only the window/surface. Menus, HUD, gameplay and rendering
        // belong to the original Legacy Console engine and must not be recreated here.
        setContentView(glView);
    }

    private void forwardTouchEvent(MotionEvent event) {
        final int action = event.getActionMasked();

        if (action == MotionEvent.ACTION_MOVE) {
            for (int index = 0; index < event.getPointerCount(); ++index) {
                NativeBridge.touch(
                    action,
                    event.getPointerId(index),
                    event.getX(index),
                    event.getY(index)
                );
            }
            return;
        }

        final int index = event.getActionIndex();
        NativeBridge.touch(
            action,
            event.getPointerId(index),
            event.getX(index),
            event.getY(index)
        );
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (glView != null) {
            glView.onResume();
        }
    }

    @Override
    protected void onPause() {
        if (glView != null) {
            glView.onPause();
        }
        super.onPause();
    }
}
