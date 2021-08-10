module top_eco(a, b, c, y2);
  input a, b, c;
  output y2;
  wire patchNew_g9, patchNew_g8, patchNew_g7_g8;
  or eco1 (y2, patchNew_g7_g8, patchNew_g9);
  and eco2 (patchNew_g9, c, b);
  nor eco3 (patchNew_g8, a, b);
  or eco4 (patchNew_g7_g8, patchNew_g7, patchNew_g8);
  and eco5 (patchNew_g7, a, patchNew_g6);
  not eco6 (patchNew_g6, c);
endmodule
