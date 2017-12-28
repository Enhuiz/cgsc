import numpy as np
import json
import pandas as pd
import itertools
import matplotlib.pyplot as plt
import os
import copy
from datetime import datetime

from .path import fig_dir, data_dir, bin_dir
from .plot import plot_polygons, plot_directly


def execute_bin(settings):
    '''
    side effect:
        1. create query a report file and then remove it
    '''
    settings["rois_dir"] = data_dir(['rois'])
    settings["products_dir"] = data_dir(['products'])
    output_path = data_dir(['experiment',
                            'tmp',
                            '{}.json'.format(datetime.now())])
    settings["output_path"] = output_path

    os.system("{} -s '{}'".format(bin_dir(['main']), json.dumps(settings)))

    report = {}
    try:
        with open(output_arg, 'r') as f:
            report = json.load(f)
        os.system("rm {}", output_path)
    except Exception as e:
        print(e)
    return report


def run_expt(configures):
    '''
    side effect:
        1. create query results files
    '''
    df = pd.DataFrame(list(itertools.product(*configures.values())))
    df.columns = configures.keys()
    df = df.sort_index(axis=1)
    results = []
    for settings in df.to_dict(orient='records'):
        result = {"settings": settings}
        result["report"] = execute_bin(settings)
        if result["report"] != {}:
            results.append(result)
    df = pd.DataFrame(results)
    results_path = data_dir(['experiment', 'results',
                             '{}.gz'.format(datetime.now())])
    df.to_pickle(results_path)
    return results_path


def load_tail_results(n=1):
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
        dfs[-1]['timestamp'] = timestamp
    return pd.concat(dfs)
