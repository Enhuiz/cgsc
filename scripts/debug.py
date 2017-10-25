from utils.plot import plot_polygons as plot_polygons_
import matplotlib.pyplot as plt

fig, ax = plt.subplots()

a = [[126.87150419284516545, 48.422832186031996571], [126.871870681943264, 48.424109732309673859], [126.87407986797624915, 48.430938144298060877], [126.78788150639459786, 48.445036169845600682], [126.78467087995912266, 48.436537288181924055]]



plot_polygons = lambda *args: plot_polygons_(*args, annotate=True)

plot_polygons(ax, [a],'red', 0.5)
# plot_polygons(ax, [b],'green', 0.5)
# plot_polygons(ax, [c],'blue', 0.5)

plt.show()
