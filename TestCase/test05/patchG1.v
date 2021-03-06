module top(clk, ena, rst, Tsync[0], Tsync[1], Tsync[2], Tsync[3], Tsync[4], Tsync[5], Tsync[6], Tsync[7], Tgdel[0], Tgdel[1], Tgdel[2], Tgdel[3], Tgdel[4], Tgdel[5], Tgdel[6], Tgdel[7], Tgate[0], Tgate[1], Tgate[2], Tgate[3], Tgate[4], Tgate[5], Tgate[6], Tgate[7], Tgate[8], Tgate[9], Tgate[10], Tgate[11], Tgate[12], Tgate[13], Tgate[14], Tgate[15], Tlen[0], Tlen[1], Tlen[2], Tlen[3], Tlen[4], Tlen[5], Tlen[6], Tlen[7], Tlen[8], Tlen[9], Tlen[10], Tlen[11], Tlen[12], Tlen[13], Tlen[14], Tlen[15], prev_cnt[0], prev_cnt[1], prev_cnt[2], prev_cnt[3], prev_cnt[4], prev_cnt[5], prev_cnt[6], prev_cnt[7], prev_cnt[8], prev_cnt[9], prev_cnt[10], prev_cnt[11], prev_cnt[12], prev_cnt[13], prev_cnt[14], prev_cnt[15], prev_cnt_len[0], prev_cnt_len[1], prev_cnt_len[2], prev_cnt_len[3], prev_cnt_len[4], prev_cnt_len[5], prev_cnt_len[6], prev_cnt_len[7], prev_cnt_len[8], prev_cnt_len[9], prev_cnt_len[10], prev_cnt_len[11], prev_cnt_len[12], prev_cnt_len[13], prev_cnt_len[14], prev_cnt_len[15], prev_state[0], prev_state[1], prev_state[2], prev_state[3], prev_state[4], Sync, Gate, Done);
  input clk, ena, rst, Tsync[0], Tsync[1], Tsync[2];
  input Tsync[3], Tsync[4], Tsync[5], Tsync[6], Tsync[7], Tgdel[0];
  input Tgdel[1], Tgdel[2], Tgdel[3], Tgdel[4], Tgdel[5], Tgdel[6];
  input Tgdel[7], Tgate[0], Tgate[1], Tgate[2], Tgate[3], Tgate[4];
  input Tgate[5], Tgate[6], Tgate[7], Tgate[8], Tgate[9], Tgate[10];
  input Tgate[11], Tgate[12], Tgate[13], Tgate[14], Tgate[15], Tlen[0];
  input Tlen[1], Tlen[2], Tlen[3], Tlen[4], Tlen[5], Tlen[6];
  input Tlen[7], Tlen[8], Tlen[9], Tlen[10], Tlen[11], Tlen[12];
  input Tlen[13], Tlen[14], Tlen[15], prev_cnt[0], prev_cnt[1], prev_cnt[2];
  input prev_cnt[3], prev_cnt[4], prev_cnt[5], prev_cnt[6], prev_cnt[7], prev_cnt[8];
  input prev_cnt[9], prev_cnt[10], prev_cnt[11], prev_cnt[12], prev_cnt[13], prev_cnt[14];
  input prev_cnt[15], prev_cnt_len[0], prev_cnt_len[1], prev_cnt_len[2], prev_cnt_len[3], prev_cnt_len[4];
  input prev_cnt_len[5], prev_cnt_len[6], prev_cnt_len[7], prev_cnt_len[8], prev_cnt_len[9], prev_cnt_len[10];
  input prev_cnt_len[11], prev_cnt_len[12], prev_cnt_len[13], prev_cnt_len[14], prev_cnt_len[15], prev_state[0];
  input prev_state[1], prev_state[2], prev_state[3], prev_state[4];
  output Sync, Gate, Done;
  nor eco1 (Sync, patchNew_n_81, patchNew_n_82);
  assign Gate = 1'b0;
  assign Done = 1'b0;
  not eco4 (wc0, prev_state[0]);
  or eco5 (n_292, wc2, rst);
  not eco6 (wc2, ena);
  or eco7 (patchNew_n_81, wc0, n_292);
  or eco8 (patchNew_n_82, patchNew_n_79, patchNew_n_80);
  or eco9 (patchNew_n_79, prev_state[4], prev_state[3]);
  or eco10 (patchNew_n_80, prev_state[2], prev_state[1]);
endmodule