from utils.path import data_dir
from utils.expt_tools import run_expt, get_tag
from utils.path import data_dir
from utils.plot import plot_query_result, plot_directly

def query_dir(l): 
    return data_dir(['experiment', 'results', 'query'] + l)

def main():
    config = {
        'delta': [0.02],
        'aoi_size': [0.1],
        'n_aois': [1],
        'archive': [1000],
    }

    run_expt(config)
    
    query_result_path = query_dir(['{}.json'.format(get_tag({k: v[0] for k, v in config.items()}))])

    plot_directly(plot_query_result, query_result_path)

if __name__ == '__main__':
    main()
