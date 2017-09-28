import numpy as np
import pandas as pd
import matplotlib.path as mpltPath
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import math

XMIN = 73
XMAX = 135
YMIN = 4
YMAX = 54
DELTA = 0.2

def contains(polygon, point):
    return mpltPath.Path(polygon).contains_points([point])[0]

def bounds(polygon):
    xmin = XMAX
    xmax = XMIN
    ymin = YMAX
    ymax = YMIN
    for x, y in polygon:
        xmin = min(xmin, x)        
        xmax = max(xmax, x)
        ymin = min(ymin, y)
        ymax = max(ymax, y)
    
    xmin = math.floor((xmin - XMIN) / DELTA)
    xmax = math.ceil((xmax - XMIN) / DELTA)
    
    ymin = math.floor((ymin - YMIN) / DELTA)
    ymax = math.ceil((ymax - YMIN) / DELTA)
    
    return xmin, xmax, ymin, ymax

def main():
    n_scenes = 1000000

    df = pd.read_csv('../../data/input/scenes.csv', nrows=n_scenes)

    polygons = [eval(s) for s in df['Polygon'].values]

    xs = np.arange(XMIN, XMAX, DELTA)
    ys = np.arange(YMIN, YMAX, DELTA)

    dist = np.zeros([len(xs), len(ys)])

    cnt = 0
    pcnt = 0
    for polygon in polygons:
        cnt += 1
        xmin, xmax, ymin, ymax = map(int, bounds(polygon))

        for i in np.arange(xmin, min(xmax, dist.shape[0])):
            for j in np.arange(ymin, min(ymax, dist.shape[1])):
                if contains(polygon, (i * DELTA + XMIN, j * DELTA + YMIN)):
                    dist[i, -j] += 1
                    pcnt += 1

        if cnt % 10000 == 0:
            print(cnt)
            print(pcnt, 'points added')
            pcnt = 0

    plt.imshow(dist.T, extent=(XMIN, XMAX, YMIN, YMAX))
    plt.colorbar()
    plt.show()

if __name__ == '__main__':
    main()