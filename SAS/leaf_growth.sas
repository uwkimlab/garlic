proc import datafile="leaf_growth.xls"
     out=a0 dbms=xls replace;
     sheet='leaf_growth';
     getnames=yes; mixed=yes;
run;

data a2;
 set a0;
 rank = leafNo;
* do rank  = 1 to 11 by 1;
  l_top = 11+0.5;
  l_peak = 0.65*l_top;
  beta = 1.0; l_b = 0.0;
  f = (rank-l_b)/(l_peak-l_b);
  g = (l_top-rank)/(l_top-l_peak);
  alpha = beta*(l_peak-l_b)/(l_top-l_peak);
  beta_fn = f**alpha*g**beta;
  R_max = 50;
  yhat = max(0, R_max*beta_fn);
  output;
* end;
Keep pot dap leafNo rank length yhat;
run;
proc sort data=a2; by pot leafNo dap; run;

symbol1 v=circle i=none c=blue;
symbol2 v=dot i=none c=red;
symbol3 v=dot i=none c=red;
symbol4 v=triangle i=none c=green;
symbol5 v=star i=none c=brown;
symbol6 v=none i=reg c=brown;

legend1 label=none
        shape=symbol(4,1)
        position=(bottom center outside)
        mode=share;

proc gplot data=a2;
 where dap = 51;
 plot (length yhat)*leafNo /overlay;
run;

data a1;
input length width;
cards;	
24.0 	1.3 
37.3 	1.5 
35.0 	1.3 
41.0 	1.7 
16.3 	1.0 
43.5 	1.8 
40.5 	1.2 
45.5 	1.6 
44.3 	2.5 
36.6 	2.3 
52.8 	3.4 
43.5 	2.1 
61.8 	2.3 
44.0 	2.7 
43.5 	2.3 
	
24.3 	1.0 
25.5 	1.2 
42.3 	1.6 
41.5 	1.4 
32.0 	1.4 
39.5 	1.6 
29.2 	1.1 
47.0 	2.3 
44.2 	2.3 
37.5 	2.1 
44.0 	3.3 
44.7 	2.2 
51.0 	2.4 
46.0 	2.6 
43.0 	2.1 
	
20.2 	0.8 
13.7 	0.7 
30.5 	0.9 
31.5 	1.4 
26.5 	1.1 
34.2 	1.4 
28.0 	1.5 
37.0 	2.1 
32.7 	1.9 
31.5 	1.8 
44.0 	2.5 
36.8 	1.9 
49.0 	2.6 
44.0 	2.2 
44.0 	2.0 
	
29.5 	1.2 
11.3 	0.8 
32.5 	1.2 
35.0 	1.6 
26.0 	1.3 
23.0 	1.3 
28.2 	1.3 
34.0 	1.9 
32.2 	2.0 
33.5 	1.8 
36.3 	2.2 
48.0 	2.5 
47.0 	2.3 
54.0 	2.8 
40.8 	2.3 
	
14.4 	0.9 
30.5 	0.9 
33.5 	1.2 
38.0 	1.1 
38.0 	1.4 
27.3 	1.2 
28.0 	1.1 
38.5 	2.2 
31.0 	1.6 
39.0 	2.3 
40.5 	2.5 
49.6 	3.2 
50.3 	2.7 
38.5 	2.1 
45.0 	2.6 
	
25.0 	0.9 
32.0 	1.2 
34.0 	1.3 
32.3 	1.3 
33.8 	1.4 
28.5 	1.3 
25.7 	1.4 
34.0 	2.1 
37.4 	2.6 
33.5 	2.0 
35.3 	2.3 
35.0 	1.6 
50.3 	2.0 
41.0 	2.3 
40.7 	1.8 
	
27.5 	1.0 
25.7 	1.1 
20.6 	1.0 
25.7 	1.1 
26.7 	1.2 
42.8 	1.8 
36.5 	1.3 
32.0 	2.0 
34.6 	1.9 
28.5 	2.1 
35.2 	2.3 
54.0 	2.9 
46.0 	2.6 
39.7 	1.9 
36.0 	1.6 
	
33.0 	1.1 
23.7 	0.8 
31.2 	1.2 
22.5 	0.9 
46.2 	1.4 
40.5 	1.3 
23.0 	1.4 
34.6 	1.9 
29.5 	2.0 
29.0 	2.0 
40.0 	2.0 
49.5 	2.2 
51.5 	2.8 
40.0 	1.7 
36.8 	1.8 
	
29.7 	1.2 
24.2 	0.7 
21.2 	1.0 
23.2 	1.0 
40.0 	1.1 
37.5 	1.7 
31.3 	0.9 
32.0 	1.9 
39.0 	1.6 
32.0 	1.8 
37.3 	2.1 
39.5 	1.8 
47.0 	2.2 
39.0 	2.3 
26.0 	1.5 
	
18.0 	0.8 
36.6 	1.3 
27.7 	1.1 
12.0 	0.6 
31.0 	1.5 
19.0 	1.3 
20.4 	0.9 
31.7 	1.9 
25.0 	1.5 
31.2 	2.1 
	
42.6 	1.8 
43.0 	2.4 
33.0 	1.9 
38.5 	1.7 
;

proc glm data=a1;
 model width = length;
run;
quit;
