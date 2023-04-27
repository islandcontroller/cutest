#syntax=docker/dockerfile:1

#--[ Build/Deploy Runner ]------------------------------------------------------
# Base uses an x86 alpine container. The x86 part is essential, since all tar-
# geted MCUs are using 32-bit architectures
FROM i386/alpine:latest AS base-runner

# Minimal build environment dependencies
RUN apk add --update --no-cache \
    autoconf \
    build-base

# Use /var/cutest as working directory
WORKDIR /var/cutest


#--[ Build stage ]--------------------------------------------------------------
# Uses the previously defined build/deploy runner
FROM base-runner AS builder
ARG CUTEST_LIB_VERSION
ENV CUTEST_LIB_VERSION ${CUTEST_LIB_VERSION}

# Copy CuTest sources into workdir
COPY src/* ./

# Build libcutest binaries
RUN make all


#--[ Deploy stage ]-------------------------------------------------------------
# Uses the previously defined build/deploy runner
FROM base-runner as cutest

# Copy CuTest headers and built library
COPY --from=builder /var/cutest/libcutest.a /usr/lib/
COPY --from=builder /var/cutest/CuTest.h /usr/include/
