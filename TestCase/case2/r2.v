
module top(a, b, c, y);
  input a, b, c;
  output y1, y2;
  not g6 (g6, c);
  and g7 (g7, a, g6);
  nor g8 (g8, a, b);
  and g9 (g9, c, b);
  and g10 (y1, g7, b);
  or g11 (y2, g7, g8, g9);
endmodule

