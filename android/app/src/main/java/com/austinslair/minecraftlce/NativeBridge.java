package com.austinslair.minecraftlce;

public final class NativeBridge {
    private NativeBridge() {}

    public static native void touch(int action, int pointerId, float x, float y);
    public static native void surfaceCreated();
    public static native void surfaceChanged(int width, int height);
    public static native void renderFrame();
}
