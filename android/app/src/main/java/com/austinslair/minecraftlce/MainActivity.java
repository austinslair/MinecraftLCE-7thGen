package com.austinslair.minecraftlce;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public final class MainActivity extends Activity {
    private GLSurfaceView glView;
    private MenuOverlay menuOverlay;
    private int currentScreen = 0; // 0 = Main Menu, 1 = In Game

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        NativeBridge.initAssets(getAssets());

        glView = new GLSurfaceView(this);
        glView.setEGLContextClientVersion(3);
        glView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        glView.setPreserveEGLContextOnPause(true);
        glView.setRenderer(new MinecraftRenderer());
        glView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        menuOverlay = new MenuOverlay();

        FrameLayout root = new FrameLayout(this);
        root.addView(glView, new FrameLayout.LayoutParams(-1, -1));
        root.addView(menuOverlay, new FrameLayout.LayoutParams(-1, -1));
        setContentView(root);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (glView != null) glView.onResume();
    }

    @Override
    protected void onPause() {
        if (glView != null) glView.onPause();
        super.onPause();
    }

    private final class MenuOverlay extends View {
        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private Bitmap logoBitmap;
        private Typeface mojanglesFont;
        private final List<String> splashes = new ArrayList<>();
        private String currentSplash = "Console Edition on Android!";
        private float splashScale = 1.0f;
        private float splashTime = 0.0f;
        private final Random random = new Random();

        // Button boundaries
        private final RectF btnPlay = new RectF();
        private final RectF btnOptions = new RectF();
        private final RectF btnExit = new RectF();
        private final RectF btnBackToMenu = new RectF();

        MenuOverlay() {
            super(MainActivity.this);
            loadAssets();
        }

        private void loadAssets() {
            try {
                InputStream is = getAssets().open("title/mclogo.png");
                logoBitmap = BitmapFactory.decodeStream(is);
                is.close();
            } catch (Exception e) {
                // fallback
            }

            try {
                mojanglesFont = Typeface.createFromAsset(getAssets(), "font/Mojangles.ttf");
            } catch (Exception e) {
                mojanglesFont = Typeface.DEFAULT_BOLD;
            }

            try {
                InputStream is = getAssets().open("splashes.txt");
                java.util.Scanner s = new java.util.Scanner(is, "UTF-8");
                while (s.hasNextLine()) {
                    String line = s.nextLine().trim();
                    if (!line.isEmpty()) splashes.add(line);
                }
                s.close();
                is.close();
                if (!splashes.isEmpty()) {
                    currentSplash = splashes.get(random.nextInt(splashes.size()));
                }
            } catch (Exception e) {
                // default
            }
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            final int action = event.getActionMasked();
            final float x = event.getX();
            final float y = event.getY();

            if (currentScreen == 0) { // Main Menu
                if (action == MotionEvent.ACTION_UP) {
                    if (btnPlay.contains(x, y)) {
                        currentScreen = 1;
                        NativeBridge.setScreen(1);
                        invalidate();
                        return true;
                    } else if (btnOptions.contains(x, y)) {
                        if (!splashes.isEmpty()) {
                            currentSplash = splashes.get(random.nextInt(splashes.size()));
                            invalidate();
                        }
                        return true;
                    } else if (btnExit.contains(x, y)) {
                        finish();
                        return true;
                    }
                }
                return true;
            } else { // In-Game HUD
                if (action == MotionEvent.ACTION_UP && btnBackToMenu.contains(x, y)) {
                    currentScreen = 0;
                    NativeBridge.setScreen(0);
                    invalidate();
                    return true;
                }
                final int index = event.getActionIndex();
                NativeBridge.touch(
                    action,
                    event.getPointerId(index),
                    event.getX(index),
                    event.getY(index)
                );
                return true;
            }
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            final float w = getWidth();
            final float h = getHeight();

            if (currentScreen == 0) {
                drawMainMenu(canvas, w, h);
            } else {
                drawInGameHud(canvas, w, h);
            }
            postInvalidateOnAnimation();
        }

        private void drawMainMenu(Canvas canvas, float w, float h) {
            // Draw Minecraft Logo
            if (logoBitmap != null) {
                float logoAspect = (float) logoBitmap.getWidth() / (float) logoBitmap.getHeight();
                float logoW = Math.min(w * 0.65f, 540f);
                float logoH = logoW / logoAspect;
                float logoX = (w - logoW) * 0.5f;
                float logoY = h * 0.12f;
                Rect src = new Rect(0, 0, logoBitmap.getWidth(), logoBitmap.getHeight());
                RectF dst = new RectF(logoX, logoY, logoX + logoW, logoY + logoH);
                canvas.drawBitmap(logoBitmap, src, dst, paint);

                // Animated Yellow Splash Text
                splashTime += 0.05f;
                splashScale = 1.0f + 0.08f * (float) Math.sin(splashTime * 3.0f);
                canvas.save();
                canvas.translate(logoX + logoW * 0.85f, logoY + logoH * 0.90f);
                canvas.rotate(-20f);
                canvas.scale(splashScale, splashScale);
                paint.setTypeface(mojanglesFont);
                paint.setTextSize(Math.max(22f, h * 0.038f));
                paint.setTextAlign(Paint.Align.CENTER);
                // Shadow
                paint.setColor(0xFF3F3F00);
                canvas.drawText(currentSplash, 2f, 2f, paint);
                // Front
                paint.setColor(0xFFFFFF55);
                canvas.drawText(currentSplash, 0f, 0f, paint);
                canvas.restore();
            }

            // Buttons
            float btnW = Math.min(w * 0.45f, 400f);
            float btnH = Math.max(48f, h * 0.09f);
            float btnX = (w - btnW) * 0.5f;
            float startY = h * 0.48f;
            float gap = btnH * 1.25f;

            btnPlay.set(btnX, startY, btnX + btnW, startY + btnH);
            btnOptions.set(btnX, startY + gap, btnX + btnW, startY + gap + btnH);
            btnExit.set(btnX, startY + gap * 2, btnX + btnW, startY + gap * 2 + btnH);

            drawLceButton(canvas, btnPlay, "Play Game");
            drawLceButton(canvas, btnOptions, "Help & Options");
            drawLceButton(canvas, btnExit, "Exit Game");

            // Footer info
            paint.setTypeface(mojanglesFont);
            paint.setTextSize(Math.max(16f, h * 0.026f));
            paint.setTextAlign(Paint.Align.LEFT);
            paint.setColor(0xFFFFFFFF);
            canvas.drawText("Minecraft Legacy Console Edition (Android 7thGen Port)", 16f, h - 20f, paint);

            paint.setTextAlign(Paint.Align.RIGHT);
            canvas.drawText("4J Studios / Mojang", w - 16f, h - 20f, paint);
        }

        private void drawLceButton(Canvas canvas, RectF rect, String text) {
            // Button Body (Classic 4J / Java stone button border & fill)
            paint.setStyle(Paint.Style.FILL);
            paint.setColor(0xCC202020);
            canvas.drawRoundRect(rect, 4f, 4f, paint);

            // Highlight border
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(3f);
            paint.setColor(0xFF888888);
            canvas.drawRoundRect(rect, 4f, 4f, paint);

            // Top highlight
            paint.setColor(0xFFCCCCCC);
            canvas.drawLine(rect.left + 2, rect.top + 2, rect.right - 2, rect.top + 2, paint);

            // Text
            paint.setStyle(Paint.Style.FILL);
            paint.setTypeface(mojanglesFont);
            paint.setTextSize(rect.height() * 0.45f);
            paint.setTextAlign(Paint.Align.CENTER);
            float textY = rect.centerY() - ((paint.descent() + paint.ascent()) / 2);

            // Shadow
            paint.setColor(0xFF222222);
            canvas.drawText(text, rect.centerX() + 2f, textY + 2f, paint);

            // Text Face
            paint.setColor(0xFFE0E0E0);
            canvas.drawText(text, rect.centerX(), textY, paint);
        }

        private void drawInGameHud(Canvas canvas, float w, float h) {
            float radius = Math.min(w, h) * 0.075f;

            // Touch Joystick Left
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(Math.max(3f, radius * 0.08f));
            paint.setColor(0x99FFFFFF);
            canvas.drawCircle(w * 0.14f, h * 0.78f, radius, paint);
            canvas.drawCircle(w * 0.14f, h * 0.78f, radius * 0.35f, paint);

            // Action Buttons Right
            canvas.drawCircle(w * 0.86f, h * 0.78f, radius, paint);
            paint.setStyle(Paint.Style.FILL);
            paint.setTypeface(mojanglesFont);
            paint.setTextSize(Math.max(22f, h * 0.032f));
            paint.setTextAlign(Paint.Align.CENTER);
            paint.setColor(0xFFFFFFFF);
            canvas.drawText("JUMP", w * 0.79f, h * 0.61f, paint);
            canvas.drawText("USE", w * 0.885f, h * 0.78f, paint);

            // Back to Menu Button at Top
            float menuW = 140f;
            float menuH = 44f;
            btnBackToMenu.set(w * 0.5f - menuW * 0.5f, 16f, w * 0.5f + menuW * 0.5f, 16f + menuH);
            drawLceButton(canvas, btnBackToMenu, "MENU");
        }
    }
}
