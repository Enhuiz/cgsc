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

def plot_query_result(ax, report_path):
    report = json.load(open(report_path, 'r'))[0]

    def parse_cell_helper(cid, delta):
        xi = cid & ((1 << 32) - 1)
        yi = cid >> 32
        x = xi * delta
        y = yi * delta
        return [[x, y], [x + delta, y], [x + delta, y + delta], [x, y + delta]]

    parse_cell = lambda cid: parse_cell_helper(cid, report['delta'])

    aoi = eval(report['aoi']['polygon'])
    aoi_cells = [parse_cell(cell) for cell in report['aoi']['cells']]
    
    possible_scenes = [eval(scene['polygon']) for scene in report['possible_scenes'] or []]
    possible_cells = [parse_cell(cell) for scene in report['possible_scenes'] or [] for cell in scene['cells'] or []]

    result_scenes = [eval(scene['polygon']) for scene in report['result_scenes'] or []]
    result_cells = [parse_cell(cell) for scene in report['result_scenes'] or [] for cell in scene['cells'] or []]

    show_polygons(ax, aoi_cells, 'white')
    show_polygons(ax, [aoi], 'purple', 0.5)
    show_polygons(ax, possible_scenes, 'yellow', alpha=0.1)
    show_polygons(ax, possible_cells, 'red', alpha=0.5)
    show_polygons(ax, result_scenes, 'green', alpha=0.3)
    show_polygons(ax, result_cells, 'blue', alpha=0.3)

def plot_directly(f, *args):
    fig, ax = plt.subplots()
    f(ax, *args)
    plt.show()