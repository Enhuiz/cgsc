import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
import numpy as np
import json
import pandas as pd


def show_polygons(ax, polygons, face_color='white', alpha=1):
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

    for patch in patches:
        ax.add_patch(patch)

    ax.autoscale()
    ax.axis('equal')


def generate_cells(poly, delta, keep_edge=False):
    from math import ceil, floor
    import matplotlib.path as mpltPath

    def contains(polygon, point):
        return mpltPath.Path(polygon).contains_points([point])[0]

    min_trunc, max_trunc = ceil, floor
    if keep_edge:
        min_trunc, max_trunc = floor, ceil

    minxi = min_trunc(min([p[0] for p in poly]) / delta)
    minyi = min_trunc(min([p[1] for p in poly]) / delta)
    maxxi = max_trunc(max([p[0] for p in poly]) / delta)
    maxyi = max_trunc(max([p[1] for p in poly]) / delta)

    def generate_cell_poly(xi, yi):
        x = xi * delta
        y = yi * delta
        return [[x, y], [x + delta, y], [x + delta, y + delta], [x, y + delta]]

    ret = []

    cond = any if keep_edge else all

    for xi in range(minxi, maxxi):
        for yi in range(minyi, maxyi):
            cell = generate_cell_poly(xi, yi)
            if cond([contains(poly, p) for p in cell]):
                ret.append(cell)

    return ret


def plot_query_result(ax, report_path):
    report = json.load(open(report_path, 'r'))[0]

    def parse_cell_helper(cid, delta):
        xi = cid & ((1 << 32) - 1)
        yi = cid >> 32
        x = xi * delta
        y = yi * delta
        return [[x, y], [x + delta, y], [x + delta, y + delta], [x, y + delta]]

    def parse_cell(cid): return parse_cell_helper(cid, report['delta'])

    aoi = eval(report['aoi']['polygon'])
    aoi_cells = [parse_cell(cell) for cell in report['aoi']['cells']]

    possible_scenes = [eval(scene['polygon'])
                       for scene in report['possible_scenes'] or []]
    possible_cells = [parse_cell(cell) for scene in report['possible_scenes'] or [] for cell in scene['cells'] or []]

    result_scenes = [eval(scene['polygon']) for scene in report['result_scenes'] or []]
    result_cells = [parse_cell(cell) for scene in report['result_scenes'] or [] for cell in scene['cells'] or []]

    show_polygons(ax, aoi_cells, 'white')
    # show_polygons(ax, generate_cells(aoi, 0.05, True), 'white', 1)
    show_polygons(ax, result_scenes, 'green', alpha=0.7)
    show_polygons(ax, result_cells, 'blue', alpha=0.7)
    show_polygons(ax, possible_scenes, 'green', alpha=0.2)
    show_polygons(ax, possible_cells, 'blue', alpha=0.2)
    show_polygons(ax, [aoi], 'none')

    # show_polygons(ax, result_scenes_bpolys, ['yellow', 'green', 'blue', 'red', 'black', 'orange', 'purple'], alpha=0.5)


def plot_directly(f, *args):
    fig, ax = plt.subplots()
    f(ax, *args)
    plt.show()
