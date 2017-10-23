from utils.path import data_dir, fig_dir
import json
import pandas as pd
import matplotlib.pyplot as plt

def plot_summary(var_name, reports):
    def savefig(tag):
        plt.savefig(fig_dir(['experiment', '{}.png'.format(tag)]))
    df = pd.DataFrame(reports).sort_values(var_name)
    for y_name in df.columns:
        if y_name not in [var_name, 't1', 't2']:
            df.plot.bar(x=var_name, y=y_name, rot=0)
            savefig('{}-{}'.format(y_name, var_name))
    df[[var_name, 't1', 't2']].plot.bar(x=var_name, stacked=True, rot=0)
    savefig('{}-{}'.format('t', var_name))

def main():
    summary = json.load(open(data_dir(['experiment', 'results', 'summary.json']), 'r'))
    for sub_summary in summary:
        plot_summary(next(iter(sub_summary['variable'].keys())), sub_summary['reports'])

if __name__ == '__main__':
    main()
