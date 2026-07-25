package com.example.vertexarrayobject;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * MainActivity – Vertex Array Object demo.
 *
 * Renders a rotating coloured cube and a spinning ground grid,
 * both using VAOs (Vertex Array Objects). The VAO captures the
 * VBO attribute layout and IBO binding at init time so that each
 * Render() call only needs glBindVertexArray() before drawing.
 * Touch to pause / resume the cube rotation.
 *
 * Ported from the OpenGL ES 3.0 Cookbook (Chapter 3 – Vertex Array Object).
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "VertexArrayObject";

    static { System.loadLibrary("vertexarrayobject"); }

    private GLSurfaceView glSurfaceView;
    private boolean rendererInitialized = false;

    private native boolean nativeInit(AssetManager assetManager);
    private native void    nativeResize(int width, int height);
    private native void    nativeRender();
    private native void    nativeTouchDown(float x, float y);
    private native void    nativeTouchMove(float x, float y);
    private native void    nativeTouchRelease(float x, float y);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.gl_surface_view);
        glSurfaceView.setEGLContextClientVersion(3);

        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Log.d(TAG, "onSurfaceCreated");
                if (nativeInit(getAssets())) {
                    rendererInitialized = true;
                    Log.d(TAG, "Renderer initialized");
                } else {
                    Log.e(TAG, "Renderer init failed");
                }
            }
            @Override
            public void onSurfaceChanged(GL10 gl, int w, int h) {
                if (rendererInitialized) nativeResize(w, h);
            }
            @Override
            public void onDrawFrame(GL10 gl) {
                if (rendererInitialized) nativeRender();
            }
        });

        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        glSurfaceView.setOnTouchListener((v, event) -> {
            if (!rendererInitialized) return false;
            final float x = event.getX(), y = event.getY();
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    glSurfaceView.queueEvent(() -> nativeTouchDown(x, y)); break;
                case MotionEvent.ACTION_MOVE:
                    glSurfaceView.queueEvent(() -> nativeTouchMove(x, y)); break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    glSurfaceView.queueEvent(() -> nativeTouchRelease(x, y)); break;
            }
            return true;
        });
    }

    @Override protected void onPause()  { super.onPause();  glSurfaceView.onPause();  }
    @Override protected void onResume() { super.onResume(); glSurfaceView.onResume(); }
}
