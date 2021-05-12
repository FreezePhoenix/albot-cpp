# Start from ubuntu as start image
FROM ubuntu:20.10

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    cmake \
    libpoco-dev \
    libpoconet70 \
    libpocofoundation70 \
    libpoconetssl70 \
    nlohmann-json3-dev \
    rapidjson-dev \
    libspdlog-dev \
    libspdlog1 \
    make \
    clang

RUN mkdir -p /var/albot-cpp
# Create a work space inside the container
WORKDIR /var/albot-cpp
# Copy the content outside the container (except .dockerignore files) inside the container
COPY . .

RUN ls > nano.txt

RUN cmake .
RUN cmake --build .

# Define default start app
CMD ["./albot-cpp"]
