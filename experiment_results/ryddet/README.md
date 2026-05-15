
base_unikernel
- Vanlig base
- (ikke print underveis)

optimized_unikernel
- Alt av i BIOS
- (ikke print underveis)

base_linux
- Vanlig base
- (print underveis)

optimized_linux_wo_iso_eist_c-state_off
- Alt av BIOS-innstillinger (alle 4 cores)
- Alle cores i performance
- Ikke noen kernel-params
- Kjører med `nice taskset -c 3`
- (ikke print underveis)

optimized_linux_iso_eist_c-state_off
- Alt av BIOS-innstillinger (alle 4 cores)
- Alle cores i performance
- Isolert med fullhz og nocb i kernel-params, og i runtime med systemd
- (ikke print underveis)


Naive-linux
- Som base, bare uten temperatur-venting og oppvarming i starten. (her også uten tastatur og video)

(additional experiments)
optimized_linux_wo_iso_eist_c-state_off
- Samme som over, men EIST og C-states på
- (print underveis)






