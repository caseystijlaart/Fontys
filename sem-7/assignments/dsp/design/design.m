Fs = 10000; % 10.0 kHz

Fc_low = 2000; % 2.0 kHz
Fc_high = 3000; % 3.0 kHz

filter_length = 100;

lpf = designfilt('lowpassfir', 'FilterOrder', filter_length, 'CutoffFrequency', Fc_low, 'SampleRate', Fs);

hpf = designfilt('highpassfir', 'FilterOrder', filter_length, 'CutoffFrequency', Fc_high, 'SampleRate', Fs);

[lpf_num, lpf_den] = tf(lpf);
[hpf_num, hpf_den] = tf(hpf);

bpf_num = conv(hpf_num, lpf_num);
bpf_den = conv(hpf_den, lpf_den);

figure;
freqz(lpf, 8192, Fs);
title('Lowpass Filter Frequency Response');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');

figure;
freqz(hpf, 8192, Fs);
title('Highpass Filter Frequency Response');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');

figure;
freqz(bpf_num, bpf_den, 8192, Fs);
title('Bandpass Filter Frequency Response');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
