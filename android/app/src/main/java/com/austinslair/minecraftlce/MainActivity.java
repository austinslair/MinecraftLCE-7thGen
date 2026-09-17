package com.austinslair.minecraftlce;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

public final class MainActivity extends Activity {
    private GLSurfaceView glView;

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(3);
        glView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
glView.setPreserveEGLContextOnPause(true);
        glView.setRenderer(new MinecraftRenderer());
        glView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        FrameLayout root = new FrameLayout(this);
        root.addView(glView, new FrameLayout.LayoutParams(-1, -1));
        root.addView(new TouchOverlay(), new FrameLayout.LayoutParams(-1, -1));
        setContentView(root);
    }

    @Override protected void onResume() {
        super.onResume();
        if (glView != null) glView.onResume();
    }

    @Override protected void onPause() {
        if (glView != null) glView.onPause();
        super.onPause();
    }

    private final class TouchOverlay extends View {
        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);

        TouchOverlay() {
            super(MainActivity.this);
            setFocusable(false);
        }

        @Override public boolean onTouchEvent(MotionEvent event) {
            final int index = event.getActionIndex();
            NativeBridge.touch(
                event.getActionMasked(),
                event.getPointerId(index),
                event.getX(index),
                event.getY(index));
            return true;
        }

        @Override protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            float w = getWidth();
            float h = getHeight();
            float radius = Math.min(w, h) * 0.075f;

            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(Math.max(3f, radius * 0.08f));
            paint.setColor(0x99FFFFFF);

            canvas.drawCircle(w * 0.14f, h * 0.78f, radius, paint);
            canvas.drawCircle(w * 0.14f, h * 0.78f, radius * 0.35f, paint);
            canvas.drawCircle(w * 0.86f, h * 0.78f, radius, paint);

            paint.setStyle(Paint.Style.FILL);
            paint.setTextSize(Math.max(24f, h * 0.032f));
            canvas.drawText("JUMP", w * 0.79f, h * 0.61f, paint);
            canvas.drawText("USE", w * 0.885f, h * 0.78f, paint);
            canvas.drawText("MENU", w * 0.47f, h * 0.10f, paint);
        }
    }
}
