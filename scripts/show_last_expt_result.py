from utils.expt_tools import load_latest_results
import pandas as pd
import json
import matplotlib.pyplot as plt

def plot_column(df, col, title=None):
    if title is None: title = col
    axes = df.plot(y=col, subplots=False, marker='x', title=title)
    plt.savefig('figures/{}.png'.format(title))

def main():
    df = load_latest_results(1)
    reports = []
    for report in df['report']:
        reports += report
    df = pd.io.json.json_normalize(reports)
    df = df.round(5)
    df['roi_size(x)'] = df['area_of_roi'] / 0.011
    
    df1 = df.groupby(['transformer', 'roi_size(x)']).mean().unstack(level=0)
    plot_column(df1, 'transformation.time', 'transformation.time')
    plot_column(df1, 'number_of_cells')
    plot_column(df1, 'number_of_possible_products')

    df2 = df.groupby(['transformer', 'optimizer', 'roi_size(x)']).mean().unstack(level=[0, 1])
    plot_column(df2, 'optimization.time')
    plot_column(df2, 'optimization.cost')

    plt.show()

if __name__ == '__main__':
    main()