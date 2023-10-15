FROM debian:latest
RUN DEBIAN_FRONTEND=noninteractive && apt-get update && \
apt-get -y install build-essential cmake git autoconf libtool pkg-config

RUN git clone --recurse-submodules -b v1.58.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc && \
cd grpc && \
mkdir -p cmake/build &&\
cd cmake/build && \
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
       ../.. && \
make -j$(nproc) && make install && cd / && rm -rf grpc