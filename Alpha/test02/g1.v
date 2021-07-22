
// Generated by Cadence Genus(TM) Synthesis Solution 20.10-p003_1
// Generated on: Mar 31 2021 19:10:52 PDT (Apr  1 2021 02:10:52 UTC)

// Verification Directory fv/top 

module top(clk, a, b, op, oe, y, parity, overflow, greater, is_eq,
     less);
  input clk, oe;
  input [7:0] a, b;
  input [1:0] op;
  output [7:0] y;
  output parity, overflow, greater, is_eq, less;
  wire clk, oe;
  wire [7:0] a, b;
  wire [1:0] op;
  wire [7:0] y;
  wire parity, overflow, greater, is_eq, less;
  wire n_471, n_472, n_473, n_474, n_475, n_476, n_477, n_478;
  wire n_479, n_480, n_481, n_482, n_483, n_484, n_485, n_486;
  wire n_487, n_488, n_489, n_490, n_491, n_492, n_493, n_494;
  wire n_495, n_496, n_497, n_498, n_499, n_500, n_501, n_502;
  wire n_503, n_504, n_505, n_506, n_507, n_508, n_509, n_510;
  wire n_512, n_513, n_515, n_516, n_517, n_518, n_519, n_520;
  wire n_534, n_535, n_537, n_539, n_541, n_542, n_543, n_544;
  wire n_545, n_546, n_547, n_549, n_555, n_556, n_557, n_558;
  wire n_559, n_560, n_562, n_563, n_564, n_565, n_566, n_567;
  wire n_568, n_569, n_570, n_571, n_572, n_573, n_574, n_575;
  wire n_576, n_581, n_582, n_585, n_588, n_591, n_594, n_597;
  wire n_600, n_603, n_606, n_613, n_614, n_615, n_622, n_623;
  wire n_624, n_633, n_634, n_635, n_636, n_645, n_646, n_647;
  wire n_648, n_651, n_654, n_665, n_666, n_667, n_668, n_669;
  wire n_680, n_681, n_682, n_683, n_684, n_695, n_696, n_697;
  wire n_698, n_699, n_710, n_711, n_712, n_713, n_714, n_725;
  wire n_726, n_727, n_728, n_729, n_740, n_741, n_742, n_743;
  wire n_744, n_751, n_752, n_753, n_758, n_759, n_762, n_765;
  wire n_768, n_771, n_774, n_777, n_803, n_804, n_810, n_811;
  wire n_814, n_815, n_816, n_817, n_818, n_819, n_820, n_821;
  wire n_822, n_823, n_827, n_828, n_829, n_830, n_831, n_832;
  wire n_833, n_834, n_835, n_836, n_837, n_838, n_839, n_840;
  wire n_841, n_842, n_843, n_844, n_845, n_846, n_849, n_850;
  wire n_851, n_852;
  assign overflow = y[7];
  or g809 (n_505, wc, a[1]);
  not gc (wc, b[1]);
  or g810 (n_506, b[1], wc0);
  not gc0 (wc0, a[1]);
  or g1164 (n_473, b[2], wc1);
  not gc1 (wc1, a[2]);
  or g1165 (n_474, wc2, a[2]);
  not gc2 (wc2, b[2]);
  or g1170 (n_481, b[4], wc3);
  not gc3 (wc3, a[4]);
  or g1171 (n_482, wc4, a[4]);
  not gc4 (wc4, b[4]);
  or g1176 (n_489, wc5, a[3]);
  not gc5 (wc5, b[3]);
  or g1177 (n_490, b[3], wc6);
  not gc6 (wc6, a[3]);
  not g1183 (y[7], n_513);
  or g1185 (n_515, b[5], wc7);
  not gc7 (wc7, a[5]);
  nand g1222 (n_537, n_473, n_506);
  nand g1227 (n_539, n_474, n_505);
  or g1232 (n_541, a[2], n_475);
  or g1235 (n_542, n_483, a[3]);
  or g1238 (n_543, n_477, a[1]);
  or g1244 (n_545, a[4], n_486);
  or g1247 (n_546, n_499, a[6]);
  or g1250 (n_547, n_496, a[5]);
  or g1258 (n_535, wc8, a[6]);
  not gc8 (wc8, b[6]);
  or g1259 (n_534, b[6], wc9);
  not gc9 (wc9, a[6]);
  or g1260 (n_549, b[7], wc10);
  not gc10 (wc10, a[7]);
  or g1261 (n_544, wc11, a[7]);
  not gc11 (wc11, b[7]);
  or g1317 (n_558, n_482, wc12);
  not gc12 (wc12, n_515);
  nand g1331 (n_565, n_473, n_474);
  nand g1335 (n_567, n_535, n_534);
  nand g1339 (n_569, n_489, n_490);
  nand g1343 (n_571, n_481, n_482);
  nand g1347 (n_573, n_505, n_506);
  or g1500 (n_803, wc13, n_510);
  not gc13 (wc13, n_494);
  or g1501 (n_804, n_494, wc14);
  not gc14 (wc14, n_510);
  nand g1502 (parity, n_803, n_804);
  or g1513 (n_810, wc15, n_509);
  not gc15 (wc15, n_502);
  or g1514 (n_811, n_502, wc16);
  not gc16 (wc16, n_509);
  nand g1515 (n_510, n_810, n_811);
  or g1520 (n_513, n_753, op[1]);
  or g1521 (n_814, wc17, n_501);
  not gc17 (wc17, n_495);
  or g1522 (n_815, n_495, wc18);
  not gc18 (wc18, n_501);
  nand g1523 (n_502, n_814, n_815);
  or g1524 (n_501, wc19, n_744);
  not gc19 (wc19, n_743);
  nand g1525 (n_753, n_751, n_752);
  or g1526 (n_816, wc20, n_508);
  not gc20 (wc20, n_504);
  or g1527 (n_817, n_504, wc21);
  not gc21 (wc21, n_508);
  nand g1528 (n_509, n_816, n_817);
  or g1529 (n_751, n_512, op[0]);
  nand g1530 (n_752, n_512, op[0]);
  nand g1531 (n_744, n_741, n_742);
  nand g1532 (n_512, n_594, n_771);
  or g1533 (n_818, wc22, n_493);
  not gc22 (wc22, n_480);
  or g1534 (n_819, n_480, wc23);
  not gc23 (wc23, n_493);
  nand g1535 (n_494, n_818, n_819);
  or g1536 (n_741, wc24, op[1]);
  not gc24 (wc24, n_568);
  or g1537 (n_504, wc25, n_729);
  not gc25 (wc25, n_728);
  nand g1538 (n_820, n_500, n_498);
  or g1539 (n_821, n_500, n_498);
  nand g1540 (n_568, n_820, n_821);
  nand g1541 (n_729, n_726, n_727);
  or g1542 (n_822, wc26, n_492);
  not gc26 (wc26, n_488);
  or g1543 (n_823, n_488, wc27);
  not gc27 (wc27, n_492);
  nand g1544 (n_493, n_822, n_823);
  nand g1545 (n_771, n_546, n_498);
  or g1546 (n_726, wc28, op[1]);
  not gc28 (wc28, n_564);
  or g1547 (n_488, wc29, n_714);
  not gc29 (wc29, n_713);
  nand g1548 (n_498, n_597, n_774);
  nand g1553 (n_774, n_547, n_497);
  nand g1556 (n_827, n_503, n_497);
  or g1557 (n_828, n_503, n_497);
  nand g1558 (n_564, n_827, n_828);
  nand g1559 (n_714, n_711, n_712);
  or g1560 (n_517, n_636, n_516);
  or g1561 (n_520, n_648, n_519);
  or g1562 (n_711, wc30, op[1]);
  not gc30 (wc30, n_572);
  or g1563 (n_492, wc31, n_699);
  not gc31 (wc31, n_698);
  nand g1564 (n_497, n_588, n_765);
  or g1565 (n_636, n_635, wc32);
  not gc32 (wc32, n_515);
  nand g1566 (n_699, n_696, n_697);
  nand g1567 (n_765, n_545, n_485);
  or g1568 (n_648, n_647, wc33);
  not gc33 (wc33, n_518);
  nand g1569 (n_829, n_487, n_485);
  or g1570 (n_830, n_487, n_485);
  nand g1571 (n_572, n_829, n_830);
  or g1572 (n_647, n_646, wc34);
  not gc34 (wc34, n_489);
  or g1573 (n_480, wc35, n_669);
  not gc35 (wc35, n_668);
  nand g1574 (n_485, n_591, n_768);
  or g1575 (n_696, wc36, op[1]);
  not gc36 (wc36, n_570);
  or g1576 (n_635, n_634, wc37);
  not gc37 (wc37, n_490);
  nand g1577 (n_646, n_645, n_482);
  or g1578 (n_508, wc38, n_684);
  not gc38 (wc38, n_683);
  nand g1579 (n_669, n_666, n_667);
  nand g1580 (n_634, n_633, n_481);
  nand g1581 (n_768, n_542, n_484);
  nand g1582 (n_831, n_491, n_484);
  or g1583 (n_832, n_491, n_484);
  nand g1584 (n_570, n_831, n_832);
  nand g1585 (n_645, n_576, n_490);
  nand g1586 (n_484, n_585, n_762);
  nand g1587 (n_684, n_681, n_682);
  nand g1588 (n_633, n_575, n_489);
  or g1589 (n_666, wc39, op[1]);
  not gc39 (wc39, n_566);
  nand g1590 (n_651, n_557, n_556);
  or g1591 (n_681, wc40, op[1]);
  not gc40 (wc40, n_574);
  nand g1592 (n_575, n_614, n_615);
  nand g1593 (n_654, n_560, n_559);
  nand g1594 (n_833, n_476, n_479);
  or g1595 (n_834, n_476, n_479);
  nand g1596 (n_566, n_833, n_834);
  nand g1597 (n_576, n_623, n_624);
  nand g1598 (n_762, n_541, n_479);
  or g1599 (n_557, n_519, wc41);
  not gc41 (wc41, n_603);
  or g1600 (n_624, n_622, wc42);
  not gc42 (wc42, b[0]);
  nand g1601 (n_479, n_600, n_777);
  nand g1602 (n_835, n_478, n_507);
  or g1603 (n_836, n_478, n_507);
  nand g1604 (n_574, n_835, n_836);
  or g1605 (n_615, n_613, b[0]);
  or g1606 (n_560, n_516, wc43);
  not gc43 (wc43, n_606);
  nand g1607 (n_777, n_478, n_543);
  nand g1608 (n_503, n_547, n_597);
  nand g1609 (n_507, n_543, n_600);
  or g1610 (n_697, wc44, n_472);
  not gc44 (wc44, n_569);
  or g1611 (n_682, wc45, n_472);
  not gc45 (wc45, n_573);
  nand g1612 (n_500, n_546, n_594);
  nand g1613 (n_614, n_474, n_537);
  or g1614 (n_742, wc46, n_472);
  not gc46 (wc46, n_567);
  nand g1615 (n_487, n_545, n_588);
  or g1616 (n_613, n_539, wc47);
  not gc47 (wc47, a[0]);
  nand g1617 (n_495, n_758, n_759);
  nand g1618 (n_476, n_541, n_585);
  nand g1619 (n_623, n_473, n_539);
  or g1620 (n_622, n_537, a[0]);
  or g1621 (n_712, wc48, n_472);
  not gc48 (wc48, n_571);
  nand g1622 (n_556, n_516, n_544);
  nand g1623 (n_559, n_519, n_549);
  nand g1624 (n_603, n_555, n_515);
  nand g1625 (n_606, n_558, n_518);
  or g1626 (n_667, wc49, n_472);
  not gc49 (wc49, n_565);
  nand g1627 (n_491, n_542, n_591);
  nand g1628 (n_594, n_499, a[6]);
  or g1629 (n_743, n_740, n_471);
  nand g1630 (n_597, n_496, a[5]);
  nand g1631 (n_759, n_562, n_471);
  or g1632 (n_713, n_710, n_471);
  or g1633 (n_758, n_582, n_471);
  or g1634 (n_728, n_725, n_471);
  or g1635 (n_727, wc50, n_472);
  not gc50 (wc50, n_563);
  or g1636 (n_683, n_680, n_471);
  nand g1637 (n_588, n_486, a[4]);
  nand g1638 (n_478, n_581, n_582);
  or g1639 (n_698, n_695, n_471);
  nand g1640 (n_600, n_477, a[1]);
  nand g1641 (n_516, n_549, n_534);
  nand g1642 (n_591, n_483, a[3]);
  nand g1643 (n_585, n_475, a[2]);
  nand g1644 (n_519, n_544, n_535);
  or g1645 (n_555, n_481, wc51);
  not gc51 (wc51, n_518);
  or g1646 (n_668, n_665, n_471);
  or g1647 (n_837, wc52, b[3]);
  not gc52 (wc52, op[0]);
  or g1648 (n_838, op[0], wc53);
  not gc53 (wc53, b[3]);
  nand g1649 (n_483, n_837, n_838);
  nand g1650 (n_695, b[3], a[3]);
  or g1651 (n_839, wc54, b[4]);
  not gc54 (wc54, op[0]);
  or g1652 (n_840, op[0], wc55);
  not gc55 (wc55, b[4]);
  nand g1653 (n_486, n_839, n_840);
  nand g1654 (n_710, b[4], a[4]);
  or g1655 (n_841, wc56, b[0]);
  not gc56 (wc56, a[0]);
  or g1656 (n_842, a[0], wc57);
  not gc57 (wc57, b[0]);
  nand g1657 (n_562, n_841, n_842);
  nand g1658 (n_740, b[6], a[6]);
  or g1659 (n_843, wc58, b[6]);
  not gc58 (wc58, op[0]);
  or g1660 (n_844, op[0], wc59);
  not gc59 (wc59, b[6]);
  nand g1661 (n_499, n_843, n_844);
  nand g1662 (n_472, op[1], op[0]);
  or g1663 (n_845, wc60, b[5]);
  not gc60 (wc60, op[0]);
  or g1664 (n_846, op[0], wc61);
  not gc61 (wc61, b[5]);
  nand g1665 (n_496, n_845, n_846);
  nand g1666 (n_725, b[5], a[5]);
  or g1668 (n_518, a[5], wc62);
  not gc62 (wc62, b[5]);
  nand g1669 (n_563, n_515, n_518);
  nand g1670 (n_680, b[1], a[1]);
  nand g1671 (n_582, a[0], b[0]);
  or g1672 (n_581, b[0], wc63);
  not gc63 (wc63, op[0]);
  or g1673 (n_849, wc64, b[1]);
  not gc64 (wc64, op[0]);
  or g1674 (n_850, op[0], wc65);
  not gc65 (wc65, b[1]);
  nand g1675 (n_477, n_849, n_850);
  or g1677 (n_851, wc66, b[2]);
  not gc66 (wc66, op[0]);
  or g1678 (n_852, op[0], wc67);
  not gc67 (wc67, b[2]);
  nand g1679 (n_475, n_851, n_852);
  or g1680 (n_471, wc68, op[0]);
  not gc68 (wc68, op[1]);
  nand g1681 (n_665, b[2], a[2]);
  nor g1682 (is_eq, n_517, n_520);
  and g1683 (less, wc69, n_520);
  not gc69 (wc69, n_651);
  and g1684 (greater, wc70, n_517);
  not gc70 (wc70, n_654);
  and g1685 (y[5], n_504, wc71);
  not gc71 (wc71, n_513);
  and g1686 (y[2], n_480, wc72);
  not gc72 (wc72, n_513);
  and g1687 (y[1], n_508, wc73);
  not gc73 (wc73, n_513);
  and g1688 (y[6], n_501, wc74);
  not gc74 (wc74, n_513);
  and g1689 (y[4], n_488, wc75);
  not gc75 (wc75, n_513);
  and g1690 (y[3], n_492, wc76);
  not gc76 (wc76, n_513);
  and g1691 (y[0], n_495, wc77);
  not gc77 (wc77, n_513);
endmodule

