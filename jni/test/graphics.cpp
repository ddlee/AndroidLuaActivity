#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/native_window.h>

int animating;
EGLDisplay gDisplay;
EGLSurface gSurface;
EGLContext gContext;
int32_t width;
int32_t height;

extern "C"
int init_display(ANativeWindow *window) {
  // initialize OpenGL ES and EGL

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
            EGL_NONE
  };
  EGLint w, h, dummy, format;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(display, 0, 0);

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry(window, 0, 0, format);

  surface = eglCreateWindowSurface(display, config, window, NULL);
  context = eglCreateContext(display, config, NULL, NULL);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    //    LOGW("Unable to eglMakeCurrent");
    return -1;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  gDisplay = display;
  gContext = context;
  gSurface = surface;
  width = w;
  height = h;

  /*
  // Initialize GL state.
  // Set the background color to black ( rgba ).
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  // Enable Smooth Shading, default not really needed.
  glShadeModel(GL10.GL_SMOOTH);
  // Depth buffer setup.
  glClearDepthf(1.0f);
  // Enables depth testing.
  glEnable(GL10.GL_DEPTH_TEST);
  // The type of depth testing to do.
  glDepthFunc(GL10.GL_LEQUAL);
  // Really nice perspective calculations.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,
	    GL_NICEST);
  */

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_DEPTH_TEST);

  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  return 0;
}

extern "C"
void draw_frame() {
  static int n = 0;

  n++;
  if (gDisplay == NULL) {
    // No display.
    return;
  }

  // Just fill the screen with a color.
  float f = (n % 64)/64.0;
  glClearColor(f, f, f, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glColor4f(1.0f, 0.3f, 0.0f, .5f);
  GLfloat vertices[] = {10,0,0, 0,10,0, 0,0,0};
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, vertices);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableClientState(GL_VERTEX_ARRAY);

  eglSwapBuffers(gDisplay, gSurface);
}

extern "C"
void term_display() {
  if (gDisplay != EGL_NO_DISPLAY) {
    eglMakeCurrent(gDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (gContext != EGL_NO_CONTEXT) {
      eglDestroyContext(gDisplay, gContext);
    }
    if (gSurface != EGL_NO_SURFACE) {
      eglDestroySurface(gDisplay, gSurface);
    }
    eglTerminate(gDisplay);
  }
  gDisplay = EGL_NO_DISPLAY;
  gContext = EGL_NO_CONTEXT;
  gSurface = EGL_NO_SURFACE;
}
