import json
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from utils.path import data_dir

deltas = [0.01, 0.02, 0.05, 0.1, 0.2, 0.5]
n_scenes = []

for i in [0.01, 0.02, 0.05, 0.1, 0.2, 0.5]:
    file_path = data_dir(['experiment', 'results', 'query', '0.01-50-{}-15000.json'.format(i)])
    reports = json.load(open(file_path, 'r'))
    n_scenes.append(np.mean([len(report['possible_scenes']) for report in reports]))

df = pd.DataFrame([deltas, n_scenes]).transpose().sort_values(0)
df.columns = ['delta', 'number_of_scenes']
df.plot(x='delta', y='number_of_scenes', marker='x')

plt.show()