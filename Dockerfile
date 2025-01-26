## Default arch. Pass in like "--build-arg arch=arm64".
#  Supports Debian arches: amd64, arm64, etc.
#  Our circleci arm64 build uses this specifically.
#  https://docs.docker.com/engine/reference/commandline/build/
## To run a build using this file locally, do: 
# docker run --privileged --rm -it multiarch/qemu-user-static:register
# docker build -t htm-arm64-docker --build-arg arch=arm64 .
# docker run -it htm-arm64-docker

#target compile arch
ARG arch=arm64
#host HW arch
ARG host=amd64

## Stage 0: deboostrap: setup cross-compile env 
FROM multiarch/qemu-user-static as bootstrap
ARG arch
ARG host
RUN echo "Switching from $host to $arch" && uname -a

## Stage 1: build of htm.core on the target platform
# Multiarch Debian 10 Buster (amd64, arm64, etc).
#  https://hub.docker.com/r/multiarch/debian-debootstrap
FROM multiarch/alpine:${arch}-latest-stable as build
ARG arch
#copy value of ARG arch from above 
RUN echo "Building HTM for ${arch}" && uname -a


ADD . /usr/local/src/htm.core
WORKDIR /usr/local/src/htm.core




# Run the htm_install.py script to build and extract build artifacts
RUN python htm_install.py



