import time
import argparse
from cffi import FFI
import time, argparse
from array2gif import write_gif
import random
from scipy import ndimage
import numpy as np

def init(ffi):
  sim = ffi.dlopen("./sim.so");
  sim.init();
  return sim

def get_frame(_frame):
  return (np.frombuffer(_frame, dtype=np.float).reshape(1,128,128)[:,:,:] * 128 + 128)

def run_sim(opt):

    ffi = FFI()

    #C header stuff
    ffi.cdef("""
       int frame;
       void init();
       void simulate();
       float get_pix(int x, int y);
       float* get_p();
       float* get_ux();
       float* get_uy();
    """)

    C = init(ffi)
    _frame_p = ffi.buffer(C.get_p(), 8*128*128)
    _frame_ux = ffi.buffer(C.get_ux(), 8*128*128)
    _frame_uy = ffi.buffer(C.get_uy(), 8*128*128)

    if opt.output:
      imgs = []
      for s in range(opt.steps):
        fr_r = get_frame(_frame_p).astype(np.uint8)
        fr_g = get_frame(_frame_ux).astype(np.uint8)
        fr_b = get_frame(_frame_uy).astype(np.uint8)
        fr3 = fr_b.repeat(3, axis=0)
        imgs.append(fr3)
        print(fr3.shape)
        C.simulate()
        C.simulate()
      return imgs

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-c', '--steps', type=int, default=100, help='number of cycles')
    parser.add_argument('-o', '--output', type=str, default=None, help='output GIF')

    opt = parser.parse_args()
    print(opt)

    frames = run_sim(opt)
    if opt.output: write_gif(frames, opt.output, fps=25)

