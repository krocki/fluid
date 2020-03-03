#include "defs.h"

u8 full = 0;
u8 vsync = 1;
int scr_h = 512;
int scr_w = 512;

GLFWwindow *window = NULL;
const GLubyte *renderer;
const GLubyte *version;
int fb_h, fb_w;

int display_init() {

  if (!glfwInit()) {
    fprintf(stderr, "couldn't initialize glfw3\n");
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(
    scr_w, scr_h, "GAME",
    full ? glfwGetPrimaryMonitor() :
    NULL, NULL);

  if (NULL==window) {
    fprintf(stderr,
    "couldn't create glfw window "
    "of size [%u, %u]\n", scr_h, scr_w);
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwGetFramebufferSize(window, &fb_w, &fb_h);

  fb_w = full ? 640 : fb_w;
  fb_h = full ? 480 : fb_h;

  fprintf(stdout, "FB size: %u x %u\n", fb_w, fb_h);
  renderer = glGetString(GL_RENDERER);
  version = glGetString(GL_VERSION);

  fprintf(stdout,
    "GL_RENDERER: %s\n"
    "GL_VERSION: %s\n",
    renderer, version);

  glfwSwapInterval(vsync);

  return 0;
}

int display_kill() {

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;

}
