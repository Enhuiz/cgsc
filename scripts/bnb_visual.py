import pandas as pd
import json
from utils.path import data_dir, fig_dir
import matplotlib.pyplot as plt
import matplotlib 
import os
import re

vinfo_path = data_dir(['visualize', 'bnb'])
fig_path = fig_dir(['bnb_visual'])

matplotlib.rcParams.update({'font.size': 18})

d = {}

for dirpath, dirnames, filenames in os.walk(vinfo_path):
    for filename in filenames:
        btype, s = re.match("(\[.+\])(.+)", filename).groups()
        roi_ratio = s.strip().split('-')[2]
        if roi_ratio not in d:
            d[roi_ratio] = {}
        for k, v in json.load(open(os.path.join(dirpath, filename), 'r'))[0].items():
            if k not in d[roi_ratio]:
                d[roi_ratio][k] = {}
            d[roi_ratio][k][btype] = v

for roi_ratio, subd in d.items():
    dirpath = os.path.join(fig_path, roi_ratio)
    if not os.path.exists(dirpath):
        os.makedirs(dirpath)
    for k, subsubd in subd.items():
        if k == 't': continue
        filepath = os.path.join(dirpath, '{}.png'.format(k))
        fig, ax = plt.subplots(figsize=(15, 10))
        for btype, v in subsubd.items():
            df = pd.DataFrame({'t': subd['t'][btype], btype + ' ' + k: subsubd[btype]})
            df = df.set_index('t')
            df = df.iloc[::max(1, len(df)//10000), :]
            df.plot(figsize=(15,10), ax=ax)
        plt.savefig(filepath)