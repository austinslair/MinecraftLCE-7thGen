package com.austinslair.minecraftlce;

import android.content.res.AssetManager;

public final class NativeBridge {
    static {
        System.loadLibrary("minecraft_android");
    }

    private NativeBridge() {}

    public static native void initAssets(AssetManager assetManager);
    public static native void touch(int action, int pointerId, float x, float y);
    public static native void surfaceCreated();
    public static native void surfaceChanged(int width, int height);
    public static native void renderFrame();
    public static native void setScreen(int screenId); // 0 = Main Menu, 1 = Play/World
}
