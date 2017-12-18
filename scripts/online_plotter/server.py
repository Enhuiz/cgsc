import matplotlib.pyplot as plt
import numpy as np
import time
from matplotlib.patches import Polygon, Circle
from matplotlib.collections import PatchCollection


def plot_polygons(ax, polygons, face_color='white', alpha=1, annotate=False):
    patches = []

    if isinstance(face_color, list):
        def get_face_color():
            color = face_color[0]
            del face_color[0]
            return color
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

    # ax.set_xlim([120, 128])
    # ax.set_ylim([42, 50])
    ax.autoscale()
    ax.axis('equal')


def get_colors(n):
    x = np.linspace(0.1, 0.1, n)
    return list(zip(x, x, x))


def draw(ax):
    try:
        content = open('polygons.txt', 'r').read()
    except:
        content = ""
    max_num_polygon = 1500
    polygons = [eval(s) for s in content.strip().split('\n') if len(s) > 0]
    polygons = [polygon for polygon in polygons if len(
        polygon) > 0][-max_num_polygon:]
    plot_polygons(ax, polygons, get_colors(max_num_polygon), alpha=0.5)


def main():
    plt.ion()
    fig, ax = plt.subplots()
    ax.set_autoscale_on(False)
    while True:
        ax.clear()
        draw(ax)
        plt.pause(1)


if __name__ == '__main__':
    main()
