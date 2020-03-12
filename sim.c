#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "sim.h"

float arr_p0[WIDTH * HEIGHT];
float arr_u0x[WIDTH * HEIGHT];
float arr_u0y[WIDTH * HEIGHT];
float arr_p1[WIDTH * HEIGHT];
float arr_u1x[WIDTH * HEIGHT];
float arr_u1y[WIDTH * HEIGHT];
float arr_divergence[WIDTH * HEIGHT];

float step = 1.0f;
u64 frame;

float lerp(float a, float b, float c) {
  c = c < 0.0f ? 0.0f : (c > 1.0f ? 1.0f : c);
  return a * (1.0f - c) + b * c;
}

float bilerp(float *arr, float x, float y) {

  u16 x0 = floor(x);
  u16 y0 = floor(y);
  u16 x1 = x0+1;
  u16 y1 = y0+1;

  float p00 = arr[at(x0,y0)];
  float p01 = arr[at(x0,y1)];
  float p10 = arr[at(x1,y0)];
  float p11 = arr[at(x1,y1)];

  float interp = lerp(lerp(p00, p10, x-x0), lerp(p01, p11, x-x0), y-y0);

  return interp;
}

void swap_ptr(float **x, float **y){
  float *t0, *t1;
  t0 = *x, t1 = *y;
  *y = t0, *x = t1;
}

void advect(float *ux, float *uy, float *src, float *dst, float t) {
  for (int y=1; y<HEIGHT-1; y++) {
    for (int x=1; x<WIDTH-1; x++)
    {
      float vx = ux[at(x,y)] * t;
      float vy = uy[at(x,y)] * t;
      float vv = bilerp(src, x+vx, y+vy);
      dst[at(x,y)] = vv;
    }
  }
}

void add_external(float *ux, float *uy, int x, int y, float magnitude_x, float magnitude_y) {

  ux[at(x, y)] -= magnitude_x;
  uy[at(x, y)] -= magnitude_y;

}

void pressure_boundary(float *p) {
  for (int x=0; x<WIDTH; x++) {
    p[at(x, 0)] = p[at(x, 1)];
    p[at(x, HEIGHT-1)] = p[at(x, HEIGHT-2)];
  }
  for (int y=0; y<HEIGHT; y++) {
    p[at(0, y)] = p[at(1, y)];
    p[at(WIDTH-1, y)] = p[at(WIDTH-2, y)];
  }
}

void velocity_boundary(float *ux, float *uy) {

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

void compute_divergence(float *ux, float *uy, float *div) {

  float x0, y0, x1, y1;
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

void jacobi(float *p0, float *p1, float *b, float alpha, float beta, int iterations) {

  float x0, x1, y0, y1;

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

  swap_ptr(&p0, &p1);
  pressure_boundary(p0);
}

void sub_press_grad(float *ux, float *uy, float *p) {

  float x0, x1, y0, y1, dx, dy;
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
float *p0, *u0x, *u0y;
float *p1, *u1x, *u1y;
float *divergence;

void init() {
  p0 = arr_p0, u0x = arr_u0x, u0y = arr_u0y;
  p1 = arr_p1, u1x = arr_u1x, u1y = arr_u1y;
  divergence = arr_divergence;
  frame = 0;
  for (int i=0; i<HEIGHT; i++) {
    for (int j=0; j<WIDTH; j++) {
      u0x[at(i,j)] = 0.0f;
      u0y[at(i,j)] = 0.0f;
      u1x[at(i,j)] = 0.0f;
      u1y[at(i,j)] = 0.0f;
      p0[at(i,j)] = 0.5f;
      p1[at(i,j)] = 0.5f;
    }
  }
  printf("init, frame=%llu\n", frame);
}

float *get_ux()  { return u0x; }
float *get_uy()  { return u0y; }
float *get_p()   { return p0; }
float *get_div() { return divergence; }

float get_pix(int x, int y) {
  return p0[at(x,y)];
}

void simulate() {

  velocity_boundary(u0x, u0y);
  advect(u0x, u0y, u0x, u1x, step);
  advect(u0x, u0y, u0y, u1y, step);
  add_external(u1x, u1y, 10, HEIGHT/2, 0.25f, 0);
  compute_divergence(u1x, u1y, divergence);
  jacobi(p0, p1, divergence, -1.0f, 0.25f, 8);
  sub_press_grad(u1x, u1y, p0);

  swap_ptr(&p0,  &p1);
  swap_ptr(&u0x, &u1x);
  swap_ptr(&u0y, &u1y);

  frame++;
  printf("simulate, frame=%llu\n", frame);
}
