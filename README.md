# Benchmark for (file-based) Spatial Index

This benchmark focus on the balance of insertion time, query time and storage cost of spatial index libraries (C/C++)

# Result on Ubuntu20.04 (WSL2)
## libspatialindex
```
===== Insertion test with 100 boxes =====
Insertion time: 0.1 ms/box
Current file size: 81920
===== Query test with 100 boxes =====
Query time: 0.02 ms/box
Query checksum: 140726214417592
===== Insertion test with 1000 boxes =====
Insertion time: 0.136 ms/box
Current file size: 311296
===== Query test with 1000 boxes =====
Query time: 0.183 ms/box
Query checksum: 140726214902577
===== Insertion test with 10000 boxes =====
Insertion time: 0.5618 ms/box
Current file size: 2670592
===== Query test with 10000 boxes =====
Query time: 0.2852 ms/box
Query checksum: 140726264405949
===== Insertion test with 100000 boxes =====
Insertion time: 0.8756 ms/box
Current file size: 25821184
Benchmark End.
File size after flush: 25859028
```
## SQLite3 with Rtree* extension
```
===== Insertion test with 100 boxes =====
Insertion time: 0.19 ms/box
Current file size: 16384
===== Query test with 100 boxes =====
Query time: 0.66 ms/box
Query sum: 140725633604892
===== Insertion test with 1000 boxes =====
Insertion time: 0.03 ms/box
Current file size: 16384
===== Query test with 1000 boxes =====
Query time: 0.634 ms/box
Query sum: 140725634090670
===== Insertion test with 10000 boxes =====
Insertion time: 0.0301 ms/box
Current file size: 16384
===== Query test with 10000 boxes =====
Query time: 0.6142 ms/box
Query sum: 140725683581960
===== Insertion test with 100000 boxes =====
Insertion time: 0.44314 ms/box
Current file size: 8536064
End
File size after flush: 8536064
```
