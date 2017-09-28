import numpy as np
import json
import pandas as pd
from aoi_generator import generate_aoi, show_polygons
import matplotlib.pyplot as plt
import os

results = {}


def save_polygon(tag, polygon):
    fig, ax = plt.subplots()
    show_polygons(ax, polygon)
    plt.savefig('../../fig/{}_aois.png'.format(tag))


def extract_result(path):
    j = json.load(open(path, 'r'))
    average_t1 = np.mean([result['timestamp']['t1'] for result in j])
    average_t2 = np.mean([result['timestamp']['t2'] for result in j])

    average_price = np.mean([result['totalPrice'] / result['coverageArea']
                             for result in j if result['coverageArea'] > 0])

    average_count = np.mean([result['scenesCount'] for result in j])

    scenes_count = len(j)

    return {'t1': average_t1, 't2': average_t2, 'price': average_price, 'count': average_count}


def generate_aoi_file(path, aois, delta):
    df = pd.DataFrame([str([list(v) for v in aoi]) for aoi in aois])
    df.columns = ['Polygon']
    df['Delta'] = delta

    df.to_csv(path, index=None)


def run_expt(tag, aois, delta, scenes):
    print('runing {}'.format(tag))
    ret = {}

    # read
    scenes_path = '/data/input/csv/{}.csv'.format(scenes)
    bin_path = '/bin/expt'

    # write
    aoi_path = '/data/tmp/aoi_{}.csv'.format(tag)

    # write
    target_path = '/data/output/result_{}.json'.format(tag)

    # add root
    bin_path, scenes_path, aoi_path, target_path = map(
        lambda s: '../../' + s, (bin_path, scenes_path, aoi_path, target_path))

    generate_aoi_file(aoi_path, aois, delta)

    os.system('{} -a {} -s {} -o {}'.format(bin_path, aoi_path,
                                            scenes_path, target_path))

    ret['tag'] = tag
    ret['result'] = extract_result(target_path)

    print('finished {}'.format(tag))

    return ret


def var_delta(aois):
    ret = []
    for delta in [0.0125, 0.025, 0.0375, 0.05, 0.075]:
        tag = 'var_delta_{:.4f}'.format(delta)
        ret.append(run_expt(tag, aois, delta, 15000))
    return ret


def var_scenes(aois):
    ret = []
    for n in [5000, 10000, 15000, 20000, 50000]:
        tag = 'var_number-of-scenes_{}'.format(n)
        ret.append(run_expt(tag, aois, 0.025, n))
    return ret


def var_size():
    ret = []
    for length in [0.1, 0.25, 0.5, 0.75, 1.0]:
        size = length * length
        aois = generate_aoi(AOI_SAMPES, size)
        tag = 'var_size_{:.2f}'.format(size)
        save_polygon(tag, aois)
        ret.append(run_expt(tag, aois, 0.025, 15000))
    return ret


AOI_SAMPES = 50

def main():
    # 50 aois, with size 1
    aois = generate_aoi(AOI_SAMPES, 0.25)
    save_polygon('general', aois)

    ret = []

    ret += var_delta(aois)
    ret += var_scenes(aois)
    ret += var_size()

    json.dump(ret, open('../../data/output/results.json', 'w'))


if __name__ == '__main__':
    main()
