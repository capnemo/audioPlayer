#CC=clang++ -fsanitize=address -fno-omit-frame-pointer -ggdb -std=c++11 -Wall
#CC=clang++ -fstack-protector -ggdb -std=c++11 -Wall
#CC=clang++ -fsanitize=address -ggdb -std=c++11 -Wall
#CC=clang++ -ggdb -Iincl -std=c++11 -Wall
CC=clang++ -ggdb -Iincl -std=c++11 -Wall

objs=src/streamInit.o src/playAudio.o src/reader.o src/audioPlayer.o src/xPlot.o src/lockedQ.o src/audioResampler.o src/conPlot.o
srcs=src/audioPlayer.cpp src/lockedQ.cpp src/playAudio.cpp src/reader.cpp src/streamInit.cpp src/xPlot.cpp src/audioResampler.cpp src/conPlot.cpp
libs=-lswresample -lavutil -lavformat -lavcodec -lpthread -lasound -lX11 -lm

all:playAudio
src/audioPlayer.o: src/audioPlayer.cpp incl/audioPlayer.h incl/threadRunner.h incl/lockedQ.h incl/audioResampler.h incl/conPlot.h incl/xPlot.h
	$(CC) -c src/audioPlayer.cpp -o $@
src/audioResampler.o: src/audioResampler.cpp incl/audioResampler.h
	$(CC) -c src/audioResampler.cpp -o $@
src/conPlot.o: src/conPlot.cpp incl/conPlot.h incl/audioResampler.h incl/xPlot.h
	$(CC) -c src/conPlot.cpp -o $@
src/lockedQ.o: src/lockedQ.cpp incl/lockedQ.h incl/streamInit.h
	$(CC) -c src/lockedQ.cpp -o $@
src/playAudio.o: src/playAudio.cpp incl/streamInit.h incl/reader.h incl/lockedQ.h incl/threadRunner.h incl/audioPlayer.h incl/audioResampler.h incl/conPlot.h incl/xPlot.h
	$(CC) -c src/playAudio.cpp -o $@
src/reader.o: src/reader.cpp incl/reader.h incl/lockedQ.h incl/threadRunner.h
	$(CC) -c src/reader.cpp -o $@
src/streamInit.o: src/streamInit.cpp incl/streamInit.h
	$(CC) -c src/streamInit.cpp -o $@
src/xPlot.o: src/xPlot.cpp incl/xPlot.h incl/audioResampler.h
	$(CC) -c src/xPlot.cpp -o $@
src/xPlotStream.o: src/xPlotStream.cpp incl/xPlotStream.h incl/audioResampler.h
	$(CC) -c src/xPlotStream.cpp -o $@
playAudio:$(objs)
	$(CC) $(objs) $(libs) -o playAudio

clean:
	rm -f playAudio $(objs)
