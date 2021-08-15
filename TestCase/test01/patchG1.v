module top(a, b, c, o);
  input a, b, c;
  output o;
  and eco1 (patchNew_n1, b, c);
  or eco2 (o, a, patchNew_n1);
endmodule