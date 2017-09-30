import os
from functools import reduce

def apply(fs, a):
    return reduce(lambda acc, f: f(acc), fs, a)

ROOT = apply([os.path.dirname] * 3, os.path.realpath(__file__))

def root_dir(path=[]):
    return os.path.join(*([ROOT] + path))

def data_dir(path=[]):
    return root_dir(['data'] + path)

def fig_dir(path=[]):
    return root_dir(['fig'] + path)

def bin_dir(path=[]):
    return root_dir(['bin'] + path)


if __name__ == '__main__':
    print(root_dir())