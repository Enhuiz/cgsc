import numpy as np
import json
import pandas as pd
import itertools
import matplotlib.pyplot as plt
import os
import copy
from datetime import datetime

from .gen_poly import gen_poly, gen_axis_aligned_rectangle
from .path import fig_dir, data_dir, bin_dir
from .plot import plot_polygons, plot_directly


def prepare_roi_arg(config):
    '''
    side effect:
        1. create roi files
    '''
    n_rois, roi_ratio = config['n_rois'], config['roi_ratio']

    tag = '{}-{}-rect'.format(n_rois, roi_ratio)
    path = data_dir(['experiment', 'generated_rois', '{}.csv'.format(tag)])

    if not os.path.exists(path):
        rois = gen_axis_aligned_rectangle(n_rois, roi_ratio)
        df = pd.DataFrame([str([list(v) for v in roi]) for roi in rois])
        df.columns = ['Polygon']
        df.to_csv(path, index=None)
        # plot_directly(plot_polygons, rois)

    return path


def execute_bin(setting):
    '''
    side effect:
        1. create query a report file and then remove it
    '''
    bin_arg = bin_dir(['main'])
    setting_arg = json.dumps(setting)
    roi_arg = data_dir(['experiment', 'rois'])
    archive_arg = data_dir(['scenes', 'archives'])
    output_arg = data_dir(['experiment', 'tmp',
                           '{}.json'.format(datetime.now())])

    os.system("{} -s '{}' -r {} -a {} -o '{}'".format(bin_arg,
                                                    setting_arg,
                                                    roi_arg,
                                                    archive_arg,
                                                    output_arg))

    report = {}
    try:
        with open(output_arg, 'r') as f:
            report = json.load(f)
        os.system("rm '{}'".format(output_arg))
    except Exception as e:
        print(e)
    return report


def run_expt(settings):
    '''
    side effect:
        1. create query results files
    '''
    df = pd.DataFrame(list(itertools.product(*settings.values())))
    df.columns = settings.keys()
    df = df.sort_index(axis=1)
    results = []
    for setting in df.to_dict(orient='records'):
        result = {"setting": setting}
        result["report"] = execute_bin(setting)
        results.append(result)
    df = pd.DataFrame(results)
    df = pd.concat([df['setting'].apply(pd.Series), df['report']], axis=1)
    results_path = data_dir(['experiment', 'results',
                             '{}.gz'.format(datetime.now())])
    df.to_pickle(results_path)
    return results_path


def latest_results(n=1):
    results_dir = data_dir(['experiment', 'results'])
    for dirpath, dirnames, filenames in os.walk(results_dir):
        timestamps = [filename.strip('.gz') for filename in filenames]
    timestmaps = pd.to_datetime(pd.Series(timestamps))
    timestmaps = timestmaps.sort_values()
    timestmaps = timestmaps.tail(n)
    dfs = []
    for timestamp in timestmaps:
        dfs.append(pd.read_pickle(os.path.join(
            results_dir, '{}.gz'.format(timestamp))))
    return pd.concat(dfs)

# def run_expt(configs):
#     '''
#     side effect:
#         1. create roi files
#         2. create query result files
#         3. create a summary file
#     '''
#     try:
#         var_name, var_values = next(
#             kv for kv in configs.items() if len(kv[1]) > 1)
#     except:
#         var_name, var_values = next(kv for kv in configs.items())
#     parameters = {k: vs[0] for k, vs in configs.items() if k != var_name}
#     configs = [{**parameters, var_name: var_value} for var_value in var_values]
#     reports = []
#     for config in configs:
#         print(get_tag(config), 'start running!')
#         report = execute_bin(config)
#         report[var_name] = config[var_name]
#         reports.append(report)

#     return {'variable': {var_name: var_values},
#             'parameters': parameters,
#             'reports': reports}
