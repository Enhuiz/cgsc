import json
import sys
sys.path.append("../")

def plot_result(path):
    results = json.load(open(path, 'r'))

    result = results[0]

    print(result.keys())
    print(result['aoi'])
    exit()

    aoi = eval(result['aoi']['vertices'])
    
    aoi_grids = [eval(grid['vertices']) for grid in result['aoi']['grids']]
    
    possible_scenes = [eval(scene['vertices']) for scene in result['possible_scenes'] or []]
    
    result_scenes = [eval(scene['vertices']) for scene in result['result_scenes'] or []]
    result_grids = [eval(grid['vertices']) for scene in result['result_scenes'] or [] for grid in scene['grids'] or []]

    fig, ax = plt.subplots()

    show_polygons(ax, aoi_grids, 'white')
    show_polygons(ax, [aoi], 'red', alpha=0.2)
    show_polygons(ax, possible_scenes, 'yellow', alpha=0.4)
    show_polygons(ax, result_scenes, 'green', alpha=1)
    show_polygons(ax, result_grids, 'white', alpha=0.3)

    plt.show()

def main():
    plot_result('../../data/illustration/output/result.json')

if __name__ == '__main__':
    main()