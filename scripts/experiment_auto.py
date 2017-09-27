import numpy as np
import json
import pandas as pd
from aoi_generator import generate_aoi
import os

def extract_result(tag):
    j = json.load(open("../data/tmp/" + tag + '.json', 'r'))
    average_t1 = sum([result['t1'] for result in j]) / len(j)
    average_t2 = sum([result['t2'] for result in j]) / len(j)
    average_price = sum([result['totalPrice'] for result in j]) / len(j)
    return {'t1': average_t1, 't2': average_t2, 'price': average_price}

def run_expt(n_aois, aoi_size, delta):
    aois = generate_aoi(n_aois, aoi_size)
    df = pd.DataFrame([str([list(v) for v in aoi]) for aoi in aois])
    df.columns = ['Polygon']
    df['Delta'] = delta

    tag = '{}-{}-{}'.format(n_aois, aoi_size, delta)
    df.to_csv('../data/tmp/' + tag + '.csv', index=None)

    os.system("../bin/expt -a ../data/tmp/" + tag +
              ".csv -s ../data/input/scenes_100k.csv -o ../data/tmp/" + tag + ".json")


    return extract_result(tag)

def main():
    print(run_expt(2, 1, 0.1))


if __name__ == '__main__':
    main()
