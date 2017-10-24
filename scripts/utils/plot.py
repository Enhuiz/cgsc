import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, Circle
from matplotlib.collections import PatchCollection
import numpy as np
import json
import pandas as pd


def plot_polygons(ax, polygons, face_color='white', alpha=1, annotate=False):
    patches = []

    if isinstance(face_color, list):
        def get_face_color():
            np.random.shuffle(face_color)
            return face_color[0]
    else:
        def get_face_color():
            return face_color

    for vertices in polygons:
        polygon = Polygon(
            vertices,
            alpha=alpha,
            ec='black',
            fc=get_face_color(),
            lw=1.5)
        patches.append(polygon)
        if annotate:
            for i in range(len(vertices)):
                ax.text(*vertices[i], str(i))
    for patch in patches:
        ax.add_patch(patch)

    ax.autoscale()
    ax.axis('equal')


def plot_directly(f, *args):
    fig, ax = plt.subplots()
    f(ax, *args)
    plt.show()
