# opengl-stereo
Some test code for rendering side-by-side stereo pairs a-la VR headsets

The idea here is to get to a basic stereo side-by-side OpenGL renderer for a VR headset, suitable for playing around and learning about how VR works.

There are two make targets:

    make x11_glut

Which builds a binary to run under X11 in a window using GLUT to initialize an OpenGL context, and:

    make rpi_egl

Which builds a binary to run on a raspberry pi (tested on raspberry pi 3 raspian jessie) running directly on the console using EGL.
