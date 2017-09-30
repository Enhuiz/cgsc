import numpy as np
import pandas as pd
import sys


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


def gen_aoi(n_aois, size=1, x_range=[120, 128], y_range=[42, 50]):
    length = np.sqrt(size)
    polygons = [unit_polygon(np.random.randint(3, 10)) for i in range(n_aois)]
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


def main():
    import matplotlib.pyplot as plt
    from plot import show_polygons

    fig, ax = plt.subplots()
    show_polygons(ax, gen_aoi(100, 0.25))
    plt.show()


if __name__ == '__main__':
    main()
