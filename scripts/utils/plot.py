import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
import numpy as np
import json
import pandas as pd

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

def plot_query_result(ax, query_result_path):
    query_result = json.load(open(query_result_path, 'r'))[0]

    aoi = eval(query_result['aoi']['vertices'])
    # aoi_grids = [eval(grid['vertices']) for grid in query_result['aoi']['grids']]
    
    possible_scenes = [eval(scene['vertices']) for scene in query_result['possibleScenes'] or []]
    possible_grids = [eval(grid['vertices']) for scene in query_result['possibleScenes'] or [] for grid in scene['grids'] or []]

    result_scenes = [eval(scene['vertices']) for scene in query_result['resultScenes'] or []]
    result_grids = [eval(grid['vertices']) for scene in query_result['resultScenes'] or [] for grid in scene['grids'] or []]

    # show_polygons(ax, aoi_grids, 'white')
    show_polygons(ax, [aoi], 'red', 0.5)
    show_polygons(ax, possible_scenes, 'yellow', alpha=0.1)
    # show_polygons(ax, result_scenes, 'green', alpha=1)
    show_polygons(ax, result_grids, 'blue', alpha=0.3)
    # show_polygons(ax, possible_grids, 'red', alpha=1)

def plot_directly(f, *args):
    fig, ax = plt.subplots()
    f(ax, *args)
    plt.show()