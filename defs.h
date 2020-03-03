#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#endif

#include <GLFW/glfw3.h>

extern int display_init(void);
extern int display_kill(void);

extern u8 full;  /* full screen ? */
extern u8 vsync; /* vsync ? */
extern GLFWwindow *window;
