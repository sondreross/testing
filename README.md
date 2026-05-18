# Testing
This repository contains experiment code, results, and data analysis. The actual results from this thesis are in [./experiment_results/nixround/](./experiment_results/nixround/).

## Overview
The benchmarks are in [benchmarks](./benchmarks/) directory.

The measurement code used in the experiments is in [unikernel](./unikernel/) and [linux](./linux/).

The experiment results are in [experiment_results](./experiment_results/).

[power_monitor.cpp](./power_monitor.cpp) is the program run on the external machine that communicates with the power monitor and DUT.

[setup_isolation.sh](./setup_isolation.sh) is the script referenced in the thesis and used for part of the Linux ISO configuration.

