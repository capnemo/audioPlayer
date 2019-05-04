CC=clang++ -std=c++11 -ggdb
libs=-lswresample -lavutil -lavformat -lavcodec -lswresample -lpthread
decSrcs=decodeTest.cpp ../src/reader.cpp ../src/streamInit.cpp ../src/lockedQ.cpp

all:qTest decodeTest
qTest:qTest.cpp ../src/lockedQ.cpp 
	$(CC) -I../incl/ $? -o $@ -lpthread

decodeTest:$(decSrcs)
	$(CC) -I../incl/ $(decSrcs) -o $@ $(libs)
 	
clean:
	rm -f qTest decodeTest *.o
