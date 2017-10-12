1. 5/10/2017, use boost::geometry, change cell id to integer.

![](1/t-delta.png)
![](1/t-aoi_size.png)

---

2. 12/10/2017, based on experiment 1, give up boost::geometry, update cell id sets by removing covered cells from it after each greedy selection.

![](2/t-delta.png)
![](2/t-aoi_size.png)

Giving up boost:geometry gives almost 5x faster in t1. Removing covered cells from all rest cell id sets after each greedy seletion also make t2 process 2x faster.

