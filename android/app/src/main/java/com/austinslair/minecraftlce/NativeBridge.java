package com.austinslair.minecraftlce;

import android.content.res.AssetManager;

public final class NativeBridge {
    static {
        System.loadLibrary("minecraft_android");
    }

    private NativeBridge() {}

    public static native void initPlatform(AssetManager assetManager);
    public static native void touch(int action, int pointerId, float x, float y);
    public static native void surfaceCreated();
    public static native void surfaceChanged(int width, int height);
    public static native void renderFrame();
}
