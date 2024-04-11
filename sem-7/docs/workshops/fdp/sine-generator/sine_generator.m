system('C:\Users\casey\Documents\Fontys\Semester7\ses7\docs\workshops\fdp\sine-generator\myprogram.exe');
data = load('output.txt');

figure;
plot(data(:, 1), data(:, 2), 'b', 'DisplayName', 'Real Part');
hold on;
plot(data(:, 1), data(:, 3), 'r', 'DisplayName', 'Imaginary Part');
xlabel('Frequency');
ylabel('Value');
title('Complex Number Evolution');
legend('show');
hold off;
