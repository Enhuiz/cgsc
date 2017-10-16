# Exp 1. 5/10/2017
Use boost::geometry, change cell id to integer.

![](1/t-delta.png)
![](1/t-aoi_size.png)


# Exp 2. 12/10/2017
Based on Exp 1, give up boost::geometry, update cell id sets by removing covered cells from it after each greedy selection.

![](2/t-delta.png)
![](2/t-aoi_size.png)

Giving up boost:geometry gives almost 5x faster in t1. Removing covered cells from all rest cell id sets after each greedy seletion also make t2 process 2x faster.

# Exp 3. 14/10/2017
Based on Exp 2, discretize scene by checking only the cells inside the intersection of bounding-box of AOI and scene.

![](3/t-delta.png)
![](3/t-aoi_size.png)

Checking only the cells inside the intersection of bounding-box of AOI and scene makes t1 faster. However, if AOI is large and scene is relatively small, t1 will takes more time to calculate the intersection and the performance will benifit very little from the calculation of intersection.

Note that the calculation of intersection is based on the boost::geometry toolkit. While other polygon checking algorithms are implemented from scratch.

# Exp 4. 16/10/2017
Change the aoi-size variable to aoi-ratio, where aoi-ratio = the aoi-size / archieve region size.

![](4/t-aoi_ratio.png)

0.01, 0.02, 0.05, 0.1, 0.2, 0.5 means 1%, 2%, 5%, 10%, 20%, 50% of the area of archieve region (8 * 8).

