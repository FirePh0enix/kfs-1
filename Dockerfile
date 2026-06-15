FROM debian:latest

RUN apt-get update && apt-get install -y \
    grub-pc-bin \
    grub-common \
    xorriso \
    mtools \
    libisoburn1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /os

ENTRYPOINT ["/bin/sh", "-c"]

CMD ["mkdir -p isodir/boot/grub && \
cp kernel.elf isodir/boot/ && \
cp grub.cfg isodir/boot/grub/ && \
grub-mkrescue -o /os/kfs.iso isodir && \
rm -rf isodir"]

