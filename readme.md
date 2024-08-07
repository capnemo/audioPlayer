Audio Player for Linux
-----------------------
audioPlayer as the name suggests is a command line audio player for Linux. It plays all audio formats supported by FFmpeg. Unlike many other audio players, it does not change the sample format prior to playback. The utility is structured as follows. There are 2 threads; the first one reads and decodes the audio stream from the  input file and the second thread writes the decoded data to libasound (alsa). The two threads communicate via a queue. The tests directory contains component tests. The clips directory contains audio files that may be used to test for regressions.

Install and build
-----------------
* Make sure all the dependencies in the dependencies section  are satisfied.  
* Clone the repository to your machine.  
* *cd  code_location*  
* *make*

Usage
-----
After the code is built.  
*./playAudio  fileName*

Dependencies
------------
* FFmpeg version 7.0 or later.  
* libasound version 1.1.0 or later.  
* Clang ver 7.  
* Developed and tested on Ubuntu 18.10  

