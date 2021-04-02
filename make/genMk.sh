#!/bin/bash
#Shell script to generate .cpp targets and dependencies.
#Output to be added to the Makefile.
#To be used whenever there is a change to the dependencies.
#echo "CC=g++ -ggdb -Iincl -std=c++11 -Wall"
cd ..
cat make/header.txt > Makefile
for i in src/*.cpp
do
    stub=$(clang++ -std=c++11 -Iincl -MM $i | sed 's/\\//')
    echo src/$stub >> Makefile
    echo "	\$(CC) -c $i -o \$@" >> Makefile
done
cat make/footer.txt >> Makefile
