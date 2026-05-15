# Linux setup

To build the final Linux binary, run:

```bash
nix-build static-linux-musl-includeos-rev.nix
```

This is intended to run on Linux, sending results and START/STOP signals over the serial port.