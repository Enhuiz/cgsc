import os
import json

from utils.path import data_dir
from utils.expt_tools import run_expt, get_tag
from utils.path import data_dir
from utils.plot import plot_polygons, plt

def query_dir(l): 
    return data_dir(['experiment', 'results', 'query'] + l)

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

def plot_discrete_query(ax, report):
    def parse_cell(cid): 
        def parse_cell_helper(cid, delta):
            xi = cid & ((1 << 32) - 1)
            yi = cid >> 32
            x = xi * delta
            y = yi * delta
            return [[x, y], [x + delta, y], [x + delta, y + delta], [x, y + delta]]
        return parse_cell_helper(cid, report['delta'])

    aoi = eval(report['aoi']['polygon'])
    aoi_cells = [parse_cell(cell) for cell in report['aoi']['cells']]

    possible_scenes = [eval(scene['polygon'])
                       for scene in report['possible_scenes'] or []]
    possible_cells = [parse_cell(cell) for scene in report['possible_scenes'] or [] for cell in scene['cells'] or []]

    result_scenes = [eval(scene['polygon']) for scene in report['result_scenes'] or []]
    result_cells = [parse_cell(cell) for scene in report['result_scenes'] or [] for cell in scene['cells'] or []]

    plot_polygons(ax, aoi_cells, 'white')
    # plot_polygons(ax, generate_cells(aoi, 0.05, True), 'white', 1)
    plot_polygons(ax, result_scenes, 'green', alpha=0.7)
    plot_polygons(ax, result_cells, 'blue', alpha=0.7)
    plot_polygons(ax, possible_scenes, 'green', alpha=0.2)
    plot_polygons(ax, possible_cells, 'blue', alpha=0.2)
    plot_polygons(ax, [aoi], 'none')

def plot_continuous_query(ax, report):
    aoi = eval(report['aoi']['polygon'])
    aoi_offcuts = [eval(offcut) for offcut in report['aoi']['offcuts']]

    possible_scenes = [eval(scene['polygon']) for scene in report['possible_scenes'] or []]
    possible_offcuts = [eval(offcut) for scene in report['possible_scenes'] or [] for offcut in scene['offcuts'] or []]

    result_scenes = [eval(scene['polygon']) for scene in report['result_scenes'] or []]
    result_offcuts = [eval(offcut) for scene in report['result_scenes'] or [] for offcut in scene['offcuts'] or []]

    # plot_polygons(ax, possible_scenes, 'green', alpha=0.2)
    plot_polygons(ax, result_scenes, 'green', alpha=0.7)
    plot_polygons(ax, result_offcuts, ['blue', 'red', 'yellow', 'orange', 'purple'], alpha=0.7)
    # plot_polygons(ax, possible_offcuts, 'blue', alpha=0.2)
    plot_polygons(ax, [aoi], 'none')
    plot_polygons(ax, aoi_offcuts, 'none')

def show(plot):
    fig, ax = plt.subplots()
    plot(ax)
    plt.show()

def main():
    # config = {
    #     'delta': [0.02],
    #     'aoi_ratio': [0.0005],
    #     'n_aois': [1],
    #     'archive': [1000],
    # }

    config = {
        'delta': [0.02],
        'aoi_ratio': [0.01],
        'n_aois': [1],
        'archive': [15000],
    }

    run_expt(config)

    query_result_path = query_dir(['{}.json'.format(get_tag({k: v[0] for k, v in config.items()}))])
    reports = json.load(open(query_result_path, 'r'))

    # show(lambda ax: plot_discrete_query(ax, reports['discrete'][0]))
    show(lambda ax: plot_continuous_query(ax, reports['continuous'][0]))


if __name__ == '__main__':
    main()
