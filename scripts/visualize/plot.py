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
    polygons = [eval(polygon) for polygon in pd.read_csv('../../data/input/csv/3000.csv')['Polygon'].values]
    fig, ax = plt.subplots()

    show_polygons(ax, polygons, 'white')

    plt.show()

    # j = json.load(open('../../data/output/result_0.json', 'r'))
    
    # aoi = eval(j['aoi']['vertices'])
    # aoi_grids = [eval(grid['vertices']) for grid in j['aoi']['grids']]
    
    # possible_scenes = [eval(scene['vertices']) for scene in j['possible_scenes'] or []]
    
    # result_scenes = [eval(scene['vertices']) for scene in j['result_scenes'] or []]
    # result_grids = [eval(grid['vertices']) for scene in j['result_scenes'] or [] for grid in scene['grids'] or []]

    # fig, ax = plt.subplots()

    # show_polygons(ax, aoi_grids, 'white')
    # show_polygons(ax, [aoi], 'red', alpha=0.2)
    # show_polygons(ax, possible_scenes, 'yellow', alpha=0.4)
    # show_polygons(ax, result_scenes, 'green', alpha=1)
    # show_polygons(ax, result_grids, 'white', alpha=0.3)

    # plt.show()

if __name__ == '__main__':
    main()