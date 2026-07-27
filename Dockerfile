# Build: docker build -t chromaforge .
# Build project: docker run --rm -it -v$(pwd):/project chromaforge bash -c "cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && cmake --build build"
# Run: docker run --rm -it -v$(pwd):/project -v/tmp/.X11-unix:/tmp/.X11-unix -v${XAUTHORITY}:/home/user/.Xauthority:ro -eDISPLAY --network=host chromaforge ./build/ChromaForge

FROM debian:bookworm-slim
LABEL Description="Docker container for building ChromaForge for Linux"

RUN apt-get update && apt-get install --no-install-recommends -y \
    git \
    g++ \
    make \
    cmake \
    pkg-config \
    xauth \
    gdb \
    gdbserver \
    libglfw3-dev \
    libglfw3 \
    libglew-dev \
    libglew2.2 \
    libopenal-dev \
    libluajit-5.1-dev \
    libvorbis-dev \
    libcurl4-openssl-dev \
    libfmt-dev \
    libspdlog-dev \
    ca-certificates \
    wget \
    && rm -rf /var/lib/apt/lists/*

ARG CMAKE_VERSION=3.27.9
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
    chmod +x cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
    ./cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/usr/local && \
    rm cmake-${CMAKE_VERSION}-linux-x86_64.sh

RUN git clone --branch v3.16.0 https://github.com/skypjack/entt.git && \
    cd entt/build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DENTT_INSTALL=on .. && \
    make install && \
    cd ../.. && rm -rf entt

RUN ln -s /usr/lib/x86_64-linux-gnu/libluajit-5.1.a /usr/lib/x86_64-linux-gnu/liblua5.1.a \
    && ln -s /usr/include/luajit-2.1 /usr/include/lua

RUN git clone https://luajit.org/git/luajit.git \
    && cd luajit \
    && make && make install INSTALL_INC=/usr/include/lua \
    && cd .. && rm -rf luajit

ARG USER=user
ARG UID=1000
ARG GID=1000
RUN useradd -m ${USER} --uid=${UID}
USER ${UID}:${GID}

WORKDIR /project
