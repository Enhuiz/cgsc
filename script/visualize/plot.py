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


def main():
    import pandas as pd
    polygons = [eval(polygon) for polygon in pd.read_csv('../../data/input/csv/15000.csv')['Polygon'].values]
    fig, ax = plt.subplots()

    show_polygons(ax, polygons, 'white')

    plt.show()


if __name__ == '__main__':
    main()