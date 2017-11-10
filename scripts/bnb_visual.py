import pandas as pd
import json
from utils.path import data_dir, fig_dir
import matplotlib.pyplot as plt

vinfo_path = data_dir(['visualize', 'bnb', 'vinfo.json'])
fig_path = fig_dir(['bnb_visual'])

df = pd.DataFrame(json.load(open(vinfo_path, 'r')))
df = df.set_index('t')

for col in df.columns:
    df.plot(y=col, figsize=(15,10))
    plt.savefig(fig_path + '/0.5%/{}.png'.format(col))