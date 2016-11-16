package com.mapbox.mapboxsdk.maps;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;

import com.mapbox.mapboxsdk.constants.Style;
import com.mapbox.mapboxsdk.geometry.LatLng;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static com.mapbox.mapboxsdk.constants.MapboxConstants.TAG;

public class MapSurfaceView extends GLSurfaceView {

    private MapRenderer renderer;
    private NativeMapView nativeMapView;

    public MapSurfaceView(Context context) {
        super(context);
        init(context, null);
    }

    public MapSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs);
    }

    private void init(Context context, AttributeSet attrs) {
        setDebugFlags(DEBUG_CHECK_GL_ERROR | DEBUG_LOG_GL_CALLS);
        setEGLContextClientVersion(2);
        nativeMapView = new NativeMapView(this);
        nativeMapView.setAccessToken("pk.eyJ1IjoidG9icnVuIiwiYSI6ImNpajVlajR0cjAwNjN2NmtyY204eWw2eG0ifQ.x3_WEoExNW5Qyv9T3Vj7Mw");
        nativeMapView.setStyleUrl(Style.LIGHT);
        renderer = new MapRenderer();
        setRenderer(renderer);
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                nativeMapView.flyTo(90, new LatLng(22, 22), 3000, 2, 11);
            }
        }, 3000);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
    }

    public void onInvalidate() {
        throw new RuntimeException("OnInvalidate");
    }

    public void onMapChanged(int rawChange) {
        throw new RuntimeException("OnMapChanged " + rawChange);
    }

    public void onFpsChanged(double fps) {
        throw new RuntimeException("OnFpsChanged " + fps);
    }

    public void onSnapshotReady(byte[] bytes) {
        throw new RuntimeException("OnSnapshotReady");
    }

    private class MapRenderer implements GLSurfaceView.Renderer {
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Log.d(TAG, "onSurfaceCreated");
            nativeMapView.createSurface(getHolder().getSurface());
        }

        public void onSurfaceChanged(GL10 gl, final int w, final int h) {
            Log.d(TAG, "onSurfaceChanged " + w + " " + h);
            gl.glViewport(0, 0, w, h);
            nativeMapView.resizeView(w, h);
            nativeMapView.resizeFramebuffer(w, h);
            nativeMapView.update();
        }

        public void onDrawFrame(GL10 gl) {
            Log.d(TAG, "onDrawFrame render");
            nativeMapView.render();
        }
    }
}
