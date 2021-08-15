
module top(a, b, c, y);
  input a, b, c;
  output y1,y2;
  and g1 (g1, a, b);
  xor g2 (g2, a, c);
  nor g3 (g3, c, b);
  and g4 (y1,g1,g2);
  or  g5 (y2,g3,1'b0);
endmodule

