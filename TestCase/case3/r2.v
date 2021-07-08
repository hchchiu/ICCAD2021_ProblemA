
module top(a, b, c, y);
  input a, b, c;
  output y1;
  and g10 (g10, a, b);
  or g11 (g11, a, c);
  not g12 (g12, b);
  or g13 (g13,g10,g11);
  nand g14 (g14,g12,g11);
  and g15 (g15,c,g13);
  and g16 (g16,g14,c);
  or g17 (y1,g15,g16);
endmodule

