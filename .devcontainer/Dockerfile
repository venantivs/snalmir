FROM debian:trixie-slim

WORKDIR /usr/local/app

EXPOSE 8281 80

COPY . .

RUN apt update && apt upgrade
RUN apt install -y make gcc gdb
