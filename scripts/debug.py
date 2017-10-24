from utils.plot import plot_polygons as plot_polygons_
import matplotlib.pyplot as plt

fig, ax = plt.subplots()


a = [[125.46442992154537421, 44.831271943379441325], [125.4644299138678889, 44.831271938077499328], [125.55618703508896772, 44.831271938119975573], [125.55618704773239358, 44.831271942958991872]]




plot_polygons = lambda *args: plot_polygons_(*args, annotate=True)

plot_polygons(ax, [a],'none', 0.5)
# plot_polygons(ax, [b],'grey', 0.5)

plt.show()
