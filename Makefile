
#CC=clang++ -fsanitize=address -fno-omit-frame-pointer -ggdb -std=c++11 -Wall
#CC=clang++ -fstack-protector -ggdb -std=c++11 -Wall
#CC=clang++ -fsanitize=address -ggdb -std=c++11 -Wall
#CC=clang++ -ggdb -Iincl -std=c++11 -Wall
CC=g++ -ggdb -Iincl -std=c++11 -Wall

objs=src/streamInit.o src/playAudio.o src/reader.o src/audioPlayer.o src/xPlot.o src/lockedQ.o src/audioResampler.o src/conPlot.o
srcs=src/audioPlayer.cpp src/lockedQ.cpp src/playAudio.cpp src/reader.cpp src/streamInit.cpp src/xPlot.cpp src/audioResampler.cpp src/conPlot.cpp
libs=-lswresample -lavutil -lavformat -lavcodec -lpthread -lasound -lX11 -lm

all:playAudio

dep:$(srcs)
	clang++ -std=c++11 -Iincl -MM $(srcs) > dep

-include dep

%.o:%.cpp
	$(CC) -c $*.cpp  -o $*.o

playAudio:$(objs)
	$(CC) $(objs) $(libs) -o playAudio

clean:
	rm -f playAudio $(objs) dep
