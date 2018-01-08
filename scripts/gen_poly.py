import numpy as np
import os
import pandas as pd
import sys
import matplotlib.pyplot as plt
from utils.path import data_dir
from utils.plot import plot_polygons

def apply_transform(mat, vertices):
    return [v.dot(mat.T) for v in vertices]


def rotate(vertices, theta):
    mat = np.array([[np.cos(theta), -np.sin(theta)],
                    [np.sin(theta), np.cos(theta)]])

    return apply_transform(mat, vertices)


def move(vertices, vec):
    return [np.array(vec) + v for v in vertices]


def scale(vertices, vec):
    mat = np.array([[vec[0], 0], [0, vec[1]]])

    return apply_transform(mat, vertices)


def unit_polygon(n_vertices):
    vertices = [np.array([np.cos(theta), np.sin(theta)])
                for theta in np.arange(0, 2 * np.pi, 2 * np.pi / n_vertices)]

    area = np.sqrt(0.5 * np.sin(2 * np.pi / n_vertices) * n_vertices)

    return scale(vertices, [1 / area, 1 / area])


def gen_poly(n, ratio=1, x_range=[120, 128], y_range=[42, 50]):
    size = (x_range[1] - x_range[0]) * (y_range[1] - y_range[0]) * ratio
    length = np.sqrt(size)
    polygons = [unit_polygon(np.random.randint(3, 10)) for i in range(n)]
    polygons = [scale(polygon, [np.sqrt(size), np.sqrt(size)])
                for polygon in polygons]

    for i in range(3):
        polygons = [scale(polygon, [0.8, 1.25]) for polygon in polygons]
        polygons = [rotate(polygon, np.random.uniform() * 2 * np.pi)
                    for polygon in polygons]
                    
    polygons = [move(polygon,
                     [np.random.uniform(x_range[0] + length, x_range[1] - length),
                      np.random.uniform(y_range[0] + length, y_range[1] - length)]) for polygon in polygons]
    return polygons


def gen_axis_aligned_rectangle(n, size=1, x_range=[120, 128], y_range=[42, 50]):
    x0 = x_range[0]
    x1 = x_range[1]
    y0 = y_range[0]
    y1 = y_range[1]
    range_width = x1 - x0
    range_height = y1 - y0
    def gen_one_axis_aligned_rectangle():
        sqrt_size = np.sqrt(size)
        width = np.random.normal(sqrt_size, sqrt_size / 3)
        width = np.clip(width, max(size / range_height, sqrt_size / 2), min(range_width, sqrt_size * 2))
        height = size / width
        minx = x0
        miny = y0
        maxx = minx + width
        maxy = miny + height
        polygon = [[minx, miny], [maxx, miny], [maxx, maxy], [minx, maxy]]
        return move(polygon, [np.random.uniform(0, x1 - maxx), np.random.uniform(0, y1 - maxy)])

    return [gen_one_axis_aligned_rectangle() for i in range(n)]


def main():
    MIN_PRODUCT_AREA = 0.011
    small = [2, 4, 6, 8, 10, 12, 14, 16]
    median = [20, 40, 60, 80, 100]
    large = []
    for ratio in small + median + large:
        path = data_dir(['rois', 'rect(x)_{}.csv'.format(ratio)])
        size = ratio * MIN_PRODUCT_AREA
        if not os.path.isfile(path):
            rects = [[list(point) for point in rect] for rect in gen_axis_aligned_rectangle(1000, size)]
            # fig, ax = plt.subplots()
            # plot_polygons(ax, rects)
            # plt.show()
            df = pd.DataFrame([rects]).transpose()
            df.columns = ['Polygon']
            df.to_csv(path, index=None)
        else:
            print(path, 'exists!')


if __name__ == '__main__':
    main()
