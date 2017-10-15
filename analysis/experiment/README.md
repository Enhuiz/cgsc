1. 5/10/2017, use boost::geometry, change cell id to integer.

![](1/t-delta.png)
![](1/t-aoi_size.png)

---

2. 12/10/2017, based on experiment 1, give up boost::geometry, update cell id sets by removing covered cells from it after each greedy selection.

![](2/t-delta.png)
![](2/t-aoi_size.png)

Giving up boost:geometry gives almost 5x faster in t1. Removing covered cells from all rest cell id sets after each greedy seletion also make t2 process 2x faster.

3. 14/10/2017, based on experiment 2, discretize scene by checking only the cells inside the intersection of bounding-box of AOI and scene.

![](3/t-delta.png)
![](3/t-aoi_size.png)


Checking only the cells inside the intersection of bounding-box of AOI and scene makes t1 faster. However, if AOI is large and scene is relatively small, t1 will takes more time to calculate the intersection and the performance will benifit very little from the calculation of intersection.

Note that the calculation of intersection is based on the boost::geometry toolkit. While other polygon checking algorithms are implemented from scratch.