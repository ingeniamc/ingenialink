cmake -H. -B ../../_build -DCMAKE_INSTALL_PREFIX=../../_install -DWITH_PROT_MCB=ON
cmake --build ../../_build
cmake --build ../../_build --target install
