FROM ubuntu:18.04

LABEL maintainer "jechkoj@gmail.com"

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    curl \
    cmake \
    gcc-7 g++-7 \
    gdb \
    git \
    netcat \
    python3 python3-pip python3-distutils python3-setuptools \
    vim && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

WORKDIR /root

# Download and unarchive Intel Pin 3.6
RUN PIN_URL=https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.6-97554-g31f0a167d-gcc-linux.tar.gz && \
    curl -fSL -o pin.tar.gz $PIN_URL && \
    mkdir -p /usr/local/pin && \
    tar -xzf pin.tar.gz --directory /usr/local/pin --strip-components=1 && \
    rm -r pin.tar.gz

# Set Pin variables
ENV PIN_ROOT="/usr/local/pin"
ENV PATH=$PIN_ROOT:$PATH

# Install python3 ptrace and keystone
RUN pip3 install --upgrade pip
RUN pip3 install --upgrade python-ptrace \
                           keystone-engine

# Copy files
COPY jitmenot /root/jitmenot
COPY sandbox /root/sandbox
COPY pwin /root/pwin
COPY shadow /root/shadow

# Setup example Pintool
RUN cp -R /usr/local/pin/source/tools/MyPinTool /root/inst-count && \
    mv /root/inst-count/MyPinTool.cpp /root/inst-count/InstCountPinTool.cpp && \
    chmod 644 /root/inst-count/InstCountPinTool.cpp && \
    sed -i '20s/.*/TEST_TOOL_ROOTS := InstCountPinTool/' /root/inst-count/makefile.rules

RUN make -C /root/jitmenot && \
    make -C /root/sandbox && \
    make -C /root/shadow && \
    make -C /root/inst-count && \
    gcc -Wl,-z,relro,-z,now -fPIC -pie -fpie -D_FORTIFY_SOURCE=2 -O3 -o /root/sandbox/escape /root/sandbox/escape.c && \
    gcc -Wl,-z,relro,-z,now -fPIC -pie -fpie -D_FORTIFY_SOURCE=2 -o /root/pwin/shell /root/pwin/shell.c
