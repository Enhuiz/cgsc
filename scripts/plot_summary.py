from utils.path import data_dir, fig_dir
import json
import pandas as pd
import matplotlib.pyplot as plt
import re

def plot_summary(var_name, reports):
    def savefig(tag):
        plt.savefig(fig_dir(['experiment', '{}.png'.format(tag)]))

    def get_y_names(l):
        return set([re.sub('continuous_|discrete_', '', s) for s in l])

    df = pd.DataFrame(reports).sort_values(var_name)
    
    for y_name in get_y_names(df.columns):
        if y_name not in [var_name, 'delta']:
            continuous_y_name = 'continuous_' + y_name
            discrete_y_name = 'discrete_' + y_name
            df[[var_name,  discrete_y_name, continuous_y_name]].plot.bar(x=var_name, rot=0)
            savefig('{}-{}'.format(y_name, var_name))
    # df.plot.bar(x=var_name, y=y_name, rot=0)
    # savefig('{}-{}'.format('t', var_name))

def main():
    summary = json.load(open(data_dir(['experiment', 'results', 'summary.json']), 'r'))
    for sub_summary in summary:
        plot_summary(next(iter(sub_summary['variable'].keys())), sub_summary['reports'])

if __name__ == '__main__':
    main()
