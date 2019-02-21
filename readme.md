
#Audio Player for Linux.
========================
audioPlayer as the name suggests is a command line audio player for Linux. It plays all audio formats supported by FFmpeg. Unlike many other audio players, it does not change the sample format prior to playback.

##Install and build.
====================
Make sure all the dependencies in the doc. are satisfied.
Clone the repository to your machine. 
cd <code directory>
make

##Usage
=======
After the code is built.
cd <code directory>
./audioPlayer <fileName>

##Dependencies
==============
FFmpeg version 4.0 or later.
libasound version 1.1.6 or later.
Clang ver 7.
Developed and tested on Ubuntu 18.10
