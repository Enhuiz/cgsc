import pandas as pd
import numpy as np
# df = pd.read_csv('../data/scenes_full.csv')
# df[['Polygon', 'Price']].to_csv('../data/scenes.csv', index=None)


df = pd.read_csv('../data/input/scenes_small.csv')

polygons = df['Polygon'].apply(lambda s: eval(s))

print(min([v[0] for polygon in polygons for v in polygon]))
print(max([v[0] for polygon in polygons for v in polygon]))

print(min([v[1] for polygon in polygons for v in polygon]))
print(max([v[1] for polygon in polygons for v in polygon]))

