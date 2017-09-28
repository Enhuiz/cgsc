import pandas as pd
import numpy as np
import matplotlib.path as mpltPath

def load_scenes():
    try:
        df = pd.read_pickle('../data/input/pkl/scenes.pkl')
    except:
        df = pd.read_csv('../data/input/csv/scenes.csv')
        df['Polygon'] = df['Polygon'].apply(lambda s: eval(s))
        df.to_pickle('../data/input/pkl/scenes.pkl')
    return df

POLYGON = [[120, 42], [128, 42], [128, 50], [120, 50]]
inside = lambda points: mpltPath.Path(POLYGON).contains_points(points)

def is_interest(polygon):
    return all(inside(polygon))

def main():
    df = load_scenes()
    df = df[df['Polygon'].map(is_interest)]
    df = df.sample(frac=1) #important, to shuffle the data
    df.to_csv('../data/input/csv/expt_scenes.csv', index=None)

    for i in [1000, 1500, 2000, 2500, 5000, 10000, 15000, 20000, 50000]:
        df.head(i).to_csv('../data/input/csv/{}.csv'.format(i), index=None)

if __name__ == '__main__':
    main()

