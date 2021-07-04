
module top(a, b, c, y);
  input a, b, c;
  output y;
  and g1 (g1, a, b);
  or g5 (g5, g1, g3);
  and y (y, g4, g5);
  xor g2 (g2, a, c);
  nor g3 (g3, b, c);
  and g4 (g4, g1, g2);
endmodule

