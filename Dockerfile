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
LABEL org.opencontainers.image.description="Docker image with a pre-built, tested and packaged community maintained htm.core library"
LABEL org.opencontainers.image.source="https://github.com/htm-community/htm.core"

RUN echo "Switching from $host to $arch" && uname -a

## Stage 1: build of htm.core on the target platform
# Official Alpine Linux (amd64, arm64, etc).
#  https://hub.docker.com/_/alpine
FROM --platform=linux/${arch} alpine:3.19 as build
ARG arch
#copy value of ARG arch from above 
RUN echo "Building HTM for ${arch}" && uname -a


ADD . /usr/local/src/htm.core
WORKDIR /usr/local/src/htm.core

# Install build dependencies
RUN apk add --no-cache \
  cmake \
  make \
  g++ \
  git \
  python3 \
  python3-dev \
  py3-pip \
  py3-numpy

# Create python symlink
RUN ln -s /usr/bin/python3 /usr/local/bin/python && python --version

# Set environment variable to indicate we're in Docker (for htm_install.py)
ENV DOCKER_CONTAINER=1

# Remove PEP 668 marker to allow pip installs (safe in Docker)
RUN rm -f /usr/lib/python*/EXTERNALLY-MANAGED

# Upgrade pip
RUN python -m pip install --upgrade pip setuptools wheel

# Run the htm_install.py script to build and extract build artifacts
RUN python htm_install.py



