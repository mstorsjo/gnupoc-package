# Place the following files in the current directory
# and run "docker build .":
#  gcc-539-2aeh-source.tar.bz2
#  gnu-csl-arm-2005Q1C-arm-none-symbianelf.src.tar.bz2
#  S60_SDK_v21c_CW.zip
#  S60_SDK_0616_3.0_mr.3.749.zip

# This works on (at least) debian:wheezy, ubuntu:12.04
FROM ubuntu:12.04

RUN apt-get update && apt-get install -y build-essential zlib1g-dev flex libncurses-dev unzip lib32z1 cabextract libssl-dev bison

RUN mkdir -p /symbian-build/gnupoc-package

COPY sdks /symbian-build/gnupoc-package/sdks/
COPY tools /symbian-build/gnupoc-package/tools/
COPY test /symbian-build/gnupoc-package/test/

COPY gcc-539-2aeh-source.tar.bz2 S60_SDK_v21c_CW.zip /symbian-build/
RUN cd /symbian-build/gnupoc-package/test && ./test_sdk ../.. s60_21_cw /symbian-test -j4

COPY gnu-csl-arm-2005Q1C-arm-none-symbianelf.src.tar.bz2 S60_SDK_0616_3.0_mr.3.749.zip /symbian-build/
RUN cd /symbian-build/gnupoc-package/test && ./test_sdk ../.. s60_30 /symbian-test -j4

