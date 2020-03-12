#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sim.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#endif

#include <GLFW/glfw3.h>

#define NUM_BUFFERS 4

GLuint tex[NUM_BUFFERS];

#define bind_key_toggle(x,y) \
{ if (action == GLFW_PRESS && key == (x)) (y) = (y); if (action == GLFW_RELEASE && key == (x)) { (y) = 1-(y); printf(#x ", " #y "=%u\n", (y));} }

int mode = 0;
int tex_no = 0;
int drag_active = 0;
float mx = -1.f;
float my = -1.f;
float last_x = -1.f;
float last_y = -1.f;
float scale = 1.0f;

void texupdate(float *u0x, float *u0y, float *p0, float *dv) {

  glBindTexture(GL_TEXTURE_2D, tex[0]);
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, u0x );
  glBindTexture(GL_TEXTURE_2D, tex[1]);
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, u0y );
  glBindTexture(GL_TEXTURE_2D, tex[2]);
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, p0 );
  glBindTexture(GL_TEXTURE_2D, tex[3]);
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, dv );

}

void mouse_btn_callback(GLFWwindow* window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    drag_active = 1;
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    drag_active = 0;
  }

}

void mouse_pos_callback(GLFWwindow* window, double x, double y) {

  int xpos, ypos;
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  last_x = mx;
  last_y = my;
  mx = x/(float)width;
  my = y/(float)height;
  xpos = (int)(mx * WIDTH);
  ypos = (int)(my * HEIGHT);
  //printf("%f %f, %f %f, data[%d][%d] = %f\n", x, y, mx, my, ypos, xpos, u0x[xpos + ypos*WIDTH]);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  bind_key_toggle(GLFW_KEY_M,     mode);
  if (action==GLFW_PRESS &&
    key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  else if (action==GLFW_RELEASE) {
    switch(key) {
      //case GLFW_KEY_0:
      //  diag();
      //  texupdate(); break;
      //case GLFW_KEY_1:
      //  r();
      //  texupdate(); break;
      //case GLFW_KEY_T:
      //  tex_no = (tex_no + 1) % NUM_BUFFERS;
      //  printf("tex_no = %d\n", tex_no);
        //break;
      default:
      break;
    }
  }
}

const char *vs_name = "tex_fluid_vs.glsl";
const char *fs_name = "tex_fluid_fs.glsl";
//const char *vs_name = "tex_vs.glsl";
//const char *fs_name = "tex_fs.glsl";

char *load_src(const char *file) {
  FILE *f = fopen(file, "r");
  if (!f) {
    fprintf(stderr,
      "couldn't open %s\n", file);
    return NULL;
  }
  fseek(f, 0L, SEEK_END);
  int len = ftell(f);
  rewind(f);

  char *src = malloc(len+1);
  size_t cnt = fread(src, len, 1, f);
  if (0==cnt)
    fprintf(stderr, "fread failed\n");
  fclose(f);
  src[len] = '\0';
  return src;
}

void check_err(const char *m, GLuint *s) {
  GLint res = GL_FALSE;
  int log_len;
  glGetShaderiv(*s, GL_COMPILE_STATUS, &res);
  glGetShaderiv(*s, GL_INFO_LOG_LENGTH, &log_len);
  if (log_len > 0) {
    char *message = malloc(log_len+1);
    glGetShaderInfoLog(*s, log_len, NULL, message);
    printf("%s: %s\n", m, message);
    free(message);
  }
}

void load_shaders(GLuint *v, const char *vf,
                  GLuint *f, const char *ff) {
  char *v_src = load_src(vf);
  char *f_src = load_src(ff);
  *v = glCreateShader(GL_VERTEX_SHADER);
  *f = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(*v, 1, (const char*const*)&v_src, NULL);
  glShaderSource(*f, 1, (const char*const*)&f_src, NULL);
  glCompileShader(*v);
  glCompileShader(*f);
  free(v_src);
  free(f_src);

  /* check */
  check_err("vertex shader", v);
  check_err("fragment shader", f);
}

int main(int argc, char **argv) {

  GLuint width = argc > 1 ?
    strtol(argv[1], NULL, 10) : 512;
  GLuint height = argc > 2 ?
    strtol(argv[2], NULL, 10) : 512;

  GLFWwindow *window = NULL;
  const GLubyte *renderer;
  const GLubyte *version;
  GLuint vbo, tex_vbo, vao;
  GLuint vert_shader, frag_shader;
  GLuint shader_prog;

  float size = scale;

  GLfloat vertices[] = {
   -size,  size,
    size,  size,
    size, -size,
    size, -size,
   -size, -size,
   -size,  size
  };

  GLfloat texcoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
  };

  if (!glfwInit()) {
    fprintf(stderr,
      "couldn't initialize glfw3\n");
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE,
  GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(
    width, height, "GLSL test", NULL, NULL);

  if (!window) {
    fprintf(stderr,
      "couldn't initialize GLFWwindow\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  renderer = glGetString(GL_RENDERER);
  version = glGetString(GL_VERSION);

  fprintf(stdout,
    "GL_RENDERER: %s\n"
    "GL_VERSION: %s\n",
    renderer, version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  /* allocate gpu's memory for vertices */
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
    sizeof(vertices), vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &tex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
  glBufferData(GL_ARRAY_BUFFER,
    sizeof(texcoords), texcoords, GL_STATIC_DRAW);

  /* use the vbo and use 2 float per 'variable' */
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  load_shaders(&vert_shader, vs_name,
               &frag_shader, fs_name);

  shader_prog = glCreateProgram();
  glAttachShader(shader_prog, frag_shader);
  glAttachShader(shader_prog, vert_shader);
  glLinkProgram(shader_prog);

  /* generate texture buffers */
  /* test pattern */
  init();

  glGenTextures( NUM_BUFFERS, tex );
  glActiveTexture(GL_TEXTURE0);
  for (int i=0; i<NUM_BUFFERS; i++) {
    glBindTexture(GL_TEXTURE_2D, tex[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  GLuint cursor     = glGetUniformLocation(shader_prog, "cursor");
  GLuint colormap_a = glGetUniformLocation(shader_prog, "colormap_a");
  GLuint colormap_b = glGetUniformLocation(shader_prog, "colormap_b");
  GLuint colormap_c = glGetUniformLocation(shader_prog, "colormap_c");
  GLuint colormap_d = glGetUniformLocation(shader_prog, "colormap_d");

  float a[2][3] = { {0.5f, 0.5f, 0.5f }, {0.5f, 0.5f, 0.5f } };
  float b[2][3] = { {0.5f, 0.5f, 0.5f }, {0.5f, 0.5f, 0.5f } };
  float c[2][3] = { {1.0f, 1.0f, 1.0f }, {2.0f, 1.0f, 0.0f } };
  float d[2][3] = { {0.0f, 0.33, 0.67 }, {0.5f, 0.2f, 0.25f } };

  glfwSetCursorPosCallback(window, mouse_pos_callback);
  glfwSetMouseButtonCallback(window, mouse_btn_callback);
  glfwSetKeyCallback(window, key_callback);

  texupdate(arr_u0x, arr_u0y, arr_p0, arr_divergence);

  /* main loop */
  int vsync = 0;
  glfwSwapInterval(vsync);

  tex_no=0;

  GLuint tex_ux = glGetUniformLocation(shader_prog, "tex_ux");
  GLuint tex_uy = glGetUniformLocation(shader_prog, "tex_uy");
  GLuint tex_p = glGetUniformLocation(shader_prog, "tex_p");
  GLuint tex_dv = glGetUniformLocation(shader_prog, "tex_dv");

  while (!glfwWindowShouldClose(window)) {

    simulate();
    texupdate(u0x, u0y, p0, arr_divergence);

    /* clear */
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_prog);
    glBindVertexArray(vao);
    glUniform1i(tex_ux, 0);
    glUniform1i(tex_uy, 1);
    glUniform1i(tex_p, 2);
    glUniform1i(tex_dv, 3);

    for (int k=0; k<4; k++) {
     glActiveTexture(GL_TEXTURE0 + k);
     glBindTexture(GL_TEXTURE_2D + k, tex[k]);
    }

    glUniform2f(cursor,     mx, my);
    glUniform3f(colormap_a, a[mode][0], a[mode][1], a[mode][2]);
    glUniform3f(colormap_b, b[mode][0], b[mode][1], b[mode][2]);
    glUniform3f(colormap_c, c[mode][0], c[mode][1], c[mode][2]);
    glUniform3f(colormap_d, d[mode][0], d[mode][1], d[mode][2]);

    /* draw */
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwPollEvents();
    if (GLFW_PRESS == glfwGetKey(
      window, GLFW_KEY_ESCAPE))
      glfwSetWindowShouldClose(window, 1);
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &tex_vbo);
  glDeleteTextures(NUM_BUFFERS, tex);
  return 0;
}
