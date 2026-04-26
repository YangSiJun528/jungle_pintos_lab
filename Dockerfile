FROM ubuntu:22.04

ENV TZ=Asia/Seoul \
    LANG=ko_KR.UTF-8 \
    LANGUAGE=ko_KR.UTF-8

RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y \
        locales \
        tzdata \
        build-essential \
        gcc \
        gdb \
        vim \
        git \
        sudo \
        qemu-system-x86 \
        python3 && \
    locale-gen ko_KR.UTF-8 && \
    update-locale LANG=ko_KR.UTF-8

RUN useradd -m -s /bin/bash jungle && \
    usermod -aG sudo jungle && \
    echo "jungle ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/jungle

USER jungle
WORKDIR /workspace

RUN echo 'if [ -f /workspace/pintos/activate ]; then source /workspace/pintos/activate; fi' >> /home/jungle/.bashrc
