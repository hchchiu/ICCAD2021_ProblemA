module top_eco(\prev_state[1], \prev_state[2], \prev_state[3], \prev_state[4], wc0, n_292, Sync, Gate, Done);
  input \prev_state[1], \prev_state[2], \prev_state[3], \prev_state[4], wc0;
  input n_292;
  output Sync, Gate, Done;
  wire nn_81, nn_82, nn_79, nn_80;
  nor eco1 (Sync, nn_81, nn_82);
  assign Gate = 1'b0;
  assign Done = 1'b0;
  or eco4 (nn_81, wc0, n_292);
  or eco5 (nn_82, nn_79, nn_80);
  or eco6 (nn_79, \prev_state[4], \prev_state[3]);
  or eco7 (nn_80, \prev_state[2], \prev_state[1]);
endmodule
// cost:14