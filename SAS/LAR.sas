data g1;
input T_avg r @@;
datalines;
28.3540		0.1112
25.2712		0.1265
24.6334		0.1363
23.1070		0.1536
22.7050		0.1537
21.3510		0.1217
18.4260		0.1153
15.2102		0.1024
;
* 9.4431		0.1199 ;


proc nlin data=g1 method=newton; * Yin 1995, Yan and Hunt (1999);
title " Reduced Beta fit for relative leaf-expansion rate (r) using daily mean temperature";
 parms
  R_max = 0.16
  T_opt = 23
  T_max = 30
  
;
 T_base = 0.0;
 beta = 1.0;
 f1 = (T_avg-T_base)/(T_opt-T_base);
 g1 = (T_max-T_avg)/(T_max-T_opt);

 alpha = beta*(T_opt-T_base)/(T_max-T_opt);

 h1 = R_max*f1**alpha*g1**beta;

 model     r = h1; * See Kim et al. (2007) EEB;
 id T_avg;
 output out=p1 p=r_pred parms = R_max T_opt T_max T_pred;
run;

symbol1 v=circle i=none c=blue;
symbol2 v=star i=none c=red;
symbol3 v=dot i=none c=red;
symbol4 v=triangle i=none c=green;
symbol5 v=star i=none c=brown;
symbol6 v=none i=none c=brown;

legend1 label=none
        shape=symbol(4,1)
        position=(bottom center outside)
        mode=share;

proc gplot data=p1; 
   title "beta fit";
      plot (r r_pred)* T_avg /  overlay legend=legend1;
   run;
title;

data p;
 do R_max = 0.2 to 0.4 by 0.05;
  do T_opt = 19 to 23 by 1;
     T_max = T_opt + 12;
    do T_avg = 0 to T_max by 1;
     T_base = 0.0;
     beta = 1.0;
     f1 = (T_avg-T_base)/(T_opt-T_base);
     g1 = (T_max-T_avg)/(T_max-T_opt);
     alpha = beta*(T_opt-T_base)/(T_max-T_opt);
     rate = R_max*f1**alpha*g1**beta;
	 output;
	end;
   end;
 end;

 proc gplot data=p; 
  where (R_max = 0.25 or R_max = 0.35) and (T_opt = 20 or T_opt = 23);
      plot rate* T_avg = T_opt/ legend=legend1;
	  plot2 rate* T_avg = R_max/ legend=legend1;
   run;

quit;
