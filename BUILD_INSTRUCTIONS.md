# Building with Shared bubblesort.o

This setup ensures both Linux and Unikernel use the **exact same** pre-compiled bubblesort object file.

## Build Steps

### 1. Compile bubblesort.o (do this first)
```bash
cd /Users/sondrerosslund/git/testing
make -f Makefile.bubblesort
```

This creates `bubblesort.o` with:
- Compiler: `clang`
- Flags: `-O3 -DNDEBUG -fPIC -c`

### 2. Build Linux binary
```bash
cd linux
make
```

This links the pre-compiled `../bubblesort.o` into the Linux benchmark.

### 3. Build Unikernel binary
```bash
cd unikernel
# Use your IncludeOS build process
```

This also links the same pre-compiled `../bubblesort.o` into the Unikernel.

## Important Notes

- **bubblesort.o must be compiled FIRST** before building either project
- Both Linux and Unikernel will use the identical bubblesort binary code
- Any performance differences are purely from OS/runtime environment, not code generation
- To rebuild bubblesort.o: `make -f Makefile.bubblesort clean && make -f Makefile.bubblesort`
