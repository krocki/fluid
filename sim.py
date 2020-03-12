import time
import argparse
from cffi import FFI
import time, argparse
from array2gif import write_gif
import random
from scipy import ndimage
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

def init(ffi):
  sim = ffi.dlopen("./sim.so");
  sim.init();
  return sim

def get_frame_f(_frame):
  return np.frombuffer(_frame, dtype=np.float32).reshape(1,128,128)[:,:,:]

def get_frame(_frame):
  return (np.frombuffer(_frame, dtype=np.float32).reshape(1,128,128)[:,:,:] * 64 + 64)

def frame_rgb(_frame):
  rgb = np.zeros((128,128,3)).astype(np.uint8)
  v = get_frame_f(_frame)
  m0, m1 = np.min(v[:]), np.max(v[:])
  r = m1 - m0
  v = ((v - m0) / r * 255)
  rgb[:,:,0] = v.astype(np.uint8)
  rgb[:,:,1] = v.astype(np.uint8)
  rgb[:,:,2] = v.astype(np.uint8)
  return rgb

def run_sim(opt):

    ffi = FFI()

    #C header stuff
    ffi.cdef("""
       unsigned frame;
       void init();
       void simulate();
       float get_pix(int x, int y);
       float* get_p();
       float* get_ux();
       float* get_uy();
       float* get_div();
    """)

    C = init(ffi)
    _frame_p = ffi.buffer(C.get_p(),   4*128*128)
    _frame_ux = ffi.buffer(C.get_ux(), 4*128*128)
    _frame_uy = ffi.buffer(C.get_uy(), 4*128*128)
    _frame_div = ffi.buffer(C.get_div(), 4*128*128)

    if opt.output:
      imgs = []
      for s in range(opt.steps):
        fr_r = get_frame(_frame_p).astype(np.uint8)
        fr_g = get_frame(_frame_ux).astype(np.uint8)
        fr_b = get_frame(_frame_uy).astype(np.uint8)
        fr3 = fr_b.repeat(3, axis=0)
        imgs.append(fr3)
        C.simulate()
        C.simulate()
      return imgs
    else:
      imgs = []
      rgb = np.zeros((2*128,2*128,3)).astype(np.uint8)
      #plt.figure(figsize=(2., 2.))
      #f, (ax1, ax2, ax3, ax4) = plt.subplots(1, 4)
      #im1 = ax1.imshow(rgb); im2 = ax2.imshow(rgb)
      #im3 = ax3.imshow(rgb); im4 = ax4.imshow(rgb)
      im = plt.imshow(rgb)
      for s in range(opt.steps):
        rgb[:128, :128, :] = frame_rgb(_frame_ux)
        rgb[:128, 128:, :] = frame_rgb(_frame_uy)
        rgb[128:, :128, :] = frame_rgb(_frame_p)
        rgb[128:, 128:, :] = frame_rgb(_frame_div)
        imgs.append(rgb)
        im.set_array(rgb)
        plt.pause(0.0000001)
        C.simulate()
      plt.ioff()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-c', '--steps', type=int, default=10000, help='number of cycles')
    parser.add_argument('-o', '--output', type=str, default=None, help='output GIF')

    opt = parser.parse_args()
    print(opt)

    frames = run_sim(opt)
    write_gif(frames, opt.output, fps=25)
    write_gif(frames, opt.output, fps=25)

