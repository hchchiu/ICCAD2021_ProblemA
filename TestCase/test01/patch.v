module top_eco(a, b, c, o);
  input a, b, c;
  output o;
  wire patchNew_n1;
  or eco1 (o, a, patchNew_n1);
  and eco2 (patchNew_n1, b, c);
endmodule
