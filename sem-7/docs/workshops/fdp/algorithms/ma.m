system('C:\Users\casey\Documents\Fontys\Semester7\ses7\docs\workshops\fdp\algorithms\myprogram.exe');

y1 = load('y1.txt');
y2 = load('y2.txt');

figure;
subplot(2,1,1);
plot(y1);
title('Moving Average with FIIR filter');
xlabel('Sample');
ylabel('Value');

subplot(2,1,2);
plot(y2);
title('Moving Average with IIR filter');
xlabel('Sample');
ylabel('Value');
