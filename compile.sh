docker start tp-so
docker exec -it tp-so make clean -C /root/Toolchain
docker exec -it tp-so make clean -C /root/
docker exec -it tp-so make -C /root/Toolchain
docker exec -it tp-so make -C /root/
