{ srcFile ? ./bench_linux.cpp }:

let
  nixpkgs = builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/refs/tags/25.05.tar.gz";
    sha256 = "1915r28xc4znrh2vf4rrjnxldw2imysz819gzhk9qlrkqanmfsxd";
  };

  pkgs = import nixpkgs { config = {}; };
  ps = pkgs.pkgsStatic;
  linuxHeaders = pkgs.linuxHeaders;

  llvmPkgs = ps.llvmPackages_18;

  muslUnpatched = ps.stdenv.mkDerivation rec {
    pname = "musl-unpatched";
    version = "1.2.5";

    src = builtins.fetchGit {
      url = "git://git.musl-libc.org/musl";
      rev = "0784374d561435f7c787a555aeab8ede699ed298";
    };

    enableParallelBuilding = true;

    configurePhase = ''
      echo "Configuring with musl's configure script"
      echo "Target platform is ${ps.stdenv.targetPlatform.config}"
      ./configure --prefix=$out --with-malloc=oldmalloc --disable-shared --enable-debug CROSS_COMPILE=${ps.stdenv.targetPlatform.config}-
    '';

    # Copy linux headers - taken from upstream nixpkgs musl, needed for libcxx to build
    postInstall = ''
      (cd $out/include && ln -s $(ls -d ${linuxHeaders}/include/* | grep -v "scsi$") .)
    '';

    CFLAGS = "-Wno-error=int-conversion -nostdinc";

    passthru.linuxHeaders = linuxHeaders;
  };

  clangMuslNoLibcxx = llvmPkgs.clangNoLibcxx.override (_old: {
    bintools = ps.bintools.override {
      defaultHardeningFlags = [];
      libc = muslUnpatched;
    };
    libc = muslUnpatched;
  });

  # Mirror IncludeOS layering:
  # 1) build libc++ with clang + musl-unpatched
  # 2) use a final clang configured with that libc++ and musl-unpatched as libc
  libcxxMuslUnpatched = llvmPkgs.libcxx.override (_old: {
    stdenv = ps.overrideCC llvmPkgs.libcxxStdenv clangMuslNoLibcxx;
  });

  clangMuslLibcxx = llvmPkgs.libcxxClang.override (_old: {
    bintools = ps.bintools.override {
      defaultHardeningFlags = [];
      libc = muslUnpatched;
    };
    libc = muslUnpatched;
    libcxx = libcxxMuslUnpatched;
  });

  stdenvMuslClang = ps.overrideCC llvmPkgs.libcxxStdenv clangMuslLibcxx;

  projectRoot = ../.;
  arch = "x86_64";
in
stdenvMuslClang.mkDerivation {
  pname = "bench-linux-static";
  version = "1.0";
  dontUnpack = true;

  buildPhase = ''
    # Use the stdenv-provided C++ compiler to stay on the clang/libc++ toolchain.
    $CXX -O3 -DNDEBUG -g \
      -rtlib=compiler-rt \
      -nostdinc++ -isystem ${libcxxMuslUnpatched.dev}/include/c++/v1 \
      -fstack-protector-strong -fno-omit-frame-pointer \
      -fno-threadsafe-statics -ffunction-sections -fdata-sections \
      -Wall -Wextra -Wno-frame-address -static \
      -I${projectRoot}/linux \
      ${srcFile} \
      ${../benchmarks/benchmarks-build/crc_32.o} \
      ${../benchmarks/benchmarks-build/libcubic.o} \
      ${../benchmarks/benchmarks-build/basicmath_small.o} \
      ${../benchmarks/benchmarks-build/dijkstra_small.o} \
      ${../benchmarks/benchmarks-build/libfdct.o} \
      ${../benchmarks/benchmarks-build/libfir.o} \
      ${../benchmarks/benchmarks-build/matmult_float.o} \
      ${../benchmarks/benchmarks-build/matmult_int.o} \
      ${../benchmarks/benchmarks-build/nettle-sha256.o} \
      ${../benchmarks/benchmarks-build/aesxam.o} \
      ${../benchmarks/benchmarks-build/aes.o} \
        ${libcxxMuslUnpatched}/lib/libc++.a \
        ${llvmPkgs.libraries.libunwind}/lib/libunwind.a \
        ${llvmPkgs.compiler-rt}/lib/linux/libclang_rt.builtins-${arch}.a \
      -lm -o bench_linux
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp bench_linux $out/bin/bench_linux
  '';

  passthru = {
    muslVersion = "1.2.5";
    muslRev = "0784374d561435f7c787a555aeab8ede699ed298";
    compiler = "clang/libc++ (pkgsStatic stdenv)";
  };
}
