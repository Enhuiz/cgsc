from polygon import Polygon
import numpy as np
import unittest

class TestArea(unittest.TestCase):
    def build_points(self, values):
        return [(x, y) for x, y in zip(values[::2], values[1::2])]

    def test1(self):
        points = self.build_points([
            -0.5, 1, 
            0, 0, 
            -0.5, -1, 
            1, 0])
        for i in range(len(points)):
            polygon = Polygon(list(np.roll(points, i, axis=0)))
            self.assertTrue(polygon.get_area() == 1)
    
    def test2(self):
        half = 0.5
        sqrt3half = 0.5 * np.sqrt(3)
        points = self.build_points([
            1, 0,
            half, sqrt3half,
            -half, sqrt3half,
            -1, 0,
            -half, -sqrt3half,
            half, -sqrt3half
        ])
        for i in range(len(points)):
            polygon = Polygon(list(np.roll(points, i, axis=0)))
            self.assertAlmostEqual(polygon.get_area(), 2.59807621135332)

def main():
    unittest.main()

if __name__ == '__main__':
    main()