function v = deltasigma5mod(u)
% Taken from "Understanding Delta-Sigma Data Converters" (Schreier, R. and
% Temes, G. C.) pag. 304 (figure 9.17), pag. 302 (figure 9.15)

a = [1.561, 1.75, 1.355, 0.9588, 0.7931];
b = [0.3561, 0, 0, 0, 0, 1];
c = [0.3561, 0.4025, 0.2819, 0.2215, 0.08296];
g = [0.001586, 0.01526];

x = [0, 0, 0, 0, 0];
xn = [0, 0, 0, 0, 0];
y = 0;

for i = 1:length(u)
    if (y >= 0)
        v(i) = 1;
    else
        v(i) = -1;
    end
    
    x(1) = xn(1) + b(1)*u(i) - c(1)*v(i);
    x(2) = c(2)*xn(1) + xn(2) - g(1)*xn(3);
    x(3) = c(3)*x(2) + xn(3);
    x(4) = c(4)*xn(3) + xn(4) - g(2)*xn(5);
    x(5) = c(5)*x(4) + xn(5);
    
    y = a(1)*xn(1) + a(2)*x(2) + a(3)*xn(3) + a(4)*x(4) + a(5)*xn(5) + b(6)*u(i);
    
    xn = x;
end

end