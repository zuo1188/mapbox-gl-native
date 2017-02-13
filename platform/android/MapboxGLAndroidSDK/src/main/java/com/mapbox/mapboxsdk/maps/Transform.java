package com.mapbox.mapboxsdk.maps;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ObjectAnimator;
import android.animation.TypeEvaluator;
import android.animation.ValueAnimator;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.UiThread;
import android.support.annotation.WorkerThread;
import android.view.animation.AnticipateOvershootInterpolator;
import android.view.animation.BounceInterpolator;
import android.view.animation.LinearInterpolator;

import com.mapbox.mapboxsdk.annotations.MarkerViewManager;
import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.camera.CameraUpdate;
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory;
import com.mapbox.mapboxsdk.constants.MapboxConstants;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.widgets.MyLocationView;

import java.util.concurrent.TimeUnit;

import timber.log.Timber;

/**
 * Resembles the current Map transformation.
 * <p>
 * Responsible for synchronising {@link CameraPosition} state and notifying
 * {@link com.mapbox.mapboxsdk.maps.MapboxMap.OnCameraChangeListener}.
 * </p>
 */
// FIXME use MapState instead of exending MapThreadExecutor
final class Transform extends MapThreadExecutor implements State {

  private final MarkerViewManager markerViewManager;
  private final TrackingSettings trackingSettings;
  private final MyLocationView myLocationView;

  private MapboxMap.CancelableCallback cameraCancelableCallback;
  private MapboxMap.OnCameraChangeListener onCameraChangeListener;

  private final TransformState currentTransformState = new TransformState();
  private final TransformState futureTransformState = new TransformState();

  Transform(NativeMapView mapView, ThreadExecutor threadExecutor, MarkerViewManager markerViewManager,
            TrackingSettings trackingSettings) {
    super(mapView, threadExecutor);
    this.markerViewManager = markerViewManager;
    this.trackingSettings = trackingSettings;
    this.myLocationView = trackingSettings.getMyLocationView();
    currentTransformState.setLatLng(new LatLng(0, 0));
  }

  void initialise(@NonNull MapboxMap mapboxMap, @NonNull MapboxMapOptions options) {
    CameraPosition position = options.getCamera();
    if (position != null && !position.equals(CameraPosition.DEFAULT)) {
      moveCamera(mapboxMap, CameraUpdateFactory.newCameraPosition(position), null);
    }
  }

  //
  // State
  //

  @Override
  public void onSaveInstanceState(Bundle outState) {
    outState.putDouble(MapboxConstants.STATE_CAMERA_LAT, currentTransformState.latLng.getLatitude());
    outState.putDouble(MapboxConstants.STATE_CAMERA_LNG, currentTransformState.latLng.getLongitude());
    outState.putDouble(MapboxConstants.STATE_CAMERA_BEARING, currentTransformState.bearing);
    outState.putDouble(MapboxConstants.STATE_CAMERA_TILT, currentTransformState.tilt);
    outState.putDouble(MapboxConstants.STATE_CAMERA_ZOOM, currentTransformState.zoom);
  }

  @Override
  public void onRestoreInstanceState(Bundle savedInstanceState) {
    double latitude = savedInstanceState.getDouble(MapboxConstants.STATE_CAMERA_LAT, 0);
    double longitude = savedInstanceState.getDouble(MapboxConstants.STATE_CAMERA_LNG, 0);
    //currentTransformState = new TransformState(new LatLng(latitude, longitude),
    //savedInstanceState.getDouble(MapboxConstants.STATE_CAMERA_BEARING, 0),
    //savedInstanceState.getDouble(MapboxConstants.STATE_CAMERA_TILT, 0),
    //savedInstanceState.getDouble(MapboxConstants.STATE_CAMERA_ZOOM, 0));
    // FIXME: 10/02/2017 ACTUALLY RESTORE STATE
  }

  //
  // Camera API
  //

  @UiThread
  public final CameraPosition getCameraPosition() {
    return new CameraPosition.Builder(
      currentTransformState.getLatLng(),
      currentTransformState.getZoom(),
      currentTransformState.getTilt(),
      currentTransformState.getBearing()
    ).build();
  }

  void setCameraPosition(CameraPosition cameraPostion) {
    currentTransformState.setZoom(cameraPostion.zoom);
    currentTransformState.setLatLng(cameraPostion.target);
    currentTransformState.setTilt(cameraPostion.tilt);
    currentTransformState.setBearing(cameraPostion.bearing);
  }

  @UiThread
  void updateCameraPosition(@NonNull CameraPosition position) {
    if (myLocationView != null) {
      myLocationView.setCameraPosition(position);
    }
    markerViewManager.setTilt((float) position.tilt);
  }

  @UiThread
  final void moveCamera(final MapboxMap mapboxMap, final CameraUpdate update,
                        final MapboxMap.CancelableCallback callback) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        CameraPosition cameraPosition = update.getCameraPosition(mapboxMap);
        setCameraPosition(cameraPosition);
        cancelTransitions(nativeMapView);
        nativeMapView.jumpTo(cameraPosition.bearing, cameraPosition.target, cameraPosition.tilt, cameraPosition.zoom);
        trackingSettings.resetTrackingModesIfRequired(cameraPosition);
        if (callback != null) {
          callback.onFinish();
        }
      }
    });

  }

  @UiThread
  final void easeCamera(final MapboxMap mapboxMap, final CameraUpdate update, final int durationMs,
                        final boolean easingInterpolator, final MapboxMap.CancelableCallback callback) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        CameraPosition position = update.getCameraPosition(mapboxMap);
        trackingSettings.resetTrackingModesIfRequired(position);

        cancelTransitions(nativeMapView);
        if (callback != null) {
          cameraCancelableCallback = callback;
          // nativeMapView.addOnMapChangedListener(Transform.this);
        }

        nativeMapView.easeTo(position.bearing, position.target, getDurationNano(durationMs), position.tilt,
          position.zoom, easingInterpolator);
      }
    });
  }

  @UiThread
  final void animateCamera(final MapboxMap mapboxMap, final CameraUpdate update, final int durationMs,
                           final MapboxMap.CancelableCallback callback) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        CameraPosition position = update.getCameraPosition(mapboxMap);
        trackingSettings.resetTrackingModesIfRequired(position);

        cancelTransitions(nativeMapView);
        if (callback != null) {
          cameraCancelableCallback = callback;
          // nativeMapView.addOnMapChangedListener(Transform.this);
        }

        nativeMapView.flyTo(position.bearing, position.target, getDurationNano(durationMs), position.tilt,
          position.zoom);
      }
    });
  }

  @UiThread
  void invalidateCameraPosition() {
//    queueRenderEvent(new MapRunnable() {
//      @Override
//      public void execute(@NonNull NativeMapView nativeMapView) {
//        final CameraPosition cameraPosition = new CameraPosition.Builder(nativeMapView.getCameraValues()).build();
//        queueUiEvent(new Runnable() {
//          @Override
//          public void run() {
//            updateCameraPosition(cameraPosition);
//            if (onCameraChangeListener != null) {
//              // post camera change event on ui Thread
//              onCameraChangeListener.onCameraChange(cameraPosition);
//            }
//          }
//        });
//      }
//    });
  }

  @UiThread
  void cancelTransitions() {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        cancelTransitions(nativeMapView);
      }
    });
  }

  @WorkerThread
  void cancelTransitions(NativeMapView nativeMapView) {
    if (cameraCancelableCallback != null) {
      queueUiEvent(new Runnable() {
        @Override
        public void run() {
          cameraCancelableCallback.onCancel();
          cameraCancelableCallback = null;
        }
      });
    }
    nativeMapView.cancelTransitions();
  }

  @UiThread
  void resetNorth() {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        cancelTransitions(nativeMapView);
        nativeMapView.resetNorth();
      }
    });
  }

  @UiThread
  void setOnCameraChangeListener(@Nullable MapboxMap.OnCameraChangeListener listener) {
    this.onCameraChangeListener = listener;
  }

  private long getDurationNano(long durationMs) {
    return durationMs > 0 ? TimeUnit.NANOSECONDS.convert(durationMs, TimeUnit.MILLISECONDS) : 0;
  }

  @UiThread
  double getZoom() {
    return currentTransformState.getZoom();
  }

  @UiThread
  void zoom(boolean zoomIn) {
    currentTransformState.setZoom(zoomIn ? currentTransformState.getZoom() + 1 : currentTransformState.getZoom() - 1);
    zoom(zoomIn, -1.0f, -1.0f);
  }

  @WorkerThread
  private synchronized void updateTransformationState(NativeMapView nativeMapView, TransformState transformState) {
    currentTransformState.latLng = transformState.latLng;
    currentTransformState.zoom = transformState.zoom;
    currentTransformState.bearing = transformState.bearing;
    currentTransformState.tilt = transformState.tilt;

    nativeMapView.setLatLng(currentTransformState.latLng);
    nativeMapView.setZoom(currentTransformState.zoom);
    nativeMapView.setPitch(currentTransformState.tilt);
    nativeMapView.setBearing(currentTransformState.bearing);

//    nativeMapView.easeTo(currentTransformState.bearing,
//      currentTransformState.latLng,
//      0,
//      currentTransformState.tilt,
//      currentTransformState.zoom,
//      true);

    Timber.e("STATE ANIMATING: %s", currentTransformState);
  }

  @UiThread
  void zoom(final boolean zoomIn, final float x, final float y) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull final NativeMapView nativeMapView) {
        // calculate new state
        final LatLng latLng = nativeMapView.fromScreenLocation(x, y);
        queueUiEvent(new Runnable() {
          @Override
          public void run() {
            futureTransformState.setLatLng(latLng);
            futureTransformState.setZoom(zoomIn ? currentTransformState.zoom + 1 : currentTransformState.zoom - 1);
            futureTransformState.setTilt(currentTransformState.tilt);
            futureTransformState.setBearing(currentTransformState.bearing);

            Timber.e("STATE: Now  %s", currentTransformState);
            Timber.e("STATE: Future %s", futureTransformState);
            final ValueAnimator stateAnimator = ObjectAnimator.ofObject(this, "currentTransformState",
              new TransformStateEvaluator(), new TransformState(currentTransformState), futureTransformState);
            stateAnimator.setDuration(MapboxConstants.ANIMATION_DURATION);
            stateAnimator.setInterpolator(new AnticipateOvershootInterpolator());
            stateAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
              @Override
              public void onAnimationUpdate(ValueAnimator valueAnimator) {
                final TransformState transformState = (TransformState) valueAnimator.getAnimatedValue();
                Timber.e(transformState.toString());
                queueRenderEvent(new MapRunnable() {
                  @Override
                  public void execute(@NonNull NativeMapView nativeMapView) {
                    updateTransformationState(nativeMapView, transformState);
                  }
                });
              }
            });
            stateAnimator.addListener(new AnimatorListenerAdapter() {
              @Override
              public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                Timber.d("OnAnimationEnd");
                queueRenderEvent(new MapRunnable() {
                  @Override
                  public void execute(@NonNull NativeMapView nativeMapView) {
                    nativeMapView.setGestureInProgress(false);
                  }
                });
              }
            });
            stateAnimator.start();
          }
        });

        nativeMapView.setGestureInProgress(true);
      }
    });
  }

  @UiThread
  void setZoom(final double zoom) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setZoom(zoom);
      }
    });
  }

  @UiThread
  double getBearing() {
    return currentTransformState.getBearing();
  }

  @UiThread
  void setBearing(final double bearing) {
    if (myLocationView != null) {
      myLocationView.setBearing(bearing);
    }
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setBearing(bearing);
      }
    });
  }

  @UiThread
  void setBearing(final double bearing, final float focalX, final float focalY) {
    currentTransformState.setBearing(bearing);
    if (myLocationView != null) {
      myLocationView.setBearing(bearing);
    }
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setBearing(bearing, focalX, focalY);
      }
    });
  }

  //
  // Pitch / Tilt
  //

  @UiThread
  double getTilt() {
    return currentTransformState.getBearing();
  }

  @UiThread
  void setTilt(final Double pitch) {
    currentTransformState.setTilt(pitch);
    if (myLocationView != null) {
      myLocationView.setTilt(pitch);
    }
    markerViewManager.setTilt(pitch.floatValue());
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setPitch(pitch, 0);
      }
    });
  }

  //
  // Center coordinate
  //

  void setGestureInProgress(final boolean gestureInProgress) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setGestureInProgress(gestureInProgress);
      }
    });
    if (!gestureInProgress) {
      invalidateCameraPosition();
    }
  }

  void zoomBy(final double pow, final float x, final float y) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.scaleBy(pow, x, y);
      }
    });
  }

  void moveBy(final double offsetX, final double offsetY, final long duration) {
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.moveBy(offsetX, offsetY, duration);
      }
    });
  }

  //
  // Min & Max ZoomLevel
  //

  private double minZoom;
  private double maxZoom;

  void setMinZoom(final double minZoom) {
    if ((minZoom < MapboxConstants.MINIMUM_ZOOM) || (minZoom > MapboxConstants.MAXIMUM_ZOOM)) {
      Timber.e("Not setting minZoomPreference, value is in unsupported range: " + minZoom);
      return;
    }
    this.minZoom = minZoom;
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setMinZoom(minZoom);
      }
    });
  }

  double getMinZoom() {
    return minZoom;
  }

  void setMaxZoom(final double maxZoom) {
    if ((maxZoom < MapboxConstants.MINIMUM_ZOOM) || (maxZoom > MapboxConstants.MAXIMUM_ZOOM)) {
      Timber.e("Not setting maxZoomPreference, value is in unsupported range: " + maxZoom);
      return;
    }

    this.maxZoom = maxZoom;
    queueRenderEvent(new MapRunnable() {
      @Override
      public void execute(@NonNull NativeMapView nativeMapView) {
        nativeMapView.setMaxZoom(maxZoom);
      }
    });
  }

  double getMaxZoom() {
    return maxZoom;
  }


  private class TransformState {

    private LatLng latLng;
    private double zoom;
    private double tilt;
    private double bearing;

    TransformState() {
      latLng = new LatLng();
    }

    TransformState(TransformState transformState) {
      latLng = transformState.getLatLng();
      zoom = transformState.getZoom();
      tilt = transformState.getTilt();
      bearing = transformState.getBearing();
    }

    public LatLng getLatLng() {
      return latLng;
    }

    public void setLatLng(LatLng latLng) {
      this.latLng = latLng;
    }

    public double getZoom() {
      return zoom;
    }

    public void setZoom(double zoom) {
      this.zoom = zoom;
    }

    public double getTilt() {
      return tilt;
    }

    public void setTilt(double tilt) {
      this.tilt = tilt;
    }

    public double getBearing() {
      return bearing;
    }

    public void setBearing(double bearing) {
      this.bearing = bearing;
    }

    /**
     * Returns a String with the camera target, zoom, bearing and tilt.
     *
     * @return A String with CameraPosition information.
     */
    @Override
    public String toString() {
      return "Target: " + latLng + ", Zoom:" + zoom + ", Bearing:" + bearing + ", Tilt:" + tilt;
    }

  }

  private class TransformStateEvaluator implements TypeEvaluator<TransformState> {

    private final TransformState transformState = new TransformState();

    @Override
    public TransformState evaluate(float fraction, TransformState startValue, TransformState endValue) {
      transformState.latLng.setLatitude(startValue.latLng.getLatitude()
        + (endValue.latLng.getLatitude() - startValue.latLng.getLatitude()) * fraction);
      transformState.latLng.setLongitude(startValue.latLng.getLongitude()
        + (endValue.latLng.getLongitude() - startValue.latLng.getLongitude()) * fraction);
      transformState.zoom = startValue.zoom + (endValue.zoom - startValue.zoom) * fraction;
      return transformState;
    }
  }


}
