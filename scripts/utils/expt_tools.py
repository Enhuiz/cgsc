import numpy as np
import json
import pandas as pd
import itertools
import matplotlib.pyplot as plt
import os

from .gen_poly import gen_poly, gen_axis_aligned_rectangle
from .path import fig_dir, data_dir, bin_dir
from .plot import show_polygons, plot_directly


def prepare_aoi_arg(config):
    '''
    side effect:
        1. create aoi files
    '''
    n_aois, aoi_ratio = config['n_aois'], config['aoi_ratio']

    tag = '{}-{}-rect'.format(n_aois, aoi_ratio)
    path = data_dir(['experiment', 'generated_aois', '{}.csv'.format(tag)])

    if not os.path.exists(path):
        aois = gen_axis_aligned_rectangle(n_aois, aoi_ratio)
        df = pd.DataFrame([str([list(v) for v in aoi]) for aoi in aois])
        df.columns = ['Polygon']
        df.to_csv(path, index=None)
        # plot_directly(show_polygons, aois)

    return path


def prepare_archive_arg(config):
    archive = config['archive']
    path = data_dir(['scenes', 'archives', '{}.csv'.format(archive)])
    return path


def prepare_delta_arg(config):
    delta = config['delta']
    return delta


def get_tag(config):
    delta = config['delta']
    aoi_ratio = config['aoi_ratio']
    n_aois = config['n_aois']
    archive = config['archive']
    tag = '{}-{}-{}-{}'.format(delta, n_aois, aoi_ratio, archive)

    return tag


def prepare_output_arg(config):
    tag = get_tag(config)

    path = data_dir(['experiment',
                     'results',
                     'query',
                     '{}.json'.format(tag)])
    return path


def extract_reports(path):
    raw_reports = json.load(open(path, 'r'))
    reports = {}
    from numbers import Number
    for k, v in raw_reports[0].items():
        if isinstance(v, Number):
            reports[k] = np.mean([raw_report[k] for raw_report in raw_reports])

    reports['price'] = np.mean([np.sum(
        [scene['price'] for scene in raw_report['result_scenes'] or []]) for raw_report in raw_reports])
    return reports


def save_fig(tag):
    plt.savefig(fig_dir(['experiment', '{}.png'.format(tag)]))


def save_figures(reports, var_name):
    df = pd.DataFrame(reports).sort_values(var_name)
    for y_name in df.columns:
        if y_name not in [var_name, 't1', 't2']:
            df.plot.bar(x=var_name, y=y_name, rot=0)
            save_fig('{}-{}'.format(y_name, var_name))
    df[['t1', 't2']].plot.bar(stacked=True, rot=0)
    save_fig('{}-{}'.format('t', var_name))


def execute(config):
    '''
    side effect:
        1. create aoi files
        2. create query results files
    '''
    aoi_arg = prepare_aoi_arg(config)
    archive_arg = prepare_archive_arg(config)
    delta_arg = prepare_delta_arg(config)
    output_arg = prepare_output_arg(config)
    bin_arg = bin_dir(['main'])

    os.system('{} -a {} -s {} -d {} -o {}'.format(bin_arg,
                                                  aoi_arg,
                                                  archive_arg,
                                                  delta_arg,
                                                  output_arg))

    return extract_reports(output_arg)


def run_expt(configs):
    '''
    side effect: 
        1. create aoi files
        2. create query result files
        3. create a summary file
        4. create figs (optional)
    '''
    try:
        var_name, var_values = next(
            kv for kv in configs.items() if len(kv[1]) > 1)
    except:
        var_name, var_values = next(kv for kv in configs.items())
    parameters = {k: vs[0] for k, vs in configs.items() if k != var_name}
    configs = [{**parameters, var_name: var_value} for var_value in var_values]
    reports = []
    for config in configs:
        print(get_tag(config), 'start running!')
        report = execute(config)
        report[var_name] = config[var_name]
        reports.append(report)
    if len(var_values) > 1:
        save_figures(reports, var_name)
    return reports
