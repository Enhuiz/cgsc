import pandas as pd

df = pd.read_csv('../data/scenes_full.csv')

df[['Polygon', 'Price']].to_csv('../data/scenes.csv', index=None)