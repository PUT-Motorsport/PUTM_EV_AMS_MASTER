du = linspace(24 * 1e-2, 24 * 10e-2);
t = 154e-6 * du / 3.8;

plot(du, t);
xlabel 'du[V]'
ylabel 't[s]'