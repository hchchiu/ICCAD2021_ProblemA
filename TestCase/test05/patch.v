module top_eco(\prev_state[1] ,\prev_state[2] ,\prev_state[3] ,\prev_state[4] , wc0, n_292, Sync, Gate, Done);
  input \prev_state[1] ,\prev_state[2] ,\prev_state[3] ,\prev_state[4] ;
  input wc0, n_292;
  output Sync, Gate, Done;
  wire patchNew_n_82, patchNew_n_81, patchNew_n_79, patchNew_n_80;
  or eco1 (patchNew_n_82, patchNew_n_79, patchNew_n_80);
  nor eco2 (Sync, patchNew_n_81, patchNew_n_82);
  assign Gate = 1'b0;
  assign Done = 1'b0;
  or eco5 (patchNew_n_81, wc0, n_292);
  or eco6 (patchNew_n_79,\prev_state[4] ,\prev_state[3] );
  or eco7 (patchNew_n_80,\prev_state[2] ,\prev_state[1] );
endmodule
