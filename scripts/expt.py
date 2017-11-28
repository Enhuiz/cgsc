from utils.path import data_dir
from utils.expt_tools import run_expt
import json
import numpy as np


def main():
    results = []

    default_delta = 0.01
    default_roi_ratio = 0.0005
    default_archive_size = 15000
    default_num_rois = 1
    default_roi_type = 'rect'

    # var delta
    # run_expt({'delta': [0.005, 0.01, 0.015, 0.02, 0.025],
    #           'num_rois': [default_num_rois],
    #           'roi_type': [default_roi_type],
    #           'roi_ratio': [default_roi_ratio],
    #           'archive_size': [default_archive_size]})

    # var roi size
    run_expt({'delta': [default_delta],
              'num_rois': [default_num_rois],
              'roi_type': [default_roi_type],
              # 'roi_ratio': [0.01, 0.02, 0.05, 0.1, 0.2, 0.5],
              'roi_ratio': [0.01],
              # 'roi_ratio': [0.02],# [x for x in np.linspace(0.002, 0.1, 20)],
              'archive_size': [default_archive_size],
              'target_coverage': [0.9]})

    # var archive_size
    # run_expt({'delta': [default_delta],
    #           'num_rois': [default_num_rois],
    #           'roi_type': [default_roi_type],
    #           'roi_ratio':  [default_roi_ratio],
    #           'archive_size': [15000, 20000, 50000, 75000, 100000],
    #           'target_coverage': [0.9, 0.95]})


if __name__ == '__main__':
    main()
