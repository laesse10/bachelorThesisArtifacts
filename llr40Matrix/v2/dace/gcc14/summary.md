# DaCe against translated C++ with GCC 13 and with GCC 14.2

| build | DaCe over C++ (geomean, 95% CI) | kernels differing by > 3% |
|---|---|---:|
| GCC 13.3 (job 4992604) | 1.44 [1.10, 1.89] | 23 of 40 |
| GCC 14.2 (job 5121174) | 1.47 [1.12, 1.93] | 25 of 40 |

Kernels whose C++ or DaCe time moved by more than 10% (ms, median):

| kernel | C++ 13 | C++ 14 | DaCe 13 | DaCe 14 | ratio 13 | ratio 14 |
|---|---:|---:|---:|---:|---:|---:|
| `tsvc_2_s1232` | 43.2 | 47.7 | 7.9 | 7.9 | 5.45 | 6.03 |
| `tsvc_2_s2275` | 854.0 | 952.8 | 74.3 | 74.5 | 11.49 | 12.79 |
| `tsvc_2_s275` | 436.2 | 461.6 | 169.1 | 133.1 | 2.58 | 3.47 |
| `tsvc_2_s3110` | 103.6 | 143.7 | 101.4 | 104.0 | 1.02 | 1.38 |
| `tsvc_2_s316` | 79.6 | 281.5 | 79.5 | 281.7 | 1.00 | 1.00 |

Code-generator configurations (ms, median; GCC 13 / GCC 14.2):

| kernel | pipeline | C++ | old | legacy | new |
|---|---|---:|---:|---:|---:|
| `tsvc_2_s3111` | canon | 123.8 / 123.5 | 1519.1 / 1541.7 | 122.0 / 123.6 | 122.0 / 123.7 |
| `tsvc_2_s319` | canon | 68.5 / 69.5 | 408.4 / 408.2 | 69.2 / 69.5 | 67.6 / 67.4 |
| `tsvc_2_s313` | canon | 82.2 / 83.7 | 1005.7 / 1040.7 | 81.5 / 83.6 | 82.1 / 83.7 |
| `tsvc_2_s313` | none | 82.2 / 83.7 | 395.7 / 410.0 | 81.8 / 83.7 | 81.8 / 83.9 |
| `tsvc_2_vdotr` | canon | 55.5 / 54.8 | 674.1 / 678.6 | 54.4 / 55.2 | 54.4 / 54.9 |
| `tsvc_2_vdotr` | none | 55.5 / 54.8 | 264.9 / 266.9 | 54.5 / 54.8 | 54.3 / 55.0 |

Compiler stamps of the 232 libraries built in job 5121174: ['[     0]  GCC: (SUSE Linux) 14.2.0'].
