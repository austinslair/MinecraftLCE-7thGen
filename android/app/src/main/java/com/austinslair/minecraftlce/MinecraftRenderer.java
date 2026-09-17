package com.austinslair.minecraftlce;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

final class MinecraftRenderer implements GLSurfaceView.Renderer {
    @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        NativeBridge.surfaceCreated();
    }

    @Override public void onSurfaceChanged(GL10 gl, int width, int height) {
        NativeBridge.surfaceChanged(width, height);
    }

    @Override public void onDrawFrame(GL10 gl) {
        NativeBridge.renderFrame();
    }
}
