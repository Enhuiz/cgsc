from utils.path import data_dir, fig_dir
import json
import pandas as pd
import matplotlib.pyplot as plt
import re

def plot_summary(var_name, reports):
    def savefig(tag):
        plt.savefig(fig_dir(['experiment', '{}.png'.format(tag)]))

    def get_y_names(l):
        return set('_'.join(s.split('_')[1:]) for s in l if s != var_name)

    def get_prefixes(l):
        return set(s.split('_')[0] + '_' for s in l if s != var_name)

    df = pd.DataFrame(reports).sort_values(var_name)

    if var_name == 'roi_ratio':
        df['roi_pct'] = df['roi_ratio'] * 100
        del df['roi_ratio']
        var_name = 'roi_pct'
        df = df[df['roi_pct'] < 50]

    for y_name in get_y_names(df.columns):
        if y_name not in [var_name, 'delta']:
            prefixes = get_prefixes(df.columns)
            df[[var_name,  *[prefix + y_name for prefix in prefixes]]].plot(x=var_name, rot=0, marker='x')
            savefig('{}-{}'.format(y_name, var_name))
    # df.plot.bar(x=var_name, y=y_name, rot=0)
    # savefig('{}-{}'.format('t', var_name))

def main():
    summary = json.load(open(data_dir(['experiment', 'results', 'summary.json']), 'r'))
    for sub_summary in summary:
        plot_summary(next(iter(sub_summary['variable'].keys())), sub_summary['reports'])

if __name__ == '__main__':
    main()
