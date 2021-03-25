#!/usr/bin/octave
#Sampling frequency needs to be sufficiently large for Octave not to maltreat floats.
amp=2 #Amplitude of the wave
freq=10 #Frequency of the wave
sampling_freq=200 #Sampling frequency.

plot(amp*sin(freq*2*pi*(0:1/sampling_freq:1)));
