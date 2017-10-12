import pandas as pd
import numpy as np
import matplotlib.path as mpltPath
from utils.path import data_dir

POLYGON = [[120, 42], [128, 42], [128, 50], [120, 50]]


def load_scenes():
    try:
        df = pd.read_pickle(data_dir(['scenes', 'origin', 'scenes.pkl']))
    except:
        df = pd.read_csv(data_dir(['scenes', 'origin', 'scenes.csv']))
        df['Polygon'] = df['Polygon'].apply(lambda s: eval(s))
        df.to_pickle(data_dir(['scenes', 'origin', 'scenes.pkl']))
    return df


def inside(points):
    return mpltPath.Path(POLYGON).contains_points(points)


def is_interested(polygon):
    return all(inside(polygon))


def main():
    df = load_scenes()
    df = df[df['Polygon'].map(is_interested)]

    filepath = lambda s: data_dir(['scenes', 'archives', '{}.csv'.format(s)])
    df.to_csv(filepath('total'), index=None)

    exit()
    
    df = df.sample(frac=1)  # important, to shuffle the data
    for i in [1000, 1500, 2000, 2500, 5000, 10000, 15000, 20000, 50000]:
        df.head(i).to_csv(filepath(str(i)), index=None)


if __name__ == '__main__':
    main()
