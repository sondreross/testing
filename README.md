## First round
Tried running 40 runs with a bubblesort between 10 and 13 seconds of work on the DUT. Same array, results in energy_comparisons. Tried to get the same compile options, dont know i did.

## Second round
I have now changed out the vector from stdlib to a normal malloced array. And the bubblesort itself is a .o file that both plattforms use.

### Differnt versions
Most things are on the default: Intel SpeedStep ON, C states on, TurboBoost ON. On round 1 and 2 the base linux uses on_demand, on schedutil, i change to schedutil.

## Third round
Now i have made 4 version:
- unikernel as is
- Unikernel tweaked
- linux as is, this is intel-pstate cpufreq driver with powersave (governor)
- linux tweaked, (everything)
(here i have used the beeps bubblesort on 80 000) with worst case. og egen .o fil, så samme for begge systemene.

