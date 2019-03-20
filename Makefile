
CC=clang++ -fstack-protector -ggdb -std=c++11 -Wall
objs=streamInit.o playAudio.o reader.o audioPlayer.o xPlot.o lockedQ.o audioResampler.o
srcs=audioPlayer.cpp lockedQ.cpp playAudio.cpp reader.cpp streamInit.cpp xPlot.cpp audioResampler.cpp
libs=-lavutil -lavformat -lavcodec -lpthread /work/ffmpeg/ffmpeg-4.1.1/libswresample/libswresample.a -lasound -lsoxr -lX11 -lm
#libs= -lswresample -lavutil -lavformat -lavcodec -lswresample -lpthread -lasound -lX11 -lm

all:playAudio

dep:$(srcs)
	clang++ -std=c++11 -MM $(srcs) > dep

-include dep

%.o:%.cpp
	$(CC) -c $*.cpp  -o $*.o

playAudio:$(objs)
	$(CC) $(objs) $(libs) -o playAudio

clean:
	rm -f playAudio $(objs) dep
