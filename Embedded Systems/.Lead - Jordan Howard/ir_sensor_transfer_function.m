close all; clear; clc;

% -----------------------------
% DATA
% -----------------------------
d = [300 290 280 270 260 250 240 230 220 210 200 190 180 170 160 150 140 130 120 110 100]';
V = [515 532 575 615 635 690 740 755 830 870 950 1050 1150 1250 1350 1450 1550 1900 2100 2300 2500]';

% -----------------------------
% SETTINGS
% -----------------------------
signal_span = 3300;  % full-scale voltage range in mV

% Smooth axis
d_fit = linspace(min(d), max(d), 300)';

% -----------------------------
% 1) LINEAR FIT (1st order)
% -----------------------------
p1 = polyfit(d, V, 1);
V_fit1 = polyval(p1, d_fit);
V_pred1 = polyval(p1, d);
MAE1 = mean(abs(V - V_pred1));
RelErr1 = (MAE1 / signal_span) * 100;

% -----------------------------
% 2) QUADRATIC FIT (2nd order)
% -----------------------------
p2 = polyfit(d, V, 2);
V_fit2 = polyval(p2, d_fit);
V_pred2 = polyval(p2, d);
MAE2 = mean(abs(V - V_pred2));
RelErr2 = (MAE2 / signal_span) * 100;

% -----------------------------
% PLOT
% -----------------------------
figure; hold on; grid on;
plot(d, V, 'ko', 'MarkerFaceColor','k', 'DisplayName', 'Measured Data');
plot(d_fit, V_fit1, 'b-', 'LineWidth', 1.8, ...
    'DisplayName', sprintf('1st-Order: MAE = %.1f mV (%.2f%% FSR)', MAE1, RelErr1));
plot(d_fit, V_fit2, 'r-', 'LineWidth', 1.8, ...
    'DisplayName', sprintf('2nd-Order: MAE = %.1f mV (%.2f%% FSR)', MAE2, RelErr2));

xlabel('Distance (mm)');
ylabel('Voltage (mV)');
title('1st vs 2nd Order Polynomial Fit of IR Sensor');
legend('Location','northeast');

% -----------------------------
% PRINT EQUATIONS AND ERRORS
% -----------------------------
fprintf('Linear Fit (1st):   V = %.4f*d %+ .4f\n', p1(1), p1(2));
fprintf('Quadratic Fit (2nd): V = %.4e*d^2 %+ .4e*d %+ .4e\n', p2(1), p2(2), p2(3));
fprintf('MAE1 = %.2f mV (%.2f%% FSR)\n', MAE1, RelErr1);
fprintf('MAE2 = %.2f mV (%.2f%% FSR)\n', MAE2, RelErr2);


