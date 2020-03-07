#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 128
#define HEIGHT 128

#define at(x,y) ((y)*WIDTH + (x))

double u0x[WIDTH * HEIGHT];
double u0y[WIDTH * HEIGHT];
double u1x[WIDTH * HEIGHT];
double u1y[WIDTH * HEIGHT];
double p0[WIDTH * HEIGHT];
double p1[WIDTH * HEIGHT];
double divergence[WIDTH * HEIGHT];
int frame;
double step = 1.f;

double lerp(double a, double b, double c) {
  c = c < 0.0f ? 0.0f : (c > 1.0f ? 1.0f : c);
  return a * (1.f - c) + b * c;
}

double bilerp(double *arr, double x, double y) {

  int x0 = floor(x);
  int y0 = floor(y);
  int x1 = x0+1;
  int y1 = y0+1;

  double p00 = arr[at(x0,y0)];
  double p01 = arr[at(x0,y1)];
  double p10 = arr[at(x1,y0)];
  double p11 = arr[at(x1,y1)];

  double interp = lerp(lerp(p00, p10, x-x0), lerp(p01, p11, x-x0), y-y0);

  return interp;
}

void copy(double *dst, double *src) {
  for (int y=1; y<HEIGHT-1; y++) {
    for (int x=1; x<WIDTH-1; x++)
    {
      dst[at(x,y)] = src[at(x,y)];
    }
  }

}

void advect(double *ux, double *uy, double *src, double *dst, double t) {
  for (int y=1; y<HEIGHT-1; y++) {
    for (int x=1; x<WIDTH-1; x++)
    {
      double vx = ux[y*WIDTH + x] * t;
      double vy = uy[y*WIDTH + x] * t;
      double vv = bilerp(src, x+vx, y+vy);
      dst[y*WIDTH + x] = vv;
    }
  }
}

void add_external(double *ux, double *uy) {

  //double dx = mx - last_x;
  //double dy = my - last_y;
  //int xpos, ypos;
  //xpos = (int)(mx * WIDTH);
  //ypos = (int)(my * HEIGHT);
  //ux[at(xpos, ypos)] -= 100.0f * dx * 2.0f;
  //uy[at(xpos, ypos)] -= 100.0f * dy * 2.0f;
  ux[at(10, HEIGHT/2)] -= 0.5f * 2.0f;

}

void pressure_boundary(double *p) {
  for (int x=0; x<WIDTH; x++) {
    p[at(x, 0)] = p[at(x, 1)];
    p[at(x, HEIGHT-1)] = p[at(x, HEIGHT-2)];
  }
  for (int y=0; y<HEIGHT; y++) {
    p[at(0, y)] = p[at(1, y)];
    p[at(WIDTH-1, y)] = p[at(WIDTH-2, y)];
  }
}

void velocity_boundary(double *ux, double *uy) {

  for (int x=0; x<WIDTH; x++) {
    ux[at(x, 0)] = -ux[at(x, 1)];
    uy[at(x, 0)] = -uy[at(x, 1)];
    ux[at(x, HEIGHT-1)] = -ux[at(x, HEIGHT-2)];
    uy[at(x, HEIGHT-1)] = -uy[at(x, HEIGHT-2)];
  }

  for (int y=0; y<HEIGHT; y++) {
    ux[at(0, y)] = -ux[at(1, y)];
    uy[at(0, y)] = -uy[at(1, y)];
    ux[at(WIDTH-1, y)] = -ux[at(WIDTH-2, y)];
    uy[at(WIDTH-1, y)] = -uy[at(WIDTH-2, y)];
  }
}

void compute_divergence(double *ux, double *uy, double *div) {

  double x0, y0, x1, y1;
  for (int y=1; y<HEIGHT-1; y++) {
    for (int x=1; x<WIDTH-1; x++) {
      x0 = ux[at(x-1, y)];
      x1 = ux[at(x+1, y)];
      y0 = uy[at(x, y-1)];
      y1 = uy[at(x, y+1)];
      div[at(x, y)] = (x1-x0 + y1-y0)*0.5f;
    }
  }

}

void jacobi(double *p0, double *p1, double *b, double alpha, double beta, int iterations) {

  double x0, x1, y0, y1;
  //for (int y=1; y<HEIGHT-1; y++) {
  //  for (int x=1; x<WIDTH-1; x++) {
  //    p0[at(x,y)] = 0.5f;
  //    p1[at(x,y)] = 0.5f;
  //  }
  //}

  for (int i=0; i<iterations; i++) {
    for (int y=1; y<HEIGHT-1; y++) {
      for (int x=1; x<WIDTH-1; x++) {
        x0 = p0[at(x-1, y)];
        x1 = p0[at(x+1, y)];
        y0 = p0[at(x, y-1)];
        y1 = p0[at(x, y+1)];
        p1[at(x, y)] = (x0 + x1 + y0 + y1 + alpha * b[at(x, y)]) * beta;
      }
    }
  }

  copy(p0, p1);
  pressure_boundary(p0);
}

void sub_press_grad(double *ux, double *uy, double *p) {

  double x0, x1, y0, y1, dx, dy;
  for (int y=1; y<HEIGHT-1; y++) {
    for (int x=1; x<WIDTH-1; x++)
    {
      x0 = p[at(x-1, y)];
      x1 = p[at(x+1, y)];
      y0 = p[at(x, y-1)];
      y1 = p[at(x, y+1)];
      dx = (x1-x0)/2.0f;
      dy = (y1-y0)/2.0f;
      ux[at(x, y)] -= dx;
      uy[at(x, y)] -= dy;
    }
  }
}

/* generate texture buffers */
/* test pattern */
void init() {
  frame = 0;
  for (int i=0; i<HEIGHT; i++) {
    for (int j=0; j<WIDTH; j++) {
      u0x[i*WIDTH+j] = 0.0f;
      u0y[i*WIDTH+j] = 0.0f;
      u1x[i*WIDTH+j] = 0.0f;
      u1y[i*WIDTH+j] = 0.0f;
      p0[i*WIDTH+j] = rand() / (RAND_MAX + 1.0f);
      p1[i*WIDTH+j] = rand() / (RAND_MAX + 1.0f);
    }
  }
  printf("init, frame=%d\n", frame);
}

double *get_ux() { return u0x; }
double *get_uy() { return u0y; }
double *get_p() { return p0; }

double get_pix(int x, int y) {
  return p0[at(x,y)];
}

void simulate() {

  velocity_boundary(u0x, u0y);
  advect(u0x, u0y, u0x, u1x, step);
  advect(u0x, u0y, u0y, u1y, step);
  add_external(u1x, u1y);
  compute_divergence(u1x, u1y, divergence);
  jacobi(p0, p1, divergence, -1.0f, 0.25f, 8);
  sub_press_grad(u1x, u1y, p0);

  copy(p0, p1);
  copy(u0x, u1x);
  copy(u0y, u1y);

  frame++;
  printf("simulate, frame=%d\n", frame);
}
