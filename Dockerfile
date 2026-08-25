FROM ubuntu:24.04 AS builder
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        git \
        clang-format \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build
RUN ctest --test-dir build --output-on-failure
RUN cmake --install build --prefix /app/install

FROM ubuntu:24.04 AS runtime
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libstdc++6 && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /app/install/ /app/
CMD ["./bin/TelecomSimulator"]