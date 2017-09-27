import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
import matplotlib
import numpy as np

def show_polygons(ax, polygons, facecolor='white', alpha=1):
    patches = []

    for vertices in polygons:
        polygon = Polygon(
            vertices,
            alpha=alpha, 
            ec='black',
            fc=facecolor,
            lw=1.5)
        patches.append(polygon)

    for patch in patches:
        ax.add_patch(patch)

    ax.autoscale()