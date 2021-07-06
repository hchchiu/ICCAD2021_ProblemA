
module top(a, b, c, y);
  input a, b, c;
  output y;
  and g7 (g7, a, b);
  xor g8 (g8, a, c);
  and g9 (g9, b, c);
  and g10 (g10, g7, g8);
  or g11 (g11, g10, g9);
  or g12 (g12, g8, g9);
  and y (y, g11, g12);
endmodule

