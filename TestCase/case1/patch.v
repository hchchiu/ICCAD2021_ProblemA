module top_eco(b, c, g2, g4, y);
  input b, c, g2, g4;
  output y;
  wire ng9, ng11, ng12;
  and eco1 (y, ng11, ng12);
  and eco2 (ng9, b, c);
  or eco3 (ng11, g4, ng9);
  or eco4 (ng12, g2, ng9);
endmodule
// cost:8