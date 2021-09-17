FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y \
    build-essential \
    cmake\
    ninja-build \
    wget \
    python3 \
    git

RUN wget https://ziglang.org/download/0.8.1/zig-linux-x86_64-0.8.1.tar.xz && tar -C /usr/local -xf zig-linux-x86_64-0.8.1.tar.xz
RUN wget https://golang.org/dl/go1.17.1.linux-amd64.tar.gz && tar -C /usr/local -xzf go1.17.1.linux-amd64.tar.gz
RUN wget https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-12.0.1.tar.gz && tar -xf llvmorg-12.0.1.tar.gz && mv llvm-project-llvmorg-12.0.1 /llvm-project

ENV PATH=$PATH:/usr/local/go/bin:/usr/local/zig-linux-x86_64-0.8.1
ENV CC="zig cc -fno-sanitize=all -target x86_64-linux-musl"
ENV CXX="zig c++ -fno-sanitize=all -target x86_64-linux-musl"

RUN mkdir -p /llvm-project/build && cd /llvm-project/build && cmake -G Ninja \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD=BPF \
  -DCMAKE_INSTALL_PREFIX="/llvm-project/out" \
  -DCMAKE_PREFIX_PATH="/llvm-project/out" \
  -DLLVM_ENABLE_LIBXML2=OFF \
  -DLLVM_ENABLE_TERMINFO=OFF \
  -DLLVM_ENABLE_BACKTRACES=OFF \
  -DLLVM_ENABLE_PLUGINS=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CROSSCOMPILING=True \
  -DLLVM_INCLUDE_UTILS=OFF \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_GO_TESTS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_ENABLE_BINDINGS=OFF \
  -DLLVM_ENABLE_OCAMLDOC=OFF \
  -DLLVM_ENABLE_Z3_SOLVER=OFF \
  -DCLANG_BUILD_TOOLS=OFF \
  -DCLANG_ENABLE_ARCMT=ON \
  -DLLVM_BUILD_STATIC=ON \
  -DLIBCLANG_BUILD_STATIC=ON \
  -DLLVM_DEFAULT_TARGET_TRIPLE="bpfel-none-none" \
  ../llvm && ninja install

CMD ["/work/entrypoint.sh"]
