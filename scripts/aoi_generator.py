import numpy as np
from visualize.plot import show_polygons
import matplotlib.pyplot as plt

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

def generate_aoi(n_aois, size=1):
    polygons = [unit_polygon(np.random.randint(3, 10)) for i in range(n_aois)]
    polygons = [scale(polygon, [np.sqrt(size), np.sqrt(size)]) for polygon in polygons]
    for i in range(3):
        polygons = [scale(polygon, [0.8, 1.25]) for polygon in polygons]
        polygons = [rotate(polygon, np.random.uniform() * 2 * np.pi) for polygon in polygons]
    polygons = [move(polygon, [np.random.uniform(60, 133), np.random.uniform(4, 50)]) for polygon in polygons]
    return polygons

def main():
    fig, ax = plt.subplots()
    show_polygons(ax, generate_aoi(100))
    plt.show()

if __name__ == '__main__':
    main()