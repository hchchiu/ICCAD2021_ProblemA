module top_eco(\prev_state[0], \prev_state[1], \prev_state[2], \prev_state[3], \prev_state[4], wc, wc0, n_394, n_393, n_292, n_390, n_391, n_384, n_385, Sync, Gate, Done);
  input \prev_state[0], \prev_state[1], \prev_state[2], \prev_state[3], \prev_state[4];
  input wc, wc0, n_394, n_393, n_292;
  input n_390, n_391, n_384, n_385;
  output Sync, Gate, Done;
  wire nn_194, nn_39, nn_200, nn_193, nn_101;
  wire nn_138, nn_187, nn_188, nn_199, nn_186;
  wire nn_181, nn_163, nn_164;
  or eco1 (nn_194, nn_193, wc);
  nand eco2 (Sync, nn_39, nn_200);
  assign Gate = 1'b0;
  not eco4 (Done, nn_39);
  or eco5 (nn_39, nn_194, \prev_state[0]);
  or eco6 (nn_200, nn_199, wc0);
  or eco7 (nn_193, nn_101, nn_138);
  or eco8 (nn_101, nn_187, nn_188);
  or eco9 (nn_138, nn_163, nn_164);
  or eco10 (nn_187, n_385, n_390);
  or eco11 (nn_188, n_391, nn_186);
  or eco12 (nn_199, nn_138, \prev_state[4]);
  or eco13 (nn_186, nn_181, n_384);
  or eco14 (nn_181, n_394, n_393);
  or eco15 (nn_163, \prev_state[3], \prev_state[2]);
  or eco16 (nn_164, \prev_state[1], n_292);
endmodule
// cost:30