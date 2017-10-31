from utils.plot import plot_polygons as plot_polygons_
import matplotlib.pyplot as plt

fig, ax = plt.subplots()


a = [[125.31422421466177752, 42.048886485277520819], [125.31422633739619243, 42.048879076672449173], [125.31426773378700545, 42.048879076678062461]]


plot_polygons = lambda *args: plot_polygons_(*args, annotate=True)

plot_polygons(ax, [a],'red', 0.5)
# # plot_polygons(ax, [b],'green', 0.5)
# # plot_polygons(ax, [c, d],'blue', 0.5)

plt.show()
exit()

def line_intersection(line1, line2):
    xdiff = (line1[0][0] - line1[1][0], line2[0][0] - line2[1][0])
    ydiff = (line1[0][1] - line1[1][1], line2[0][1] - line2[1][1]) #Typo was here

    def det(a, b):
        return a[0] * b[1] - a[1] * b[0]

    div = det(xdiff, ydiff)
    if div == 0:
       raise Exception('lines do not intersect')

    d = (det(*line1), det(*line2))
    x = det(d, xdiff) / div
    y = det(d, ydiff) / div
    return x, y


print(line_intersection(((0.5, 0.5), (1.5, 0.5)), ((1, 0), (1, 2))))

a = [[124.6683432789071162, 43.440406559122031638], [124.67400025748183623, 43.458399619003742487]]
b = [[124.67405930001825709, 43.458389614469581375], [124.67313364049230984, 43.455490310687522992]]
c = [124.66998385051222442, 43.445624698536931874]

c = [c] + [line_intersection(a, b)]

# a = [[126.65121867383328436, 46.482154241115679838], [126.67773039587733308, 46.472721260902972062]]
# b = [[126.65121948460721057, 46.482153964633340593], [126.66811681630474595, 46.476141814975981958]]
# c = [[126.87947800079686544, 46.550098475913578966], line_intersection(a, b)]

plot_polygons = lambda *args: plot_polygons_(*args, annotate=True)

plot_polygons(ax, [a],'red', 0.5)
plot_polygons(ax, [b],'green', 0.5)
plot_polygons(ax, [c],'blue', 0.5)

plt.show()
