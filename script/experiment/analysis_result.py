import numpy as np
import json
import matplotlib.pyplot as plt

def bar_char(xs, ys, title, ylabel):
    fig, ax = plt.subplots()

    ind = np.arange(len(xs))

    width = 0.2

    ax.bar(ind, ys, width)

    plt.xticks(ind, xs)
    plt.xlabel(title)
    plt.ylabel(ylabel)

    plt.title(title)

    plt.savefig('../../fig/{}_bar_char.png'.format(title))
  

def stacked_bar_char(xs, y1s, y2s, label1, label2, title):
    fig, ax = plt.subplots()

    ind = np.arange(len(xs))

    width = 0.2

    ax.bar(ind, y1s, width, label=label1)
    ax.bar(ind, y2s, width, bottom=y1s, label=label2)

    plt.xticks(ind, xs)
    plt.xlabel(title)
    plt.ylabel('average time(s)')

    plt.legend()

    plt.savefig('../../fig/{}_stacked_bar_char.png'.format(title))

def line_chart(xs, ys, title, ylabel):
    fig, ax = plt.subplots()

    plt.plot(xs, ys, marker='x')

    plt.xlabel(title)
    plt.ylabel(ylabel)

    plt.savefig('../../fig/{}_line_chart.png'.format(title))


def plot_all(results):
    data = {}

    for result in results:
        _, variable, value = result['tag'].split('_')
        r = result['result']
        if variable not in data:
            data[variable] = {}
        data[variable][value] = r;

    for variable in data:
        xs = list(data[variable].keys())
        xs = sorted(xs, key=lambda s: float(s.strip('k')))
        t1s = [data[variable][x]['t1'] for x in xs]
        t2s = [data[variable][x]['t2'] for x in xs]
        stacked_bar_char(xs, t1s, t2s, 't1', 't2', variable)

    for variable in data:
        xs = list(data[variable].keys())
        xs = sorted(xs, key=lambda s: float(s))
        ys = [data[variable][x]['price'] for x in xs]
        line_chart(xs, ys, variable, 'price/size(¥)')

    for variable in data:
        xs = list(data[variable].keys())
        xs = sorted(xs, key=lambda s: float(s))
        ys = [data[variable][x]['count'] for x in xs]
        bar_char(xs, ys, variable, 'count')

def main():
    plot_all(results)

if __name__ == '__main__':
    main()