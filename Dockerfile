FROM ubuntu:16.04
#LABEL maintainer="Jeremy Lan <air.petrichor@gmail.com>" version="0.1.1" \
#  description="This is UCHAIN-WORLD/UChain image" website="http://uc.org/" \
#  , etc..."

RUN echo 'APT::Install-Recommends 0;' >> /etc/apt/apt.conf.d/01norecommends \
  && echo 'APT::Install-Suggests 0;' >> /etc/apt/apt.conf.d/01norecommends \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y sudo wget curl net-tools ca-certificates unzip

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git-core automake autoconf libtool build-essential pkg-config libtool apt-utils \
    mpi-default-dev libicu-dev libbz2-dev zlib1g-dev openssl libssl-dev libgmp-dev \
  && rm -rf /var/lib/apt/lists/*


RUN cd /tmp && wget https://cmake.org/files/v3.9/cmake-3.9.0-Linux-x86_64.sh \
  && mkdir /opt/cmake && chmod +x /tmp/cmake-3.9.0-Linux-x86_64.sh \
  && sh /tmp/cmake-3.9.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license \
  && ln -s /opt/cmake/bin/cmake /usr/local/bin

COPY ./ /tmp/UChain/

# setup gmp link => without-bignum
ENV IS_TRAVIS_LINUX 1

RUN cd /tmp/UChain && /bin/bash install_dependencies.sh --build-boost --build-upnpc

RUN cd /tmp/UChain \
  && mkdir -p build && cd build && cmake .. && make -j2 && make install \
  && rm -rf /tmp/UChain/build \
  && rm -rf /tmp/UChain/thirdlibrary

# TODO...
# Should has `make test` here

RUN cd /tmp/UChain/utils && /bin/bash setup_uc_conf.sh

VOLUME [~/.UChain]

# P2P Network
EXPOSE 5678
# JSON-RPC CALL
EXPOSE 8707
# Websocket notifcations
EXPOSE 28707

ENTRYPOINT ["/usr/local/bin/ucd"]
