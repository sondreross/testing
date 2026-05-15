# Experiment results

Here is every intermidate experiment run, note that it is quite messy, some explainations are given under. 

[./nixround](./nixround/) is the final results used in the thesis.

## First_real_failedround
Tried running a bubblesort workload between 10 and 13 seconds of work on the DUT. Same array.

## Real_second
I have now changed out the vector from stdlib to a normal malloced array. And the bubblesort itself is a .o file that both plattforms use.

### Differnt versions
Most things are on the default: Intel SpeedStep ON, C states on, TurboBoost ON. On round 1 and 2 the base linux uses on_demand, on schedutil, i change to schedutil.

## Short
This is the second round 


## Third round
Now i have made 4 version:
- unikernel as is
- Unikernel tweaked
- linux as is, this is intel-pstate cpufreq driver with powersave (governor)
- linux tweaked, (everything)
(here i have used the beeps bubblesort on 80 000) with worst case. og egen .o fil, så samme for begge systemene.

## Scaling freq results
In [scaling_freq_results](./scaling_freq_results/) the results for the scaling_freq tool is placed (see [freq_scaling_tool](https://github.com/sondreross/freq-scaling-tool/tree/main)).

## Ryddet
This is near the final results and configurations, but not taking the c standard library into account.

## Nixround
This is the actuall results used for the thesis. configurations are as described in the report.