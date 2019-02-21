
CC=clang++ -fstack-protector -ggdb -std=c++11 -Wall
objs=streamInit.o playAudio.o reader.o audioPlayer.o lockedQ.o
srcs=audioPlayer.cpp lockedQ.cpp playAudio.cpp reader.cpp streamInit.cpp 
snd_lib=/work/alsa/alsa-lib-1.1.6/src/.libs/libasound.a
#libs=-lavutil -lavformat -lavcodec -lpthread -lswresample $(snd_lib) -ldl
libs=-lavutil -lavformat -lavcodec -lpthread -lswresample -lasound

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
