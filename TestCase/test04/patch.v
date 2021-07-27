module top_eco(n_401, \prev_state[0], n_410, Sync, Gate);
  input n_401, \prev_state[0], n_410;
  output Sync, Gate;
  wire nn_39;
  nand eco1 (Sync, nn_39, n_410);
  assign Gate = 1'b0;
  or eco3 (nn_39, n_401, \prev_state[0]);
endmodule
// cost:7