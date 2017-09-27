import json
import numpy as np
from plot import show_polygons
import matplotlib.pyplot as plt

class Polygon:
    def __init__(self, points=None, price=None):
        self._points = [] if points == None else points
        self._price = 0 if price == None else price

    def set_price(self, price):
        self._price = price

    def __calculate_triangle_area(self, a, b, c):
        a, b, c = map(np.array, (a, b, c))
        v1 = b - a
        v2 = c - b
        return 0.5 * np.cross(v1, v2)

    def get_area(self):
        # how to calculate the area? solved.
        area = 0
        a = self._points[0]
        for b, c in zip(self._points[1:-1], self._points[2:]):
            area += self.__calculate_triangle_area(a, b, c)
        return area

    def get_points(self):
        return self._points

    def __str__(self):
        return '{} sided polygon, area: {:.4f}km^2, price: ${:.2f}/km^2, cost: ${:.2f}'.format(
                    len(self._points), 
                    self.get_area(), 
                    self._price)


def main():
    j = json.load(open('../../data/output/result_0.json', 'r'))
    
    aoi = eval(j['aoi']['vertices'])
    aoi_grids = [eval(grid['vertices']) for grid in j['aoi']['grids']]
    
    possible_scenes = [eval(scene['vertices']) for scene in j['possible_scenes'] or []]
    
    result_scenes = [eval(scene['vertices']) for scene in j['result_scenes'] or []]
    result_grids = [eval(grid['vertices']) for scene in j['result_scenes'] or [] for grid in scene['grids'] or []]

    fig, ax = plt.subplots()

    show_polygons(ax, aoi_grids, 'white')
    show_polygons(ax, [aoi], 'red', alpha=0.2)
    show_polygons(ax, possible_scenes, 'yellow', alpha=0.4)
    show_polygons(ax, result_scenes, 'green', alpha=1)
    show_polygons(ax, result_grids, 'white', alpha=0.3)

    plt.show()

if __name__ == '__main__':
    main()