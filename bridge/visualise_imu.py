"""visualise_imu.py - Live 3D quaternion visualiser.

Reads quaternion lines from the ESP32 (the "Quaternion: w, x, y, z" format
produced by webserial_3d.ino) and renders a single rectangular "bone" in a
pygame/OpenGL window. The bone is rotated by the latest quaternion so the
on-screen object moves in real time with the physical sensor.

This was the simplest validation tool used during bring-up: tilt the sensor,
watch the bone follow. The coloured XYZ axes drawn at the origin make it
easy to see which axis is being rotated.
"""

import serial
import time
import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
import math

def quat_to_matrix(w, x, y, z):
    """Convert a quaternion (w, x, y, z) into a 4x4 column-major OpenGL
    rotation matrix. Standard quaternion-to-matrix formula; the trailing 1
    in the bottom-right corner makes it a valid homogeneous transform."""
    return [
        1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w), 0,
        2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w), 0,
        2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y), 0,
        0, 0, 0, 1
    ]

def draw_bone(length=2.0):
    """Draw a rectangular box along the +X axis. Each face is a slightly
    different shade of blue so the orientation is visually obvious as the
    bone rotates."""
    glBegin(GL_QUADS)
    w = 0.3
    h = 0.15
    # top
    glColor3f(0.2, 0.6, 1.0)
    glVertex3f(0, h, -w)
    glVertex3f(length, h, -w)
    glVertex3f(length, h, w)
    glVertex3f(0, h, w)
    # bottom
    glColor3f(0.1, 0.4, 0.8)
    glVertex3f(0, -h, -w)
    glVertex3f(0, -h, w)
    glVertex3f(length, -h, w)
    glVertex3f(length, -h, -w)
    # front
    glColor3f(0.15, 0.5, 0.9)
    glVertex3f(0, -h, w)
    glVertex3f(0, h, w)
    glVertex3f(length, h, w)
    glVertex3f(length, -h, w)
    # back
    glVertex3f(0, -h, -w)
    glVertex3f(length, -h, -w)
    glVertex3f(length, h, -w)
    glVertex3f(0, h, -w)
    # left
    glColor3f(0.1, 0.3, 0.7)
    glVertex3f(0, -h, -w)
    glVertex3f(0, h, -w)
    glVertex3f(0, h, w)
    glVertex3f(0, -h, w)
    # right
    glVertex3f(length, -h, -w)
    glVertex3f(length, -h, w)
    glVertex3f(length, h, w)
    glVertex3f(length, h, -w)
    glEnd()

def draw_axes():
    """Draw red/green/blue lines along the X/Y/Z world axes for reference."""
    glBegin(GL_LINES)
    glColor3f(1, 0, 0); glVertex3f(0,0,0); glVertex3f(1,0,0)   # X = red
    glColor3f(0, 1, 0); glVertex3f(0,0,0); glVertex3f(0,1,0)   # Y = green
    glColor3f(0, 0, 1); glVertex3f(0,0,0); glVertex3f(0,0,1)   # Z = blue
    glEnd()

# Serial setup. The very short timeout makes the read loop non-blocking
# enough that pygame stays responsive even when the sensor is idle.
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.01)
time.sleep(3)        # wait through ESP32 reset
ser.flushInput()

# Pygame / OpenGL window setup.
pygame.init()
display = (800, 600)
pygame.display.set_mode(display, DOUBLEBUF | OPENGL)
pygame.display.set_caption("IMU Bone Visualiser")

glEnable(GL_DEPTH_TEST)
glMatrixMode(GL_PROJECTION)
gluPerspective(45, display[0]/display[1], 0.1, 50.0)
glMatrixMode(GL_MODELVIEW)

# Start at the identity quaternion (no rotation) until the first real sample
# arrives over serial.
qw, qx, qy, qz = 1.0, 0.0, 0.0, 0.0

print("Visualiser running... tilt the sensor!")

while True:
    for event in pygame.event.get():
        if event.type == QUIT:
            pygame.quit()
            ser.close()
            exit()

    # Drain all pending serial lines so we always render the most recent
    # quaternion rather than lagging behind the sensor.
    while ser.in_waiting:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith("Quaternion:"):
                data = line.split(":")[1].strip()
                qw, qx, qy, qz = [float(v) for v in data.split(',')]
        except:
            pass

    # Render one frame: clear, position the camera, draw axes, then draw the
    # bone rotated by the current quaternion.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glTranslatef(0, 0, -8)

    draw_axes()

    matrix = quat_to_matrix(qw, qx, qy, qz)
    glMultMatrixf(matrix)
    draw_bone()

    pygame.display.flip()
    pygame.time.wait(10)
