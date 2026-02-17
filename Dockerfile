FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libpcap-dev \
    git \
    gdb \
    tcpdump \
    wireshark-common \
    iptables \
    && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /workspace

# Copy source code
COPY . /workspace

# Build
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Set capabilities for raw socket
RUN setcap cap_net_raw,cap_net_admin=eip /workspace/build/examples/simple_http_get || true

CMD ["/bin/bash"]
