# IncludeOS setup

To run this you must be on the correct version of [IncludeOS](https://github.com/sondreross/IncludeOS/tree/bench_func), use the `devolp.nix`, and compile the `CMakeLists.txt` from that nix-shell environment (described in the thesis). After compiling, run `make_grub_iso.sh` to create a bootable .iso image from the IncludeOS binary.

The mentioned [IncludeOS](https://github.com/sondreross/IncludeOS/tree/bench_func) version is needed because the actual measurements were implemented there.