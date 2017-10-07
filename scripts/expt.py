from utils.path import data_dir
from utils.expt_tools import run_expt
import json

def main():
    results = []

    default_delta = 0.01
    default_aoi_size = 0.25
    default_archive = 15000
    default_n_aois = 1

    # var delta
    results += run_expt({
        'delta': [0.0005, 0.001, 0.002, 0.005, 0.01, 0.02],
        'n_aois': [default_n_aois],
        'aoi_size': [default_aoi_size],
        'archive': [default_archive],
    })

    # # var aoi size
    # results += run_expt({
    #     'delta': [default_delta],
    #     'n_aois': [default_n_aois],
    #     'aoi_size': [0.1, 0.25, 0.5, 0.75, 1.0],
    #     'archive': [default_archive],
    # })

    # # var archive
    # results += run_expt({
    #     'delta': [default_delta],
    #     'n_aois': [default_n_aois],
    #     'aoi_size':  [default_aoi_size],
    #     'archive': [5000, 10000, 15000, 20000, 50000]
    # })

    results_path = data_dir(['experiment', 'results', 'summary.json'])

    json.dump(results, open(results_path, 'w'))


if __name__ == '__main__':
    main()
