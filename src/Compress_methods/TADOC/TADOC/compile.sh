
cd examples
make clean
make

cd ..
cd src/sequiturBasedEngine/
make clean
make
cp sequitur ../../bin/

cd ../../
g++ -O3 -std=c++11 src/getdic.cpp -o bin/getdic
g++ -O3 -std=c++11 src/show_crate.cpp -o bin/showcrate
