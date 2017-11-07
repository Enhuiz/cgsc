import numpy as np
import json
import pandas as pd
import itertools
import matplotlib.pyplot as plt
import os
import copy
from collections import ChainMap

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


def prepare_archive_arg(config):
    archive = config['archive']
    path = data_dir(['scenes', 'archives', '{}.csv'.format(archive)])
    return path


def prepare_delta_arg(config):
    delta = config['delta']
    return delta

def prepare_target_coverage_ratio_arg(config):
    target_coverage_ratio = config['target_coverage_ratio']
    return target_coverage_ratio


def get_tag(config):
    delta = config['delta']
    roi_ratio = config['roi_ratio']
    n_rois = config['n_rois']
    archive = config['archive']
    tag = '{}-{}-{}-{}'.format(delta, n_rois, roi_ratio, archive)

    return tag


def prepare_output_arg(config):
    tag = get_tag(config)

    path = data_dir(['experiment',
                     'results',
                     'query',
                     '{}.json'.format(tag)])
    return path


def extract_reports(path):
    def extract_reports_helper(raw_reports):
        raw_reports = [raw_report for raw_report in raw_reports if raw_report is not None]
        if len(raw_reports) == 0: return {}
        from numbers import Number
        reports = {}
        for k, v in raw_reports[0].items():
            if isinstance(v, Number):
                reports[k] = np.mean([raw_report[k]
                                      for raw_report in raw_reports])
        # reports['price'] = np.mean([np.sum(
        #     [scene['price'] for scene in raw_report['result_scenes'] or []]) for raw_report in raw_reports])
        return reports

    def add_prefix_to_dict_key(d, prefix):
        ret = copy.deepcopy(d)
        for k in d.keys():
            ret[prefix + "_" + k] = ret.pop(k)
        return ret

    def extract(raw_reports, k):
        reports = extract_reports_helper(raw_reports[k])
        return add_prefix_to_dict_key(reports, k)

    raw_reports = json.load(open(path, 'r'))
    return dict(ChainMap(*[extract(raw_reports, k) for k in raw_reports.keys()]))


def execute(config):
    '''
    side effect:
        1. create roi files
        2. create query results files
    '''
    roi_arg = prepare_roi_arg(config)
    archive_arg = prepare_archive_arg(config)
    delta_arg = prepare_delta_arg(config)
    output_arg = prepare_output_arg(config)
    target_coverage_ratio_arg = prepare_target_coverage_ratio_arg(config)
    bin_arg = bin_dir(['main'])

    os.system('{} -r {} -s {} -d {} -o {} -t {}'.format(bin_arg,
                                                  roi_arg,
                                                  archive_arg,
                                                  delta_arg,
                                                  output_arg,
                                                  target_coverage_ratio_arg))

    return extract_reports(output_arg)


def run_expt(configs):
    '''
    side effect: 
        1. create roi files
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

    return {'variable': {var_name: var_values},
            'parameters': parameters,
            'reports': reports}
