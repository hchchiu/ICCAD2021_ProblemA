
// Generated by Cadence Genus(TM) Synthesis Solution 19.20-d227_1
// Generated on: Aug  3 2021 03:02:20 PDT (Aug  3 2021 10:02:20 UTC)

// Verification Directory fv/top 

module top(clk, ena, rst, Tsync, Tgdel, Tgate, Tlen, Sync, Gate, Done,
     prev_state, prev_cnt, prev_cnt_len, cnt, state);
  input clk, ena, rst;
  input [7:0] Tsync, Tgdel;
  input [15:0] Tgate, Tlen, prev_cnt, prev_cnt_len;
  input [4:0] prev_state;
  output Sync, Gate, Done;
  output [15:0] cnt;
  output [4:0] state;
  wire clk, ena, rst;
  wire [7:0] Tsync, Tgdel;
  wire [15:0] Tgate, Tlen, prev_cnt, prev_cnt_len;
  wire [4:0] prev_state;
  wire Sync, Gate, Done;
  wire [15:0] cnt;
  wire [4:0] state;
  wire n_0, n_1, n_2, n_3, n_4, n_5, n_6, n_7;
  wire n_8, n_9, n_10, n_11, n_12, n_13, n_14, n_15;
  wire n_16, n_17, n_18, n_19, n_20, n_21, n_22, n_23;
  wire n_24, n_25, n_26, n_27, n_30, n_31, n_32, n_33;
  wire n_34, n_35, n_36, n_37, n_38, n_39, n_40, n_41;
  wire n_42, n_43, n_44, n_45, n_46, n_47, n_48, n_49;
  wire n_50, n_51, n_52, n_53, n_54, n_55, n_56, n_57;
  wire n_58, n_59, n_60, n_61, n_62, n_63, n_64, n_65;
  wire n_66, n_67, n_68, n_69, n_70, n_71, n_72, n_73;
  wire n_74, n_75, n_77, n_78, n_79, n_80, n_81, n_82;
  wire n_83, n_84, n_85, n_86, n_87, n_88, n_89, n_90;
  wire n_91, n_92, n_93, n_94, n_95, n_96, n_97, n_98;
  wire n_99, n_100, n_101, n_102, n_103, n_104, n_105, n_106;
  wire n_107, n_108, n_109, n_110, n_111, n_112, n_113, n_114;
  wire n_115, n_116, n_117, n_119, n_120, n_121, n_122, n_123;
  wire n_124, n_125, n_126, n_127, n_128, n_129, n_130, n_131;
  wire n_132, n_133, n_134, n_135, n_136, n_137;
  nand g7293 (n_137, n_105, n_113);
  nand g7294 (n_136, n_107, n_115);
  nand g7295 (n_135, n_108, n_114);
  nand g7296 (n_134, n_109, n_111);
  nand g7297 (n_133, n_121, n_112);
  nand g7298 (n_132, n_117, n_120);
  nand g7299 (n_131, n_103, n_104);
  nand g7300 (n_130, n_110, n_106);
  nand g7313 (n_129, n_80, n_97);
  nand g7310 (n_128, n_82, n_96);
  nand g7308 (n_127, n_79, n_99);
  nand g7316 (n_126, n_78, n_100);
  nand g7304 (n_125, n_90, n_102);
  nand g7302 (n_124, n_91, n_101);
  nand g7309 (n_123, n_84, n_95);
  nand g7305 (n_122, n_88, n_98);
  wire w;
  nor g7332 (n_121, w, n_81);
  and g (w, Tsync[1], n_116);
  wire w0, w1;
  and g7311 (n_120, w0, w1);
  nand g1 (w1, Tgate[2], n_75);
  nand g0 (w0, Tgdel[2], n_119);
  wire w2;
  nor g7331 (n_117, w2, n_85);
  and g2 (w2, Tsync[2], n_116);
  wire w3, w4;
  and g7301 (n_115, w3, w4);
  nand g4 (w4, Tgate[6], n_75);
  nand g3 (w3, Tgdel[6], n_119);
  wire w5, w6;
  and g7303 (n_114, w5, w6);
  nand g6 (w6, Tgate[7], n_75);
  nand g5 (w5, Tgdel[7], n_119);
  wire w7, w8;
  and g7306 (n_113, w7, w8);
  nand g8 (w8, Tgate[5], n_75);
  nand g7 (w7, Tgdel[5], n_119);
  wire w9, w10;
  and g7312 (n_112, w9, w10);
  nand g10 (w10, Tgate[1], n_75);
  nand g9 (w9, Tgdel[1], n_119);
  wire w11, w12;
  and g7307 (n_111, w11, w12);
  nand g12 (w12, Tgate[4], n_75);
  nand g11 (w11, Tgdel[4], n_119);
  wire w13;
  nor g7315 (n_110, w13, n_83);
  and g13 (w13, Tgdel[0], n_119);
  wire w14;
  nor g7325 (n_109, w14, n_77);
  and g14 (w14, Tsync[4], n_116);
  wire w15;
  nor g7326 (n_108, w15, n_94);
  and g15 (w15, Tsync[7], n_116);
  wire w16;
  nor g7327 (n_107, w16, n_87);
  and g16 (w16, Tsync[6], n_116);
  wire w17, w18;
  and g7314 (n_106, w17, w18);
  nand g18 (w18, Tgate[0], n_75);
  nand g17 (w17, Tsync[0], n_116);
  wire w19;
  nor g7328 (n_105, w19, n_86);
  and g19 (w19, Tsync[5], n_116);
  wire w20, w21;
  and g7317 (n_104, w20, w21);
  nand g21 (w21, Tgate[3], n_75);
  nand g20 (w20, Tgdel[3], n_119);
  wire w22;
  nor g7330 (n_103, w22, n_93);
  and g22 (w22, Tsync[3], n_116);
  nand g7320 (n_102, Tgate[8], n_75);
  nand g7319 (n_101, Tgate[9], n_75);
  nand g7318 (n_100, Tgate[11], n_75);
  nand g7321 (n_99, Tgate[14], n_75);
  nand g7324 (n_98, Tgate[15], n_75);
  nand g7323 (n_97, Tgate[12], n_75);
  nand g7322 (n_96, Tgate[13], n_75);
  nand g7329 (n_95, Tgate[10], n_75);
  nor g7336 (n_94, n_40, n_92);
  nor g7333 (n_93, n_21, n_92);
  or g7334 (n_91, n_46, n_92);
  or g7335 (n_90, n_43, n_92);
  and g7353 (n_89, n_73, n_30);
  or g7337 (n_88, n_63, n_92);
  nor g7338 (n_87, n_37, n_92);
  nor g7339 (n_86, n_34, n_92);
  nor g7350 (n_85, n_14, n_92);
  or g7341 (n_84, n_49, n_92);
  nor g7344 (n_83, prev_cnt[0], n_92);
  or g7345 (n_82, n_58, n_92);
  nor g7346 (n_81, n_7, n_92);
  or g7347 (n_80, n_55, n_92);
  or g7348 (n_79, n_61, n_92);
  or g7349 (n_78, n_52, n_92);
  nor g7340 (n_77, n_27, n_92);
  assign Gate = n_75;
  and g7351 (n_119, prev_state[1], n_74, n_71, n_72);
  nor g7352 (n_75, prev_state[1], prev_state[3], n_74, n_65);
  wire w23, w24;
  nand g7357 (n_73, w23, w24, n_72);
  or g24 (w24, n_71, n_69);
  or g23 (w23, prev_state[3], n_66);
  wire w25;
  nor g7355 (n_92, w25, n_68);
  and g25 (w25, n_33, n_12);
  and g7354 (n_70, prev_state[3], n_69, n_64, n_67);
  wire w26;
  nor g7356 (n_68, w26, n_11);
  and g26 (w26, n_66, n_67);
  not g7358 (n_72, n_65);
  nand g7359 (n_65, n_64, n_67);
  wire w27;
  nor g7360 (n_63, w27, n_67);
  and g27 (w27, prev_cnt[15], n_62);
  nor g7361 (n_67, prev_cnt[15], n_62);
  wire w28;
  nor g7362 (n_61, w28, n_60);
  and g28 (w28, prev_cnt[14], n_59);
  not g7363 (n_62, n_60);
  nor g7364 (n_60, prev_cnt[14], n_59);
  wire w29;
  nor g7365 (n_58, w29, n_57);
  and g29 (w29, prev_cnt[13], n_56);
  not g7366 (n_59, n_57);
  nor g7367 (n_57, prev_cnt[13], n_56);
  wire w30;
  nor g7368 (n_55, w30, n_54);
  and g30 (w30, prev_cnt[12], n_53);
  not g7369 (n_56, n_54);
  nor g7370 (n_54, prev_cnt[12], n_53);
  wire w31;
  nor g7371 (n_52, w31, n_51);
  and g31 (w31, prev_cnt[11], n_50);
  not g7372 (n_53, n_51);
  nor g7373 (n_51, prev_cnt[11], n_50);
  wire w32;
  nor g7374 (n_49, w32, n_48);
  and g32 (w32, prev_cnt[10], n_47);
  not g7375 (n_50, n_48);
  nor g7376 (n_48, prev_cnt[10], n_47);
  wire w33;
  nor g7377 (n_46, w33, n_45);
  and g33 (w33, prev_cnt[9], n_44);
  not g7378 (n_47, n_45);
  nor g7379 (n_45, prev_cnt[9], n_44);
  wire w34;
  nor g7380 (n_43, w34, n_42);
  and g34 (w34, prev_cnt[8], n_41);
  not g7382 (n_44, n_42);
  nor g7383 (n_42, prev_cnt[8], n_41);
  wire w35;
  nor g7387 (n_40, w35, n_39);
  and g35 (w35, prev_cnt[7], n_38);
  not g7389 (n_41, n_39);
  nor g7391 (n_39, prev_cnt[7], n_38);
  wire w36;
  nor g7390 (n_37, w36, n_36);
  and g36 (w36, prev_cnt[6], n_35);
  not g7395 (n_38, n_36);
  nor g7396 (n_36, prev_cnt[6], n_35);
  wire w37;
  nor g7397 (n_34, w37, n_32);
  and g37 (w37, prev_cnt[5], n_31);
  wire w38;
  nand g7381 (n_33, w38, n_71);
  or g38 (w38, n_9, n_26);
  not g7398 (n_35, n_32);
  nor g7399 (n_32, prev_cnt[5], n_31);
  not g7385 (n_30, n_116);
  assign state[1] = n_116;
  assign Sync = n_116;
  wire w39;
  nor g7401 (n_27, w39, n_25);
  and g39 (w39, prev_cnt[4], n_22);
  wire w40;
  nand g7388 (n_116, w40, n_23);
  or g40 (w40, prev_state[4], n_17);
  wire w41;
  nor g7393 (n_26, w41, n_6);
  and g41 (w41, prev_state[4], n_20);
  not g7403 (n_31, n_25);
  not g7392 (n_24, n_23);
  nor g7405 (n_25, prev_cnt[4], n_22);
  nand g7394 (n_23, prev_state[4], n_16, n_19);
  wire w42;
  nor g7406 (n_21, w42, n_18);
  and g42 (w42, prev_cnt[3], n_15);
  not g7400 (n_20, n_19);
  not g7408 (n_22, n_18);
  nor g7402 (n_19, prev_state[0], n_13);
  nand g7404 (n_17, prev_state[0], n_16);
  nor g7409 (n_18, prev_cnt[3], n_15);
  wire w43;
  nor g7410 (n_14, w43, n_10);
  and g43 (w43, prev_cnt[2], n_8);
  nand g7407 (n_13, n_5, n_2, n_4, n_3);
  and g7411 (n_16, n_71, n_12, n_69);
  not g7412 (n_64, n_11);
  not g7413 (n_15, n_10);
  nand g7415 (n_11, n_9, n_12);
  nor g7417 (n_10, prev_cnt[2], n_8);
  wire w44;
  nor g7414 (n_7, w44, n_1);
  and g44 (w44, prev_cnt[0], prev_cnt[1]);
  wire w45;
  nor g7416 (n_66, w45, n_69);
  and g45 (w45, prev_state[1], prev_state[2]);
  not g7425 (n_6, n_69);
  nor g7418 (n_5, prev_cnt_len[9], prev_cnt_len[14], prev_cnt_len[10],
       prev_cnt_len[15]);
  nor g7419 (n_4, prev_cnt_len[0], prev_cnt_len[1], prev_cnt_len[2],
       prev_cnt_len[3]);
  nor g7421 (n_3, prev_cnt_len[8], prev_cnt_len[11], prev_cnt_len[12],
       prev_cnt_len[13]);
  nor g7420 (n_2, prev_cnt_len[4], prev_cnt_len[5], prev_cnt_len[6],
       prev_cnt_len[7]);
  not g7424 (n_8, n_1);
  nor g7423 (n_12, rst, n_0);
  nor g7426 (n_1, prev_cnt[0], prev_cnt[1]);
  nor g7422 (n_9, prev_state[0], prev_state[4]);
  nor g7427 (n_69, prev_state[1], prev_state[2]);
  not g7428 (n_0, ena);
  not g7429 (n_74, prev_state[2]);
  not g7430 (n_71, prev_state[3]);
  assign state[2] = n_119;
  assign state[3] = n_75;
  assign state[0] = n_89;
  assign state[4] = n_70;
  assign cnt[15] = n_122;
  assign cnt[12] = n_129;
  assign cnt[10] = n_123;
  assign cnt[9] = n_124;
  assign cnt[8] = n_125;
  assign cnt[11] = n_126;
  assign cnt[14] = n_127;
  assign cnt[13] = n_128;
  assign cnt[4] = n_134;
  assign cnt[3] = n_131;
  assign cnt[2] = n_132;
  assign cnt[1] = n_133;
  assign cnt[5] = n_137;
  assign cnt[7] = n_135;
  assign cnt[6] = n_136;
  assign cnt[0] = n_130;
  assign Done = n_24;
endmodule

