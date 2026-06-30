FROM archlinux:latest AS builder

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm \
    base-devel \
    cmake \
    boost \
    openssl \
    sqlite \
    git \
    && pacman -Scc --noconfirm

WORKDIR /app

COPY CMakeLists.txt .
COPY include/ include/
COPY server/ server/

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CLIENT=OFF \
    -DBUILD_TESTING_TARGET=OFF \
    && cmake --build build --target server -j$(nproc)

FROM archlinux:latest AS runtime

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm \
    boost-libs \
    openssl \
    sqlite \
    && pacman -Scc --noconfirm

RUN useradd -m -u 1001 messenger

WORKDIR /app

COPY --from=builder /app/build/server/server .

RUN mkdir -p /app/data && chown -R messenger:messenger /app

USER messenger

EXPOSE 12345

CMD ["./server"]