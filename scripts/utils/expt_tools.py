import numpy as np
import json
import pandas as pd
import itertools
import matplotlib.pyplot as plt
import os

from .gen_aoi import gen_aoi
from .path import fig_dir, data_dir, bin_dir
from .plot import show_polygons


def prepare_aoi_arg(config):
    '''
    side effect:
        1. create aoi files
    '''
    n_aois, aoi_size = config['n_aois'], config['aoi_size']

    tag = '{}-{}'.format(n_aois, aoi_size)
    path = data_dir(['experiment', 'generated_aois', '{}.csv'.format(tag)])

    if not os.path.exists(path):
        aois = gen_aoi(n_aois, aoi_size)
        df = pd.DataFrame([str([list(v) for v in aoi]) for aoi in aois])
        df.columns = ['Polygon']
        df.to_csv(path, index=None)

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
    aoi_size = config['aoi_size']
    n_aois = config['n_aois']
    archive = config['archive']
    tag = '{}-{}-{}-{}'.format(delta, n_aois, aoi_size, archive)

    return tag

def prepare_output_arg(config):
    tag = get_tag(config)

    path = data_dir(['experiment',
                     'results',
                     'query',
                     '{}.json'.format(tag)])
    return path


def extract_result(path):
    reports = json.load(open(path, 'r'))

    average_t1 = np.mean([report['timestamp']['t1'] for report in reports])
    average_t2 = np.mean([report['timestamp']['t2'] for report in reports])

    average_price = np.mean([np.sum([scene['price'] for scene in report['resultScenes']]) for report in reports])
    average_count = np.mean([report['coverageRatio'] for report in reports])

    return {'t1': average_t1, 't2': average_t2, 'price': average_price, 'coverage-ratio': average_count}


def save_fig(tag):
    plt.savefig(fig_dir(['experiment', '{}.png'.format(tag)]))


def to_figures(query_results, variable_list):
    df = pd.DataFrame(query_results)

    dfs = {}

    for i in range(len(variable_list)):
        current_variable = variable_list[i]
        fixed_variables = variable_list[:i] + variable_list[i+1:]
        groupby_instance = df.groupby(fixed_variables)
        # the next step select the group which has max rows
        # the number of rows equal to the len of the varying varaible
        varying_df = max(groupby_instance, key=lambda grouped_df: len(grouped_df[1]))[1]
        if len(varying_df) > 1:
            dfs[current_variable] = varying_df

    for variable, plot_df in dfs.items():
        plot_df = plot_df.drop([c for c in variable_list if c != variable], axis=1)
        plot_df = plot_df.set_index(variable).sort_index()
        for col in plot_df:
            if col not in ['t1', 't2']:
                plot_df.plot.bar(y=col, rot=0)
                save_fig('{}-{}'.format(col, variable))
        plot_df[['t1','t2']].plot.bar(stacked=True, rot=0)
        save_fig('{}-{}'.format('t', variable))

def run_expt_helper(config):
    '''
    side effect:
        1. create aoi files
        2. create query results files
    '''
    aoi_arg = prepare_aoi_arg(config)
    archive_arg = prepare_archive_arg(config)
    delta_arg = prepare_delta_arg(config)
    output_arg = prepare_output_arg(config)
    bin_arg = bin_dir(['expt'])

    os.system('{} -a {} -s {} -d {} -o {}'.format(bin_arg,
                                                  aoi_arg,
                                                  archive_arg,
                                                  delta_arg,
                                                  output_arg))

    return output_arg


def run_expt(configs, draw_result=True):
    '''
    side effect: 
        1. create aoi files
        2. create query result files
        3. create a summary file
        4. create figs
    '''
    results = []
    configs_keys, configs_values = map(list, zip(*configs.items()))

    for config_values in itertools.product(*configs_values):
        config = {k: v for k, v in zip(configs_keys, config_values)}
        print(get_tag(config), 'start running!')
        result_path = run_expt_helper(config)
        result = extract_result(result_path)
        result = {**result, **config}
        results.append(result)

    if draw_result:
        to_figures(results, configs_keys)
    
    return results




# def bar_char(xs, ys, title, ylabel):
#     fig, ax = plt.subplots()

#     ind = np.arange(len(xs))

#     width = 0.2

#     ax.bar(ind, ys, width)

#     plt.xticks(ind, xs)
#     plt.xlabel(title)
#     plt.ylabel(ylabel)

#     plt.title(title)

#     plt.savefig(fig_dir(['{}_bar_char.png'.format(title)]))


# def stacked_bar_char(xs, y1s, y2s, label1, label2, title):
#     fig, ax = plt.subplots()

#     ind = np.arange(len(xs))

#     width = 0.2

#     ax.bar(ind, y1s, width, label=label1)
#     ax.bar(ind, y2s, width, bottom=y1s, label=label2)

#     plt.xticks(ind, xs)
#     plt.xlabel(title)
#     plt.ylabel('average time(s)')

#     plt.legend()

#     plt.savefig(fig_dir(['{}_stacked_bar_char.png'.format(title)]))


# def line_chart(xs, ys, title, ylabel):
#     fig, ax = plt.subplots()

#     plt.plot(xs, ys, marker='x')

#     plt.xlabel(title)
#     plt.ylabel(ylabel)

#     plt.savefig(fig_dir(['{}_line_chart.png'.format(title)]))