Andre Runde
    - Base Linux
        ○ Alt base, printer underveis, schedutil på alle
    - Optimized Linux (m isol)
        ○ Alle cores på performance.
        ○ Slå av boost
        ○ Slå av boost fra userspace.
        ○ Script + cpuset,  and run, performance alle cores printer underveis
        ○ Med watchdog=0. Og 
    - Optimized linux (uten isol)
        ○ Taskset for å kjøre på kjerne 3
        ○ Med nice -20 prioritet
        ○ "nice -n -20 taskset -c 3 sudo ./bench_linux /dev/ttyS0" aksakt kommando
        ○ Har på c states!
        ○ Har på EIST tror jeg
        ○ Kjørte med nmi_watchdog=0
    - Base unikernel
        ○ Alle instillinger p , printer underveis

    - Ver 2
Venter med å printe til slutten