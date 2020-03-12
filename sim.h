#define WIDTH 128
#define HEIGHT 128

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define at(x,y) ((y)*WIDTH + (x))

extern float arr_u0x[WIDTH * HEIGHT];
extern float arr_u0y[WIDTH * HEIGHT];
extern float arr_u1x[WIDTH * HEIGHT];
extern float arr_u1y[WIDTH * HEIGHT];
extern float arr_p0[WIDTH * HEIGHT];
extern float arr_p1[WIDTH * HEIGHT];
extern float arr_divergence[WIDTH * HEIGHT];

extern float *p0, *p1;
extern float *u0x, *u0y;
extern float *u1x, *u1y;
extern float *divergence;

extern u64 frame;
extern float step;

extern void init();
extern void simulate();
extern void swap_ptr(float **x, float **y);
extern void advect(float *ux, float *uy,
                   float *src, float *dst,
                   float step);

extern void add_external(float *ux, float *uy, int x, int y, float magnitude_x, float magnitude_y);

extern void pressure_boundary(float *p);
extern void velocity_boundary(float *ux, float *uy);
extern void compute_divergence(float *ux, float *uy, float *div);
extern void jacobi(float *in_p0, float *in_p1, float *b, float alpha, float beta, int iterations);
extern void sub_press_grad(float *ux, float *uy, float *p);


