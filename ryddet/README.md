base_unikernel
- Vanlig base 
- (ikke print underveis)

optimized_unikernel
- Alt av i bios
- (ikke print underveis)

base_linux
- Vanlig base
- (print underveis)

optimized_linux_wo_iso_eist_c-state_off
- alt av i bios innstillinger av (alle 4 cores)
- alle cores i performance
- Ikke noe kernel params
- Kjører med nice taskset -c 3
- (ikke print underveis)

optimized_linux_iso_eist_c-state_off
- alt av i bios innstillinger av (alle 4 cores)
- alle cores i performance
- Isolert både med fullhz, and nocb in kernelparams, and in "runtime" with systemds
- (ikke print underveis)



Naive-linux
- Som base, bare uten tempratur venting og oppvarming i starten. (her også uten tastatur og video)

(additional experiments)
optimized_linux_wo_iso_eist_c-state_off
- Samme som ^ bare 
- EIST og C-states på
- (print underveis)



