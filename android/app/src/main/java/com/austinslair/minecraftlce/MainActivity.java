package com.austinslair.minecraftlce;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.graphics.Canvas;
import android.graphics.Paint;

public final class MainActivity extends Activity {
    static { System.loadLibrary("minecraft_android"); }

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(new TouchSurface());
    }

    private final class TouchSurface extends View {
        TouchSurface() { super(MainActivity.this); setFocusable(true); }
        @Override public boolean onTouchEvent(MotionEvent event) {
            NativeBridge.touch(event.getActionMasked(), event.getPointerId(event.getActionIndex()), event.getX(), event.getY());
            return true;
        }
        @Override protected void onDraw(Canvas canvas) {
            Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
            p.setColor(0xFF101010);
            canvas.drawRect(0, 0, getWidth(), getHeight(), p);
            p.setColor(0x99FFFFFF); p.setStyle(Paint.Style.STROKE); p.setStrokeWidth(4);
            float r = Math.min(getWidth(), getHeight()) * .09f;
            canvas.drawCircle(getWidth() * .14f, getHeight() * .76f, r, p);
            canvas.drawCircle(getWidth() * .86f, getHeight() * .76f, r, p);
            p.setStyle(Paint.Style.FILL); p.setTextSize(Math.max(28, getHeight() * .035f));
            canvas.drawText("JUMP", getWidth() * .80f, getHeight() * .62f, p);
            canvas.drawText("USE", getWidth() * .90f, getHeight() * .76f, p);
        }
    }
}
