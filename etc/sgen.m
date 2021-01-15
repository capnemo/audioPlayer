#!/usr/bin/octave

sr = 1000;
tm = [0:1/sr:1];

s = 1 * sin(tm * 10 * 2 * pi);
t = 0.5 * sin(tm * 100 * 2 * pi);
u = s + t;

nw = 8;
subplot(nw, 1, 1);
plot(tm, s);
sfft = fft(s, 1001);
subplot(nw, 1, 2);
plot(abs(sfft));

subplot(nw, 1, 3);
plot(tm, t);
tfft = fft(t);
subplot(nw, 1, 4);
plot(abs(tfft));

subplot(nw, 1, 5);
plot(tm, u);
ufft = fft(u);
subplot(nw, 1, 6);
plot(abs(ufft));

vfft = ufft - tfft;
subplot(nw, 1, 7);
plot(abs(vfft)); #Works.
subplot(nw, 1, 8);
plot(tm, real(ifft((vfft)))); 

#To play an audio file use the sound function
#To read the samples in a sound file, use audioread.
