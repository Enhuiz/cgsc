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


def gen_axis_aligned_rectangle(n, ratio=1, x_range=[120, 128], y_range=[42, 50]):
    x0, x1 = x_range
    y0, y1 = y_range
    range_width = x1 - x0
    range_height = y1 - y0
    size = range_width * range_height * ratio
    def gen_one_axis_aligned_rectangle():
        width = np.sqrt(size)
        width = np.clip(np.random.normal(width, width / 4), size / range_height, range_width)
        height = size / width
        minx = x0
        miny = y0
        maxx = minx + width
        maxy = miny + height
        polygon = [[minx, miny], [maxx, miny], [maxx, maxy], [minx, maxy]]
        return move(polygon, [np.random.uniform(0, x1 - maxx), np.random.uniform(0, y1 - maxy)])

    return [gen_one_axis_aligned_rectangle() for i in range(n)]


def main():
    import matplotlib.pyplot as plt
    from plot import plot_polygons

    fig, ax = plt.subplots()
    # plot_polygons(ax, gen_poly(100, 0.25))
    plot_polygons(ax, gen_axis_aligned_rectangle(100, 0.01))
    plt.show()


if __name__ == '__main__':
    main()
