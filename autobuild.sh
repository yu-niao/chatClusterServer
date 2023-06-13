
set -x

rm -rf `pwd`/bin/* `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make 
