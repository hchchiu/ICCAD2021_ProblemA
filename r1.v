
module top(a, b, c, y);
  input a, b, c;
  output y;
  and g1 (g1, a, b);
  xor g2 (g2, a, c);
  nor g3 (g3, b, c);
  and g4 (g4, g1, g2);
  or g5 (g5, g1, g3);
  and g6 (g6, g4, g5);
endmodule

