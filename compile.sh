docker start itba-so-tp
docker exec -it itba-so-tp make clean -C /root/Toolchain
docker exec -it itba-so-tp make clean -C /root/
docker exec -it itba-so-tp make -C /root/Toolchain
docker exec -it itba-so-tp make -C /root/
