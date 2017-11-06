import json
from utils.path import data_dir
from utils.plot import plot_polygons, plt
from matplotlib.animation import FuncAnimation
import io
from PIL import Image
from utils.images2gif import writeGif

def load_vinfo(n):
    discrete = json.load(
        open(data_dir(['visualize', 'vinfo', 'd{}_vinfo.json'.format(n)])))
    continuous = json.load(
        open(data_dir(['visualize', 'vinfo', 'c{}_vinfo.json'.format(n)])))
    return discrete, continuous


def parse_cell(cid):
    def parse_cell_helper(cid, delta):
        xi = cid & ((1 << 32) - 1)
        yi = cid >> 32
        x = xi * delta
        y = yi * delta
        return [[x, y], [x + delta, y], [x + delta, y + delta], [x, y + delta]]
    return parse_cell_helper(cid, 0.02)


def plot_model(ax, model, colors=['none', 'none', 'none'], alphas=[0.5, 0.5, 0.5]):
    poly = eval(model['polygon'])
    cells = [parse_cell(cell) for cell in model['cells'] or []]
    offcuts = [eval(offcut) for offcut in model['offcuts'] or []]

    if alphas[0] > 0:
        plot_polygons(ax, [poly], colors[0], alphas[0])
    if alphas[1] > 0:
        plot_polygons(ax, cells, colors[1], alphas[1])
    if alphas[2] > 0:
        plot_polygons(ax, offcuts, colors[2], alphas[2])

    plt.xlim(126.1, 126.7)
    plt.ylim(46.1, 46.7)


def draw_frame(roi, selected, unselected):
    fig, ax = plt.subplots(figsize=(10, 10))
    plot_model(ax, roi, alphas=[1, 1, 0])
    plot_model(ax, selected[-1], colors=['green'] * 3, alphas=[0.5, 1, 1])
    for s in selected[:-1]:
        plot_model(ax, s, ['green'] * 3, alphas=[0.0, 0.3, 0.3])
    for us in unselected:
        plot_model(ax, us, ['yellow'] * 3, alphas=[0.0, 1, 1])
    buf = io.BytesIO()
    plt.savefig(buf, format='png')
    buf.seek(0)
    im = Image.open(buf)
    return im

def visualize(vinfo, tag):
    roi = vinfo['roi']
    selected = []
    unselected = []
    sorted_vinfo = sorted(vinfo['frames'].items(), key=lambda kv: float(kv[0]))
    frame_no = 0
    frames = []
    for timestamp, frame in sorted_vinfo:
        selected.append(frame['selected'])
        unselected = frame['unselected'] or []
        frames.append(draw_frame(roi, selected, unselected))
        frame_no += 1
        print(frame_no, 'frame')
    writeGif(data_dir(['visualize', '{}.gif'.format(tag)]), frames, duration=0.7, dither=0)

discrete, continuous = load_vinfo(0)
visualize(continuous, 'c')
visualize(discrete, 'd')
