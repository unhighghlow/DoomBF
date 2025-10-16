#! /bin/bash
if [ ! -d ./buildx ]; then
    mkdir ./buildx
fi
if [ ! -d ./bin ]; then
    mkdir ./bin
fi
cd ./buildx
cmake ..
cmake --build .
cd ..