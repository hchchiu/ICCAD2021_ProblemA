module top_eco(a, b, c, o);
  input a, b, c;
  output o;
  wire nn1;
  or eco1 (o, a, nn1);
  and eco2 (nn1, b, c);
endmodule
// cost:5