function v = deltasigma2mod(u)
% Taken from "Understanding Delta-Sigma Data Converters" (Schreier, R. and
% Temes, G. C.) pag. 74 (3.10 i 3.11), pag. 64 (figure 3.2)

x = [0; 0]; % [x1, x2]

for i = 1:length(u)
    if (x(2) >= 0)
        v(i) = 1;
    else
        v(i) = -1;
    end
    
    x = [1, 0; 1, 1]*x + [-1; -2]*v(i) + [1; 1]*u(i);
end

end