
module top(a, b, c, y);
  input a, b, c;
  output y1;
  and g1 (g1, a, b);
  or g2 (g2, a, c);
  not g3 (g3, b);
  not g4 (g4, c);
  or g5 (g5,g1,g2);
  nand g6 (g6,g3,g2);
  not g7 (g7, g4);
  or g8 (g8,g5,g6);
  and g9 (y1,g8,g7);
endmodule

