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
    clang

RUN mkdir /var/albot-cpp
WORKDIR /var/albot-cpp
COPY . .

RUN cmake .
RUN make .

CMD ["albot-cpp"]
