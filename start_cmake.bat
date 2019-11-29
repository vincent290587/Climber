

mkdir TDDV
:: if SystemView is used, the build must be 32 bits
cmake -G "Visual Studio 16 2019" -S . -B TDDV

mkdir TDDW
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug -S . -B TDDW

