from utils.path import data_dir
from utils.expt_tools import run_expt
import json
import numpy as np 

def main():
    results = []

    default_delta = 0.01
    default_aoi_ratio = 0.25
    default_archive = 15000
    default_n_aois = 5

    # var delta
    # results.append(run_expt({
    #     'delta': [0.0005, 0.001, 0.002, 0.005, 0.01, 0.02],
    #     'n_aois': [default_n_aois],
    #     'aoi_ratio': [default_aoi_ratio],
    #     'archive': [default_archive],
    # }))

    # var aoi size
    results.append(run_expt({
        'delta': [default_delta],
        'n_aois': [default_n_aois],
        'aoi_ratio': [0.02, 0.05, 0.1, 0.2, 0.5],
        'archive': [default_archive],
    }))

    # var archive
    # results.append(run_expt({
    #     'delta': [default_delta],
    #     'n_aois': [default_n_aois],
    #     'aoi_ratio':  [default_aoi_ratio],
    #     'archive': [5000, 10000, 15000, 20000, 50000]
    # }))

    results_path = data_dir(['experiment', 'results', 'summary.json'])
    json.dump(results, open(results_path, 'w'))


if __name__ == '__main__':
    main()
