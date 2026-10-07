! hpcagent_bench-autogen -- generated from warpx_field_gather_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine warpx_field_gather_fp64(Bxp, Byp, Bzp, Exp, Eyp, Ezp, bx_arr, bx_type, by_arr, by_type, bz_arr, bz_type, &
&dinv, ex_arr, ex_type, ey_arr, ey_type, ez_arr, ez_type, lo, xp, xyzmin, yp, zp, depos_order, ncells, np_particles) &
&bind(C, name="warpx_field_gather_fp64")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), parameter :: galerkin_interpolation = 1_8
    integer(c_int64_t), parameter :: geom = 3_8
    integer(c_int64_t), parameter :: n_rz_azimuthal_modes = 1_8
    integer(c_int64_t), value, intent(in) :: depos_order
    integer(c_int64_t), value, intent(in) :: ncells
    integer(c_int64_t), value, intent(in) :: np_particles
    real(c_double), intent(inout) :: Bxp(np_particles)
    real(c_double), intent(inout) :: Byp(np_particles)
    real(c_double), intent(inout) :: Bzp(np_particles)
    real(c_double), intent(inout) :: Exp(np_particles)
    real(c_double), intent(inout) :: Eyp(np_particles)
    real(c_double), intent(inout) :: Ezp(np_particles)
    real(c_double), intent(in) :: bx_arr(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), &
    &((ncells + (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    integer(c_int32_t), intent(in) :: bx_type(3)
    real(c_double), intent(in) :: by_arr(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), &
    &((ncells + (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    integer(c_int32_t), intent(in) :: by_type(3)
    real(c_double), intent(in) :: bz_arr(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), &
    &((ncells + (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    integer(c_int32_t), intent(in) :: bz_type(3)
    real(c_double), intent(in) :: dinv(3)
    real(c_double), intent(in) :: ex_arr(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), &
    &((ncells + (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    integer(c_int32_t), intent(in) :: ex_type(3)
    real(c_double), intent(in) :: ey_arr(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), &
    &((ncells + (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    integer(c_int32_t), intent(in) :: ey_type(3)
    real(c_double), intent(in) :: ez_arr(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), &
    &((ncells + (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    integer(c_int32_t), intent(in) :: ez_type(3)
    integer(c_int32_t), intent(in) :: lo(3)
    real(c_double), intent(in) :: xp(np_particles)
    real(c_double), intent(in) :: xyzmin(3)
    real(c_double), intent(in) :: yp(np_particles)
    real(c_double), intent(in) :: zp(np_particles)
    integer(c_int64_t) :: si0_l1004, si0_l1007, si0_l1010, si0_l1013, si0_l1033, si0_l1036, si0_l1039, si0_l1042, &
    &si0_l1064, si0_l1067, si0_l1070, si0_l1073, si0_l1093, si0_l1096, si0_l1099, si0_l1102, si0_l1124, si0_l1127, &
    &si0_l1130, si0_l1133, si0_l1153, si0_l1156, si0_l1159, si0_l1162, si0_l1184, si0_l1187, si0_l1190, si0_l1193, &
    &si0_l1213, si0_l1216, si0_l1219, si0_l1222, si0_l1244, si0_l1247, si0_l1250, si0_l1253, si0_l1273, si0_l1276, &
    &si0_l1279, si0_l1282, si0_l1478, si0_l1482, si0_l1486, si0_l1490, si0_l1494, si0_l1498, si0_l1526, si0_l1530, &
    &si0_l1534, si0_l1538, si0_l1542, si0_l1546, si0_l1574, si0_l1578, si0_l1582, si0_l1586, si0_l1590, si0_l1594, &
    &si0_l1622, si0_l1626, si0_l1630, si0_l1634, si0_l1638, si0_l1642, si0_l1670, si0_l1674, si0_l1678, si0_l1682, &
    &si0_l1686, si0_l1690, si0_l1718, si0_l1722, si0_l1726, si0_l1730, si0_l1734, si0_l1738, si0_l577, si0_l580, &
    &si0_l583, si0_l586, si0_l607, si0_l610, si0_l613, si0_l616, si0_l637, si0_l640, si0_l643, si0_l646, si0_l667, &
    &si0_l670, si0_l673, si0_l676, si0_l697, si0_l700, si0_l703, si0_l706, si0_l727, si0_l730, si0_l733, si0_l736, &
    &si0_l757, si0_l760, si0_l763, si0_l766, si0_l786, si0_l789, si0_l792, si0_l795, si0_l815, si0_l818, si0_l821, &
    &si0_l824, si0_l845, si0_l848, si0_l851, si0_l854, si0_l875, si0_l878, si0_l881, si0_l884, si0_l904, si0_l907, &
    &si0_l910, si0_l913, si0_l944, si0_l947, si0_l950, si0_l953, si0_l973, si0_l976, si0_l979, si0_l982, si1_l100, &
    &si1_l1005, si1_l1008, si1_l1011, si1_l1014, si1_l1034, si1_l1037, si1_l104, si1_l1040, si1_l1043, si1_l1065, &
    &si1_l1068, si1_l1071, si1_l1074, si1_l108, si1_l109, si1_l1094, si1_l1097, si1_l1100, si1_l1103, si1_l1125, &
    &si1_l1128, si1_l113, si1_l1131, si1_l1134, si1_l114, si1_l115, si1_l1154, si1_l1157, si1_l1160, si1_l1163, &
    &si1_l1185, si1_l1188, si1_l119, si1_l1191, si1_l1194, si1_l12, si1_l120, si1_l121, si1_l1214, si1_l1217, &
    &si1_l122, si1_l1220, si1_l1223, si1_l1245, si1_l1248, si1_l1251, si1_l1254, si1_l1274, si1_l1277, si1_l128, &
    &si1_l1280, si1_l1283, si1_l129, si1_l13, si1_l130, si1_l131, si1_l132, si1_l1479, si1_l1483, si1_l1487, &
    &si1_l1491, si1_l1495, si1_l1499, si1_l1527, si1_l1531, si1_l1535, si1_l1539, si1_l1543, si1_l1547, si1_l1575, &
    &si1_l1579, si1_l1583, si1_l1587, si1_l1591, si1_l1595, si1_l1623, si1_l1627, si1_l1631, si1_l1635, si1_l1639, &
    &si1_l1643, si1_l1671, si1_l1675, si1_l1679, si1_l1683, si1_l1687, si1_l1691, si1_l17, si1_l1719, si1_l1723, &
    &si1_l1727, si1_l173, si1_l1731, si1_l1735, si1_l1739, si1_l177, si1_l178, si1_l18, si1_l182, si1_l183, si1_l184, &
    &si1_l188, si1_l189, si1_l19, si1_l190, si1_l191, si1_l197, si1_l198, si1_l199, si1_l200, si1_l201, si1_l205, &
    &si1_l209, si1_l210, si1_l214, si1_l215, si1_l216, si1_l220, si1_l221, si1_l222, si1_l223, si1_l229, si1_l23, &
    &si1_l230, si1_l231, si1_l232, si1_l233, si1_l237, si1_l24, si1_l241, si1_l242, si1_l246, si1_l247, si1_l248, &
    &si1_l25, si1_l252, si1_l253, si1_l254, si1_l255, si1_l26, si1_l261, si1_l262, si1_l263, si1_l264, si1_l265, &
    &si1_l269, si1_l273, si1_l274, si1_l278, si1_l279, si1_l280, si1_l284, si1_l285, si1_l286, si1_l287, si1_l293, &
    &si1_l294, si1_l295, si1_l296, si1_l297, si1_l32, si1_l33, si1_l338, si1_l34, si1_l342, si1_l343, si1_l347, &
    &si1_l348, si1_l349, si1_l35, si1_l353, si1_l354, si1_l355, si1_l356, si1_l36, si1_l362, si1_l363, si1_l364, &
    &si1_l365, si1_l366, si1_l370, si1_l374, si1_l375, si1_l379, si1_l380, si1_l381, si1_l385, si1_l386, si1_l387, &
    &si1_l388, si1_l394, si1_l395, si1_l396, si1_l397, si1_l398, si1_l40, si1_l402, si1_l406, si1_l407, si1_l411, &
    &si1_l412, si1_l413, si1_l417, si1_l418, si1_l419, si1_l420, si1_l426, si1_l427, si1_l428, si1_l429, si1_l430, &
    &si1_l434, si1_l438, si1_l439, si1_l44, si1_l443, si1_l444, si1_l445, si1_l449, si1_l45, si1_l450, si1_l451, &
    &si1_l452, si1_l458, si1_l459, si1_l460, si1_l461, si1_l462, si1_l49, si1_l50, si1_l51, si1_l55, si1_l56, si1_l57, &
    &si1_l578, si1_l58, si1_l581, si1_l584, si1_l587, si1_l608, si1_l611, si1_l614, si1_l617, si1_l638, si1_l64, &
    &si1_l641, si1_l644, si1_l647, si1_l65, si1_l66, si1_l668, si1_l67, si1_l671, si1_l674, si1_l677, si1_l68, &
    &si1_l698, si1_l701, si1_l704, si1_l707, si1_l72, si1_l728, si1_l731, si1_l734, si1_l737, si1_l758, si1_l76, &
    &si1_l761, si1_l764, si1_l767, si1_l77, si1_l787, si1_l790, si1_l793, si1_l796, si1_l8, si1_l81, si1_l816, &
    &si1_l819, si1_l82, si1_l822, si1_l825, si1_l83, si1_l846, si1_l849, si1_l852, si1_l855, si1_l87, si1_l876, &
    &si1_l879, si1_l88, si1_l882, si1_l885, si1_l89, si1_l90, si1_l905, si1_l908, si1_l911, si1_l914, si1_l945, &
    &si1_l948, si1_l951, si1_l954, si1_l96, si1_l97, si1_l974, si1_l977, si1_l98, si1_l980, si1_l983, si1_l99, &
    &si2_l1006, si2_l1009, si2_l1012, si2_l1015, si2_l1035, si2_l1038, si2_l1041, si2_l1044, si2_l1066, si2_l1069, &
    &si2_l1072, si2_l1075, si2_l1095, si2_l1098, si2_l1101, si2_l1104, si2_l1126, si2_l1129, si2_l1132, si2_l1135, &
    &si2_l1155, si2_l1158, si2_l1161, si2_l1164, si2_l1186, si2_l1189, si2_l1192, si2_l1195, si2_l1215, si2_l1218, &
    &si2_l1221, si2_l1224, si2_l1246, si2_l1249, si2_l1252, si2_l1255, si2_l1275, si2_l1278, si2_l1281, si2_l1284, &
    &si2_l1480, si2_l1484, si2_l1488, si2_l1492, si2_l1496, si2_l1500, si2_l1528, si2_l1532, si2_l1536, si2_l1540, &
    &si2_l1544, si2_l1548, si2_l1576, si2_l1580, si2_l1584, si2_l1588, si2_l1592, si2_l1596, si2_l1624, si2_l1628, &
    &si2_l1632, si2_l1636, si2_l1640, si2_l1644, si2_l1672, si2_l1676, si2_l1680, si2_l1684, si2_l1688, si2_l1692, &
    &si2_l1720, si2_l1724, si2_l1728, si2_l1732, si2_l1736, si2_l1740, si2_l579, si2_l582, si2_l585, si2_l588, &
    &si2_l609, si2_l612, si2_l615, si2_l618, si2_l639, si2_l642, si2_l645, si2_l648, si2_l669, si2_l672, si2_l675, &
    &si2_l678, si2_l699, si2_l702, si2_l705, si2_l708, si2_l729, si2_l732, si2_l735, si2_l738, si2_l759, si2_l762, &
    &si2_l765, si2_l768, si2_l788, si2_l791, si2_l794, si2_l797, si2_l817, si2_l820, si2_l823, si2_l826, si2_l847, &
    &si2_l850, si2_l853, si2_l856, si2_l877, si2_l880, si2_l883, si2_l886, si2_l906, si2_l909, si2_l912, si2_l915, &
    &si2_l946, si2_l949, si2_l952, si2_l955, si2_l975, si2_l978, si2_l981, si2_l984, si3_l1481, si3_l1485, si3_l1489, &
    &si3_l1493, si3_l1497, si3_l1501, si3_l1529, si3_l1533, si3_l1537, si3_l1541, si3_l1545, si3_l1549, si3_l1577, &
    &si3_l1581, si3_l1585, si3_l1589, si3_l1593, si3_l1597, si3_l1625, si3_l1629, si3_l1633, si3_l1637, si3_l1641, &
    &si3_l1645, si3_l1673, si3_l1677, si3_l1681, si3_l1685, si3_l1689, si3_l1693, si3_l1721, si3_l1725, si3_l1729, &
    &si3_l1733, si3_l1737, si3_l1741, x_ax0_1025, x_ax0_1054, x_ax0_1085, x_ax0_1114, x_ax0_1145, x_ax0_1174, &
    &x_ax0_1205, x_ax0_1234, x_ax0_1265, x_ax0_1294, x_ax0_1316, x_ax0_1327, x_ax0_1338, x_ax0_1350, x_ax0_1362, &
    &x_ax0_1373, x_ax0_1394, x_ax0_1405, x_ax0_1416, x_ax0_1427, x_ax0_1438, x_ax0_1449, x_ax0_1514, x_ax0_1562, &
    &x_ax0_1610, x_ax0_1658, x_ax0_1706, x_ax0_1754, x_ax0_509, x_ax0_521, x_ax0_533, x_ax0_545, x_ax0_557, x_ax0_569, &
    &x_ax0_598, x_ax0_628, x_ax0_658, x_ax0_688, x_ax0_718, x_ax0_748, x_ax0_778, x_ax0_807, x_ax0_836, x_ax0_866, &
    &x_ax0_896, x_ax0_925, x_ax0_965, x_ax0_994, x_i_1000, x_i_1002, x_i_1029, x_i_1031, x_i_1060, x_i_1062, x_i_1089, &
    &x_i_1091, x_i_1120, x_i_1122, x_i_1149, x_i_1151, x_i_1180, x_i_1182, x_i_1209, x_i_1211, x_i_1240, x_i_1242, &
    &x_i_1269, x_i_1271, x_i_1308, x_i_1319, x_i_1330, x_i_1342, x_i_1354, x_i_1365, x_i_1386, x_i_1397, x_i_1408, &
    &x_i_1419, x_i_1430, x_i_1441, x_i_1472, x_i_1474, x_i_1476, x_i_1520, x_i_1522, x_i_1524, x_i_1568, x_i_1570, &
    &x_i_1572, x_i_1616, x_i_1618, x_i_1620, x_i_1664, x_i_1666, x_i_1668, x_i_1712, x_i_1714, x_i_1716, x_i_501, &
    &x_i_513, x_i_525, x_i_537, x_i_549, x_i_561, x_i_573, x_i_575, x_i_603, x_i_605, x_i_633, x_i_635, x_i_663, &
    &x_i_665, x_i_693, x_i_695, x_i_723, x_i_725, x_i_753, x_i_755, x_i_782, x_i_784, x_i_811, x_i_813, x_i_841, &
    &x_i_843, x_i_871, x_i_873, x_i_900, x_i_902, x_i_940, x_i_942, x_i_969, x_i_971, x_inl1_imode_939, x_r0_0, &
    &x_r0_135, x_r0_1376, x_r0_1378, x_r0_1380, x_r0_139, x_r0_143, x_r0_1452, x_r0_1454, x_r0_1456, x_r0_1458, &
    &x_r0_1460, x_r0_1462, x_r0_1464, x_r0_147, x_r0_151, x_r0_155, x_r0_159, x_r0_161, x_r0_163, x_r0_165, x_r0_167, &
    &x_r0_169, x_r0_3, x_r0_300, x_r0_304, x_r0_308, x_r0_312, x_r0_316, x_r0_320, x_r0_324, x_r0_326, x_r0_328, &
    &x_r0_330, x_r0_332, x_r0_334, x_r0_465, x_r0_469, x_r0_473, x_r0_477, x_r0_481, x_r0_485, x_r0_489, x_r0_491, &
    &x_r0_493, x_r0_495, x_r0_497, x_r0_499, x_r0_929, x_r0_931, x_r0_933, x_r1_136, x_r1_140, x_r1_144, x_r1_148, &
    &x_r1_152, x_r1_156, x_r1_301, x_r1_305, x_r1_309, x_r1_313, x_r1_317, x_r1_321, x_r1_466, x_r1_470, x_r1_474, &
    &x_r1_478, x_r1_482, x_r1_486, x_rd0_1026, x_rd0_1055, x_rd0_1086, x_rd0_1115, x_rd0_1146, x_rd0_1175, x_rd0_1206, &
    &x_rd0_1235, x_rd0_1266, x_rd0_1295, x_rd0_1317, x_rd0_1328, x_rd0_1339, x_rd0_1351, x_rd0_1363, x_rd0_1374, &
    &x_rd0_1395, x_rd0_1406, x_rd0_1417, x_rd0_1428, x_rd0_1439, x_rd0_1450, x_rd0_1515, x_rd0_1563, x_rd0_1611, &
    &x_rd0_1659, x_rd0_1707, x_rd0_1755, x_rd0_510, x_rd0_522, x_rd0_534, x_rd0_546, x_rd0_558, x_rd0_570, x_rd0_599, &
    &x_rd0_629, x_rd0_659, x_rd0_689, x_rd0_719, x_rd0_749, x_rd0_779, x_rd0_808, x_rd0_837, x_rd0_867, x_rd0_897, &
    &x_rd0_926, x_rd0_966, x_rd0_995, x_rd1_1027, x_rd1_1056, x_rd1_1087, x_rd1_1116, x_rd1_1147, x_rd1_1176, &
    &x_rd1_1207, x_rd1_1236, x_rd1_1267, x_rd1_1296, x_rd1_1516, x_rd1_1564, x_rd1_1612, x_rd1_1660, x_rd1_1708, &
    &x_rd1_1756, x_rd1_600, x_rd1_630, x_rd1_660, x_rd1_690, x_rd1_720, x_rd1_750, x_rd1_780, x_rd1_809, x_rd1_838, &
    &x_rd1_868, x_rd1_898, x_rd1_927, x_rd1_967, x_rd1_996, x_rd2_1517, x_rd2_1565, x_rd2_1613, x_rd2_1661, &
    &x_rd2_1709, x_rd2_1757, x_w0_1, x_w0_10, x_w0_1001, x_w0_1003, x_w0_101, x_w0_1016, x_w0_1019, x_w0_102, &
    &x_w0_1022, x_w0_1028, x_w0_103, x_w0_1030, x_w0_1032, x_w0_1045, x_w0_1048, x_w0_105, x_w0_1051, x_w0_1057, &
    &x_w0_1058, x_w0_1059, x_w0_106, x_w0_1061, x_w0_1063, x_w0_107, x_w0_1076, x_w0_1079, x_w0_1082, x_w0_1088, &
    &x_w0_1090, x_w0_1092, x_w0_11, x_w0_110, x_w0_1105, x_w0_1108, x_w0_111, x_w0_1111, x_w0_1117, x_w0_1118, &
    &x_w0_1119, x_w0_112, x_w0_1121, x_w0_1123, x_w0_1136, x_w0_1139, x_w0_1142, x_w0_1148, x_w0_1150, x_w0_1152, &
    &x_w0_116, x_w0_1165, x_w0_1168, x_w0_117, x_w0_1171, x_w0_1177, x_w0_1178, x_w0_1179, x_w0_118, x_w0_1181, &
    &x_w0_1183, x_w0_1196, x_w0_1199, x_w0_1202, x_w0_1208, x_w0_1210, x_w0_1212, x_w0_1225, x_w0_1228, x_w0_123, &
    &x_w0_1231, x_w0_1237, x_w0_1238, x_w0_1239, x_w0_124, x_w0_1241, x_w0_1243, x_w0_125, x_w0_1256, x_w0_1259, &
    &x_w0_126, x_w0_1262, x_w0_1268, x_w0_127, x_w0_1270, x_w0_1272, x_w0_1285, x_w0_1288, x_w0_1291, x_w0_1297, &
    &x_w0_1298, x_w0_1299, x_w0_1300, x_w0_1301, x_w0_1302, x_w0_1303, x_w0_1304, x_w0_1305, x_w0_1306, x_w0_1307, &
    &x_w0_1309, x_w0_1310, x_w0_1312, x_w0_1314, x_w0_1318, x_w0_1320, x_w0_1321, x_w0_1323, x_w0_1325, x_w0_1329, &
    &x_w0_133, x_w0_1331, x_w0_1332, x_w0_1334, x_w0_1336, x_w0_134, x_w0_1340, x_w0_1341, x_w0_1343, x_w0_1344, &
    &x_w0_1346, x_w0_1348, x_w0_1352, x_w0_1353, x_w0_1355, x_w0_1356, x_w0_1358, x_w0_1360, x_w0_1364, x_w0_1366, &
    &x_w0_1367, x_w0_1369, x_w0_137, x_w0_1371, x_w0_1375, x_w0_1377, x_w0_1379, x_w0_1381, x_w0_1382, x_w0_1383, &
    &x_w0_1384, x_w0_1385, x_w0_1387, x_w0_1388, x_w0_1390, x_w0_1392, x_w0_1396, x_w0_1398, x_w0_1399, x_w0_14, &
    &x_w0_1401, x_w0_1403, x_w0_1407, x_w0_1409, x_w0_141, x_w0_1410, x_w0_1412, x_w0_1414, x_w0_1418, x_w0_1420, &
    &x_w0_1421, x_w0_1423, x_w0_1425, x_w0_1429, x_w0_1431, x_w0_1432, x_w0_1434, x_w0_1436, x_w0_1440, x_w0_1442, &
    &x_w0_1443, x_w0_1445, x_w0_1447, x_w0_145, x_w0_1451, x_w0_1453, x_w0_1455, x_w0_1457, x_w0_1459, x_w0_1461, &
    &x_w0_1463, x_w0_1465, x_w0_1466, x_w0_1467, x_w0_1468, x_w0_1469, x_w0_1470, x_w0_1471, x_w0_1473, x_w0_1475, &
    &x_w0_1477, x_w0_149, x_w0_15, x_w0_1502, x_w0_1506, x_w0_1510, x_w0_1518, x_w0_1519, x_w0_1521, x_w0_1523, &
    &x_w0_1525, x_w0_153, x_w0_1550, x_w0_1554, x_w0_1558, x_w0_1566, x_w0_1567, x_w0_1569, x_w0_157, x_w0_1571, &
    &x_w0_1573, x_w0_1598, x_w0_16, x_w0_160, x_w0_1602, x_w0_1606, x_w0_1614, x_w0_1615, x_w0_1617, x_w0_1619, &
    &x_w0_162, x_w0_1621, x_w0_164, x_w0_1646, x_w0_1650, x_w0_1654, x_w0_166, x_w0_1662, x_w0_1663, x_w0_1665, &
    &x_w0_1667, x_w0_1669, x_w0_168, x_w0_1694, x_w0_1698, x_w0_170, x_w0_1702, x_w0_171, x_w0_1710, x_w0_1711, &
    &x_w0_1713, x_w0_1715, x_w0_1717, x_w0_172, x_w0_174, x_w0_1742, x_w0_1746, x_w0_175, x_w0_1750, x_w0_1758, &
    &x_w0_1759, x_w0_176, x_w0_179, x_w0_180, x_w0_181, x_w0_185, x_w0_186, x_w0_187, x_w0_192, x_w0_193, x_w0_194, &
    &x_w0_195, x_w0_196, x_w0_2, x_w0_20, x_w0_202, x_w0_203, x_w0_204, x_w0_206, x_w0_207, x_w0_208, x_w0_21, &
    &x_w0_211, x_w0_212, x_w0_213, x_w0_217, x_w0_218, x_w0_219, x_w0_22, x_w0_224, x_w0_225, x_w0_226, x_w0_227, &
    &x_w0_228, x_w0_234, x_w0_235, x_w0_236, x_w0_238, x_w0_239, x_w0_240, x_w0_243, x_w0_244, x_w0_245, x_w0_249, &
    &x_w0_250, x_w0_251, x_w0_256, x_w0_257, x_w0_258, x_w0_259, x_w0_260, x_w0_266, x_w0_267, x_w0_268, x_w0_27, &
    &x_w0_270, x_w0_271, x_w0_272, x_w0_275, x_w0_276, x_w0_277, x_w0_28, x_w0_281, x_w0_282, x_w0_283, x_w0_288, &
    &x_w0_289, x_w0_29, x_w0_290, x_w0_291, x_w0_292, x_w0_298, x_w0_299, x_w0_30, x_w0_302, x_w0_306, x_w0_31, &
    &x_w0_310, x_w0_314, x_w0_318, x_w0_322, x_w0_325, x_w0_327, x_w0_329, x_w0_331, x_w0_333, x_w0_335, x_w0_336, &
    &x_w0_337, x_w0_339, x_w0_340, x_w0_341, x_w0_344, x_w0_345, x_w0_346, x_w0_350, x_w0_351, x_w0_352, x_w0_357, &
    &x_w0_358, x_w0_359, x_w0_360, x_w0_361, x_w0_367, x_w0_368, x_w0_369, x_w0_37, x_w0_371, x_w0_372, x_w0_373, &
    &x_w0_376, x_w0_377, x_w0_378, x_w0_38, x_w0_382, x_w0_383, x_w0_384, x_w0_389, x_w0_39, x_w0_390, x_w0_391, &
    &x_w0_392, x_w0_393, x_w0_399, x_w0_4, x_w0_400, x_w0_401, x_w0_403, x_w0_404, x_w0_405, x_w0_408, x_w0_409, &
    &x_w0_41, x_w0_410, x_w0_414, x_w0_415, x_w0_416, x_w0_42, x_w0_421, x_w0_422, x_w0_423, x_w0_424, x_w0_425, &
    &x_w0_43, x_w0_431, x_w0_432, x_w0_433, x_w0_435, x_w0_436, x_w0_437, x_w0_440, x_w0_441, x_w0_442, x_w0_446, &
    &x_w0_447, x_w0_448, x_w0_453, x_w0_454, x_w0_455, x_w0_456, x_w0_457, x_w0_46, x_w0_463, x_w0_464, x_w0_467, &
    &x_w0_47, x_w0_471, x_w0_475, x_w0_479, x_w0_48, x_w0_483, x_w0_487, x_w0_490, x_w0_492, x_w0_494, x_w0_496, &
    &x_w0_498, x_w0_5, x_w0_500, x_w0_502, x_w0_503, x_w0_505, x_w0_507, x_w0_511, x_w0_512, x_w0_514, x_w0_515, &
    &x_w0_517, x_w0_519, x_w0_52, x_w0_523, x_w0_524, x_w0_526, x_w0_527, x_w0_529, x_w0_53, x_w0_531, x_w0_535, &
    &x_w0_536, x_w0_538, x_w0_539, x_w0_54, x_w0_541, x_w0_543, x_w0_547, x_w0_548, x_w0_550, x_w0_551, x_w0_553, &
    &x_w0_555, x_w0_559, x_w0_560, x_w0_562, x_w0_563, x_w0_565, x_w0_567, x_w0_571, x_w0_572, x_w0_574, x_w0_576, &
    &x_w0_589, x_w0_59, x_w0_592, x_w0_595, x_w0_6, x_w0_60, x_w0_601, x_w0_602, x_w0_604, x_w0_606, x_w0_61, &
    &x_w0_619, x_w0_62, x_w0_622, x_w0_625, x_w0_63, x_w0_631, x_w0_632, x_w0_634, x_w0_636, x_w0_649, x_w0_652, &
    &x_w0_655, x_w0_661, x_w0_662, x_w0_664, x_w0_666, x_w0_679, x_w0_682, x_w0_685, x_w0_69, x_w0_691, x_w0_692, &
    &x_w0_694, x_w0_696, x_w0_7, x_w0_70, x_w0_709, x_w0_71, x_w0_712, x_w0_715, x_w0_721, x_w0_722, x_w0_724, &
    &x_w0_726, x_w0_73, x_w0_739, x_w0_74, x_w0_742, x_w0_745, x_w0_75, x_w0_751, x_w0_752, x_w0_754, x_w0_756, &
    &x_w0_769, x_w0_772, x_w0_775, x_w0_78, x_w0_781, x_w0_783, x_w0_785, x_w0_79, x_w0_798, x_w0_80, x_w0_801, &
    &x_w0_804, x_w0_810, x_w0_812, x_w0_814, x_w0_827, x_w0_830, x_w0_833, x_w0_839, x_w0_84, x_w0_840, x_w0_842, &
    &x_w0_844, x_w0_85, x_w0_857, x_w0_86, x_w0_860, x_w0_863, x_w0_869, x_w0_870, x_w0_872, x_w0_874, x_w0_887, &
    &x_w0_890, x_w0_893, x_w0_899, x_w0_9, x_w0_901, x_w0_903, x_w0_91, x_w0_916, x_w0_919, x_w0_92, x_w0_922, &
    &x_w0_928, x_w0_93, x_w0_930, x_w0_932, x_w0_934, x_w0_935, x_w0_936, x_w0_937, x_w0_938, x_w0_94, x_w0_941, &
    &x_w0_943, x_w0_95, x_w0_956, x_w0_959, x_w0_962, x_w0_968, x_w0_970, x_w0_972, x_w0_985, x_w0_988, x_w0_991, &
    &x_w0_997, x_w0_998, x_w0_999, x_w1_1017, x_w1_1020, x_w1_1023, x_w1_1046, x_w1_1049, x_w1_1052, x_w1_1077, &
    &x_w1_1080, x_w1_1083, x_w1_1106, x_w1_1109, x_w1_1112, x_w1_1137, x_w1_1140, x_w1_1143, x_w1_1166, x_w1_1169, &
    &x_w1_1172, x_w1_1197, x_w1_1200, x_w1_1203, x_w1_1226, x_w1_1229, x_w1_1232, x_w1_1257, x_w1_1260, x_w1_1263, &
    &x_w1_1286, x_w1_1289, x_w1_1292, x_w1_1311, x_w1_1313, x_w1_1315, x_w1_1322, x_w1_1324, x_w1_1326, x_w1_1333, &
    &x_w1_1335, x_w1_1337, x_w1_1345, x_w1_1347, x_w1_1349, x_w1_1357, x_w1_1359, x_w1_1361, x_w1_1368, x_w1_1370, &
    &x_w1_1372, x_w1_138, x_w1_1389, x_w1_1391, x_w1_1393, x_w1_1400, x_w1_1402, x_w1_1404, x_w1_1411, x_w1_1413, &
    &x_w1_1415, x_w1_142, x_w1_1422, x_w1_1424, x_w1_1426, x_w1_1433, x_w1_1435, x_w1_1437, x_w1_1444, x_w1_1446, &
    &x_w1_1448, x_w1_146, x_w1_150, x_w1_1503, x_w1_1507, x_w1_1511, x_w1_154, x_w1_1551, x_w1_1555, x_w1_1559, &
    &x_w1_158, x_w1_1599, x_w1_1603, x_w1_1607, x_w1_1647, x_w1_1651, x_w1_1655, x_w1_1695, x_w1_1699, x_w1_1703, &
    &x_w1_1743, x_w1_1747, x_w1_1751, x_w1_303, x_w1_307, x_w1_311, x_w1_315, x_w1_319, x_w1_323, x_w1_468, x_w1_472, &
    &x_w1_476, x_w1_480, x_w1_484, x_w1_488, x_w1_504, x_w1_506, x_w1_508, x_w1_516, x_w1_518, x_w1_520, x_w1_528, &
    &x_w1_530, x_w1_532, x_w1_540, x_w1_542, x_w1_544, x_w1_552, x_w1_554, x_w1_556, x_w1_564, x_w1_566, x_w1_568, &
    &x_w1_590, x_w1_593, x_w1_596, x_w1_620, x_w1_623, x_w1_626, x_w1_650, x_w1_653, x_w1_656, x_w1_680, x_w1_683, &
    &x_w1_686, x_w1_710, x_w1_713, x_w1_716, x_w1_740, x_w1_743, x_w1_746, x_w1_770, x_w1_773, x_w1_776, x_w1_799, &
    &x_w1_802, x_w1_805, x_w1_828, x_w1_831, x_w1_834, x_w1_858, x_w1_861, x_w1_864, x_w1_888, x_w1_891, x_w1_894, &
    &x_w1_917, x_w1_920, x_w1_923, x_w1_957, x_w1_960, x_w1_963, x_w1_986, x_w1_989, x_w1_992, x_w2_1018, x_w2_1021, &
    &x_w2_1024, x_w2_1047, x_w2_1050, x_w2_1053, x_w2_1078, x_w2_1081, x_w2_1084, x_w2_1107, x_w2_1110, x_w2_1113, &
    &x_w2_1138, x_w2_1141, x_w2_1144, x_w2_1167, x_w2_1170, x_w2_1173, x_w2_1198, x_w2_1201, x_w2_1204, x_w2_1227, &
    &x_w2_1230, x_w2_1233, x_w2_1258, x_w2_1261, x_w2_1264, x_w2_1287, x_w2_1290, x_w2_1293, x_w2_1504, x_w2_1508, &
    &x_w2_1512, x_w2_1552, x_w2_1556, x_w2_1560, x_w2_1600, x_w2_1604, x_w2_1608, x_w2_1648, x_w2_1652, x_w2_1656, &
    &x_w2_1696, x_w2_1700, x_w2_1704, x_w2_1744, x_w2_1748, x_w2_1752, x_w2_591, x_w2_594, x_w2_597, x_w2_621, &
    &x_w2_624, x_w2_627, x_w2_651, x_w2_654, x_w2_657, x_w2_681, x_w2_684, x_w2_687, x_w2_711, x_w2_714, x_w2_717, &
    &x_w2_741, x_w2_744, x_w2_747, x_w2_771, x_w2_774, x_w2_777, x_w2_800, x_w2_803, x_w2_806, x_w2_829, x_w2_832, &
    &x_w2_835, x_w2_859, x_w2_862, x_w2_865, x_w2_889, x_w2_892, x_w2_895, x_w2_918, x_w2_921, x_w2_924, x_w2_958, &
    &x_w2_961, x_w2_964, x_w2_987, x_w2_990, x_w2_993, x_w3_1505, x_w3_1509, x_w3_1513, x_w3_1553, x_w3_1557, &
    &x_w3_1561, x_w3_1601, x_w3_1605, x_w3_1609, x_w3_1649, x_w3_1653, x_w3_1657, x_w3_1697, x_w3_1701, x_w3_1705, &
    &x_w3_1745, x_w3_1749, x_w3_1753
    integer(c_int64_t) :: o
    integer(c_int64_t) :: gal
    integer(c_int64_t) :: g
    integer(c_int64_t) :: nmodes
    integer(c_int64_t) :: x_inl1_o
    integer(c_int64_t) :: x_inl1_og
    integer(c_int32_t) :: x_inl1_lox
    integer(c_int32_t) :: x_inl1_loy
    integer(c_int32_t) :: x_inl1_loz
    integer(c_int64_t) :: x_inl1_zdir
    integer(c_int64_t) :: x_inl1_n_sx_ex
    integer(c_int64_t) :: x_inl1_n_sx_by
    integer(c_int64_t) :: x_inl1_n_sx_bz
    integer(c_int64_t) :: x_inl1_n_sx_ey
    integer(c_int64_t) :: x_inl1_n_sx_ez
    integer(c_int64_t) :: x_inl1_n_sx_bx
    integer(c_int64_t) :: x_inl1_n_sy_ey
    integer(c_int64_t) :: x_inl1_n_sy_bx
    integer(c_int64_t) :: x_inl1_n_sy_bz
    integer(c_int64_t) :: x_inl1_n_sy_ex
    integer(c_int64_t) :: x_inl1_n_sy_ez
    integer(c_int64_t) :: x_inl1_n_sy_by
    integer(c_int64_t) :: x_inl1_n_sz_ez
    integer(c_int64_t) :: x_inl1_n_sz_bx
    integer(c_int64_t) :: x_inl1_n_sz_by
    integer(c_int64_t) :: x_inl1_n_sz_ex
    integer(c_int64_t) :: x_inl1_n_sz_ey
    integer(c_int64_t) :: x_inl1_n_sz_bz
    integer(c_int64_t) :: x_ifexp6
    integer(c_int64_t) :: x_ifexp7
    integer(c_int64_t) :: x_ifexp8
    integer(c_int64_t) :: x_ifexp9
    integer(c_int64_t) :: x_ifexp10
    integer(c_int64_t) :: x_ifexp11
    integer(c_int64_t) :: x_ifexp18
    integer(c_int64_t) :: x_ifexp19
    integer(c_int64_t) :: x_ifexp20
    integer(c_int64_t) :: x_ifexp21
    integer(c_int64_t) :: x_ifexp22
    integer(c_int64_t) :: x_ifexp23
    integer(c_int64_t) :: x_ifexp30
    integer(c_int64_t) :: x_ifexp31
    integer(c_int64_t) :: x_ifexp32
    integer(c_int64_t) :: x_ifexp33
    integer(c_int64_t) :: x_ifexp34
    integer(c_int64_t) :: x_ifexp35
    real(c_double) :: x_ifexp0
    real(c_double) :: x_ifexp1
    real(c_double) :: x_ifexp2
    real(c_double) :: x_ifexp3
    real(c_double) :: x_ifexp4
    real(c_double) :: x_ifexp5
    real(c_double) :: x_ifexp12
    real(c_double) :: x_ifexp13
    real(c_double) :: x_ifexp14
    real(c_double) :: x_ifexp15
    real(c_double) :: x_ifexp16
    real(c_double) :: x_ifexp17
    real(c_double) :: x_ifexp24
    real(c_double) :: x_ifexp25
    real(c_double) :: x_ifexp26
    real(c_double) :: x_ifexp27
    real(c_double) :: x_ifexp28
    real(c_double) :: x_ifexp29
    real(c_double) :: x_ifexp36
    real(c_double) :: x_ifexp37
    real(c_double) :: x_ifexp38
    real(c_double) :: x_ifexp39
    real(c_double) :: x_ifexp40
    real(c_double) :: x_ifexp41
    real(c_double) :: x_ifexp42
    real(c_double) :: x_ifexp43
    real(c_double) :: x_ifexp44
    real(c_double) :: x_ifexp45
    real(c_double) :: x_ifexp46
    real(c_double) :: x_ifexp47
    real(c_double), allocatable :: x_inl1_sx_node(:, :)
    real(c_double), allocatable :: x_inl1_sx_cell(:, :)
    real(c_double), allocatable :: x_inl1_sx_node_g(:, :)
    real(c_double), allocatable :: x_inl1_sx_cell_g(:, :)
    integer(c_int64_t) :: x_inl1_j_node(np_particles)
    integer(c_int64_t) :: x_inl1_j_cell(np_particles)
    integer(c_int64_t) :: x_inl1_j_node_v(np_particles)
    integer(c_int64_t) :: x_inl1_j_cell_v(np_particles)
    integer(c_int64_t) :: x_inl2_idx(np_particles)
    integer(c_int64_t) :: x_inl3_idx(np_particles)
    integer(c_int64_t) :: x_inl4_idx(np_particles)
    integer(c_int64_t) :: x_inl5_idx(np_particles)
    real(c_double), allocatable :: x_inl1_sy_node(:, :)
    real(c_double), allocatable :: x_inl1_sy_cell(:, :)
    real(c_double), allocatable :: x_inl1_sy_node_v(:, :)
    real(c_double), allocatable :: x_inl1_sy_cell_v(:, :)
    integer(c_int64_t) :: x_inl1_k_node(np_particles)
    integer(c_int64_t) :: x_inl1_k_cell(np_particles)
    integer(c_int64_t) :: x_inl1_k_node_v(np_particles)
    integer(c_int64_t) :: x_inl1_k_cell_v(np_particles)
    integer(c_int64_t) :: x_inl6_idx(np_particles)
    integer(c_int64_t) :: x_inl7_idx(np_particles)
    integer(c_int64_t) :: x_inl8_idx(np_particles)
    integer(c_int64_t) :: x_inl9_idx(np_particles)
    real(c_double), allocatable :: x_inl1_sz_node(:, :)
    real(c_double), allocatable :: x_inl1_sz_cell(:, :)
    real(c_double), allocatable :: x_inl1_sz_node_v(:, :)
    real(c_double), allocatable :: x_inl1_sz_cell_v(:, :)
    integer(c_int64_t) :: x_inl1_l_node(np_particles)
    integer(c_int64_t) :: x_inl1_l_cell(np_particles)
    integer(c_int64_t) :: x_inl1_l_node_v(np_particles)
    integer(c_int64_t) :: x_inl1_l_cell_v(np_particles)
    integer(c_int64_t) :: x_inl10_idx(np_particles)
    integer(c_int64_t) :: x_inl11_idx(np_particles)
    integer(c_int64_t) :: x_inl12_idx(np_particles)
    integer(c_int64_t) :: x_inl13_idx(np_particles)
    integer(c_int64_t), allocatable :: x_inl20_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl20_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl21_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl21_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl22_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl22_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl23_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl23_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl24_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl24_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl25_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl25_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl26_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl26_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl27_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl27_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl28_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl28_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl29_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl29_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl30_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl30_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl31_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl31_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl32_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl32_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl33_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl33_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl34_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl34_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl35_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl35_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl36_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl36_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl37_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl37_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl38_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl38_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl39_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl39_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl40_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl40_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl41_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl41_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl42_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl42_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl43_ia_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl43_ib_b(:, :, :)
    integer(c_int64_t), allocatable :: x_inl56_ix_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl56_iy_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl56_iz_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl57_ix_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl57_iy_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl57_iz_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl58_ix_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl58_iy_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl58_iz_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl59_ix_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl59_iy_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl59_iz_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl60_ix_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl60_iy_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl60_iz_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl61_ix_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl61_iy_b(:, :, :, :)
    integer(c_int64_t), allocatable :: x_inl61_iz_b(:, :, :, :)
    real(c_double) :: x_cb1(np_particles)
    real(c_double) :: x_cb2(np_particles)
    real(c_double), allocatable :: x_cb3(:, :)
    real(c_double), allocatable :: x_cb4(:, :)
    real(c_double), allocatable :: x_cb5(:, :)
    real(c_double), allocatable :: x_cb6(:, :)
    real(c_double), allocatable :: x_cb7(:, :)
    real(c_double), allocatable :: x_cb8(:, :)
    integer(c_int64_t) :: x_cb9(np_particles)
    integer(c_int64_t) :: x_cb10(np_particles)
    integer(c_int64_t) :: x_cb11(np_particles)
    integer(c_int64_t) :: x_cb12(np_particles)
    integer(c_int64_t) :: x_cb13(np_particles)
    integer(c_int64_t) :: x_cb14(np_particles)
    real(c_double), allocatable :: x_cb15(:, :)
    real(c_double), allocatable :: x_cb16(:, :)
    real(c_double), allocatable :: x_cb17(:, :)
    real(c_double), allocatable :: x_cb18(:, :)
    real(c_double), allocatable :: x_cb19(:, :)
    real(c_double), allocatable :: x_cb20(:, :)
    integer(c_int64_t) :: x_cb21(np_particles)
    integer(c_int64_t) :: x_cb22(np_particles)
    integer(c_int64_t) :: x_cb23(np_particles)
    integer(c_int64_t) :: x_cb24(np_particles)
    integer(c_int64_t) :: x_cb25(np_particles)
    integer(c_int64_t) :: x_cb26(np_particles)
    real(c_double), allocatable :: x_cb27(:, :)
    real(c_double), allocatable :: x_cb28(:, :)
    real(c_double), allocatable :: x_cb29(:, :)
    real(c_double), allocatable :: x_cb30(:, :)
    real(c_double), allocatable :: x_cb31(:, :)
    real(c_double), allocatable :: x_cb32(:, :)
    integer(c_int64_t) :: x_cb33(np_particles)
    integer(c_int64_t) :: x_cb34(np_particles)
    integer(c_int64_t) :: x_cb35(np_particles)
    integer(c_int64_t) :: x_cb36(np_particles)
    integer(c_int64_t) :: x_cb37(np_particles)
    integer(c_int64_t) :: x_cb38(np_particles)
    integer(c_int64_t), allocatable :: x_cb39(:)
    real(c_double), allocatable :: x_cb40(:, :)
    real(c_double) :: x_cb41(np_particles)
    integer(c_int64_t), allocatable :: x_cb42(:)
    real(c_double), allocatable :: x_cb43(:, :)
    real(c_double) :: x_cb44(np_particles)
    integer(c_int64_t), allocatable :: x_cb45(:)
    real(c_double), allocatable :: x_cb46(:, :)
    real(c_double) :: x_cb47(np_particles)
    integer(c_int64_t), allocatable :: x_cb48(:)
    real(c_double), allocatable :: x_cb49(:, :)
    real(c_double) :: x_cb50(np_particles)
    integer(c_int64_t), allocatable :: x_cb51(:)
    real(c_double), allocatable :: x_cb52(:, :)
    real(c_double) :: x_cb53(np_particles)
    integer(c_int64_t), allocatable :: x_cb54(:)
    real(c_double), allocatable :: x_cb55(:, :)
    real(c_double) :: x_cb56(np_particles)
    integer(c_int64_t), allocatable :: x_cb57(:)
    integer(c_int64_t), allocatable :: x_cb58(:)
    real(c_double), allocatable :: x_cb59(:, :, :)
    real(c_double) :: x_cb60(np_particles)
    integer(c_int64_t), allocatable :: x_cb61(:)
    integer(c_int64_t), allocatable :: x_cb62(:)
    real(c_double), allocatable :: x_cb63(:, :, :)
    real(c_double) :: x_cb64(np_particles)
    integer(c_int64_t), allocatable :: x_cb65(:)
    integer(c_int64_t), allocatable :: x_cb66(:)
    real(c_double), allocatable :: x_cb67(:, :, :)
    real(c_double) :: x_cb68(np_particles)
    integer(c_int64_t), allocatable :: x_cb69(:)
    integer(c_int64_t), allocatable :: x_cb70(:)
    real(c_double), allocatable :: x_cb71(:, :, :)
    real(c_double) :: x_cb72(np_particles)
    integer(c_int64_t), allocatable :: x_cb73(:)
    integer(c_int64_t), allocatable :: x_cb74(:)
    real(c_double), allocatable :: x_cb75(:, :, :)
    real(c_double) :: x_cb76(np_particles)
    integer(c_int64_t), allocatable :: x_cb77(:)
    integer(c_int64_t), allocatable :: x_cb78(:)
    real(c_double), allocatable :: x_cb79(:, :, :)
    real(c_double) :: x_cb80(np_particles)
    integer(c_int64_t), allocatable :: x_cb81(:)
    integer(c_int64_t), allocatable :: x_cb82(:)
    real(c_double), allocatable :: x_cb83(:, :, :)
    real(c_double) :: x_cb84(np_particles)
    integer(c_int64_t), allocatable :: x_cb85(:)
    integer(c_int64_t), allocatable :: x_cb86(:)
    real(c_double), allocatable :: x_cb87(:, :, :)
    real(c_double) :: x_cb88(np_particles)
    integer(c_int64_t), allocatable :: x_cb89(:)
    integer(c_int64_t), allocatable :: x_cb90(:)
    real(c_double), allocatable :: x_cb91(:, :, :)
    real(c_double) :: x_cb92(np_particles)
    integer(c_int64_t), allocatable :: x_cb93(:)
    integer(c_int64_t), allocatable :: x_cb94(:)
    real(c_double), allocatable :: x_cb95(:, :, :)
    real(c_double) :: x_cb96(np_particles)
    integer(c_int64_t), allocatable :: x_cb97(:)
    integer(c_int64_t), allocatable :: x_cb98(:)
    real(c_double), allocatable :: x_cb99(:, :, :)
    real(c_double) :: x_cb100(np_particles)
    integer(c_int64_t), allocatable :: x_cb101(:)
    integer(c_int64_t), allocatable :: x_cb102(:)
    real(c_double), allocatable :: x_cb103(:, :, :)
    real(c_double) :: x_cb104(np_particles)
    real(c_double) :: x_cb105(np_particles)
    real(c_double) :: x_cb106(np_particles)
    real(c_double) :: x_cb107(np_particles)
    integer(c_int64_t), allocatable :: x_cb108(:)
    integer(c_int64_t), allocatable :: x_cb109(:)
    real(c_double), allocatable :: x_cb110(:, :, :)
    real(c_double) :: x_cb111(np_particles)
    integer(c_int64_t), allocatable :: x_cb112(:)
    integer(c_int64_t), allocatable :: x_cb113(:)
    real(c_double), allocatable :: x_cb114(:, :, :)
    real(c_double) :: x_cb115(np_particles)
    integer(c_int64_t), allocatable :: x_cb116(:)
    integer(c_int64_t), allocatable :: x_cb117(:)
    real(c_double), allocatable :: x_cb118(:, :, :)
    real(c_double) :: x_cb119(np_particles)
    integer(c_int64_t), allocatable :: x_cb120(:)
    integer(c_int64_t), allocatable :: x_cb121(:)
    real(c_double), allocatable :: x_cb122(:, :, :)
    real(c_double) :: x_cb123(np_particles)
    integer(c_int64_t), allocatable :: x_cb124(:)
    integer(c_int64_t), allocatable :: x_cb125(:)
    real(c_double), allocatable :: x_cb126(:, :, :)
    real(c_double) :: x_cb127(np_particles)
    integer(c_int64_t), allocatable :: x_cb128(:)
    integer(c_int64_t), allocatable :: x_cb129(:)
    real(c_double), allocatable :: x_cb130(:, :, :)
    real(c_double) :: x_cb131(np_particles)
    integer(c_int64_t), allocatable :: x_cb132(:)
    integer(c_int64_t), allocatable :: x_cb133(:)
    real(c_double), allocatable :: x_cb134(:, :, :)
    real(c_double) :: x_cb135(np_particles)
    integer(c_int64_t), allocatable :: x_cb136(:)
    integer(c_int64_t), allocatable :: x_cb137(:)
    real(c_double), allocatable :: x_cb138(:, :, :)
    real(c_double) :: x_cb139(np_particles)
    integer(c_int64_t), allocatable :: x_cb140(:)
    integer(c_int64_t), allocatable :: x_cb141(:)
    real(c_double), allocatable :: x_cb142(:, :, :)
    real(c_double) :: x_cb143(np_particles)
    integer(c_int64_t), allocatable :: x_cb144(:)
    integer(c_int64_t), allocatable :: x_cb145(:)
    real(c_double), allocatable :: x_cb146(:, :, :)
    real(c_double) :: x_cb147(np_particles)
    integer(c_int64_t), allocatable :: x_cb148(:)
    integer(c_int64_t), allocatable :: x_cb149(:)
    real(c_double), allocatable :: x_cb150(:, :, :)
    real(c_double) :: x_cb151(np_particles)
    integer(c_int64_t), allocatable :: x_cb152(:)
    integer(c_int64_t), allocatable :: x_cb153(:)
    real(c_double), allocatable :: x_cb154(:, :, :)
    real(c_double) :: x_cb155(np_particles)
    integer(c_int64_t), allocatable :: x_cb156(:)
    real(c_double), allocatable :: x_cb157(:, :)
    real(c_double) :: x_cb158(np_particles)
    integer(c_int64_t), allocatable :: x_cb159(:)
    real(c_double), allocatable :: x_cb160(:, :)
    real(c_double) :: x_cb161(np_particles)
    integer(c_int64_t), allocatable :: x_cb162(:)
    real(c_double), allocatable :: x_cb163(:, :)
    real(c_double) :: x_cb164(np_particles)
    integer(c_int64_t), allocatable :: x_cb165(:)
    real(c_double), allocatable :: x_cb166(:, :)
    real(c_double) :: x_cb167(np_particles)
    integer(c_int64_t), allocatable :: x_cb168(:)
    real(c_double), allocatable :: x_cb169(:, :)
    real(c_double) :: x_cb170(np_particles)
    integer(c_int64_t), allocatable :: x_cb171(:)
    real(c_double), allocatable :: x_cb172(:, :)
    real(c_double) :: x_cb173(np_particles)
    real(c_double) :: x_cb174(np_particles)
    real(c_double) :: x_cb175(np_particles)
    real(c_double) :: x_cb176(np_particles)
    integer(c_int64_t), allocatable :: x_cb177(:)
    real(c_double), allocatable :: x_cb178(:, :)
    real(c_double) :: x_cb179(np_particles)
    integer(c_int64_t), allocatable :: x_cb180(:)
    real(c_double), allocatable :: x_cb181(:, :)
    real(c_double) :: x_cb182(np_particles)
    integer(c_int64_t), allocatable :: x_cb183(:)
    real(c_double), allocatable :: x_cb184(:, :)
    real(c_double) :: x_cb185(np_particles)
    integer(c_int64_t), allocatable :: x_cb186(:)
    real(c_double), allocatable :: x_cb187(:, :)
    real(c_double) :: x_cb188(np_particles)
    integer(c_int64_t), allocatable :: x_cb189(:)
    real(c_double), allocatable :: x_cb190(:, :)
    real(c_double) :: x_cb191(np_particles)
    integer(c_int64_t), allocatable :: x_cb192(:)
    real(c_double), allocatable :: x_cb193(:, :)
    real(c_double) :: x_cb194(np_particles)
    real(c_double) :: x_cb195(np_particles)
    real(c_double) :: x_cb196(np_particles)
    real(c_double) :: x_cb197(np_particles)
    real(c_double) :: x_cb198(np_particles)
    real(c_double) :: x_cb199(np_particles)
    real(c_double) :: x_cb200(np_particles)
    real(c_double) :: x_cb201(np_particles)
    integer(c_int64_t), allocatable :: x_cb202(:)
    integer(c_int64_t), allocatable :: x_cb203(:)
    integer(c_int64_t), allocatable :: x_cb204(:)
    real(c_double), allocatable :: x_cb205(:, :, :, :)
    real(c_double) :: x_cb206(np_particles)
    integer(c_int64_t), allocatable :: x_cb207(:)
    integer(c_int64_t), allocatable :: x_cb208(:)
    integer(c_int64_t), allocatable :: x_cb209(:)
    real(c_double), allocatable :: x_cb210(:, :, :, :)
    real(c_double) :: x_cb211(np_particles)
    integer(c_int64_t), allocatable :: x_cb212(:)
    integer(c_int64_t), allocatable :: x_cb213(:)
    integer(c_int64_t), allocatable :: x_cb214(:)
    real(c_double), allocatable :: x_cb215(:, :, :, :)
    real(c_double) :: x_cb216(np_particles)
    integer(c_int64_t), allocatable :: x_cb217(:)
    integer(c_int64_t), allocatable :: x_cb218(:)
    integer(c_int64_t), allocatable :: x_cb219(:)
    real(c_double), allocatable :: x_cb220(:, :, :, :)
    real(c_double) :: x_cb221(np_particles)
    integer(c_int64_t), allocatable :: x_cb222(:)
    integer(c_int64_t), allocatable :: x_cb223(:)
    integer(c_int64_t), allocatable :: x_cb224(:)
    real(c_double), allocatable :: x_cb225(:, :, :, :)
    real(c_double) :: x_cb226(np_particles)
    integer(c_int64_t), allocatable :: x_cb227(:)
    integer(c_int64_t), allocatable :: x_cb228(:)
    integer(c_int64_t), allocatable :: x_cb229(:)
    real(c_double), allocatable :: x_cb230(:, :, :, :)
    real(c_double) :: x_cb231(np_particles)
    real(c_double) :: x_inl1_rp(np_particles)
    real(c_double) :: x_inl1_x(np_particles)
    real(c_double) :: x_inl2_xint(np_particles)
    real(c_double) :: x_inl2_sm(np_particles)
    real(c_double) :: x_inl2_sp(np_particles)
    real(c_double) :: x_inl3_xint(np_particles)
    real(c_double) :: x_inl3_sm(np_particles)
    real(c_double) :: x_inl3_sp(np_particles)
    real(c_double) :: x_inl4_xint(np_particles)
    real(c_double) :: x_inl4_sm(np_particles)
    real(c_double) :: x_inl4_sp(np_particles)
    real(c_double) :: x_inl5_xint(np_particles)
    real(c_double) :: x_inl5_sm(np_particles)
    real(c_double) :: x_inl5_sp(np_particles)
    real(c_double), allocatable :: x_inl1_sx_ex(:, :)
    real(c_double), allocatable :: x_inl1_sx_ey(:, :)
    real(c_double), allocatable :: x_inl1_sx_ez(:, :)
    real(c_double), allocatable :: x_inl1_sx_bx(:, :)
    real(c_double), allocatable :: x_inl1_sx_by(:, :)
    real(c_double), allocatable :: x_inl1_sx_bz(:, :)
    integer(c_int64_t) :: x_inl1_j_ex(np_particles)
    integer(c_int64_t) :: x_inl1_j_ey(np_particles)
    integer(c_int64_t) :: x_inl1_j_ez(np_particles)
    integer(c_int64_t) :: x_inl1_j_bx(np_particles)
    integer(c_int64_t) :: x_inl1_j_by(np_particles)
    integer(c_int64_t) :: x_inl1_j_bz(np_particles)
    real(c_double) :: x_inl1_y(np_particles)
    real(c_double) :: x_inl6_xint(np_particles)
    real(c_double) :: x_inl6_sm(np_particles)
    real(c_double) :: x_inl6_sp(np_particles)
    real(c_double) :: x_inl7_xint(np_particles)
    real(c_double) :: x_inl7_sm(np_particles)
    real(c_double) :: x_inl7_sp(np_particles)
    real(c_double) :: x_inl8_xint(np_particles)
    real(c_double) :: x_inl8_sm(np_particles)
    real(c_double) :: x_inl8_sp(np_particles)
    real(c_double) :: x_inl9_xint(np_particles)
    real(c_double) :: x_inl9_sm(np_particles)
    real(c_double) :: x_inl9_sp(np_particles)
    real(c_double), allocatable :: x_inl1_sy_ex(:, :)
    real(c_double), allocatable :: x_inl1_sy_ey(:, :)
    real(c_double), allocatable :: x_inl1_sy_ez(:, :)
    real(c_double), allocatable :: x_inl1_sy_bx(:, :)
    real(c_double), allocatable :: x_inl1_sy_by(:, :)
    real(c_double), allocatable :: x_inl1_sy_bz(:, :)
    integer(c_int64_t) :: x_inl1_k_ex(np_particles)
    integer(c_int64_t) :: x_inl1_k_ey(np_particles)
    integer(c_int64_t) :: x_inl1_k_ez(np_particles)
    integer(c_int64_t) :: x_inl1_k_bx(np_particles)
    integer(c_int64_t) :: x_inl1_k_by(np_particles)
    integer(c_int64_t) :: x_inl1_k_bz(np_particles)
    real(c_double) :: x_inl1_z(np_particles)
    real(c_double) :: x_inl10_xint(np_particles)
    real(c_double) :: x_inl10_sm(np_particles)
    real(c_double) :: x_inl10_sp(np_particles)
    real(c_double) :: x_inl11_xint(np_particles)
    real(c_double) :: x_inl11_sm(np_particles)
    real(c_double) :: x_inl11_sp(np_particles)
    real(c_double) :: x_inl12_xint(np_particles)
    real(c_double) :: x_inl12_sm(np_particles)
    real(c_double) :: x_inl12_sp(np_particles)
    real(c_double) :: x_inl13_xint(np_particles)
    real(c_double) :: x_inl13_sm(np_particles)
    real(c_double) :: x_inl13_sp(np_particles)
    real(c_double), allocatable :: x_inl1_sz_ex(:, :)
    real(c_double), allocatable :: x_inl1_sz_ey(:, :)
    real(c_double), allocatable :: x_inl1_sz_ez(:, :)
    real(c_double), allocatable :: x_inl1_sz_bx(:, :)
    real(c_double), allocatable :: x_inl1_sz_by(:, :)
    real(c_double), allocatable :: x_inl1_sz_bz(:, :)
    integer(c_int64_t) :: x_inl1_l_ex(np_particles)
    integer(c_int64_t) :: x_inl1_l_ey(np_particles)
    integer(c_int64_t) :: x_inl1_l_ez(np_particles)
    integer(c_int64_t) :: x_inl1_l_bx(np_particles)
    integer(c_int64_t) :: x_inl1_l_by(np_particles)
    integer(c_int64_t) :: x_inl1_l_bz(np_particles)
    integer(c_int64_t), allocatable :: x_inl14_taps(:)
    integer(c_int64_t), allocatable :: x_inl14_rows(:, :)
    real(c_double) :: x_hcall1(np_particles)
    integer(c_int64_t), allocatable :: x_inl15_taps(:)
    integer(c_int64_t), allocatable :: x_inl15_rows(:, :)
    real(c_double) :: x_hcall2(np_particles)
    integer(c_int64_t), allocatable :: x_inl16_taps(:)
    integer(c_int64_t), allocatable :: x_inl16_rows(:, :)
    real(c_double) :: x_hcall3(np_particles)
    integer(c_int64_t), allocatable :: x_inl17_taps(:)
    integer(c_int64_t), allocatable :: x_inl17_rows(:, :)
    real(c_double) :: x_hcall4(np_particles)
    integer(c_int64_t), allocatable :: x_inl18_taps(:)
    integer(c_int64_t), allocatable :: x_inl18_rows(:, :)
    real(c_double) :: x_hcall5(np_particles)
    integer(c_int64_t), allocatable :: x_inl19_taps(:)
    integer(c_int64_t), allocatable :: x_inl19_rows(:, :)
    real(c_double) :: x_hcall6(np_particles)
    integer(c_int64_t), allocatable :: x_inl20_ta(:)
    integer(c_int64_t), allocatable :: x_inl20_tb(:)
    real(c_double), allocatable :: x_inl20_weight(:, :, :)
    real(c_double) :: x_hcall7(np_particles)
    integer(c_int64_t), allocatable :: x_inl21_ta(:)
    integer(c_int64_t), allocatable :: x_inl21_tb(:)
    real(c_double), allocatable :: x_inl21_weight(:, :, :)
    real(c_double) :: x_hcall8(np_particles)
    integer(c_int64_t), allocatable :: x_inl22_ta(:)
    integer(c_int64_t), allocatable :: x_inl22_tb(:)
    real(c_double), allocatable :: x_inl22_weight(:, :, :)
    real(c_double) :: x_hcall9(np_particles)
    integer(c_int64_t), allocatable :: x_inl23_ta(:)
    integer(c_int64_t), allocatable :: x_inl23_tb(:)
    real(c_double), allocatable :: x_inl23_weight(:, :, :)
    real(c_double) :: x_hcall10(np_particles)
    integer(c_int64_t), allocatable :: x_inl24_ta(:)
    integer(c_int64_t), allocatable :: x_inl24_tb(:)
    real(c_double), allocatable :: x_inl24_weight(:, :, :)
    real(c_double) :: x_hcall11(np_particles)
    integer(c_int64_t), allocatable :: x_inl25_ta(:)
    integer(c_int64_t), allocatable :: x_inl25_tb(:)
    real(c_double), allocatable :: x_inl25_weight(:, :, :)
    real(c_double) :: x_hcall12(np_particles)
    integer(c_int64_t), allocatable :: x_inl26_ta(:)
    integer(c_int64_t), allocatable :: x_inl26_tb(:)
    real(c_double), allocatable :: x_inl26_weight(:, :, :)
    real(c_double) :: x_inl1_Ethetap(np_particles)
    integer(c_int64_t), allocatable :: x_inl27_ta(:)
    integer(c_int64_t), allocatable :: x_inl27_tb(:)
    real(c_double), allocatable :: x_inl27_weight(:, :, :)
    real(c_double) :: x_inl1_Erp(np_particles)
    integer(c_int64_t), allocatable :: x_inl28_ta(:)
    integer(c_int64_t), allocatable :: x_inl28_tb(:)
    real(c_double), allocatable :: x_inl28_weight(:, :, :)
    real(c_double) :: x_hcall13(np_particles)
    integer(c_int64_t), allocatable :: x_inl29_ta(:)
    integer(c_int64_t), allocatable :: x_inl29_tb(:)
    real(c_double), allocatable :: x_inl29_weight(:, :, :)
    real(c_double) :: x_hcall14(np_particles)
    integer(c_int64_t), allocatable :: x_inl30_ta(:)
    integer(c_int64_t), allocatable :: x_inl30_tb(:)
    real(c_double), allocatable :: x_inl30_weight(:, :, :)
    real(c_double) :: x_inl1_Brp(np_particles)
    integer(c_int64_t), allocatable :: x_inl31_ta(:)
    integer(c_int64_t), allocatable :: x_inl31_tb(:)
    real(c_double), allocatable :: x_inl31_weight(:, :, :)
    real(c_double) :: x_inl1_Bthetap(np_particles)
    real(c_double) :: x_inl1_rp_safe(np_particles)
    real(c_double) :: x_inl1_costheta(np_particles)
    real(c_double) :: x_inl1_sintheta(np_particles)
    real(c_double) :: x_inl1_xy0_re(np_particles)
    real(c_double) :: x_inl1_xy0_im(np_particles)
    real(c_double) :: x_inl1_xy_re(np_particles)
    real(c_double) :: x_inl1_xy_im(np_particles)
    integer(c_int64_t), allocatable :: x_inl32_ta(:)
    integer(c_int64_t), allocatable :: x_inl32_tb(:)
    real(c_double), allocatable :: x_inl32_weight(:, :, :)
    real(c_double) :: x_hcall15(np_particles)
    integer(c_int64_t), allocatable :: x_inl33_ta(:)
    integer(c_int64_t), allocatable :: x_inl33_tb(:)
    real(c_double), allocatable :: x_inl33_weight(:, :, :)
    real(c_double) :: x_hcall16(np_particles)
    real(c_double) :: x_inl1_dEy(np_particles)
    integer(c_int64_t), allocatable :: x_inl34_ta(:)
    integer(c_int64_t), allocatable :: x_inl34_tb(:)
    real(c_double), allocatable :: x_inl34_weight(:, :, :)
    real(c_double) :: x_hcall17(np_particles)
    integer(c_int64_t), allocatable :: x_inl35_ta(:)
    integer(c_int64_t), allocatable :: x_inl35_tb(:)
    real(c_double), allocatable :: x_inl35_weight(:, :, :)
    real(c_double) :: x_hcall18(np_particles)
    real(c_double) :: x_inl1_dEx(np_particles)
    integer(c_int64_t), allocatable :: x_inl36_ta(:)
    integer(c_int64_t), allocatable :: x_inl36_tb(:)
    real(c_double), allocatable :: x_inl36_weight(:, :, :)
    real(c_double) :: x_hcall19(np_particles)
    integer(c_int64_t), allocatable :: x_inl37_ta(:)
    integer(c_int64_t), allocatable :: x_inl37_tb(:)
    real(c_double), allocatable :: x_inl37_weight(:, :, :)
    real(c_double) :: x_hcall20(np_particles)
    real(c_double) :: x_inl1_dBz(np_particles)
    integer(c_int64_t), allocatable :: x_inl38_ta(:)
    integer(c_int64_t), allocatable :: x_inl38_tb(:)
    real(c_double), allocatable :: x_inl38_weight(:, :, :)
    real(c_double) :: x_hcall21(np_particles)
    integer(c_int64_t), allocatable :: x_inl39_ta(:)
    integer(c_int64_t), allocatable :: x_inl39_tb(:)
    real(c_double), allocatable :: x_inl39_weight(:, :, :)
    real(c_double) :: x_hcall22(np_particles)
    real(c_double) :: x_inl1_dEz(np_particles)
    integer(c_int64_t), allocatable :: x_inl40_ta(:)
    integer(c_int64_t), allocatable :: x_inl40_tb(:)
    real(c_double), allocatable :: x_inl40_weight(:, :, :)
    real(c_double) :: x_hcall23(np_particles)
    integer(c_int64_t), allocatable :: x_inl41_ta(:)
    integer(c_int64_t), allocatable :: x_inl41_tb(:)
    real(c_double), allocatable :: x_inl41_weight(:, :, :)
    real(c_double) :: x_hcall24(np_particles)
    real(c_double) :: x_inl1_dBx(np_particles)
    integer(c_int64_t), allocatable :: x_inl42_ta(:)
    integer(c_int64_t), allocatable :: x_inl42_tb(:)
    real(c_double), allocatable :: x_inl42_weight(:, :, :)
    real(c_double) :: x_hcall25(np_particles)
    integer(c_int64_t), allocatable :: x_inl43_ta(:)
    integer(c_int64_t), allocatable :: x_inl43_tb(:)
    real(c_double), allocatable :: x_inl43_weight(:, :, :)
    real(c_double) :: x_hcall26(np_particles)
    real(c_double) :: x_inl1_dBy(np_particles)
    real(c_double) :: x_inl1_tmp_re(np_particles)
    real(c_double) :: x_inl1_tmp_im(np_particles)
    integer(c_int64_t), allocatable :: x_inl44_taps(:)
    integer(c_int64_t), allocatable :: x_inl44_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl45_taps(:)
    integer(c_int64_t), allocatable :: x_inl45_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl46_taps(:)
    integer(c_int64_t), allocatable :: x_inl46_rows(:, :)
    real(c_double) :: x_hcall27(np_particles)
    integer(c_int64_t), allocatable :: x_inl47_taps(:)
    integer(c_int64_t), allocatable :: x_inl47_rows(:, :)
    real(c_double) :: x_hcall28(np_particles)
    integer(c_int64_t), allocatable :: x_inl48_taps(:)
    integer(c_int64_t), allocatable :: x_inl48_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl49_taps(:)
    integer(c_int64_t), allocatable :: x_inl49_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl50_taps(:)
    integer(c_int64_t), allocatable :: x_inl50_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl51_taps(:)
    integer(c_int64_t), allocatable :: x_inl51_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl52_taps(:)
    integer(c_int64_t), allocatable :: x_inl52_rows(:, :)
    real(c_double) :: x_inl1_Bphip(np_particles)
    integer(c_int64_t), allocatable :: x_inl53_taps(:)
    integer(c_int64_t), allocatable :: x_inl53_rows(:, :)
    real(c_double) :: x_inl1_Ephip(np_particles)
    integer(c_int64_t), allocatable :: x_inl54_taps(:)
    integer(c_int64_t), allocatable :: x_inl54_rows(:, :)
    integer(c_int64_t), allocatable :: x_inl55_taps(:)
    integer(c_int64_t), allocatable :: x_inl55_rows(:, :)
    real(c_double) :: x_inl1_rpxy(np_particles)
    real(c_double) :: x_inl1_rpxy_safe(np_particles)
    real(c_double) :: x_inl1_cosphi(np_particles)
    real(c_double) :: x_inl1_sinphi(np_particles)
    integer(c_int64_t), allocatable :: x_inl56_tx(:)
    integer(c_int64_t), allocatable :: x_inl56_ty(:)
    integer(c_int64_t), allocatable :: x_inl56_tz(:)
    real(c_double), allocatable :: x_inl56_weight(:, :, :, :)
    real(c_double) :: x_hcall29(np_particles)
    integer(c_int64_t), allocatable :: x_inl57_tx(:)
    integer(c_int64_t), allocatable :: x_inl57_ty(:)
    integer(c_int64_t), allocatable :: x_inl57_tz(:)
    real(c_double), allocatable :: x_inl57_weight(:, :, :, :)
    real(c_double) :: x_hcall30(np_particles)
    integer(c_int64_t), allocatable :: x_inl58_tx(:)
    integer(c_int64_t), allocatable :: x_inl58_ty(:)
    integer(c_int64_t), allocatable :: x_inl58_tz(:)
    real(c_double), allocatable :: x_inl58_weight(:, :, :, :)
    real(c_double) :: x_hcall31(np_particles)
    integer(c_int64_t), allocatable :: x_inl59_tx(:)
    integer(c_int64_t), allocatable :: x_inl59_ty(:)
    integer(c_int64_t), allocatable :: x_inl59_tz(:)
    real(c_double), allocatable :: x_inl59_weight(:, :, :, :)
    real(c_double) :: x_hcall32(np_particles)
    integer(c_int64_t), allocatable :: x_inl60_tx(:)
    integer(c_int64_t), allocatable :: x_inl60_ty(:)
    integer(c_int64_t), allocatable :: x_inl60_tz(:)
    real(c_double), allocatable :: x_inl60_weight(:, :, :, :)
    real(c_double) :: x_hcall33(np_particles)
    integer(c_int64_t), allocatable :: x_inl61_tx(:)
    integer(c_int64_t), allocatable :: x_inl61_ty(:)
    integer(c_int64_t), allocatable :: x_inl61_tz(:)
    real(c_double), allocatable :: x_inl61_weight(:, :, :, :)
    real(c_double) :: x_hcall34(np_particles)
    real(c_double), allocatable :: x_inl14_gathered(:, :)
    real(c_double), allocatable :: x_inl15_gathered(:, :)
    real(c_double), allocatable :: x_inl16_gathered(:, :)
    real(c_double), allocatable :: x_inl17_gathered(:, :)
    real(c_double), allocatable :: x_inl18_gathered(:, :)
    real(c_double), allocatable :: x_inl19_gathered(:, :)
    real(c_double), allocatable :: x_inl20_ia(:, :, :)
    real(c_double), allocatable :: x_inl20_ib(:, :, :)
    real(c_double), allocatable :: x_inl20_gathered(:, :, :)
    real(c_double), allocatable :: x_inl21_ia(:, :, :)
    real(c_double), allocatable :: x_inl21_ib(:, :, :)
    real(c_double), allocatable :: x_inl21_gathered(:, :, :)
    real(c_double), allocatable :: x_inl22_ia(:, :, :)
    real(c_double), allocatable :: x_inl22_ib(:, :, :)
    real(c_double), allocatable :: x_inl22_gathered(:, :, :)
    real(c_double), allocatable :: x_inl23_ia(:, :, :)
    real(c_double), allocatable :: x_inl23_ib(:, :, :)
    real(c_double), allocatable :: x_inl23_gathered(:, :, :)
    real(c_double), allocatable :: x_inl24_ia(:, :, :)
    real(c_double), allocatable :: x_inl24_ib(:, :, :)
    real(c_double), allocatable :: x_inl24_gathered(:, :, :)
    real(c_double), allocatable :: x_inl25_ia(:, :, :)
    real(c_double), allocatable :: x_inl25_ib(:, :, :)
    real(c_double), allocatable :: x_inl25_gathered(:, :, :)
    integer(c_int64_t) :: x_inl2_j(np_particles)
    integer(c_int64_t) :: x_inl3_j(np_particles)
    integer(c_int64_t) :: x_inl4_j(np_particles)
    integer(c_int64_t) :: x_inl5_j(np_particles)
    integer(c_int64_t) :: x_inl6_j(np_particles)
    integer(c_int64_t) :: x_inl7_j(np_particles)
    integer(c_int64_t) :: x_inl8_j(np_particles)
    integer(c_int64_t) :: x_inl9_j(np_particles)
    integer(c_int64_t) :: x_inl10_j(np_particles)
    integer(c_int64_t) :: x_inl11_j(np_particles)
    integer(c_int64_t) :: x_inl12_j(np_particles)
    integer(c_int64_t) :: x_inl13_j(np_particles)
    real(c_double), allocatable :: x_inl26_ia(:, :, :)
    real(c_double), allocatable :: x_inl26_ib(:, :, :)
    real(c_double), allocatable :: x_inl26_gathered(:, :, :)
    real(c_double), allocatable :: x_inl27_ia(:, :, :)
    real(c_double), allocatable :: x_inl27_ib(:, :, :)
    real(c_double), allocatable :: x_inl27_gathered(:, :, :)
    real(c_double), allocatable :: x_inl28_ia(:, :, :)
    real(c_double), allocatable :: x_inl28_ib(:, :, :)
    real(c_double), allocatable :: x_inl28_gathered(:, :, :)
    real(c_double), allocatable :: x_inl29_ia(:, :, :)
    real(c_double), allocatable :: x_inl29_ib(:, :, :)
    real(c_double), allocatable :: x_inl29_gathered(:, :, :)
    real(c_double), allocatable :: x_inl30_ia(:, :, :)
    real(c_double), allocatable :: x_inl30_ib(:, :, :)
    real(c_double), allocatable :: x_inl30_gathered(:, :, :)
    real(c_double), allocatable :: x_inl31_ia(:, :, :)
    real(c_double), allocatable :: x_inl31_ib(:, :, :)
    real(c_double), allocatable :: x_inl31_gathered(:, :, :)
    real(c_double), allocatable :: x_inl32_ia(:, :, :)
    real(c_double), allocatable :: x_inl32_ib(:, :, :)
    real(c_double), allocatable :: x_inl32_gathered(:, :, :)
    real(c_double), allocatable :: x_inl33_ia(:, :, :)
    real(c_double), allocatable :: x_inl33_ib(:, :, :)
    real(c_double), allocatable :: x_inl33_gathered(:, :, :)
    real(c_double), allocatable :: x_inl34_ia(:, :, :)
    real(c_double), allocatable :: x_inl34_ib(:, :, :)
    real(c_double), allocatable :: x_inl34_gathered(:, :, :)
    real(c_double), allocatable :: x_inl35_ia(:, :, :)
    real(c_double), allocatable :: x_inl35_ib(:, :, :)
    real(c_double), allocatable :: x_inl35_gathered(:, :, :)
    real(c_double), allocatable :: x_inl36_ia(:, :, :)
    real(c_double), allocatable :: x_inl36_ib(:, :, :)
    real(c_double), allocatable :: x_inl36_gathered(:, :, :)
    real(c_double), allocatable :: x_inl37_ia(:, :, :)
    real(c_double), allocatable :: x_inl37_ib(:, :, :)
    real(c_double), allocatable :: x_inl37_gathered(:, :, :)
    real(c_double), allocatable :: x_inl38_ia(:, :, :)
    real(c_double), allocatable :: x_inl38_ib(:, :, :)
    real(c_double), allocatable :: x_inl38_gathered(:, :, :)
    real(c_double), allocatable :: x_inl39_ia(:, :, :)
    real(c_double), allocatable :: x_inl39_ib(:, :, :)
    real(c_double), allocatable :: x_inl39_gathered(:, :, :)
    real(c_double), allocatable :: x_inl40_ia(:, :, :)
    real(c_double), allocatable :: x_inl40_ib(:, :, :)
    real(c_double), allocatable :: x_inl40_gathered(:, :, :)
    real(c_double), allocatable :: x_inl41_ia(:, :, :)
    real(c_double), allocatable :: x_inl41_ib(:, :, :)
    real(c_double), allocatable :: x_inl41_gathered(:, :, :)
    real(c_double), allocatable :: x_inl42_ia(:, :, :)
    real(c_double), allocatable :: x_inl42_ib(:, :, :)
    real(c_double), allocatable :: x_inl42_gathered(:, :, :)
    real(c_double), allocatable :: x_inl43_ia(:, :, :)
    real(c_double), allocatable :: x_inl43_ib(:, :, :)
    real(c_double), allocatable :: x_inl43_gathered(:, :, :)
    real(c_double), allocatable :: x_inl44_gathered(:, :)
    real(c_double), allocatable :: x_inl45_gathered(:, :)
    real(c_double), allocatable :: x_inl46_gathered(:, :)
    real(c_double), allocatable :: x_inl47_gathered(:, :)
    real(c_double), allocatable :: x_inl48_gathered(:, :)
    real(c_double), allocatable :: x_inl49_gathered(:, :)
    real(c_double), allocatable :: x_inl50_gathered(:, :)
    real(c_double), allocatable :: x_inl51_gathered(:, :)
    real(c_double), allocatable :: x_inl52_gathered(:, :)
    real(c_double), allocatable :: x_inl53_gathered(:, :)
    real(c_double), allocatable :: x_inl54_gathered(:, :)
    real(c_double), allocatable :: x_inl55_gathered(:, :)
    real(c_double), allocatable :: x_inl56_ix(:, :, :, :)
    real(c_double), allocatable :: x_inl56_iy(:, :, :, :)
    real(c_double), allocatable :: x_inl56_iz(:, :, :, :)
    real(c_double), allocatable :: x_inl56_gathered(:, :, :, :)
    real(c_double), allocatable :: x_inl57_ix(:, :, :, :)
    real(c_double), allocatable :: x_inl57_iy(:, :, :, :)
    real(c_double), allocatable :: x_inl57_iz(:, :, :, :)
    real(c_double), allocatable :: x_inl57_gathered(:, :, :, :)
    real(c_double), allocatable :: x_inl58_ix(:, :, :, :)
    real(c_double), allocatable :: x_inl58_iy(:, :, :, :)
    real(c_double), allocatable :: x_inl58_iz(:, :, :, :)
    real(c_double), allocatable :: x_inl58_gathered(:, :, :, :)
    real(c_double), allocatable :: x_inl59_ix(:, :, :, :)
    real(c_double), allocatable :: x_inl59_iy(:, :, :, :)
    real(c_double), allocatable :: x_inl59_iz(:, :, :, :)
    real(c_double), allocatable :: x_inl59_gathered(:, :, :, :)
    real(c_double), allocatable :: x_inl60_ix(:, :, :, :)
    real(c_double), allocatable :: x_inl60_iy(:, :, :, :)
    real(c_double), allocatable :: x_inl60_iz(:, :, :, :)
    real(c_double), allocatable :: x_inl60_gathered(:, :, :, :)
    real(c_double), allocatable :: x_inl61_ix(:, :, :, :)
    real(c_double), allocatable :: x_inl61_iy(:, :, :, :)
    real(c_double), allocatable :: x_inl61_iz(:, :, :, :)
    real(c_double), allocatable :: x_inl61_gathered(:, :, :, :)
    o = INT(depos_order, c_int64_t)
    gal = INT(galerkin_interpolation, c_int64_t)
    g = INT(geom, c_int64_t)
    nmodes = INT(n_rz_azimuthal_modes, c_int64_t)
    x_inl1_o = o
    x_inl1_og = (o - gal)
    if (((g == 1) .OR. (g == 2))) then
        x_inl1_zdir = 1
    else if ((g == 3)) then
        x_inl1_zdir = 2
    else
        x_inl1_zdir = 0
    end if
    if ((g /= 0)) then
        if (((g == 2) .OR. (g == 4))) then
            ! numpy: np.sqrt(xp * xp + yp * yp)
            do x_r0_0 = 0, (np_particles) - 1
                x_cb1((x_r0_0) + 1) = SQRT(((xp((x_r0_0) + 1) * xp((x_r0_0) + 1)) + (yp((x_r0_0) + 1) * yp((x_r0_0) + &
                &1))))
            end do
            do x_w0_1 = 0, (np_particles) - 1
                x_inl1_rp((x_w0_1) + 1) = x_cb1((x_w0_1) + 1)
            end do
            do x_w0_2 = 0, (np_particles) - 1
                x_inl1_x((x_w0_2) + 1) = ((x_inl1_rp((x_w0_2) + 1) - xyzmin((0) + 1)) * dinv((0) + 1))
            end do
        else if ((g == 5)) then
            ! numpy: np.sqrt(xp * xp + yp * yp + zp * zp)
            do x_r0_3 = 0, (np_particles) - 1
                x_cb2((x_r0_3) + 1) = SQRT((((xp((x_r0_3) + 1) * xp((x_r0_3) + 1)) + (yp((x_r0_3) + 1) * yp((x_r0_3) + &
                &1))) + (zp((x_r0_3) + 1) * zp((x_r0_3) + 1))))
            end do
            do x_w0_4 = 0, (np_particles) - 1
                x_inl1_rp((x_w0_4) + 1) = x_cb2((x_w0_4) + 1)
            end do
            do x_w0_5 = 0, (np_particles) - 1
                x_inl1_x((x_w0_5) + 1) = ((x_inl1_rp((x_w0_5) + 1) - xyzmin((0) + 1)) * dinv((0) + 1))
            end do
        else
            do x_w0_6 = 0, (np_particles) - 1
                x_inl1_x((x_w0_6) + 1) = ((xp((x_w0_6) + 1) - xyzmin((0) + 1)) * dinv((0) + 1))
            end do
        end if
        if (.not. allocated(x_inl1_sx_node)) then
            allocate(x_inl1_sx_node(np_particles, (o + 1)))
        else if (size(x_inl1_sx_node, 1) /= (np_particles) .or. size(x_inl1_sx_node, 2) /= ((o + 1))) then
            deallocate(x_inl1_sx_node)
            allocate(x_inl1_sx_node(np_particles, (o + 1)))
        end if
        x_inl1_sx_node = 0
        if (.not. allocated(x_inl1_sx_cell)) then
            allocate(x_inl1_sx_cell(np_particles, (o + 1)))
        else if (size(x_inl1_sx_cell, 1) /= (np_particles) .or. size(x_inl1_sx_cell, 2) /= ((o + 1))) then
            deallocate(x_inl1_sx_cell)
            allocate(x_inl1_sx_cell(np_particles, (o + 1)))
        end if
        x_inl1_sx_cell = 0
        if (.not. allocated(x_inl1_sx_node_g)) then
            allocate(x_inl1_sx_node_g(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sx_node_g, 1) /= (np_particles) .or. size(x_inl1_sx_node_g, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sx_node_g)
            allocate(x_inl1_sx_node_g(np_particles, ((o - gal) + 1)))
        end if
        x_inl1_sx_node_g = 0
        if (.not. allocated(x_inl1_sx_cell_g)) then
            allocate(x_inl1_sx_cell_g(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sx_cell_g, 1) /= (np_particles) .or. size(x_inl1_sx_cell_g, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sx_cell_g)
            allocate(x_inl1_sx_cell_g(np_particles, ((o - gal) + 1)))
        end if
        x_inl1_sx_cell_g = 0
        x_inl1_j_node = 0
        x_inl1_j_cell = 0
        x_inl1_j_node_v = 0
        x_inl1_j_cell_v = 0
        if (((INT(ey_type((0) + 1), c_int64_t) == 1) .OR. (INT(ez_type((0) + 1), c_int64_t) == 1) .OR. &
        &(INT(bx_type((0) + 1), c_int64_t) == 1))) then
            x_inl2_idx = 0
            if ((x_inl1_o == 0)) then
                do x_w0_7 = 0, (np_particles) - 1
                    x_inl2_j((x_w0_7) + 1) = INT((x_inl1_x((x_w0_7) + 1) + 0.5_c_double), c_int64_t)
                end do
                do si1_l8 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l8) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_9 = 0, (np_particles) - 1
                    x_inl2_idx((x_w0_9) + 1) = x_inl2_j((x_w0_9) + 1)
                end do
            end if
            if ((x_inl1_o == 1)) then
                do x_w0_10 = 0, (np_particles) - 1
                    x_inl2_j((x_w0_10) + 1) = INT(x_inl1_x((x_w0_10) + 1), c_int64_t)
                end do
                do x_w0_11 = 0, (np_particles) - 1
                    x_inl2_xint((x_w0_11) + 1) = (x_inl1_x((x_w0_11) + 1) - x_inl2_j((x_w0_11) + 1))
                end do
                do si1_l12 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l12) + 1, (0) + 1) = (1.0_c_double - x_inl2_xint((si1_l12) + 1))
                end do
                do si1_l13 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l13) + 1, (1) + 1) = x_inl2_xint((si1_l13) + 1)
                end do
                do x_w0_14 = 0, (np_particles) - 1
                    x_inl2_idx((x_w0_14) + 1) = x_inl2_j((x_w0_14) + 1)
                end do
            end if
            if ((x_inl1_o == 2)) then
                do x_w0_15 = 0, (np_particles) - 1
                    x_inl2_j((x_w0_15) + 1) = INT((x_inl1_x((x_w0_15) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_16 = 0, (np_particles) - 1
                    x_inl2_xint((x_w0_16) + 1) = (x_inl1_x((x_w0_16) + 1) - x_inl2_j((x_w0_16) + 1))
                end do
                do si1_l17 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l17) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl2_xint((si1_l17) + &
                    &1))) * (0.5_c_double - x_inl2_xint((si1_l17) + 1)))
                end do
                do si1_l18 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l18) + 1, (1) + 1) = (0.75_c_double - (x_inl2_xint((si1_l18) + 1) * &
                    &x_inl2_xint((si1_l18) + 1)))
                end do
                do si1_l19 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l19) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl2_xint((si1_l19) + &
                    &1))) * (0.5_c_double + x_inl2_xint((si1_l19) + 1)))
                end do
                do x_w0_20 = 0, (np_particles) - 1
                    x_inl2_idx((x_w0_20) + 1) = (x_inl2_j((x_w0_20) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 3)) then
                do x_w0_21 = 0, (np_particles) - 1
                    x_inl2_j((x_w0_21) + 1) = INT(x_inl1_x((x_w0_21) + 1), c_int64_t)
                end do
                do x_w0_22 = 0, (np_particles) - 1
                    x_inl2_xint((x_w0_22) + 1) = (x_inl1_x((x_w0_22) + 1) - x_inl2_j((x_w0_22) + 1))
                end do
                do si1_l23 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l23) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl2_xint((si1_l23) + 1))) * (1.0_c_double - x_inl2_xint((si1_l23) + 1))) * (1.0_c_double - &
                    &x_inl2_xint((si1_l23) + 1)))
                end do
                do si1_l24 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l24) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - ((x_inl2_xint((si1_l24) &
                    &+ 1) * x_inl2_xint((si1_l24) + 1)) * (1.0_c_double - (x_inl2_xint((si1_l24) + 1) / 2.0_c_double))))
                end do
                do si1_l25 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l25) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl2_xint((si1_l25) + 1)) * (1.0_c_double - x_inl2_xint((si1_l25) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl2_xint((si1_l25) + 1))))))
                end do
                do si1_l26 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l26) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * x_inl2_xint((si1_l26) &
                    &+ 1)) * x_inl2_xint((si1_l26) + 1)) * x_inl2_xint((si1_l26) + 1))
                end do
                do x_w0_27 = 0, (np_particles) - 1
                    x_inl2_idx((x_w0_27) + 1) = (x_inl2_j((x_w0_27) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 4)) then
                do x_w0_28 = 0, (np_particles) - 1
                    x_inl2_j((x_w0_28) + 1) = INT((x_inl1_x((x_w0_28) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_29 = 0, (np_particles) - 1
                    x_inl2_xint((x_w0_29) + 1) = (x_inl1_x((x_w0_29) + 1) - x_inl2_j((x_w0_29) + 1))
                end do
                do x_w0_30 = 0, (np_particles) - 1
                    x_inl2_sm((x_w0_30) + 1) = (0.5_c_double - x_inl2_xint((x_w0_30) + 1))
                end do
                do x_w0_31 = 0, (np_particles) - 1
                    x_inl2_sp((x_w0_31) + 1) = (0.5_c_double + x_inl2_xint((x_w0_31) + 1))
                end do
                do si1_l32 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l32) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * x_inl2_sm((si1_l32) &
                    &+ 1)) * x_inl2_sm((si1_l32) + 1)) * x_inl2_sm((si1_l32) + 1)) * x_inl2_sm((si1_l32) + 1))
                end do
                do si1_l33 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l33) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl2_xint((si1_l33) + 1))) + (((4.0_c_double * x_inl2_xint((si1_l33) + 1)) * &
                    &x_inl2_xint((si1_l33) + 1)) * ((1.5_c_double + x_inl2_xint((si1_l33) + 1)) - &
                    &(x_inl2_xint((si1_l33) + 1) * x_inl2_xint((si1_l33) + 1))))))
                end do
                do si1_l34 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l34) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl2_xint((si1_l34) + 1)) * x_inl2_xint((si1_l34) + 1)) * &
                    &((x_inl2_xint((si1_l34) + 1) * x_inl2_xint((si1_l34) + 1)) - 2.5_c_double))))
                end do
                do si1_l35 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l35) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl2_xint((si1_l35) + 1))) + (((4.0_c_double * x_inl2_xint((si1_l35) + 1)) * &
                    &x_inl2_xint((si1_l35) + 1)) * ((1.5_c_double - x_inl2_xint((si1_l35) + 1)) - &
                    &(x_inl2_xint((si1_l35) + 1) * x_inl2_xint((si1_l35) + 1))))))
                end do
                do si1_l36 = 0, (np_particles) - 1
                    x_inl1_sx_node((si1_l36) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * x_inl2_sp((si1_l36) &
                    &+ 1)) * x_inl2_sp((si1_l36) + 1)) * x_inl2_sp((si1_l36) + 1)) * x_inl2_sp((si1_l36) + 1))
                end do
                do x_w0_37 = 0, (np_particles) - 1
                    x_inl2_idx((x_w0_37) + 1) = (x_inl2_j((x_w0_37) + 1) - 2)
                end do
            end if
            do x_w0_38 = 0, (np_particles) - 1
                x_inl1_j_node((x_w0_38) + 1) = x_inl2_idx((x_w0_38) + 1)
            end do
        end if
        if (((INT(ey_type((0) + 1), c_int64_t) == 0) .OR. (INT(ez_type((0) + 1), c_int64_t) == 0) .OR. &
        &(INT(bx_type((0) + 1), c_int64_t) == 0))) then
            x_inl3_idx = 0
            if ((x_inl1_o == 0)) then
                do x_w0_39 = 0, (np_particles) - 1
                    x_inl3_j((x_w0_39) + 1) = INT(((x_inl1_x((x_w0_39) + 1) - 0.5_c_double) + 0.5_c_double), c_int64_t)
                end do
                do si1_l40 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l40) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_41 = 0, (np_particles) - 1
                    x_inl3_idx((x_w0_41) + 1) = x_inl3_j((x_w0_41) + 1)
                end do
            end if
            if ((x_inl1_o == 1)) then
                do x_w0_42 = 0, (np_particles) - 1
                    x_inl3_j((x_w0_42) + 1) = INT((x_inl1_x((x_w0_42) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_43 = 0, (np_particles) - 1
                    x_inl3_xint((x_w0_43) + 1) = ((x_inl1_x((x_w0_43) + 1) - 0.5_c_double) - x_inl3_j((x_w0_43) + 1))
                end do
                do si1_l44 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l44) + 1, (0) + 1) = (1.0_c_double - x_inl3_xint((si1_l44) + 1))
                end do
                do si1_l45 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l45) + 1, (1) + 1) = x_inl3_xint((si1_l45) + 1)
                end do
                do x_w0_46 = 0, (np_particles) - 1
                    x_inl3_idx((x_w0_46) + 1) = x_inl3_j((x_w0_46) + 1)
                end do
            end if
            if ((x_inl1_o == 2)) then
                do x_w0_47 = 0, (np_particles) - 1
                    x_inl3_j((x_w0_47) + 1) = INT(((x_inl1_x((x_w0_47) + 1) - 0.5_c_double) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_48 = 0, (np_particles) - 1
                    x_inl3_xint((x_w0_48) + 1) = ((x_inl1_x((x_w0_48) + 1) - 0.5_c_double) - x_inl3_j((x_w0_48) + 1))
                end do
                do si1_l49 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l49) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl3_xint((si1_l49) + &
                    &1))) * (0.5_c_double - x_inl3_xint((si1_l49) + 1)))
                end do
                do si1_l50 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l50) + 1, (1) + 1) = (0.75_c_double - (x_inl3_xint((si1_l50) + 1) * &
                    &x_inl3_xint((si1_l50) + 1)))
                end do
                do si1_l51 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l51) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl3_xint((si1_l51) + &
                    &1))) * (0.5_c_double + x_inl3_xint((si1_l51) + 1)))
                end do
                do x_w0_52 = 0, (np_particles) - 1
                    x_inl3_idx((x_w0_52) + 1) = (x_inl3_j((x_w0_52) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 3)) then
                do x_w0_53 = 0, (np_particles) - 1
                    x_inl3_j((x_w0_53) + 1) = INT((x_inl1_x((x_w0_53) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_54 = 0, (np_particles) - 1
                    x_inl3_xint((x_w0_54) + 1) = ((x_inl1_x((x_w0_54) + 1) - 0.5_c_double) - x_inl3_j((x_w0_54) + 1))
                end do
                do si1_l55 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l55) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl3_xint((si1_l55) + 1))) * (1.0_c_double - x_inl3_xint((si1_l55) + 1))) * (1.0_c_double - &
                    &x_inl3_xint((si1_l55) + 1)))
                end do
                do si1_l56 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l56) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - ((x_inl3_xint((si1_l56) &
                    &+ 1) * x_inl3_xint((si1_l56) + 1)) * (1.0_c_double - (x_inl3_xint((si1_l56) + 1) / 2.0_c_double))))
                end do
                do si1_l57 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l57) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl3_xint((si1_l57) + 1)) * (1.0_c_double - x_inl3_xint((si1_l57) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl3_xint((si1_l57) + 1))))))
                end do
                do si1_l58 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l58) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * x_inl3_xint((si1_l58) &
                    &+ 1)) * x_inl3_xint((si1_l58) + 1)) * x_inl3_xint((si1_l58) + 1))
                end do
                do x_w0_59 = 0, (np_particles) - 1
                    x_inl3_idx((x_w0_59) + 1) = (x_inl3_j((x_w0_59) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 4)) then
                do x_w0_60 = 0, (np_particles) - 1
                    x_inl3_j((x_w0_60) + 1) = INT(((x_inl1_x((x_w0_60) + 1) - 0.5_c_double) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_61 = 0, (np_particles) - 1
                    x_inl3_xint((x_w0_61) + 1) = ((x_inl1_x((x_w0_61) + 1) - 0.5_c_double) - x_inl3_j((x_w0_61) + 1))
                end do
                do x_w0_62 = 0, (np_particles) - 1
                    x_inl3_sm((x_w0_62) + 1) = (0.5_c_double - x_inl3_xint((x_w0_62) + 1))
                end do
                do x_w0_63 = 0, (np_particles) - 1
                    x_inl3_sp((x_w0_63) + 1) = (0.5_c_double + x_inl3_xint((x_w0_63) + 1))
                end do
                do si1_l64 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l64) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * x_inl3_sm((si1_l64) &
                    &+ 1)) * x_inl3_sm((si1_l64) + 1)) * x_inl3_sm((si1_l64) + 1)) * x_inl3_sm((si1_l64) + 1))
                end do
                do si1_l65 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l65) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl3_xint((si1_l65) + 1))) + (((4.0_c_double * x_inl3_xint((si1_l65) + 1)) * &
                    &x_inl3_xint((si1_l65) + 1)) * ((1.5_c_double + x_inl3_xint((si1_l65) + 1)) - &
                    &(x_inl3_xint((si1_l65) + 1) * x_inl3_xint((si1_l65) + 1))))))
                end do
                do si1_l66 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l66) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl3_xint((si1_l66) + 1)) * x_inl3_xint((si1_l66) + 1)) * &
                    &((x_inl3_xint((si1_l66) + 1) * x_inl3_xint((si1_l66) + 1)) - 2.5_c_double))))
                end do
                do si1_l67 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l67) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl3_xint((si1_l67) + 1))) + (((4.0_c_double * x_inl3_xint((si1_l67) + 1)) * &
                    &x_inl3_xint((si1_l67) + 1)) * ((1.5_c_double - x_inl3_xint((si1_l67) + 1)) - &
                    &(x_inl3_xint((si1_l67) + 1) * x_inl3_xint((si1_l67) + 1))))))
                end do
                do si1_l68 = 0, (np_particles) - 1
                    x_inl1_sx_cell((si1_l68) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * x_inl3_sp((si1_l68) &
                    &+ 1)) * x_inl3_sp((si1_l68) + 1)) * x_inl3_sp((si1_l68) + 1)) * x_inl3_sp((si1_l68) + 1))
                end do
                do x_w0_69 = 0, (np_particles) - 1
                    x_inl3_idx((x_w0_69) + 1) = (x_inl3_j((x_w0_69) + 1) - 2)
                end do
            end if
            do x_w0_70 = 0, (np_particles) - 1
                x_inl1_j_cell((x_w0_70) + 1) = x_inl3_idx((x_w0_70) + 1)
            end do
        end if
        if (((INT(ex_type((0) + 1), c_int64_t) == 1) .OR. (INT(by_type((0) + 1), c_int64_t) == 1) .OR. &
        &(INT(bz_type((0) + 1), c_int64_t) == 1))) then
            x_inl4_idx = 0
            if ((x_inl1_og == 0)) then
                do x_w0_71 = 0, (np_particles) - 1
                    x_inl4_j((x_w0_71) + 1) = INT((x_inl1_x((x_w0_71) + 1) + 0.5_c_double), c_int64_t)
                end do
                do si1_l72 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l72) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_73 = 0, (np_particles) - 1
                    x_inl4_idx((x_w0_73) + 1) = x_inl4_j((x_w0_73) + 1)
                end do
            end if
            if ((x_inl1_og == 1)) then
                do x_w0_74 = 0, (np_particles) - 1
                    x_inl4_j((x_w0_74) + 1) = INT(x_inl1_x((x_w0_74) + 1), c_int64_t)
                end do
                do x_w0_75 = 0, (np_particles) - 1
                    x_inl4_xint((x_w0_75) + 1) = (x_inl1_x((x_w0_75) + 1) - x_inl4_j((x_w0_75) + 1))
                end do
                do si1_l76 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l76) + 1, (0) + 1) = (1.0_c_double - x_inl4_xint((si1_l76) + 1))
                end do
                do si1_l77 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l77) + 1, (1) + 1) = x_inl4_xint((si1_l77) + 1)
                end do
                do x_w0_78 = 0, (np_particles) - 1
                    x_inl4_idx((x_w0_78) + 1) = x_inl4_j((x_w0_78) + 1)
                end do
            end if
            if ((x_inl1_og == 2)) then
                do x_w0_79 = 0, (np_particles) - 1
                    x_inl4_j((x_w0_79) + 1) = INT((x_inl1_x((x_w0_79) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_80 = 0, (np_particles) - 1
                    x_inl4_xint((x_w0_80) + 1) = (x_inl1_x((x_w0_80) + 1) - x_inl4_j((x_w0_80) + 1))
                end do
                do si1_l81 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l81) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl4_xint((si1_l81) &
                    &+ 1))) * (0.5_c_double - x_inl4_xint((si1_l81) + 1)))
                end do
                do si1_l82 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l82) + 1, (1) + 1) = (0.75_c_double - (x_inl4_xint((si1_l82) + 1) * &
                    &x_inl4_xint((si1_l82) + 1)))
                end do
                do si1_l83 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l83) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl4_xint((si1_l83) &
                    &+ 1))) * (0.5_c_double + x_inl4_xint((si1_l83) + 1)))
                end do
                do x_w0_84 = 0, (np_particles) - 1
                    x_inl4_idx((x_w0_84) + 1) = (x_inl4_j((x_w0_84) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 3)) then
                do x_w0_85 = 0, (np_particles) - 1
                    x_inl4_j((x_w0_85) + 1) = INT(x_inl1_x((x_w0_85) + 1), c_int64_t)
                end do
                do x_w0_86 = 0, (np_particles) - 1
                    x_inl4_xint((x_w0_86) + 1) = (x_inl1_x((x_w0_86) + 1) - x_inl4_j((x_w0_86) + 1))
                end do
                do si1_l87 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l87) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl4_xint((si1_l87) + 1))) * (1.0_c_double - x_inl4_xint((si1_l87) + 1))) * (1.0_c_double - &
                    &x_inl4_xint((si1_l87) + 1)))
                end do
                do si1_l88 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l88) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl4_xint((si1_l88) + 1) * x_inl4_xint((si1_l88) + 1)) * (1.0_c_double - &
                    &(x_inl4_xint((si1_l88) + 1) / 2.0_c_double))))
                end do
                do si1_l89 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l89) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl4_xint((si1_l89) + 1)) * (1.0_c_double - x_inl4_xint((si1_l89) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl4_xint((si1_l89) + 1))))))
                end do
                do si1_l90 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l90) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl4_xint((si1_l90) + 1)) * x_inl4_xint((si1_l90) + 1)) * x_inl4_xint((si1_l90) + 1))
                end do
                do x_w0_91 = 0, (np_particles) - 1
                    x_inl4_idx((x_w0_91) + 1) = (x_inl4_j((x_w0_91) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 4)) then
                do x_w0_92 = 0, (np_particles) - 1
                    x_inl4_j((x_w0_92) + 1) = INT((x_inl1_x((x_w0_92) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_93 = 0, (np_particles) - 1
                    x_inl4_xint((x_w0_93) + 1) = (x_inl1_x((x_w0_93) + 1) - x_inl4_j((x_w0_93) + 1))
                end do
                do x_w0_94 = 0, (np_particles) - 1
                    x_inl4_sm((x_w0_94) + 1) = (0.5_c_double - x_inl4_xint((x_w0_94) + 1))
                end do
                do x_w0_95 = 0, (np_particles) - 1
                    x_inl4_sp((x_w0_95) + 1) = (0.5_c_double + x_inl4_xint((x_w0_95) + 1))
                end do
                do si1_l96 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l96) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl4_sm((si1_l96) + 1)) * x_inl4_sm((si1_l96) + 1)) * x_inl4_sm((si1_l96) + 1)) * &
                    &x_inl4_sm((si1_l96) + 1))
                end do
                do si1_l97 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l97) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl4_xint((si1_l97) + 1))) + (((4.0_c_double * x_inl4_xint((si1_l97) + 1)) * &
                    &x_inl4_xint((si1_l97) + 1)) * ((1.5_c_double + x_inl4_xint((si1_l97) + 1)) - &
                    &(x_inl4_xint((si1_l97) + 1) * x_inl4_xint((si1_l97) + 1))))))
                end do
                do si1_l98 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l98) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl4_xint((si1_l98) + 1)) * x_inl4_xint((si1_l98) + 1)) * &
                    &((x_inl4_xint((si1_l98) + 1) * x_inl4_xint((si1_l98) + 1)) - 2.5_c_double))))
                end do
                do si1_l99 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l99) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl4_xint((si1_l99) + 1))) + (((4.0_c_double * x_inl4_xint((si1_l99) + 1)) * &
                    &x_inl4_xint((si1_l99) + 1)) * ((1.5_c_double - x_inl4_xint((si1_l99) + 1)) - &
                    &(x_inl4_xint((si1_l99) + 1) * x_inl4_xint((si1_l99) + 1))))))
                end do
                do si1_l100 = 0, (np_particles) - 1
                    x_inl1_sx_node_g((si1_l100) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl4_sp((si1_l100) + 1)) * x_inl4_sp((si1_l100) + 1)) * x_inl4_sp((si1_l100) + 1)) * &
                    &x_inl4_sp((si1_l100) + 1))
                end do
                do x_w0_101 = 0, (np_particles) - 1
                    x_inl4_idx((x_w0_101) + 1) = (x_inl4_j((x_w0_101) + 1) - 2)
                end do
            end if
            do x_w0_102 = 0, (np_particles) - 1
                x_inl1_j_node_v((x_w0_102) + 1) = x_inl4_idx((x_w0_102) + 1)
            end do
        end if
        if (((INT(ex_type((0) + 1), c_int64_t) == 0) .OR. (INT(by_type((0) + 1), c_int64_t) == 0) .OR. &
        &(INT(bz_type((0) + 1), c_int64_t) == 0))) then
            x_inl5_idx = 0
            if ((x_inl1_og == 0)) then
                do x_w0_103 = 0, (np_particles) - 1
                    x_inl5_j((x_w0_103) + 1) = INT(((x_inl1_x((x_w0_103) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do si1_l104 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l104) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_105 = 0, (np_particles) - 1
                    x_inl5_idx((x_w0_105) + 1) = x_inl5_j((x_w0_105) + 1)
                end do
            end if
            if ((x_inl1_og == 1)) then
                do x_w0_106 = 0, (np_particles) - 1
                    x_inl5_j((x_w0_106) + 1) = INT((x_inl1_x((x_w0_106) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_107 = 0, (np_particles) - 1
                    x_inl5_xint((x_w0_107) + 1) = ((x_inl1_x((x_w0_107) + 1) - 0.5_c_double) - x_inl5_j((x_w0_107) + 1))
                end do
                do si1_l108 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l108) + 1, (0) + 1) = (1.0_c_double - x_inl5_xint((si1_l108) + 1))
                end do
                do si1_l109 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l109) + 1, (1) + 1) = x_inl5_xint((si1_l109) + 1)
                end do
                do x_w0_110 = 0, (np_particles) - 1
                    x_inl5_idx((x_w0_110) + 1) = x_inl5_j((x_w0_110) + 1)
                end do
            end if
            if ((x_inl1_og == 2)) then
                do x_w0_111 = 0, (np_particles) - 1
                    x_inl5_j((x_w0_111) + 1) = INT(((x_inl1_x((x_w0_111) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_112 = 0, (np_particles) - 1
                    x_inl5_xint((x_w0_112) + 1) = ((x_inl1_x((x_w0_112) + 1) - 0.5_c_double) - x_inl5_j((x_w0_112) + 1))
                end do
                do si1_l113 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l113) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - &
                    &x_inl5_xint((si1_l113) + 1))) * (0.5_c_double - x_inl5_xint((si1_l113) + 1)))
                end do
                do si1_l114 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l114) + 1, (1) + 1) = (0.75_c_double - (x_inl5_xint((si1_l114) + 1) * &
                    &x_inl5_xint((si1_l114) + 1)))
                end do
                do si1_l115 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l115) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + &
                    &x_inl5_xint((si1_l115) + 1))) * (0.5_c_double + x_inl5_xint((si1_l115) + 1)))
                end do
                do x_w0_116 = 0, (np_particles) - 1
                    x_inl5_idx((x_w0_116) + 1) = (x_inl5_j((x_w0_116) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 3)) then
                do x_w0_117 = 0, (np_particles) - 1
                    x_inl5_j((x_w0_117) + 1) = INT((x_inl1_x((x_w0_117) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_118 = 0, (np_particles) - 1
                    x_inl5_xint((x_w0_118) + 1) = ((x_inl1_x((x_w0_118) + 1) - 0.5_c_double) - x_inl5_j((x_w0_118) + 1))
                end do
                do si1_l119 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l119) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl5_xint((si1_l119) + 1))) * (1.0_c_double - x_inl5_xint((si1_l119) + 1))) * (1.0_c_double - &
                    &x_inl5_xint((si1_l119) + 1)))
                end do
                do si1_l120 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l120) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl5_xint((si1_l120) + 1) * x_inl5_xint((si1_l120) + 1)) * (1.0_c_double - &
                    &(x_inl5_xint((si1_l120) + 1) / 2.0_c_double))))
                end do
                do si1_l121 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l121) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl5_xint((si1_l121) + 1)) * (1.0_c_double - x_inl5_xint((si1_l121) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl5_xint((si1_l121) + 1))))))
                end do
                do si1_l122 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l122) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl5_xint((si1_l122) + 1)) * x_inl5_xint((si1_l122) + 1)) * x_inl5_xint((si1_l122) + 1))
                end do
                do x_w0_123 = 0, (np_particles) - 1
                    x_inl5_idx((x_w0_123) + 1) = (x_inl5_j((x_w0_123) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 4)) then
                do x_w0_124 = 0, (np_particles) - 1
                    x_inl5_j((x_w0_124) + 1) = INT(((x_inl1_x((x_w0_124) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_125 = 0, (np_particles) - 1
                    x_inl5_xint((x_w0_125) + 1) = ((x_inl1_x((x_w0_125) + 1) - 0.5_c_double) - x_inl5_j((x_w0_125) + 1))
                end do
                do x_w0_126 = 0, (np_particles) - 1
                    x_inl5_sm((x_w0_126) + 1) = (0.5_c_double - x_inl5_xint((x_w0_126) + 1))
                end do
                do x_w0_127 = 0, (np_particles) - 1
                    x_inl5_sp((x_w0_127) + 1) = (0.5_c_double + x_inl5_xint((x_w0_127) + 1))
                end do
                do si1_l128 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l128) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl5_sm((si1_l128) + 1)) * x_inl5_sm((si1_l128) + 1)) * x_inl5_sm((si1_l128) + 1)) * &
                    &x_inl5_sm((si1_l128) + 1))
                end do
                do si1_l129 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l129) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl5_xint((si1_l129) + 1))) + (((4.0_c_double * x_inl5_xint((si1_l129) + 1)) * &
                    &x_inl5_xint((si1_l129) + 1)) * ((1.5_c_double + x_inl5_xint((si1_l129) + 1)) - &
                    &(x_inl5_xint((si1_l129) + 1) * x_inl5_xint((si1_l129) + 1))))))
                end do
                do si1_l130 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l130) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl5_xint((si1_l130) + 1)) * x_inl5_xint((si1_l130) + 1)) * &
                    &((x_inl5_xint((si1_l130) + 1) * x_inl5_xint((si1_l130) + 1)) - 2.5_c_double))))
                end do
                do si1_l131 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l131) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl5_xint((si1_l131) + 1))) + (((4.0_c_double * x_inl5_xint((si1_l131) + 1)) * &
                    &x_inl5_xint((si1_l131) + 1)) * ((1.5_c_double - x_inl5_xint((si1_l131) + 1)) - &
                    &(x_inl5_xint((si1_l131) + 1) * x_inl5_xint((si1_l131) + 1))))))
                end do
                do si1_l132 = 0, (np_particles) - 1
                    x_inl1_sx_cell_g((si1_l132) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl5_sp((si1_l132) + 1)) * x_inl5_sp((si1_l132) + 1)) * x_inl5_sp((si1_l132) + 1)) * &
                    &x_inl5_sp((si1_l132) + 1))
                end do
                do x_w0_133 = 0, (np_particles) - 1
                    x_inl5_idx((x_w0_133) + 1) = (x_inl5_j((x_w0_133) + 1) - 2)
                end do
            end if
            do x_w0_134 = 0, (np_particles) - 1
                x_inl1_j_cell_v((x_w0_134) + 1) = x_inl5_idx((x_w0_134) + 1)
            end do
        end if
        if (.not. allocated(x_cb3)) then
            allocate(x_cb3(np_particles, ((o - gal) + 1)))
        else if (size(x_cb3, 1) /= (np_particles) .or. size(x_cb3, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb3)
            allocate(x_cb3(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(ex_type[0] == 1, __inl1_sx_node_g, __inl1_sx_cell_g)
        do x_r0_135 = 0, (((o - gal) + 1)) - 1
            do x_r1_136 = 0, (np_particles) - 1
                if ((INT(ex_type((0) + 1), c_int64_t) == 1)) then
                    x_ifexp0 = x_inl1_sx_node_g((x_r1_136) + 1, (x_r0_135) + 1)
                else
                    x_ifexp0 = x_inl1_sx_cell_g((x_r1_136) + 1, (x_r0_135) + 1)
                end if
                x_cb3((x_r1_136) + 1, (x_r0_135) + 1) = x_ifexp0
            end do
        end do
        if (.not. allocated(x_inl1_sx_ex)) then
            allocate(x_inl1_sx_ex(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sx_ex, 1) /= (np_particles) .or. size(x_inl1_sx_ex, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sx_ex)
            allocate(x_inl1_sx_ex(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_137 = 0, (((o - gal) + 1)) - 1
            do x_w1_138 = 0, (np_particles) - 1
                x_inl1_sx_ex((x_w1_138) + 1, (x_w0_137) + 1) = x_cb3((x_w1_138) + 1, (x_w0_137) + 1)
            end do
        end do
        if (.not. allocated(x_cb4)) then
            allocate(x_cb4(np_particles, (o + 1)))
        else if (size(x_cb4, 1) /= (np_particles) .or. size(x_cb4, 2) /= ((o + 1))) then
            deallocate(x_cb4)
            allocate(x_cb4(np_particles, (o + 1)))
        end if
        ! numpy: np.where(ey_type[0] == 1, __inl1_sx_node, __inl1_sx_cell)
        do x_r0_139 = 0, ((o + 1)) - 1
            do x_r1_140 = 0, (np_particles) - 1
                if ((INT(ey_type((0) + 1), c_int64_t) == 1)) then
                    x_ifexp1 = x_inl1_sx_node((x_r1_140) + 1, (x_r0_139) + 1)
                else
                    x_ifexp1 = x_inl1_sx_cell((x_r1_140) + 1, (x_r0_139) + 1)
                end if
                x_cb4((x_r1_140) + 1, (x_r0_139) + 1) = x_ifexp1
            end do
        end do
        if (.not. allocated(x_inl1_sx_ey)) then
            allocate(x_inl1_sx_ey(np_particles, (o + 1)))
        else if (size(x_inl1_sx_ey, 1) /= (np_particles) .or. size(x_inl1_sx_ey, 2) /= ((o + 1))) then
            deallocate(x_inl1_sx_ey)
            allocate(x_inl1_sx_ey(np_particles, (o + 1)))
        end if
        do x_w0_141 = 0, ((o + 1)) - 1
            do x_w1_142 = 0, (np_particles) - 1
                x_inl1_sx_ey((x_w1_142) + 1, (x_w0_141) + 1) = x_cb4((x_w1_142) + 1, (x_w0_141) + 1)
            end do
        end do
        if (.not. allocated(x_cb5)) then
            allocate(x_cb5(np_particles, (o + 1)))
        else if (size(x_cb5, 1) /= (np_particles) .or. size(x_cb5, 2) /= ((o + 1))) then
            deallocate(x_cb5)
            allocate(x_cb5(np_particles, (o + 1)))
        end if
        ! numpy: np.where(ez_type[0] == 1, __inl1_sx_node, __inl1_sx_cell)
        do x_r0_143 = 0, ((o + 1)) - 1
            do x_r1_144 = 0, (np_particles) - 1
                if ((INT(ez_type((0) + 1), c_int64_t) == 1)) then
                    x_ifexp2 = x_inl1_sx_node((x_r1_144) + 1, (x_r0_143) + 1)
                else
                    x_ifexp2 = x_inl1_sx_cell((x_r1_144) + 1, (x_r0_143) + 1)
                end if
                x_cb5((x_r1_144) + 1, (x_r0_143) + 1) = x_ifexp2
            end do
        end do
        if (.not. allocated(x_inl1_sx_ez)) then
            allocate(x_inl1_sx_ez(np_particles, (o + 1)))
        else if (size(x_inl1_sx_ez, 1) /= (np_particles) .or. size(x_inl1_sx_ez, 2) /= ((o + 1))) then
            deallocate(x_inl1_sx_ez)
            allocate(x_inl1_sx_ez(np_particles, (o + 1)))
        end if
        do x_w0_145 = 0, ((o + 1)) - 1
            do x_w1_146 = 0, (np_particles) - 1
                x_inl1_sx_ez((x_w1_146) + 1, (x_w0_145) + 1) = x_cb5((x_w1_146) + 1, (x_w0_145) + 1)
            end do
        end do
        if (.not. allocated(x_cb6)) then
            allocate(x_cb6(np_particles, (o + 1)))
        else if (size(x_cb6, 1) /= (np_particles) .or. size(x_cb6, 2) /= ((o + 1))) then
            deallocate(x_cb6)
            allocate(x_cb6(np_particles, (o + 1)))
        end if
        ! numpy: np.where(bx_type[0] == 1, __inl1_sx_node, __inl1_sx_cell)
        do x_r0_147 = 0, ((o + 1)) - 1
            do x_r1_148 = 0, (np_particles) - 1
                if ((INT(bx_type((0) + 1), c_int64_t) == 1)) then
                    x_ifexp3 = x_inl1_sx_node((x_r1_148) + 1, (x_r0_147) + 1)
                else
                    x_ifexp3 = x_inl1_sx_cell((x_r1_148) + 1, (x_r0_147) + 1)
                end if
                x_cb6((x_r1_148) + 1, (x_r0_147) + 1) = x_ifexp3
            end do
        end do
        if (.not. allocated(x_inl1_sx_bx)) then
            allocate(x_inl1_sx_bx(np_particles, (o + 1)))
        else if (size(x_inl1_sx_bx, 1) /= (np_particles) .or. size(x_inl1_sx_bx, 2) /= ((o + 1))) then
            deallocate(x_inl1_sx_bx)
            allocate(x_inl1_sx_bx(np_particles, (o + 1)))
        end if
        do x_w0_149 = 0, ((o + 1)) - 1
            do x_w1_150 = 0, (np_particles) - 1
                x_inl1_sx_bx((x_w1_150) + 1, (x_w0_149) + 1) = x_cb6((x_w1_150) + 1, (x_w0_149) + 1)
            end do
        end do
        if (.not. allocated(x_cb7)) then
            allocate(x_cb7(np_particles, ((o - gal) + 1)))
        else if (size(x_cb7, 1) /= (np_particles) .or. size(x_cb7, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb7)
            allocate(x_cb7(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(by_type[0] == 1, __inl1_sx_node_g, __inl1_sx_cell_g)
        do x_r0_151 = 0, (((o - gal) + 1)) - 1
            do x_r1_152 = 0, (np_particles) - 1
                if ((INT(by_type((0) + 1), c_int64_t) == 1)) then
                    x_ifexp4 = x_inl1_sx_node_g((x_r1_152) + 1, (x_r0_151) + 1)
                else
                    x_ifexp4 = x_inl1_sx_cell_g((x_r1_152) + 1, (x_r0_151) + 1)
                end if
                x_cb7((x_r1_152) + 1, (x_r0_151) + 1) = x_ifexp4
            end do
        end do
        if (.not. allocated(x_inl1_sx_by)) then
            allocate(x_inl1_sx_by(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sx_by, 1) /= (np_particles) .or. size(x_inl1_sx_by, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sx_by)
            allocate(x_inl1_sx_by(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_153 = 0, (((o - gal) + 1)) - 1
            do x_w1_154 = 0, (np_particles) - 1
                x_inl1_sx_by((x_w1_154) + 1, (x_w0_153) + 1) = x_cb7((x_w1_154) + 1, (x_w0_153) + 1)
            end do
        end do
        if (.not. allocated(x_cb8)) then
            allocate(x_cb8(np_particles, ((o - gal) + 1)))
        else if (size(x_cb8, 1) /= (np_particles) .or. size(x_cb8, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb8)
            allocate(x_cb8(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(bz_type[0] == 1, __inl1_sx_node_g, __inl1_sx_cell_g)
        do x_r0_155 = 0, (((o - gal) + 1)) - 1
            do x_r1_156 = 0, (np_particles) - 1
                if ((INT(bz_type((0) + 1), c_int64_t) == 1)) then
                    x_ifexp5 = x_inl1_sx_node_g((x_r1_156) + 1, (x_r0_155) + 1)
                else
                    x_ifexp5 = x_inl1_sx_cell_g((x_r1_156) + 1, (x_r0_155) + 1)
                end if
                x_cb8((x_r1_156) + 1, (x_r0_155) + 1) = x_ifexp5
            end do
        end do
        if (.not. allocated(x_inl1_sx_bz)) then
            allocate(x_inl1_sx_bz(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sx_bz, 1) /= (np_particles) .or. size(x_inl1_sx_bz, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sx_bz)
            allocate(x_inl1_sx_bz(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_157 = 0, (((o - gal) + 1)) - 1
            do x_w1_158 = 0, (np_particles) - 1
                x_inl1_sx_bz((x_w1_158) + 1, (x_w0_157) + 1) = x_cb8((x_w1_158) + 1, (x_w0_157) + 1)
            end do
        end do
        x_inl1_n_sx_ex = (x_inl1_og + 1)
        x_inl1_n_sx_by = (x_inl1_og + 1)
        x_inl1_n_sx_bz = (x_inl1_og + 1)
        x_inl1_n_sx_ey = (x_inl1_o + 1)
        x_inl1_n_sx_ez = (x_inl1_o + 1)
        x_inl1_n_sx_bx = (x_inl1_o + 1)
        ! numpy: np.where(ex_type[0] == 1, __inl1_j_node_v, __inl1_j_cell_v)
        do x_r0_159 = 0, (np_particles) - 1
            if ((INT(ex_type((0) + 1), c_int64_t) == 1)) then
                x_ifexp6 = x_inl1_j_node_v((x_r0_159) + 1)
            else
                x_ifexp6 = x_inl1_j_cell_v((x_r0_159) + 1)
            end if
            x_cb9((x_r0_159) + 1) = x_ifexp6
        end do
        do x_w0_160 = 0, (np_particles) - 1
            x_inl1_j_ex((x_w0_160) + 1) = x_cb9((x_w0_160) + 1)
        end do
        ! numpy: np.where(ey_type[0] == 1, __inl1_j_node, __inl1_j_cell)
        do x_r0_161 = 0, (np_particles) - 1
            if ((INT(ey_type((0) + 1), c_int64_t) == 1)) then
                x_ifexp7 = x_inl1_j_node((x_r0_161) + 1)
            else
                x_ifexp7 = x_inl1_j_cell((x_r0_161) + 1)
            end if
            x_cb10((x_r0_161) + 1) = x_ifexp7
        end do
        do x_w0_162 = 0, (np_particles) - 1
            x_inl1_j_ey((x_w0_162) + 1) = x_cb10((x_w0_162) + 1)
        end do
        ! numpy: np.where(ez_type[0] == 1, __inl1_j_node, __inl1_j_cell)
        do x_r0_163 = 0, (np_particles) - 1
            if ((INT(ez_type((0) + 1), c_int64_t) == 1)) then
                x_ifexp8 = x_inl1_j_node((x_r0_163) + 1)
            else
                x_ifexp8 = x_inl1_j_cell((x_r0_163) + 1)
            end if
            x_cb11((x_r0_163) + 1) = x_ifexp8
        end do
        do x_w0_164 = 0, (np_particles) - 1
            x_inl1_j_ez((x_w0_164) + 1) = x_cb11((x_w0_164) + 1)
        end do
        ! numpy: np.where(bx_type[0] == 1, __inl1_j_node, __inl1_j_cell)
        do x_r0_165 = 0, (np_particles) - 1
            if ((INT(bx_type((0) + 1), c_int64_t) == 1)) then
                x_ifexp9 = x_inl1_j_node((x_r0_165) + 1)
            else
                x_ifexp9 = x_inl1_j_cell((x_r0_165) + 1)
            end if
            x_cb12((x_r0_165) + 1) = x_ifexp9
        end do
        do x_w0_166 = 0, (np_particles) - 1
            x_inl1_j_bx((x_w0_166) + 1) = x_cb12((x_w0_166) + 1)
        end do
        ! numpy: np.where(by_type[0] == 1, __inl1_j_node_v, __inl1_j_cell_v)
        do x_r0_167 = 0, (np_particles) - 1
            if ((INT(by_type((0) + 1), c_int64_t) == 1)) then
                x_ifexp10 = x_inl1_j_node_v((x_r0_167) + 1)
            else
                x_ifexp10 = x_inl1_j_cell_v((x_r0_167) + 1)
            end if
            x_cb13((x_r0_167) + 1) = x_ifexp10
        end do
        do x_w0_168 = 0, (np_particles) - 1
            x_inl1_j_by((x_w0_168) + 1) = x_cb13((x_w0_168) + 1)
        end do
        ! numpy: np.where(bz_type[0] == 1, __inl1_j_node_v, __inl1_j_cell_v)
        do x_r0_169 = 0, (np_particles) - 1
            if ((INT(bz_type((0) + 1), c_int64_t) == 1)) then
                x_ifexp11 = x_inl1_j_node_v((x_r0_169) + 1)
            else
                x_ifexp11 = x_inl1_j_cell_v((x_r0_169) + 1)
            end if
            x_cb14((x_r0_169) + 1) = x_ifexp11
        end do
        do x_w0_170 = 0, (np_particles) - 1
            x_inl1_j_bz((x_w0_170) + 1) = x_cb14((x_w0_170) + 1)
        end do
    end if
    if ((g == 3)) then
        do x_w0_171 = 0, (np_particles) - 1
            x_inl1_y((x_w0_171) + 1) = ((yp((x_w0_171) + 1) - xyzmin((1) + 1)) * dinv((1) + 1))
        end do
        if (.not. allocated(x_inl1_sy_node)) then
            allocate(x_inl1_sy_node(np_particles, (o + 1)))
        else if (size(x_inl1_sy_node, 1) /= (np_particles) .or. size(x_inl1_sy_node, 2) /= ((o + 1))) then
            deallocate(x_inl1_sy_node)
            allocate(x_inl1_sy_node(np_particles, (o + 1)))
        end if
        x_inl1_sy_node = 0
        if (.not. allocated(x_inl1_sy_cell)) then
            allocate(x_inl1_sy_cell(np_particles, (o + 1)))
        else if (size(x_inl1_sy_cell, 1) /= (np_particles) .or. size(x_inl1_sy_cell, 2) /= ((o + 1))) then
            deallocate(x_inl1_sy_cell)
            allocate(x_inl1_sy_cell(np_particles, (o + 1)))
        end if
        x_inl1_sy_cell = 0
        if (.not. allocated(x_inl1_sy_node_v)) then
            allocate(x_inl1_sy_node_v(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sy_node_v, 1) /= (np_particles) .or. size(x_inl1_sy_node_v, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sy_node_v)
            allocate(x_inl1_sy_node_v(np_particles, ((o - gal) + 1)))
        end if
        x_inl1_sy_node_v = 0
        if (.not. allocated(x_inl1_sy_cell_v)) then
            allocate(x_inl1_sy_cell_v(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sy_cell_v, 1) /= (np_particles) .or. size(x_inl1_sy_cell_v, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sy_cell_v)
            allocate(x_inl1_sy_cell_v(np_particles, ((o - gal) + 1)))
        end if
        x_inl1_sy_cell_v = 0
        x_inl1_k_node = 0
        x_inl1_k_cell = 0
        x_inl1_k_node_v = 0
        x_inl1_k_cell_v = 0
        if (((INT(ex_type((1) + 1), c_int64_t) == 1) .OR. (INT(ez_type((1) + 1), c_int64_t) == 1) .OR. &
        &(INT(by_type((1) + 1), c_int64_t) == 1))) then
            x_inl6_idx = 0
            if ((x_inl1_o == 0)) then
                do x_w0_172 = 0, (np_particles) - 1
                    x_inl6_j((x_w0_172) + 1) = INT((x_inl1_y((x_w0_172) + 1) + 0.5_c_double), c_int64_t)
                end do
                do si1_l173 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l173) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_174 = 0, (np_particles) - 1
                    x_inl6_idx((x_w0_174) + 1) = x_inl6_j((x_w0_174) + 1)
                end do
            end if
            if ((x_inl1_o == 1)) then
                do x_w0_175 = 0, (np_particles) - 1
                    x_inl6_j((x_w0_175) + 1) = INT(x_inl1_y((x_w0_175) + 1), c_int64_t)
                end do
                do x_w0_176 = 0, (np_particles) - 1
                    x_inl6_xint((x_w0_176) + 1) = (x_inl1_y((x_w0_176) + 1) - x_inl6_j((x_w0_176) + 1))
                end do
                do si1_l177 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l177) + 1, (0) + 1) = (1.0_c_double - x_inl6_xint((si1_l177) + 1))
                end do
                do si1_l178 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l178) + 1, (1) + 1) = x_inl6_xint((si1_l178) + 1)
                end do
                do x_w0_179 = 0, (np_particles) - 1
                    x_inl6_idx((x_w0_179) + 1) = x_inl6_j((x_w0_179) + 1)
                end do
            end if
            if ((x_inl1_o == 2)) then
                do x_w0_180 = 0, (np_particles) - 1
                    x_inl6_j((x_w0_180) + 1) = INT((x_inl1_y((x_w0_180) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_181 = 0, (np_particles) - 1
                    x_inl6_xint((x_w0_181) + 1) = (x_inl1_y((x_w0_181) + 1) - x_inl6_j((x_w0_181) + 1))
                end do
                do si1_l182 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l182) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl6_xint((si1_l182) &
                    &+ 1))) * (0.5_c_double - x_inl6_xint((si1_l182) + 1)))
                end do
                do si1_l183 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l183) + 1, (1) + 1) = (0.75_c_double - (x_inl6_xint((si1_l183) + 1) * &
                    &x_inl6_xint((si1_l183) + 1)))
                end do
                do si1_l184 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l184) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl6_xint((si1_l184) &
                    &+ 1))) * (0.5_c_double + x_inl6_xint((si1_l184) + 1)))
                end do
                do x_w0_185 = 0, (np_particles) - 1
                    x_inl6_idx((x_w0_185) + 1) = (x_inl6_j((x_w0_185) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 3)) then
                do x_w0_186 = 0, (np_particles) - 1
                    x_inl6_j((x_w0_186) + 1) = INT(x_inl1_y((x_w0_186) + 1), c_int64_t)
                end do
                do x_w0_187 = 0, (np_particles) - 1
                    x_inl6_xint((x_w0_187) + 1) = (x_inl1_y((x_w0_187) + 1) - x_inl6_j((x_w0_187) + 1))
                end do
                do si1_l188 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l188) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl6_xint((si1_l188) + 1))) * (1.0_c_double - x_inl6_xint((si1_l188) + 1))) * (1.0_c_double - &
                    &x_inl6_xint((si1_l188) + 1)))
                end do
                do si1_l189 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l189) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl6_xint((si1_l189) + 1) * x_inl6_xint((si1_l189) + 1)) * (1.0_c_double - &
                    &(x_inl6_xint((si1_l189) + 1) / 2.0_c_double))))
                end do
                do si1_l190 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l190) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl6_xint((si1_l190) + 1)) * (1.0_c_double - x_inl6_xint((si1_l190) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl6_xint((si1_l190) + 1))))))
                end do
                do si1_l191 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l191) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl6_xint((si1_l191) + 1)) * x_inl6_xint((si1_l191) + 1)) * x_inl6_xint((si1_l191) + 1))
                end do
                do x_w0_192 = 0, (np_particles) - 1
                    x_inl6_idx((x_w0_192) + 1) = (x_inl6_j((x_w0_192) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 4)) then
                do x_w0_193 = 0, (np_particles) - 1
                    x_inl6_j((x_w0_193) + 1) = INT((x_inl1_y((x_w0_193) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_194 = 0, (np_particles) - 1
                    x_inl6_xint((x_w0_194) + 1) = (x_inl1_y((x_w0_194) + 1) - x_inl6_j((x_w0_194) + 1))
                end do
                do x_w0_195 = 0, (np_particles) - 1
                    x_inl6_sm((x_w0_195) + 1) = (0.5_c_double - x_inl6_xint((x_w0_195) + 1))
                end do
                do x_w0_196 = 0, (np_particles) - 1
                    x_inl6_sp((x_w0_196) + 1) = (0.5_c_double + x_inl6_xint((x_w0_196) + 1))
                end do
                do si1_l197 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l197) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl6_sm((si1_l197) + 1)) * x_inl6_sm((si1_l197) + 1)) * x_inl6_sm((si1_l197) + 1)) * &
                    &x_inl6_sm((si1_l197) + 1))
                end do
                do si1_l198 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l198) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl6_xint((si1_l198) + 1))) + (((4.0_c_double * x_inl6_xint((si1_l198) + 1)) * &
                    &x_inl6_xint((si1_l198) + 1)) * ((1.5_c_double + x_inl6_xint((si1_l198) + 1)) - &
                    &(x_inl6_xint((si1_l198) + 1) * x_inl6_xint((si1_l198) + 1))))))
                end do
                do si1_l199 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l199) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl6_xint((si1_l199) + 1)) * x_inl6_xint((si1_l199) + 1)) * &
                    &((x_inl6_xint((si1_l199) + 1) * x_inl6_xint((si1_l199) + 1)) - 2.5_c_double))))
                end do
                do si1_l200 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l200) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl6_xint((si1_l200) + 1))) + (((4.0_c_double * x_inl6_xint((si1_l200) + 1)) * &
                    &x_inl6_xint((si1_l200) + 1)) * ((1.5_c_double - x_inl6_xint((si1_l200) + 1)) - &
                    &(x_inl6_xint((si1_l200) + 1) * x_inl6_xint((si1_l200) + 1))))))
                end do
                do si1_l201 = 0, (np_particles) - 1
                    x_inl1_sy_node((si1_l201) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl6_sp((si1_l201) + 1)) * x_inl6_sp((si1_l201) + 1)) * x_inl6_sp((si1_l201) + 1)) * &
                    &x_inl6_sp((si1_l201) + 1))
                end do
                do x_w0_202 = 0, (np_particles) - 1
                    x_inl6_idx((x_w0_202) + 1) = (x_inl6_j((x_w0_202) + 1) - 2)
                end do
            end if
            do x_w0_203 = 0, (np_particles) - 1
                x_inl1_k_node((x_w0_203) + 1) = x_inl6_idx((x_w0_203) + 1)
            end do
        end if
        if (((INT(ex_type((1) + 1), c_int64_t) == 0) .OR. (INT(ez_type((1) + 1), c_int64_t) == 0) .OR. &
        &(INT(by_type((1) + 1), c_int64_t) == 0))) then
            x_inl7_idx = 0
            if ((x_inl1_o == 0)) then
                do x_w0_204 = 0, (np_particles) - 1
                    x_inl7_j((x_w0_204) + 1) = INT(((x_inl1_y((x_w0_204) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do si1_l205 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l205) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_206 = 0, (np_particles) - 1
                    x_inl7_idx((x_w0_206) + 1) = x_inl7_j((x_w0_206) + 1)
                end do
            end if
            if ((x_inl1_o == 1)) then
                do x_w0_207 = 0, (np_particles) - 1
                    x_inl7_j((x_w0_207) + 1) = INT((x_inl1_y((x_w0_207) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_208 = 0, (np_particles) - 1
                    x_inl7_xint((x_w0_208) + 1) = ((x_inl1_y((x_w0_208) + 1) - 0.5_c_double) - x_inl7_j((x_w0_208) + 1))
                end do
                do si1_l209 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l209) + 1, (0) + 1) = (1.0_c_double - x_inl7_xint((si1_l209) + 1))
                end do
                do si1_l210 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l210) + 1, (1) + 1) = x_inl7_xint((si1_l210) + 1)
                end do
                do x_w0_211 = 0, (np_particles) - 1
                    x_inl7_idx((x_w0_211) + 1) = x_inl7_j((x_w0_211) + 1)
                end do
            end if
            if ((x_inl1_o == 2)) then
                do x_w0_212 = 0, (np_particles) - 1
                    x_inl7_j((x_w0_212) + 1) = INT(((x_inl1_y((x_w0_212) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_213 = 0, (np_particles) - 1
                    x_inl7_xint((x_w0_213) + 1) = ((x_inl1_y((x_w0_213) + 1) - 0.5_c_double) - x_inl7_j((x_w0_213) + 1))
                end do
                do si1_l214 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l214) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl7_xint((si1_l214) &
                    &+ 1))) * (0.5_c_double - x_inl7_xint((si1_l214) + 1)))
                end do
                do si1_l215 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l215) + 1, (1) + 1) = (0.75_c_double - (x_inl7_xint((si1_l215) + 1) * &
                    &x_inl7_xint((si1_l215) + 1)))
                end do
                do si1_l216 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l216) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl7_xint((si1_l216) &
                    &+ 1))) * (0.5_c_double + x_inl7_xint((si1_l216) + 1)))
                end do
                do x_w0_217 = 0, (np_particles) - 1
                    x_inl7_idx((x_w0_217) + 1) = (x_inl7_j((x_w0_217) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 3)) then
                do x_w0_218 = 0, (np_particles) - 1
                    x_inl7_j((x_w0_218) + 1) = INT((x_inl1_y((x_w0_218) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_219 = 0, (np_particles) - 1
                    x_inl7_xint((x_w0_219) + 1) = ((x_inl1_y((x_w0_219) + 1) - 0.5_c_double) - x_inl7_j((x_w0_219) + 1))
                end do
                do si1_l220 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l220) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl7_xint((si1_l220) + 1))) * (1.0_c_double - x_inl7_xint((si1_l220) + 1))) * (1.0_c_double - &
                    &x_inl7_xint((si1_l220) + 1)))
                end do
                do si1_l221 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l221) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl7_xint((si1_l221) + 1) * x_inl7_xint((si1_l221) + 1)) * (1.0_c_double - &
                    &(x_inl7_xint((si1_l221) + 1) / 2.0_c_double))))
                end do
                do si1_l222 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l222) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl7_xint((si1_l222) + 1)) * (1.0_c_double - x_inl7_xint((si1_l222) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl7_xint((si1_l222) + 1))))))
                end do
                do si1_l223 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l223) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl7_xint((si1_l223) + 1)) * x_inl7_xint((si1_l223) + 1)) * x_inl7_xint((si1_l223) + 1))
                end do
                do x_w0_224 = 0, (np_particles) - 1
                    x_inl7_idx((x_w0_224) + 1) = (x_inl7_j((x_w0_224) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 4)) then
                do x_w0_225 = 0, (np_particles) - 1
                    x_inl7_j((x_w0_225) + 1) = INT(((x_inl1_y((x_w0_225) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_226 = 0, (np_particles) - 1
                    x_inl7_xint((x_w0_226) + 1) = ((x_inl1_y((x_w0_226) + 1) - 0.5_c_double) - x_inl7_j((x_w0_226) + 1))
                end do
                do x_w0_227 = 0, (np_particles) - 1
                    x_inl7_sm((x_w0_227) + 1) = (0.5_c_double - x_inl7_xint((x_w0_227) + 1))
                end do
                do x_w0_228 = 0, (np_particles) - 1
                    x_inl7_sp((x_w0_228) + 1) = (0.5_c_double + x_inl7_xint((x_w0_228) + 1))
                end do
                do si1_l229 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l229) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl7_sm((si1_l229) + 1)) * x_inl7_sm((si1_l229) + 1)) * x_inl7_sm((si1_l229) + 1)) * &
                    &x_inl7_sm((si1_l229) + 1))
                end do
                do si1_l230 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l230) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl7_xint((si1_l230) + 1))) + (((4.0_c_double * x_inl7_xint((si1_l230) + 1)) * &
                    &x_inl7_xint((si1_l230) + 1)) * ((1.5_c_double + x_inl7_xint((si1_l230) + 1)) - &
                    &(x_inl7_xint((si1_l230) + 1) * x_inl7_xint((si1_l230) + 1))))))
                end do
                do si1_l231 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l231) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl7_xint((si1_l231) + 1)) * x_inl7_xint((si1_l231) + 1)) * &
                    &((x_inl7_xint((si1_l231) + 1) * x_inl7_xint((si1_l231) + 1)) - 2.5_c_double))))
                end do
                do si1_l232 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l232) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl7_xint((si1_l232) + 1))) + (((4.0_c_double * x_inl7_xint((si1_l232) + 1)) * &
                    &x_inl7_xint((si1_l232) + 1)) * ((1.5_c_double - x_inl7_xint((si1_l232) + 1)) - &
                    &(x_inl7_xint((si1_l232) + 1) * x_inl7_xint((si1_l232) + 1))))))
                end do
                do si1_l233 = 0, (np_particles) - 1
                    x_inl1_sy_cell((si1_l233) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl7_sp((si1_l233) + 1)) * x_inl7_sp((si1_l233) + 1)) * x_inl7_sp((si1_l233) + 1)) * &
                    &x_inl7_sp((si1_l233) + 1))
                end do
                do x_w0_234 = 0, (np_particles) - 1
                    x_inl7_idx((x_w0_234) + 1) = (x_inl7_j((x_w0_234) + 1) - 2)
                end do
            end if
            do x_w0_235 = 0, (np_particles) - 1
                x_inl1_k_cell((x_w0_235) + 1) = x_inl7_idx((x_w0_235) + 1)
            end do
        end if
        if (((INT(ey_type((1) + 1), c_int64_t) == 1) .OR. (INT(bx_type((1) + 1), c_int64_t) == 1) .OR. &
        &(INT(bz_type((1) + 1), c_int64_t) == 1))) then
            x_inl8_idx = 0
            if ((x_inl1_og == 0)) then
                do x_w0_236 = 0, (np_particles) - 1
                    x_inl8_j((x_w0_236) + 1) = INT((x_inl1_y((x_w0_236) + 1) + 0.5_c_double), c_int64_t)
                end do
                do si1_l237 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l237) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_238 = 0, (np_particles) - 1
                    x_inl8_idx((x_w0_238) + 1) = x_inl8_j((x_w0_238) + 1)
                end do
            end if
            if ((x_inl1_og == 1)) then
                do x_w0_239 = 0, (np_particles) - 1
                    x_inl8_j((x_w0_239) + 1) = INT(x_inl1_y((x_w0_239) + 1), c_int64_t)
                end do
                do x_w0_240 = 0, (np_particles) - 1
                    x_inl8_xint((x_w0_240) + 1) = (x_inl1_y((x_w0_240) + 1) - x_inl8_j((x_w0_240) + 1))
                end do
                do si1_l241 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l241) + 1, (0) + 1) = (1.0_c_double - x_inl8_xint((si1_l241) + 1))
                end do
                do si1_l242 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l242) + 1, (1) + 1) = x_inl8_xint((si1_l242) + 1)
                end do
                do x_w0_243 = 0, (np_particles) - 1
                    x_inl8_idx((x_w0_243) + 1) = x_inl8_j((x_w0_243) + 1)
                end do
            end if
            if ((x_inl1_og == 2)) then
                do x_w0_244 = 0, (np_particles) - 1
                    x_inl8_j((x_w0_244) + 1) = INT((x_inl1_y((x_w0_244) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_245 = 0, (np_particles) - 1
                    x_inl8_xint((x_w0_245) + 1) = (x_inl1_y((x_w0_245) + 1) - x_inl8_j((x_w0_245) + 1))
                end do
                do si1_l246 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l246) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - &
                    &x_inl8_xint((si1_l246) + 1))) * (0.5_c_double - x_inl8_xint((si1_l246) + 1)))
                end do
                do si1_l247 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l247) + 1, (1) + 1) = (0.75_c_double - (x_inl8_xint((si1_l247) + 1) * &
                    &x_inl8_xint((si1_l247) + 1)))
                end do
                do si1_l248 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l248) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + &
                    &x_inl8_xint((si1_l248) + 1))) * (0.5_c_double + x_inl8_xint((si1_l248) + 1)))
                end do
                do x_w0_249 = 0, (np_particles) - 1
                    x_inl8_idx((x_w0_249) + 1) = (x_inl8_j((x_w0_249) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 3)) then
                do x_w0_250 = 0, (np_particles) - 1
                    x_inl8_j((x_w0_250) + 1) = INT(x_inl1_y((x_w0_250) + 1), c_int64_t)
                end do
                do x_w0_251 = 0, (np_particles) - 1
                    x_inl8_xint((x_w0_251) + 1) = (x_inl1_y((x_w0_251) + 1) - x_inl8_j((x_w0_251) + 1))
                end do
                do si1_l252 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l252) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl8_xint((si1_l252) + 1))) * (1.0_c_double - x_inl8_xint((si1_l252) + 1))) * (1.0_c_double - &
                    &x_inl8_xint((si1_l252) + 1)))
                end do
                do si1_l253 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l253) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl8_xint((si1_l253) + 1) * x_inl8_xint((si1_l253) + 1)) * (1.0_c_double - &
                    &(x_inl8_xint((si1_l253) + 1) / 2.0_c_double))))
                end do
                do si1_l254 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l254) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl8_xint((si1_l254) + 1)) * (1.0_c_double - x_inl8_xint((si1_l254) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl8_xint((si1_l254) + 1))))))
                end do
                do si1_l255 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l255) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl8_xint((si1_l255) + 1)) * x_inl8_xint((si1_l255) + 1)) * x_inl8_xint((si1_l255) + 1))
                end do
                do x_w0_256 = 0, (np_particles) - 1
                    x_inl8_idx((x_w0_256) + 1) = (x_inl8_j((x_w0_256) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 4)) then
                do x_w0_257 = 0, (np_particles) - 1
                    x_inl8_j((x_w0_257) + 1) = INT((x_inl1_y((x_w0_257) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_258 = 0, (np_particles) - 1
                    x_inl8_xint((x_w0_258) + 1) = (x_inl1_y((x_w0_258) + 1) - x_inl8_j((x_w0_258) + 1))
                end do
                do x_w0_259 = 0, (np_particles) - 1
                    x_inl8_sm((x_w0_259) + 1) = (0.5_c_double - x_inl8_xint((x_w0_259) + 1))
                end do
                do x_w0_260 = 0, (np_particles) - 1
                    x_inl8_sp((x_w0_260) + 1) = (0.5_c_double + x_inl8_xint((x_w0_260) + 1))
                end do
                do si1_l261 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l261) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl8_sm((si1_l261) + 1)) * x_inl8_sm((si1_l261) + 1)) * x_inl8_sm((si1_l261) + 1)) * &
                    &x_inl8_sm((si1_l261) + 1))
                end do
                do si1_l262 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l262) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl8_xint((si1_l262) + 1))) + (((4.0_c_double * x_inl8_xint((si1_l262) + 1)) * &
                    &x_inl8_xint((si1_l262) + 1)) * ((1.5_c_double + x_inl8_xint((si1_l262) + 1)) - &
                    &(x_inl8_xint((si1_l262) + 1) * x_inl8_xint((si1_l262) + 1))))))
                end do
                do si1_l263 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l263) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl8_xint((si1_l263) + 1)) * x_inl8_xint((si1_l263) + 1)) * &
                    &((x_inl8_xint((si1_l263) + 1) * x_inl8_xint((si1_l263) + 1)) - 2.5_c_double))))
                end do
                do si1_l264 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l264) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl8_xint((si1_l264) + 1))) + (((4.0_c_double * x_inl8_xint((si1_l264) + 1)) * &
                    &x_inl8_xint((si1_l264) + 1)) * ((1.5_c_double - x_inl8_xint((si1_l264) + 1)) - &
                    &(x_inl8_xint((si1_l264) + 1) * x_inl8_xint((si1_l264) + 1))))))
                end do
                do si1_l265 = 0, (np_particles) - 1
                    x_inl1_sy_node_v((si1_l265) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl8_sp((si1_l265) + 1)) * x_inl8_sp((si1_l265) + 1)) * x_inl8_sp((si1_l265) + 1)) * &
                    &x_inl8_sp((si1_l265) + 1))
                end do
                do x_w0_266 = 0, (np_particles) - 1
                    x_inl8_idx((x_w0_266) + 1) = (x_inl8_j((x_w0_266) + 1) - 2)
                end do
            end if
            do x_w0_267 = 0, (np_particles) - 1
                x_inl1_k_node_v((x_w0_267) + 1) = x_inl8_idx((x_w0_267) + 1)
            end do
        end if
        if (((INT(ey_type((1) + 1), c_int64_t) == 0) .OR. (INT(bx_type((1) + 1), c_int64_t) == 0) .OR. &
        &(INT(bz_type((1) + 1), c_int64_t) == 0))) then
            x_inl9_idx = 0
            if ((x_inl1_og == 0)) then
                do x_w0_268 = 0, (np_particles) - 1
                    x_inl9_j((x_w0_268) + 1) = INT(((x_inl1_y((x_w0_268) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do si1_l269 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l269) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_270 = 0, (np_particles) - 1
                    x_inl9_idx((x_w0_270) + 1) = x_inl9_j((x_w0_270) + 1)
                end do
            end if
            if ((x_inl1_og == 1)) then
                do x_w0_271 = 0, (np_particles) - 1
                    x_inl9_j((x_w0_271) + 1) = INT((x_inl1_y((x_w0_271) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_272 = 0, (np_particles) - 1
                    x_inl9_xint((x_w0_272) + 1) = ((x_inl1_y((x_w0_272) + 1) - 0.5_c_double) - x_inl9_j((x_w0_272) + 1))
                end do
                do si1_l273 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l273) + 1, (0) + 1) = (1.0_c_double - x_inl9_xint((si1_l273) + 1))
                end do
                do si1_l274 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l274) + 1, (1) + 1) = x_inl9_xint((si1_l274) + 1)
                end do
                do x_w0_275 = 0, (np_particles) - 1
                    x_inl9_idx((x_w0_275) + 1) = x_inl9_j((x_w0_275) + 1)
                end do
            end if
            if ((x_inl1_og == 2)) then
                do x_w0_276 = 0, (np_particles) - 1
                    x_inl9_j((x_w0_276) + 1) = INT(((x_inl1_y((x_w0_276) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_277 = 0, (np_particles) - 1
                    x_inl9_xint((x_w0_277) + 1) = ((x_inl1_y((x_w0_277) + 1) - 0.5_c_double) - x_inl9_j((x_w0_277) + 1))
                end do
                do si1_l278 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l278) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - &
                    &x_inl9_xint((si1_l278) + 1))) * (0.5_c_double - x_inl9_xint((si1_l278) + 1)))
                end do
                do si1_l279 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l279) + 1, (1) + 1) = (0.75_c_double - (x_inl9_xint((si1_l279) + 1) * &
                    &x_inl9_xint((si1_l279) + 1)))
                end do
                do si1_l280 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l280) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + &
                    &x_inl9_xint((si1_l280) + 1))) * (0.5_c_double + x_inl9_xint((si1_l280) + 1)))
                end do
                do x_w0_281 = 0, (np_particles) - 1
                    x_inl9_idx((x_w0_281) + 1) = (x_inl9_j((x_w0_281) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 3)) then
                do x_w0_282 = 0, (np_particles) - 1
                    x_inl9_j((x_w0_282) + 1) = INT((x_inl1_y((x_w0_282) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_283 = 0, (np_particles) - 1
                    x_inl9_xint((x_w0_283) + 1) = ((x_inl1_y((x_w0_283) + 1) - 0.5_c_double) - x_inl9_j((x_w0_283) + 1))
                end do
                do si1_l284 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l284) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl9_xint((si1_l284) + 1))) * (1.0_c_double - x_inl9_xint((si1_l284) + 1))) * (1.0_c_double - &
                    &x_inl9_xint((si1_l284) + 1)))
                end do
                do si1_l285 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l285) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl9_xint((si1_l285) + 1) * x_inl9_xint((si1_l285) + 1)) * (1.0_c_double - &
                    &(x_inl9_xint((si1_l285) + 1) / 2.0_c_double))))
                end do
                do si1_l286 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l286) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl9_xint((si1_l286) + 1)) * (1.0_c_double - x_inl9_xint((si1_l286) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl9_xint((si1_l286) + 1))))))
                end do
                do si1_l287 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l287) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl9_xint((si1_l287) + 1)) * x_inl9_xint((si1_l287) + 1)) * x_inl9_xint((si1_l287) + 1))
                end do
                do x_w0_288 = 0, (np_particles) - 1
                    x_inl9_idx((x_w0_288) + 1) = (x_inl9_j((x_w0_288) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 4)) then
                do x_w0_289 = 0, (np_particles) - 1
                    x_inl9_j((x_w0_289) + 1) = INT(((x_inl1_y((x_w0_289) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_290 = 0, (np_particles) - 1
                    x_inl9_xint((x_w0_290) + 1) = ((x_inl1_y((x_w0_290) + 1) - 0.5_c_double) - x_inl9_j((x_w0_290) + 1))
                end do
                do x_w0_291 = 0, (np_particles) - 1
                    x_inl9_sm((x_w0_291) + 1) = (0.5_c_double - x_inl9_xint((x_w0_291) + 1))
                end do
                do x_w0_292 = 0, (np_particles) - 1
                    x_inl9_sp((x_w0_292) + 1) = (0.5_c_double + x_inl9_xint((x_w0_292) + 1))
                end do
                do si1_l293 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l293) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl9_sm((si1_l293) + 1)) * x_inl9_sm((si1_l293) + 1)) * x_inl9_sm((si1_l293) + 1)) * &
                    &x_inl9_sm((si1_l293) + 1))
                end do
                do si1_l294 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l294) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl9_xint((si1_l294) + 1))) + (((4.0_c_double * x_inl9_xint((si1_l294) + 1)) * &
                    &x_inl9_xint((si1_l294) + 1)) * ((1.5_c_double + x_inl9_xint((si1_l294) + 1)) - &
                    &(x_inl9_xint((si1_l294) + 1) * x_inl9_xint((si1_l294) + 1))))))
                end do
                do si1_l295 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l295) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl9_xint((si1_l295) + 1)) * x_inl9_xint((si1_l295) + 1)) * &
                    &((x_inl9_xint((si1_l295) + 1) * x_inl9_xint((si1_l295) + 1)) - 2.5_c_double))))
                end do
                do si1_l296 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l296) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl9_xint((si1_l296) + 1))) + (((4.0_c_double * x_inl9_xint((si1_l296) + 1)) * &
                    &x_inl9_xint((si1_l296) + 1)) * ((1.5_c_double - x_inl9_xint((si1_l296) + 1)) - &
                    &(x_inl9_xint((si1_l296) + 1) * x_inl9_xint((si1_l296) + 1))))))
                end do
                do si1_l297 = 0, (np_particles) - 1
                    x_inl1_sy_cell_v((si1_l297) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl9_sp((si1_l297) + 1)) * x_inl9_sp((si1_l297) + 1)) * x_inl9_sp((si1_l297) + 1)) * &
                    &x_inl9_sp((si1_l297) + 1))
                end do
                do x_w0_298 = 0, (np_particles) - 1
                    x_inl9_idx((x_w0_298) + 1) = (x_inl9_j((x_w0_298) + 1) - 2)
                end do
            end if
            do x_w0_299 = 0, (np_particles) - 1
                x_inl1_k_cell_v((x_w0_299) + 1) = x_inl9_idx((x_w0_299) + 1)
            end do
        end if
        if (.not. allocated(x_cb15)) then
            allocate(x_cb15(np_particles, (o + 1)))
        else if (size(x_cb15, 1) /= (np_particles) .or. size(x_cb15, 2) /= ((o + 1))) then
            deallocate(x_cb15)
            allocate(x_cb15(np_particles, (o + 1)))
        end if
        ! numpy: np.where(ex_type[1] == 1, __inl1_sy_node, __inl1_sy_cell)
        do x_r0_300 = 0, ((o + 1)) - 1
            do x_r1_301 = 0, (np_particles) - 1
                if ((INT(ex_type((1) + 1), c_int64_t) == 1)) then
                    x_ifexp12 = x_inl1_sy_node((x_r1_301) + 1, (x_r0_300) + 1)
                else
                    x_ifexp12 = x_inl1_sy_cell((x_r1_301) + 1, (x_r0_300) + 1)
                end if
                x_cb15((x_r1_301) + 1, (x_r0_300) + 1) = x_ifexp12
            end do
        end do
        if (.not. allocated(x_inl1_sy_ex)) then
            allocate(x_inl1_sy_ex(np_particles, (o + 1)))
        else if (size(x_inl1_sy_ex, 1) /= (np_particles) .or. size(x_inl1_sy_ex, 2) /= ((o + 1))) then
            deallocate(x_inl1_sy_ex)
            allocate(x_inl1_sy_ex(np_particles, (o + 1)))
        end if
        do x_w0_302 = 0, ((o + 1)) - 1
            do x_w1_303 = 0, (np_particles) - 1
                x_inl1_sy_ex((x_w1_303) + 1, (x_w0_302) + 1) = x_cb15((x_w1_303) + 1, (x_w0_302) + 1)
            end do
        end do
        if (.not. allocated(x_cb16)) then
            allocate(x_cb16(np_particles, ((o - gal) + 1)))
        else if (size(x_cb16, 1) /= (np_particles) .or. size(x_cb16, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb16)
            allocate(x_cb16(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(ey_type[1] == 1, __inl1_sy_node_v, __inl1_sy_cell_v)
        do x_r0_304 = 0, (((o - gal) + 1)) - 1
            do x_r1_305 = 0, (np_particles) - 1
                if ((INT(ey_type((1) + 1), c_int64_t) == 1)) then
                    x_ifexp13 = x_inl1_sy_node_v((x_r1_305) + 1, (x_r0_304) + 1)
                else
                    x_ifexp13 = x_inl1_sy_cell_v((x_r1_305) + 1, (x_r0_304) + 1)
                end if
                x_cb16((x_r1_305) + 1, (x_r0_304) + 1) = x_ifexp13
            end do
        end do
        if (.not. allocated(x_inl1_sy_ey)) then
            allocate(x_inl1_sy_ey(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sy_ey, 1) /= (np_particles) .or. size(x_inl1_sy_ey, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sy_ey)
            allocate(x_inl1_sy_ey(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_306 = 0, (((o - gal) + 1)) - 1
            do x_w1_307 = 0, (np_particles) - 1
                x_inl1_sy_ey((x_w1_307) + 1, (x_w0_306) + 1) = x_cb16((x_w1_307) + 1, (x_w0_306) + 1)
            end do
        end do
        if (.not. allocated(x_cb17)) then
            allocate(x_cb17(np_particles, (o + 1)))
        else if (size(x_cb17, 1) /= (np_particles) .or. size(x_cb17, 2) /= ((o + 1))) then
            deallocate(x_cb17)
            allocate(x_cb17(np_particles, (o + 1)))
        end if
        ! numpy: np.where(ez_type[1] == 1, __inl1_sy_node, __inl1_sy_cell)
        do x_r0_308 = 0, ((o + 1)) - 1
            do x_r1_309 = 0, (np_particles) - 1
                if ((INT(ez_type((1) + 1), c_int64_t) == 1)) then
                    x_ifexp14 = x_inl1_sy_node((x_r1_309) + 1, (x_r0_308) + 1)
                else
                    x_ifexp14 = x_inl1_sy_cell((x_r1_309) + 1, (x_r0_308) + 1)
                end if
                x_cb17((x_r1_309) + 1, (x_r0_308) + 1) = x_ifexp14
            end do
        end do
        if (.not. allocated(x_inl1_sy_ez)) then
            allocate(x_inl1_sy_ez(np_particles, (o + 1)))
        else if (size(x_inl1_sy_ez, 1) /= (np_particles) .or. size(x_inl1_sy_ez, 2) /= ((o + 1))) then
            deallocate(x_inl1_sy_ez)
            allocate(x_inl1_sy_ez(np_particles, (o + 1)))
        end if
        do x_w0_310 = 0, ((o + 1)) - 1
            do x_w1_311 = 0, (np_particles) - 1
                x_inl1_sy_ez((x_w1_311) + 1, (x_w0_310) + 1) = x_cb17((x_w1_311) + 1, (x_w0_310) + 1)
            end do
        end do
        if (.not. allocated(x_cb18)) then
            allocate(x_cb18(np_particles, ((o - gal) + 1)))
        else if (size(x_cb18, 1) /= (np_particles) .or. size(x_cb18, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb18)
            allocate(x_cb18(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(bx_type[1] == 1, __inl1_sy_node_v, __inl1_sy_cell_v)
        do x_r0_312 = 0, (((o - gal) + 1)) - 1
            do x_r1_313 = 0, (np_particles) - 1
                if ((INT(bx_type((1) + 1), c_int64_t) == 1)) then
                    x_ifexp15 = x_inl1_sy_node_v((x_r1_313) + 1, (x_r0_312) + 1)
                else
                    x_ifexp15 = x_inl1_sy_cell_v((x_r1_313) + 1, (x_r0_312) + 1)
                end if
                x_cb18((x_r1_313) + 1, (x_r0_312) + 1) = x_ifexp15
            end do
        end do
        if (.not. allocated(x_inl1_sy_bx)) then
            allocate(x_inl1_sy_bx(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sy_bx, 1) /= (np_particles) .or. size(x_inl1_sy_bx, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sy_bx)
            allocate(x_inl1_sy_bx(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_314 = 0, (((o - gal) + 1)) - 1
            do x_w1_315 = 0, (np_particles) - 1
                x_inl1_sy_bx((x_w1_315) + 1, (x_w0_314) + 1) = x_cb18((x_w1_315) + 1, (x_w0_314) + 1)
            end do
        end do
        if (.not. allocated(x_cb19)) then
            allocate(x_cb19(np_particles, (o + 1)))
        else if (size(x_cb19, 1) /= (np_particles) .or. size(x_cb19, 2) /= ((o + 1))) then
            deallocate(x_cb19)
            allocate(x_cb19(np_particles, (o + 1)))
        end if
        ! numpy: np.where(by_type[1] == 1, __inl1_sy_node, __inl1_sy_cell)
        do x_r0_316 = 0, ((o + 1)) - 1
            do x_r1_317 = 0, (np_particles) - 1
                if ((INT(by_type((1) + 1), c_int64_t) == 1)) then
                    x_ifexp16 = x_inl1_sy_node((x_r1_317) + 1, (x_r0_316) + 1)
                else
                    x_ifexp16 = x_inl1_sy_cell((x_r1_317) + 1, (x_r0_316) + 1)
                end if
                x_cb19((x_r1_317) + 1, (x_r0_316) + 1) = x_ifexp16
            end do
        end do
        if (.not. allocated(x_inl1_sy_by)) then
            allocate(x_inl1_sy_by(np_particles, (o + 1)))
        else if (size(x_inl1_sy_by, 1) /= (np_particles) .or. size(x_inl1_sy_by, 2) /= ((o + 1))) then
            deallocate(x_inl1_sy_by)
            allocate(x_inl1_sy_by(np_particles, (o + 1)))
        end if
        do x_w0_318 = 0, ((o + 1)) - 1
            do x_w1_319 = 0, (np_particles) - 1
                x_inl1_sy_by((x_w1_319) + 1, (x_w0_318) + 1) = x_cb19((x_w1_319) + 1, (x_w0_318) + 1)
            end do
        end do
        if (.not. allocated(x_cb20)) then
            allocate(x_cb20(np_particles, ((o - gal) + 1)))
        else if (size(x_cb20, 1) /= (np_particles) .or. size(x_cb20, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb20)
            allocate(x_cb20(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(bz_type[1] == 1, __inl1_sy_node_v, __inl1_sy_cell_v)
        do x_r0_320 = 0, (((o - gal) + 1)) - 1
            do x_r1_321 = 0, (np_particles) - 1
                if ((INT(bz_type((1) + 1), c_int64_t) == 1)) then
                    x_ifexp17 = x_inl1_sy_node_v((x_r1_321) + 1, (x_r0_320) + 1)
                else
                    x_ifexp17 = x_inl1_sy_cell_v((x_r1_321) + 1, (x_r0_320) + 1)
                end if
                x_cb20((x_r1_321) + 1, (x_r0_320) + 1) = x_ifexp17
            end do
        end do
        if (.not. allocated(x_inl1_sy_bz)) then
            allocate(x_inl1_sy_bz(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sy_bz, 1) /= (np_particles) .or. size(x_inl1_sy_bz, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sy_bz)
            allocate(x_inl1_sy_bz(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_322 = 0, (((o - gal) + 1)) - 1
            do x_w1_323 = 0, (np_particles) - 1
                x_inl1_sy_bz((x_w1_323) + 1, (x_w0_322) + 1) = x_cb20((x_w1_323) + 1, (x_w0_322) + 1)
            end do
        end do
        x_inl1_n_sy_ey = (x_inl1_og + 1)
        x_inl1_n_sy_bx = (x_inl1_og + 1)
        x_inl1_n_sy_bz = (x_inl1_og + 1)
        x_inl1_n_sy_ex = (x_inl1_o + 1)
        x_inl1_n_sy_ez = (x_inl1_o + 1)
        x_inl1_n_sy_by = (x_inl1_o + 1)
        ! numpy: np.where(ex_type[1] == 1, __inl1_k_node, __inl1_k_cell)
        do x_r0_324 = 0, (np_particles) - 1
            if ((INT(ex_type((1) + 1), c_int64_t) == 1)) then
                x_ifexp18 = x_inl1_k_node((x_r0_324) + 1)
            else
                x_ifexp18 = x_inl1_k_cell((x_r0_324) + 1)
            end if
            x_cb21((x_r0_324) + 1) = x_ifexp18
        end do
        do x_w0_325 = 0, (np_particles) - 1
            x_inl1_k_ex((x_w0_325) + 1) = x_cb21((x_w0_325) + 1)
        end do
        ! numpy: np.where(ey_type[1] == 1, __inl1_k_node_v, __inl1_k_cell_v)
        do x_r0_326 = 0, (np_particles) - 1
            if ((INT(ey_type((1) + 1), c_int64_t) == 1)) then
                x_ifexp19 = x_inl1_k_node_v((x_r0_326) + 1)
            else
                x_ifexp19 = x_inl1_k_cell_v((x_r0_326) + 1)
            end if
            x_cb22((x_r0_326) + 1) = x_ifexp19
        end do
        do x_w0_327 = 0, (np_particles) - 1
            x_inl1_k_ey((x_w0_327) + 1) = x_cb22((x_w0_327) + 1)
        end do
        ! numpy: np.where(ez_type[1] == 1, __inl1_k_node, __inl1_k_cell)
        do x_r0_328 = 0, (np_particles) - 1
            if ((INT(ez_type((1) + 1), c_int64_t) == 1)) then
                x_ifexp20 = x_inl1_k_node((x_r0_328) + 1)
            else
                x_ifexp20 = x_inl1_k_cell((x_r0_328) + 1)
            end if
            x_cb23((x_r0_328) + 1) = x_ifexp20
        end do
        do x_w0_329 = 0, (np_particles) - 1
            x_inl1_k_ez((x_w0_329) + 1) = x_cb23((x_w0_329) + 1)
        end do
        ! numpy: np.where(bx_type[1] == 1, __inl1_k_node_v, __inl1_k_cell_v)
        do x_r0_330 = 0, (np_particles) - 1
            if ((INT(bx_type((1) + 1), c_int64_t) == 1)) then
                x_ifexp21 = x_inl1_k_node_v((x_r0_330) + 1)
            else
                x_ifexp21 = x_inl1_k_cell_v((x_r0_330) + 1)
            end if
            x_cb24((x_r0_330) + 1) = x_ifexp21
        end do
        do x_w0_331 = 0, (np_particles) - 1
            x_inl1_k_bx((x_w0_331) + 1) = x_cb24((x_w0_331) + 1)
        end do
        ! numpy: np.where(by_type[1] == 1, __inl1_k_node, __inl1_k_cell)
        do x_r0_332 = 0, (np_particles) - 1
            if ((INT(by_type((1) + 1), c_int64_t) == 1)) then
                x_ifexp22 = x_inl1_k_node((x_r0_332) + 1)
            else
                x_ifexp22 = x_inl1_k_cell((x_r0_332) + 1)
            end if
            x_cb25((x_r0_332) + 1) = x_ifexp22
        end do
        do x_w0_333 = 0, (np_particles) - 1
            x_inl1_k_by((x_w0_333) + 1) = x_cb25((x_w0_333) + 1)
        end do
        ! numpy: np.where(bz_type[1] == 1, __inl1_k_node_v, __inl1_k_cell_v)
        do x_r0_334 = 0, (np_particles) - 1
            if ((INT(bz_type((1) + 1), c_int64_t) == 1)) then
                x_ifexp23 = x_inl1_k_node_v((x_r0_334) + 1)
            else
                x_ifexp23 = x_inl1_k_cell_v((x_r0_334) + 1)
            end if
            x_cb26((x_r0_334) + 1) = x_ifexp23
        end do
        do x_w0_335 = 0, (np_particles) - 1
            x_inl1_k_bz((x_w0_335) + 1) = x_cb26((x_w0_335) + 1)
        end do
    end if
    if (((g /= 4) .AND. (g /= 5))) then
        do x_w0_336 = 0, (np_particles) - 1
            x_inl1_z((x_w0_336) + 1) = ((zp((x_w0_336) + 1) - xyzmin((2) + 1)) * dinv((2) + 1))
        end do
        if (.not. allocated(x_inl1_sz_node)) then
            allocate(x_inl1_sz_node(np_particles, (o + 1)))
        else if (size(x_inl1_sz_node, 1) /= (np_particles) .or. size(x_inl1_sz_node, 2) /= ((o + 1))) then
            deallocate(x_inl1_sz_node)
            allocate(x_inl1_sz_node(np_particles, (o + 1)))
        end if
        x_inl1_sz_node = 0
        if (.not. allocated(x_inl1_sz_cell)) then
            allocate(x_inl1_sz_cell(np_particles, (o + 1)))
        else if (size(x_inl1_sz_cell, 1) /= (np_particles) .or. size(x_inl1_sz_cell, 2) /= ((o + 1))) then
            deallocate(x_inl1_sz_cell)
            allocate(x_inl1_sz_cell(np_particles, (o + 1)))
        end if
        x_inl1_sz_cell = 0
        if (.not. allocated(x_inl1_sz_node_v)) then
            allocate(x_inl1_sz_node_v(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sz_node_v, 1) /= (np_particles) .or. size(x_inl1_sz_node_v, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sz_node_v)
            allocate(x_inl1_sz_node_v(np_particles, ((o - gal) + 1)))
        end if
        x_inl1_sz_node_v = 0
        if (.not. allocated(x_inl1_sz_cell_v)) then
            allocate(x_inl1_sz_cell_v(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sz_cell_v, 1) /= (np_particles) .or. size(x_inl1_sz_cell_v, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sz_cell_v)
            allocate(x_inl1_sz_cell_v(np_particles, ((o - gal) + 1)))
        end if
        x_inl1_sz_cell_v = 0
        x_inl1_l_node = 0
        x_inl1_l_cell = 0
        x_inl1_l_node_v = 0
        x_inl1_l_cell_v = 0
        if (((INT(ex_type((x_inl1_zdir) + 1), c_int64_t) == 1) .OR. (INT(ey_type((x_inl1_zdir) + 1), c_int64_t) == 1) &
        &.OR. (INT(bz_type((x_inl1_zdir) + 1), c_int64_t) == 1))) then
            x_inl10_idx = 0
            if ((x_inl1_o == 0)) then
                do x_w0_337 = 0, (np_particles) - 1
                    x_inl10_j((x_w0_337) + 1) = INT((x_inl1_z((x_w0_337) + 1) + 0.5_c_double), c_int64_t)
                end do
                do si1_l338 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l338) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_339 = 0, (np_particles) - 1
                    x_inl10_idx((x_w0_339) + 1) = x_inl10_j((x_w0_339) + 1)
                end do
            end if
            if ((x_inl1_o == 1)) then
                do x_w0_340 = 0, (np_particles) - 1
                    x_inl10_j((x_w0_340) + 1) = INT(x_inl1_z((x_w0_340) + 1), c_int64_t)
                end do
                do x_w0_341 = 0, (np_particles) - 1
                    x_inl10_xint((x_w0_341) + 1) = (x_inl1_z((x_w0_341) + 1) - x_inl10_j((x_w0_341) + 1))
                end do
                do si1_l342 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l342) + 1, (0) + 1) = (1.0_c_double - x_inl10_xint((si1_l342) + 1))
                end do
                do si1_l343 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l343) + 1, (1) + 1) = x_inl10_xint((si1_l343) + 1)
                end do
                do x_w0_344 = 0, (np_particles) - 1
                    x_inl10_idx((x_w0_344) + 1) = x_inl10_j((x_w0_344) + 1)
                end do
            end if
            if ((x_inl1_o == 2)) then
                do x_w0_345 = 0, (np_particles) - 1
                    x_inl10_j((x_w0_345) + 1) = INT((x_inl1_z((x_w0_345) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_346 = 0, (np_particles) - 1
                    x_inl10_xint((x_w0_346) + 1) = (x_inl1_z((x_w0_346) + 1) - x_inl10_j((x_w0_346) + 1))
                end do
                do si1_l347 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l347) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl10_xint((si1_l347) &
                    &+ 1))) * (0.5_c_double - x_inl10_xint((si1_l347) + 1)))
                end do
                do si1_l348 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l348) + 1, (1) + 1) = (0.75_c_double - (x_inl10_xint((si1_l348) + 1) * &
                    &x_inl10_xint((si1_l348) + 1)))
                end do
                do si1_l349 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l349) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl10_xint((si1_l349) &
                    &+ 1))) * (0.5_c_double + x_inl10_xint((si1_l349) + 1)))
                end do
                do x_w0_350 = 0, (np_particles) - 1
                    x_inl10_idx((x_w0_350) + 1) = (x_inl10_j((x_w0_350) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 3)) then
                do x_w0_351 = 0, (np_particles) - 1
                    x_inl10_j((x_w0_351) + 1) = INT(x_inl1_z((x_w0_351) + 1), c_int64_t)
                end do
                do x_w0_352 = 0, (np_particles) - 1
                    x_inl10_xint((x_w0_352) + 1) = (x_inl1_z((x_w0_352) + 1) - x_inl10_j((x_w0_352) + 1))
                end do
                do si1_l353 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l353) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl10_xint((si1_l353) + 1))) * (1.0_c_double - x_inl10_xint((si1_l353) + 1))) * (1.0_c_double - &
                    &x_inl10_xint((si1_l353) + 1)))
                end do
                do si1_l354 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l354) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl10_xint((si1_l354) + 1) * x_inl10_xint((si1_l354) + 1)) * (1.0_c_double - &
                    &(x_inl10_xint((si1_l354) + 1) / 2.0_c_double))))
                end do
                do si1_l355 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l355) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl10_xint((si1_l355) + 1)) * (1.0_c_double - x_inl10_xint((si1_l355) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl10_xint((si1_l355) + 1))))))
                end do
                do si1_l356 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l356) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl10_xint((si1_l356) + 1)) * x_inl10_xint((si1_l356) + 1)) * x_inl10_xint((si1_l356) + 1))
                end do
                do x_w0_357 = 0, (np_particles) - 1
                    x_inl10_idx((x_w0_357) + 1) = (x_inl10_j((x_w0_357) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 4)) then
                do x_w0_358 = 0, (np_particles) - 1
                    x_inl10_j((x_w0_358) + 1) = INT((x_inl1_z((x_w0_358) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_359 = 0, (np_particles) - 1
                    x_inl10_xint((x_w0_359) + 1) = (x_inl1_z((x_w0_359) + 1) - x_inl10_j((x_w0_359) + 1))
                end do
                do x_w0_360 = 0, (np_particles) - 1
                    x_inl10_sm((x_w0_360) + 1) = (0.5_c_double - x_inl10_xint((x_w0_360) + 1))
                end do
                do x_w0_361 = 0, (np_particles) - 1
                    x_inl10_sp((x_w0_361) + 1) = (0.5_c_double + x_inl10_xint((x_w0_361) + 1))
                end do
                do si1_l362 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l362) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl10_sm((si1_l362) + 1)) * x_inl10_sm((si1_l362) + 1)) * x_inl10_sm((si1_l362) + 1)) * &
                    &x_inl10_sm((si1_l362) + 1))
                end do
                do si1_l363 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l363) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl10_xint((si1_l363) + 1))) + (((4.0_c_double * x_inl10_xint((si1_l363) + 1)) &
                    &* x_inl10_xint((si1_l363) + 1)) * ((1.5_c_double + x_inl10_xint((si1_l363) + 1)) - &
                    &(x_inl10_xint((si1_l363) + 1) * x_inl10_xint((si1_l363) + 1))))))
                end do
                do si1_l364 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l364) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl10_xint((si1_l364) + 1)) * x_inl10_xint((si1_l364) + 1)) * &
                    &((x_inl10_xint((si1_l364) + 1) * x_inl10_xint((si1_l364) + 1)) - 2.5_c_double))))
                end do
                do si1_l365 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l365) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl10_xint((si1_l365) + 1))) + (((4.0_c_double * x_inl10_xint((si1_l365) + 1)) &
                    &* x_inl10_xint((si1_l365) + 1)) * ((1.5_c_double - x_inl10_xint((si1_l365) + 1)) - &
                    &(x_inl10_xint((si1_l365) + 1) * x_inl10_xint((si1_l365) + 1))))))
                end do
                do si1_l366 = 0, (np_particles) - 1
                    x_inl1_sz_node((si1_l366) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl10_sp((si1_l366) + 1)) * x_inl10_sp((si1_l366) + 1)) * x_inl10_sp((si1_l366) + 1)) * &
                    &x_inl10_sp((si1_l366) + 1))
                end do
                do x_w0_367 = 0, (np_particles) - 1
                    x_inl10_idx((x_w0_367) + 1) = (x_inl10_j((x_w0_367) + 1) - 2)
                end do
            end if
            do x_w0_368 = 0, (np_particles) - 1
                x_inl1_l_node((x_w0_368) + 1) = x_inl10_idx((x_w0_368) + 1)
            end do
        end if
        if (((INT(ex_type((x_inl1_zdir) + 1), c_int64_t) == 0) .OR. (INT(ey_type((x_inl1_zdir) + 1), c_int64_t) == 0) &
        &.OR. (INT(bz_type((x_inl1_zdir) + 1), c_int64_t) == 0))) then
            x_inl11_idx = 0
            if ((x_inl1_o == 0)) then
                do x_w0_369 = 0, (np_particles) - 1
                    x_inl11_j((x_w0_369) + 1) = INT(((x_inl1_z((x_w0_369) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do si1_l370 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l370) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_371 = 0, (np_particles) - 1
                    x_inl11_idx((x_w0_371) + 1) = x_inl11_j((x_w0_371) + 1)
                end do
            end if
            if ((x_inl1_o == 1)) then
                do x_w0_372 = 0, (np_particles) - 1
                    x_inl11_j((x_w0_372) + 1) = INT((x_inl1_z((x_w0_372) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_373 = 0, (np_particles) - 1
                    x_inl11_xint((x_w0_373) + 1) = ((x_inl1_z((x_w0_373) + 1) - 0.5_c_double) - x_inl11_j((x_w0_373) + &
                    &1))
                end do
                do si1_l374 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l374) + 1, (0) + 1) = (1.0_c_double - x_inl11_xint((si1_l374) + 1))
                end do
                do si1_l375 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l375) + 1, (1) + 1) = x_inl11_xint((si1_l375) + 1)
                end do
                do x_w0_376 = 0, (np_particles) - 1
                    x_inl11_idx((x_w0_376) + 1) = x_inl11_j((x_w0_376) + 1)
                end do
            end if
            if ((x_inl1_o == 2)) then
                do x_w0_377 = 0, (np_particles) - 1
                    x_inl11_j((x_w0_377) + 1) = INT(((x_inl1_z((x_w0_377) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_378 = 0, (np_particles) - 1
                    x_inl11_xint((x_w0_378) + 1) = ((x_inl1_z((x_w0_378) + 1) - 0.5_c_double) - x_inl11_j((x_w0_378) + &
                    &1))
                end do
                do si1_l379 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l379) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - x_inl11_xint((si1_l379) &
                    &+ 1))) * (0.5_c_double - x_inl11_xint((si1_l379) + 1)))
                end do
                do si1_l380 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l380) + 1, (1) + 1) = (0.75_c_double - (x_inl11_xint((si1_l380) + 1) * &
                    &x_inl11_xint((si1_l380) + 1)))
                end do
                do si1_l381 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l381) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + x_inl11_xint((si1_l381) &
                    &+ 1))) * (0.5_c_double + x_inl11_xint((si1_l381) + 1)))
                end do
                do x_w0_382 = 0, (np_particles) - 1
                    x_inl11_idx((x_w0_382) + 1) = (x_inl11_j((x_w0_382) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 3)) then
                do x_w0_383 = 0, (np_particles) - 1
                    x_inl11_j((x_w0_383) + 1) = INT((x_inl1_z((x_w0_383) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_384 = 0, (np_particles) - 1
                    x_inl11_xint((x_w0_384) + 1) = ((x_inl1_z((x_w0_384) + 1) - 0.5_c_double) - x_inl11_j((x_w0_384) + &
                    &1))
                end do
                do si1_l385 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l385) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl11_xint((si1_l385) + 1))) * (1.0_c_double - x_inl11_xint((si1_l385) + 1))) * (1.0_c_double - &
                    &x_inl11_xint((si1_l385) + 1)))
                end do
                do si1_l386 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l386) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl11_xint((si1_l386) + 1) * x_inl11_xint((si1_l386) + 1)) * (1.0_c_double - &
                    &(x_inl11_xint((si1_l386) + 1) / 2.0_c_double))))
                end do
                do si1_l387 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l387) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl11_xint((si1_l387) + 1)) * (1.0_c_double - x_inl11_xint((si1_l387) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl11_xint((si1_l387) + 1))))))
                end do
                do si1_l388 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l388) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl11_xint((si1_l388) + 1)) * x_inl11_xint((si1_l388) + 1)) * x_inl11_xint((si1_l388) + 1))
                end do
                do x_w0_389 = 0, (np_particles) - 1
                    x_inl11_idx((x_w0_389) + 1) = (x_inl11_j((x_w0_389) + 1) - 1)
                end do
            end if
            if ((x_inl1_o == 4)) then
                do x_w0_390 = 0, (np_particles) - 1
                    x_inl11_j((x_w0_390) + 1) = INT(((x_inl1_z((x_w0_390) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_391 = 0, (np_particles) - 1
                    x_inl11_xint((x_w0_391) + 1) = ((x_inl1_z((x_w0_391) + 1) - 0.5_c_double) - x_inl11_j((x_w0_391) + &
                    &1))
                end do
                do x_w0_392 = 0, (np_particles) - 1
                    x_inl11_sm((x_w0_392) + 1) = (0.5_c_double - x_inl11_xint((x_w0_392) + 1))
                end do
                do x_w0_393 = 0, (np_particles) - 1
                    x_inl11_sp((x_w0_393) + 1) = (0.5_c_double + x_inl11_xint((x_w0_393) + 1))
                end do
                do si1_l394 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l394) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl11_sm((si1_l394) + 1)) * x_inl11_sm((si1_l394) + 1)) * x_inl11_sm((si1_l394) + 1)) * &
                    &x_inl11_sm((si1_l394) + 1))
                end do
                do si1_l395 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l395) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl11_xint((si1_l395) + 1))) + (((4.0_c_double * x_inl11_xint((si1_l395) + 1)) &
                    &* x_inl11_xint((si1_l395) + 1)) * ((1.5_c_double + x_inl11_xint((si1_l395) + 1)) - &
                    &(x_inl11_xint((si1_l395) + 1) * x_inl11_xint((si1_l395) + 1))))))
                end do
                do si1_l396 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l396) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl11_xint((si1_l396) + 1)) * x_inl11_xint((si1_l396) + 1)) * &
                    &((x_inl11_xint((si1_l396) + 1) * x_inl11_xint((si1_l396) + 1)) - 2.5_c_double))))
                end do
                do si1_l397 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l397) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl11_xint((si1_l397) + 1))) + (((4.0_c_double * x_inl11_xint((si1_l397) + 1)) &
                    &* x_inl11_xint((si1_l397) + 1)) * ((1.5_c_double - x_inl11_xint((si1_l397) + 1)) - &
                    &(x_inl11_xint((si1_l397) + 1) * x_inl11_xint((si1_l397) + 1))))))
                end do
                do si1_l398 = 0, (np_particles) - 1
                    x_inl1_sz_cell((si1_l398) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl11_sp((si1_l398) + 1)) * x_inl11_sp((si1_l398) + 1)) * x_inl11_sp((si1_l398) + 1)) * &
                    &x_inl11_sp((si1_l398) + 1))
                end do
                do x_w0_399 = 0, (np_particles) - 1
                    x_inl11_idx((x_w0_399) + 1) = (x_inl11_j((x_w0_399) + 1) - 2)
                end do
            end if
            do x_w0_400 = 0, (np_particles) - 1
                x_inl1_l_cell((x_w0_400) + 1) = x_inl11_idx((x_w0_400) + 1)
            end do
        end if
        if (((INT(ez_type((x_inl1_zdir) + 1), c_int64_t) == 1) .OR. (INT(bx_type((x_inl1_zdir) + 1), c_int64_t) == 1) &
        &.OR. (INT(by_type((x_inl1_zdir) + 1), c_int64_t) == 1))) then
            x_inl12_idx = 0
            if ((x_inl1_og == 0)) then
                do x_w0_401 = 0, (np_particles) - 1
                    x_inl12_j((x_w0_401) + 1) = INT((x_inl1_z((x_w0_401) + 1) + 0.5_c_double), c_int64_t)
                end do
                do si1_l402 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l402) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_403 = 0, (np_particles) - 1
                    x_inl12_idx((x_w0_403) + 1) = x_inl12_j((x_w0_403) + 1)
                end do
            end if
            if ((x_inl1_og == 1)) then
                do x_w0_404 = 0, (np_particles) - 1
                    x_inl12_j((x_w0_404) + 1) = INT(x_inl1_z((x_w0_404) + 1), c_int64_t)
                end do
                do x_w0_405 = 0, (np_particles) - 1
                    x_inl12_xint((x_w0_405) + 1) = (x_inl1_z((x_w0_405) + 1) - x_inl12_j((x_w0_405) + 1))
                end do
                do si1_l406 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l406) + 1, (0) + 1) = (1.0_c_double - x_inl12_xint((si1_l406) + 1))
                end do
                do si1_l407 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l407) + 1, (1) + 1) = x_inl12_xint((si1_l407) + 1)
                end do
                do x_w0_408 = 0, (np_particles) - 1
                    x_inl12_idx((x_w0_408) + 1) = x_inl12_j((x_w0_408) + 1)
                end do
            end if
            if ((x_inl1_og == 2)) then
                do x_w0_409 = 0, (np_particles) - 1
                    x_inl12_j((x_w0_409) + 1) = INT((x_inl1_z((x_w0_409) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_410 = 0, (np_particles) - 1
                    x_inl12_xint((x_w0_410) + 1) = (x_inl1_z((x_w0_410) + 1) - x_inl12_j((x_w0_410) + 1))
                end do
                do si1_l411 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l411) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - &
                    &x_inl12_xint((si1_l411) + 1))) * (0.5_c_double - x_inl12_xint((si1_l411) + 1)))
                end do
                do si1_l412 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l412) + 1, (1) + 1) = (0.75_c_double - (x_inl12_xint((si1_l412) + 1) * &
                    &x_inl12_xint((si1_l412) + 1)))
                end do
                do si1_l413 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l413) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + &
                    &x_inl12_xint((si1_l413) + 1))) * (0.5_c_double + x_inl12_xint((si1_l413) + 1)))
                end do
                do x_w0_414 = 0, (np_particles) - 1
                    x_inl12_idx((x_w0_414) + 1) = (x_inl12_j((x_w0_414) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 3)) then
                do x_w0_415 = 0, (np_particles) - 1
                    x_inl12_j((x_w0_415) + 1) = INT(x_inl1_z((x_w0_415) + 1), c_int64_t)
                end do
                do x_w0_416 = 0, (np_particles) - 1
                    x_inl12_xint((x_w0_416) + 1) = (x_inl1_z((x_w0_416) + 1) - x_inl12_j((x_w0_416) + 1))
                end do
                do si1_l417 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l417) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl12_xint((si1_l417) + 1))) * (1.0_c_double - x_inl12_xint((si1_l417) + 1))) * (1.0_c_double - &
                    &x_inl12_xint((si1_l417) + 1)))
                end do
                do si1_l418 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l418) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl12_xint((si1_l418) + 1) * x_inl12_xint((si1_l418) + 1)) * (1.0_c_double - &
                    &(x_inl12_xint((si1_l418) + 1) / 2.0_c_double))))
                end do
                do si1_l419 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l419) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl12_xint((si1_l419) + 1)) * (1.0_c_double - x_inl12_xint((si1_l419) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl12_xint((si1_l419) + 1))))))
                end do
                do si1_l420 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l420) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl12_xint((si1_l420) + 1)) * x_inl12_xint((si1_l420) + 1)) * x_inl12_xint((si1_l420) + 1))
                end do
                do x_w0_421 = 0, (np_particles) - 1
                    x_inl12_idx((x_w0_421) + 1) = (x_inl12_j((x_w0_421) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 4)) then
                do x_w0_422 = 0, (np_particles) - 1
                    x_inl12_j((x_w0_422) + 1) = INT((x_inl1_z((x_w0_422) + 1) + 0.5_c_double), c_int64_t)
                end do
                do x_w0_423 = 0, (np_particles) - 1
                    x_inl12_xint((x_w0_423) + 1) = (x_inl1_z((x_w0_423) + 1) - x_inl12_j((x_w0_423) + 1))
                end do
                do x_w0_424 = 0, (np_particles) - 1
                    x_inl12_sm((x_w0_424) + 1) = (0.5_c_double - x_inl12_xint((x_w0_424) + 1))
                end do
                do x_w0_425 = 0, (np_particles) - 1
                    x_inl12_sp((x_w0_425) + 1) = (0.5_c_double + x_inl12_xint((x_w0_425) + 1))
                end do
                do si1_l426 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l426) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl12_sm((si1_l426) + 1)) * x_inl12_sm((si1_l426) + 1)) * x_inl12_sm((si1_l426) + 1)) * &
                    &x_inl12_sm((si1_l426) + 1))
                end do
                do si1_l427 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l427) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl12_xint((si1_l427) + 1))) + (((4.0_c_double * x_inl12_xint((si1_l427) + 1)) &
                    &* x_inl12_xint((si1_l427) + 1)) * ((1.5_c_double + x_inl12_xint((si1_l427) + 1)) - &
                    &(x_inl12_xint((si1_l427) + 1) * x_inl12_xint((si1_l427) + 1))))))
                end do
                do si1_l428 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l428) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl12_xint((si1_l428) + 1)) * x_inl12_xint((si1_l428) + 1)) * &
                    &((x_inl12_xint((si1_l428) + 1) * x_inl12_xint((si1_l428) + 1)) - 2.5_c_double))))
                end do
                do si1_l429 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l429) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl12_xint((si1_l429) + 1))) + (((4.0_c_double * x_inl12_xint((si1_l429) + 1)) &
                    &* x_inl12_xint((si1_l429) + 1)) * ((1.5_c_double - x_inl12_xint((si1_l429) + 1)) - &
                    &(x_inl12_xint((si1_l429) + 1) * x_inl12_xint((si1_l429) + 1))))))
                end do
                do si1_l430 = 0, (np_particles) - 1
                    x_inl1_sz_node_v((si1_l430) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl12_sp((si1_l430) + 1)) * x_inl12_sp((si1_l430) + 1)) * x_inl12_sp((si1_l430) + 1)) * &
                    &x_inl12_sp((si1_l430) + 1))
                end do
                do x_w0_431 = 0, (np_particles) - 1
                    x_inl12_idx((x_w0_431) + 1) = (x_inl12_j((x_w0_431) + 1) - 2)
                end do
            end if
            do x_w0_432 = 0, (np_particles) - 1
                x_inl1_l_node_v((x_w0_432) + 1) = x_inl12_idx((x_w0_432) + 1)
            end do
        end if
        if (((INT(ez_type((x_inl1_zdir) + 1), c_int64_t) == 0) .OR. (INT(bx_type((x_inl1_zdir) + 1), c_int64_t) == 0) &
        &.OR. (INT(by_type((x_inl1_zdir) + 1), c_int64_t) == 0))) then
            x_inl13_idx = 0
            if ((x_inl1_og == 0)) then
                do x_w0_433 = 0, (np_particles) - 1
                    x_inl13_j((x_w0_433) + 1) = INT(((x_inl1_z((x_w0_433) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do si1_l434 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l434) + 1, (0) + 1) = 1.0_c_double
                end do
                do x_w0_435 = 0, (np_particles) - 1
                    x_inl13_idx((x_w0_435) + 1) = x_inl13_j((x_w0_435) + 1)
                end do
            end if
            if ((x_inl1_og == 1)) then
                do x_w0_436 = 0, (np_particles) - 1
                    x_inl13_j((x_w0_436) + 1) = INT((x_inl1_z((x_w0_436) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_437 = 0, (np_particles) - 1
                    x_inl13_xint((x_w0_437) + 1) = ((x_inl1_z((x_w0_437) + 1) - 0.5_c_double) - x_inl13_j((x_w0_437) + &
                    &1))
                end do
                do si1_l438 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l438) + 1, (0) + 1) = (1.0_c_double - x_inl13_xint((si1_l438) + 1))
                end do
                do si1_l439 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l439) + 1, (1) + 1) = x_inl13_xint((si1_l439) + 1)
                end do
                do x_w0_440 = 0, (np_particles) - 1
                    x_inl13_idx((x_w0_440) + 1) = x_inl13_j((x_w0_440) + 1)
                end do
            end if
            if ((x_inl1_og == 2)) then
                do x_w0_441 = 0, (np_particles) - 1
                    x_inl13_j((x_w0_441) + 1) = INT(((x_inl1_z((x_w0_441) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_442 = 0, (np_particles) - 1
                    x_inl13_xint((x_w0_442) + 1) = ((x_inl1_z((x_w0_442) + 1) - 0.5_c_double) - x_inl13_j((x_w0_442) + &
                    &1))
                end do
                do si1_l443 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l443) + 1, (0) + 1) = ((0.5_c_double * (0.5_c_double - &
                    &x_inl13_xint((si1_l443) + 1))) * (0.5_c_double - x_inl13_xint((si1_l443) + 1)))
                end do
                do si1_l444 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l444) + 1, (1) + 1) = (0.75_c_double - (x_inl13_xint((si1_l444) + 1) * &
                    &x_inl13_xint((si1_l444) + 1)))
                end do
                do si1_l445 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l445) + 1, (2) + 1) = ((0.5_c_double * (0.5_c_double + &
                    &x_inl13_xint((si1_l445) + 1))) * (0.5_c_double + x_inl13_xint((si1_l445) + 1)))
                end do
                do x_w0_446 = 0, (np_particles) - 1
                    x_inl13_idx((x_w0_446) + 1) = (x_inl13_j((x_w0_446) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 3)) then
                do x_w0_447 = 0, (np_particles) - 1
                    x_inl13_j((x_w0_447) + 1) = INT((x_inl1_z((x_w0_447) + 1) - 0.5_c_double), c_int64_t)
                end do
                do x_w0_448 = 0, (np_particles) - 1
                    x_inl13_xint((x_w0_448) + 1) = ((x_inl1_z((x_w0_448) + 1) - 0.5_c_double) - x_inl13_j((x_w0_448) + &
                    &1))
                end do
                do si1_l449 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l449) + 1, (0) + 1) = ((((1.0_c_double / 6.0_c_double) * (1.0_c_double - &
                    &x_inl13_xint((si1_l449) + 1))) * (1.0_c_double - x_inl13_xint((si1_l449) + 1))) * (1.0_c_double - &
                    &x_inl13_xint((si1_l449) + 1)))
                end do
                do si1_l450 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l450) + 1, (1) + 1) = ((2.0_c_double / 3.0_c_double) - &
                    &((x_inl13_xint((si1_l450) + 1) * x_inl13_xint((si1_l450) + 1)) * (1.0_c_double - &
                    &(x_inl13_xint((si1_l450) + 1) / 2.0_c_double))))
                end do
                do si1_l451 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l451) + 1, (2) + 1) = ((2.0_c_double / 3.0_c_double) - (((1.0_c_double - &
                    &x_inl13_xint((si1_l451) + 1)) * (1.0_c_double - x_inl13_xint((si1_l451) + 1))) * (1.0_c_double - &
                    &(0.5_c_double * (1.0_c_double - x_inl13_xint((si1_l451) + 1))))))
                end do
                do si1_l452 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l452) + 1, (3) + 1) = ((((1.0_c_double / 6.0_c_double) * &
                    &x_inl13_xint((si1_l452) + 1)) * x_inl13_xint((si1_l452) + 1)) * x_inl13_xint((si1_l452) + 1))
                end do
                do x_w0_453 = 0, (np_particles) - 1
                    x_inl13_idx((x_w0_453) + 1) = (x_inl13_j((x_w0_453) + 1) - 1)
                end do
            end if
            if ((x_inl1_og == 4)) then
                do x_w0_454 = 0, (np_particles) - 1
                    x_inl13_j((x_w0_454) + 1) = INT(((x_inl1_z((x_w0_454) + 1) - 0.5_c_double) + 0.5_c_double), &
                    &c_int64_t)
                end do
                do x_w0_455 = 0, (np_particles) - 1
                    x_inl13_xint((x_w0_455) + 1) = ((x_inl1_z((x_w0_455) + 1) - 0.5_c_double) - x_inl13_j((x_w0_455) + &
                    &1))
                end do
                do x_w0_456 = 0, (np_particles) - 1
                    x_inl13_sm((x_w0_456) + 1) = (0.5_c_double - x_inl13_xint((x_w0_456) + 1))
                end do
                do x_w0_457 = 0, (np_particles) - 1
                    x_inl13_sp((x_w0_457) + 1) = (0.5_c_double + x_inl13_xint((x_w0_457) + 1))
                end do
                do si1_l458 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l458) + 1, (0) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl13_sm((si1_l458) + 1)) * x_inl13_sm((si1_l458) + 1)) * x_inl13_sm((si1_l458) + 1)) * &
                    &x_inl13_sm((si1_l458) + 1))
                end do
                do si1_l459 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l459) + 1, (1) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double - &
                    &(11.0_c_double * x_inl13_xint((si1_l459) + 1))) + (((4.0_c_double * x_inl13_xint((si1_l459) + 1)) &
                    &* x_inl13_xint((si1_l459) + 1)) * ((1.5_c_double + x_inl13_xint((si1_l459) + 1)) - &
                    &(x_inl13_xint((si1_l459) + 1) * x_inl13_xint((si1_l459) + 1))))))
                end do
                do si1_l460 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l460) + 1, (2) + 1) = ((1.0_c_double / 24.0_c_double) * (14.375_c_double + &
                    &(((6.0_c_double * x_inl13_xint((si1_l460) + 1)) * x_inl13_xint((si1_l460) + 1)) * &
                    &((x_inl13_xint((si1_l460) + 1) * x_inl13_xint((si1_l460) + 1)) - 2.5_c_double))))
                end do
                do si1_l461 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l461) + 1, (3) + 1) = ((1.0_c_double / 24.0_c_double) * ((4.75_c_double + &
                    &(11.0_c_double * x_inl13_xint((si1_l461) + 1))) + (((4.0_c_double * x_inl13_xint((si1_l461) + 1)) &
                    &* x_inl13_xint((si1_l461) + 1)) * ((1.5_c_double - x_inl13_xint((si1_l461) + 1)) - &
                    &(x_inl13_xint((si1_l461) + 1) * x_inl13_xint((si1_l461) + 1))))))
                end do
                do si1_l462 = 0, (np_particles) - 1
                    x_inl1_sz_cell_v((si1_l462) + 1, (4) + 1) = (((((1.0_c_double / 24.0_c_double) * &
                    &x_inl13_sp((si1_l462) + 1)) * x_inl13_sp((si1_l462) + 1)) * x_inl13_sp((si1_l462) + 1)) * &
                    &x_inl13_sp((si1_l462) + 1))
                end do
                do x_w0_463 = 0, (np_particles) - 1
                    x_inl13_idx((x_w0_463) + 1) = (x_inl13_j((x_w0_463) + 1) - 2)
                end do
            end if
            do x_w0_464 = 0, (np_particles) - 1
                x_inl1_l_cell_v((x_w0_464) + 1) = x_inl13_idx((x_w0_464) + 1)
            end do
        end if
        if (.not. allocated(x_cb27)) then
            allocate(x_cb27(np_particles, (o + 1)))
        else if (size(x_cb27, 1) /= (np_particles) .or. size(x_cb27, 2) /= ((o + 1))) then
            deallocate(x_cb27)
            allocate(x_cb27(np_particles, (o + 1)))
        end if
        ! numpy: np.where(ex_type[__inl1_zdir] == 1, __inl1_sz_node, __inl1_sz_cell)
        do x_r0_465 = 0, ((o + 1)) - 1
            do x_r1_466 = 0, (np_particles) - 1
                if ((INT(ex_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                    x_ifexp24 = x_inl1_sz_node((x_r1_466) + 1, (x_r0_465) + 1)
                else
                    x_ifexp24 = x_inl1_sz_cell((x_r1_466) + 1, (x_r0_465) + 1)
                end if
                x_cb27((x_r1_466) + 1, (x_r0_465) + 1) = x_ifexp24
            end do
        end do
        if (.not. allocated(x_inl1_sz_ex)) then
            allocate(x_inl1_sz_ex(np_particles, (o + 1)))
        else if (size(x_inl1_sz_ex, 1) /= (np_particles) .or. size(x_inl1_sz_ex, 2) /= ((o + 1))) then
            deallocate(x_inl1_sz_ex)
            allocate(x_inl1_sz_ex(np_particles, (o + 1)))
        end if
        do x_w0_467 = 0, ((o + 1)) - 1
            do x_w1_468 = 0, (np_particles) - 1
                x_inl1_sz_ex((x_w1_468) + 1, (x_w0_467) + 1) = x_cb27((x_w1_468) + 1, (x_w0_467) + 1)
            end do
        end do
        if (.not. allocated(x_cb28)) then
            allocate(x_cb28(np_particles, (o + 1)))
        else if (size(x_cb28, 1) /= (np_particles) .or. size(x_cb28, 2) /= ((o + 1))) then
            deallocate(x_cb28)
            allocate(x_cb28(np_particles, (o + 1)))
        end if
        ! numpy: np.where(ey_type[__inl1_zdir] == 1, __inl1_sz_node, __inl1_sz_cell)
        do x_r0_469 = 0, ((o + 1)) - 1
            do x_r1_470 = 0, (np_particles) - 1
                if ((INT(ey_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                    x_ifexp25 = x_inl1_sz_node((x_r1_470) + 1, (x_r0_469) + 1)
                else
                    x_ifexp25 = x_inl1_sz_cell((x_r1_470) + 1, (x_r0_469) + 1)
                end if
                x_cb28((x_r1_470) + 1, (x_r0_469) + 1) = x_ifexp25
            end do
        end do
        if (.not. allocated(x_inl1_sz_ey)) then
            allocate(x_inl1_sz_ey(np_particles, (o + 1)))
        else if (size(x_inl1_sz_ey, 1) /= (np_particles) .or. size(x_inl1_sz_ey, 2) /= ((o + 1))) then
            deallocate(x_inl1_sz_ey)
            allocate(x_inl1_sz_ey(np_particles, (o + 1)))
        end if
        do x_w0_471 = 0, ((o + 1)) - 1
            do x_w1_472 = 0, (np_particles) - 1
                x_inl1_sz_ey((x_w1_472) + 1, (x_w0_471) + 1) = x_cb28((x_w1_472) + 1, (x_w0_471) + 1)
            end do
        end do
        if (.not. allocated(x_cb29)) then
            allocate(x_cb29(np_particles, ((o - gal) + 1)))
        else if (size(x_cb29, 1) /= (np_particles) .or. size(x_cb29, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb29)
            allocate(x_cb29(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(ez_type[__inl1_zdir] == 1, __inl1_sz_node_v, __inl1_sz_cell_v)
        do x_r0_473 = 0, (((o - gal) + 1)) - 1
            do x_r1_474 = 0, (np_particles) - 1
                if ((INT(ez_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                    x_ifexp26 = x_inl1_sz_node_v((x_r1_474) + 1, (x_r0_473) + 1)
                else
                    x_ifexp26 = x_inl1_sz_cell_v((x_r1_474) + 1, (x_r0_473) + 1)
                end if
                x_cb29((x_r1_474) + 1, (x_r0_473) + 1) = x_ifexp26
            end do
        end do
        if (.not. allocated(x_inl1_sz_ez)) then
            allocate(x_inl1_sz_ez(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sz_ez, 1) /= (np_particles) .or. size(x_inl1_sz_ez, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sz_ez)
            allocate(x_inl1_sz_ez(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_475 = 0, (((o - gal) + 1)) - 1
            do x_w1_476 = 0, (np_particles) - 1
                x_inl1_sz_ez((x_w1_476) + 1, (x_w0_475) + 1) = x_cb29((x_w1_476) + 1, (x_w0_475) + 1)
            end do
        end do
        if (.not. allocated(x_cb30)) then
            allocate(x_cb30(np_particles, ((o - gal) + 1)))
        else if (size(x_cb30, 1) /= (np_particles) .or. size(x_cb30, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb30)
            allocate(x_cb30(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(bx_type[__inl1_zdir] == 1, __inl1_sz_node_v, __inl1_sz_cell_v)
        do x_r0_477 = 0, (((o - gal) + 1)) - 1
            do x_r1_478 = 0, (np_particles) - 1
                if ((INT(bx_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                    x_ifexp27 = x_inl1_sz_node_v((x_r1_478) + 1, (x_r0_477) + 1)
                else
                    x_ifexp27 = x_inl1_sz_cell_v((x_r1_478) + 1, (x_r0_477) + 1)
                end if
                x_cb30((x_r1_478) + 1, (x_r0_477) + 1) = x_ifexp27
            end do
        end do
        if (.not. allocated(x_inl1_sz_bx)) then
            allocate(x_inl1_sz_bx(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sz_bx, 1) /= (np_particles) .or. size(x_inl1_sz_bx, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sz_bx)
            allocate(x_inl1_sz_bx(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_479 = 0, (((o - gal) + 1)) - 1
            do x_w1_480 = 0, (np_particles) - 1
                x_inl1_sz_bx((x_w1_480) + 1, (x_w0_479) + 1) = x_cb30((x_w1_480) + 1, (x_w0_479) + 1)
            end do
        end do
        if (.not. allocated(x_cb31)) then
            allocate(x_cb31(np_particles, ((o - gal) + 1)))
        else if (size(x_cb31, 1) /= (np_particles) .or. size(x_cb31, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb31)
            allocate(x_cb31(np_particles, ((o - gal) + 1)))
        end if
        ! numpy: np.where(by_type[__inl1_zdir] == 1, __inl1_sz_node_v, __inl1_sz_cell_v)
        do x_r0_481 = 0, (((o - gal) + 1)) - 1
            do x_r1_482 = 0, (np_particles) - 1
                if ((INT(by_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                    x_ifexp28 = x_inl1_sz_node_v((x_r1_482) + 1, (x_r0_481) + 1)
                else
                    x_ifexp28 = x_inl1_sz_cell_v((x_r1_482) + 1, (x_r0_481) + 1)
                end if
                x_cb31((x_r1_482) + 1, (x_r0_481) + 1) = x_ifexp28
            end do
        end do
        if (.not. allocated(x_inl1_sz_by)) then
            allocate(x_inl1_sz_by(np_particles, ((o - gal) + 1)))
        else if (size(x_inl1_sz_by, 1) /= (np_particles) .or. size(x_inl1_sz_by, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl1_sz_by)
            allocate(x_inl1_sz_by(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_483 = 0, (((o - gal) + 1)) - 1
            do x_w1_484 = 0, (np_particles) - 1
                x_inl1_sz_by((x_w1_484) + 1, (x_w0_483) + 1) = x_cb31((x_w1_484) + 1, (x_w0_483) + 1)
            end do
        end do
        if (.not. allocated(x_cb32)) then
            allocate(x_cb32(np_particles, (o + 1)))
        else if (size(x_cb32, 1) /= (np_particles) .or. size(x_cb32, 2) /= ((o + 1))) then
            deallocate(x_cb32)
            allocate(x_cb32(np_particles, (o + 1)))
        end if
        ! numpy: np.where(bz_type[__inl1_zdir] == 1, __inl1_sz_node, __inl1_sz_cell)
        do x_r0_485 = 0, ((o + 1)) - 1
            do x_r1_486 = 0, (np_particles) - 1
                if ((INT(bz_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                    x_ifexp29 = x_inl1_sz_node((x_r1_486) + 1, (x_r0_485) + 1)
                else
                    x_ifexp29 = x_inl1_sz_cell((x_r1_486) + 1, (x_r0_485) + 1)
                end if
                x_cb32((x_r1_486) + 1, (x_r0_485) + 1) = x_ifexp29
            end do
        end do
        if (.not. allocated(x_inl1_sz_bz)) then
            allocate(x_inl1_sz_bz(np_particles, (o + 1)))
        else if (size(x_inl1_sz_bz, 1) /= (np_particles) .or. size(x_inl1_sz_bz, 2) /= ((o + 1))) then
            deallocate(x_inl1_sz_bz)
            allocate(x_inl1_sz_bz(np_particles, (o + 1)))
        end if
        do x_w0_487 = 0, ((o + 1)) - 1
            do x_w1_488 = 0, (np_particles) - 1
                x_inl1_sz_bz((x_w1_488) + 1, (x_w0_487) + 1) = x_cb32((x_w1_488) + 1, (x_w0_487) + 1)
            end do
        end do
        x_inl1_n_sz_ez = (x_inl1_og + 1)
        x_inl1_n_sz_bx = (x_inl1_og + 1)
        x_inl1_n_sz_by = (x_inl1_og + 1)
        x_inl1_n_sz_ex = (x_inl1_o + 1)
        x_inl1_n_sz_ey = (x_inl1_o + 1)
        x_inl1_n_sz_bz = (x_inl1_o + 1)
        ! numpy: np.where(ex_type[__inl1_zdir] == 1, __inl1_l_node, __inl1_l_cell)
        do x_r0_489 = 0, (np_particles) - 1
            if ((INT(ex_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                x_ifexp30 = x_inl1_l_node((x_r0_489) + 1)
            else
                x_ifexp30 = x_inl1_l_cell((x_r0_489) + 1)
            end if
            x_cb33((x_r0_489) + 1) = x_ifexp30
        end do
        do x_w0_490 = 0, (np_particles) - 1
            x_inl1_l_ex((x_w0_490) + 1) = x_cb33((x_w0_490) + 1)
        end do
        ! numpy: np.where(ey_type[__inl1_zdir] == 1, __inl1_l_node, __inl1_l_cell)
        do x_r0_491 = 0, (np_particles) - 1
            if ((INT(ey_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                x_ifexp31 = x_inl1_l_node((x_r0_491) + 1)
            else
                x_ifexp31 = x_inl1_l_cell((x_r0_491) + 1)
            end if
            x_cb34((x_r0_491) + 1) = x_ifexp31
        end do
        do x_w0_492 = 0, (np_particles) - 1
            x_inl1_l_ey((x_w0_492) + 1) = x_cb34((x_w0_492) + 1)
        end do
        ! numpy: np.where(ez_type[__inl1_zdir] == 1, __inl1_l_node_v, __inl1_l_cell_v)
        do x_r0_493 = 0, (np_particles) - 1
            if ((INT(ez_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                x_ifexp32 = x_inl1_l_node_v((x_r0_493) + 1)
            else
                x_ifexp32 = x_inl1_l_cell_v((x_r0_493) + 1)
            end if
            x_cb35((x_r0_493) + 1) = x_ifexp32
        end do
        do x_w0_494 = 0, (np_particles) - 1
            x_inl1_l_ez((x_w0_494) + 1) = x_cb35((x_w0_494) + 1)
        end do
        ! numpy: np.where(bx_type[__inl1_zdir] == 1, __inl1_l_node_v, __inl1_l_cell_v)
        do x_r0_495 = 0, (np_particles) - 1
            if ((INT(bx_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                x_ifexp33 = x_inl1_l_node_v((x_r0_495) + 1)
            else
                x_ifexp33 = x_inl1_l_cell_v((x_r0_495) + 1)
            end if
            x_cb36((x_r0_495) + 1) = x_ifexp33
        end do
        do x_w0_496 = 0, (np_particles) - 1
            x_inl1_l_bx((x_w0_496) + 1) = x_cb36((x_w0_496) + 1)
        end do
        ! numpy: np.where(by_type[__inl1_zdir] == 1, __inl1_l_node_v, __inl1_l_cell_v)
        do x_r0_497 = 0, (np_particles) - 1
            if ((INT(by_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                x_ifexp34 = x_inl1_l_node_v((x_r0_497) + 1)
            else
                x_ifexp34 = x_inl1_l_cell_v((x_r0_497) + 1)
            end if
            x_cb37((x_r0_497) + 1) = x_ifexp34
        end do
        do x_w0_498 = 0, (np_particles) - 1
            x_inl1_l_by((x_w0_498) + 1) = x_cb37((x_w0_498) + 1)
        end do
        ! numpy: np.where(bz_type[__inl1_zdir] == 1, __inl1_l_node, __inl1_l_cell)
        do x_r0_499 = 0, (np_particles) - 1
            if ((INT(bz_type((x_inl1_zdir) + 1), c_int64_t) == 1)) then
                x_ifexp35 = x_inl1_l_node((x_r0_499) + 1)
            else
                x_ifexp35 = x_inl1_l_cell((x_r0_499) + 1)
            end if
            x_cb38((x_r0_499) + 1) = x_ifexp35
        end do
        do x_w0_500 = 0, (np_particles) - 1
            x_inl1_l_bz((x_w0_500) + 1) = x_cb38((x_w0_500) + 1)
        end do
    end if
    x_inl1_lox = INT(lo((0) + 1), c_int64_t)
    x_inl1_loy = INT(lo((1) + 1), c_int64_t)
    x_inl1_loz = INT(lo((2) + 1), c_int64_t)
    if ((g == 0)) then
        if (.not. allocated(x_cb39)) then
            allocate(x_cb39((o + 1)))
        else if (size(x_cb39, 1) /= ((o + 1))) then
            deallocate(x_cb39)
            allocate(x_cb39((o + 1)))
        end if
        if (.not. allocated(x_cb39)) then
            allocate(x_cb39((o + 1)))
        else if (size(x_cb39, 1) /= ((o + 1))) then
            deallocate(x_cb39)
            allocate(x_cb39((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ey)
        do x_i_501 = 0, (x_inl1_n_sz_ey) - 1
            x_cb39((x_i_501) + 1) = x_i_501
        end do
        if (.not. allocated(x_inl14_taps)) then
            allocate(x_inl14_taps((o + 1)))
        else if (size(x_inl14_taps, 1) /= ((o + 1))) then
            deallocate(x_inl14_taps)
            allocate(x_inl14_taps((o + 1)))
        end if
        do x_w0_502 = 0, (x_inl1_n_sz_ey) - 1
            x_inl14_taps((x_w0_502) + 1) = x_cb39((x_w0_502) + 1)
        end do
        if (.not. allocated(x_inl14_rows)) then
            allocate(x_inl14_rows(np_particles, (o + 1)))
        else if (size(x_inl14_rows, 1) /= (np_particles) .or. size(x_inl14_rows, 2) /= ((o + 1))) then
            deallocate(x_inl14_rows)
            allocate(x_inl14_rows(np_particles, (o + 1)))
        end if
        do x_w0_503 = 0, (x_inl1_n_sz_ey) - 1
            do x_w1_504 = 0, (np_particles) - 1
                x_inl14_rows((x_w1_504) + 1, (x_w0_503) + 1) = ((x_inl1_lox + x_inl1_l_ey((x_w1_504) + 1)) + &
                &x_inl14_taps((x_w0_503) + 1))
            end do
        end do
        if (.not. allocated(x_inl14_gathered)) then
            allocate(x_inl14_gathered(np_particles, (o + 1)))
        else if (size(x_inl14_gathered, 1) /= (np_particles) .or. size(x_inl14_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl14_gathered)
            allocate(x_inl14_gathered(np_particles, (o + 1)))
        end if
        do x_w0_505 = 0, (x_inl1_n_sz_ey) - 1
            do x_w1_506 = 0, (np_particles) - 1
                x_inl14_gathered((x_w1_506) + 1, (x_w0_505) + 1) = ey_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl14_rows((x_w1_506) + 1, (x_w0_505) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb40)) then
            allocate(x_cb40(np_particles, (o + 1)))
        else if (size(x_cb40, 1) /= (np_particles) .or. size(x_cb40, 2) /= ((o + 1))) then
            deallocate(x_cb40)
            allocate(x_cb40(np_particles, (o + 1)))
        end if
        do x_w0_507 = 0, ((o + 1)) - 1
            do x_w1_508 = 0, (np_particles) - 1
                x_cb40((x_w1_508) + 1, (x_w0_507) + 1) = (x_inl1_sz_ey((x_w1_508) + 1, (x_w0_507) + 1) * &
                &x_inl14_gathered((x_w1_508) + 1, (x_w0_507) + 1))
            end do
        end do
        ! numpy: np.sum(__cb40, axis=0)
        do x_ax0_509 = 0, (np_particles) - 1
            x_cb41((x_ax0_509) + 1) = 0.0_c_double
            do x_rd0_510 = 0, ((o + 1)) - 1
                x_cb41((x_ax0_509) + 1) = (x_cb41((x_ax0_509) + 1) + x_cb40((x_ax0_509) + 1, (x_rd0_510) + 1))
            end do
        end do
        do x_w0_511 = 0, (np_particles) - 1
            x_hcall1((x_w0_511) + 1) = x_cb41((x_w0_511) + 1)
        end do
        do x_w0_512 = 0, (np_particles) - 1
            Eyp((x_w0_512) + 1) = Eyp((x_w0_512) + 1) + (x_hcall1((x_w0_512) + 1))
        end do
        if (.not. allocated(x_cb42)) then
            allocate(x_cb42((o + 1)))
        else if (size(x_cb42, 1) /= ((o + 1))) then
            deallocate(x_cb42)
            allocate(x_cb42((o + 1)))
        end if
        if (.not. allocated(x_cb42)) then
            allocate(x_cb42((o + 1)))
        else if (size(x_cb42, 1) /= ((o + 1))) then
            deallocate(x_cb42)
            allocate(x_cb42((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ex)
        do x_i_513 = 0, (x_inl1_n_sz_ex) - 1
            x_cb42((x_i_513) + 1) = x_i_513
        end do
        if (.not. allocated(x_inl15_taps)) then
            allocate(x_inl15_taps((o + 1)))
        else if (size(x_inl15_taps, 1) /= ((o + 1))) then
            deallocate(x_inl15_taps)
            allocate(x_inl15_taps((o + 1)))
        end if
        do x_w0_514 = 0, (x_inl1_n_sz_ex) - 1
            x_inl15_taps((x_w0_514) + 1) = x_cb42((x_w0_514) + 1)
        end do
        if (.not. allocated(x_inl15_rows)) then
            allocate(x_inl15_rows(np_particles, (o + 1)))
        else if (size(x_inl15_rows, 1) /= (np_particles) .or. size(x_inl15_rows, 2) /= ((o + 1))) then
            deallocate(x_inl15_rows)
            allocate(x_inl15_rows(np_particles, (o + 1)))
        end if
        do x_w0_515 = 0, (x_inl1_n_sz_ex) - 1
            do x_w1_516 = 0, (np_particles) - 1
                x_inl15_rows((x_w1_516) + 1, (x_w0_515) + 1) = ((x_inl1_lox + x_inl1_l_ex((x_w1_516) + 1)) + &
                &x_inl15_taps((x_w0_515) + 1))
            end do
        end do
        if (.not. allocated(x_inl15_gathered)) then
            allocate(x_inl15_gathered(np_particles, (o + 1)))
        else if (size(x_inl15_gathered, 1) /= (np_particles) .or. size(x_inl15_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl15_gathered)
            allocate(x_inl15_gathered(np_particles, (o + 1)))
        end if
        do x_w0_517 = 0, (x_inl1_n_sz_ex) - 1
            do x_w1_518 = 0, (np_particles) - 1
                x_inl15_gathered((x_w1_518) + 1, (x_w0_517) + 1) = ex_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl15_rows((x_w1_518) + 1, (x_w0_517) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb43)) then
            allocate(x_cb43(np_particles, (o + 1)))
        else if (size(x_cb43, 1) /= (np_particles) .or. size(x_cb43, 2) /= ((o + 1))) then
            deallocate(x_cb43)
            allocate(x_cb43(np_particles, (o + 1)))
        end if
        do x_w0_519 = 0, ((o + 1)) - 1
            do x_w1_520 = 0, (np_particles) - 1
                x_cb43((x_w1_520) + 1, (x_w0_519) + 1) = (x_inl1_sz_ex((x_w1_520) + 1, (x_w0_519) + 1) * &
                &x_inl15_gathered((x_w1_520) + 1, (x_w0_519) + 1))
            end do
        end do
        ! numpy: np.sum(__cb43, axis=0)
        do x_ax0_521 = 0, (np_particles) - 1
            x_cb44((x_ax0_521) + 1) = 0.0_c_double
            do x_rd0_522 = 0, ((o + 1)) - 1
                x_cb44((x_ax0_521) + 1) = (x_cb44((x_ax0_521) + 1) + x_cb43((x_ax0_521) + 1, (x_rd0_522) + 1))
            end do
        end do
        do x_w0_523 = 0, (np_particles) - 1
            x_hcall2((x_w0_523) + 1) = x_cb44((x_w0_523) + 1)
        end do
        do x_w0_524 = 0, (np_particles) - 1
            Exp((x_w0_524) + 1) = Exp((x_w0_524) + 1) + (x_hcall2((x_w0_524) + 1))
        end do
        if (.not. allocated(x_cb45)) then
            allocate(x_cb45((o + 1)))
        else if (size(x_cb45, 1) /= ((o + 1))) then
            deallocate(x_cb45)
            allocate(x_cb45((o + 1)))
        end if
        if (.not. allocated(x_cb45)) then
            allocate(x_cb45((o + 1)))
        else if (size(x_cb45, 1) /= ((o + 1))) then
            deallocate(x_cb45)
            allocate(x_cb45((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bz)
        do x_i_525 = 0, (x_inl1_n_sz_bz) - 1
            x_cb45((x_i_525) + 1) = x_i_525
        end do
        if (.not. allocated(x_inl16_taps)) then
            allocate(x_inl16_taps((o + 1)))
        else if (size(x_inl16_taps, 1) /= ((o + 1))) then
            deallocate(x_inl16_taps)
            allocate(x_inl16_taps((o + 1)))
        end if
        do x_w0_526 = 0, (x_inl1_n_sz_bz) - 1
            x_inl16_taps((x_w0_526) + 1) = x_cb45((x_w0_526) + 1)
        end do
        if (.not. allocated(x_inl16_rows)) then
            allocate(x_inl16_rows(np_particles, (o + 1)))
        else if (size(x_inl16_rows, 1) /= (np_particles) .or. size(x_inl16_rows, 2) /= ((o + 1))) then
            deallocate(x_inl16_rows)
            allocate(x_inl16_rows(np_particles, (o + 1)))
        end if
        do x_w0_527 = 0, (x_inl1_n_sz_bz) - 1
            do x_w1_528 = 0, (np_particles) - 1
                x_inl16_rows((x_w1_528) + 1, (x_w0_527) + 1) = ((x_inl1_lox + x_inl1_l_bz((x_w1_528) + 1)) + &
                &x_inl16_taps((x_w0_527) + 1))
            end do
        end do
        if (.not. allocated(x_inl16_gathered)) then
            allocate(x_inl16_gathered(np_particles, (o + 1)))
        else if (size(x_inl16_gathered, 1) /= (np_particles) .or. size(x_inl16_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl16_gathered)
            allocate(x_inl16_gathered(np_particles, (o + 1)))
        end if
        do x_w0_529 = 0, (x_inl1_n_sz_bz) - 1
            do x_w1_530 = 0, (np_particles) - 1
                x_inl16_gathered((x_w1_530) + 1, (x_w0_529) + 1) = bz_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl16_rows((x_w1_530) + 1, (x_w0_529) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb46)) then
            allocate(x_cb46(np_particles, (o + 1)))
        else if (size(x_cb46, 1) /= (np_particles) .or. size(x_cb46, 2) /= ((o + 1))) then
            deallocate(x_cb46)
            allocate(x_cb46(np_particles, (o + 1)))
        end if
        do x_w0_531 = 0, ((o + 1)) - 1
            do x_w1_532 = 0, (np_particles) - 1
                x_cb46((x_w1_532) + 1, (x_w0_531) + 1) = (x_inl1_sz_bz((x_w1_532) + 1, (x_w0_531) + 1) * &
                &x_inl16_gathered((x_w1_532) + 1, (x_w0_531) + 1))
            end do
        end do
        ! numpy: np.sum(__cb46, axis=0)
        do x_ax0_533 = 0, (np_particles) - 1
            x_cb47((x_ax0_533) + 1) = 0.0_c_double
            do x_rd0_534 = 0, ((o + 1)) - 1
                x_cb47((x_ax0_533) + 1) = (x_cb47((x_ax0_533) + 1) + x_cb46((x_ax0_533) + 1, (x_rd0_534) + 1))
            end do
        end do
        do x_w0_535 = 0, (np_particles) - 1
            x_hcall3((x_w0_535) + 1) = x_cb47((x_w0_535) + 1)
        end do
        do x_w0_536 = 0, (np_particles) - 1
            Bzp((x_w0_536) + 1) = Bzp((x_w0_536) + 1) + (x_hcall3((x_w0_536) + 1))
        end do
        if (.not. allocated(x_cb48)) then
            allocate(x_cb48(((o - gal) + 1)))
        else if (size(x_cb48, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb48)
            allocate(x_cb48(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb48)) then
            allocate(x_cb48(((o - gal) + 1)))
        else if (size(x_cb48, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb48)
            allocate(x_cb48(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ez)
        do x_i_537 = 0, (x_inl1_n_sz_ez) - 1
            x_cb48((x_i_537) + 1) = x_i_537
        end do
        if (.not. allocated(x_inl17_taps)) then
            allocate(x_inl17_taps(((o - gal) + 1)))
        else if (size(x_inl17_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl17_taps)
            allocate(x_inl17_taps(((o - gal) + 1)))
        end if
        do x_w0_538 = 0, (x_inl1_n_sz_ez) - 1
            x_inl17_taps((x_w0_538) + 1) = x_cb48((x_w0_538) + 1)
        end do
        if (.not. allocated(x_inl17_rows)) then
            allocate(x_inl17_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl17_rows, 1) /= (np_particles) .or. size(x_inl17_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl17_rows)
            allocate(x_inl17_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_539 = 0, (x_inl1_n_sz_ez) - 1
            do x_w1_540 = 0, (np_particles) - 1
                x_inl17_rows((x_w1_540) + 1, (x_w0_539) + 1) = ((x_inl1_lox + x_inl1_l_ez((x_w1_540) + 1)) + &
                &x_inl17_taps((x_w0_539) + 1))
            end do
        end do
        if (.not. allocated(x_inl17_gathered)) then
            allocate(x_inl17_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl17_gathered, 1) /= (np_particles) .or. size(x_inl17_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl17_gathered)
            allocate(x_inl17_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_541 = 0, (x_inl1_n_sz_ez) - 1
            do x_w1_542 = 0, (np_particles) - 1
                x_inl17_gathered((x_w1_542) + 1, (x_w0_541) + 1) = ez_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl17_rows((x_w1_542) + 1, (x_w0_541) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb49)) then
            allocate(x_cb49(np_particles, ((o - gal) + 1)))
        else if (size(x_cb49, 1) /= (np_particles) .or. size(x_cb49, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb49)
            allocate(x_cb49(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_543 = 0, (((o - gal) + 1)) - 1
            do x_w1_544 = 0, (np_particles) - 1
                x_cb49((x_w1_544) + 1, (x_w0_543) + 1) = (x_inl1_sz_ez((x_w1_544) + 1, (x_w0_543) + 1) * &
                &x_inl17_gathered((x_w1_544) + 1, (x_w0_543) + 1))
            end do
        end do
        ! numpy: np.sum(__cb49, axis=0)
        do x_ax0_545 = 0, (np_particles) - 1
            x_cb50((x_ax0_545) + 1) = 0.0_c_double
            do x_rd0_546 = 0, (((o - gal) + 1)) - 1
                x_cb50((x_ax0_545) + 1) = (x_cb50((x_ax0_545) + 1) + x_cb49((x_ax0_545) + 1, (x_rd0_546) + 1))
            end do
        end do
        do x_w0_547 = 0, (np_particles) - 1
            x_hcall4((x_w0_547) + 1) = x_cb50((x_w0_547) + 1)
        end do
        do x_w0_548 = 0, (np_particles) - 1
            Ezp((x_w0_548) + 1) = Ezp((x_w0_548) + 1) + (x_hcall4((x_w0_548) + 1))
        end do
        if (.not. allocated(x_cb51)) then
            allocate(x_cb51(((o - gal) + 1)))
        else if (size(x_cb51, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb51)
            allocate(x_cb51(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb51)) then
            allocate(x_cb51(((o - gal) + 1)))
        else if (size(x_cb51, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb51)
            allocate(x_cb51(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bx)
        do x_i_549 = 0, (x_inl1_n_sz_bx) - 1
            x_cb51((x_i_549) + 1) = x_i_549
        end do
        if (.not. allocated(x_inl18_taps)) then
            allocate(x_inl18_taps(((o - gal) + 1)))
        else if (size(x_inl18_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl18_taps)
            allocate(x_inl18_taps(((o - gal) + 1)))
        end if
        do x_w0_550 = 0, (x_inl1_n_sz_bx) - 1
            x_inl18_taps((x_w0_550) + 1) = x_cb51((x_w0_550) + 1)
        end do
        if (.not. allocated(x_inl18_rows)) then
            allocate(x_inl18_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl18_rows, 1) /= (np_particles) .or. size(x_inl18_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl18_rows)
            allocate(x_inl18_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_551 = 0, (x_inl1_n_sz_bx) - 1
            do x_w1_552 = 0, (np_particles) - 1
                x_inl18_rows((x_w1_552) + 1, (x_w0_551) + 1) = ((x_inl1_lox + x_inl1_l_bx((x_w1_552) + 1)) + &
                &x_inl18_taps((x_w0_551) + 1))
            end do
        end do
        if (.not. allocated(x_inl18_gathered)) then
            allocate(x_inl18_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl18_gathered, 1) /= (np_particles) .or. size(x_inl18_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl18_gathered)
            allocate(x_inl18_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_553 = 0, (x_inl1_n_sz_bx) - 1
            do x_w1_554 = 0, (np_particles) - 1
                x_inl18_gathered((x_w1_554) + 1, (x_w0_553) + 1) = bx_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl18_rows((x_w1_554) + 1, (x_w0_553) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb52)) then
            allocate(x_cb52(np_particles, ((o - gal) + 1)))
        else if (size(x_cb52, 1) /= (np_particles) .or. size(x_cb52, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb52)
            allocate(x_cb52(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_555 = 0, (((o - gal) + 1)) - 1
            do x_w1_556 = 0, (np_particles) - 1
                x_cb52((x_w1_556) + 1, (x_w0_555) + 1) = (x_inl1_sz_bx((x_w1_556) + 1, (x_w0_555) + 1) * &
                &x_inl18_gathered((x_w1_556) + 1, (x_w0_555) + 1))
            end do
        end do
        ! numpy: np.sum(__cb52, axis=0)
        do x_ax0_557 = 0, (np_particles) - 1
            x_cb53((x_ax0_557) + 1) = 0.0_c_double
            do x_rd0_558 = 0, (((o - gal) + 1)) - 1
                x_cb53((x_ax0_557) + 1) = (x_cb53((x_ax0_557) + 1) + x_cb52((x_ax0_557) + 1, (x_rd0_558) + 1))
            end do
        end do
        do x_w0_559 = 0, (np_particles) - 1
            x_hcall5((x_w0_559) + 1) = x_cb53((x_w0_559) + 1)
        end do
        do x_w0_560 = 0, (np_particles) - 1
            Bxp((x_w0_560) + 1) = Bxp((x_w0_560) + 1) + (x_hcall5((x_w0_560) + 1))
        end do
        if (.not. allocated(x_cb54)) then
            allocate(x_cb54(((o - gal) + 1)))
        else if (size(x_cb54, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb54)
            allocate(x_cb54(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb54)) then
            allocate(x_cb54(((o - gal) + 1)))
        else if (size(x_cb54, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb54)
            allocate(x_cb54(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_by)
        do x_i_561 = 0, (x_inl1_n_sz_by) - 1
            x_cb54((x_i_561) + 1) = x_i_561
        end do
        if (.not. allocated(x_inl19_taps)) then
            allocate(x_inl19_taps(((o - gal) + 1)))
        else if (size(x_inl19_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl19_taps)
            allocate(x_inl19_taps(((o - gal) + 1)))
        end if
        do x_w0_562 = 0, (x_inl1_n_sz_by) - 1
            x_inl19_taps((x_w0_562) + 1) = x_cb54((x_w0_562) + 1)
        end do
        if (.not. allocated(x_inl19_rows)) then
            allocate(x_inl19_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl19_rows, 1) /= (np_particles) .or. size(x_inl19_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl19_rows)
            allocate(x_inl19_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_563 = 0, (x_inl1_n_sz_by) - 1
            do x_w1_564 = 0, (np_particles) - 1
                x_inl19_rows((x_w1_564) + 1, (x_w0_563) + 1) = ((x_inl1_lox + x_inl1_l_by((x_w1_564) + 1)) + &
                &x_inl19_taps((x_w0_563) + 1))
            end do
        end do
        if (.not. allocated(x_inl19_gathered)) then
            allocate(x_inl19_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl19_gathered, 1) /= (np_particles) .or. size(x_inl19_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl19_gathered)
            allocate(x_inl19_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_565 = 0, (x_inl1_n_sz_by) - 1
            do x_w1_566 = 0, (np_particles) - 1
                x_inl19_gathered((x_w1_566) + 1, (x_w0_565) + 1) = by_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl19_rows((x_w1_566) + 1, (x_w0_565) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb55)) then
            allocate(x_cb55(np_particles, ((o - gal) + 1)))
        else if (size(x_cb55, 1) /= (np_particles) .or. size(x_cb55, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb55)
            allocate(x_cb55(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_567 = 0, (((o - gal) + 1)) - 1
            do x_w1_568 = 0, (np_particles) - 1
                x_cb55((x_w1_568) + 1, (x_w0_567) + 1) = (x_inl1_sz_by((x_w1_568) + 1, (x_w0_567) + 1) * &
                &x_inl19_gathered((x_w1_568) + 1, (x_w0_567) + 1))
            end do
        end do
        ! numpy: np.sum(__cb55, axis=0)
        do x_ax0_569 = 0, (np_particles) - 1
            x_cb56((x_ax0_569) + 1) = 0.0_c_double
            do x_rd0_570 = 0, (((o - gal) + 1)) - 1
                x_cb56((x_ax0_569) + 1) = (x_cb56((x_ax0_569) + 1) + x_cb55((x_ax0_569) + 1, (x_rd0_570) + 1))
            end do
        end do
        do x_w0_571 = 0, (np_particles) - 1
            x_hcall6((x_w0_571) + 1) = x_cb56((x_w0_571) + 1)
        end do
        do x_w0_572 = 0, (np_particles) - 1
            Byp((x_w0_572) + 1) = Byp((x_w0_572) + 1) + (x_hcall6((x_w0_572) + 1))
        end do
    else if ((g == 1)) then
        if (.not. allocated(x_cb57)) then
            allocate(x_cb57((o + 1)))
        else if (size(x_cb57, 1) /= ((o + 1))) then
            deallocate(x_cb57)
            allocate(x_cb57((o + 1)))
        end if
        if (.not. allocated(x_cb57)) then
            allocate(x_cb57((o + 1)))
        else if (size(x_cb57, 1) /= ((o + 1))) then
            deallocate(x_cb57)
            allocate(x_cb57((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ey)
        do x_i_573 = 0, (x_inl1_n_sx_ey) - 1
            x_cb57((x_i_573) + 1) = x_i_573
        end do
        if (.not. allocated(x_inl20_ta)) then
            allocate(x_inl20_ta((o + 1)))
        else if (size(x_inl20_ta, 1) /= ((o + 1))) then
            deallocate(x_inl20_ta)
            allocate(x_inl20_ta((o + 1)))
        end if
        do x_w0_574 = 0, (x_inl1_n_sx_ey) - 1
            x_inl20_ta((x_w0_574) + 1) = x_cb57((x_w0_574) + 1)
        end do
        if (.not. allocated(x_cb58)) then
            allocate(x_cb58((o + 1)))
        else if (size(x_cb58, 1) /= ((o + 1))) then
            deallocate(x_cb58)
            allocate(x_cb58((o + 1)))
        end if
        if (.not. allocated(x_cb58)) then
            allocate(x_cb58((o + 1)))
        else if (size(x_cb58, 1) /= ((o + 1))) then
            deallocate(x_cb58)
            allocate(x_cb58((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ey)
        do x_i_575 = 0, (x_inl1_n_sz_ey) - 1
            x_cb58((x_i_575) + 1) = x_i_575
        end do
        if (.not. allocated(x_inl20_tb)) then
            allocate(x_inl20_tb((o + 1)))
        else if (size(x_inl20_tb, 1) /= ((o + 1))) then
            deallocate(x_inl20_tb)
            allocate(x_inl20_tb((o + 1)))
        end if
        do x_w0_576 = 0, (x_inl1_n_sz_ey) - 1
            x_inl20_tb((x_w0_576) + 1) = x_cb58((x_w0_576) + 1)
        end do
        if (.not. allocated(x_inl20_ia)) then
            allocate(x_inl20_ia(np_particles, 1, (o + 1)))
        else if (size(x_inl20_ia, 1) /= (np_particles) .or. size(x_inl20_ia, 2) /= (1) .or. size(x_inl20_ia, 3) /= ((o &
        &+ 1))) then
            deallocate(x_inl20_ia)
            allocate(x_inl20_ia(np_particles, 1, (o + 1)))
        end if
        do si0_l577 = 0, ((o + 1)) - 1
            do si1_l578 = 0, (1) - 1
                do si2_l579 = 0, (np_particles) - 1
                    x_inl20_ia((si2_l579) + 1, (si1_l578) + 1, (si0_l577) + 1) = ((x_inl1_lox + x_inl1_j_ey((si2_l579) &
                    &+ 1)) + x_inl20_ta((si0_l577) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl20_ib)) then
            allocate(x_inl20_ib(np_particles, (o + 1), 1))
        else if (size(x_inl20_ib, 1) /= (np_particles) .or. size(x_inl20_ib, 2) /= ((o + 1)) .or. size(x_inl20_ib, 3) &
        &/= (1)) then
            deallocate(x_inl20_ib)
            allocate(x_inl20_ib(np_particles, (o + 1), 1))
        end if
        do si0_l580 = 0, (1) - 1
            do si1_l581 = 0, ((o + 1)) - 1
                do si2_l582 = 0, (np_particles) - 1
                    x_inl20_ib((si2_l582) + 1, (si1_l581) + 1, (si0_l580) + 1) = ((x_inl1_loy + x_inl1_l_ey((si2_l582) &
                    &+ 1)) + x_inl20_tb((si1_l581) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl20_ia_b)) then
            allocate(x_inl20_ia_b(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl20_ia_b, 1) /= (np_particles) .or. size(x_inl20_ia_b, 2) /= ((o + 1)) .or. &
        &size(x_inl20_ia_b, 3) /= ((o + 1))) then
            deallocate(x_inl20_ia_b)
            allocate(x_inl20_ia_b(np_particles, (o + 1), (o + 1)))
        end if
        do si0_l583 = 0, ((o + 1)) - 1
            do si1_l584 = 0, ((o + 1)) - 1
                do si2_l585 = 0, (np_particles) - 1
                    x_inl20_ia_b((si2_l585) + 1, (si1_l584) + 1, (si0_l583) + 1) = x_inl20_ia((si2_l585) + 1, (0) + 1, &
                    &(si0_l583) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl20_ib_b)) then
            allocate(x_inl20_ib_b(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl20_ib_b, 1) /= (np_particles) .or. size(x_inl20_ib_b, 2) /= ((o + 1)) .or. &
        &size(x_inl20_ib_b, 3) /= ((o + 1))) then
            deallocate(x_inl20_ib_b)
            allocate(x_inl20_ib_b(np_particles, (o + 1), (o + 1)))
        end if
        do si0_l586 = 0, ((o + 1)) - 1
            do si1_l587 = 0, ((o + 1)) - 1
                do si2_l588 = 0, (np_particles) - 1
                    x_inl20_ib_b((si2_l588) + 1, (si1_l587) + 1, (si0_l586) + 1) = x_inl20_ib((si2_l588) + 1, &
                    &(si1_l587) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl20_gathered)) then
            allocate(x_inl20_gathered(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl20_gathered, 1) /= (np_particles) .or. size(x_inl20_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl20_gathered, 3) /= ((o + 1))) then
            deallocate(x_inl20_gathered)
            allocate(x_inl20_gathered(np_particles, (o + 1), (o + 1)))
        end if
        do x_w0_589 = 0, ((o + 1)) - 1
            do x_w1_590 = 0, ((o + 1)) - 1
                do x_w2_591 = 0, (np_particles) - 1
                    x_inl20_gathered((x_w2_591) + 1, (x_w1_590) + 1, (x_w0_589) + 1) = ey_arr((0) + 1, (0) + 1, &
                    &(x_inl20_ib_b((x_w2_591) + 1, (x_w1_590) + 1, (x_w0_589) + 1)) + 1, (x_inl20_ia_b((x_w2_591) + 1, &
                    &(x_w1_590) + 1, (x_w0_589) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl20_weight)) then
            allocate(x_inl20_weight(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl20_weight, 1) /= (np_particles) .or. size(x_inl20_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl20_weight, 3) /= ((o + 1))) then
            deallocate(x_inl20_weight)
            allocate(x_inl20_weight(np_particles, (o + 1), (o + 1)))
        end if
        do x_w0_592 = 0, ((o + 1)) - 1
            do x_w1_593 = 0, ((o + 1)) - 1
                do x_w2_594 = 0, (np_particles) - 1
                    x_inl20_weight((x_w2_594) + 1, (x_w1_593) + 1, (x_w0_592) + 1) = (x_inl1_sx_ey((x_w2_594) + 1, &
                    &(x_w0_592) + 1) * x_inl1_sz_ey((x_w2_594) + 1, (x_w1_593) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb59)) then
            allocate(x_cb59(np_particles, (o + 1), (o + 1)))
        else if (size(x_cb59, 1) /= (np_particles) .or. size(x_cb59, 2) /= ((o + 1)) .or. size(x_cb59, 3) /= ((o + &
        &1))) then
            deallocate(x_cb59)
            allocate(x_cb59(np_particles, (o + 1), (o + 1)))
        end if
        do x_w0_595 = 0, ((o + 1)) - 1
            do x_w1_596 = 0, ((o + 1)) - 1
                do x_w2_597 = 0, (np_particles) - 1
                    x_cb59((x_w2_597) + 1, (x_w1_596) + 1, (x_w0_595) + 1) = (x_inl20_weight((x_w2_597) + 1, &
                    &(x_w1_596) + 1, (x_w0_595) + 1) * x_inl20_gathered((x_w2_597) + 1, (x_w1_596) + 1, (x_w0_595) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb59, axis=(0, 1))
        do x_ax0_598 = 0, (np_particles) - 1
            x_cb60((x_ax0_598) + 1) = 0.0_c_double
            do x_rd0_599 = 0, ((o + 1)) - 1
                do x_rd1_600 = 0, ((o + 1)) - 1
                    x_cb60((x_ax0_598) + 1) = (x_cb60((x_ax0_598) + 1) + x_cb59((x_ax0_598) + 1, (x_rd1_600) + 1, &
                    &(x_rd0_599) + 1))
                end do
            end do
        end do
        do x_w0_601 = 0, (np_particles) - 1
            x_hcall7((x_w0_601) + 1) = x_cb60((x_w0_601) + 1)
        end do
        do x_w0_602 = 0, (np_particles) - 1
            Eyp((x_w0_602) + 1) = Eyp((x_w0_602) + 1) + (x_hcall7((x_w0_602) + 1))
        end do
        if (.not. allocated(x_cb61)) then
            allocate(x_cb61(((o - gal) + 1)))
        else if (size(x_cb61, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb61)
            allocate(x_cb61(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb61)) then
            allocate(x_cb61(((o - gal) + 1)))
        else if (size(x_cb61, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb61)
            allocate(x_cb61(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ex)
        do x_i_603 = 0, (x_inl1_n_sx_ex) - 1
            x_cb61((x_i_603) + 1) = x_i_603
        end do
        if (.not. allocated(x_inl21_ta)) then
            allocate(x_inl21_ta(((o - gal) + 1)))
        else if (size(x_inl21_ta, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl21_ta)
            allocate(x_inl21_ta(((o - gal) + 1)))
        end if
        do x_w0_604 = 0, (x_inl1_n_sx_ex) - 1
            x_inl21_ta((x_w0_604) + 1) = x_cb61((x_w0_604) + 1)
        end do
        if (.not. allocated(x_cb62)) then
            allocate(x_cb62((o + 1)))
        else if (size(x_cb62, 1) /= ((o + 1))) then
            deallocate(x_cb62)
            allocate(x_cb62((o + 1)))
        end if
        if (.not. allocated(x_cb62)) then
            allocate(x_cb62((o + 1)))
        else if (size(x_cb62, 1) /= ((o + 1))) then
            deallocate(x_cb62)
            allocate(x_cb62((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ex)
        do x_i_605 = 0, (x_inl1_n_sz_ex) - 1
            x_cb62((x_i_605) + 1) = x_i_605
        end do
        if (.not. allocated(x_inl21_tb)) then
            allocate(x_inl21_tb((o + 1)))
        else if (size(x_inl21_tb, 1) /= ((o + 1))) then
            deallocate(x_inl21_tb)
            allocate(x_inl21_tb((o + 1)))
        end if
        do x_w0_606 = 0, (x_inl1_n_sz_ex) - 1
            x_inl21_tb((x_w0_606) + 1) = x_cb62((x_w0_606) + 1)
        end do
        if (.not. allocated(x_inl21_ia)) then
            allocate(x_inl21_ia(np_particles, 1, ((o - gal) + 1)))
        else if (size(x_inl21_ia, 1) /= (np_particles) .or. size(x_inl21_ia, 2) /= (1) .or. size(x_inl21_ia, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_inl21_ia)
            allocate(x_inl21_ia(np_particles, 1, ((o - gal) + 1)))
        end if
        do si0_l607 = 0, (((o - gal) + 1)) - 1
            do si1_l608 = 0, (1) - 1
                do si2_l609 = 0, (np_particles) - 1
                    x_inl21_ia((si2_l609) + 1, (si1_l608) + 1, (si0_l607) + 1) = ((x_inl1_lox + x_inl1_j_ex((si2_l609) &
                    &+ 1)) + x_inl21_ta((si0_l607) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl21_ib)) then
            allocate(x_inl21_ib(np_particles, (o + 1), 1))
        else if (size(x_inl21_ib, 1) /= (np_particles) .or. size(x_inl21_ib, 2) /= ((o + 1)) .or. size(x_inl21_ib, 3) &
        &/= (1)) then
            deallocate(x_inl21_ib)
            allocate(x_inl21_ib(np_particles, (o + 1), 1))
        end if
        do si0_l610 = 0, (1) - 1
            do si1_l611 = 0, ((o + 1)) - 1
                do si2_l612 = 0, (np_particles) - 1
                    x_inl21_ib((si2_l612) + 1, (si1_l611) + 1, (si0_l610) + 1) = ((x_inl1_loy + x_inl1_l_ex((si2_l612) &
                    &+ 1)) + x_inl21_tb((si1_l611) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl21_ia_b)) then
            allocate(x_inl21_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl21_ia_b, 1) /= (np_particles) .or. size(x_inl21_ia_b, 2) /= ((o + 1)) .or. &
        &size(x_inl21_ia_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl21_ia_b)
            allocate(x_inl21_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l613 = 0, (((o - gal) + 1)) - 1
            do si1_l614 = 0, ((o + 1)) - 1
                do si2_l615 = 0, (np_particles) - 1
                    x_inl21_ia_b((si2_l615) + 1, (si1_l614) + 1, (si0_l613) + 1) = x_inl21_ia((si2_l615) + 1, (0) + 1, &
                    &(si0_l613) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl21_ib_b)) then
            allocate(x_inl21_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl21_ib_b, 1) /= (np_particles) .or. size(x_inl21_ib_b, 2) /= ((o + 1)) .or. &
        &size(x_inl21_ib_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl21_ib_b)
            allocate(x_inl21_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l616 = 0, (((o - gal) + 1)) - 1
            do si1_l617 = 0, ((o + 1)) - 1
                do si2_l618 = 0, (np_particles) - 1
                    x_inl21_ib_b((si2_l618) + 1, (si1_l617) + 1, (si0_l616) + 1) = x_inl21_ib((si2_l618) + 1, &
                    &(si1_l617) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl21_gathered)) then
            allocate(x_inl21_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl21_gathered, 1) /= (np_particles) .or. size(x_inl21_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl21_gathered, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl21_gathered)
            allocate(x_inl21_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_619 = 0, (((o - gal) + 1)) - 1
            do x_w1_620 = 0, ((o + 1)) - 1
                do x_w2_621 = 0, (np_particles) - 1
                    x_inl21_gathered((x_w2_621) + 1, (x_w1_620) + 1, (x_w0_619) + 1) = ex_arr((0) + 1, (0) + 1, &
                    &(x_inl21_ib_b((x_w2_621) + 1, (x_w1_620) + 1, (x_w0_619) + 1)) + 1, (x_inl21_ia_b((x_w2_621) + 1, &
                    &(x_w1_620) + 1, (x_w0_619) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl21_weight)) then
            allocate(x_inl21_weight(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl21_weight, 1) /= (np_particles) .or. size(x_inl21_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl21_weight, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl21_weight)
            allocate(x_inl21_weight(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_622 = 0, (((o - gal) + 1)) - 1
            do x_w1_623 = 0, ((o + 1)) - 1
                do x_w2_624 = 0, (np_particles) - 1
                    x_inl21_weight((x_w2_624) + 1, (x_w1_623) + 1, (x_w0_622) + 1) = (x_inl1_sx_ex((x_w2_624) + 1, &
                    &(x_w0_622) + 1) * x_inl1_sz_ex((x_w2_624) + 1, (x_w1_623) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb63)) then
            allocate(x_cb63(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_cb63, 1) /= (np_particles) .or. size(x_cb63, 2) /= ((o + 1)) .or. size(x_cb63, 3) /= (((o - &
        &gal) + 1))) then
            deallocate(x_cb63)
            allocate(x_cb63(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_625 = 0, (((o - gal) + 1)) - 1
            do x_w1_626 = 0, ((o + 1)) - 1
                do x_w2_627 = 0, (np_particles) - 1
                    x_cb63((x_w2_627) + 1, (x_w1_626) + 1, (x_w0_625) + 1) = (x_inl21_weight((x_w2_627) + 1, &
                    &(x_w1_626) + 1, (x_w0_625) + 1) * x_inl21_gathered((x_w2_627) + 1, (x_w1_626) + 1, (x_w0_625) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb63, axis=(0, 1))
        do x_ax0_628 = 0, (np_particles) - 1
            x_cb64((x_ax0_628) + 1) = 0.0_c_double
            do x_rd0_629 = 0, (((o - gal) + 1)) - 1
                do x_rd1_630 = 0, ((o + 1)) - 1
                    x_cb64((x_ax0_628) + 1) = (x_cb64((x_ax0_628) + 1) + x_cb63((x_ax0_628) + 1, (x_rd1_630) + 1, &
                    &(x_rd0_629) + 1))
                end do
            end do
        end do
        do x_w0_631 = 0, (np_particles) - 1
            x_hcall8((x_w0_631) + 1) = x_cb64((x_w0_631) + 1)
        end do
        do x_w0_632 = 0, (np_particles) - 1
            Exp((x_w0_632) + 1) = Exp((x_w0_632) + 1) + (x_hcall8((x_w0_632) + 1))
        end do
        if (.not. allocated(x_cb65)) then
            allocate(x_cb65(((o - gal) + 1)))
        else if (size(x_cb65, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb65)
            allocate(x_cb65(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb65)) then
            allocate(x_cb65(((o - gal) + 1)))
        else if (size(x_cb65, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb65)
            allocate(x_cb65(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bz)
        do x_i_633 = 0, (x_inl1_n_sx_bz) - 1
            x_cb65((x_i_633) + 1) = x_i_633
        end do
        if (.not. allocated(x_inl22_ta)) then
            allocate(x_inl22_ta(((o - gal) + 1)))
        else if (size(x_inl22_ta, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl22_ta)
            allocate(x_inl22_ta(((o - gal) + 1)))
        end if
        do x_w0_634 = 0, (x_inl1_n_sx_bz) - 1
            x_inl22_ta((x_w0_634) + 1) = x_cb65((x_w0_634) + 1)
        end do
        if (.not. allocated(x_cb66)) then
            allocate(x_cb66((o + 1)))
        else if (size(x_cb66, 1) /= ((o + 1))) then
            deallocate(x_cb66)
            allocate(x_cb66((o + 1)))
        end if
        if (.not. allocated(x_cb66)) then
            allocate(x_cb66((o + 1)))
        else if (size(x_cb66, 1) /= ((o + 1))) then
            deallocate(x_cb66)
            allocate(x_cb66((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bz)
        do x_i_635 = 0, (x_inl1_n_sz_bz) - 1
            x_cb66((x_i_635) + 1) = x_i_635
        end do
        if (.not. allocated(x_inl22_tb)) then
            allocate(x_inl22_tb((o + 1)))
        else if (size(x_inl22_tb, 1) /= ((o + 1))) then
            deallocate(x_inl22_tb)
            allocate(x_inl22_tb((o + 1)))
        end if
        do x_w0_636 = 0, (x_inl1_n_sz_bz) - 1
            x_inl22_tb((x_w0_636) + 1) = x_cb66((x_w0_636) + 1)
        end do
        if (.not. allocated(x_inl22_ia)) then
            allocate(x_inl22_ia(np_particles, 1, ((o - gal) + 1)))
        else if (size(x_inl22_ia, 1) /= (np_particles) .or. size(x_inl22_ia, 2) /= (1) .or. size(x_inl22_ia, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_inl22_ia)
            allocate(x_inl22_ia(np_particles, 1, ((o - gal) + 1)))
        end if
        do si0_l637 = 0, (((o - gal) + 1)) - 1
            do si1_l638 = 0, (1) - 1
                do si2_l639 = 0, (np_particles) - 1
                    x_inl22_ia((si2_l639) + 1, (si1_l638) + 1, (si0_l637) + 1) = ((x_inl1_lox + x_inl1_j_bz((si2_l639) &
                    &+ 1)) + x_inl22_ta((si0_l637) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl22_ib)) then
            allocate(x_inl22_ib(np_particles, (o + 1), 1))
        else if (size(x_inl22_ib, 1) /= (np_particles) .or. size(x_inl22_ib, 2) /= ((o + 1)) .or. size(x_inl22_ib, 3) &
        &/= (1)) then
            deallocate(x_inl22_ib)
            allocate(x_inl22_ib(np_particles, (o + 1), 1))
        end if
        do si0_l640 = 0, (1) - 1
            do si1_l641 = 0, ((o + 1)) - 1
                do si2_l642 = 0, (np_particles) - 1
                    x_inl22_ib((si2_l642) + 1, (si1_l641) + 1, (si0_l640) + 1) = ((x_inl1_loy + x_inl1_l_bz((si2_l642) &
                    &+ 1)) + x_inl22_tb((si1_l641) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl22_ia_b)) then
            allocate(x_inl22_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl22_ia_b, 1) /= (np_particles) .or. size(x_inl22_ia_b, 2) /= ((o + 1)) .or. &
        &size(x_inl22_ia_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl22_ia_b)
            allocate(x_inl22_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l643 = 0, (((o - gal) + 1)) - 1
            do si1_l644 = 0, ((o + 1)) - 1
                do si2_l645 = 0, (np_particles) - 1
                    x_inl22_ia_b((si2_l645) + 1, (si1_l644) + 1, (si0_l643) + 1) = x_inl22_ia((si2_l645) + 1, (0) + 1, &
                    &(si0_l643) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl22_ib_b)) then
            allocate(x_inl22_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl22_ib_b, 1) /= (np_particles) .or. size(x_inl22_ib_b, 2) /= ((o + 1)) .or. &
        &size(x_inl22_ib_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl22_ib_b)
            allocate(x_inl22_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l646 = 0, (((o - gal) + 1)) - 1
            do si1_l647 = 0, ((o + 1)) - 1
                do si2_l648 = 0, (np_particles) - 1
                    x_inl22_ib_b((si2_l648) + 1, (si1_l647) + 1, (si0_l646) + 1) = x_inl22_ib((si2_l648) + 1, &
                    &(si1_l647) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl22_gathered)) then
            allocate(x_inl22_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl22_gathered, 1) /= (np_particles) .or. size(x_inl22_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl22_gathered, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl22_gathered)
            allocate(x_inl22_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_649 = 0, (((o - gal) + 1)) - 1
            do x_w1_650 = 0, ((o + 1)) - 1
                do x_w2_651 = 0, (np_particles) - 1
                    x_inl22_gathered((x_w2_651) + 1, (x_w1_650) + 1, (x_w0_649) + 1) = bz_arr((0) + 1, (0) + 1, &
                    &(x_inl22_ib_b((x_w2_651) + 1, (x_w1_650) + 1, (x_w0_649) + 1)) + 1, (x_inl22_ia_b((x_w2_651) + 1, &
                    &(x_w1_650) + 1, (x_w0_649) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl22_weight)) then
            allocate(x_inl22_weight(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl22_weight, 1) /= (np_particles) .or. size(x_inl22_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl22_weight, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl22_weight)
            allocate(x_inl22_weight(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_652 = 0, (((o - gal) + 1)) - 1
            do x_w1_653 = 0, ((o + 1)) - 1
                do x_w2_654 = 0, (np_particles) - 1
                    x_inl22_weight((x_w2_654) + 1, (x_w1_653) + 1, (x_w0_652) + 1) = (x_inl1_sx_bz((x_w2_654) + 1, &
                    &(x_w0_652) + 1) * x_inl1_sz_bz((x_w2_654) + 1, (x_w1_653) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb67)) then
            allocate(x_cb67(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_cb67, 1) /= (np_particles) .or. size(x_cb67, 2) /= ((o + 1)) .or. size(x_cb67, 3) /= (((o - &
        &gal) + 1))) then
            deallocate(x_cb67)
            allocate(x_cb67(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_655 = 0, (((o - gal) + 1)) - 1
            do x_w1_656 = 0, ((o + 1)) - 1
                do x_w2_657 = 0, (np_particles) - 1
                    x_cb67((x_w2_657) + 1, (x_w1_656) + 1, (x_w0_655) + 1) = (x_inl22_weight((x_w2_657) + 1, &
                    &(x_w1_656) + 1, (x_w0_655) + 1) * x_inl22_gathered((x_w2_657) + 1, (x_w1_656) + 1, (x_w0_655) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb67, axis=(0, 1))
        do x_ax0_658 = 0, (np_particles) - 1
            x_cb68((x_ax0_658) + 1) = 0.0_c_double
            do x_rd0_659 = 0, (((o - gal) + 1)) - 1
                do x_rd1_660 = 0, ((o + 1)) - 1
                    x_cb68((x_ax0_658) + 1) = (x_cb68((x_ax0_658) + 1) + x_cb67((x_ax0_658) + 1, (x_rd1_660) + 1, &
                    &(x_rd0_659) + 1))
                end do
            end do
        end do
        do x_w0_661 = 0, (np_particles) - 1
            x_hcall9((x_w0_661) + 1) = x_cb68((x_w0_661) + 1)
        end do
        do x_w0_662 = 0, (np_particles) - 1
            Bzp((x_w0_662) + 1) = Bzp((x_w0_662) + 1) + (x_hcall9((x_w0_662) + 1))
        end do
        if (.not. allocated(x_cb69)) then
            allocate(x_cb69((o + 1)))
        else if (size(x_cb69, 1) /= ((o + 1))) then
            deallocate(x_cb69)
            allocate(x_cb69((o + 1)))
        end if
        if (.not. allocated(x_cb69)) then
            allocate(x_cb69((o + 1)))
        else if (size(x_cb69, 1) /= ((o + 1))) then
            deallocate(x_cb69)
            allocate(x_cb69((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ez)
        do x_i_663 = 0, (x_inl1_n_sx_ez) - 1
            x_cb69((x_i_663) + 1) = x_i_663
        end do
        if (.not. allocated(x_inl23_ta)) then
            allocate(x_inl23_ta((o + 1)))
        else if (size(x_inl23_ta, 1) /= ((o + 1))) then
            deallocate(x_inl23_ta)
            allocate(x_inl23_ta((o + 1)))
        end if
        do x_w0_664 = 0, (x_inl1_n_sx_ez) - 1
            x_inl23_ta((x_w0_664) + 1) = x_cb69((x_w0_664) + 1)
        end do
        if (.not. allocated(x_cb70)) then
            allocate(x_cb70(((o - gal) + 1)))
        else if (size(x_cb70, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb70)
            allocate(x_cb70(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb70)) then
            allocate(x_cb70(((o - gal) + 1)))
        else if (size(x_cb70, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb70)
            allocate(x_cb70(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ez)
        do x_i_665 = 0, (x_inl1_n_sz_ez) - 1
            x_cb70((x_i_665) + 1) = x_i_665
        end do
        if (.not. allocated(x_inl23_tb)) then
            allocate(x_inl23_tb(((o - gal) + 1)))
        else if (size(x_inl23_tb, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl23_tb)
            allocate(x_inl23_tb(((o - gal) + 1)))
        end if
        do x_w0_666 = 0, (x_inl1_n_sz_ez) - 1
            x_inl23_tb((x_w0_666) + 1) = x_cb70((x_w0_666) + 1)
        end do
        if (.not. allocated(x_inl23_ia)) then
            allocate(x_inl23_ia(np_particles, 1, (o + 1)))
        else if (size(x_inl23_ia, 1) /= (np_particles) .or. size(x_inl23_ia, 2) /= (1) .or. size(x_inl23_ia, 3) /= ((o &
        &+ 1))) then
            deallocate(x_inl23_ia)
            allocate(x_inl23_ia(np_particles, 1, (o + 1)))
        end if
        do si0_l667 = 0, ((o + 1)) - 1
            do si1_l668 = 0, (1) - 1
                do si2_l669 = 0, (np_particles) - 1
                    x_inl23_ia((si2_l669) + 1, (si1_l668) + 1, (si0_l667) + 1) = ((x_inl1_lox + x_inl1_j_ez((si2_l669) &
                    &+ 1)) + x_inl23_ta((si0_l667) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl23_ib)) then
            allocate(x_inl23_ib(np_particles, ((o - gal) + 1), 1))
        else if (size(x_inl23_ib, 1) /= (np_particles) .or. size(x_inl23_ib, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl23_ib, 3) /= (1)) then
            deallocate(x_inl23_ib)
            allocate(x_inl23_ib(np_particles, ((o - gal) + 1), 1))
        end if
        do si0_l670 = 0, (1) - 1
            do si1_l671 = 0, (((o - gal) + 1)) - 1
                do si2_l672 = 0, (np_particles) - 1
                    x_inl23_ib((si2_l672) + 1, (si1_l671) + 1, (si0_l670) + 1) = ((x_inl1_loy + x_inl1_l_ez((si2_l672) &
                    &+ 1)) + x_inl23_tb((si1_l671) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl23_ia_b)) then
            allocate(x_inl23_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl23_ia_b, 1) /= (np_particles) .or. size(x_inl23_ia_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl23_ia_b, 3) /= ((o + 1))) then
            deallocate(x_inl23_ia_b)
            allocate(x_inl23_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l673 = 0, ((o + 1)) - 1
            do si1_l674 = 0, (((o - gal) + 1)) - 1
                do si2_l675 = 0, (np_particles) - 1
                    x_inl23_ia_b((si2_l675) + 1, (si1_l674) + 1, (si0_l673) + 1) = x_inl23_ia((si2_l675) + 1, (0) + 1, &
                    &(si0_l673) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl23_ib_b)) then
            allocate(x_inl23_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl23_ib_b, 1) /= (np_particles) .or. size(x_inl23_ib_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl23_ib_b, 3) /= ((o + 1))) then
            deallocate(x_inl23_ib_b)
            allocate(x_inl23_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l676 = 0, ((o + 1)) - 1
            do si1_l677 = 0, (((o - gal) + 1)) - 1
                do si2_l678 = 0, (np_particles) - 1
                    x_inl23_ib_b((si2_l678) + 1, (si1_l677) + 1, (si0_l676) + 1) = x_inl23_ib((si2_l678) + 1, &
                    &(si1_l677) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl23_gathered)) then
            allocate(x_inl23_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl23_gathered, 1) /= (np_particles) .or. size(x_inl23_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl23_gathered, 3) /= ((o + 1))) then
            deallocate(x_inl23_gathered)
            allocate(x_inl23_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_679 = 0, ((o + 1)) - 1
            do x_w1_680 = 0, (((o - gal) + 1)) - 1
                do x_w2_681 = 0, (np_particles) - 1
                    x_inl23_gathered((x_w2_681) + 1, (x_w1_680) + 1, (x_w0_679) + 1) = ez_arr((0) + 1, (0) + 1, &
                    &(x_inl23_ib_b((x_w2_681) + 1, (x_w1_680) + 1, (x_w0_679) + 1)) + 1, (x_inl23_ia_b((x_w2_681) + 1, &
                    &(x_w1_680) + 1, (x_w0_679) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl23_weight)) then
            allocate(x_inl23_weight(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl23_weight, 1) /= (np_particles) .or. size(x_inl23_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl23_weight, 3) /= ((o + 1))) then
            deallocate(x_inl23_weight)
            allocate(x_inl23_weight(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_682 = 0, ((o + 1)) - 1
            do x_w1_683 = 0, (((o - gal) + 1)) - 1
                do x_w2_684 = 0, (np_particles) - 1
                    x_inl23_weight((x_w2_684) + 1, (x_w1_683) + 1, (x_w0_682) + 1) = (x_inl1_sx_ez((x_w2_684) + 1, &
                    &(x_w0_682) + 1) * x_inl1_sz_ez((x_w2_684) + 1, (x_w1_683) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb71)) then
            allocate(x_cb71(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_cb71, 1) /= (np_particles) .or. size(x_cb71, 2) /= (((o - gal) + 1)) .or. size(x_cb71, 3) /= &
        &((o + 1))) then
            deallocate(x_cb71)
            allocate(x_cb71(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_685 = 0, ((o + 1)) - 1
            do x_w1_686 = 0, (((o - gal) + 1)) - 1
                do x_w2_687 = 0, (np_particles) - 1
                    x_cb71((x_w2_687) + 1, (x_w1_686) + 1, (x_w0_685) + 1) = (x_inl23_weight((x_w2_687) + 1, &
                    &(x_w1_686) + 1, (x_w0_685) + 1) * x_inl23_gathered((x_w2_687) + 1, (x_w1_686) + 1, (x_w0_685) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb71, axis=(0, 1))
        do x_ax0_688 = 0, (np_particles) - 1
            x_cb72((x_ax0_688) + 1) = 0.0_c_double
            do x_rd0_689 = 0, ((o + 1)) - 1
                do x_rd1_690 = 0, (((o - gal) + 1)) - 1
                    x_cb72((x_ax0_688) + 1) = (x_cb72((x_ax0_688) + 1) + x_cb71((x_ax0_688) + 1, (x_rd1_690) + 1, &
                    &(x_rd0_689) + 1))
                end do
            end do
        end do
        do x_w0_691 = 0, (np_particles) - 1
            x_hcall10((x_w0_691) + 1) = x_cb72((x_w0_691) + 1)
        end do
        do x_w0_692 = 0, (np_particles) - 1
            Ezp((x_w0_692) + 1) = Ezp((x_w0_692) + 1) + (x_hcall10((x_w0_692) + 1))
        end do
        if (.not. allocated(x_cb73)) then
            allocate(x_cb73((o + 1)))
        else if (size(x_cb73, 1) /= ((o + 1))) then
            deallocate(x_cb73)
            allocate(x_cb73((o + 1)))
        end if
        if (.not. allocated(x_cb73)) then
            allocate(x_cb73((o + 1)))
        else if (size(x_cb73, 1) /= ((o + 1))) then
            deallocate(x_cb73)
            allocate(x_cb73((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bx)
        do x_i_693 = 0, (x_inl1_n_sx_bx) - 1
            x_cb73((x_i_693) + 1) = x_i_693
        end do
        if (.not. allocated(x_inl24_ta)) then
            allocate(x_inl24_ta((o + 1)))
        else if (size(x_inl24_ta, 1) /= ((o + 1))) then
            deallocate(x_inl24_ta)
            allocate(x_inl24_ta((o + 1)))
        end if
        do x_w0_694 = 0, (x_inl1_n_sx_bx) - 1
            x_inl24_ta((x_w0_694) + 1) = x_cb73((x_w0_694) + 1)
        end do
        if (.not. allocated(x_cb74)) then
            allocate(x_cb74(((o - gal) + 1)))
        else if (size(x_cb74, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb74)
            allocate(x_cb74(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb74)) then
            allocate(x_cb74(((o - gal) + 1)))
        else if (size(x_cb74, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb74)
            allocate(x_cb74(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bx)
        do x_i_695 = 0, (x_inl1_n_sz_bx) - 1
            x_cb74((x_i_695) + 1) = x_i_695
        end do
        if (.not. allocated(x_inl24_tb)) then
            allocate(x_inl24_tb(((o - gal) + 1)))
        else if (size(x_inl24_tb, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl24_tb)
            allocate(x_inl24_tb(((o - gal) + 1)))
        end if
        do x_w0_696 = 0, (x_inl1_n_sz_bx) - 1
            x_inl24_tb((x_w0_696) + 1) = x_cb74((x_w0_696) + 1)
        end do
        if (.not. allocated(x_inl24_ia)) then
            allocate(x_inl24_ia(np_particles, 1, (o + 1)))
        else if (size(x_inl24_ia, 1) /= (np_particles) .or. size(x_inl24_ia, 2) /= (1) .or. size(x_inl24_ia, 3) /= ((o &
        &+ 1))) then
            deallocate(x_inl24_ia)
            allocate(x_inl24_ia(np_particles, 1, (o + 1)))
        end if
        do si0_l697 = 0, ((o + 1)) - 1
            do si1_l698 = 0, (1) - 1
                do si2_l699 = 0, (np_particles) - 1
                    x_inl24_ia((si2_l699) + 1, (si1_l698) + 1, (si0_l697) + 1) = ((x_inl1_lox + x_inl1_j_bx((si2_l699) &
                    &+ 1)) + x_inl24_ta((si0_l697) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl24_ib)) then
            allocate(x_inl24_ib(np_particles, ((o - gal) + 1), 1))
        else if (size(x_inl24_ib, 1) /= (np_particles) .or. size(x_inl24_ib, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl24_ib, 3) /= (1)) then
            deallocate(x_inl24_ib)
            allocate(x_inl24_ib(np_particles, ((o - gal) + 1), 1))
        end if
        do si0_l700 = 0, (1) - 1
            do si1_l701 = 0, (((o - gal) + 1)) - 1
                do si2_l702 = 0, (np_particles) - 1
                    x_inl24_ib((si2_l702) + 1, (si1_l701) + 1, (si0_l700) + 1) = ((x_inl1_loy + x_inl1_l_bx((si2_l702) &
                    &+ 1)) + x_inl24_tb((si1_l701) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl24_ia_b)) then
            allocate(x_inl24_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl24_ia_b, 1) /= (np_particles) .or. size(x_inl24_ia_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl24_ia_b, 3) /= ((o + 1))) then
            deallocate(x_inl24_ia_b)
            allocate(x_inl24_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l703 = 0, ((o + 1)) - 1
            do si1_l704 = 0, (((o - gal) + 1)) - 1
                do si2_l705 = 0, (np_particles) - 1
                    x_inl24_ia_b((si2_l705) + 1, (si1_l704) + 1, (si0_l703) + 1) = x_inl24_ia((si2_l705) + 1, (0) + 1, &
                    &(si0_l703) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl24_ib_b)) then
            allocate(x_inl24_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl24_ib_b, 1) /= (np_particles) .or. size(x_inl24_ib_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl24_ib_b, 3) /= ((o + 1))) then
            deallocate(x_inl24_ib_b)
            allocate(x_inl24_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l706 = 0, ((o + 1)) - 1
            do si1_l707 = 0, (((o - gal) + 1)) - 1
                do si2_l708 = 0, (np_particles) - 1
                    x_inl24_ib_b((si2_l708) + 1, (si1_l707) + 1, (si0_l706) + 1) = x_inl24_ib((si2_l708) + 1, &
                    &(si1_l707) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl24_gathered)) then
            allocate(x_inl24_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl24_gathered, 1) /= (np_particles) .or. size(x_inl24_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl24_gathered, 3) /= ((o + 1))) then
            deallocate(x_inl24_gathered)
            allocate(x_inl24_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_709 = 0, ((o + 1)) - 1
            do x_w1_710 = 0, (((o - gal) + 1)) - 1
                do x_w2_711 = 0, (np_particles) - 1
                    x_inl24_gathered((x_w2_711) + 1, (x_w1_710) + 1, (x_w0_709) + 1) = bx_arr((0) + 1, (0) + 1, &
                    &(x_inl24_ib_b((x_w2_711) + 1, (x_w1_710) + 1, (x_w0_709) + 1)) + 1, (x_inl24_ia_b((x_w2_711) + 1, &
                    &(x_w1_710) + 1, (x_w0_709) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl24_weight)) then
            allocate(x_inl24_weight(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl24_weight, 1) /= (np_particles) .or. size(x_inl24_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl24_weight, 3) /= ((o + 1))) then
            deallocate(x_inl24_weight)
            allocate(x_inl24_weight(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_712 = 0, ((o + 1)) - 1
            do x_w1_713 = 0, (((o - gal) + 1)) - 1
                do x_w2_714 = 0, (np_particles) - 1
                    x_inl24_weight((x_w2_714) + 1, (x_w1_713) + 1, (x_w0_712) + 1) = (x_inl1_sx_bx((x_w2_714) + 1, &
                    &(x_w0_712) + 1) * x_inl1_sz_bx((x_w2_714) + 1, (x_w1_713) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb75)) then
            allocate(x_cb75(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_cb75, 1) /= (np_particles) .or. size(x_cb75, 2) /= (((o - gal) + 1)) .or. size(x_cb75, 3) /= &
        &((o + 1))) then
            deallocate(x_cb75)
            allocate(x_cb75(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_715 = 0, ((o + 1)) - 1
            do x_w1_716 = 0, (((o - gal) + 1)) - 1
                do x_w2_717 = 0, (np_particles) - 1
                    x_cb75((x_w2_717) + 1, (x_w1_716) + 1, (x_w0_715) + 1) = (x_inl24_weight((x_w2_717) + 1, &
                    &(x_w1_716) + 1, (x_w0_715) + 1) * x_inl24_gathered((x_w2_717) + 1, (x_w1_716) + 1, (x_w0_715) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb75, axis=(0, 1))
        do x_ax0_718 = 0, (np_particles) - 1
            x_cb76((x_ax0_718) + 1) = 0.0_c_double
            do x_rd0_719 = 0, ((o + 1)) - 1
                do x_rd1_720 = 0, (((o - gal) + 1)) - 1
                    x_cb76((x_ax0_718) + 1) = (x_cb76((x_ax0_718) + 1) + x_cb75((x_ax0_718) + 1, (x_rd1_720) + 1, &
                    &(x_rd0_719) + 1))
                end do
            end do
        end do
        do x_w0_721 = 0, (np_particles) - 1
            x_hcall11((x_w0_721) + 1) = x_cb76((x_w0_721) + 1)
        end do
        do x_w0_722 = 0, (np_particles) - 1
            Bxp((x_w0_722) + 1) = Bxp((x_w0_722) + 1) + (x_hcall11((x_w0_722) + 1))
        end do
        if (.not. allocated(x_cb77)) then
            allocate(x_cb77(((o - gal) + 1)))
        else if (size(x_cb77, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb77)
            allocate(x_cb77(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb77)) then
            allocate(x_cb77(((o - gal) + 1)))
        else if (size(x_cb77, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb77)
            allocate(x_cb77(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_by)
        do x_i_723 = 0, (x_inl1_n_sx_by) - 1
            x_cb77((x_i_723) + 1) = x_i_723
        end do
        if (.not. allocated(x_inl25_ta)) then
            allocate(x_inl25_ta(((o - gal) + 1)))
        else if (size(x_inl25_ta, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl25_ta)
            allocate(x_inl25_ta(((o - gal) + 1)))
        end if
        do x_w0_724 = 0, (x_inl1_n_sx_by) - 1
            x_inl25_ta((x_w0_724) + 1) = x_cb77((x_w0_724) + 1)
        end do
        if (.not. allocated(x_cb78)) then
            allocate(x_cb78(((o - gal) + 1)))
        else if (size(x_cb78, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb78)
            allocate(x_cb78(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb78)) then
            allocate(x_cb78(((o - gal) + 1)))
        else if (size(x_cb78, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb78)
            allocate(x_cb78(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_by)
        do x_i_725 = 0, (x_inl1_n_sz_by) - 1
            x_cb78((x_i_725) + 1) = x_i_725
        end do
        if (.not. allocated(x_inl25_tb)) then
            allocate(x_inl25_tb(((o - gal) + 1)))
        else if (size(x_inl25_tb, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl25_tb)
            allocate(x_inl25_tb(((o - gal) + 1)))
        end if
        do x_w0_726 = 0, (x_inl1_n_sz_by) - 1
            x_inl25_tb((x_w0_726) + 1) = x_cb78((x_w0_726) + 1)
        end do
        if (.not. allocated(x_inl25_ia)) then
            allocate(x_inl25_ia(np_particles, 1, ((o - gal) + 1)))
        else if (size(x_inl25_ia, 1) /= (np_particles) .or. size(x_inl25_ia, 2) /= (1) .or. size(x_inl25_ia, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_inl25_ia)
            allocate(x_inl25_ia(np_particles, 1, ((o - gal) + 1)))
        end if
        do si0_l727 = 0, (((o - gal) + 1)) - 1
            do si1_l728 = 0, (1) - 1
                do si2_l729 = 0, (np_particles) - 1
                    x_inl25_ia((si2_l729) + 1, (si1_l728) + 1, (si0_l727) + 1) = ((x_inl1_lox + x_inl1_j_by((si2_l729) &
                    &+ 1)) + x_inl25_ta((si0_l727) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl25_ib)) then
            allocate(x_inl25_ib(np_particles, ((o - gal) + 1), 1))
        else if (size(x_inl25_ib, 1) /= (np_particles) .or. size(x_inl25_ib, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl25_ib, 3) /= (1)) then
            deallocate(x_inl25_ib)
            allocate(x_inl25_ib(np_particles, ((o - gal) + 1), 1))
        end if
        do si0_l730 = 0, (1) - 1
            do si1_l731 = 0, (((o - gal) + 1)) - 1
                do si2_l732 = 0, (np_particles) - 1
                    x_inl25_ib((si2_l732) + 1, (si1_l731) + 1, (si0_l730) + 1) = ((x_inl1_loy + x_inl1_l_by((si2_l732) &
                    &+ 1)) + x_inl25_tb((si1_l731) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl25_ia_b)) then
            allocate(x_inl25_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl25_ia_b, 1) /= (np_particles) .or. size(x_inl25_ia_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl25_ia_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl25_ia_b)
            allocate(x_inl25_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l733 = 0, (((o - gal) + 1)) - 1
            do si1_l734 = 0, (((o - gal) + 1)) - 1
                do si2_l735 = 0, (np_particles) - 1
                    x_inl25_ia_b((si2_l735) + 1, (si1_l734) + 1, (si0_l733) + 1) = x_inl25_ia((si2_l735) + 1, (0) + 1, &
                    &(si0_l733) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl25_ib_b)) then
            allocate(x_inl25_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl25_ib_b, 1) /= (np_particles) .or. size(x_inl25_ib_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl25_ib_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl25_ib_b)
            allocate(x_inl25_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l736 = 0, (((o - gal) + 1)) - 1
            do si1_l737 = 0, (((o - gal) + 1)) - 1
                do si2_l738 = 0, (np_particles) - 1
                    x_inl25_ib_b((si2_l738) + 1, (si1_l737) + 1, (si0_l736) + 1) = x_inl25_ib((si2_l738) + 1, &
                    &(si1_l737) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl25_gathered)) then
            allocate(x_inl25_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl25_gathered, 1) /= (np_particles) .or. size(x_inl25_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl25_gathered, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl25_gathered)
            allocate(x_inl25_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_739 = 0, (((o - gal) + 1)) - 1
            do x_w1_740 = 0, (((o - gal) + 1)) - 1
                do x_w2_741 = 0, (np_particles) - 1
                    x_inl25_gathered((x_w2_741) + 1, (x_w1_740) + 1, (x_w0_739) + 1) = by_arr((0) + 1, (0) + 1, &
                    &(x_inl25_ib_b((x_w2_741) + 1, (x_w1_740) + 1, (x_w0_739) + 1)) + 1, (x_inl25_ia_b((x_w2_741) + 1, &
                    &(x_w1_740) + 1, (x_w0_739) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl25_weight)) then
            allocate(x_inl25_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl25_weight, 1) /= (np_particles) .or. size(x_inl25_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl25_weight, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl25_weight)
            allocate(x_inl25_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_742 = 0, (((o - gal) + 1)) - 1
            do x_w1_743 = 0, (((o - gal) + 1)) - 1
                do x_w2_744 = 0, (np_particles) - 1
                    x_inl25_weight((x_w2_744) + 1, (x_w1_743) + 1, (x_w0_742) + 1) = (x_inl1_sx_by((x_w2_744) + 1, &
                    &(x_w0_742) + 1) * x_inl1_sz_by((x_w2_744) + 1, (x_w1_743) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb79)) then
            allocate(x_cb79(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_cb79, 1) /= (np_particles) .or. size(x_cb79, 2) /= (((o - gal) + 1)) .or. size(x_cb79, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_cb79)
            allocate(x_cb79(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_745 = 0, (((o - gal) + 1)) - 1
            do x_w1_746 = 0, (((o - gal) + 1)) - 1
                do x_w2_747 = 0, (np_particles) - 1
                    x_cb79((x_w2_747) + 1, (x_w1_746) + 1, (x_w0_745) + 1) = (x_inl25_weight((x_w2_747) + 1, &
                    &(x_w1_746) + 1, (x_w0_745) + 1) * x_inl25_gathered((x_w2_747) + 1, (x_w1_746) + 1, (x_w0_745) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb79, axis=(0, 1))
        do x_ax0_748 = 0, (np_particles) - 1
            x_cb80((x_ax0_748) + 1) = 0.0_c_double
            do x_rd0_749 = 0, (((o - gal) + 1)) - 1
                do x_rd1_750 = 0, (((o - gal) + 1)) - 1
                    x_cb80((x_ax0_748) + 1) = (x_cb80((x_ax0_748) + 1) + x_cb79((x_ax0_748) + 1, (x_rd1_750) + 1, &
                    &(x_rd0_749) + 1))
                end do
            end do
        end do
        do x_w0_751 = 0, (np_particles) - 1
            x_hcall12((x_w0_751) + 1) = x_cb80((x_w0_751) + 1)
        end do
        do x_w0_752 = 0, (np_particles) - 1
            Byp((x_w0_752) + 1) = Byp((x_w0_752) + 1) + (x_hcall12((x_w0_752) + 1))
        end do
    else if ((g == 2)) then
        if (.not. allocated(x_cb81)) then
            allocate(x_cb81((o + 1)))
        else if (size(x_cb81, 1) /= ((o + 1))) then
            deallocate(x_cb81)
            allocate(x_cb81((o + 1)))
        end if
        if (.not. allocated(x_cb81)) then
            allocate(x_cb81((o + 1)))
        else if (size(x_cb81, 1) /= ((o + 1))) then
            deallocate(x_cb81)
            allocate(x_cb81((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ey)
        do x_i_753 = 0, (x_inl1_n_sx_ey) - 1
            x_cb81((x_i_753) + 1) = x_i_753
        end do
        if (.not. allocated(x_inl26_ta)) then
            allocate(x_inl26_ta((o + 1)))
        else if (size(x_inl26_ta, 1) /= ((o + 1))) then
            deallocate(x_inl26_ta)
            allocate(x_inl26_ta((o + 1)))
        end if
        do x_w0_754 = 0, (x_inl1_n_sx_ey) - 1
            x_inl26_ta((x_w0_754) + 1) = x_cb81((x_w0_754) + 1)
        end do
        if (.not. allocated(x_cb82)) then
            allocate(x_cb82((o + 1)))
        else if (size(x_cb82, 1) /= ((o + 1))) then
            deallocate(x_cb82)
            allocate(x_cb82((o + 1)))
        end if
        if (.not. allocated(x_cb82)) then
            allocate(x_cb82((o + 1)))
        else if (size(x_cb82, 1) /= ((o + 1))) then
            deallocate(x_cb82)
            allocate(x_cb82((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ey)
        do x_i_755 = 0, (x_inl1_n_sz_ey) - 1
            x_cb82((x_i_755) + 1) = x_i_755
        end do
        if (.not. allocated(x_inl26_tb)) then
            allocate(x_inl26_tb((o + 1)))
        else if (size(x_inl26_tb, 1) /= ((o + 1))) then
            deallocate(x_inl26_tb)
            allocate(x_inl26_tb((o + 1)))
        end if
        do x_w0_756 = 0, (x_inl1_n_sz_ey) - 1
            x_inl26_tb((x_w0_756) + 1) = x_cb82((x_w0_756) + 1)
        end do
        if (.not. allocated(x_inl26_ia)) then
            allocate(x_inl26_ia(np_particles, 1, (o + 1)))
        else if (size(x_inl26_ia, 1) /= (np_particles) .or. size(x_inl26_ia, 2) /= (1) .or. size(x_inl26_ia, 3) /= ((o &
        &+ 1))) then
            deallocate(x_inl26_ia)
            allocate(x_inl26_ia(np_particles, 1, (o + 1)))
        end if
        do si0_l757 = 0, ((o + 1)) - 1
            do si1_l758 = 0, (1) - 1
                do si2_l759 = 0, (np_particles) - 1
                    x_inl26_ia((si2_l759) + 1, (si1_l758) + 1, (si0_l757) + 1) = ((x_inl1_lox + x_inl1_j_ey((si2_l759) &
                    &+ 1)) + x_inl26_ta((si0_l757) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl26_ib)) then
            allocate(x_inl26_ib(np_particles, (o + 1), 1))
        else if (size(x_inl26_ib, 1) /= (np_particles) .or. size(x_inl26_ib, 2) /= ((o + 1)) .or. size(x_inl26_ib, 3) &
        &/= (1)) then
            deallocate(x_inl26_ib)
            allocate(x_inl26_ib(np_particles, (o + 1), 1))
        end if
        do si0_l760 = 0, (1) - 1
            do si1_l761 = 0, ((o + 1)) - 1
                do si2_l762 = 0, (np_particles) - 1
                    x_inl26_ib((si2_l762) + 1, (si1_l761) + 1, (si0_l760) + 1) = ((x_inl1_loy + x_inl1_l_ey((si2_l762) &
                    &+ 1)) + x_inl26_tb((si1_l761) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl26_ia_b)) then
            allocate(x_inl26_ia_b(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl26_ia_b, 1) /= (np_particles) .or. size(x_inl26_ia_b, 2) /= ((o + 1)) .or. &
        &size(x_inl26_ia_b, 3) /= ((o + 1))) then
            deallocate(x_inl26_ia_b)
            allocate(x_inl26_ia_b(np_particles, (o + 1), (o + 1)))
        end if
        do si0_l763 = 0, ((o + 1)) - 1
            do si1_l764 = 0, ((o + 1)) - 1
                do si2_l765 = 0, (np_particles) - 1
                    x_inl26_ia_b((si2_l765) + 1, (si1_l764) + 1, (si0_l763) + 1) = x_inl26_ia((si2_l765) + 1, (0) + 1, &
                    &(si0_l763) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl26_ib_b)) then
            allocate(x_inl26_ib_b(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl26_ib_b, 1) /= (np_particles) .or. size(x_inl26_ib_b, 2) /= ((o + 1)) .or. &
        &size(x_inl26_ib_b, 3) /= ((o + 1))) then
            deallocate(x_inl26_ib_b)
            allocate(x_inl26_ib_b(np_particles, (o + 1), (o + 1)))
        end if
        do si0_l766 = 0, ((o + 1)) - 1
            do si1_l767 = 0, ((o + 1)) - 1
                do si2_l768 = 0, (np_particles) - 1
                    x_inl26_ib_b((si2_l768) + 1, (si1_l767) + 1, (si0_l766) + 1) = x_inl26_ib((si2_l768) + 1, &
                    &(si1_l767) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl26_gathered)) then
            allocate(x_inl26_gathered(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl26_gathered, 1) /= (np_particles) .or. size(x_inl26_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl26_gathered, 3) /= ((o + 1))) then
            deallocate(x_inl26_gathered)
            allocate(x_inl26_gathered(np_particles, (o + 1), (o + 1)))
        end if
        do x_w0_769 = 0, ((o + 1)) - 1
            do x_w1_770 = 0, ((o + 1)) - 1
                do x_w2_771 = 0, (np_particles) - 1
                    x_inl26_gathered((x_w2_771) + 1, (x_w1_770) + 1, (x_w0_769) + 1) = ey_arr((0) + 1, (0) + 1, &
                    &(x_inl26_ib_b((x_w2_771) + 1, (x_w1_770) + 1, (x_w0_769) + 1)) + 1, (x_inl26_ia_b((x_w2_771) + 1, &
                    &(x_w1_770) + 1, (x_w0_769) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl26_weight)) then
            allocate(x_inl26_weight(np_particles, (o + 1), (o + 1)))
        else if (size(x_inl26_weight, 1) /= (np_particles) .or. size(x_inl26_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl26_weight, 3) /= ((o + 1))) then
            deallocate(x_inl26_weight)
            allocate(x_inl26_weight(np_particles, (o + 1), (o + 1)))
        end if
        do x_w0_772 = 0, ((o + 1)) - 1
            do x_w1_773 = 0, ((o + 1)) - 1
                do x_w2_774 = 0, (np_particles) - 1
                    x_inl26_weight((x_w2_774) + 1, (x_w1_773) + 1, (x_w0_772) + 1) = (x_inl1_sx_ey((x_w2_774) + 1, &
                    &(x_w0_772) + 1) * x_inl1_sz_ey((x_w2_774) + 1, (x_w1_773) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb83)) then
            allocate(x_cb83(np_particles, (o + 1), (o + 1)))
        else if (size(x_cb83, 1) /= (np_particles) .or. size(x_cb83, 2) /= ((o + 1)) .or. size(x_cb83, 3) /= ((o + &
        &1))) then
            deallocate(x_cb83)
            allocate(x_cb83(np_particles, (o + 1), (o + 1)))
        end if
        do x_w0_775 = 0, ((o + 1)) - 1
            do x_w1_776 = 0, ((o + 1)) - 1
                do x_w2_777 = 0, (np_particles) - 1
                    x_cb83((x_w2_777) + 1, (x_w1_776) + 1, (x_w0_775) + 1) = (x_inl26_weight((x_w2_777) + 1, &
                    &(x_w1_776) + 1, (x_w0_775) + 1) * x_inl26_gathered((x_w2_777) + 1, (x_w1_776) + 1, (x_w0_775) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb83, axis=(0, 1))
        do x_ax0_778 = 0, (np_particles) - 1
            x_cb84((x_ax0_778) + 1) = 0.0_c_double
            do x_rd0_779 = 0, ((o + 1)) - 1
                do x_rd1_780 = 0, ((o + 1)) - 1
                    x_cb84((x_ax0_778) + 1) = (x_cb84((x_ax0_778) + 1) + x_cb83((x_ax0_778) + 1, (x_rd1_780) + 1, &
                    &(x_rd0_779) + 1))
                end do
            end do
        end do
        do x_w0_781 = 0, (np_particles) - 1
            x_inl1_Ethetap((x_w0_781) + 1) = x_cb84((x_w0_781) + 1)
        end do
        if (.not. allocated(x_cb85)) then
            allocate(x_cb85(((o - gal) + 1)))
        else if (size(x_cb85, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb85)
            allocate(x_cb85(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb85)) then
            allocate(x_cb85(((o - gal) + 1)))
        else if (size(x_cb85, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb85)
            allocate(x_cb85(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ex)
        do x_i_782 = 0, (x_inl1_n_sx_ex) - 1
            x_cb85((x_i_782) + 1) = x_i_782
        end do
        if (.not. allocated(x_inl27_ta)) then
            allocate(x_inl27_ta(((o - gal) + 1)))
        else if (size(x_inl27_ta, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl27_ta)
            allocate(x_inl27_ta(((o - gal) + 1)))
        end if
        do x_w0_783 = 0, (x_inl1_n_sx_ex) - 1
            x_inl27_ta((x_w0_783) + 1) = x_cb85((x_w0_783) + 1)
        end do
        if (.not. allocated(x_cb86)) then
            allocate(x_cb86((o + 1)))
        else if (size(x_cb86, 1) /= ((o + 1))) then
            deallocate(x_cb86)
            allocate(x_cb86((o + 1)))
        end if
        if (.not. allocated(x_cb86)) then
            allocate(x_cb86((o + 1)))
        else if (size(x_cb86, 1) /= ((o + 1))) then
            deallocate(x_cb86)
            allocate(x_cb86((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ex)
        do x_i_784 = 0, (x_inl1_n_sz_ex) - 1
            x_cb86((x_i_784) + 1) = x_i_784
        end do
        if (.not. allocated(x_inl27_tb)) then
            allocate(x_inl27_tb((o + 1)))
        else if (size(x_inl27_tb, 1) /= ((o + 1))) then
            deallocate(x_inl27_tb)
            allocate(x_inl27_tb((o + 1)))
        end if
        do x_w0_785 = 0, (x_inl1_n_sz_ex) - 1
            x_inl27_tb((x_w0_785) + 1) = x_cb86((x_w0_785) + 1)
        end do
        if (.not. allocated(x_inl27_ia)) then
            allocate(x_inl27_ia(np_particles, 1, ((o - gal) + 1)))
        else if (size(x_inl27_ia, 1) /= (np_particles) .or. size(x_inl27_ia, 2) /= (1) .or. size(x_inl27_ia, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_inl27_ia)
            allocate(x_inl27_ia(np_particles, 1, ((o - gal) + 1)))
        end if
        do si0_l786 = 0, (((o - gal) + 1)) - 1
            do si1_l787 = 0, (1) - 1
                do si2_l788 = 0, (np_particles) - 1
                    x_inl27_ia((si2_l788) + 1, (si1_l787) + 1, (si0_l786) + 1) = ((x_inl1_lox + x_inl1_j_ex((si2_l788) &
                    &+ 1)) + x_inl27_ta((si0_l786) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl27_ib)) then
            allocate(x_inl27_ib(np_particles, (o + 1), 1))
        else if (size(x_inl27_ib, 1) /= (np_particles) .or. size(x_inl27_ib, 2) /= ((o + 1)) .or. size(x_inl27_ib, 3) &
        &/= (1)) then
            deallocate(x_inl27_ib)
            allocate(x_inl27_ib(np_particles, (o + 1), 1))
        end if
        do si0_l789 = 0, (1) - 1
            do si1_l790 = 0, ((o + 1)) - 1
                do si2_l791 = 0, (np_particles) - 1
                    x_inl27_ib((si2_l791) + 1, (si1_l790) + 1, (si0_l789) + 1) = ((x_inl1_loy + x_inl1_l_ex((si2_l791) &
                    &+ 1)) + x_inl27_tb((si1_l790) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl27_ia_b)) then
            allocate(x_inl27_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl27_ia_b, 1) /= (np_particles) .or. size(x_inl27_ia_b, 2) /= ((o + 1)) .or. &
        &size(x_inl27_ia_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl27_ia_b)
            allocate(x_inl27_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l792 = 0, (((o - gal) + 1)) - 1
            do si1_l793 = 0, ((o + 1)) - 1
                do si2_l794 = 0, (np_particles) - 1
                    x_inl27_ia_b((si2_l794) + 1, (si1_l793) + 1, (si0_l792) + 1) = x_inl27_ia((si2_l794) + 1, (0) + 1, &
                    &(si0_l792) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl27_ib_b)) then
            allocate(x_inl27_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl27_ib_b, 1) /= (np_particles) .or. size(x_inl27_ib_b, 2) /= ((o + 1)) .or. &
        &size(x_inl27_ib_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl27_ib_b)
            allocate(x_inl27_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l795 = 0, (((o - gal) + 1)) - 1
            do si1_l796 = 0, ((o + 1)) - 1
                do si2_l797 = 0, (np_particles) - 1
                    x_inl27_ib_b((si2_l797) + 1, (si1_l796) + 1, (si0_l795) + 1) = x_inl27_ib((si2_l797) + 1, &
                    &(si1_l796) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl27_gathered)) then
            allocate(x_inl27_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl27_gathered, 1) /= (np_particles) .or. size(x_inl27_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl27_gathered, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl27_gathered)
            allocate(x_inl27_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_798 = 0, (((o - gal) + 1)) - 1
            do x_w1_799 = 0, ((o + 1)) - 1
                do x_w2_800 = 0, (np_particles) - 1
                    x_inl27_gathered((x_w2_800) + 1, (x_w1_799) + 1, (x_w0_798) + 1) = ex_arr((0) + 1, (0) + 1, &
                    &(x_inl27_ib_b((x_w2_800) + 1, (x_w1_799) + 1, (x_w0_798) + 1)) + 1, (x_inl27_ia_b((x_w2_800) + 1, &
                    &(x_w1_799) + 1, (x_w0_798) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl27_weight)) then
            allocate(x_inl27_weight(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl27_weight, 1) /= (np_particles) .or. size(x_inl27_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl27_weight, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl27_weight)
            allocate(x_inl27_weight(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_801 = 0, (((o - gal) + 1)) - 1
            do x_w1_802 = 0, ((o + 1)) - 1
                do x_w2_803 = 0, (np_particles) - 1
                    x_inl27_weight((x_w2_803) + 1, (x_w1_802) + 1, (x_w0_801) + 1) = (x_inl1_sx_ex((x_w2_803) + 1, &
                    &(x_w0_801) + 1) * x_inl1_sz_ex((x_w2_803) + 1, (x_w1_802) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb87)) then
            allocate(x_cb87(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_cb87, 1) /= (np_particles) .or. size(x_cb87, 2) /= ((o + 1)) .or. size(x_cb87, 3) /= (((o - &
        &gal) + 1))) then
            deallocate(x_cb87)
            allocate(x_cb87(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_804 = 0, (((o - gal) + 1)) - 1
            do x_w1_805 = 0, ((o + 1)) - 1
                do x_w2_806 = 0, (np_particles) - 1
                    x_cb87((x_w2_806) + 1, (x_w1_805) + 1, (x_w0_804) + 1) = (x_inl27_weight((x_w2_806) + 1, &
                    &(x_w1_805) + 1, (x_w0_804) + 1) * x_inl27_gathered((x_w2_806) + 1, (x_w1_805) + 1, (x_w0_804) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb87, axis=(0, 1))
        do x_ax0_807 = 0, (np_particles) - 1
            x_cb88((x_ax0_807) + 1) = 0.0_c_double
            do x_rd0_808 = 0, (((o - gal) + 1)) - 1
                do x_rd1_809 = 0, ((o + 1)) - 1
                    x_cb88((x_ax0_807) + 1) = (x_cb88((x_ax0_807) + 1) + x_cb87((x_ax0_807) + 1, (x_rd1_809) + 1, &
                    &(x_rd0_808) + 1))
                end do
            end do
        end do
        do x_w0_810 = 0, (np_particles) - 1
            x_inl1_Erp((x_w0_810) + 1) = x_cb88((x_w0_810) + 1)
        end do
        if (.not. allocated(x_cb89)) then
            allocate(x_cb89(((o - gal) + 1)))
        else if (size(x_cb89, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb89)
            allocate(x_cb89(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb89)) then
            allocate(x_cb89(((o - gal) + 1)))
        else if (size(x_cb89, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb89)
            allocate(x_cb89(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bz)
        do x_i_811 = 0, (x_inl1_n_sx_bz) - 1
            x_cb89((x_i_811) + 1) = x_i_811
        end do
        if (.not. allocated(x_inl28_ta)) then
            allocate(x_inl28_ta(((o - gal) + 1)))
        else if (size(x_inl28_ta, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl28_ta)
            allocate(x_inl28_ta(((o - gal) + 1)))
        end if
        do x_w0_812 = 0, (x_inl1_n_sx_bz) - 1
            x_inl28_ta((x_w0_812) + 1) = x_cb89((x_w0_812) + 1)
        end do
        if (.not. allocated(x_cb90)) then
            allocate(x_cb90((o + 1)))
        else if (size(x_cb90, 1) /= ((o + 1))) then
            deallocate(x_cb90)
            allocate(x_cb90((o + 1)))
        end if
        if (.not. allocated(x_cb90)) then
            allocate(x_cb90((o + 1)))
        else if (size(x_cb90, 1) /= ((o + 1))) then
            deallocate(x_cb90)
            allocate(x_cb90((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bz)
        do x_i_813 = 0, (x_inl1_n_sz_bz) - 1
            x_cb90((x_i_813) + 1) = x_i_813
        end do
        if (.not. allocated(x_inl28_tb)) then
            allocate(x_inl28_tb((o + 1)))
        else if (size(x_inl28_tb, 1) /= ((o + 1))) then
            deallocate(x_inl28_tb)
            allocate(x_inl28_tb((o + 1)))
        end if
        do x_w0_814 = 0, (x_inl1_n_sz_bz) - 1
            x_inl28_tb((x_w0_814) + 1) = x_cb90((x_w0_814) + 1)
        end do
        if (.not. allocated(x_inl28_ia)) then
            allocate(x_inl28_ia(np_particles, 1, ((o - gal) + 1)))
        else if (size(x_inl28_ia, 1) /= (np_particles) .or. size(x_inl28_ia, 2) /= (1) .or. size(x_inl28_ia, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_inl28_ia)
            allocate(x_inl28_ia(np_particles, 1, ((o - gal) + 1)))
        end if
        do si0_l815 = 0, (((o - gal) + 1)) - 1
            do si1_l816 = 0, (1) - 1
                do si2_l817 = 0, (np_particles) - 1
                    x_inl28_ia((si2_l817) + 1, (si1_l816) + 1, (si0_l815) + 1) = ((x_inl1_lox + x_inl1_j_bz((si2_l817) &
                    &+ 1)) + x_inl28_ta((si0_l815) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl28_ib)) then
            allocate(x_inl28_ib(np_particles, (o + 1), 1))
        else if (size(x_inl28_ib, 1) /= (np_particles) .or. size(x_inl28_ib, 2) /= ((o + 1)) .or. size(x_inl28_ib, 3) &
        &/= (1)) then
            deallocate(x_inl28_ib)
            allocate(x_inl28_ib(np_particles, (o + 1), 1))
        end if
        do si0_l818 = 0, (1) - 1
            do si1_l819 = 0, ((o + 1)) - 1
                do si2_l820 = 0, (np_particles) - 1
                    x_inl28_ib((si2_l820) + 1, (si1_l819) + 1, (si0_l818) + 1) = ((x_inl1_loy + x_inl1_l_bz((si2_l820) &
                    &+ 1)) + x_inl28_tb((si1_l819) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl28_ia_b)) then
            allocate(x_inl28_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl28_ia_b, 1) /= (np_particles) .or. size(x_inl28_ia_b, 2) /= ((o + 1)) .or. &
        &size(x_inl28_ia_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl28_ia_b)
            allocate(x_inl28_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l821 = 0, (((o - gal) + 1)) - 1
            do si1_l822 = 0, ((o + 1)) - 1
                do si2_l823 = 0, (np_particles) - 1
                    x_inl28_ia_b((si2_l823) + 1, (si1_l822) + 1, (si0_l821) + 1) = x_inl28_ia((si2_l823) + 1, (0) + 1, &
                    &(si0_l821) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl28_ib_b)) then
            allocate(x_inl28_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl28_ib_b, 1) /= (np_particles) .or. size(x_inl28_ib_b, 2) /= ((o + 1)) .or. &
        &size(x_inl28_ib_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl28_ib_b)
            allocate(x_inl28_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do si0_l824 = 0, (((o - gal) + 1)) - 1
            do si1_l825 = 0, ((o + 1)) - 1
                do si2_l826 = 0, (np_particles) - 1
                    x_inl28_ib_b((si2_l826) + 1, (si1_l825) + 1, (si0_l824) + 1) = x_inl28_ib((si2_l826) + 1, &
                    &(si1_l825) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl28_gathered)) then
            allocate(x_inl28_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl28_gathered, 1) /= (np_particles) .or. size(x_inl28_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl28_gathered, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl28_gathered)
            allocate(x_inl28_gathered(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_827 = 0, (((o - gal) + 1)) - 1
            do x_w1_828 = 0, ((o + 1)) - 1
                do x_w2_829 = 0, (np_particles) - 1
                    x_inl28_gathered((x_w2_829) + 1, (x_w1_828) + 1, (x_w0_827) + 1) = bz_arr((0) + 1, (0) + 1, &
                    &(x_inl28_ib_b((x_w2_829) + 1, (x_w1_828) + 1, (x_w0_827) + 1)) + 1, (x_inl28_ia_b((x_w2_829) + 1, &
                    &(x_w1_828) + 1, (x_w0_827) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl28_weight)) then
            allocate(x_inl28_weight(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_inl28_weight, 1) /= (np_particles) .or. size(x_inl28_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl28_weight, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl28_weight)
            allocate(x_inl28_weight(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_830 = 0, (((o - gal) + 1)) - 1
            do x_w1_831 = 0, ((o + 1)) - 1
                do x_w2_832 = 0, (np_particles) - 1
                    x_inl28_weight((x_w2_832) + 1, (x_w1_831) + 1, (x_w0_830) + 1) = (x_inl1_sx_bz((x_w2_832) + 1, &
                    &(x_w0_830) + 1) * x_inl1_sz_bz((x_w2_832) + 1, (x_w1_831) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb91)) then
            allocate(x_cb91(np_particles, (o + 1), ((o - gal) + 1)))
        else if (size(x_cb91, 1) /= (np_particles) .or. size(x_cb91, 2) /= ((o + 1)) .or. size(x_cb91, 3) /= (((o - &
        &gal) + 1))) then
            deallocate(x_cb91)
            allocate(x_cb91(np_particles, (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_833 = 0, (((o - gal) + 1)) - 1
            do x_w1_834 = 0, ((o + 1)) - 1
                do x_w2_835 = 0, (np_particles) - 1
                    x_cb91((x_w2_835) + 1, (x_w1_834) + 1, (x_w0_833) + 1) = (x_inl28_weight((x_w2_835) + 1, &
                    &(x_w1_834) + 1, (x_w0_833) + 1) * x_inl28_gathered((x_w2_835) + 1, (x_w1_834) + 1, (x_w0_833) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb91, axis=(0, 1))
        do x_ax0_836 = 0, (np_particles) - 1
            x_cb92((x_ax0_836) + 1) = 0.0_c_double
            do x_rd0_837 = 0, (((o - gal) + 1)) - 1
                do x_rd1_838 = 0, ((o + 1)) - 1
                    x_cb92((x_ax0_836) + 1) = (x_cb92((x_ax0_836) + 1) + x_cb91((x_ax0_836) + 1, (x_rd1_838) + 1, &
                    &(x_rd0_837) + 1))
                end do
            end do
        end do
        do x_w0_839 = 0, (np_particles) - 1
            x_hcall13((x_w0_839) + 1) = x_cb92((x_w0_839) + 1)
        end do
        do x_w0_840 = 0, (np_particles) - 1
            Bzp((x_w0_840) + 1) = Bzp((x_w0_840) + 1) + (x_hcall13((x_w0_840) + 1))
        end do
        if (.not. allocated(x_cb93)) then
            allocate(x_cb93((o + 1)))
        else if (size(x_cb93, 1) /= ((o + 1))) then
            deallocate(x_cb93)
            allocate(x_cb93((o + 1)))
        end if
        if (.not. allocated(x_cb93)) then
            allocate(x_cb93((o + 1)))
        else if (size(x_cb93, 1) /= ((o + 1))) then
            deallocate(x_cb93)
            allocate(x_cb93((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ez)
        do x_i_841 = 0, (x_inl1_n_sx_ez) - 1
            x_cb93((x_i_841) + 1) = x_i_841
        end do
        if (.not. allocated(x_inl29_ta)) then
            allocate(x_inl29_ta((o + 1)))
        else if (size(x_inl29_ta, 1) /= ((o + 1))) then
            deallocate(x_inl29_ta)
            allocate(x_inl29_ta((o + 1)))
        end if
        do x_w0_842 = 0, (x_inl1_n_sx_ez) - 1
            x_inl29_ta((x_w0_842) + 1) = x_cb93((x_w0_842) + 1)
        end do
        if (.not. allocated(x_cb94)) then
            allocate(x_cb94(((o - gal) + 1)))
        else if (size(x_cb94, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb94)
            allocate(x_cb94(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb94)) then
            allocate(x_cb94(((o - gal) + 1)))
        else if (size(x_cb94, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb94)
            allocate(x_cb94(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ez)
        do x_i_843 = 0, (x_inl1_n_sz_ez) - 1
            x_cb94((x_i_843) + 1) = x_i_843
        end do
        if (.not. allocated(x_inl29_tb)) then
            allocate(x_inl29_tb(((o - gal) + 1)))
        else if (size(x_inl29_tb, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl29_tb)
            allocate(x_inl29_tb(((o - gal) + 1)))
        end if
        do x_w0_844 = 0, (x_inl1_n_sz_ez) - 1
            x_inl29_tb((x_w0_844) + 1) = x_cb94((x_w0_844) + 1)
        end do
        if (.not. allocated(x_inl29_ia)) then
            allocate(x_inl29_ia(np_particles, 1, (o + 1)))
        else if (size(x_inl29_ia, 1) /= (np_particles) .or. size(x_inl29_ia, 2) /= (1) .or. size(x_inl29_ia, 3) /= ((o &
        &+ 1))) then
            deallocate(x_inl29_ia)
            allocate(x_inl29_ia(np_particles, 1, (o + 1)))
        end if
        do si0_l845 = 0, ((o + 1)) - 1
            do si1_l846 = 0, (1) - 1
                do si2_l847 = 0, (np_particles) - 1
                    x_inl29_ia((si2_l847) + 1, (si1_l846) + 1, (si0_l845) + 1) = ((x_inl1_lox + x_inl1_j_ez((si2_l847) &
                    &+ 1)) + x_inl29_ta((si0_l845) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl29_ib)) then
            allocate(x_inl29_ib(np_particles, ((o - gal) + 1), 1))
        else if (size(x_inl29_ib, 1) /= (np_particles) .or. size(x_inl29_ib, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl29_ib, 3) /= (1)) then
            deallocate(x_inl29_ib)
            allocate(x_inl29_ib(np_particles, ((o - gal) + 1), 1))
        end if
        do si0_l848 = 0, (1) - 1
            do si1_l849 = 0, (((o - gal) + 1)) - 1
                do si2_l850 = 0, (np_particles) - 1
                    x_inl29_ib((si2_l850) + 1, (si1_l849) + 1, (si0_l848) + 1) = ((x_inl1_loy + x_inl1_l_ez((si2_l850) &
                    &+ 1)) + x_inl29_tb((si1_l849) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl29_ia_b)) then
            allocate(x_inl29_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl29_ia_b, 1) /= (np_particles) .or. size(x_inl29_ia_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl29_ia_b, 3) /= ((o + 1))) then
            deallocate(x_inl29_ia_b)
            allocate(x_inl29_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l851 = 0, ((o + 1)) - 1
            do si1_l852 = 0, (((o - gal) + 1)) - 1
                do si2_l853 = 0, (np_particles) - 1
                    x_inl29_ia_b((si2_l853) + 1, (si1_l852) + 1, (si0_l851) + 1) = x_inl29_ia((si2_l853) + 1, (0) + 1, &
                    &(si0_l851) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl29_ib_b)) then
            allocate(x_inl29_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl29_ib_b, 1) /= (np_particles) .or. size(x_inl29_ib_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl29_ib_b, 3) /= ((o + 1))) then
            deallocate(x_inl29_ib_b)
            allocate(x_inl29_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l854 = 0, ((o + 1)) - 1
            do si1_l855 = 0, (((o - gal) + 1)) - 1
                do si2_l856 = 0, (np_particles) - 1
                    x_inl29_ib_b((si2_l856) + 1, (si1_l855) + 1, (si0_l854) + 1) = x_inl29_ib((si2_l856) + 1, &
                    &(si1_l855) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl29_gathered)) then
            allocate(x_inl29_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl29_gathered, 1) /= (np_particles) .or. size(x_inl29_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl29_gathered, 3) /= ((o + 1))) then
            deallocate(x_inl29_gathered)
            allocate(x_inl29_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_857 = 0, ((o + 1)) - 1
            do x_w1_858 = 0, (((o - gal) + 1)) - 1
                do x_w2_859 = 0, (np_particles) - 1
                    x_inl29_gathered((x_w2_859) + 1, (x_w1_858) + 1, (x_w0_857) + 1) = ez_arr((0) + 1, (0) + 1, &
                    &(x_inl29_ib_b((x_w2_859) + 1, (x_w1_858) + 1, (x_w0_857) + 1)) + 1, (x_inl29_ia_b((x_w2_859) + 1, &
                    &(x_w1_858) + 1, (x_w0_857) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl29_weight)) then
            allocate(x_inl29_weight(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl29_weight, 1) /= (np_particles) .or. size(x_inl29_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl29_weight, 3) /= ((o + 1))) then
            deallocate(x_inl29_weight)
            allocate(x_inl29_weight(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_860 = 0, ((o + 1)) - 1
            do x_w1_861 = 0, (((o - gal) + 1)) - 1
                do x_w2_862 = 0, (np_particles) - 1
                    x_inl29_weight((x_w2_862) + 1, (x_w1_861) + 1, (x_w0_860) + 1) = (x_inl1_sx_ez((x_w2_862) + 1, &
                    &(x_w0_860) + 1) * x_inl1_sz_ez((x_w2_862) + 1, (x_w1_861) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb95)) then
            allocate(x_cb95(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_cb95, 1) /= (np_particles) .or. size(x_cb95, 2) /= (((o - gal) + 1)) .or. size(x_cb95, 3) /= &
        &((o + 1))) then
            deallocate(x_cb95)
            allocate(x_cb95(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_863 = 0, ((o + 1)) - 1
            do x_w1_864 = 0, (((o - gal) + 1)) - 1
                do x_w2_865 = 0, (np_particles) - 1
                    x_cb95((x_w2_865) + 1, (x_w1_864) + 1, (x_w0_863) + 1) = (x_inl29_weight((x_w2_865) + 1, &
                    &(x_w1_864) + 1, (x_w0_863) + 1) * x_inl29_gathered((x_w2_865) + 1, (x_w1_864) + 1, (x_w0_863) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb95, axis=(0, 1))
        do x_ax0_866 = 0, (np_particles) - 1
            x_cb96((x_ax0_866) + 1) = 0.0_c_double
            do x_rd0_867 = 0, ((o + 1)) - 1
                do x_rd1_868 = 0, (((o - gal) + 1)) - 1
                    x_cb96((x_ax0_866) + 1) = (x_cb96((x_ax0_866) + 1) + x_cb95((x_ax0_866) + 1, (x_rd1_868) + 1, &
                    &(x_rd0_867) + 1))
                end do
            end do
        end do
        do x_w0_869 = 0, (np_particles) - 1
            x_hcall14((x_w0_869) + 1) = x_cb96((x_w0_869) + 1)
        end do
        do x_w0_870 = 0, (np_particles) - 1
            Ezp((x_w0_870) + 1) = Ezp((x_w0_870) + 1) + (x_hcall14((x_w0_870) + 1))
        end do
        if (.not. allocated(x_cb97)) then
            allocate(x_cb97((o + 1)))
        else if (size(x_cb97, 1) /= ((o + 1))) then
            deallocate(x_cb97)
            allocate(x_cb97((o + 1)))
        end if
        if (.not. allocated(x_cb97)) then
            allocate(x_cb97((o + 1)))
        else if (size(x_cb97, 1) /= ((o + 1))) then
            deallocate(x_cb97)
            allocate(x_cb97((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bx)
        do x_i_871 = 0, (x_inl1_n_sx_bx) - 1
            x_cb97((x_i_871) + 1) = x_i_871
        end do
        if (.not. allocated(x_inl30_ta)) then
            allocate(x_inl30_ta((o + 1)))
        else if (size(x_inl30_ta, 1) /= ((o + 1))) then
            deallocate(x_inl30_ta)
            allocate(x_inl30_ta((o + 1)))
        end if
        do x_w0_872 = 0, (x_inl1_n_sx_bx) - 1
            x_inl30_ta((x_w0_872) + 1) = x_cb97((x_w0_872) + 1)
        end do
        if (.not. allocated(x_cb98)) then
            allocate(x_cb98(((o - gal) + 1)))
        else if (size(x_cb98, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb98)
            allocate(x_cb98(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb98)) then
            allocate(x_cb98(((o - gal) + 1)))
        else if (size(x_cb98, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb98)
            allocate(x_cb98(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bx)
        do x_i_873 = 0, (x_inl1_n_sz_bx) - 1
            x_cb98((x_i_873) + 1) = x_i_873
        end do
        if (.not. allocated(x_inl30_tb)) then
            allocate(x_inl30_tb(((o - gal) + 1)))
        else if (size(x_inl30_tb, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl30_tb)
            allocate(x_inl30_tb(((o - gal) + 1)))
        end if
        do x_w0_874 = 0, (x_inl1_n_sz_bx) - 1
            x_inl30_tb((x_w0_874) + 1) = x_cb98((x_w0_874) + 1)
        end do
        if (.not. allocated(x_inl30_ia)) then
            allocate(x_inl30_ia(np_particles, 1, (o + 1)))
        else if (size(x_inl30_ia, 1) /= (np_particles) .or. size(x_inl30_ia, 2) /= (1) .or. size(x_inl30_ia, 3) /= ((o &
        &+ 1))) then
            deallocate(x_inl30_ia)
            allocate(x_inl30_ia(np_particles, 1, (o + 1)))
        end if
        do si0_l875 = 0, ((o + 1)) - 1
            do si1_l876 = 0, (1) - 1
                do si2_l877 = 0, (np_particles) - 1
                    x_inl30_ia((si2_l877) + 1, (si1_l876) + 1, (si0_l875) + 1) = ((x_inl1_lox + x_inl1_j_bx((si2_l877) &
                    &+ 1)) + x_inl30_ta((si0_l875) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl30_ib)) then
            allocate(x_inl30_ib(np_particles, ((o - gal) + 1), 1))
        else if (size(x_inl30_ib, 1) /= (np_particles) .or. size(x_inl30_ib, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl30_ib, 3) /= (1)) then
            deallocate(x_inl30_ib)
            allocate(x_inl30_ib(np_particles, ((o - gal) + 1), 1))
        end if
        do si0_l878 = 0, (1) - 1
            do si1_l879 = 0, (((o - gal) + 1)) - 1
                do si2_l880 = 0, (np_particles) - 1
                    x_inl30_ib((si2_l880) + 1, (si1_l879) + 1, (si0_l878) + 1) = ((x_inl1_loy + x_inl1_l_bx((si2_l880) &
                    &+ 1)) + x_inl30_tb((si1_l879) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl30_ia_b)) then
            allocate(x_inl30_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl30_ia_b, 1) /= (np_particles) .or. size(x_inl30_ia_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl30_ia_b, 3) /= ((o + 1))) then
            deallocate(x_inl30_ia_b)
            allocate(x_inl30_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l881 = 0, ((o + 1)) - 1
            do si1_l882 = 0, (((o - gal) + 1)) - 1
                do si2_l883 = 0, (np_particles) - 1
                    x_inl30_ia_b((si2_l883) + 1, (si1_l882) + 1, (si0_l881) + 1) = x_inl30_ia((si2_l883) + 1, (0) + 1, &
                    &(si0_l881) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl30_ib_b)) then
            allocate(x_inl30_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl30_ib_b, 1) /= (np_particles) .or. size(x_inl30_ib_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl30_ib_b, 3) /= ((o + 1))) then
            deallocate(x_inl30_ib_b)
            allocate(x_inl30_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do si0_l884 = 0, ((o + 1)) - 1
            do si1_l885 = 0, (((o - gal) + 1)) - 1
                do si2_l886 = 0, (np_particles) - 1
                    x_inl30_ib_b((si2_l886) + 1, (si1_l885) + 1, (si0_l884) + 1) = x_inl30_ib((si2_l886) + 1, &
                    &(si1_l885) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl30_gathered)) then
            allocate(x_inl30_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl30_gathered, 1) /= (np_particles) .or. size(x_inl30_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl30_gathered, 3) /= ((o + 1))) then
            deallocate(x_inl30_gathered)
            allocate(x_inl30_gathered(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_887 = 0, ((o + 1)) - 1
            do x_w1_888 = 0, (((o - gal) + 1)) - 1
                do x_w2_889 = 0, (np_particles) - 1
                    x_inl30_gathered((x_w2_889) + 1, (x_w1_888) + 1, (x_w0_887) + 1) = bx_arr((0) + 1, (0) + 1, &
                    &(x_inl30_ib_b((x_w2_889) + 1, (x_w1_888) + 1, (x_w0_887) + 1)) + 1, (x_inl30_ia_b((x_w2_889) + 1, &
                    &(x_w1_888) + 1, (x_w0_887) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl30_weight)) then
            allocate(x_inl30_weight(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_inl30_weight, 1) /= (np_particles) .or. size(x_inl30_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl30_weight, 3) /= ((o + 1))) then
            deallocate(x_inl30_weight)
            allocate(x_inl30_weight(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_890 = 0, ((o + 1)) - 1
            do x_w1_891 = 0, (((o - gal) + 1)) - 1
                do x_w2_892 = 0, (np_particles) - 1
                    x_inl30_weight((x_w2_892) + 1, (x_w1_891) + 1, (x_w0_890) + 1) = (x_inl1_sx_bx((x_w2_892) + 1, &
                    &(x_w0_890) + 1) * x_inl1_sz_bx((x_w2_892) + 1, (x_w1_891) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb99)) then
            allocate(x_cb99(np_particles, ((o - gal) + 1), (o + 1)))
        else if (size(x_cb99, 1) /= (np_particles) .or. size(x_cb99, 2) /= (((o - gal) + 1)) .or. size(x_cb99, 3) /= &
        &((o + 1))) then
            deallocate(x_cb99)
            allocate(x_cb99(np_particles, ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_893 = 0, ((o + 1)) - 1
            do x_w1_894 = 0, (((o - gal) + 1)) - 1
                do x_w2_895 = 0, (np_particles) - 1
                    x_cb99((x_w2_895) + 1, (x_w1_894) + 1, (x_w0_893) + 1) = (x_inl30_weight((x_w2_895) + 1, &
                    &(x_w1_894) + 1, (x_w0_893) + 1) * x_inl30_gathered((x_w2_895) + 1, (x_w1_894) + 1, (x_w0_893) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb99, axis=(0, 1))
        do x_ax0_896 = 0, (np_particles) - 1
            x_cb100((x_ax0_896) + 1) = 0.0_c_double
            do x_rd0_897 = 0, ((o + 1)) - 1
                do x_rd1_898 = 0, (((o - gal) + 1)) - 1
                    x_cb100((x_ax0_896) + 1) = (x_cb100((x_ax0_896) + 1) + x_cb99((x_ax0_896) + 1, (x_rd1_898) + 1, &
                    &(x_rd0_897) + 1))
                end do
            end do
        end do
        do x_w0_899 = 0, (np_particles) - 1
            x_inl1_Brp((x_w0_899) + 1) = x_cb100((x_w0_899) + 1)
        end do
        if (.not. allocated(x_cb101)) then
            allocate(x_cb101(((o - gal) + 1)))
        else if (size(x_cb101, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb101)
            allocate(x_cb101(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb101)) then
            allocate(x_cb101(((o - gal) + 1)))
        else if (size(x_cb101, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb101)
            allocate(x_cb101(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_by)
        do x_i_900 = 0, (x_inl1_n_sx_by) - 1
            x_cb101((x_i_900) + 1) = x_i_900
        end do
        if (.not. allocated(x_inl31_ta)) then
            allocate(x_inl31_ta(((o - gal) + 1)))
        else if (size(x_inl31_ta, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl31_ta)
            allocate(x_inl31_ta(((o - gal) + 1)))
        end if
        do x_w0_901 = 0, (x_inl1_n_sx_by) - 1
            x_inl31_ta((x_w0_901) + 1) = x_cb101((x_w0_901) + 1)
        end do
        if (.not. allocated(x_cb102)) then
            allocate(x_cb102(((o - gal) + 1)))
        else if (size(x_cb102, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb102)
            allocate(x_cb102(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb102)) then
            allocate(x_cb102(((o - gal) + 1)))
        else if (size(x_cb102, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb102)
            allocate(x_cb102(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_by)
        do x_i_902 = 0, (x_inl1_n_sz_by) - 1
            x_cb102((x_i_902) + 1) = x_i_902
        end do
        if (.not. allocated(x_inl31_tb)) then
            allocate(x_inl31_tb(((o - gal) + 1)))
        else if (size(x_inl31_tb, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl31_tb)
            allocate(x_inl31_tb(((o - gal) + 1)))
        end if
        do x_w0_903 = 0, (x_inl1_n_sz_by) - 1
            x_inl31_tb((x_w0_903) + 1) = x_cb102((x_w0_903) + 1)
        end do
        if (.not. allocated(x_inl31_ia)) then
            allocate(x_inl31_ia(np_particles, 1, ((o - gal) + 1)))
        else if (size(x_inl31_ia, 1) /= (np_particles) .or. size(x_inl31_ia, 2) /= (1) .or. size(x_inl31_ia, 3) /= &
        &(((o - gal) + 1))) then
            deallocate(x_inl31_ia)
            allocate(x_inl31_ia(np_particles, 1, ((o - gal) + 1)))
        end if
        do si0_l904 = 0, (((o - gal) + 1)) - 1
            do si1_l905 = 0, (1) - 1
                do si2_l906 = 0, (np_particles) - 1
                    x_inl31_ia((si2_l906) + 1, (si1_l905) + 1, (si0_l904) + 1) = ((x_inl1_lox + x_inl1_j_by((si2_l906) &
                    &+ 1)) + x_inl31_ta((si0_l904) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl31_ib)) then
            allocate(x_inl31_ib(np_particles, ((o - gal) + 1), 1))
        else if (size(x_inl31_ib, 1) /= (np_particles) .or. size(x_inl31_ib, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl31_ib, 3) /= (1)) then
            deallocate(x_inl31_ib)
            allocate(x_inl31_ib(np_particles, ((o - gal) + 1), 1))
        end if
        do si0_l907 = 0, (1) - 1
            do si1_l908 = 0, (((o - gal) + 1)) - 1
                do si2_l909 = 0, (np_particles) - 1
                    x_inl31_ib((si2_l909) + 1, (si1_l908) + 1, (si0_l907) + 1) = ((x_inl1_loy + x_inl1_l_by((si2_l909) &
                    &+ 1)) + x_inl31_tb((si1_l908) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_inl31_ia_b)) then
            allocate(x_inl31_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl31_ia_b, 1) /= (np_particles) .or. size(x_inl31_ia_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl31_ia_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl31_ia_b)
            allocate(x_inl31_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l910 = 0, (((o - gal) + 1)) - 1
            do si1_l911 = 0, (((o - gal) + 1)) - 1
                do si2_l912 = 0, (np_particles) - 1
                    x_inl31_ia_b((si2_l912) + 1, (si1_l911) + 1, (si0_l910) + 1) = x_inl31_ia((si2_l912) + 1, (0) + 1, &
                    &(si0_l910) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl31_ib_b)) then
            allocate(x_inl31_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl31_ib_b, 1) /= (np_particles) .or. size(x_inl31_ib_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl31_ib_b, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl31_ib_b)
            allocate(x_inl31_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l913 = 0, (((o - gal) + 1)) - 1
            do si1_l914 = 0, (((o - gal) + 1)) - 1
                do si2_l915 = 0, (np_particles) - 1
                    x_inl31_ib_b((si2_l915) + 1, (si1_l914) + 1, (si0_l913) + 1) = x_inl31_ib((si2_l915) + 1, &
                    &(si1_l914) + 1, (0) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl31_gathered)) then
            allocate(x_inl31_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl31_gathered, 1) /= (np_particles) .or. size(x_inl31_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl31_gathered, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl31_gathered)
            allocate(x_inl31_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_916 = 0, (((o - gal) + 1)) - 1
            do x_w1_917 = 0, (((o - gal) + 1)) - 1
                do x_w2_918 = 0, (np_particles) - 1
                    x_inl31_gathered((x_w2_918) + 1, (x_w1_917) + 1, (x_w0_916) + 1) = by_arr((0) + 1, (0) + 1, &
                    &(x_inl31_ib_b((x_w2_918) + 1, (x_w1_917) + 1, (x_w0_916) + 1)) + 1, (x_inl31_ia_b((x_w2_918) + 1, &
                    &(x_w1_917) + 1, (x_w0_916) + 1)) + 1)
                end do
            end do
        end do
        if (.not. allocated(x_inl31_weight)) then
            allocate(x_inl31_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl31_weight, 1) /= (np_particles) .or. size(x_inl31_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl31_weight, 3) /= (((o - gal) + 1))) then
            deallocate(x_inl31_weight)
            allocate(x_inl31_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_919 = 0, (((o - gal) + 1)) - 1
            do x_w1_920 = 0, (((o - gal) + 1)) - 1
                do x_w2_921 = 0, (np_particles) - 1
                    x_inl31_weight((x_w2_921) + 1, (x_w1_920) + 1, (x_w0_919) + 1) = (x_inl1_sx_by((x_w2_921) + 1, &
                    &(x_w0_919) + 1) * x_inl1_sz_by((x_w2_921) + 1, (x_w1_920) + 1))
                end do
            end do
        end do
        if (.not. allocated(x_cb103)) then
            allocate(x_cb103(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_cb103, 1) /= (np_particles) .or. size(x_cb103, 2) /= (((o - gal) + 1)) .or. size(x_cb103, 3) &
        &/= (((o - gal) + 1))) then
            deallocate(x_cb103)
            allocate(x_cb103(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_922 = 0, (((o - gal) + 1)) - 1
            do x_w1_923 = 0, (((o - gal) + 1)) - 1
                do x_w2_924 = 0, (np_particles) - 1
                    x_cb103((x_w2_924) + 1, (x_w1_923) + 1, (x_w0_922) + 1) = (x_inl31_weight((x_w2_924) + 1, &
                    &(x_w1_923) + 1, (x_w0_922) + 1) * x_inl31_gathered((x_w2_924) + 1, (x_w1_923) + 1, (x_w0_922) + 1))
                end do
            end do
        end do
        ! numpy: np.sum(__cb103, axis=(0, 1))
        do x_ax0_925 = 0, (np_particles) - 1
            x_cb104((x_ax0_925) + 1) = 0.0_c_double
            do x_rd0_926 = 0, (((o - gal) + 1)) - 1
                do x_rd1_927 = 0, (((o - gal) + 1)) - 1
                    x_cb104((x_ax0_925) + 1) = (x_cb104((x_ax0_925) + 1) + x_cb103((x_ax0_925) + 1, (x_rd1_927) + 1, &
                    &(x_rd0_926) + 1))
                end do
            end do
        end do
        do x_w0_928 = 0, (np_particles) - 1
            x_inl1_Bthetap((x_w0_928) + 1) = x_cb104((x_w0_928) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, __inl1_rp, 1.0)
        do x_r0_929 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_929) + 1) > 0.0_c_double)) then
                x_ifexp36 = x_inl1_rp((x_r0_929) + 1)
            else
                x_ifexp36 = 1.0_c_double
            end if
            x_cb105((x_r0_929) + 1) = x_ifexp36
        end do
        do x_w0_930 = 0, (np_particles) - 1
            x_inl1_rp_safe((x_w0_930) + 1) = x_cb105((x_w0_930) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, xp / __inl1_rp_safe, 1.0)
        do x_r0_931 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_931) + 1) > 0.0_c_double)) then
                x_ifexp37 = (xp((x_r0_931) + 1) / x_inl1_rp_safe((x_r0_931) + 1))
            else
                x_ifexp37 = 1.0_c_double
            end if
            x_cb106((x_r0_931) + 1) = x_ifexp37
        end do
        do x_w0_932 = 0, (np_particles) - 1
            x_inl1_costheta((x_w0_932) + 1) = x_cb106((x_w0_932) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, yp / __inl1_rp_safe, 0.0)
        do x_r0_933 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_933) + 1) > 0.0_c_double)) then
                x_ifexp38 = (yp((x_r0_933) + 1) / x_inl1_rp_safe((x_r0_933) + 1))
            else
                x_ifexp38 = 0.0_c_double
            end if
            x_cb107((x_r0_933) + 1) = x_ifexp38
        end do
        do x_w0_934 = 0, (np_particles) - 1
            x_inl1_sintheta((x_w0_934) + 1) = x_cb107((x_w0_934) + 1)
        end do
        do x_w0_935 = 0, (np_particles) - 1
            x_inl1_xy0_re((x_w0_935) + 1) = x_inl1_costheta((x_w0_935) + 1)
        end do
        do x_w0_936 = 0, (np_particles) - 1
            x_inl1_xy0_im((x_w0_936) + 1) = (-(x_inl1_sintheta((x_w0_936) + 1)))
        end do
        do x_w0_937 = 0, (np_particles) - 1
            x_inl1_xy_re((x_w0_937) + 1) = x_inl1_xy0_re((x_w0_937) + 1)
        end do
        do x_w0_938 = 0, (np_particles) - 1
            x_inl1_xy_im((x_w0_938) + 1) = x_inl1_xy0_im((x_w0_938) + 1)
        end do
        do x_inl1_imode_939 = 1, (nmodes) - 1
            if (.not. allocated(x_cb108)) then
                allocate(x_cb108((o + 1)))
            else if (size(x_cb108, 1) /= ((o + 1))) then
                deallocate(x_cb108)
                allocate(x_cb108((o + 1)))
            end if
            if (.not. allocated(x_cb108)) then
                allocate(x_cb108((o + 1)))
            else if (size(x_cb108, 1) /= ((o + 1))) then
                deallocate(x_cb108)
                allocate(x_cb108((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_ey)
            do x_i_940 = 0, (x_inl1_n_sx_ey) - 1
                x_cb108((x_i_940) + 1) = x_i_940
            end do
            if (.not. allocated(x_inl32_ta)) then
                allocate(x_inl32_ta((o + 1)))
            else if (size(x_inl32_ta, 1) /= ((o + 1))) then
                deallocate(x_inl32_ta)
                allocate(x_inl32_ta((o + 1)))
            end if
            do x_w0_941 = 0, (x_inl1_n_sx_ey) - 1
                x_inl32_ta((x_w0_941) + 1) = x_cb108((x_w0_941) + 1)
            end do
            if (.not. allocated(x_cb109)) then
                allocate(x_cb109((o + 1)))
            else if (size(x_cb109, 1) /= ((o + 1))) then
                deallocate(x_cb109)
                allocate(x_cb109((o + 1)))
            end if
            if (.not. allocated(x_cb109)) then
                allocate(x_cb109((o + 1)))
            else if (size(x_cb109, 1) /= ((o + 1))) then
                deallocate(x_cb109)
                allocate(x_cb109((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_ey)
            do x_i_942 = 0, (x_inl1_n_sz_ey) - 1
                x_cb109((x_i_942) + 1) = x_i_942
            end do
            if (.not. allocated(x_inl32_tb)) then
                allocate(x_inl32_tb((o + 1)))
            else if (size(x_inl32_tb, 1) /= ((o + 1))) then
                deallocate(x_inl32_tb)
                allocate(x_inl32_tb((o + 1)))
            end if
            do x_w0_943 = 0, (x_inl1_n_sz_ey) - 1
                x_inl32_tb((x_w0_943) + 1) = x_cb109((x_w0_943) + 1)
            end do
            if (.not. allocated(x_inl32_ia)) then
                allocate(x_inl32_ia(np_particles, 1, (o + 1)))
            else if (size(x_inl32_ia, 1) /= (np_particles) .or. size(x_inl32_ia, 2) /= (1) .or. size(x_inl32_ia, 3) /= &
            &((o + 1))) then
                deallocate(x_inl32_ia)
                allocate(x_inl32_ia(np_particles, 1, (o + 1)))
            end if
            do si0_l944 = 0, ((o + 1)) - 1
                do si1_l945 = 0, (1) - 1
                    do si2_l946 = 0, (np_particles) - 1
                        x_inl32_ia((si2_l946) + 1, (si1_l945) + 1, (si0_l944) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_ey((si2_l946) + 1)) + x_inl32_ta((si0_l944) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl32_ib)) then
                allocate(x_inl32_ib(np_particles, (o + 1), 1))
            else if (size(x_inl32_ib, 1) /= (np_particles) .or. size(x_inl32_ib, 2) /= ((o + 1)) .or. size(x_inl32_ib, &
            &3) /= (1)) then
                deallocate(x_inl32_ib)
                allocate(x_inl32_ib(np_particles, (o + 1), 1))
            end if
            do si0_l947 = 0, (1) - 1
                do si1_l948 = 0, ((o + 1)) - 1
                    do si2_l949 = 0, (np_particles) - 1
                        x_inl32_ib((si2_l949) + 1, (si1_l948) + 1, (si0_l947) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_ey((si2_l949) + 1)) + x_inl32_tb((si1_l948) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl32_ia_b)) then
                allocate(x_inl32_ia_b(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl32_ia_b, 1) /= (np_particles) .or. size(x_inl32_ia_b, 2) /= ((o + 1)) .or. &
            &size(x_inl32_ia_b, 3) /= ((o + 1))) then
                deallocate(x_inl32_ia_b)
                allocate(x_inl32_ia_b(np_particles, (o + 1), (o + 1)))
            end if
            do si0_l950 = 0, ((o + 1)) - 1
                do si1_l951 = 0, ((o + 1)) - 1
                    do si2_l952 = 0, (np_particles) - 1
                        x_inl32_ia_b((si2_l952) + 1, (si1_l951) + 1, (si0_l950) + 1) = x_inl32_ia((si2_l952) + 1, (0) &
                        &+ 1, (si0_l950) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl32_ib_b)) then
                allocate(x_inl32_ib_b(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl32_ib_b, 1) /= (np_particles) .or. size(x_inl32_ib_b, 2) /= ((o + 1)) .or. &
            &size(x_inl32_ib_b, 3) /= ((o + 1))) then
                deallocate(x_inl32_ib_b)
                allocate(x_inl32_ib_b(np_particles, (o + 1), (o + 1)))
            end if
            do si0_l953 = 0, ((o + 1)) - 1
                do si1_l954 = 0, ((o + 1)) - 1
                    do si2_l955 = 0, (np_particles) - 1
                        x_inl32_ib_b((si2_l955) + 1, (si1_l954) + 1, (si0_l953) + 1) = x_inl32_ib((si2_l955) + 1, &
                        &(si1_l954) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl32_gathered)) then
                allocate(x_inl32_gathered(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl32_gathered, 1) /= (np_particles) .or. size(x_inl32_gathered, 2) /= ((o + 1)) .or. &
            &size(x_inl32_gathered, 3) /= ((o + 1))) then
                deallocate(x_inl32_gathered)
                allocate(x_inl32_gathered(np_particles, (o + 1), (o + 1)))
            end if
            do x_w0_956 = 0, ((o + 1)) - 1
                do x_w1_957 = 0, ((o + 1)) - 1
                    do x_w2_958 = 0, (np_particles) - 1
                        x_inl32_gathered((x_w2_958) + 1, (x_w1_957) + 1, (x_w0_956) + 1) = ey_arr((((2 * &
                        &x_inl1_imode_939) - 1)) + 1, (0) + 1, (x_inl32_ib_b((x_w2_958) + 1, (x_w1_957) + 1, &
                        &(x_w0_956) + 1)) + 1, (x_inl32_ia_b((x_w2_958) + 1, (x_w1_957) + 1, (x_w0_956) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl32_weight)) then
                allocate(x_inl32_weight(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl32_weight, 1) /= (np_particles) .or. size(x_inl32_weight, 2) /= ((o + 1)) .or. &
            &size(x_inl32_weight, 3) /= ((o + 1))) then
                deallocate(x_inl32_weight)
                allocate(x_inl32_weight(np_particles, (o + 1), (o + 1)))
            end if
            do x_w0_959 = 0, ((o + 1)) - 1
                do x_w1_960 = 0, ((o + 1)) - 1
                    do x_w2_961 = 0, (np_particles) - 1
                        x_inl32_weight((x_w2_961) + 1, (x_w1_960) + 1, (x_w0_959) + 1) = (x_inl1_sx_ey((x_w2_961) + 1, &
                        &(x_w0_959) + 1) * x_inl1_sz_ey((x_w2_961) + 1, (x_w1_960) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb110)) then
                allocate(x_cb110(np_particles, (o + 1), (o + 1)))
            else if (size(x_cb110, 1) /= (np_particles) .or. size(x_cb110, 2) /= ((o + 1)) .or. size(x_cb110, 3) /= &
            &((o + 1))) then
                deallocate(x_cb110)
                allocate(x_cb110(np_particles, (o + 1), (o + 1)))
            end if
            do x_w0_962 = 0, ((o + 1)) - 1
                do x_w1_963 = 0, ((o + 1)) - 1
                    do x_w2_964 = 0, (np_particles) - 1
                        x_cb110((x_w2_964) + 1, (x_w1_963) + 1, (x_w0_962) + 1) = (x_inl32_weight((x_w2_964) + 1, &
                        &(x_w1_963) + 1, (x_w0_962) + 1) * x_inl32_gathered((x_w2_964) + 1, (x_w1_963) + 1, (x_w0_962) &
                        &+ 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb110, axis=(0, 1))
            do x_ax0_965 = 0, (np_particles) - 1
                x_cb111((x_ax0_965) + 1) = 0.0_c_double
                do x_rd0_966 = 0, ((o + 1)) - 1
                    do x_rd1_967 = 0, ((o + 1)) - 1
                        x_cb111((x_ax0_965) + 1) = (x_cb111((x_ax0_965) + 1) + x_cb110((x_ax0_965) + 1, (x_rd1_967) + &
                        &1, (x_rd0_966) + 1))
                    end do
                end do
            end do
            do x_w0_968 = 0, (np_particles) - 1
                x_hcall15((x_w0_968) + 1) = x_cb111((x_w0_968) + 1)
            end do
            if (.not. allocated(x_cb112)) then
                allocate(x_cb112((o + 1)))
            else if (size(x_cb112, 1) /= ((o + 1))) then
                deallocate(x_cb112)
                allocate(x_cb112((o + 1)))
            end if
            if (.not. allocated(x_cb112)) then
                allocate(x_cb112((o + 1)))
            else if (size(x_cb112, 1) /= ((o + 1))) then
                deallocate(x_cb112)
                allocate(x_cb112((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_ey)
            do x_i_969 = 0, (x_inl1_n_sx_ey) - 1
                x_cb112((x_i_969) + 1) = x_i_969
            end do
            if (.not. allocated(x_inl33_ta)) then
                allocate(x_inl33_ta((o + 1)))
            else if (size(x_inl33_ta, 1) /= ((o + 1))) then
                deallocate(x_inl33_ta)
                allocate(x_inl33_ta((o + 1)))
            end if
            do x_w0_970 = 0, (x_inl1_n_sx_ey) - 1
                x_inl33_ta((x_w0_970) + 1) = x_cb112((x_w0_970) + 1)
            end do
            if (.not. allocated(x_cb113)) then
                allocate(x_cb113((o + 1)))
            else if (size(x_cb113, 1) /= ((o + 1))) then
                deallocate(x_cb113)
                allocate(x_cb113((o + 1)))
            end if
            if (.not. allocated(x_cb113)) then
                allocate(x_cb113((o + 1)))
            else if (size(x_cb113, 1) /= ((o + 1))) then
                deallocate(x_cb113)
                allocate(x_cb113((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_ey)
            do x_i_971 = 0, (x_inl1_n_sz_ey) - 1
                x_cb113((x_i_971) + 1) = x_i_971
            end do
            if (.not. allocated(x_inl33_tb)) then
                allocate(x_inl33_tb((o + 1)))
            else if (size(x_inl33_tb, 1) /= ((o + 1))) then
                deallocate(x_inl33_tb)
                allocate(x_inl33_tb((o + 1)))
            end if
            do x_w0_972 = 0, (x_inl1_n_sz_ey) - 1
                x_inl33_tb((x_w0_972) + 1) = x_cb113((x_w0_972) + 1)
            end do
            if (.not. allocated(x_inl33_ia)) then
                allocate(x_inl33_ia(np_particles, 1, (o + 1)))
            else if (size(x_inl33_ia, 1) /= (np_particles) .or. size(x_inl33_ia, 2) /= (1) .or. size(x_inl33_ia, 3) /= &
            &((o + 1))) then
                deallocate(x_inl33_ia)
                allocate(x_inl33_ia(np_particles, 1, (o + 1)))
            end if
            do si0_l973 = 0, ((o + 1)) - 1
                do si1_l974 = 0, (1) - 1
                    do si2_l975 = 0, (np_particles) - 1
                        x_inl33_ia((si2_l975) + 1, (si1_l974) + 1, (si0_l973) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_ey((si2_l975) + 1)) + x_inl33_ta((si0_l973) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl33_ib)) then
                allocate(x_inl33_ib(np_particles, (o + 1), 1))
            else if (size(x_inl33_ib, 1) /= (np_particles) .or. size(x_inl33_ib, 2) /= ((o + 1)) .or. size(x_inl33_ib, &
            &3) /= (1)) then
                deallocate(x_inl33_ib)
                allocate(x_inl33_ib(np_particles, (o + 1), 1))
            end if
            do si0_l976 = 0, (1) - 1
                do si1_l977 = 0, ((o + 1)) - 1
                    do si2_l978 = 0, (np_particles) - 1
                        x_inl33_ib((si2_l978) + 1, (si1_l977) + 1, (si0_l976) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_ey((si2_l978) + 1)) + x_inl33_tb((si1_l977) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl33_ia_b)) then
                allocate(x_inl33_ia_b(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl33_ia_b, 1) /= (np_particles) .or. size(x_inl33_ia_b, 2) /= ((o + 1)) .or. &
            &size(x_inl33_ia_b, 3) /= ((o + 1))) then
                deallocate(x_inl33_ia_b)
                allocate(x_inl33_ia_b(np_particles, (o + 1), (o + 1)))
            end if
            do si0_l979 = 0, ((o + 1)) - 1
                do si1_l980 = 0, ((o + 1)) - 1
                    do si2_l981 = 0, (np_particles) - 1
                        x_inl33_ia_b((si2_l981) + 1, (si1_l980) + 1, (si0_l979) + 1) = x_inl33_ia((si2_l981) + 1, (0) &
                        &+ 1, (si0_l979) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl33_ib_b)) then
                allocate(x_inl33_ib_b(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl33_ib_b, 1) /= (np_particles) .or. size(x_inl33_ib_b, 2) /= ((o + 1)) .or. &
            &size(x_inl33_ib_b, 3) /= ((o + 1))) then
                deallocate(x_inl33_ib_b)
                allocate(x_inl33_ib_b(np_particles, (o + 1), (o + 1)))
            end if
            do si0_l982 = 0, ((o + 1)) - 1
                do si1_l983 = 0, ((o + 1)) - 1
                    do si2_l984 = 0, (np_particles) - 1
                        x_inl33_ib_b((si2_l984) + 1, (si1_l983) + 1, (si0_l982) + 1) = x_inl33_ib((si2_l984) + 1, &
                        &(si1_l983) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl33_gathered)) then
                allocate(x_inl33_gathered(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl33_gathered, 1) /= (np_particles) .or. size(x_inl33_gathered, 2) /= ((o + 1)) .or. &
            &size(x_inl33_gathered, 3) /= ((o + 1))) then
                deallocate(x_inl33_gathered)
                allocate(x_inl33_gathered(np_particles, (o + 1), (o + 1)))
            end if
            do x_w0_985 = 0, ((o + 1)) - 1
                do x_w1_986 = 0, ((o + 1)) - 1
                    do x_w2_987 = 0, (np_particles) - 1
                        x_inl33_gathered((x_w2_987) + 1, (x_w1_986) + 1, (x_w0_985) + 1) = ey_arr(((2 * &
                        &x_inl1_imode_939)) + 1, (0) + 1, (x_inl33_ib_b((x_w2_987) + 1, (x_w1_986) + 1, (x_w0_985) + &
                        &1)) + 1, (x_inl33_ia_b((x_w2_987) + 1, (x_w1_986) + 1, (x_w0_985) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl33_weight)) then
                allocate(x_inl33_weight(np_particles, (o + 1), (o + 1)))
            else if (size(x_inl33_weight, 1) /= (np_particles) .or. size(x_inl33_weight, 2) /= ((o + 1)) .or. &
            &size(x_inl33_weight, 3) /= ((o + 1))) then
                deallocate(x_inl33_weight)
                allocate(x_inl33_weight(np_particles, (o + 1), (o + 1)))
            end if
            do x_w0_988 = 0, ((o + 1)) - 1
                do x_w1_989 = 0, ((o + 1)) - 1
                    do x_w2_990 = 0, (np_particles) - 1
                        x_inl33_weight((x_w2_990) + 1, (x_w1_989) + 1, (x_w0_988) + 1) = (x_inl1_sx_ey((x_w2_990) + 1, &
                        &(x_w0_988) + 1) * x_inl1_sz_ey((x_w2_990) + 1, (x_w1_989) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb114)) then
                allocate(x_cb114(np_particles, (o + 1), (o + 1)))
            else if (size(x_cb114, 1) /= (np_particles) .or. size(x_cb114, 2) /= ((o + 1)) .or. size(x_cb114, 3) /= &
            &((o + 1))) then
                deallocate(x_cb114)
                allocate(x_cb114(np_particles, (o + 1), (o + 1)))
            end if
            do x_w0_991 = 0, ((o + 1)) - 1
                do x_w1_992 = 0, ((o + 1)) - 1
                    do x_w2_993 = 0, (np_particles) - 1
                        x_cb114((x_w2_993) + 1, (x_w1_992) + 1, (x_w0_991) + 1) = (x_inl33_weight((x_w2_993) + 1, &
                        &(x_w1_992) + 1, (x_w0_991) + 1) * x_inl33_gathered((x_w2_993) + 1, (x_w1_992) + 1, (x_w0_991) &
                        &+ 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb114, axis=(0, 1))
            do x_ax0_994 = 0, (np_particles) - 1
                x_cb115((x_ax0_994) + 1) = 0.0_c_double
                do x_rd0_995 = 0, ((o + 1)) - 1
                    do x_rd1_996 = 0, ((o + 1)) - 1
                        x_cb115((x_ax0_994) + 1) = (x_cb115((x_ax0_994) + 1) + x_cb114((x_ax0_994) + 1, (x_rd1_996) + &
                        &1, (x_rd0_995) + 1))
                    end do
                end do
            end do
            do x_w0_997 = 0, (np_particles) - 1
                x_hcall16((x_w0_997) + 1) = x_cb115((x_w0_997) + 1)
            end do
            do x_w0_998 = 0, (np_particles) - 1
                x_inl1_dEy((x_w0_998) + 1) = ((x_inl1_xy_re((x_w0_998) + 1) * x_hcall15((x_w0_998) + 1)) - &
                &(x_inl1_xy_im((x_w0_998) + 1) * x_hcall16((x_w0_998) + 1)))
            end do
            do x_w0_999 = 0, (np_particles) - 1
                x_inl1_Ethetap((x_w0_999) + 1) = x_inl1_Ethetap((x_w0_999) + 1) + (x_inl1_dEy((x_w0_999) + 1))
            end do
            if (.not. allocated(x_cb116)) then
                allocate(x_cb116(((o - gal) + 1)))
            else if (size(x_cb116, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb116)
                allocate(x_cb116(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb116)) then
                allocate(x_cb116(((o - gal) + 1)))
            else if (size(x_cb116, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb116)
                allocate(x_cb116(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_ex)
            do x_i_1000 = 0, (x_inl1_n_sx_ex) - 1
                x_cb116((x_i_1000) + 1) = x_i_1000
            end do
            if (.not. allocated(x_inl34_ta)) then
                allocate(x_inl34_ta(((o - gal) + 1)))
            else if (size(x_inl34_ta, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl34_ta)
                allocate(x_inl34_ta(((o - gal) + 1)))
            end if
            do x_w0_1001 = 0, (x_inl1_n_sx_ex) - 1
                x_inl34_ta((x_w0_1001) + 1) = x_cb116((x_w0_1001) + 1)
            end do
            if (.not. allocated(x_cb117)) then
                allocate(x_cb117((o + 1)))
            else if (size(x_cb117, 1) /= ((o + 1))) then
                deallocate(x_cb117)
                allocate(x_cb117((o + 1)))
            end if
            if (.not. allocated(x_cb117)) then
                allocate(x_cb117((o + 1)))
            else if (size(x_cb117, 1) /= ((o + 1))) then
                deallocate(x_cb117)
                allocate(x_cb117((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_ex)
            do x_i_1002 = 0, (x_inl1_n_sz_ex) - 1
                x_cb117((x_i_1002) + 1) = x_i_1002
            end do
            if (.not. allocated(x_inl34_tb)) then
                allocate(x_inl34_tb((o + 1)))
            else if (size(x_inl34_tb, 1) /= ((o + 1))) then
                deallocate(x_inl34_tb)
                allocate(x_inl34_tb((o + 1)))
            end if
            do x_w0_1003 = 0, (x_inl1_n_sz_ex) - 1
                x_inl34_tb((x_w0_1003) + 1) = x_cb117((x_w0_1003) + 1)
            end do
            if (.not. allocated(x_inl34_ia)) then
                allocate(x_inl34_ia(np_particles, 1, ((o - gal) + 1)))
            else if (size(x_inl34_ia, 1) /= (np_particles) .or. size(x_inl34_ia, 2) /= (1) .or. size(x_inl34_ia, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_inl34_ia)
                allocate(x_inl34_ia(np_particles, 1, ((o - gal) + 1)))
            end if
            do si0_l1004 = 0, (((o - gal) + 1)) - 1
                do si1_l1005 = 0, (1) - 1
                    do si2_l1006 = 0, (np_particles) - 1
                        x_inl34_ia((si2_l1006) + 1, (si1_l1005) + 1, (si0_l1004) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_ex((si2_l1006) + 1)) + x_inl34_ta((si0_l1004) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl34_ib)) then
                allocate(x_inl34_ib(np_particles, (o + 1), 1))
            else if (size(x_inl34_ib, 1) /= (np_particles) .or. size(x_inl34_ib, 2) /= ((o + 1)) .or. size(x_inl34_ib, &
            &3) /= (1)) then
                deallocate(x_inl34_ib)
                allocate(x_inl34_ib(np_particles, (o + 1), 1))
            end if
            do si0_l1007 = 0, (1) - 1
                do si1_l1008 = 0, ((o + 1)) - 1
                    do si2_l1009 = 0, (np_particles) - 1
                        x_inl34_ib((si2_l1009) + 1, (si1_l1008) + 1, (si0_l1007) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_ex((si2_l1009) + 1)) + x_inl34_tb((si1_l1008) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl34_ia_b)) then
                allocate(x_inl34_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl34_ia_b, 1) /= (np_particles) .or. size(x_inl34_ia_b, 2) /= ((o + 1)) .or. &
            &size(x_inl34_ia_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl34_ia_b)
                allocate(x_inl34_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1010 = 0, (((o - gal) + 1)) - 1
                do si1_l1011 = 0, ((o + 1)) - 1
                    do si2_l1012 = 0, (np_particles) - 1
                        x_inl34_ia_b((si2_l1012) + 1, (si1_l1011) + 1, (si0_l1010) + 1) = x_inl34_ia((si2_l1012) + 1, &
                        &(0) + 1, (si0_l1010) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl34_ib_b)) then
                allocate(x_inl34_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl34_ib_b, 1) /= (np_particles) .or. size(x_inl34_ib_b, 2) /= ((o + 1)) .or. &
            &size(x_inl34_ib_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl34_ib_b)
                allocate(x_inl34_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1013 = 0, (((o - gal) + 1)) - 1
                do si1_l1014 = 0, ((o + 1)) - 1
                    do si2_l1015 = 0, (np_particles) - 1
                        x_inl34_ib_b((si2_l1015) + 1, (si1_l1014) + 1, (si0_l1013) + 1) = x_inl34_ib((si2_l1015) + 1, &
                        &(si1_l1014) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl34_gathered)) then
                allocate(x_inl34_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl34_gathered, 1) /= (np_particles) .or. size(x_inl34_gathered, 2) /= ((o + 1)) .or. &
            &size(x_inl34_gathered, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl34_gathered)
                allocate(x_inl34_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1016 = 0, (((o - gal) + 1)) - 1
                do x_w1_1017 = 0, ((o + 1)) - 1
                    do x_w2_1018 = 0, (np_particles) - 1
                        x_inl34_gathered((x_w2_1018) + 1, (x_w1_1017) + 1, (x_w0_1016) + 1) = ex_arr((((2 * &
                        &x_inl1_imode_939) - 1)) + 1, (0) + 1, (x_inl34_ib_b((x_w2_1018) + 1, (x_w1_1017) + 1, &
                        &(x_w0_1016) + 1)) + 1, (x_inl34_ia_b((x_w2_1018) + 1, (x_w1_1017) + 1, (x_w0_1016) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl34_weight)) then
                allocate(x_inl34_weight(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl34_weight, 1) /= (np_particles) .or. size(x_inl34_weight, 2) /= ((o + 1)) .or. &
            &size(x_inl34_weight, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl34_weight)
                allocate(x_inl34_weight(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1019 = 0, (((o - gal) + 1)) - 1
                do x_w1_1020 = 0, ((o + 1)) - 1
                    do x_w2_1021 = 0, (np_particles) - 1
                        x_inl34_weight((x_w2_1021) + 1, (x_w1_1020) + 1, (x_w0_1019) + 1) = (x_inl1_sx_ex((x_w2_1021) &
                        &+ 1, (x_w0_1019) + 1) * x_inl1_sz_ex((x_w2_1021) + 1, (x_w1_1020) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb118)) then
                allocate(x_cb118(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_cb118, 1) /= (np_particles) .or. size(x_cb118, 2) /= ((o + 1)) .or. size(x_cb118, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_cb118)
                allocate(x_cb118(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1022 = 0, (((o - gal) + 1)) - 1
                do x_w1_1023 = 0, ((o + 1)) - 1
                    do x_w2_1024 = 0, (np_particles) - 1
                        x_cb118((x_w2_1024) + 1, (x_w1_1023) + 1, (x_w0_1022) + 1) = (x_inl34_weight((x_w2_1024) + 1, &
                        &(x_w1_1023) + 1, (x_w0_1022) + 1) * x_inl34_gathered((x_w2_1024) + 1, (x_w1_1023) + 1, &
                        &(x_w0_1022) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb118, axis=(0, 1))
            do x_ax0_1025 = 0, (np_particles) - 1
                x_cb119((x_ax0_1025) + 1) = 0.0_c_double
                do x_rd0_1026 = 0, (((o - gal) + 1)) - 1
                    do x_rd1_1027 = 0, ((o + 1)) - 1
                        x_cb119((x_ax0_1025) + 1) = (x_cb119((x_ax0_1025) + 1) + x_cb118((x_ax0_1025) + 1, &
                        &(x_rd1_1027) + 1, (x_rd0_1026) + 1))
                    end do
                end do
            end do
            do x_w0_1028 = 0, (np_particles) - 1
                x_hcall17((x_w0_1028) + 1) = x_cb119((x_w0_1028) + 1)
            end do
            if (.not. allocated(x_cb120)) then
                allocate(x_cb120(((o - gal) + 1)))
            else if (size(x_cb120, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb120)
                allocate(x_cb120(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb120)) then
                allocate(x_cb120(((o - gal) + 1)))
            else if (size(x_cb120, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb120)
                allocate(x_cb120(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_ex)
            do x_i_1029 = 0, (x_inl1_n_sx_ex) - 1
                x_cb120((x_i_1029) + 1) = x_i_1029
            end do
            if (.not. allocated(x_inl35_ta)) then
                allocate(x_inl35_ta(((o - gal) + 1)))
            else if (size(x_inl35_ta, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl35_ta)
                allocate(x_inl35_ta(((o - gal) + 1)))
            end if
            do x_w0_1030 = 0, (x_inl1_n_sx_ex) - 1
                x_inl35_ta((x_w0_1030) + 1) = x_cb120((x_w0_1030) + 1)
            end do
            if (.not. allocated(x_cb121)) then
                allocate(x_cb121((o + 1)))
            else if (size(x_cb121, 1) /= ((o + 1))) then
                deallocate(x_cb121)
                allocate(x_cb121((o + 1)))
            end if
            if (.not. allocated(x_cb121)) then
                allocate(x_cb121((o + 1)))
            else if (size(x_cb121, 1) /= ((o + 1))) then
                deallocate(x_cb121)
                allocate(x_cb121((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_ex)
            do x_i_1031 = 0, (x_inl1_n_sz_ex) - 1
                x_cb121((x_i_1031) + 1) = x_i_1031
            end do
            if (.not. allocated(x_inl35_tb)) then
                allocate(x_inl35_tb((o + 1)))
            else if (size(x_inl35_tb, 1) /= ((o + 1))) then
                deallocate(x_inl35_tb)
                allocate(x_inl35_tb((o + 1)))
            end if
            do x_w0_1032 = 0, (x_inl1_n_sz_ex) - 1
                x_inl35_tb((x_w0_1032) + 1) = x_cb121((x_w0_1032) + 1)
            end do
            if (.not. allocated(x_inl35_ia)) then
                allocate(x_inl35_ia(np_particles, 1, ((o - gal) + 1)))
            else if (size(x_inl35_ia, 1) /= (np_particles) .or. size(x_inl35_ia, 2) /= (1) .or. size(x_inl35_ia, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_inl35_ia)
                allocate(x_inl35_ia(np_particles, 1, ((o - gal) + 1)))
            end if
            do si0_l1033 = 0, (((o - gal) + 1)) - 1
                do si1_l1034 = 0, (1) - 1
                    do si2_l1035 = 0, (np_particles) - 1
                        x_inl35_ia((si2_l1035) + 1, (si1_l1034) + 1, (si0_l1033) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_ex((si2_l1035) + 1)) + x_inl35_ta((si0_l1033) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl35_ib)) then
                allocate(x_inl35_ib(np_particles, (o + 1), 1))
            else if (size(x_inl35_ib, 1) /= (np_particles) .or. size(x_inl35_ib, 2) /= ((o + 1)) .or. size(x_inl35_ib, &
            &3) /= (1)) then
                deallocate(x_inl35_ib)
                allocate(x_inl35_ib(np_particles, (o + 1), 1))
            end if
            do si0_l1036 = 0, (1) - 1
                do si1_l1037 = 0, ((o + 1)) - 1
                    do si2_l1038 = 0, (np_particles) - 1
                        x_inl35_ib((si2_l1038) + 1, (si1_l1037) + 1, (si0_l1036) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_ex((si2_l1038) + 1)) + x_inl35_tb((si1_l1037) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl35_ia_b)) then
                allocate(x_inl35_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl35_ia_b, 1) /= (np_particles) .or. size(x_inl35_ia_b, 2) /= ((o + 1)) .or. &
            &size(x_inl35_ia_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl35_ia_b)
                allocate(x_inl35_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1039 = 0, (((o - gal) + 1)) - 1
                do si1_l1040 = 0, ((o + 1)) - 1
                    do si2_l1041 = 0, (np_particles) - 1
                        x_inl35_ia_b((si2_l1041) + 1, (si1_l1040) + 1, (si0_l1039) + 1) = x_inl35_ia((si2_l1041) + 1, &
                        &(0) + 1, (si0_l1039) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl35_ib_b)) then
                allocate(x_inl35_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl35_ib_b, 1) /= (np_particles) .or. size(x_inl35_ib_b, 2) /= ((o + 1)) .or. &
            &size(x_inl35_ib_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl35_ib_b)
                allocate(x_inl35_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1042 = 0, (((o - gal) + 1)) - 1
                do si1_l1043 = 0, ((o + 1)) - 1
                    do si2_l1044 = 0, (np_particles) - 1
                        x_inl35_ib_b((si2_l1044) + 1, (si1_l1043) + 1, (si0_l1042) + 1) = x_inl35_ib((si2_l1044) + 1, &
                        &(si1_l1043) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl35_gathered)) then
                allocate(x_inl35_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl35_gathered, 1) /= (np_particles) .or. size(x_inl35_gathered, 2) /= ((o + 1)) .or. &
            &size(x_inl35_gathered, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl35_gathered)
                allocate(x_inl35_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1045 = 0, (((o - gal) + 1)) - 1
                do x_w1_1046 = 0, ((o + 1)) - 1
                    do x_w2_1047 = 0, (np_particles) - 1
                        x_inl35_gathered((x_w2_1047) + 1, (x_w1_1046) + 1, (x_w0_1045) + 1) = ex_arr(((2 * &
                        &x_inl1_imode_939)) + 1, (0) + 1, (x_inl35_ib_b((x_w2_1047) + 1, (x_w1_1046) + 1, (x_w0_1045) &
                        &+ 1)) + 1, (x_inl35_ia_b((x_w2_1047) + 1, (x_w1_1046) + 1, (x_w0_1045) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl35_weight)) then
                allocate(x_inl35_weight(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl35_weight, 1) /= (np_particles) .or. size(x_inl35_weight, 2) /= ((o + 1)) .or. &
            &size(x_inl35_weight, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl35_weight)
                allocate(x_inl35_weight(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1048 = 0, (((o - gal) + 1)) - 1
                do x_w1_1049 = 0, ((o + 1)) - 1
                    do x_w2_1050 = 0, (np_particles) - 1
                        x_inl35_weight((x_w2_1050) + 1, (x_w1_1049) + 1, (x_w0_1048) + 1) = (x_inl1_sx_ex((x_w2_1050) &
                        &+ 1, (x_w0_1048) + 1) * x_inl1_sz_ex((x_w2_1050) + 1, (x_w1_1049) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb122)) then
                allocate(x_cb122(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_cb122, 1) /= (np_particles) .or. size(x_cb122, 2) /= ((o + 1)) .or. size(x_cb122, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_cb122)
                allocate(x_cb122(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1051 = 0, (((o - gal) + 1)) - 1
                do x_w1_1052 = 0, ((o + 1)) - 1
                    do x_w2_1053 = 0, (np_particles) - 1
                        x_cb122((x_w2_1053) + 1, (x_w1_1052) + 1, (x_w0_1051) + 1) = (x_inl35_weight((x_w2_1053) + 1, &
                        &(x_w1_1052) + 1, (x_w0_1051) + 1) * x_inl35_gathered((x_w2_1053) + 1, (x_w1_1052) + 1, &
                        &(x_w0_1051) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb122, axis=(0, 1))
            do x_ax0_1054 = 0, (np_particles) - 1
                x_cb123((x_ax0_1054) + 1) = 0.0_c_double
                do x_rd0_1055 = 0, (((o - gal) + 1)) - 1
                    do x_rd1_1056 = 0, ((o + 1)) - 1
                        x_cb123((x_ax0_1054) + 1) = (x_cb123((x_ax0_1054) + 1) + x_cb122((x_ax0_1054) + 1, &
                        &(x_rd1_1056) + 1, (x_rd0_1055) + 1))
                    end do
                end do
            end do
            do x_w0_1057 = 0, (np_particles) - 1
                x_hcall18((x_w0_1057) + 1) = x_cb123((x_w0_1057) + 1)
            end do
            do x_w0_1058 = 0, (np_particles) - 1
                x_inl1_dEx((x_w0_1058) + 1) = ((x_inl1_xy_re((x_w0_1058) + 1) * x_hcall17((x_w0_1058) + 1)) - &
                &(x_inl1_xy_im((x_w0_1058) + 1) * x_hcall18((x_w0_1058) + 1)))
            end do
            do x_w0_1059 = 0, (np_particles) - 1
                x_inl1_Erp((x_w0_1059) + 1) = x_inl1_Erp((x_w0_1059) + 1) + (x_inl1_dEx((x_w0_1059) + 1))
            end do
            if (.not. allocated(x_cb124)) then
                allocate(x_cb124(((o - gal) + 1)))
            else if (size(x_cb124, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb124)
                allocate(x_cb124(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb124)) then
                allocate(x_cb124(((o - gal) + 1)))
            else if (size(x_cb124, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb124)
                allocate(x_cb124(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_bz)
            do x_i_1060 = 0, (x_inl1_n_sx_bz) - 1
                x_cb124((x_i_1060) + 1) = x_i_1060
            end do
            if (.not. allocated(x_inl36_ta)) then
                allocate(x_inl36_ta(((o - gal) + 1)))
            else if (size(x_inl36_ta, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl36_ta)
                allocate(x_inl36_ta(((o - gal) + 1)))
            end if
            do x_w0_1061 = 0, (x_inl1_n_sx_bz) - 1
                x_inl36_ta((x_w0_1061) + 1) = x_cb124((x_w0_1061) + 1)
            end do
            if (.not. allocated(x_cb125)) then
                allocate(x_cb125((o + 1)))
            else if (size(x_cb125, 1) /= ((o + 1))) then
                deallocate(x_cb125)
                allocate(x_cb125((o + 1)))
            end if
            if (.not. allocated(x_cb125)) then
                allocate(x_cb125((o + 1)))
            else if (size(x_cb125, 1) /= ((o + 1))) then
                deallocate(x_cb125)
                allocate(x_cb125((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_bz)
            do x_i_1062 = 0, (x_inl1_n_sz_bz) - 1
                x_cb125((x_i_1062) + 1) = x_i_1062
            end do
            if (.not. allocated(x_inl36_tb)) then
                allocate(x_inl36_tb((o + 1)))
            else if (size(x_inl36_tb, 1) /= ((o + 1))) then
                deallocate(x_inl36_tb)
                allocate(x_inl36_tb((o + 1)))
            end if
            do x_w0_1063 = 0, (x_inl1_n_sz_bz) - 1
                x_inl36_tb((x_w0_1063) + 1) = x_cb125((x_w0_1063) + 1)
            end do
            if (.not. allocated(x_inl36_ia)) then
                allocate(x_inl36_ia(np_particles, 1, ((o - gal) + 1)))
            else if (size(x_inl36_ia, 1) /= (np_particles) .or. size(x_inl36_ia, 2) /= (1) .or. size(x_inl36_ia, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_inl36_ia)
                allocate(x_inl36_ia(np_particles, 1, ((o - gal) + 1)))
            end if
            do si0_l1064 = 0, (((o - gal) + 1)) - 1
                do si1_l1065 = 0, (1) - 1
                    do si2_l1066 = 0, (np_particles) - 1
                        x_inl36_ia((si2_l1066) + 1, (si1_l1065) + 1, (si0_l1064) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_bz((si2_l1066) + 1)) + x_inl36_ta((si0_l1064) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl36_ib)) then
                allocate(x_inl36_ib(np_particles, (o + 1), 1))
            else if (size(x_inl36_ib, 1) /= (np_particles) .or. size(x_inl36_ib, 2) /= ((o + 1)) .or. size(x_inl36_ib, &
            &3) /= (1)) then
                deallocate(x_inl36_ib)
                allocate(x_inl36_ib(np_particles, (o + 1), 1))
            end if
            do si0_l1067 = 0, (1) - 1
                do si1_l1068 = 0, ((o + 1)) - 1
                    do si2_l1069 = 0, (np_particles) - 1
                        x_inl36_ib((si2_l1069) + 1, (si1_l1068) + 1, (si0_l1067) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_bz((si2_l1069) + 1)) + x_inl36_tb((si1_l1068) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl36_ia_b)) then
                allocate(x_inl36_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl36_ia_b, 1) /= (np_particles) .or. size(x_inl36_ia_b, 2) /= ((o + 1)) .or. &
            &size(x_inl36_ia_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl36_ia_b)
                allocate(x_inl36_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1070 = 0, (((o - gal) + 1)) - 1
                do si1_l1071 = 0, ((o + 1)) - 1
                    do si2_l1072 = 0, (np_particles) - 1
                        x_inl36_ia_b((si2_l1072) + 1, (si1_l1071) + 1, (si0_l1070) + 1) = x_inl36_ia((si2_l1072) + 1, &
                        &(0) + 1, (si0_l1070) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl36_ib_b)) then
                allocate(x_inl36_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl36_ib_b, 1) /= (np_particles) .or. size(x_inl36_ib_b, 2) /= ((o + 1)) .or. &
            &size(x_inl36_ib_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl36_ib_b)
                allocate(x_inl36_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1073 = 0, (((o - gal) + 1)) - 1
                do si1_l1074 = 0, ((o + 1)) - 1
                    do si2_l1075 = 0, (np_particles) - 1
                        x_inl36_ib_b((si2_l1075) + 1, (si1_l1074) + 1, (si0_l1073) + 1) = x_inl36_ib((si2_l1075) + 1, &
                        &(si1_l1074) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl36_gathered)) then
                allocate(x_inl36_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl36_gathered, 1) /= (np_particles) .or. size(x_inl36_gathered, 2) /= ((o + 1)) .or. &
            &size(x_inl36_gathered, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl36_gathered)
                allocate(x_inl36_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1076 = 0, (((o - gal) + 1)) - 1
                do x_w1_1077 = 0, ((o + 1)) - 1
                    do x_w2_1078 = 0, (np_particles) - 1
                        x_inl36_gathered((x_w2_1078) + 1, (x_w1_1077) + 1, (x_w0_1076) + 1) = bz_arr((((2 * &
                        &x_inl1_imode_939) - 1)) + 1, (0) + 1, (x_inl36_ib_b((x_w2_1078) + 1, (x_w1_1077) + 1, &
                        &(x_w0_1076) + 1)) + 1, (x_inl36_ia_b((x_w2_1078) + 1, (x_w1_1077) + 1, (x_w0_1076) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl36_weight)) then
                allocate(x_inl36_weight(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl36_weight, 1) /= (np_particles) .or. size(x_inl36_weight, 2) /= ((o + 1)) .or. &
            &size(x_inl36_weight, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl36_weight)
                allocate(x_inl36_weight(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1079 = 0, (((o - gal) + 1)) - 1
                do x_w1_1080 = 0, ((o + 1)) - 1
                    do x_w2_1081 = 0, (np_particles) - 1
                        x_inl36_weight((x_w2_1081) + 1, (x_w1_1080) + 1, (x_w0_1079) + 1) = (x_inl1_sx_bz((x_w2_1081) &
                        &+ 1, (x_w0_1079) + 1) * x_inl1_sz_bz((x_w2_1081) + 1, (x_w1_1080) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb126)) then
                allocate(x_cb126(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_cb126, 1) /= (np_particles) .or. size(x_cb126, 2) /= ((o + 1)) .or. size(x_cb126, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_cb126)
                allocate(x_cb126(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1082 = 0, (((o - gal) + 1)) - 1
                do x_w1_1083 = 0, ((o + 1)) - 1
                    do x_w2_1084 = 0, (np_particles) - 1
                        x_cb126((x_w2_1084) + 1, (x_w1_1083) + 1, (x_w0_1082) + 1) = (x_inl36_weight((x_w2_1084) + 1, &
                        &(x_w1_1083) + 1, (x_w0_1082) + 1) * x_inl36_gathered((x_w2_1084) + 1, (x_w1_1083) + 1, &
                        &(x_w0_1082) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb126, axis=(0, 1))
            do x_ax0_1085 = 0, (np_particles) - 1
                x_cb127((x_ax0_1085) + 1) = 0.0_c_double
                do x_rd0_1086 = 0, (((o - gal) + 1)) - 1
                    do x_rd1_1087 = 0, ((o + 1)) - 1
                        x_cb127((x_ax0_1085) + 1) = (x_cb127((x_ax0_1085) + 1) + x_cb126((x_ax0_1085) + 1, &
                        &(x_rd1_1087) + 1, (x_rd0_1086) + 1))
                    end do
                end do
            end do
            do x_w0_1088 = 0, (np_particles) - 1
                x_hcall19((x_w0_1088) + 1) = x_cb127((x_w0_1088) + 1)
            end do
            if (.not. allocated(x_cb128)) then
                allocate(x_cb128(((o - gal) + 1)))
            else if (size(x_cb128, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb128)
                allocate(x_cb128(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb128)) then
                allocate(x_cb128(((o - gal) + 1)))
            else if (size(x_cb128, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb128)
                allocate(x_cb128(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_bz)
            do x_i_1089 = 0, (x_inl1_n_sx_bz) - 1
                x_cb128((x_i_1089) + 1) = x_i_1089
            end do
            if (.not. allocated(x_inl37_ta)) then
                allocate(x_inl37_ta(((o - gal) + 1)))
            else if (size(x_inl37_ta, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl37_ta)
                allocate(x_inl37_ta(((o - gal) + 1)))
            end if
            do x_w0_1090 = 0, (x_inl1_n_sx_bz) - 1
                x_inl37_ta((x_w0_1090) + 1) = x_cb128((x_w0_1090) + 1)
            end do
            if (.not. allocated(x_cb129)) then
                allocate(x_cb129((o + 1)))
            else if (size(x_cb129, 1) /= ((o + 1))) then
                deallocate(x_cb129)
                allocate(x_cb129((o + 1)))
            end if
            if (.not. allocated(x_cb129)) then
                allocate(x_cb129((o + 1)))
            else if (size(x_cb129, 1) /= ((o + 1))) then
                deallocate(x_cb129)
                allocate(x_cb129((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_bz)
            do x_i_1091 = 0, (x_inl1_n_sz_bz) - 1
                x_cb129((x_i_1091) + 1) = x_i_1091
            end do
            if (.not. allocated(x_inl37_tb)) then
                allocate(x_inl37_tb((o + 1)))
            else if (size(x_inl37_tb, 1) /= ((o + 1))) then
                deallocate(x_inl37_tb)
                allocate(x_inl37_tb((o + 1)))
            end if
            do x_w0_1092 = 0, (x_inl1_n_sz_bz) - 1
                x_inl37_tb((x_w0_1092) + 1) = x_cb129((x_w0_1092) + 1)
            end do
            if (.not. allocated(x_inl37_ia)) then
                allocate(x_inl37_ia(np_particles, 1, ((o - gal) + 1)))
            else if (size(x_inl37_ia, 1) /= (np_particles) .or. size(x_inl37_ia, 2) /= (1) .or. size(x_inl37_ia, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_inl37_ia)
                allocate(x_inl37_ia(np_particles, 1, ((o - gal) + 1)))
            end if
            do si0_l1093 = 0, (((o - gal) + 1)) - 1
                do si1_l1094 = 0, (1) - 1
                    do si2_l1095 = 0, (np_particles) - 1
                        x_inl37_ia((si2_l1095) + 1, (si1_l1094) + 1, (si0_l1093) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_bz((si2_l1095) + 1)) + x_inl37_ta((si0_l1093) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl37_ib)) then
                allocate(x_inl37_ib(np_particles, (o + 1), 1))
            else if (size(x_inl37_ib, 1) /= (np_particles) .or. size(x_inl37_ib, 2) /= ((o + 1)) .or. size(x_inl37_ib, &
            &3) /= (1)) then
                deallocate(x_inl37_ib)
                allocate(x_inl37_ib(np_particles, (o + 1), 1))
            end if
            do si0_l1096 = 0, (1) - 1
                do si1_l1097 = 0, ((o + 1)) - 1
                    do si2_l1098 = 0, (np_particles) - 1
                        x_inl37_ib((si2_l1098) + 1, (si1_l1097) + 1, (si0_l1096) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_bz((si2_l1098) + 1)) + x_inl37_tb((si1_l1097) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl37_ia_b)) then
                allocate(x_inl37_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl37_ia_b, 1) /= (np_particles) .or. size(x_inl37_ia_b, 2) /= ((o + 1)) .or. &
            &size(x_inl37_ia_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl37_ia_b)
                allocate(x_inl37_ia_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1099 = 0, (((o - gal) + 1)) - 1
                do si1_l1100 = 0, ((o + 1)) - 1
                    do si2_l1101 = 0, (np_particles) - 1
                        x_inl37_ia_b((si2_l1101) + 1, (si1_l1100) + 1, (si0_l1099) + 1) = x_inl37_ia((si2_l1101) + 1, &
                        &(0) + 1, (si0_l1099) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl37_ib_b)) then
                allocate(x_inl37_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl37_ib_b, 1) /= (np_particles) .or. size(x_inl37_ib_b, 2) /= ((o + 1)) .or. &
            &size(x_inl37_ib_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl37_ib_b)
                allocate(x_inl37_ib_b(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do si0_l1102 = 0, (((o - gal) + 1)) - 1
                do si1_l1103 = 0, ((o + 1)) - 1
                    do si2_l1104 = 0, (np_particles) - 1
                        x_inl37_ib_b((si2_l1104) + 1, (si1_l1103) + 1, (si0_l1102) + 1) = x_inl37_ib((si2_l1104) + 1, &
                        &(si1_l1103) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl37_gathered)) then
                allocate(x_inl37_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl37_gathered, 1) /= (np_particles) .or. size(x_inl37_gathered, 2) /= ((o + 1)) .or. &
            &size(x_inl37_gathered, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl37_gathered)
                allocate(x_inl37_gathered(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1105 = 0, (((o - gal) + 1)) - 1
                do x_w1_1106 = 0, ((o + 1)) - 1
                    do x_w2_1107 = 0, (np_particles) - 1
                        x_inl37_gathered((x_w2_1107) + 1, (x_w1_1106) + 1, (x_w0_1105) + 1) = bz_arr(((2 * &
                        &x_inl1_imode_939)) + 1, (0) + 1, (x_inl37_ib_b((x_w2_1107) + 1, (x_w1_1106) + 1, (x_w0_1105) &
                        &+ 1)) + 1, (x_inl37_ia_b((x_w2_1107) + 1, (x_w1_1106) + 1, (x_w0_1105) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl37_weight)) then
                allocate(x_inl37_weight(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_inl37_weight, 1) /= (np_particles) .or. size(x_inl37_weight, 2) /= ((o + 1)) .or. &
            &size(x_inl37_weight, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl37_weight)
                allocate(x_inl37_weight(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1108 = 0, (((o - gal) + 1)) - 1
                do x_w1_1109 = 0, ((o + 1)) - 1
                    do x_w2_1110 = 0, (np_particles) - 1
                        x_inl37_weight((x_w2_1110) + 1, (x_w1_1109) + 1, (x_w0_1108) + 1) = (x_inl1_sx_bz((x_w2_1110) &
                        &+ 1, (x_w0_1108) + 1) * x_inl1_sz_bz((x_w2_1110) + 1, (x_w1_1109) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb130)) then
                allocate(x_cb130(np_particles, (o + 1), ((o - gal) + 1)))
            else if (size(x_cb130, 1) /= (np_particles) .or. size(x_cb130, 2) /= ((o + 1)) .or. size(x_cb130, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_cb130)
                allocate(x_cb130(np_particles, (o + 1), ((o - gal) + 1)))
            end if
            do x_w0_1111 = 0, (((o - gal) + 1)) - 1
                do x_w1_1112 = 0, ((o + 1)) - 1
                    do x_w2_1113 = 0, (np_particles) - 1
                        x_cb130((x_w2_1113) + 1, (x_w1_1112) + 1, (x_w0_1111) + 1) = (x_inl37_weight((x_w2_1113) + 1, &
                        &(x_w1_1112) + 1, (x_w0_1111) + 1) * x_inl37_gathered((x_w2_1113) + 1, (x_w1_1112) + 1, &
                        &(x_w0_1111) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb130, axis=(0, 1))
            do x_ax0_1114 = 0, (np_particles) - 1
                x_cb131((x_ax0_1114) + 1) = 0.0_c_double
                do x_rd0_1115 = 0, (((o - gal) + 1)) - 1
                    do x_rd1_1116 = 0, ((o + 1)) - 1
                        x_cb131((x_ax0_1114) + 1) = (x_cb131((x_ax0_1114) + 1) + x_cb130((x_ax0_1114) + 1, &
                        &(x_rd1_1116) + 1, (x_rd0_1115) + 1))
                    end do
                end do
            end do
            do x_w0_1117 = 0, (np_particles) - 1
                x_hcall20((x_w0_1117) + 1) = x_cb131((x_w0_1117) + 1)
            end do
            do x_w0_1118 = 0, (np_particles) - 1
                x_inl1_dBz((x_w0_1118) + 1) = ((x_inl1_xy_re((x_w0_1118) + 1) * x_hcall19((x_w0_1118) + 1)) - &
                &(x_inl1_xy_im((x_w0_1118) + 1) * x_hcall20((x_w0_1118) + 1)))
            end do
            do x_w0_1119 = 0, (np_particles) - 1
                Bzp((x_w0_1119) + 1) = Bzp((x_w0_1119) + 1) + (x_inl1_dBz((x_w0_1119) + 1))
            end do
            if (.not. allocated(x_cb132)) then
                allocate(x_cb132((o + 1)))
            else if (size(x_cb132, 1) /= ((o + 1))) then
                deallocate(x_cb132)
                allocate(x_cb132((o + 1)))
            end if
            if (.not. allocated(x_cb132)) then
                allocate(x_cb132((o + 1)))
            else if (size(x_cb132, 1) /= ((o + 1))) then
                deallocate(x_cb132)
                allocate(x_cb132((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_ez)
            do x_i_1120 = 0, (x_inl1_n_sx_ez) - 1
                x_cb132((x_i_1120) + 1) = x_i_1120
            end do
            if (.not. allocated(x_inl38_ta)) then
                allocate(x_inl38_ta((o + 1)))
            else if (size(x_inl38_ta, 1) /= ((o + 1))) then
                deallocate(x_inl38_ta)
                allocate(x_inl38_ta((o + 1)))
            end if
            do x_w0_1121 = 0, (x_inl1_n_sx_ez) - 1
                x_inl38_ta((x_w0_1121) + 1) = x_cb132((x_w0_1121) + 1)
            end do
            if (.not. allocated(x_cb133)) then
                allocate(x_cb133(((o - gal) + 1)))
            else if (size(x_cb133, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb133)
                allocate(x_cb133(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb133)) then
                allocate(x_cb133(((o - gal) + 1)))
            else if (size(x_cb133, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb133)
                allocate(x_cb133(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_ez)
            do x_i_1122 = 0, (x_inl1_n_sz_ez) - 1
                x_cb133((x_i_1122) + 1) = x_i_1122
            end do
            if (.not. allocated(x_inl38_tb)) then
                allocate(x_inl38_tb(((o - gal) + 1)))
            else if (size(x_inl38_tb, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl38_tb)
                allocate(x_inl38_tb(((o - gal) + 1)))
            end if
            do x_w0_1123 = 0, (x_inl1_n_sz_ez) - 1
                x_inl38_tb((x_w0_1123) + 1) = x_cb133((x_w0_1123) + 1)
            end do
            if (.not. allocated(x_inl38_ia)) then
                allocate(x_inl38_ia(np_particles, 1, (o + 1)))
            else if (size(x_inl38_ia, 1) /= (np_particles) .or. size(x_inl38_ia, 2) /= (1) .or. size(x_inl38_ia, 3) /= &
            &((o + 1))) then
                deallocate(x_inl38_ia)
                allocate(x_inl38_ia(np_particles, 1, (o + 1)))
            end if
            do si0_l1124 = 0, ((o + 1)) - 1
                do si1_l1125 = 0, (1) - 1
                    do si2_l1126 = 0, (np_particles) - 1
                        x_inl38_ia((si2_l1126) + 1, (si1_l1125) + 1, (si0_l1124) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_ez((si2_l1126) + 1)) + x_inl38_ta((si0_l1124) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl38_ib)) then
                allocate(x_inl38_ib(np_particles, ((o - gal) + 1), 1))
            else if (size(x_inl38_ib, 1) /= (np_particles) .or. size(x_inl38_ib, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl38_ib, 3) /= (1)) then
                deallocate(x_inl38_ib)
                allocate(x_inl38_ib(np_particles, ((o - gal) + 1), 1))
            end if
            do si0_l1127 = 0, (1) - 1
                do si1_l1128 = 0, (((o - gal) + 1)) - 1
                    do si2_l1129 = 0, (np_particles) - 1
                        x_inl38_ib((si2_l1129) + 1, (si1_l1128) + 1, (si0_l1127) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_ez((si2_l1129) + 1)) + x_inl38_tb((si1_l1128) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl38_ia_b)) then
                allocate(x_inl38_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl38_ia_b, 1) /= (np_particles) .or. size(x_inl38_ia_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl38_ia_b, 3) /= ((o + 1))) then
                deallocate(x_inl38_ia_b)
                allocate(x_inl38_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1130 = 0, ((o + 1)) - 1
                do si1_l1131 = 0, (((o - gal) + 1)) - 1
                    do si2_l1132 = 0, (np_particles) - 1
                        x_inl38_ia_b((si2_l1132) + 1, (si1_l1131) + 1, (si0_l1130) + 1) = x_inl38_ia((si2_l1132) + 1, &
                        &(0) + 1, (si0_l1130) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl38_ib_b)) then
                allocate(x_inl38_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl38_ib_b, 1) /= (np_particles) .or. size(x_inl38_ib_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl38_ib_b, 3) /= ((o + 1))) then
                deallocate(x_inl38_ib_b)
                allocate(x_inl38_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1133 = 0, ((o + 1)) - 1
                do si1_l1134 = 0, (((o - gal) + 1)) - 1
                    do si2_l1135 = 0, (np_particles) - 1
                        x_inl38_ib_b((si2_l1135) + 1, (si1_l1134) + 1, (si0_l1133) + 1) = x_inl38_ib((si2_l1135) + 1, &
                        &(si1_l1134) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl38_gathered)) then
                allocate(x_inl38_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl38_gathered, 1) /= (np_particles) .or. size(x_inl38_gathered, 2) /= (((o - gal) + 1)) &
            &.or. size(x_inl38_gathered, 3) /= ((o + 1))) then
                deallocate(x_inl38_gathered)
                allocate(x_inl38_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1136 = 0, ((o + 1)) - 1
                do x_w1_1137 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1138 = 0, (np_particles) - 1
                        x_inl38_gathered((x_w2_1138) + 1, (x_w1_1137) + 1, (x_w0_1136) + 1) = ez_arr((((2 * &
                        &x_inl1_imode_939) - 1)) + 1, (0) + 1, (x_inl38_ib_b((x_w2_1138) + 1, (x_w1_1137) + 1, &
                        &(x_w0_1136) + 1)) + 1, (x_inl38_ia_b((x_w2_1138) + 1, (x_w1_1137) + 1, (x_w0_1136) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl38_weight)) then
                allocate(x_inl38_weight(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl38_weight, 1) /= (np_particles) .or. size(x_inl38_weight, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl38_weight, 3) /= ((o + 1))) then
                deallocate(x_inl38_weight)
                allocate(x_inl38_weight(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1139 = 0, ((o + 1)) - 1
                do x_w1_1140 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1141 = 0, (np_particles) - 1
                        x_inl38_weight((x_w2_1141) + 1, (x_w1_1140) + 1, (x_w0_1139) + 1) = (x_inl1_sx_ez((x_w2_1141) &
                        &+ 1, (x_w0_1139) + 1) * x_inl1_sz_ez((x_w2_1141) + 1, (x_w1_1140) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb134)) then
                allocate(x_cb134(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_cb134, 1) /= (np_particles) .or. size(x_cb134, 2) /= (((o - gal) + 1)) .or. size(x_cb134, &
            &3) /= ((o + 1))) then
                deallocate(x_cb134)
                allocate(x_cb134(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1142 = 0, ((o + 1)) - 1
                do x_w1_1143 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1144 = 0, (np_particles) - 1
                        x_cb134((x_w2_1144) + 1, (x_w1_1143) + 1, (x_w0_1142) + 1) = (x_inl38_weight((x_w2_1144) + 1, &
                        &(x_w1_1143) + 1, (x_w0_1142) + 1) * x_inl38_gathered((x_w2_1144) + 1, (x_w1_1143) + 1, &
                        &(x_w0_1142) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb134, axis=(0, 1))
            do x_ax0_1145 = 0, (np_particles) - 1
                x_cb135((x_ax0_1145) + 1) = 0.0_c_double
                do x_rd0_1146 = 0, ((o + 1)) - 1
                    do x_rd1_1147 = 0, (((o - gal) + 1)) - 1
                        x_cb135((x_ax0_1145) + 1) = (x_cb135((x_ax0_1145) + 1) + x_cb134((x_ax0_1145) + 1, &
                        &(x_rd1_1147) + 1, (x_rd0_1146) + 1))
                    end do
                end do
            end do
            do x_w0_1148 = 0, (np_particles) - 1
                x_hcall21((x_w0_1148) + 1) = x_cb135((x_w0_1148) + 1)
            end do
            if (.not. allocated(x_cb136)) then
                allocate(x_cb136((o + 1)))
            else if (size(x_cb136, 1) /= ((o + 1))) then
                deallocate(x_cb136)
                allocate(x_cb136((o + 1)))
            end if
            if (.not. allocated(x_cb136)) then
                allocate(x_cb136((o + 1)))
            else if (size(x_cb136, 1) /= ((o + 1))) then
                deallocate(x_cb136)
                allocate(x_cb136((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_ez)
            do x_i_1149 = 0, (x_inl1_n_sx_ez) - 1
                x_cb136((x_i_1149) + 1) = x_i_1149
            end do
            if (.not. allocated(x_inl39_ta)) then
                allocate(x_inl39_ta((o + 1)))
            else if (size(x_inl39_ta, 1) /= ((o + 1))) then
                deallocate(x_inl39_ta)
                allocate(x_inl39_ta((o + 1)))
            end if
            do x_w0_1150 = 0, (x_inl1_n_sx_ez) - 1
                x_inl39_ta((x_w0_1150) + 1) = x_cb136((x_w0_1150) + 1)
            end do
            if (.not. allocated(x_cb137)) then
                allocate(x_cb137(((o - gal) + 1)))
            else if (size(x_cb137, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb137)
                allocate(x_cb137(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb137)) then
                allocate(x_cb137(((o - gal) + 1)))
            else if (size(x_cb137, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb137)
                allocate(x_cb137(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_ez)
            do x_i_1151 = 0, (x_inl1_n_sz_ez) - 1
                x_cb137((x_i_1151) + 1) = x_i_1151
            end do
            if (.not. allocated(x_inl39_tb)) then
                allocate(x_inl39_tb(((o - gal) + 1)))
            else if (size(x_inl39_tb, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl39_tb)
                allocate(x_inl39_tb(((o - gal) + 1)))
            end if
            do x_w0_1152 = 0, (x_inl1_n_sz_ez) - 1
                x_inl39_tb((x_w0_1152) + 1) = x_cb137((x_w0_1152) + 1)
            end do
            if (.not. allocated(x_inl39_ia)) then
                allocate(x_inl39_ia(np_particles, 1, (o + 1)))
            else if (size(x_inl39_ia, 1) /= (np_particles) .or. size(x_inl39_ia, 2) /= (1) .or. size(x_inl39_ia, 3) /= &
            &((o + 1))) then
                deallocate(x_inl39_ia)
                allocate(x_inl39_ia(np_particles, 1, (o + 1)))
            end if
            do si0_l1153 = 0, ((o + 1)) - 1
                do si1_l1154 = 0, (1) - 1
                    do si2_l1155 = 0, (np_particles) - 1
                        x_inl39_ia((si2_l1155) + 1, (si1_l1154) + 1, (si0_l1153) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_ez((si2_l1155) + 1)) + x_inl39_ta((si0_l1153) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl39_ib)) then
                allocate(x_inl39_ib(np_particles, ((o - gal) + 1), 1))
            else if (size(x_inl39_ib, 1) /= (np_particles) .or. size(x_inl39_ib, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl39_ib, 3) /= (1)) then
                deallocate(x_inl39_ib)
                allocate(x_inl39_ib(np_particles, ((o - gal) + 1), 1))
            end if
            do si0_l1156 = 0, (1) - 1
                do si1_l1157 = 0, (((o - gal) + 1)) - 1
                    do si2_l1158 = 0, (np_particles) - 1
                        x_inl39_ib((si2_l1158) + 1, (si1_l1157) + 1, (si0_l1156) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_ez((si2_l1158) + 1)) + x_inl39_tb((si1_l1157) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl39_ia_b)) then
                allocate(x_inl39_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl39_ia_b, 1) /= (np_particles) .or. size(x_inl39_ia_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl39_ia_b, 3) /= ((o + 1))) then
                deallocate(x_inl39_ia_b)
                allocate(x_inl39_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1159 = 0, ((o + 1)) - 1
                do si1_l1160 = 0, (((o - gal) + 1)) - 1
                    do si2_l1161 = 0, (np_particles) - 1
                        x_inl39_ia_b((si2_l1161) + 1, (si1_l1160) + 1, (si0_l1159) + 1) = x_inl39_ia((si2_l1161) + 1, &
                        &(0) + 1, (si0_l1159) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl39_ib_b)) then
                allocate(x_inl39_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl39_ib_b, 1) /= (np_particles) .or. size(x_inl39_ib_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl39_ib_b, 3) /= ((o + 1))) then
                deallocate(x_inl39_ib_b)
                allocate(x_inl39_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1162 = 0, ((o + 1)) - 1
                do si1_l1163 = 0, (((o - gal) + 1)) - 1
                    do si2_l1164 = 0, (np_particles) - 1
                        x_inl39_ib_b((si2_l1164) + 1, (si1_l1163) + 1, (si0_l1162) + 1) = x_inl39_ib((si2_l1164) + 1, &
                        &(si1_l1163) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl39_gathered)) then
                allocate(x_inl39_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl39_gathered, 1) /= (np_particles) .or. size(x_inl39_gathered, 2) /= (((o - gal) + 1)) &
            &.or. size(x_inl39_gathered, 3) /= ((o + 1))) then
                deallocate(x_inl39_gathered)
                allocate(x_inl39_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1165 = 0, ((o + 1)) - 1
                do x_w1_1166 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1167 = 0, (np_particles) - 1
                        x_inl39_gathered((x_w2_1167) + 1, (x_w1_1166) + 1, (x_w0_1165) + 1) = ez_arr(((2 * &
                        &x_inl1_imode_939)) + 1, (0) + 1, (x_inl39_ib_b((x_w2_1167) + 1, (x_w1_1166) + 1, (x_w0_1165) &
                        &+ 1)) + 1, (x_inl39_ia_b((x_w2_1167) + 1, (x_w1_1166) + 1, (x_w0_1165) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl39_weight)) then
                allocate(x_inl39_weight(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl39_weight, 1) /= (np_particles) .or. size(x_inl39_weight, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl39_weight, 3) /= ((o + 1))) then
                deallocate(x_inl39_weight)
                allocate(x_inl39_weight(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1168 = 0, ((o + 1)) - 1
                do x_w1_1169 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1170 = 0, (np_particles) - 1
                        x_inl39_weight((x_w2_1170) + 1, (x_w1_1169) + 1, (x_w0_1168) + 1) = (x_inl1_sx_ez((x_w2_1170) &
                        &+ 1, (x_w0_1168) + 1) * x_inl1_sz_ez((x_w2_1170) + 1, (x_w1_1169) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb138)) then
                allocate(x_cb138(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_cb138, 1) /= (np_particles) .or. size(x_cb138, 2) /= (((o - gal) + 1)) .or. size(x_cb138, &
            &3) /= ((o + 1))) then
                deallocate(x_cb138)
                allocate(x_cb138(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1171 = 0, ((o + 1)) - 1
                do x_w1_1172 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1173 = 0, (np_particles) - 1
                        x_cb138((x_w2_1173) + 1, (x_w1_1172) + 1, (x_w0_1171) + 1) = (x_inl39_weight((x_w2_1173) + 1, &
                        &(x_w1_1172) + 1, (x_w0_1171) + 1) * x_inl39_gathered((x_w2_1173) + 1, (x_w1_1172) + 1, &
                        &(x_w0_1171) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb138, axis=(0, 1))
            do x_ax0_1174 = 0, (np_particles) - 1
                x_cb139((x_ax0_1174) + 1) = 0.0_c_double
                do x_rd0_1175 = 0, ((o + 1)) - 1
                    do x_rd1_1176 = 0, (((o - gal) + 1)) - 1
                        x_cb139((x_ax0_1174) + 1) = (x_cb139((x_ax0_1174) + 1) + x_cb138((x_ax0_1174) + 1, &
                        &(x_rd1_1176) + 1, (x_rd0_1175) + 1))
                    end do
                end do
            end do
            do x_w0_1177 = 0, (np_particles) - 1
                x_hcall22((x_w0_1177) + 1) = x_cb139((x_w0_1177) + 1)
            end do
            do x_w0_1178 = 0, (np_particles) - 1
                x_inl1_dEz((x_w0_1178) + 1) = ((x_inl1_xy_re((x_w0_1178) + 1) * x_hcall21((x_w0_1178) + 1)) - &
                &(x_inl1_xy_im((x_w0_1178) + 1) * x_hcall22((x_w0_1178) + 1)))
            end do
            do x_w0_1179 = 0, (np_particles) - 1
                Ezp((x_w0_1179) + 1) = Ezp((x_w0_1179) + 1) + (x_inl1_dEz((x_w0_1179) + 1))
            end do
            if (.not. allocated(x_cb140)) then
                allocate(x_cb140((o + 1)))
            else if (size(x_cb140, 1) /= ((o + 1))) then
                deallocate(x_cb140)
                allocate(x_cb140((o + 1)))
            end if
            if (.not. allocated(x_cb140)) then
                allocate(x_cb140((o + 1)))
            else if (size(x_cb140, 1) /= ((o + 1))) then
                deallocate(x_cb140)
                allocate(x_cb140((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_bx)
            do x_i_1180 = 0, (x_inl1_n_sx_bx) - 1
                x_cb140((x_i_1180) + 1) = x_i_1180
            end do
            if (.not. allocated(x_inl40_ta)) then
                allocate(x_inl40_ta((o + 1)))
            else if (size(x_inl40_ta, 1) /= ((o + 1))) then
                deallocate(x_inl40_ta)
                allocate(x_inl40_ta((o + 1)))
            end if
            do x_w0_1181 = 0, (x_inl1_n_sx_bx) - 1
                x_inl40_ta((x_w0_1181) + 1) = x_cb140((x_w0_1181) + 1)
            end do
            if (.not. allocated(x_cb141)) then
                allocate(x_cb141(((o - gal) + 1)))
            else if (size(x_cb141, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb141)
                allocate(x_cb141(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb141)) then
                allocate(x_cb141(((o - gal) + 1)))
            else if (size(x_cb141, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb141)
                allocate(x_cb141(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_bx)
            do x_i_1182 = 0, (x_inl1_n_sz_bx) - 1
                x_cb141((x_i_1182) + 1) = x_i_1182
            end do
            if (.not. allocated(x_inl40_tb)) then
                allocate(x_inl40_tb(((o - gal) + 1)))
            else if (size(x_inl40_tb, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl40_tb)
                allocate(x_inl40_tb(((o - gal) + 1)))
            end if
            do x_w0_1183 = 0, (x_inl1_n_sz_bx) - 1
                x_inl40_tb((x_w0_1183) + 1) = x_cb141((x_w0_1183) + 1)
            end do
            if (.not. allocated(x_inl40_ia)) then
                allocate(x_inl40_ia(np_particles, 1, (o + 1)))
            else if (size(x_inl40_ia, 1) /= (np_particles) .or. size(x_inl40_ia, 2) /= (1) .or. size(x_inl40_ia, 3) /= &
            &((o + 1))) then
                deallocate(x_inl40_ia)
                allocate(x_inl40_ia(np_particles, 1, (o + 1)))
            end if
            do si0_l1184 = 0, ((o + 1)) - 1
                do si1_l1185 = 0, (1) - 1
                    do si2_l1186 = 0, (np_particles) - 1
                        x_inl40_ia((si2_l1186) + 1, (si1_l1185) + 1, (si0_l1184) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_bx((si2_l1186) + 1)) + x_inl40_ta((si0_l1184) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl40_ib)) then
                allocate(x_inl40_ib(np_particles, ((o - gal) + 1), 1))
            else if (size(x_inl40_ib, 1) /= (np_particles) .or. size(x_inl40_ib, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl40_ib, 3) /= (1)) then
                deallocate(x_inl40_ib)
                allocate(x_inl40_ib(np_particles, ((o - gal) + 1), 1))
            end if
            do si0_l1187 = 0, (1) - 1
                do si1_l1188 = 0, (((o - gal) + 1)) - 1
                    do si2_l1189 = 0, (np_particles) - 1
                        x_inl40_ib((si2_l1189) + 1, (si1_l1188) + 1, (si0_l1187) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_bx((si2_l1189) + 1)) + x_inl40_tb((si1_l1188) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl40_ia_b)) then
                allocate(x_inl40_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl40_ia_b, 1) /= (np_particles) .or. size(x_inl40_ia_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl40_ia_b, 3) /= ((o + 1))) then
                deallocate(x_inl40_ia_b)
                allocate(x_inl40_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1190 = 0, ((o + 1)) - 1
                do si1_l1191 = 0, (((o - gal) + 1)) - 1
                    do si2_l1192 = 0, (np_particles) - 1
                        x_inl40_ia_b((si2_l1192) + 1, (si1_l1191) + 1, (si0_l1190) + 1) = x_inl40_ia((si2_l1192) + 1, &
                        &(0) + 1, (si0_l1190) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl40_ib_b)) then
                allocate(x_inl40_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl40_ib_b, 1) /= (np_particles) .or. size(x_inl40_ib_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl40_ib_b, 3) /= ((o + 1))) then
                deallocate(x_inl40_ib_b)
                allocate(x_inl40_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1193 = 0, ((o + 1)) - 1
                do si1_l1194 = 0, (((o - gal) + 1)) - 1
                    do si2_l1195 = 0, (np_particles) - 1
                        x_inl40_ib_b((si2_l1195) + 1, (si1_l1194) + 1, (si0_l1193) + 1) = x_inl40_ib((si2_l1195) + 1, &
                        &(si1_l1194) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl40_gathered)) then
                allocate(x_inl40_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl40_gathered, 1) /= (np_particles) .or. size(x_inl40_gathered, 2) /= (((o - gal) + 1)) &
            &.or. size(x_inl40_gathered, 3) /= ((o + 1))) then
                deallocate(x_inl40_gathered)
                allocate(x_inl40_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1196 = 0, ((o + 1)) - 1
                do x_w1_1197 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1198 = 0, (np_particles) - 1
                        x_inl40_gathered((x_w2_1198) + 1, (x_w1_1197) + 1, (x_w0_1196) + 1) = bx_arr((((2 * &
                        &x_inl1_imode_939) - 1)) + 1, (0) + 1, (x_inl40_ib_b((x_w2_1198) + 1, (x_w1_1197) + 1, &
                        &(x_w0_1196) + 1)) + 1, (x_inl40_ia_b((x_w2_1198) + 1, (x_w1_1197) + 1, (x_w0_1196) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl40_weight)) then
                allocate(x_inl40_weight(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl40_weight, 1) /= (np_particles) .or. size(x_inl40_weight, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl40_weight, 3) /= ((o + 1))) then
                deallocate(x_inl40_weight)
                allocate(x_inl40_weight(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1199 = 0, ((o + 1)) - 1
                do x_w1_1200 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1201 = 0, (np_particles) - 1
                        x_inl40_weight((x_w2_1201) + 1, (x_w1_1200) + 1, (x_w0_1199) + 1) = (x_inl1_sx_bx((x_w2_1201) &
                        &+ 1, (x_w0_1199) + 1) * x_inl1_sz_bx((x_w2_1201) + 1, (x_w1_1200) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb142)) then
                allocate(x_cb142(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_cb142, 1) /= (np_particles) .or. size(x_cb142, 2) /= (((o - gal) + 1)) .or. size(x_cb142, &
            &3) /= ((o + 1))) then
                deallocate(x_cb142)
                allocate(x_cb142(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1202 = 0, ((o + 1)) - 1
                do x_w1_1203 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1204 = 0, (np_particles) - 1
                        x_cb142((x_w2_1204) + 1, (x_w1_1203) + 1, (x_w0_1202) + 1) = (x_inl40_weight((x_w2_1204) + 1, &
                        &(x_w1_1203) + 1, (x_w0_1202) + 1) * x_inl40_gathered((x_w2_1204) + 1, (x_w1_1203) + 1, &
                        &(x_w0_1202) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb142, axis=(0, 1))
            do x_ax0_1205 = 0, (np_particles) - 1
                x_cb143((x_ax0_1205) + 1) = 0.0_c_double
                do x_rd0_1206 = 0, ((o + 1)) - 1
                    do x_rd1_1207 = 0, (((o - gal) + 1)) - 1
                        x_cb143((x_ax0_1205) + 1) = (x_cb143((x_ax0_1205) + 1) + x_cb142((x_ax0_1205) + 1, &
                        &(x_rd1_1207) + 1, (x_rd0_1206) + 1))
                    end do
                end do
            end do
            do x_w0_1208 = 0, (np_particles) - 1
                x_hcall23((x_w0_1208) + 1) = x_cb143((x_w0_1208) + 1)
            end do
            if (.not. allocated(x_cb144)) then
                allocate(x_cb144((o + 1)))
            else if (size(x_cb144, 1) /= ((o + 1))) then
                deallocate(x_cb144)
                allocate(x_cb144((o + 1)))
            end if
            if (.not. allocated(x_cb144)) then
                allocate(x_cb144((o + 1)))
            else if (size(x_cb144, 1) /= ((o + 1))) then
                deallocate(x_cb144)
                allocate(x_cb144((o + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_bx)
            do x_i_1209 = 0, (x_inl1_n_sx_bx) - 1
                x_cb144((x_i_1209) + 1) = x_i_1209
            end do
            if (.not. allocated(x_inl41_ta)) then
                allocate(x_inl41_ta((o + 1)))
            else if (size(x_inl41_ta, 1) /= ((o + 1))) then
                deallocate(x_inl41_ta)
                allocate(x_inl41_ta((o + 1)))
            end if
            do x_w0_1210 = 0, (x_inl1_n_sx_bx) - 1
                x_inl41_ta((x_w0_1210) + 1) = x_cb144((x_w0_1210) + 1)
            end do
            if (.not. allocated(x_cb145)) then
                allocate(x_cb145(((o - gal) + 1)))
            else if (size(x_cb145, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb145)
                allocate(x_cb145(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb145)) then
                allocate(x_cb145(((o - gal) + 1)))
            else if (size(x_cb145, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb145)
                allocate(x_cb145(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_bx)
            do x_i_1211 = 0, (x_inl1_n_sz_bx) - 1
                x_cb145((x_i_1211) + 1) = x_i_1211
            end do
            if (.not. allocated(x_inl41_tb)) then
                allocate(x_inl41_tb(((o - gal) + 1)))
            else if (size(x_inl41_tb, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl41_tb)
                allocate(x_inl41_tb(((o - gal) + 1)))
            end if
            do x_w0_1212 = 0, (x_inl1_n_sz_bx) - 1
                x_inl41_tb((x_w0_1212) + 1) = x_cb145((x_w0_1212) + 1)
            end do
            if (.not. allocated(x_inl41_ia)) then
                allocate(x_inl41_ia(np_particles, 1, (o + 1)))
            else if (size(x_inl41_ia, 1) /= (np_particles) .or. size(x_inl41_ia, 2) /= (1) .or. size(x_inl41_ia, 3) /= &
            &((o + 1))) then
                deallocate(x_inl41_ia)
                allocate(x_inl41_ia(np_particles, 1, (o + 1)))
            end if
            do si0_l1213 = 0, ((o + 1)) - 1
                do si1_l1214 = 0, (1) - 1
                    do si2_l1215 = 0, (np_particles) - 1
                        x_inl41_ia((si2_l1215) + 1, (si1_l1214) + 1, (si0_l1213) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_bx((si2_l1215) + 1)) + x_inl41_ta((si0_l1213) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl41_ib)) then
                allocate(x_inl41_ib(np_particles, ((o - gal) + 1), 1))
            else if (size(x_inl41_ib, 1) /= (np_particles) .or. size(x_inl41_ib, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl41_ib, 3) /= (1)) then
                deallocate(x_inl41_ib)
                allocate(x_inl41_ib(np_particles, ((o - gal) + 1), 1))
            end if
            do si0_l1216 = 0, (1) - 1
                do si1_l1217 = 0, (((o - gal) + 1)) - 1
                    do si2_l1218 = 0, (np_particles) - 1
                        x_inl41_ib((si2_l1218) + 1, (si1_l1217) + 1, (si0_l1216) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_bx((si2_l1218) + 1)) + x_inl41_tb((si1_l1217) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl41_ia_b)) then
                allocate(x_inl41_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl41_ia_b, 1) /= (np_particles) .or. size(x_inl41_ia_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl41_ia_b, 3) /= ((o + 1))) then
                deallocate(x_inl41_ia_b)
                allocate(x_inl41_ia_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1219 = 0, ((o + 1)) - 1
                do si1_l1220 = 0, (((o - gal) + 1)) - 1
                    do si2_l1221 = 0, (np_particles) - 1
                        x_inl41_ia_b((si2_l1221) + 1, (si1_l1220) + 1, (si0_l1219) + 1) = x_inl41_ia((si2_l1221) + 1, &
                        &(0) + 1, (si0_l1219) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl41_ib_b)) then
                allocate(x_inl41_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl41_ib_b, 1) /= (np_particles) .or. size(x_inl41_ib_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl41_ib_b, 3) /= ((o + 1))) then
                deallocate(x_inl41_ib_b)
                allocate(x_inl41_ib_b(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do si0_l1222 = 0, ((o + 1)) - 1
                do si1_l1223 = 0, (((o - gal) + 1)) - 1
                    do si2_l1224 = 0, (np_particles) - 1
                        x_inl41_ib_b((si2_l1224) + 1, (si1_l1223) + 1, (si0_l1222) + 1) = x_inl41_ib((si2_l1224) + 1, &
                        &(si1_l1223) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl41_gathered)) then
                allocate(x_inl41_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl41_gathered, 1) /= (np_particles) .or. size(x_inl41_gathered, 2) /= (((o - gal) + 1)) &
            &.or. size(x_inl41_gathered, 3) /= ((o + 1))) then
                deallocate(x_inl41_gathered)
                allocate(x_inl41_gathered(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1225 = 0, ((o + 1)) - 1
                do x_w1_1226 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1227 = 0, (np_particles) - 1
                        x_inl41_gathered((x_w2_1227) + 1, (x_w1_1226) + 1, (x_w0_1225) + 1) = bx_arr(((2 * &
                        &x_inl1_imode_939)) + 1, (0) + 1, (x_inl41_ib_b((x_w2_1227) + 1, (x_w1_1226) + 1, (x_w0_1225) &
                        &+ 1)) + 1, (x_inl41_ia_b((x_w2_1227) + 1, (x_w1_1226) + 1, (x_w0_1225) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl41_weight)) then
                allocate(x_inl41_weight(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_inl41_weight, 1) /= (np_particles) .or. size(x_inl41_weight, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl41_weight, 3) /= ((o + 1))) then
                deallocate(x_inl41_weight)
                allocate(x_inl41_weight(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1228 = 0, ((o + 1)) - 1
                do x_w1_1229 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1230 = 0, (np_particles) - 1
                        x_inl41_weight((x_w2_1230) + 1, (x_w1_1229) + 1, (x_w0_1228) + 1) = (x_inl1_sx_bx((x_w2_1230) &
                        &+ 1, (x_w0_1228) + 1) * x_inl1_sz_bx((x_w2_1230) + 1, (x_w1_1229) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb146)) then
                allocate(x_cb146(np_particles, ((o - gal) + 1), (o + 1)))
            else if (size(x_cb146, 1) /= (np_particles) .or. size(x_cb146, 2) /= (((o - gal) + 1)) .or. size(x_cb146, &
            &3) /= ((o + 1))) then
                deallocate(x_cb146)
                allocate(x_cb146(np_particles, ((o - gal) + 1), (o + 1)))
            end if
            do x_w0_1231 = 0, ((o + 1)) - 1
                do x_w1_1232 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1233 = 0, (np_particles) - 1
                        x_cb146((x_w2_1233) + 1, (x_w1_1232) + 1, (x_w0_1231) + 1) = (x_inl41_weight((x_w2_1233) + 1, &
                        &(x_w1_1232) + 1, (x_w0_1231) + 1) * x_inl41_gathered((x_w2_1233) + 1, (x_w1_1232) + 1, &
                        &(x_w0_1231) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb146, axis=(0, 1))
            do x_ax0_1234 = 0, (np_particles) - 1
                x_cb147((x_ax0_1234) + 1) = 0.0_c_double
                do x_rd0_1235 = 0, ((o + 1)) - 1
                    do x_rd1_1236 = 0, (((o - gal) + 1)) - 1
                        x_cb147((x_ax0_1234) + 1) = (x_cb147((x_ax0_1234) + 1) + x_cb146((x_ax0_1234) + 1, &
                        &(x_rd1_1236) + 1, (x_rd0_1235) + 1))
                    end do
                end do
            end do
            do x_w0_1237 = 0, (np_particles) - 1
                x_hcall24((x_w0_1237) + 1) = x_cb147((x_w0_1237) + 1)
            end do
            do x_w0_1238 = 0, (np_particles) - 1
                x_inl1_dBx((x_w0_1238) + 1) = ((x_inl1_xy_re((x_w0_1238) + 1) * x_hcall23((x_w0_1238) + 1)) - &
                &(x_inl1_xy_im((x_w0_1238) + 1) * x_hcall24((x_w0_1238) + 1)))
            end do
            do x_w0_1239 = 0, (np_particles) - 1
                x_inl1_Brp((x_w0_1239) + 1) = x_inl1_Brp((x_w0_1239) + 1) + (x_inl1_dBx((x_w0_1239) + 1))
            end do
            if (.not. allocated(x_cb148)) then
                allocate(x_cb148(((o - gal) + 1)))
            else if (size(x_cb148, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb148)
                allocate(x_cb148(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb148)) then
                allocate(x_cb148(((o - gal) + 1)))
            else if (size(x_cb148, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb148)
                allocate(x_cb148(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_by)
            do x_i_1240 = 0, (x_inl1_n_sx_by) - 1
                x_cb148((x_i_1240) + 1) = x_i_1240
            end do
            if (.not. allocated(x_inl42_ta)) then
                allocate(x_inl42_ta(((o - gal) + 1)))
            else if (size(x_inl42_ta, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl42_ta)
                allocate(x_inl42_ta(((o - gal) + 1)))
            end if
            do x_w0_1241 = 0, (x_inl1_n_sx_by) - 1
                x_inl42_ta((x_w0_1241) + 1) = x_cb148((x_w0_1241) + 1)
            end do
            if (.not. allocated(x_cb149)) then
                allocate(x_cb149(((o - gal) + 1)))
            else if (size(x_cb149, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb149)
                allocate(x_cb149(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb149)) then
                allocate(x_cb149(((o - gal) + 1)))
            else if (size(x_cb149, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb149)
                allocate(x_cb149(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_by)
            do x_i_1242 = 0, (x_inl1_n_sz_by) - 1
                x_cb149((x_i_1242) + 1) = x_i_1242
            end do
            if (.not. allocated(x_inl42_tb)) then
                allocate(x_inl42_tb(((o - gal) + 1)))
            else if (size(x_inl42_tb, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl42_tb)
                allocate(x_inl42_tb(((o - gal) + 1)))
            end if
            do x_w0_1243 = 0, (x_inl1_n_sz_by) - 1
                x_inl42_tb((x_w0_1243) + 1) = x_cb149((x_w0_1243) + 1)
            end do
            if (.not. allocated(x_inl42_ia)) then
                allocate(x_inl42_ia(np_particles, 1, ((o - gal) + 1)))
            else if (size(x_inl42_ia, 1) /= (np_particles) .or. size(x_inl42_ia, 2) /= (1) .or. size(x_inl42_ia, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_inl42_ia)
                allocate(x_inl42_ia(np_particles, 1, ((o - gal) + 1)))
            end if
            do si0_l1244 = 0, (((o - gal) + 1)) - 1
                do si1_l1245 = 0, (1) - 1
                    do si2_l1246 = 0, (np_particles) - 1
                        x_inl42_ia((si2_l1246) + 1, (si1_l1245) + 1, (si0_l1244) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_by((si2_l1246) + 1)) + x_inl42_ta((si0_l1244) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl42_ib)) then
                allocate(x_inl42_ib(np_particles, ((o - gal) + 1), 1))
            else if (size(x_inl42_ib, 1) /= (np_particles) .or. size(x_inl42_ib, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl42_ib, 3) /= (1)) then
                deallocate(x_inl42_ib)
                allocate(x_inl42_ib(np_particles, ((o - gal) + 1), 1))
            end if
            do si0_l1247 = 0, (1) - 1
                do si1_l1248 = 0, (((o - gal) + 1)) - 1
                    do si2_l1249 = 0, (np_particles) - 1
                        x_inl42_ib((si2_l1249) + 1, (si1_l1248) + 1, (si0_l1247) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_by((si2_l1249) + 1)) + x_inl42_tb((si1_l1248) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl42_ia_b)) then
                allocate(x_inl42_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl42_ia_b, 1) /= (np_particles) .or. size(x_inl42_ia_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl42_ia_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl42_ia_b)
                allocate(x_inl42_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do si0_l1250 = 0, (((o - gal) + 1)) - 1
                do si1_l1251 = 0, (((o - gal) + 1)) - 1
                    do si2_l1252 = 0, (np_particles) - 1
                        x_inl42_ia_b((si2_l1252) + 1, (si1_l1251) + 1, (si0_l1250) + 1) = x_inl42_ia((si2_l1252) + 1, &
                        &(0) + 1, (si0_l1250) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl42_ib_b)) then
                allocate(x_inl42_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl42_ib_b, 1) /= (np_particles) .or. size(x_inl42_ib_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl42_ib_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl42_ib_b)
                allocate(x_inl42_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do si0_l1253 = 0, (((o - gal) + 1)) - 1
                do si1_l1254 = 0, (((o - gal) + 1)) - 1
                    do si2_l1255 = 0, (np_particles) - 1
                        x_inl42_ib_b((si2_l1255) + 1, (si1_l1254) + 1, (si0_l1253) + 1) = x_inl42_ib((si2_l1255) + 1, &
                        &(si1_l1254) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl42_gathered)) then
                allocate(x_inl42_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl42_gathered, 1) /= (np_particles) .or. size(x_inl42_gathered, 2) /= (((o - gal) + 1)) &
            &.or. size(x_inl42_gathered, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl42_gathered)
                allocate(x_inl42_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do x_w0_1256 = 0, (((o - gal) + 1)) - 1
                do x_w1_1257 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1258 = 0, (np_particles) - 1
                        x_inl42_gathered((x_w2_1258) + 1, (x_w1_1257) + 1, (x_w0_1256) + 1) = by_arr((((2 * &
                        &x_inl1_imode_939) - 1)) + 1, (0) + 1, (x_inl42_ib_b((x_w2_1258) + 1, (x_w1_1257) + 1, &
                        &(x_w0_1256) + 1)) + 1, (x_inl42_ia_b((x_w2_1258) + 1, (x_w1_1257) + 1, (x_w0_1256) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl42_weight)) then
                allocate(x_inl42_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl42_weight, 1) /= (np_particles) .or. size(x_inl42_weight, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl42_weight, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl42_weight)
                allocate(x_inl42_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do x_w0_1259 = 0, (((o - gal) + 1)) - 1
                do x_w1_1260 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1261 = 0, (np_particles) - 1
                        x_inl42_weight((x_w2_1261) + 1, (x_w1_1260) + 1, (x_w0_1259) + 1) = (x_inl1_sx_by((x_w2_1261) &
                        &+ 1, (x_w0_1259) + 1) * x_inl1_sz_by((x_w2_1261) + 1, (x_w1_1260) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb150)) then
                allocate(x_cb150(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_cb150, 1) /= (np_particles) .or. size(x_cb150, 2) /= (((o - gal) + 1)) .or. size(x_cb150, &
            &3) /= (((o - gal) + 1))) then
                deallocate(x_cb150)
                allocate(x_cb150(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do x_w0_1262 = 0, (((o - gal) + 1)) - 1
                do x_w1_1263 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1264 = 0, (np_particles) - 1
                        x_cb150((x_w2_1264) + 1, (x_w1_1263) + 1, (x_w0_1262) + 1) = (x_inl42_weight((x_w2_1264) + 1, &
                        &(x_w1_1263) + 1, (x_w0_1262) + 1) * x_inl42_gathered((x_w2_1264) + 1, (x_w1_1263) + 1, &
                        &(x_w0_1262) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb150, axis=(0, 1))
            do x_ax0_1265 = 0, (np_particles) - 1
                x_cb151((x_ax0_1265) + 1) = 0.0_c_double
                do x_rd0_1266 = 0, (((o - gal) + 1)) - 1
                    do x_rd1_1267 = 0, (((o - gal) + 1)) - 1
                        x_cb151((x_ax0_1265) + 1) = (x_cb151((x_ax0_1265) + 1) + x_cb150((x_ax0_1265) + 1, &
                        &(x_rd1_1267) + 1, (x_rd0_1266) + 1))
                    end do
                end do
            end do
            do x_w0_1268 = 0, (np_particles) - 1
                x_hcall25((x_w0_1268) + 1) = x_cb151((x_w0_1268) + 1)
            end do
            if (.not. allocated(x_cb152)) then
                allocate(x_cb152(((o - gal) + 1)))
            else if (size(x_cb152, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb152)
                allocate(x_cb152(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb152)) then
                allocate(x_cb152(((o - gal) + 1)))
            else if (size(x_cb152, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb152)
                allocate(x_cb152(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sx_by)
            do x_i_1269 = 0, (x_inl1_n_sx_by) - 1
                x_cb152((x_i_1269) + 1) = x_i_1269
            end do
            if (.not. allocated(x_inl43_ta)) then
                allocate(x_inl43_ta(((o - gal) + 1)))
            else if (size(x_inl43_ta, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl43_ta)
                allocate(x_inl43_ta(((o - gal) + 1)))
            end if
            do x_w0_1270 = 0, (x_inl1_n_sx_by) - 1
                x_inl43_ta((x_w0_1270) + 1) = x_cb152((x_w0_1270) + 1)
            end do
            if (.not. allocated(x_cb153)) then
                allocate(x_cb153(((o - gal) + 1)))
            else if (size(x_cb153, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb153)
                allocate(x_cb153(((o - gal) + 1)))
            end if
            if (.not. allocated(x_cb153)) then
                allocate(x_cb153(((o - gal) + 1)))
            else if (size(x_cb153, 1) /= (((o - gal) + 1))) then
                deallocate(x_cb153)
                allocate(x_cb153(((o - gal) + 1)))
            end if
            ! numpy: np.arange(__inl1_n_sz_by)
            do x_i_1271 = 0, (x_inl1_n_sz_by) - 1
                x_cb153((x_i_1271) + 1) = x_i_1271
            end do
            if (.not. allocated(x_inl43_tb)) then
                allocate(x_inl43_tb(((o - gal) + 1)))
            else if (size(x_inl43_tb, 1) /= (((o - gal) + 1))) then
                deallocate(x_inl43_tb)
                allocate(x_inl43_tb(((o - gal) + 1)))
            end if
            do x_w0_1272 = 0, (x_inl1_n_sz_by) - 1
                x_inl43_tb((x_w0_1272) + 1) = x_cb153((x_w0_1272) + 1)
            end do
            if (.not. allocated(x_inl43_ia)) then
                allocate(x_inl43_ia(np_particles, 1, ((o - gal) + 1)))
            else if (size(x_inl43_ia, 1) /= (np_particles) .or. size(x_inl43_ia, 2) /= (1) .or. size(x_inl43_ia, 3) /= &
            &(((o - gal) + 1))) then
                deallocate(x_inl43_ia)
                allocate(x_inl43_ia(np_particles, 1, ((o - gal) + 1)))
            end if
            do si0_l1273 = 0, (((o - gal) + 1)) - 1
                do si1_l1274 = 0, (1) - 1
                    do si2_l1275 = 0, (np_particles) - 1
                        x_inl43_ia((si2_l1275) + 1, (si1_l1274) + 1, (si0_l1273) + 1) = ((x_inl1_lox + &
                        &x_inl1_j_by((si2_l1275) + 1)) + x_inl43_ta((si0_l1273) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl43_ib)) then
                allocate(x_inl43_ib(np_particles, ((o - gal) + 1), 1))
            else if (size(x_inl43_ib, 1) /= (np_particles) .or. size(x_inl43_ib, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl43_ib, 3) /= (1)) then
                deallocate(x_inl43_ib)
                allocate(x_inl43_ib(np_particles, ((o - gal) + 1), 1))
            end if
            do si0_l1276 = 0, (1) - 1
                do si1_l1277 = 0, (((o - gal) + 1)) - 1
                    do si2_l1278 = 0, (np_particles) - 1
                        x_inl43_ib((si2_l1278) + 1, (si1_l1277) + 1, (si0_l1276) + 1) = ((x_inl1_loy + &
                        &x_inl1_l_by((si2_l1278) + 1)) + x_inl43_tb((si1_l1277) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_inl43_ia_b)) then
                allocate(x_inl43_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl43_ia_b, 1) /= (np_particles) .or. size(x_inl43_ia_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl43_ia_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl43_ia_b)
                allocate(x_inl43_ia_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do si0_l1279 = 0, (((o - gal) + 1)) - 1
                do si1_l1280 = 0, (((o - gal) + 1)) - 1
                    do si2_l1281 = 0, (np_particles) - 1
                        x_inl43_ia_b((si2_l1281) + 1, (si1_l1280) + 1, (si0_l1279) + 1) = x_inl43_ia((si2_l1281) + 1, &
                        &(0) + 1, (si0_l1279) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl43_ib_b)) then
                allocate(x_inl43_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl43_ib_b, 1) /= (np_particles) .or. size(x_inl43_ib_b, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl43_ib_b, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl43_ib_b)
                allocate(x_inl43_ib_b(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do si0_l1282 = 0, (((o - gal) + 1)) - 1
                do si1_l1283 = 0, (((o - gal) + 1)) - 1
                    do si2_l1284 = 0, (np_particles) - 1
                        x_inl43_ib_b((si2_l1284) + 1, (si1_l1283) + 1, (si0_l1282) + 1) = x_inl43_ib((si2_l1284) + 1, &
                        &(si1_l1283) + 1, (0) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl43_gathered)) then
                allocate(x_inl43_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl43_gathered, 1) /= (np_particles) .or. size(x_inl43_gathered, 2) /= (((o - gal) + 1)) &
            &.or. size(x_inl43_gathered, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl43_gathered)
                allocate(x_inl43_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do x_w0_1285 = 0, (((o - gal) + 1)) - 1
                do x_w1_1286 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1287 = 0, (np_particles) - 1
                        x_inl43_gathered((x_w2_1287) + 1, (x_w1_1286) + 1, (x_w0_1285) + 1) = by_arr(((2 * &
                        &x_inl1_imode_939)) + 1, (0) + 1, (x_inl43_ib_b((x_w2_1287) + 1, (x_w1_1286) + 1, (x_w0_1285) &
                        &+ 1)) + 1, (x_inl43_ia_b((x_w2_1287) + 1, (x_w1_1286) + 1, (x_w0_1285) + 1)) + 1)
                    end do
                end do
            end do
            if (.not. allocated(x_inl43_weight)) then
                allocate(x_inl43_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_inl43_weight, 1) /= (np_particles) .or. size(x_inl43_weight, 2) /= (((o - gal) + 1)) .or. &
            &size(x_inl43_weight, 3) /= (((o - gal) + 1))) then
                deallocate(x_inl43_weight)
                allocate(x_inl43_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do x_w0_1288 = 0, (((o - gal) + 1)) - 1
                do x_w1_1289 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1290 = 0, (np_particles) - 1
                        x_inl43_weight((x_w2_1290) + 1, (x_w1_1289) + 1, (x_w0_1288) + 1) = (x_inl1_sx_by((x_w2_1290) &
                        &+ 1, (x_w0_1288) + 1) * x_inl1_sz_by((x_w2_1290) + 1, (x_w1_1289) + 1))
                    end do
                end do
            end do
            if (.not. allocated(x_cb154)) then
                allocate(x_cb154(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            else if (size(x_cb154, 1) /= (np_particles) .or. size(x_cb154, 2) /= (((o - gal) + 1)) .or. size(x_cb154, &
            &3) /= (((o - gal) + 1))) then
                deallocate(x_cb154)
                allocate(x_cb154(np_particles, ((o - gal) + 1), ((o - gal) + 1)))
            end if
            do x_w0_1291 = 0, (((o - gal) + 1)) - 1
                do x_w1_1292 = 0, (((o - gal) + 1)) - 1
                    do x_w2_1293 = 0, (np_particles) - 1
                        x_cb154((x_w2_1293) + 1, (x_w1_1292) + 1, (x_w0_1291) + 1) = (x_inl43_weight((x_w2_1293) + 1, &
                        &(x_w1_1292) + 1, (x_w0_1291) + 1) * x_inl43_gathered((x_w2_1293) + 1, (x_w1_1292) + 1, &
                        &(x_w0_1291) + 1))
                    end do
                end do
            end do
            ! numpy: np.sum(__cb154, axis=(0, 1))
            do x_ax0_1294 = 0, (np_particles) - 1
                x_cb155((x_ax0_1294) + 1) = 0.0_c_double
                do x_rd0_1295 = 0, (((o - gal) + 1)) - 1
                    do x_rd1_1296 = 0, (((o - gal) + 1)) - 1
                        x_cb155((x_ax0_1294) + 1) = (x_cb155((x_ax0_1294) + 1) + x_cb154((x_ax0_1294) + 1, &
                        &(x_rd1_1296) + 1, (x_rd0_1295) + 1))
                    end do
                end do
            end do
            do x_w0_1297 = 0, (np_particles) - 1
                x_hcall26((x_w0_1297) + 1) = x_cb155((x_w0_1297) + 1)
            end do
            do x_w0_1298 = 0, (np_particles) - 1
                x_inl1_dBy((x_w0_1298) + 1) = ((x_inl1_xy_re((x_w0_1298) + 1) * x_hcall25((x_w0_1298) + 1)) - &
                &(x_inl1_xy_im((x_w0_1298) + 1) * x_hcall26((x_w0_1298) + 1)))
            end do
            do x_w0_1299 = 0, (np_particles) - 1
                x_inl1_Bthetap((x_w0_1299) + 1) = x_inl1_Bthetap((x_w0_1299) + 1) + (x_inl1_dBy((x_w0_1299) + 1))
            end do
            do x_w0_1300 = 0, (np_particles) - 1
                x_inl1_tmp_re((x_w0_1300) + 1) = ((x_inl1_xy_re((x_w0_1300) + 1) * x_inl1_xy0_re((x_w0_1300) + 1)) - &
                &(x_inl1_xy_im((x_w0_1300) + 1) * x_inl1_xy0_im((x_w0_1300) + 1)))
            end do
            do x_w0_1301 = 0, (np_particles) - 1
                x_inl1_tmp_im((x_w0_1301) + 1) = ((x_inl1_xy_re((x_w0_1301) + 1) * x_inl1_xy0_im((x_w0_1301) + 1)) + &
                &(x_inl1_xy_im((x_w0_1301) + 1) * x_inl1_xy0_re((x_w0_1301) + 1)))
            end do
            do x_w0_1302 = 0, (np_particles) - 1
                x_inl1_xy_re((x_w0_1302) + 1) = x_inl1_tmp_re((x_w0_1302) + 1)
            end do
            do x_w0_1303 = 0, (np_particles) - 1
                x_inl1_xy_im((x_w0_1303) + 1) = x_inl1_tmp_im((x_w0_1303) + 1)
            end do
        end do
        do x_w0_1304 = 0, (np_particles) - 1
            Exp((x_w0_1304) + 1) = Exp((x_w0_1304) + 1) + (((x_inl1_costheta((x_w0_1304) + 1) * x_inl1_Erp((x_w0_1304) &
            &+ 1)) - (x_inl1_sintheta((x_w0_1304) + 1) * x_inl1_Ethetap((x_w0_1304) + 1))))
        end do
        do x_w0_1305 = 0, (np_particles) - 1
            Eyp((x_w0_1305) + 1) = Eyp((x_w0_1305) + 1) + (((x_inl1_costheta((x_w0_1305) + 1) * &
            &x_inl1_Ethetap((x_w0_1305) + 1)) + (x_inl1_sintheta((x_w0_1305) + 1) * x_inl1_Erp((x_w0_1305) + 1))))
        end do
        do x_w0_1306 = 0, (np_particles) - 1
            Bxp((x_w0_1306) + 1) = Bxp((x_w0_1306) + 1) + (((x_inl1_costheta((x_w0_1306) + 1) * x_inl1_Brp((x_w0_1306) &
            &+ 1)) - (x_inl1_sintheta((x_w0_1306) + 1) * x_inl1_Bthetap((x_w0_1306) + 1))))
        end do
        do x_w0_1307 = 0, (np_particles) - 1
            Byp((x_w0_1307) + 1) = Byp((x_w0_1307) + 1) + (((x_inl1_costheta((x_w0_1307) + 1) * &
            &x_inl1_Bthetap((x_w0_1307) + 1)) + (x_inl1_sintheta((x_w0_1307) + 1) * x_inl1_Brp((x_w0_1307) + 1))))
        end do
    else if ((g == 4)) then
        if (.not. allocated(x_cb156)) then
            allocate(x_cb156((o + 1)))
        else if (size(x_cb156, 1) /= ((o + 1))) then
            deallocate(x_cb156)
            allocate(x_cb156((o + 1)))
        end if
        if (.not. allocated(x_cb156)) then
            allocate(x_cb156((o + 1)))
        else if (size(x_cb156, 1) /= ((o + 1))) then
            deallocate(x_cb156)
            allocate(x_cb156((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ey)
        do x_i_1308 = 0, (x_inl1_n_sx_ey) - 1
            x_cb156((x_i_1308) + 1) = x_i_1308
        end do
        if (.not. allocated(x_inl44_taps)) then
            allocate(x_inl44_taps((o + 1)))
        else if (size(x_inl44_taps, 1) /= ((o + 1))) then
            deallocate(x_inl44_taps)
            allocate(x_inl44_taps((o + 1)))
        end if
        do x_w0_1309 = 0, (x_inl1_n_sx_ey) - 1
            x_inl44_taps((x_w0_1309) + 1) = x_cb156((x_w0_1309) + 1)
        end do
        if (.not. allocated(x_inl44_rows)) then
            allocate(x_inl44_rows(np_particles, (o + 1)))
        else if (size(x_inl44_rows, 1) /= (np_particles) .or. size(x_inl44_rows, 2) /= ((o + 1))) then
            deallocate(x_inl44_rows)
            allocate(x_inl44_rows(np_particles, (o + 1)))
        end if
        do x_w0_1310 = 0, (x_inl1_n_sx_ey) - 1
            do x_w1_1311 = 0, (np_particles) - 1
                x_inl44_rows((x_w1_1311) + 1, (x_w0_1310) + 1) = ((x_inl1_lox + x_inl1_j_ey((x_w1_1311) + 1)) + &
                &x_inl44_taps((x_w0_1310) + 1))
            end do
        end do
        if (.not. allocated(x_inl44_gathered)) then
            allocate(x_inl44_gathered(np_particles, (o + 1)))
        else if (size(x_inl44_gathered, 1) /= (np_particles) .or. size(x_inl44_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl44_gathered)
            allocate(x_inl44_gathered(np_particles, (o + 1)))
        end if
        do x_w0_1312 = 0, (x_inl1_n_sx_ey) - 1
            do x_w1_1313 = 0, (np_particles) - 1
                x_inl44_gathered((x_w1_1313) + 1, (x_w0_1312) + 1) = ey_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl44_rows((x_w1_1313) + 1, (x_w0_1312) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb157)) then
            allocate(x_cb157(np_particles, (o + 1)))
        else if (size(x_cb157, 1) /= (np_particles) .or. size(x_cb157, 2) /= ((o + 1))) then
            deallocate(x_cb157)
            allocate(x_cb157(np_particles, (o + 1)))
        end if
        do x_w0_1314 = 0, ((o + 1)) - 1
            do x_w1_1315 = 0, (np_particles) - 1
                x_cb157((x_w1_1315) + 1, (x_w0_1314) + 1) = (x_inl1_sx_ey((x_w1_1315) + 1, (x_w0_1314) + 1) * &
                &x_inl44_gathered((x_w1_1315) + 1, (x_w0_1314) + 1))
            end do
        end do
        ! numpy: np.sum(__cb157, axis=0)
        do x_ax0_1316 = 0, (np_particles) - 1
            x_cb158((x_ax0_1316) + 1) = 0.0_c_double
            do x_rd0_1317 = 0, ((o + 1)) - 1
                x_cb158((x_ax0_1316) + 1) = (x_cb158((x_ax0_1316) + 1) + x_cb157((x_ax0_1316) + 1, (x_rd0_1317) + 1))
            end do
        end do
        do x_w0_1318 = 0, (np_particles) - 1
            x_inl1_Ethetap((x_w0_1318) + 1) = x_cb158((x_w0_1318) + 1)
        end do
        if (.not. allocated(x_cb159)) then
            allocate(x_cb159(((o - gal) + 1)))
        else if (size(x_cb159, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb159)
            allocate(x_cb159(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb159)) then
            allocate(x_cb159(((o - gal) + 1)))
        else if (size(x_cb159, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb159)
            allocate(x_cb159(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ex)
        do x_i_1319 = 0, (x_inl1_n_sx_ex) - 1
            x_cb159((x_i_1319) + 1) = x_i_1319
        end do
        if (.not. allocated(x_inl45_taps)) then
            allocate(x_inl45_taps(((o - gal) + 1)))
        else if (size(x_inl45_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl45_taps)
            allocate(x_inl45_taps(((o - gal) + 1)))
        end if
        do x_w0_1320 = 0, (x_inl1_n_sx_ex) - 1
            x_inl45_taps((x_w0_1320) + 1) = x_cb159((x_w0_1320) + 1)
        end do
        if (.not. allocated(x_inl45_rows)) then
            allocate(x_inl45_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl45_rows, 1) /= (np_particles) .or. size(x_inl45_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl45_rows)
            allocate(x_inl45_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1321 = 0, (x_inl1_n_sx_ex) - 1
            do x_w1_1322 = 0, (np_particles) - 1
                x_inl45_rows((x_w1_1322) + 1, (x_w0_1321) + 1) = ((x_inl1_lox + x_inl1_j_ex((x_w1_1322) + 1)) + &
                &x_inl45_taps((x_w0_1321) + 1))
            end do
        end do
        if (.not. allocated(x_inl45_gathered)) then
            allocate(x_inl45_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl45_gathered, 1) /= (np_particles) .or. size(x_inl45_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl45_gathered)
            allocate(x_inl45_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1323 = 0, (x_inl1_n_sx_ex) - 1
            do x_w1_1324 = 0, (np_particles) - 1
                x_inl45_gathered((x_w1_1324) + 1, (x_w0_1323) + 1) = ex_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl45_rows((x_w1_1324) + 1, (x_w0_1323) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb160)) then
            allocate(x_cb160(np_particles, ((o - gal) + 1)))
        else if (size(x_cb160, 1) /= (np_particles) .or. size(x_cb160, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb160)
            allocate(x_cb160(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1325 = 0, (((o - gal) + 1)) - 1
            do x_w1_1326 = 0, (np_particles) - 1
                x_cb160((x_w1_1326) + 1, (x_w0_1325) + 1) = (x_inl1_sx_ex((x_w1_1326) + 1, (x_w0_1325) + 1) * &
                &x_inl45_gathered((x_w1_1326) + 1, (x_w0_1325) + 1))
            end do
        end do
        ! numpy: np.sum(__cb160, axis=0)
        do x_ax0_1327 = 0, (np_particles) - 1
            x_cb161((x_ax0_1327) + 1) = 0.0_c_double
            do x_rd0_1328 = 0, (((o - gal) + 1)) - 1
                x_cb161((x_ax0_1327) + 1) = (x_cb161((x_ax0_1327) + 1) + x_cb160((x_ax0_1327) + 1, (x_rd0_1328) + 1))
            end do
        end do
        do x_w0_1329 = 0, (np_particles) - 1
            x_inl1_Erp((x_w0_1329) + 1) = x_cb161((x_w0_1329) + 1)
        end do
        if (.not. allocated(x_cb162)) then
            allocate(x_cb162(((o - gal) + 1)))
        else if (size(x_cb162, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb162)
            allocate(x_cb162(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb162)) then
            allocate(x_cb162(((o - gal) + 1)))
        else if (size(x_cb162, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb162)
            allocate(x_cb162(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bz)
        do x_i_1330 = 0, (x_inl1_n_sx_bz) - 1
            x_cb162((x_i_1330) + 1) = x_i_1330
        end do
        if (.not. allocated(x_inl46_taps)) then
            allocate(x_inl46_taps(((o - gal) + 1)))
        else if (size(x_inl46_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl46_taps)
            allocate(x_inl46_taps(((o - gal) + 1)))
        end if
        do x_w0_1331 = 0, (x_inl1_n_sx_bz) - 1
            x_inl46_taps((x_w0_1331) + 1) = x_cb162((x_w0_1331) + 1)
        end do
        if (.not. allocated(x_inl46_rows)) then
            allocate(x_inl46_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl46_rows, 1) /= (np_particles) .or. size(x_inl46_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl46_rows)
            allocate(x_inl46_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1332 = 0, (x_inl1_n_sx_bz) - 1
            do x_w1_1333 = 0, (np_particles) - 1
                x_inl46_rows((x_w1_1333) + 1, (x_w0_1332) + 1) = ((x_inl1_lox + x_inl1_j_bz((x_w1_1333) + 1)) + &
                &x_inl46_taps((x_w0_1332) + 1))
            end do
        end do
        if (.not. allocated(x_inl46_gathered)) then
            allocate(x_inl46_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl46_gathered, 1) /= (np_particles) .or. size(x_inl46_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl46_gathered)
            allocate(x_inl46_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1334 = 0, (x_inl1_n_sx_bz) - 1
            do x_w1_1335 = 0, (np_particles) - 1
                x_inl46_gathered((x_w1_1335) + 1, (x_w0_1334) + 1) = bz_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl46_rows((x_w1_1335) + 1, (x_w0_1334) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb163)) then
            allocate(x_cb163(np_particles, ((o - gal) + 1)))
        else if (size(x_cb163, 1) /= (np_particles) .or. size(x_cb163, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb163)
            allocate(x_cb163(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1336 = 0, (((o - gal) + 1)) - 1
            do x_w1_1337 = 0, (np_particles) - 1
                x_cb163((x_w1_1337) + 1, (x_w0_1336) + 1) = (x_inl1_sx_bz((x_w1_1337) + 1, (x_w0_1336) + 1) * &
                &x_inl46_gathered((x_w1_1337) + 1, (x_w0_1336) + 1))
            end do
        end do
        ! numpy: np.sum(__cb163, axis=0)
        do x_ax0_1338 = 0, (np_particles) - 1
            x_cb164((x_ax0_1338) + 1) = 0.0_c_double
            do x_rd0_1339 = 0, (((o - gal) + 1)) - 1
                x_cb164((x_ax0_1338) + 1) = (x_cb164((x_ax0_1338) + 1) + x_cb163((x_ax0_1338) + 1, (x_rd0_1339) + 1))
            end do
        end do
        do x_w0_1340 = 0, (np_particles) - 1
            x_hcall27((x_w0_1340) + 1) = x_cb164((x_w0_1340) + 1)
        end do
        do x_w0_1341 = 0, (np_particles) - 1
            Bzp((x_w0_1341) + 1) = Bzp((x_w0_1341) + 1) + (x_hcall27((x_w0_1341) + 1))
        end do
        if (.not. allocated(x_cb165)) then
            allocate(x_cb165((o + 1)))
        else if (size(x_cb165, 1) /= ((o + 1))) then
            deallocate(x_cb165)
            allocate(x_cb165((o + 1)))
        end if
        if (.not. allocated(x_cb165)) then
            allocate(x_cb165((o + 1)))
        else if (size(x_cb165, 1) /= ((o + 1))) then
            deallocate(x_cb165)
            allocate(x_cb165((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ez)
        do x_i_1342 = 0, (x_inl1_n_sx_ez) - 1
            x_cb165((x_i_1342) + 1) = x_i_1342
        end do
        if (.not. allocated(x_inl47_taps)) then
            allocate(x_inl47_taps((o + 1)))
        else if (size(x_inl47_taps, 1) /= ((o + 1))) then
            deallocate(x_inl47_taps)
            allocate(x_inl47_taps((o + 1)))
        end if
        do x_w0_1343 = 0, (x_inl1_n_sx_ez) - 1
            x_inl47_taps((x_w0_1343) + 1) = x_cb165((x_w0_1343) + 1)
        end do
        if (.not. allocated(x_inl47_rows)) then
            allocate(x_inl47_rows(np_particles, (o + 1)))
        else if (size(x_inl47_rows, 1) /= (np_particles) .or. size(x_inl47_rows, 2) /= ((o + 1))) then
            deallocate(x_inl47_rows)
            allocate(x_inl47_rows(np_particles, (o + 1)))
        end if
        do x_w0_1344 = 0, (x_inl1_n_sx_ez) - 1
            do x_w1_1345 = 0, (np_particles) - 1
                x_inl47_rows((x_w1_1345) + 1, (x_w0_1344) + 1) = ((x_inl1_lox + x_inl1_j_ez((x_w1_1345) + 1)) + &
                &x_inl47_taps((x_w0_1344) + 1))
            end do
        end do
        if (.not. allocated(x_inl47_gathered)) then
            allocate(x_inl47_gathered(np_particles, (o + 1)))
        else if (size(x_inl47_gathered, 1) /= (np_particles) .or. size(x_inl47_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl47_gathered)
            allocate(x_inl47_gathered(np_particles, (o + 1)))
        end if
        do x_w0_1346 = 0, (x_inl1_n_sx_ez) - 1
            do x_w1_1347 = 0, (np_particles) - 1
                x_inl47_gathered((x_w1_1347) + 1, (x_w0_1346) + 1) = ez_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl47_rows((x_w1_1347) + 1, (x_w0_1346) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb166)) then
            allocate(x_cb166(np_particles, (o + 1)))
        else if (size(x_cb166, 1) /= (np_particles) .or. size(x_cb166, 2) /= ((o + 1))) then
            deallocate(x_cb166)
            allocate(x_cb166(np_particles, (o + 1)))
        end if
        do x_w0_1348 = 0, ((o + 1)) - 1
            do x_w1_1349 = 0, (np_particles) - 1
                x_cb166((x_w1_1349) + 1, (x_w0_1348) + 1) = (x_inl1_sx_ez((x_w1_1349) + 1, (x_w0_1348) + 1) * &
                &x_inl47_gathered((x_w1_1349) + 1, (x_w0_1348) + 1))
            end do
        end do
        ! numpy: np.sum(__cb166, axis=0)
        do x_ax0_1350 = 0, (np_particles) - 1
            x_cb167((x_ax0_1350) + 1) = 0.0_c_double
            do x_rd0_1351 = 0, ((o + 1)) - 1
                x_cb167((x_ax0_1350) + 1) = (x_cb167((x_ax0_1350) + 1) + x_cb166((x_ax0_1350) + 1, (x_rd0_1351) + 1))
            end do
        end do
        do x_w0_1352 = 0, (np_particles) - 1
            x_hcall28((x_w0_1352) + 1) = x_cb167((x_w0_1352) + 1)
        end do
        do x_w0_1353 = 0, (np_particles) - 1
            Ezp((x_w0_1353) + 1) = Ezp((x_w0_1353) + 1) + (x_hcall28((x_w0_1353) + 1))
        end do
        if (.not. allocated(x_cb168)) then
            allocate(x_cb168((o + 1)))
        else if (size(x_cb168, 1) /= ((o + 1))) then
            deallocate(x_cb168)
            allocate(x_cb168((o + 1)))
        end if
        if (.not. allocated(x_cb168)) then
            allocate(x_cb168((o + 1)))
        else if (size(x_cb168, 1) /= ((o + 1))) then
            deallocate(x_cb168)
            allocate(x_cb168((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bx)
        do x_i_1354 = 0, (x_inl1_n_sx_bx) - 1
            x_cb168((x_i_1354) + 1) = x_i_1354
        end do
        if (.not. allocated(x_inl48_taps)) then
            allocate(x_inl48_taps((o + 1)))
        else if (size(x_inl48_taps, 1) /= ((o + 1))) then
            deallocate(x_inl48_taps)
            allocate(x_inl48_taps((o + 1)))
        end if
        do x_w0_1355 = 0, (x_inl1_n_sx_bx) - 1
            x_inl48_taps((x_w0_1355) + 1) = x_cb168((x_w0_1355) + 1)
        end do
        if (.not. allocated(x_inl48_rows)) then
            allocate(x_inl48_rows(np_particles, (o + 1)))
        else if (size(x_inl48_rows, 1) /= (np_particles) .or. size(x_inl48_rows, 2) /= ((o + 1))) then
            deallocate(x_inl48_rows)
            allocate(x_inl48_rows(np_particles, (o + 1)))
        end if
        do x_w0_1356 = 0, (x_inl1_n_sx_bx) - 1
            do x_w1_1357 = 0, (np_particles) - 1
                x_inl48_rows((x_w1_1357) + 1, (x_w0_1356) + 1) = ((x_inl1_lox + x_inl1_j_bx((x_w1_1357) + 1)) + &
                &x_inl48_taps((x_w0_1356) + 1))
            end do
        end do
        if (.not. allocated(x_inl48_gathered)) then
            allocate(x_inl48_gathered(np_particles, (o + 1)))
        else if (size(x_inl48_gathered, 1) /= (np_particles) .or. size(x_inl48_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl48_gathered)
            allocate(x_inl48_gathered(np_particles, (o + 1)))
        end if
        do x_w0_1358 = 0, (x_inl1_n_sx_bx) - 1
            do x_w1_1359 = 0, (np_particles) - 1
                x_inl48_gathered((x_w1_1359) + 1, (x_w0_1358) + 1) = bx_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl48_rows((x_w1_1359) + 1, (x_w0_1358) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb169)) then
            allocate(x_cb169(np_particles, (o + 1)))
        else if (size(x_cb169, 1) /= (np_particles) .or. size(x_cb169, 2) /= ((o + 1))) then
            deallocate(x_cb169)
            allocate(x_cb169(np_particles, (o + 1)))
        end if
        do x_w0_1360 = 0, ((o + 1)) - 1
            do x_w1_1361 = 0, (np_particles) - 1
                x_cb169((x_w1_1361) + 1, (x_w0_1360) + 1) = (x_inl1_sx_bx((x_w1_1361) + 1, (x_w0_1360) + 1) * &
                &x_inl48_gathered((x_w1_1361) + 1, (x_w0_1360) + 1))
            end do
        end do
        ! numpy: np.sum(__cb169, axis=0)
        do x_ax0_1362 = 0, (np_particles) - 1
            x_cb170((x_ax0_1362) + 1) = 0.0_c_double
            do x_rd0_1363 = 0, ((o + 1)) - 1
                x_cb170((x_ax0_1362) + 1) = (x_cb170((x_ax0_1362) + 1) + x_cb169((x_ax0_1362) + 1, (x_rd0_1363) + 1))
            end do
        end do
        do x_w0_1364 = 0, (np_particles) - 1
            x_inl1_Brp((x_w0_1364) + 1) = x_cb170((x_w0_1364) + 1)
        end do
        if (.not. allocated(x_cb171)) then
            allocate(x_cb171(((o - gal) + 1)))
        else if (size(x_cb171, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb171)
            allocate(x_cb171(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb171)) then
            allocate(x_cb171(((o - gal) + 1)))
        else if (size(x_cb171, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb171)
            allocate(x_cb171(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_by)
        do x_i_1365 = 0, (x_inl1_n_sx_by) - 1
            x_cb171((x_i_1365) + 1) = x_i_1365
        end do
        if (.not. allocated(x_inl49_taps)) then
            allocate(x_inl49_taps(((o - gal) + 1)))
        else if (size(x_inl49_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl49_taps)
            allocate(x_inl49_taps(((o - gal) + 1)))
        end if
        do x_w0_1366 = 0, (x_inl1_n_sx_by) - 1
            x_inl49_taps((x_w0_1366) + 1) = x_cb171((x_w0_1366) + 1)
        end do
        if (.not. allocated(x_inl49_rows)) then
            allocate(x_inl49_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl49_rows, 1) /= (np_particles) .or. size(x_inl49_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl49_rows)
            allocate(x_inl49_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1367 = 0, (x_inl1_n_sx_by) - 1
            do x_w1_1368 = 0, (np_particles) - 1
                x_inl49_rows((x_w1_1368) + 1, (x_w0_1367) + 1) = ((x_inl1_lox + x_inl1_j_by((x_w1_1368) + 1)) + &
                &x_inl49_taps((x_w0_1367) + 1))
            end do
        end do
        if (.not. allocated(x_inl49_gathered)) then
            allocate(x_inl49_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl49_gathered, 1) /= (np_particles) .or. size(x_inl49_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl49_gathered)
            allocate(x_inl49_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1369 = 0, (x_inl1_n_sx_by) - 1
            do x_w1_1370 = 0, (np_particles) - 1
                x_inl49_gathered((x_w1_1370) + 1, (x_w0_1369) + 1) = by_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl49_rows((x_w1_1370) + 1, (x_w0_1369) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb172)) then
            allocate(x_cb172(np_particles, ((o - gal) + 1)))
        else if (size(x_cb172, 1) /= (np_particles) .or. size(x_cb172, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb172)
            allocate(x_cb172(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1371 = 0, (((o - gal) + 1)) - 1
            do x_w1_1372 = 0, (np_particles) - 1
                x_cb172((x_w1_1372) + 1, (x_w0_1371) + 1) = (x_inl1_sx_by((x_w1_1372) + 1, (x_w0_1371) + 1) * &
                &x_inl49_gathered((x_w1_1372) + 1, (x_w0_1371) + 1))
            end do
        end do
        ! numpy: np.sum(__cb172, axis=0)
        do x_ax0_1373 = 0, (np_particles) - 1
            x_cb173((x_ax0_1373) + 1) = 0.0_c_double
            do x_rd0_1374 = 0, (((o - gal) + 1)) - 1
                x_cb173((x_ax0_1373) + 1) = (x_cb173((x_ax0_1373) + 1) + x_cb172((x_ax0_1373) + 1, (x_rd0_1374) + 1))
            end do
        end do
        do x_w0_1375 = 0, (np_particles) - 1
            x_inl1_Bthetap((x_w0_1375) + 1) = x_cb173((x_w0_1375) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, __inl1_rp, 1.0)
        do x_r0_1376 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_1376) + 1) > 0.0_c_double)) then
                x_ifexp39 = x_inl1_rp((x_r0_1376) + 1)
            else
                x_ifexp39 = 1.0_c_double
            end if
            x_cb174((x_r0_1376) + 1) = x_ifexp39
        end do
        do x_w0_1377 = 0, (np_particles) - 1
            x_inl1_rp_safe((x_w0_1377) + 1) = x_cb174((x_w0_1377) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, xp / __inl1_rp_safe, 1.0)
        do x_r0_1378 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_1378) + 1) > 0.0_c_double)) then
                x_ifexp40 = (xp((x_r0_1378) + 1) / x_inl1_rp_safe((x_r0_1378) + 1))
            else
                x_ifexp40 = 1.0_c_double
            end if
            x_cb175((x_r0_1378) + 1) = x_ifexp40
        end do
        do x_w0_1379 = 0, (np_particles) - 1
            x_inl1_costheta((x_w0_1379) + 1) = x_cb175((x_w0_1379) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, yp / __inl1_rp_safe, 0.0)
        do x_r0_1380 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_1380) + 1) > 0.0_c_double)) then
                x_ifexp41 = (yp((x_r0_1380) + 1) / x_inl1_rp_safe((x_r0_1380) + 1))
            else
                x_ifexp41 = 0.0_c_double
            end if
            x_cb176((x_r0_1380) + 1) = x_ifexp41
        end do
        do x_w0_1381 = 0, (np_particles) - 1
            x_inl1_sintheta((x_w0_1381) + 1) = x_cb176((x_w0_1381) + 1)
        end do
        do x_w0_1382 = 0, (np_particles) - 1
            Exp((x_w0_1382) + 1) = Exp((x_w0_1382) + 1) + (((x_inl1_costheta((x_w0_1382) + 1) * x_inl1_Erp((x_w0_1382) &
            &+ 1)) - (x_inl1_sintheta((x_w0_1382) + 1) * x_inl1_Ethetap((x_w0_1382) + 1))))
        end do
        do x_w0_1383 = 0, (np_particles) - 1
            Eyp((x_w0_1383) + 1) = Eyp((x_w0_1383) + 1) + (((x_inl1_costheta((x_w0_1383) + 1) * &
            &x_inl1_Ethetap((x_w0_1383) + 1)) + (x_inl1_sintheta((x_w0_1383) + 1) * x_inl1_Erp((x_w0_1383) + 1))))
        end do
        do x_w0_1384 = 0, (np_particles) - 1
            Bxp((x_w0_1384) + 1) = Bxp((x_w0_1384) + 1) + (((x_inl1_costheta((x_w0_1384) + 1) * x_inl1_Brp((x_w0_1384) &
            &+ 1)) - (x_inl1_sintheta((x_w0_1384) + 1) * x_inl1_Bthetap((x_w0_1384) + 1))))
        end do
        do x_w0_1385 = 0, (np_particles) - 1
            Byp((x_w0_1385) + 1) = Byp((x_w0_1385) + 1) + (((x_inl1_costheta((x_w0_1385) + 1) * &
            &x_inl1_Bthetap((x_w0_1385) + 1)) + (x_inl1_sintheta((x_w0_1385) + 1) * x_inl1_Brp((x_w0_1385) + 1))))
        end do
    else if ((g == 5)) then
        if (.not. allocated(x_cb177)) then
            allocate(x_cb177((o + 1)))
        else if (size(x_cb177, 1) /= ((o + 1))) then
            deallocate(x_cb177)
            allocate(x_cb177((o + 1)))
        end if
        if (.not. allocated(x_cb177)) then
            allocate(x_cb177((o + 1)))
        else if (size(x_cb177, 1) /= ((o + 1))) then
            deallocate(x_cb177)
            allocate(x_cb177((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ey)
        do x_i_1386 = 0, (x_inl1_n_sx_ey) - 1
            x_cb177((x_i_1386) + 1) = x_i_1386
        end do
        if (.not. allocated(x_inl50_taps)) then
            allocate(x_inl50_taps((o + 1)))
        else if (size(x_inl50_taps, 1) /= ((o + 1))) then
            deallocate(x_inl50_taps)
            allocate(x_inl50_taps((o + 1)))
        end if
        do x_w0_1387 = 0, (x_inl1_n_sx_ey) - 1
            x_inl50_taps((x_w0_1387) + 1) = x_cb177((x_w0_1387) + 1)
        end do
        if (.not. allocated(x_inl50_rows)) then
            allocate(x_inl50_rows(np_particles, (o + 1)))
        else if (size(x_inl50_rows, 1) /= (np_particles) .or. size(x_inl50_rows, 2) /= ((o + 1))) then
            deallocate(x_inl50_rows)
            allocate(x_inl50_rows(np_particles, (o + 1)))
        end if
        do x_w0_1388 = 0, (x_inl1_n_sx_ey) - 1
            do x_w1_1389 = 0, (np_particles) - 1
                x_inl50_rows((x_w1_1389) + 1, (x_w0_1388) + 1) = ((x_inl1_lox + x_inl1_j_ey((x_w1_1389) + 1)) + &
                &x_inl50_taps((x_w0_1388) + 1))
            end do
        end do
        if (.not. allocated(x_inl50_gathered)) then
            allocate(x_inl50_gathered(np_particles, (o + 1)))
        else if (size(x_inl50_gathered, 1) /= (np_particles) .or. size(x_inl50_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl50_gathered)
            allocate(x_inl50_gathered(np_particles, (o + 1)))
        end if
        do x_w0_1390 = 0, (x_inl1_n_sx_ey) - 1
            do x_w1_1391 = 0, (np_particles) - 1
                x_inl50_gathered((x_w1_1391) + 1, (x_w0_1390) + 1) = ey_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl50_rows((x_w1_1391) + 1, (x_w0_1390) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb178)) then
            allocate(x_cb178(np_particles, (o + 1)))
        else if (size(x_cb178, 1) /= (np_particles) .or. size(x_cb178, 2) /= ((o + 1))) then
            deallocate(x_cb178)
            allocate(x_cb178(np_particles, (o + 1)))
        end if
        do x_w0_1392 = 0, ((o + 1)) - 1
            do x_w1_1393 = 0, (np_particles) - 1
                x_cb178((x_w1_1393) + 1, (x_w0_1392) + 1) = (x_inl1_sx_ey((x_w1_1393) + 1, (x_w0_1392) + 1) * &
                &x_inl50_gathered((x_w1_1393) + 1, (x_w0_1392) + 1))
            end do
        end do
        ! numpy: np.sum(__cb178, axis=0)
        do x_ax0_1394 = 0, (np_particles) - 1
            x_cb179((x_ax0_1394) + 1) = 0.0_c_double
            do x_rd0_1395 = 0, ((o + 1)) - 1
                x_cb179((x_ax0_1394) + 1) = (x_cb179((x_ax0_1394) + 1) + x_cb178((x_ax0_1394) + 1, (x_rd0_1395) + 1))
            end do
        end do
        do x_w0_1396 = 0, (np_particles) - 1
            x_inl1_Ethetap((x_w0_1396) + 1) = x_cb179((x_w0_1396) + 1)
        end do
        if (.not. allocated(x_cb180)) then
            allocate(x_cb180(((o - gal) + 1)))
        else if (size(x_cb180, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb180)
            allocate(x_cb180(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb180)) then
            allocate(x_cb180(((o - gal) + 1)))
        else if (size(x_cb180, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb180)
            allocate(x_cb180(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ex)
        do x_i_1397 = 0, (x_inl1_n_sx_ex) - 1
            x_cb180((x_i_1397) + 1) = x_i_1397
        end do
        if (.not. allocated(x_inl51_taps)) then
            allocate(x_inl51_taps(((o - gal) + 1)))
        else if (size(x_inl51_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl51_taps)
            allocate(x_inl51_taps(((o - gal) + 1)))
        end if
        do x_w0_1398 = 0, (x_inl1_n_sx_ex) - 1
            x_inl51_taps((x_w0_1398) + 1) = x_cb180((x_w0_1398) + 1)
        end do
        if (.not. allocated(x_inl51_rows)) then
            allocate(x_inl51_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl51_rows, 1) /= (np_particles) .or. size(x_inl51_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl51_rows)
            allocate(x_inl51_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1399 = 0, (x_inl1_n_sx_ex) - 1
            do x_w1_1400 = 0, (np_particles) - 1
                x_inl51_rows((x_w1_1400) + 1, (x_w0_1399) + 1) = ((x_inl1_lox + x_inl1_j_ex((x_w1_1400) + 1)) + &
                &x_inl51_taps((x_w0_1399) + 1))
            end do
        end do
        if (.not. allocated(x_inl51_gathered)) then
            allocate(x_inl51_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl51_gathered, 1) /= (np_particles) .or. size(x_inl51_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl51_gathered)
            allocate(x_inl51_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1401 = 0, (x_inl1_n_sx_ex) - 1
            do x_w1_1402 = 0, (np_particles) - 1
                x_inl51_gathered((x_w1_1402) + 1, (x_w0_1401) + 1) = ex_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl51_rows((x_w1_1402) + 1, (x_w0_1401) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb181)) then
            allocate(x_cb181(np_particles, ((o - gal) + 1)))
        else if (size(x_cb181, 1) /= (np_particles) .or. size(x_cb181, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb181)
            allocate(x_cb181(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1403 = 0, (((o - gal) + 1)) - 1
            do x_w1_1404 = 0, (np_particles) - 1
                x_cb181((x_w1_1404) + 1, (x_w0_1403) + 1) = (x_inl1_sx_ex((x_w1_1404) + 1, (x_w0_1403) + 1) * &
                &x_inl51_gathered((x_w1_1404) + 1, (x_w0_1403) + 1))
            end do
        end do
        ! numpy: np.sum(__cb181, axis=0)
        do x_ax0_1405 = 0, (np_particles) - 1
            x_cb182((x_ax0_1405) + 1) = 0.0_c_double
            do x_rd0_1406 = 0, (((o - gal) + 1)) - 1
                x_cb182((x_ax0_1405) + 1) = (x_cb182((x_ax0_1405) + 1) + x_cb181((x_ax0_1405) + 1, (x_rd0_1406) + 1))
            end do
        end do
        do x_w0_1407 = 0, (np_particles) - 1
            x_inl1_Erp((x_w0_1407) + 1) = x_cb182((x_w0_1407) + 1)
        end do
        if (.not. allocated(x_cb183)) then
            allocate(x_cb183(((o - gal) + 1)))
        else if (size(x_cb183, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb183)
            allocate(x_cb183(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb183)) then
            allocate(x_cb183(((o - gal) + 1)))
        else if (size(x_cb183, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb183)
            allocate(x_cb183(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bz)
        do x_i_1408 = 0, (x_inl1_n_sx_bz) - 1
            x_cb183((x_i_1408) + 1) = x_i_1408
        end do
        if (.not. allocated(x_inl52_taps)) then
            allocate(x_inl52_taps(((o - gal) + 1)))
        else if (size(x_inl52_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl52_taps)
            allocate(x_inl52_taps(((o - gal) + 1)))
        end if
        do x_w0_1409 = 0, (x_inl1_n_sx_bz) - 1
            x_inl52_taps((x_w0_1409) + 1) = x_cb183((x_w0_1409) + 1)
        end do
        if (.not. allocated(x_inl52_rows)) then
            allocate(x_inl52_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl52_rows, 1) /= (np_particles) .or. size(x_inl52_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl52_rows)
            allocate(x_inl52_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1410 = 0, (x_inl1_n_sx_bz) - 1
            do x_w1_1411 = 0, (np_particles) - 1
                x_inl52_rows((x_w1_1411) + 1, (x_w0_1410) + 1) = ((x_inl1_lox + x_inl1_j_bz((x_w1_1411) + 1)) + &
                &x_inl52_taps((x_w0_1410) + 1))
            end do
        end do
        if (.not. allocated(x_inl52_gathered)) then
            allocate(x_inl52_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl52_gathered, 1) /= (np_particles) .or. size(x_inl52_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl52_gathered)
            allocate(x_inl52_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1412 = 0, (x_inl1_n_sx_bz) - 1
            do x_w1_1413 = 0, (np_particles) - 1
                x_inl52_gathered((x_w1_1413) + 1, (x_w0_1412) + 1) = bz_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl52_rows((x_w1_1413) + 1, (x_w0_1412) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb184)) then
            allocate(x_cb184(np_particles, ((o - gal) + 1)))
        else if (size(x_cb184, 1) /= (np_particles) .or. size(x_cb184, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb184)
            allocate(x_cb184(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1414 = 0, (((o - gal) + 1)) - 1
            do x_w1_1415 = 0, (np_particles) - 1
                x_cb184((x_w1_1415) + 1, (x_w0_1414) + 1) = (x_inl1_sx_bz((x_w1_1415) + 1, (x_w0_1414) + 1) * &
                &x_inl52_gathered((x_w1_1415) + 1, (x_w0_1414) + 1))
            end do
        end do
        ! numpy: np.sum(__cb184, axis=0)
        do x_ax0_1416 = 0, (np_particles) - 1
            x_cb185((x_ax0_1416) + 1) = 0.0_c_double
            do x_rd0_1417 = 0, (((o - gal) + 1)) - 1
                x_cb185((x_ax0_1416) + 1) = (x_cb185((x_ax0_1416) + 1) + x_cb184((x_ax0_1416) + 1, (x_rd0_1417) + 1))
            end do
        end do
        do x_w0_1418 = 0, (np_particles) - 1
            x_inl1_Bphip((x_w0_1418) + 1) = x_cb185((x_w0_1418) + 1)
        end do
        if (.not. allocated(x_cb186)) then
            allocate(x_cb186((o + 1)))
        else if (size(x_cb186, 1) /= ((o + 1))) then
            deallocate(x_cb186)
            allocate(x_cb186((o + 1)))
        end if
        if (.not. allocated(x_cb186)) then
            allocate(x_cb186((o + 1)))
        else if (size(x_cb186, 1) /= ((o + 1))) then
            deallocate(x_cb186)
            allocate(x_cb186((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ez)
        do x_i_1419 = 0, (x_inl1_n_sx_ez) - 1
            x_cb186((x_i_1419) + 1) = x_i_1419
        end do
        if (.not. allocated(x_inl53_taps)) then
            allocate(x_inl53_taps((o + 1)))
        else if (size(x_inl53_taps, 1) /= ((o + 1))) then
            deallocate(x_inl53_taps)
            allocate(x_inl53_taps((o + 1)))
        end if
        do x_w0_1420 = 0, (x_inl1_n_sx_ez) - 1
            x_inl53_taps((x_w0_1420) + 1) = x_cb186((x_w0_1420) + 1)
        end do
        if (.not. allocated(x_inl53_rows)) then
            allocate(x_inl53_rows(np_particles, (o + 1)))
        else if (size(x_inl53_rows, 1) /= (np_particles) .or. size(x_inl53_rows, 2) /= ((o + 1))) then
            deallocate(x_inl53_rows)
            allocate(x_inl53_rows(np_particles, (o + 1)))
        end if
        do x_w0_1421 = 0, (x_inl1_n_sx_ez) - 1
            do x_w1_1422 = 0, (np_particles) - 1
                x_inl53_rows((x_w1_1422) + 1, (x_w0_1421) + 1) = ((x_inl1_lox + x_inl1_j_ez((x_w1_1422) + 1)) + &
                &x_inl53_taps((x_w0_1421) + 1))
            end do
        end do
        if (.not. allocated(x_inl53_gathered)) then
            allocate(x_inl53_gathered(np_particles, (o + 1)))
        else if (size(x_inl53_gathered, 1) /= (np_particles) .or. size(x_inl53_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl53_gathered)
            allocate(x_inl53_gathered(np_particles, (o + 1)))
        end if
        do x_w0_1423 = 0, (x_inl1_n_sx_ez) - 1
            do x_w1_1424 = 0, (np_particles) - 1
                x_inl53_gathered((x_w1_1424) + 1, (x_w0_1423) + 1) = ez_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl53_rows((x_w1_1424) + 1, (x_w0_1423) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb187)) then
            allocate(x_cb187(np_particles, (o + 1)))
        else if (size(x_cb187, 1) /= (np_particles) .or. size(x_cb187, 2) /= ((o + 1))) then
            deallocate(x_cb187)
            allocate(x_cb187(np_particles, (o + 1)))
        end if
        do x_w0_1425 = 0, ((o + 1)) - 1
            do x_w1_1426 = 0, (np_particles) - 1
                x_cb187((x_w1_1426) + 1, (x_w0_1425) + 1) = (x_inl1_sx_ez((x_w1_1426) + 1, (x_w0_1425) + 1) * &
                &x_inl53_gathered((x_w1_1426) + 1, (x_w0_1425) + 1))
            end do
        end do
        ! numpy: np.sum(__cb187, axis=0)
        do x_ax0_1427 = 0, (np_particles) - 1
            x_cb188((x_ax0_1427) + 1) = 0.0_c_double
            do x_rd0_1428 = 0, ((o + 1)) - 1
                x_cb188((x_ax0_1427) + 1) = (x_cb188((x_ax0_1427) + 1) + x_cb187((x_ax0_1427) + 1, (x_rd0_1428) + 1))
            end do
        end do
        do x_w0_1429 = 0, (np_particles) - 1
            x_inl1_Ephip((x_w0_1429) + 1) = x_cb188((x_w0_1429) + 1)
        end do
        if (.not. allocated(x_cb189)) then
            allocate(x_cb189((o + 1)))
        else if (size(x_cb189, 1) /= ((o + 1))) then
            deallocate(x_cb189)
            allocate(x_cb189((o + 1)))
        end if
        if (.not. allocated(x_cb189)) then
            allocate(x_cb189((o + 1)))
        else if (size(x_cb189, 1) /= ((o + 1))) then
            deallocate(x_cb189)
            allocate(x_cb189((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bx)
        do x_i_1430 = 0, (x_inl1_n_sx_bx) - 1
            x_cb189((x_i_1430) + 1) = x_i_1430
        end do
        if (.not. allocated(x_inl54_taps)) then
            allocate(x_inl54_taps((o + 1)))
        else if (size(x_inl54_taps, 1) /= ((o + 1))) then
            deallocate(x_inl54_taps)
            allocate(x_inl54_taps((o + 1)))
        end if
        do x_w0_1431 = 0, (x_inl1_n_sx_bx) - 1
            x_inl54_taps((x_w0_1431) + 1) = x_cb189((x_w0_1431) + 1)
        end do
        if (.not. allocated(x_inl54_rows)) then
            allocate(x_inl54_rows(np_particles, (o + 1)))
        else if (size(x_inl54_rows, 1) /= (np_particles) .or. size(x_inl54_rows, 2) /= ((o + 1))) then
            deallocate(x_inl54_rows)
            allocate(x_inl54_rows(np_particles, (o + 1)))
        end if
        do x_w0_1432 = 0, (x_inl1_n_sx_bx) - 1
            do x_w1_1433 = 0, (np_particles) - 1
                x_inl54_rows((x_w1_1433) + 1, (x_w0_1432) + 1) = ((x_inl1_lox + x_inl1_j_bx((x_w1_1433) + 1)) + &
                &x_inl54_taps((x_w0_1432) + 1))
            end do
        end do
        if (.not. allocated(x_inl54_gathered)) then
            allocate(x_inl54_gathered(np_particles, (o + 1)))
        else if (size(x_inl54_gathered, 1) /= (np_particles) .or. size(x_inl54_gathered, 2) /= ((o + 1))) then
            deallocate(x_inl54_gathered)
            allocate(x_inl54_gathered(np_particles, (o + 1)))
        end if
        do x_w0_1434 = 0, (x_inl1_n_sx_bx) - 1
            do x_w1_1435 = 0, (np_particles) - 1
                x_inl54_gathered((x_w1_1435) + 1, (x_w0_1434) + 1) = bx_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl54_rows((x_w1_1435) + 1, (x_w0_1434) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb190)) then
            allocate(x_cb190(np_particles, (o + 1)))
        else if (size(x_cb190, 1) /= (np_particles) .or. size(x_cb190, 2) /= ((o + 1))) then
            deallocate(x_cb190)
            allocate(x_cb190(np_particles, (o + 1)))
        end if
        do x_w0_1436 = 0, ((o + 1)) - 1
            do x_w1_1437 = 0, (np_particles) - 1
                x_cb190((x_w1_1437) + 1, (x_w0_1436) + 1) = (x_inl1_sx_bx((x_w1_1437) + 1, (x_w0_1436) + 1) * &
                &x_inl54_gathered((x_w1_1437) + 1, (x_w0_1436) + 1))
            end do
        end do
        ! numpy: np.sum(__cb190, axis=0)
        do x_ax0_1438 = 0, (np_particles) - 1
            x_cb191((x_ax0_1438) + 1) = 0.0_c_double
            do x_rd0_1439 = 0, ((o + 1)) - 1
                x_cb191((x_ax0_1438) + 1) = (x_cb191((x_ax0_1438) + 1) + x_cb190((x_ax0_1438) + 1, (x_rd0_1439) + 1))
            end do
        end do
        do x_w0_1440 = 0, (np_particles) - 1
            x_inl1_Brp((x_w0_1440) + 1) = x_cb191((x_w0_1440) + 1)
        end do
        if (.not. allocated(x_cb192)) then
            allocate(x_cb192(((o - gal) + 1)))
        else if (size(x_cb192, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb192)
            allocate(x_cb192(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb192)) then
            allocate(x_cb192(((o - gal) + 1)))
        else if (size(x_cb192, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb192)
            allocate(x_cb192(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_by)
        do x_i_1441 = 0, (x_inl1_n_sx_by) - 1
            x_cb192((x_i_1441) + 1) = x_i_1441
        end do
        if (.not. allocated(x_inl55_taps)) then
            allocate(x_inl55_taps(((o - gal) + 1)))
        else if (size(x_inl55_taps, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl55_taps)
            allocate(x_inl55_taps(((o - gal) + 1)))
        end if
        do x_w0_1442 = 0, (x_inl1_n_sx_by) - 1
            x_inl55_taps((x_w0_1442) + 1) = x_cb192((x_w0_1442) + 1)
        end do
        if (.not. allocated(x_inl55_rows)) then
            allocate(x_inl55_rows(np_particles, ((o - gal) + 1)))
        else if (size(x_inl55_rows, 1) /= (np_particles) .or. size(x_inl55_rows, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl55_rows)
            allocate(x_inl55_rows(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1443 = 0, (x_inl1_n_sx_by) - 1
            do x_w1_1444 = 0, (np_particles) - 1
                x_inl55_rows((x_w1_1444) + 1, (x_w0_1443) + 1) = ((x_inl1_lox + x_inl1_j_by((x_w1_1444) + 1)) + &
                &x_inl55_taps((x_w0_1443) + 1))
            end do
        end do
        if (.not. allocated(x_inl55_gathered)) then
            allocate(x_inl55_gathered(np_particles, ((o - gal) + 1)))
        else if (size(x_inl55_gathered, 1) /= (np_particles) .or. size(x_inl55_gathered, 2) /= (((o - gal) + 1))) then
            deallocate(x_inl55_gathered)
            allocate(x_inl55_gathered(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1445 = 0, (x_inl1_n_sx_by) - 1
            do x_w1_1446 = 0, (np_particles) - 1
                x_inl55_gathered((x_w1_1446) + 1, (x_w0_1445) + 1) = by_arr((0) + 1, (0) + 1, (0) + 1, &
                &(x_inl55_rows((x_w1_1446) + 1, (x_w0_1445) + 1)) + 1)
            end do
        end do
        if (.not. allocated(x_cb193)) then
            allocate(x_cb193(np_particles, ((o - gal) + 1)))
        else if (size(x_cb193, 1) /= (np_particles) .or. size(x_cb193, 2) /= (((o - gal) + 1))) then
            deallocate(x_cb193)
            allocate(x_cb193(np_particles, ((o - gal) + 1)))
        end if
        do x_w0_1447 = 0, (((o - gal) + 1)) - 1
            do x_w1_1448 = 0, (np_particles) - 1
                x_cb193((x_w1_1448) + 1, (x_w0_1447) + 1) = (x_inl1_sx_by((x_w1_1448) + 1, (x_w0_1447) + 1) * &
                &x_inl55_gathered((x_w1_1448) + 1, (x_w0_1447) + 1))
            end do
        end do
        ! numpy: np.sum(__cb193, axis=0)
        do x_ax0_1449 = 0, (np_particles) - 1
            x_cb194((x_ax0_1449) + 1) = 0.0_c_double
            do x_rd0_1450 = 0, (((o - gal) + 1)) - 1
                x_cb194((x_ax0_1449) + 1) = (x_cb194((x_ax0_1449) + 1) + x_cb193((x_ax0_1449) + 1, (x_rd0_1450) + 1))
            end do
        end do
        do x_w0_1451 = 0, (np_particles) - 1
            x_inl1_Bthetap((x_w0_1451) + 1) = x_cb194((x_w0_1451) + 1)
        end do
        ! numpy: np.sqrt(xp * xp + yp * yp)
        do x_r0_1452 = 0, (np_particles) - 1
            x_cb195((x_r0_1452) + 1) = SQRT(((xp((x_r0_1452) + 1) * xp((x_r0_1452) + 1)) + (yp((x_r0_1452) + 1) * &
            &yp((x_r0_1452) + 1))))
        end do
        do x_w0_1453 = 0, (np_particles) - 1
            x_inl1_rpxy((x_w0_1453) + 1) = x_cb195((x_w0_1453) + 1)
        end do
        ! numpy: np.where(__inl1_rpxy > 0.0, __inl1_rpxy, 1.0)
        do x_r0_1454 = 0, (np_particles) - 1
            if ((x_inl1_rpxy((x_r0_1454) + 1) > 0.0_c_double)) then
                x_ifexp42 = x_inl1_rpxy((x_r0_1454) + 1)
            else
                x_ifexp42 = 1.0_c_double
            end if
            x_cb196((x_r0_1454) + 1) = x_ifexp42
        end do
        do x_w0_1455 = 0, (np_particles) - 1
            x_inl1_rpxy_safe((x_w0_1455) + 1) = x_cb196((x_w0_1455) + 1)
        end do
        ! numpy: np.where(__inl1_rpxy > 0.0, xp / __inl1_rpxy_safe, 1.0)
        do x_r0_1456 = 0, (np_particles) - 1
            if ((x_inl1_rpxy((x_r0_1456) + 1) > 0.0_c_double)) then
                x_ifexp43 = (xp((x_r0_1456) + 1) / x_inl1_rpxy_safe((x_r0_1456) + 1))
            else
                x_ifexp43 = 1.0_c_double
            end if
            x_cb197((x_r0_1456) + 1) = x_ifexp43
        end do
        do x_w0_1457 = 0, (np_particles) - 1
            x_inl1_costheta((x_w0_1457) + 1) = x_cb197((x_w0_1457) + 1)
        end do
        ! numpy: np.where(__inl1_rpxy > 0.0, yp / __inl1_rpxy_safe, 0.0)
        do x_r0_1458 = 0, (np_particles) - 1
            if ((x_inl1_rpxy((x_r0_1458) + 1) > 0.0_c_double)) then
                x_ifexp44 = (yp((x_r0_1458) + 1) / x_inl1_rpxy_safe((x_r0_1458) + 1))
            else
                x_ifexp44 = 0.0_c_double
            end if
            x_cb198((x_r0_1458) + 1) = x_ifexp44
        end do
        do x_w0_1459 = 0, (np_particles) - 1
            x_inl1_sintheta((x_w0_1459) + 1) = x_cb198((x_w0_1459) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, __inl1_rp, 1.0)
        do x_r0_1460 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_1460) + 1) > 0.0_c_double)) then
                x_ifexp45 = x_inl1_rp((x_r0_1460) + 1)
            else
                x_ifexp45 = 1.0_c_double
            end if
            x_cb199((x_r0_1460) + 1) = x_ifexp45
        end do
        do x_w0_1461 = 0, (np_particles) - 1
            x_inl1_rp_safe((x_w0_1461) + 1) = x_cb199((x_w0_1461) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, __inl1_rpxy / __inl1_rp_safe, 1.0)
        do x_r0_1462 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_1462) + 1) > 0.0_c_double)) then
                x_ifexp46 = (x_inl1_rpxy((x_r0_1462) + 1) / x_inl1_rp_safe((x_r0_1462) + 1))
            else
                x_ifexp46 = 1.0_c_double
            end if
            x_cb200((x_r0_1462) + 1) = x_ifexp46
        end do
        do x_w0_1463 = 0, (np_particles) - 1
            x_inl1_cosphi((x_w0_1463) + 1) = x_cb200((x_w0_1463) + 1)
        end do
        ! numpy: np.where(__inl1_rp > 0.0, zp / __inl1_rp_safe, 0.0)
        do x_r0_1464 = 0, (np_particles) - 1
            if ((x_inl1_rp((x_r0_1464) + 1) > 0.0_c_double)) then
                x_ifexp47 = (zp((x_r0_1464) + 1) / x_inl1_rp_safe((x_r0_1464) + 1))
            else
                x_ifexp47 = 0.0_c_double
            end if
            x_cb201((x_r0_1464) + 1) = x_ifexp47
        end do
        do x_w0_1465 = 0, (np_particles) - 1
            x_inl1_sinphi((x_w0_1465) + 1) = x_cb201((x_w0_1465) + 1)
        end do
        do x_w0_1466 = 0, (np_particles) - 1
            Exp((x_w0_1466) + 1) = Exp((x_w0_1466) + 1) + (((((x_inl1_costheta((x_w0_1466) + 1) * &
            &x_inl1_cosphi((x_w0_1466) + 1)) * x_inl1_Erp((x_w0_1466) + 1)) - (x_inl1_sintheta((x_w0_1466) + 1) * &
            &x_inl1_Ethetap((x_w0_1466) + 1))) - ((x_inl1_costheta((x_w0_1466) + 1) * x_inl1_sinphi((x_w0_1466) + 1)) &
            &* x_inl1_Ephip((x_w0_1466) + 1))))
        end do
        do x_w0_1467 = 0, (np_particles) - 1
            Eyp((x_w0_1467) + 1) = Eyp((x_w0_1467) + 1) + (((((x_inl1_sintheta((x_w0_1467) + 1) * &
            &x_inl1_cosphi((x_w0_1467) + 1)) * x_inl1_Erp((x_w0_1467) + 1)) + (x_inl1_costheta((x_w0_1467) + 1) * &
            &x_inl1_Ethetap((x_w0_1467) + 1))) - ((x_inl1_sintheta((x_w0_1467) + 1) * x_inl1_sinphi((x_w0_1467) + 1)) &
            &* x_inl1_Ephip((x_w0_1467) + 1))))
        end do
        do x_w0_1468 = 0, (np_particles) - 1
            Ezp((x_w0_1468) + 1) = Ezp((x_w0_1468) + 1) + (((x_inl1_sinphi((x_w0_1468) + 1) * x_inl1_Erp((x_w0_1468) + &
            &1)) + (x_inl1_cosphi((x_w0_1468) + 1) * x_inl1_Ephip((x_w0_1468) + 1))))
        end do
        do x_w0_1469 = 0, (np_particles) - 1
            Bxp((x_w0_1469) + 1) = Bxp((x_w0_1469) + 1) + (((((x_inl1_costheta((x_w0_1469) + 1) * &
            &x_inl1_cosphi((x_w0_1469) + 1)) * x_inl1_Brp((x_w0_1469) + 1)) - (x_inl1_sintheta((x_w0_1469) + 1) * &
            &x_inl1_Bthetap((x_w0_1469) + 1))) - ((x_inl1_costheta((x_w0_1469) + 1) * x_inl1_sinphi((x_w0_1469) + 1)) &
            &* x_inl1_Bphip((x_w0_1469) + 1))))
        end do
        do x_w0_1470 = 0, (np_particles) - 1
            Byp((x_w0_1470) + 1) = Byp((x_w0_1470) + 1) + (((((x_inl1_sintheta((x_w0_1470) + 1) * &
            &x_inl1_cosphi((x_w0_1470) + 1)) * x_inl1_Brp((x_w0_1470) + 1)) + (x_inl1_costheta((x_w0_1470) + 1) * &
            &x_inl1_Bthetap((x_w0_1470) + 1))) - ((x_inl1_sintheta((x_w0_1470) + 1) * x_inl1_sinphi((x_w0_1470) + 1)) &
            &* x_inl1_Bphip((x_w0_1470) + 1))))
        end do
        do x_w0_1471 = 0, (np_particles) - 1
            Bzp((x_w0_1471) + 1) = Bzp((x_w0_1471) + 1) + (((x_inl1_sinphi((x_w0_1471) + 1) * x_inl1_Brp((x_w0_1471) + &
            &1)) + (x_inl1_cosphi((x_w0_1471) + 1) * x_inl1_Bphip((x_w0_1471) + 1))))
        end do
    else
        if (.not. allocated(x_cb202)) then
            allocate(x_cb202(((o - gal) + 1)))
        else if (size(x_cb202, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb202)
            allocate(x_cb202(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb202)) then
            allocate(x_cb202(((o - gal) + 1)))
        else if (size(x_cb202, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb202)
            allocate(x_cb202(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ex)
        do x_i_1472 = 0, (x_inl1_n_sx_ex) - 1
            x_cb202((x_i_1472) + 1) = x_i_1472
        end do
        if (.not. allocated(x_inl56_tx)) then
            allocate(x_inl56_tx(((o - gal) + 1)))
        else if (size(x_inl56_tx, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl56_tx)
            allocate(x_inl56_tx(((o - gal) + 1)))
        end if
        do x_w0_1473 = 0, (x_inl1_n_sx_ex) - 1
            x_inl56_tx((x_w0_1473) + 1) = x_cb202((x_w0_1473) + 1)
        end do
        if (.not. allocated(x_cb203)) then
            allocate(x_cb203((o + 1)))
        else if (size(x_cb203, 1) /= ((o + 1))) then
            deallocate(x_cb203)
            allocate(x_cb203((o + 1)))
        end if
        if (.not. allocated(x_cb203)) then
            allocate(x_cb203((o + 1)))
        else if (size(x_cb203, 1) /= ((o + 1))) then
            deallocate(x_cb203)
            allocate(x_cb203((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sy_ex)
        do x_i_1474 = 0, (x_inl1_n_sy_ex) - 1
            x_cb203((x_i_1474) + 1) = x_i_1474
        end do
        if (.not. allocated(x_inl56_ty)) then
            allocate(x_inl56_ty((o + 1)))
        else if (size(x_inl56_ty, 1) /= ((o + 1))) then
            deallocate(x_inl56_ty)
            allocate(x_inl56_ty((o + 1)))
        end if
        do x_w0_1475 = 0, (x_inl1_n_sy_ex) - 1
            x_inl56_ty((x_w0_1475) + 1) = x_cb203((x_w0_1475) + 1)
        end do
        if (.not. allocated(x_cb204)) then
            allocate(x_cb204((o + 1)))
        else if (size(x_cb204, 1) /= ((o + 1))) then
            deallocate(x_cb204)
            allocate(x_cb204((o + 1)))
        end if
        if (.not. allocated(x_cb204)) then
            allocate(x_cb204((o + 1)))
        else if (size(x_cb204, 1) /= ((o + 1))) then
            deallocate(x_cb204)
            allocate(x_cb204((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ex)
        do x_i_1476 = 0, (x_inl1_n_sz_ex) - 1
            x_cb204((x_i_1476) + 1) = x_i_1476
        end do
        if (.not. allocated(x_inl56_tz)) then
            allocate(x_inl56_tz((o + 1)))
        else if (size(x_inl56_tz, 1) /= ((o + 1))) then
            deallocate(x_inl56_tz)
            allocate(x_inl56_tz((o + 1)))
        end if
        do x_w0_1477 = 0, (x_inl1_n_sz_ex) - 1
            x_inl56_tz((x_w0_1477) + 1) = x_cb204((x_w0_1477) + 1)
        end do
        if (.not. allocated(x_inl56_ix)) then
            allocate(x_inl56_ix(np_particles, 1, 1, ((o - gal) + 1)))
        else if (size(x_inl56_ix, 1) /= (np_particles) .or. size(x_inl56_ix, 2) /= (1) .or. size(x_inl56_ix, 3) /= (1) &
        &.or. size(x_inl56_ix, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl56_ix)
            allocate(x_inl56_ix(np_particles, 1, 1, ((o - gal) + 1)))
        end if
        do si0_l1478 = 0, (((o - gal) + 1)) - 1
            do si1_l1479 = 0, (1) - 1
                do si2_l1480 = 0, (1) - 1
                    do si3_l1481 = 0, (np_particles) - 1
                        x_inl56_ix((si3_l1481) + 1, (si2_l1480) + 1, (si1_l1479) + 1, (si0_l1478) + 1) = ((x_inl1_lox &
                        &+ x_inl1_j_ex((si3_l1481) + 1)) + x_inl56_tx((si0_l1478) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_iy)) then
            allocate(x_inl56_iy(np_particles, 1, (o + 1), 1))
        else if (size(x_inl56_iy, 1) /= (np_particles) .or. size(x_inl56_iy, 2) /= (1) .or. size(x_inl56_iy, 3) /= ((o &
        &+ 1)) .or. size(x_inl56_iy, 4) /= (1)) then
            deallocate(x_inl56_iy)
            allocate(x_inl56_iy(np_particles, 1, (o + 1), 1))
        end if
        do si0_l1482 = 0, (1) - 1
            do si1_l1483 = 0, ((o + 1)) - 1
                do si2_l1484 = 0, (1) - 1
                    do si3_l1485 = 0, (np_particles) - 1
                        x_inl56_iy((si3_l1485) + 1, (si2_l1484) + 1, (si1_l1483) + 1, (si0_l1482) + 1) = ((x_inl1_loy &
                        &+ x_inl1_k_ex((si3_l1485) + 1)) + x_inl56_ty((si1_l1483) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_iz)) then
            allocate(x_inl56_iz(np_particles, (o + 1), 1, 1))
        else if (size(x_inl56_iz, 1) /= (np_particles) .or. size(x_inl56_iz, 2) /= ((o + 1)) .or. size(x_inl56_iz, 3) &
        &/= (1) .or. size(x_inl56_iz, 4) /= (1)) then
            deallocate(x_inl56_iz)
            allocate(x_inl56_iz(np_particles, (o + 1), 1, 1))
        end if
        do si0_l1486 = 0, (1) - 1
            do si1_l1487 = 0, (1) - 1
                do si2_l1488 = 0, ((o + 1)) - 1
                    do si3_l1489 = 0, (np_particles) - 1
                        x_inl56_iz((si3_l1489) + 1, (si2_l1488) + 1, (si1_l1487) + 1, (si0_l1486) + 1) = ((x_inl1_loz &
                        &+ x_inl1_l_ex((si3_l1489) + 1)) + x_inl56_tz((si2_l1488) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_ix_b)) then
            allocate(x_inl56_ix_b(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl56_ix_b, 1) /= (np_particles) .or. size(x_inl56_ix_b, 2) /= ((o + 1)) .or. &
        &size(x_inl56_ix_b, 3) /= ((o + 1)) .or. size(x_inl56_ix_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl56_ix_b)
            allocate(x_inl56_ix_b(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        end if
        do si0_l1490 = 0, (((o - gal) + 1)) - 1
            do si1_l1491 = 0, ((o + 1)) - 1
                do si2_l1492 = 0, ((o + 1)) - 1
                    do si3_l1493 = 0, (np_particles) - 1
                        x_inl56_ix_b((si3_l1493) + 1, (si2_l1492) + 1, (si1_l1491) + 1, (si0_l1490) + 1) = &
                        &x_inl56_ix((si3_l1493) + 1, (0) + 1, (0) + 1, (si0_l1490) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_iy_b)) then
            allocate(x_inl56_iy_b(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl56_iy_b, 1) /= (np_particles) .or. size(x_inl56_iy_b, 2) /= ((o + 1)) .or. &
        &size(x_inl56_iy_b, 3) /= ((o + 1)) .or. size(x_inl56_iy_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl56_iy_b)
            allocate(x_inl56_iy_b(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        end if
        do si0_l1494 = 0, (((o - gal) + 1)) - 1
            do si1_l1495 = 0, ((o + 1)) - 1
                do si2_l1496 = 0, ((o + 1)) - 1
                    do si3_l1497 = 0, (np_particles) - 1
                        x_inl56_iy_b((si3_l1497) + 1, (si2_l1496) + 1, (si1_l1495) + 1, (si0_l1494) + 1) = &
                        &x_inl56_iy((si3_l1497) + 1, (0) + 1, (si1_l1495) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_iz_b)) then
            allocate(x_inl56_iz_b(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl56_iz_b, 1) /= (np_particles) .or. size(x_inl56_iz_b, 2) /= ((o + 1)) .or. &
        &size(x_inl56_iz_b, 3) /= ((o + 1)) .or. size(x_inl56_iz_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl56_iz_b)
            allocate(x_inl56_iz_b(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        end if
        do si0_l1498 = 0, (((o - gal) + 1)) - 1
            do si1_l1499 = 0, ((o + 1)) - 1
                do si2_l1500 = 0, ((o + 1)) - 1
                    do si3_l1501 = 0, (np_particles) - 1
                        x_inl56_iz_b((si3_l1501) + 1, (si2_l1500) + 1, (si1_l1499) + 1, (si0_l1498) + 1) = &
                        &x_inl56_iz((si3_l1501) + 1, (si2_l1500) + 1, (0) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_gathered)) then
            allocate(x_inl56_gathered(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl56_gathered, 1) /= (np_particles) .or. size(x_inl56_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl56_gathered, 3) /= ((o + 1)) .or. size(x_inl56_gathered, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl56_gathered)
            allocate(x_inl56_gathered(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_1502 = 0, (((o - gal) + 1)) - 1
            do x_w1_1503 = 0, ((o + 1)) - 1
                do x_w2_1504 = 0, ((o + 1)) - 1
                    do x_w3_1505 = 0, (np_particles) - 1
                        x_inl56_gathered((x_w3_1505) + 1, (x_w2_1504) + 1, (x_w1_1503) + 1, (x_w0_1502) + 1) = &
                        &ex_arr((0) + 1, (x_inl56_iz_b((x_w3_1505) + 1, (x_w2_1504) + 1, (x_w1_1503) + 1, (x_w0_1502) &
                        &+ 1)) + 1, (x_inl56_iy_b((x_w3_1505) + 1, (x_w2_1504) + 1, (x_w1_1503) + 1, (x_w0_1502) + 1)) &
                        &+ 1, (x_inl56_ix_b((x_w3_1505) + 1, (x_w2_1504) + 1, (x_w1_1503) + 1, (x_w0_1502) + 1)) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl56_weight)) then
            allocate(x_inl56_weight(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl56_weight, 1) /= (np_particles) .or. size(x_inl56_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl56_weight, 3) /= ((o + 1)) .or. size(x_inl56_weight, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl56_weight)
            allocate(x_inl56_weight(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_1506 = 0, (((o - gal) + 1)) - 1
            do x_w1_1507 = 0, ((o + 1)) - 1
                do x_w2_1508 = 0, ((o + 1)) - 1
                    do x_w3_1509 = 0, (np_particles) - 1
                        x_inl56_weight((x_w3_1509) + 1, (x_w2_1508) + 1, (x_w1_1507) + 1, (x_w0_1506) + 1) = &
                        &((x_inl1_sx_ex((x_w3_1509) + 1, (x_w0_1506) + 1) * x_inl1_sy_ex((x_w3_1509) + 1, (x_w1_1507) &
                        &+ 1)) * x_inl1_sz_ex((x_w3_1509) + 1, (x_w2_1508) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_cb205)) then
            allocate(x_cb205(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_cb205, 1) /= (np_particles) .or. size(x_cb205, 2) /= ((o + 1)) .or. size(x_cb205, 3) /= ((o + &
        &1)) .or. size(x_cb205, 4) /= (((o - gal) + 1))) then
            deallocate(x_cb205)
            allocate(x_cb205(np_particles, (o + 1), (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_1510 = 0, (((o - gal) + 1)) - 1
            do x_w1_1511 = 0, ((o + 1)) - 1
                do x_w2_1512 = 0, ((o + 1)) - 1
                    do x_w3_1513 = 0, (np_particles) - 1
                        x_cb205((x_w3_1513) + 1, (x_w2_1512) + 1, (x_w1_1511) + 1, (x_w0_1510) + 1) = &
                        &(x_inl56_weight((x_w3_1513) + 1, (x_w2_1512) + 1, (x_w1_1511) + 1, (x_w0_1510) + 1) * &
                        &x_inl56_gathered((x_w3_1513) + 1, (x_w2_1512) + 1, (x_w1_1511) + 1, (x_w0_1510) + 1))
                    end do
                end do
            end do
        end do
        ! numpy: np.sum(__cb205, axis=(0, 1, 2))
        do x_ax0_1514 = 0, (np_particles) - 1
            x_cb206((x_ax0_1514) + 1) = 0.0_c_double
            do x_rd0_1515 = 0, (((o - gal) + 1)) - 1
                do x_rd1_1516 = 0, ((o + 1)) - 1
                    do x_rd2_1517 = 0, ((o + 1)) - 1
                        x_cb206((x_ax0_1514) + 1) = (x_cb206((x_ax0_1514) + 1) + x_cb205((x_ax0_1514) + 1, &
                        &(x_rd2_1517) + 1, (x_rd1_1516) + 1, (x_rd0_1515) + 1))
                    end do
                end do
            end do
        end do
        do x_w0_1518 = 0, (np_particles) - 1
            x_hcall29((x_w0_1518) + 1) = x_cb206((x_w0_1518) + 1)
        end do
        do x_w0_1519 = 0, (np_particles) - 1
            Exp((x_w0_1519) + 1) = Exp((x_w0_1519) + 1) + (x_hcall29((x_w0_1519) + 1))
        end do
        if (.not. allocated(x_cb207)) then
            allocate(x_cb207((o + 1)))
        else if (size(x_cb207, 1) /= ((o + 1))) then
            deallocate(x_cb207)
            allocate(x_cb207((o + 1)))
        end if
        if (.not. allocated(x_cb207)) then
            allocate(x_cb207((o + 1)))
        else if (size(x_cb207, 1) /= ((o + 1))) then
            deallocate(x_cb207)
            allocate(x_cb207((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ey)
        do x_i_1520 = 0, (x_inl1_n_sx_ey) - 1
            x_cb207((x_i_1520) + 1) = x_i_1520
        end do
        if (.not. allocated(x_inl57_tx)) then
            allocate(x_inl57_tx((o + 1)))
        else if (size(x_inl57_tx, 1) /= ((o + 1))) then
            deallocate(x_inl57_tx)
            allocate(x_inl57_tx((o + 1)))
        end if
        do x_w0_1521 = 0, (x_inl1_n_sx_ey) - 1
            x_inl57_tx((x_w0_1521) + 1) = x_cb207((x_w0_1521) + 1)
        end do
        if (.not. allocated(x_cb208)) then
            allocate(x_cb208(((o - gal) + 1)))
        else if (size(x_cb208, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb208)
            allocate(x_cb208(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb208)) then
            allocate(x_cb208(((o - gal) + 1)))
        else if (size(x_cb208, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb208)
            allocate(x_cb208(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sy_ey)
        do x_i_1522 = 0, (x_inl1_n_sy_ey) - 1
            x_cb208((x_i_1522) + 1) = x_i_1522
        end do
        if (.not. allocated(x_inl57_ty)) then
            allocate(x_inl57_ty(((o - gal) + 1)))
        else if (size(x_inl57_ty, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl57_ty)
            allocate(x_inl57_ty(((o - gal) + 1)))
        end if
        do x_w0_1523 = 0, (x_inl1_n_sy_ey) - 1
            x_inl57_ty((x_w0_1523) + 1) = x_cb208((x_w0_1523) + 1)
        end do
        if (.not. allocated(x_cb209)) then
            allocate(x_cb209((o + 1)))
        else if (size(x_cb209, 1) /= ((o + 1))) then
            deallocate(x_cb209)
            allocate(x_cb209((o + 1)))
        end if
        if (.not. allocated(x_cb209)) then
            allocate(x_cb209((o + 1)))
        else if (size(x_cb209, 1) /= ((o + 1))) then
            deallocate(x_cb209)
            allocate(x_cb209((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ey)
        do x_i_1524 = 0, (x_inl1_n_sz_ey) - 1
            x_cb209((x_i_1524) + 1) = x_i_1524
        end do
        if (.not. allocated(x_inl57_tz)) then
            allocate(x_inl57_tz((o + 1)))
        else if (size(x_inl57_tz, 1) /= ((o + 1))) then
            deallocate(x_inl57_tz)
            allocate(x_inl57_tz((o + 1)))
        end if
        do x_w0_1525 = 0, (x_inl1_n_sz_ey) - 1
            x_inl57_tz((x_w0_1525) + 1) = x_cb209((x_w0_1525) + 1)
        end do
        if (.not. allocated(x_inl57_ix)) then
            allocate(x_inl57_ix(np_particles, 1, 1, (o + 1)))
        else if (size(x_inl57_ix, 1) /= (np_particles) .or. size(x_inl57_ix, 2) /= (1) .or. size(x_inl57_ix, 3) /= (1) &
        &.or. size(x_inl57_ix, 4) /= ((o + 1))) then
            deallocate(x_inl57_ix)
            allocate(x_inl57_ix(np_particles, 1, 1, (o + 1)))
        end if
        do si0_l1526 = 0, ((o + 1)) - 1
            do si1_l1527 = 0, (1) - 1
                do si2_l1528 = 0, (1) - 1
                    do si3_l1529 = 0, (np_particles) - 1
                        x_inl57_ix((si3_l1529) + 1, (si2_l1528) + 1, (si1_l1527) + 1, (si0_l1526) + 1) = ((x_inl1_lox &
                        &+ x_inl1_j_ey((si3_l1529) + 1)) + x_inl57_tx((si0_l1526) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_iy)) then
            allocate(x_inl57_iy(np_particles, 1, ((o - gal) + 1), 1))
        else if (size(x_inl57_iy, 1) /= (np_particles) .or. size(x_inl57_iy, 2) /= (1) .or. size(x_inl57_iy, 3) /= &
        &(((o - gal) + 1)) .or. size(x_inl57_iy, 4) /= (1)) then
            deallocate(x_inl57_iy)
            allocate(x_inl57_iy(np_particles, 1, ((o - gal) + 1), 1))
        end if
        do si0_l1530 = 0, (1) - 1
            do si1_l1531 = 0, (((o - gal) + 1)) - 1
                do si2_l1532 = 0, (1) - 1
                    do si3_l1533 = 0, (np_particles) - 1
                        x_inl57_iy((si3_l1533) + 1, (si2_l1532) + 1, (si1_l1531) + 1, (si0_l1530) + 1) = ((x_inl1_loy &
                        &+ x_inl1_k_ey((si3_l1533) + 1)) + x_inl57_ty((si1_l1531) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_iz)) then
            allocate(x_inl57_iz(np_particles, (o + 1), 1, 1))
        else if (size(x_inl57_iz, 1) /= (np_particles) .or. size(x_inl57_iz, 2) /= ((o + 1)) .or. size(x_inl57_iz, 3) &
        &/= (1) .or. size(x_inl57_iz, 4) /= (1)) then
            deallocate(x_inl57_iz)
            allocate(x_inl57_iz(np_particles, (o + 1), 1, 1))
        end if
        do si0_l1534 = 0, (1) - 1
            do si1_l1535 = 0, (1) - 1
                do si2_l1536 = 0, ((o + 1)) - 1
                    do si3_l1537 = 0, (np_particles) - 1
                        x_inl57_iz((si3_l1537) + 1, (si2_l1536) + 1, (si1_l1535) + 1, (si0_l1534) + 1) = ((x_inl1_loz &
                        &+ x_inl1_l_ey((si3_l1537) + 1)) + x_inl57_tz((si2_l1536) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_ix_b)) then
            allocate(x_inl57_ix_b(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl57_ix_b, 1) /= (np_particles) .or. size(x_inl57_ix_b, 2) /= ((o + 1)) .or. &
        &size(x_inl57_ix_b, 3) /= (((o - gal) + 1)) .or. size(x_inl57_ix_b, 4) /= ((o + 1))) then
            deallocate(x_inl57_ix_b)
            allocate(x_inl57_ix_b(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        end if
        do si0_l1538 = 0, ((o + 1)) - 1
            do si1_l1539 = 0, (((o - gal) + 1)) - 1
                do si2_l1540 = 0, ((o + 1)) - 1
                    do si3_l1541 = 0, (np_particles) - 1
                        x_inl57_ix_b((si3_l1541) + 1, (si2_l1540) + 1, (si1_l1539) + 1, (si0_l1538) + 1) = &
                        &x_inl57_ix((si3_l1541) + 1, (0) + 1, (0) + 1, (si0_l1538) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_iy_b)) then
            allocate(x_inl57_iy_b(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl57_iy_b, 1) /= (np_particles) .or. size(x_inl57_iy_b, 2) /= ((o + 1)) .or. &
        &size(x_inl57_iy_b, 3) /= (((o - gal) + 1)) .or. size(x_inl57_iy_b, 4) /= ((o + 1))) then
            deallocate(x_inl57_iy_b)
            allocate(x_inl57_iy_b(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        end if
        do si0_l1542 = 0, ((o + 1)) - 1
            do si1_l1543 = 0, (((o - gal) + 1)) - 1
                do si2_l1544 = 0, ((o + 1)) - 1
                    do si3_l1545 = 0, (np_particles) - 1
                        x_inl57_iy_b((si3_l1545) + 1, (si2_l1544) + 1, (si1_l1543) + 1, (si0_l1542) + 1) = &
                        &x_inl57_iy((si3_l1545) + 1, (0) + 1, (si1_l1543) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_iz_b)) then
            allocate(x_inl57_iz_b(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl57_iz_b, 1) /= (np_particles) .or. size(x_inl57_iz_b, 2) /= ((o + 1)) .or. &
        &size(x_inl57_iz_b, 3) /= (((o - gal) + 1)) .or. size(x_inl57_iz_b, 4) /= ((o + 1))) then
            deallocate(x_inl57_iz_b)
            allocate(x_inl57_iz_b(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        end if
        do si0_l1546 = 0, ((o + 1)) - 1
            do si1_l1547 = 0, (((o - gal) + 1)) - 1
                do si2_l1548 = 0, ((o + 1)) - 1
                    do si3_l1549 = 0, (np_particles) - 1
                        x_inl57_iz_b((si3_l1549) + 1, (si2_l1548) + 1, (si1_l1547) + 1, (si0_l1546) + 1) = &
                        &x_inl57_iz((si3_l1549) + 1, (si2_l1548) + 1, (0) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_gathered)) then
            allocate(x_inl57_gathered(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl57_gathered, 1) /= (np_particles) .or. size(x_inl57_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl57_gathered, 3) /= (((o - gal) + 1)) .or. size(x_inl57_gathered, 4) /= ((o + 1))) then
            deallocate(x_inl57_gathered)
            allocate(x_inl57_gathered(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_1550 = 0, ((o + 1)) - 1
            do x_w1_1551 = 0, (((o - gal) + 1)) - 1
                do x_w2_1552 = 0, ((o + 1)) - 1
                    do x_w3_1553 = 0, (np_particles) - 1
                        x_inl57_gathered((x_w3_1553) + 1, (x_w2_1552) + 1, (x_w1_1551) + 1, (x_w0_1550) + 1) = &
                        &ey_arr((0) + 1, (x_inl57_iz_b((x_w3_1553) + 1, (x_w2_1552) + 1, (x_w1_1551) + 1, (x_w0_1550) &
                        &+ 1)) + 1, (x_inl57_iy_b((x_w3_1553) + 1, (x_w2_1552) + 1, (x_w1_1551) + 1, (x_w0_1550) + 1)) &
                        &+ 1, (x_inl57_ix_b((x_w3_1553) + 1, (x_w2_1552) + 1, (x_w1_1551) + 1, (x_w0_1550) + 1)) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl57_weight)) then
            allocate(x_inl57_weight(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl57_weight, 1) /= (np_particles) .or. size(x_inl57_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl57_weight, 3) /= (((o - gal) + 1)) .or. size(x_inl57_weight, 4) /= ((o + 1))) then
            deallocate(x_inl57_weight)
            allocate(x_inl57_weight(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_1554 = 0, ((o + 1)) - 1
            do x_w1_1555 = 0, (((o - gal) + 1)) - 1
                do x_w2_1556 = 0, ((o + 1)) - 1
                    do x_w3_1557 = 0, (np_particles) - 1
                        x_inl57_weight((x_w3_1557) + 1, (x_w2_1556) + 1, (x_w1_1555) + 1, (x_w0_1554) + 1) = &
                        &((x_inl1_sx_ey((x_w3_1557) + 1, (x_w0_1554) + 1) * x_inl1_sy_ey((x_w3_1557) + 1, (x_w1_1555) &
                        &+ 1)) * x_inl1_sz_ey((x_w3_1557) + 1, (x_w2_1556) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_cb210)) then
            allocate(x_cb210(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_cb210, 1) /= (np_particles) .or. size(x_cb210, 2) /= ((o + 1)) .or. size(x_cb210, 3) /= (((o - &
        &gal) + 1)) .or. size(x_cb210, 4) /= ((o + 1))) then
            deallocate(x_cb210)
            allocate(x_cb210(np_particles, (o + 1), ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_1558 = 0, ((o + 1)) - 1
            do x_w1_1559 = 0, (((o - gal) + 1)) - 1
                do x_w2_1560 = 0, ((o + 1)) - 1
                    do x_w3_1561 = 0, (np_particles) - 1
                        x_cb210((x_w3_1561) + 1, (x_w2_1560) + 1, (x_w1_1559) + 1, (x_w0_1558) + 1) = &
                        &(x_inl57_weight((x_w3_1561) + 1, (x_w2_1560) + 1, (x_w1_1559) + 1, (x_w0_1558) + 1) * &
                        &x_inl57_gathered((x_w3_1561) + 1, (x_w2_1560) + 1, (x_w1_1559) + 1, (x_w0_1558) + 1))
                    end do
                end do
            end do
        end do
        ! numpy: np.sum(__cb210, axis=(0, 1, 2))
        do x_ax0_1562 = 0, (np_particles) - 1
            x_cb211((x_ax0_1562) + 1) = 0.0_c_double
            do x_rd0_1563 = 0, ((o + 1)) - 1
                do x_rd1_1564 = 0, (((o - gal) + 1)) - 1
                    do x_rd2_1565 = 0, ((o + 1)) - 1
                        x_cb211((x_ax0_1562) + 1) = (x_cb211((x_ax0_1562) + 1) + x_cb210((x_ax0_1562) + 1, &
                        &(x_rd2_1565) + 1, (x_rd1_1564) + 1, (x_rd0_1563) + 1))
                    end do
                end do
            end do
        end do
        do x_w0_1566 = 0, (np_particles) - 1
            x_hcall30((x_w0_1566) + 1) = x_cb211((x_w0_1566) + 1)
        end do
        do x_w0_1567 = 0, (np_particles) - 1
            Eyp((x_w0_1567) + 1) = Eyp((x_w0_1567) + 1) + (x_hcall30((x_w0_1567) + 1))
        end do
        if (.not. allocated(x_cb212)) then
            allocate(x_cb212((o + 1)))
        else if (size(x_cb212, 1) /= ((o + 1))) then
            deallocate(x_cb212)
            allocate(x_cb212((o + 1)))
        end if
        if (.not. allocated(x_cb212)) then
            allocate(x_cb212((o + 1)))
        else if (size(x_cb212, 1) /= ((o + 1))) then
            deallocate(x_cb212)
            allocate(x_cb212((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_ez)
        do x_i_1568 = 0, (x_inl1_n_sx_ez) - 1
            x_cb212((x_i_1568) + 1) = x_i_1568
        end do
        if (.not. allocated(x_inl58_tx)) then
            allocate(x_inl58_tx((o + 1)))
        else if (size(x_inl58_tx, 1) /= ((o + 1))) then
            deallocate(x_inl58_tx)
            allocate(x_inl58_tx((o + 1)))
        end if
        do x_w0_1569 = 0, (x_inl1_n_sx_ez) - 1
            x_inl58_tx((x_w0_1569) + 1) = x_cb212((x_w0_1569) + 1)
        end do
        if (.not. allocated(x_cb213)) then
            allocate(x_cb213((o + 1)))
        else if (size(x_cb213, 1) /= ((o + 1))) then
            deallocate(x_cb213)
            allocate(x_cb213((o + 1)))
        end if
        if (.not. allocated(x_cb213)) then
            allocate(x_cb213((o + 1)))
        else if (size(x_cb213, 1) /= ((o + 1))) then
            deallocate(x_cb213)
            allocate(x_cb213((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sy_ez)
        do x_i_1570 = 0, (x_inl1_n_sy_ez) - 1
            x_cb213((x_i_1570) + 1) = x_i_1570
        end do
        if (.not. allocated(x_inl58_ty)) then
            allocate(x_inl58_ty((o + 1)))
        else if (size(x_inl58_ty, 1) /= ((o + 1))) then
            deallocate(x_inl58_ty)
            allocate(x_inl58_ty((o + 1)))
        end if
        do x_w0_1571 = 0, (x_inl1_n_sy_ez) - 1
            x_inl58_ty((x_w0_1571) + 1) = x_cb213((x_w0_1571) + 1)
        end do
        if (.not. allocated(x_cb214)) then
            allocate(x_cb214(((o - gal) + 1)))
        else if (size(x_cb214, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb214)
            allocate(x_cb214(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb214)) then
            allocate(x_cb214(((o - gal) + 1)))
        else if (size(x_cb214, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb214)
            allocate(x_cb214(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_ez)
        do x_i_1572 = 0, (x_inl1_n_sz_ez) - 1
            x_cb214((x_i_1572) + 1) = x_i_1572
        end do
        if (.not. allocated(x_inl58_tz)) then
            allocate(x_inl58_tz(((o - gal) + 1)))
        else if (size(x_inl58_tz, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl58_tz)
            allocate(x_inl58_tz(((o - gal) + 1)))
        end if
        do x_w0_1573 = 0, (x_inl1_n_sz_ez) - 1
            x_inl58_tz((x_w0_1573) + 1) = x_cb214((x_w0_1573) + 1)
        end do
        if (.not. allocated(x_inl58_ix)) then
            allocate(x_inl58_ix(np_particles, 1, 1, (o + 1)))
        else if (size(x_inl58_ix, 1) /= (np_particles) .or. size(x_inl58_ix, 2) /= (1) .or. size(x_inl58_ix, 3) /= (1) &
        &.or. size(x_inl58_ix, 4) /= ((o + 1))) then
            deallocate(x_inl58_ix)
            allocate(x_inl58_ix(np_particles, 1, 1, (o + 1)))
        end if
        do si0_l1574 = 0, ((o + 1)) - 1
            do si1_l1575 = 0, (1) - 1
                do si2_l1576 = 0, (1) - 1
                    do si3_l1577 = 0, (np_particles) - 1
                        x_inl58_ix((si3_l1577) + 1, (si2_l1576) + 1, (si1_l1575) + 1, (si0_l1574) + 1) = ((x_inl1_lox &
                        &+ x_inl1_j_ez((si3_l1577) + 1)) + x_inl58_tx((si0_l1574) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_iy)) then
            allocate(x_inl58_iy(np_particles, 1, (o + 1), 1))
        else if (size(x_inl58_iy, 1) /= (np_particles) .or. size(x_inl58_iy, 2) /= (1) .or. size(x_inl58_iy, 3) /= ((o &
        &+ 1)) .or. size(x_inl58_iy, 4) /= (1)) then
            deallocate(x_inl58_iy)
            allocate(x_inl58_iy(np_particles, 1, (o + 1), 1))
        end if
        do si0_l1578 = 0, (1) - 1
            do si1_l1579 = 0, ((o + 1)) - 1
                do si2_l1580 = 0, (1) - 1
                    do si3_l1581 = 0, (np_particles) - 1
                        x_inl58_iy((si3_l1581) + 1, (si2_l1580) + 1, (si1_l1579) + 1, (si0_l1578) + 1) = ((x_inl1_loy &
                        &+ x_inl1_k_ez((si3_l1581) + 1)) + x_inl58_ty((si1_l1579) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_iz)) then
            allocate(x_inl58_iz(np_particles, ((o - gal) + 1), 1, 1))
        else if (size(x_inl58_iz, 1) /= (np_particles) .or. size(x_inl58_iz, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl58_iz, 3) /= (1) .or. size(x_inl58_iz, 4) /= (1)) then
            deallocate(x_inl58_iz)
            allocate(x_inl58_iz(np_particles, ((o - gal) + 1), 1, 1))
        end if
        do si0_l1582 = 0, (1) - 1
            do si1_l1583 = 0, (1) - 1
                do si2_l1584 = 0, (((o - gal) + 1)) - 1
                    do si3_l1585 = 0, (np_particles) - 1
                        x_inl58_iz((si3_l1585) + 1, (si2_l1584) + 1, (si1_l1583) + 1, (si0_l1582) + 1) = ((x_inl1_loz &
                        &+ x_inl1_l_ez((si3_l1585) + 1)) + x_inl58_tz((si2_l1584) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_ix_b)) then
            allocate(x_inl58_ix_b(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        else if (size(x_inl58_ix_b, 1) /= (np_particles) .or. size(x_inl58_ix_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl58_ix_b, 3) /= ((o + 1)) .or. size(x_inl58_ix_b, 4) /= ((o + 1))) then
            deallocate(x_inl58_ix_b)
            allocate(x_inl58_ix_b(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        end if
        do si0_l1586 = 0, ((o + 1)) - 1
            do si1_l1587 = 0, ((o + 1)) - 1
                do si2_l1588 = 0, (((o - gal) + 1)) - 1
                    do si3_l1589 = 0, (np_particles) - 1
                        x_inl58_ix_b((si3_l1589) + 1, (si2_l1588) + 1, (si1_l1587) + 1, (si0_l1586) + 1) = &
                        &x_inl58_ix((si3_l1589) + 1, (0) + 1, (0) + 1, (si0_l1586) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_iy_b)) then
            allocate(x_inl58_iy_b(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        else if (size(x_inl58_iy_b, 1) /= (np_particles) .or. size(x_inl58_iy_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl58_iy_b, 3) /= ((o + 1)) .or. size(x_inl58_iy_b, 4) /= ((o + 1))) then
            deallocate(x_inl58_iy_b)
            allocate(x_inl58_iy_b(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        end if
        do si0_l1590 = 0, ((o + 1)) - 1
            do si1_l1591 = 0, ((o + 1)) - 1
                do si2_l1592 = 0, (((o - gal) + 1)) - 1
                    do si3_l1593 = 0, (np_particles) - 1
                        x_inl58_iy_b((si3_l1593) + 1, (si2_l1592) + 1, (si1_l1591) + 1, (si0_l1590) + 1) = &
                        &x_inl58_iy((si3_l1593) + 1, (0) + 1, (si1_l1591) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_iz_b)) then
            allocate(x_inl58_iz_b(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        else if (size(x_inl58_iz_b, 1) /= (np_particles) .or. size(x_inl58_iz_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl58_iz_b, 3) /= ((o + 1)) .or. size(x_inl58_iz_b, 4) /= ((o + 1))) then
            deallocate(x_inl58_iz_b)
            allocate(x_inl58_iz_b(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        end if
        do si0_l1594 = 0, ((o + 1)) - 1
            do si1_l1595 = 0, ((o + 1)) - 1
                do si2_l1596 = 0, (((o - gal) + 1)) - 1
                    do si3_l1597 = 0, (np_particles) - 1
                        x_inl58_iz_b((si3_l1597) + 1, (si2_l1596) + 1, (si1_l1595) + 1, (si0_l1594) + 1) = &
                        &x_inl58_iz((si3_l1597) + 1, (si2_l1596) + 1, (0) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_gathered)) then
            allocate(x_inl58_gathered(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        else if (size(x_inl58_gathered, 1) /= (np_particles) .or. size(x_inl58_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl58_gathered, 3) /= ((o + 1)) .or. size(x_inl58_gathered, 4) /= ((o + 1))) then
            deallocate(x_inl58_gathered)
            allocate(x_inl58_gathered(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        end if
        do x_w0_1598 = 0, ((o + 1)) - 1
            do x_w1_1599 = 0, ((o + 1)) - 1
                do x_w2_1600 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1601 = 0, (np_particles) - 1
                        x_inl58_gathered((x_w3_1601) + 1, (x_w2_1600) + 1, (x_w1_1599) + 1, (x_w0_1598) + 1) = &
                        &ez_arr((0) + 1, (x_inl58_iz_b((x_w3_1601) + 1, (x_w2_1600) + 1, (x_w1_1599) + 1, (x_w0_1598) &
                        &+ 1)) + 1, (x_inl58_iy_b((x_w3_1601) + 1, (x_w2_1600) + 1, (x_w1_1599) + 1, (x_w0_1598) + 1)) &
                        &+ 1, (x_inl58_ix_b((x_w3_1601) + 1, (x_w2_1600) + 1, (x_w1_1599) + 1, (x_w0_1598) + 1)) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl58_weight)) then
            allocate(x_inl58_weight(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        else if (size(x_inl58_weight, 1) /= (np_particles) .or. size(x_inl58_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl58_weight, 3) /= ((o + 1)) .or. size(x_inl58_weight, 4) /= ((o + 1))) then
            deallocate(x_inl58_weight)
            allocate(x_inl58_weight(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        end if
        do x_w0_1602 = 0, ((o + 1)) - 1
            do x_w1_1603 = 0, ((o + 1)) - 1
                do x_w2_1604 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1605 = 0, (np_particles) - 1
                        x_inl58_weight((x_w3_1605) + 1, (x_w2_1604) + 1, (x_w1_1603) + 1, (x_w0_1602) + 1) = &
                        &((x_inl1_sx_ez((x_w3_1605) + 1, (x_w0_1602) + 1) * x_inl1_sy_ez((x_w3_1605) + 1, (x_w1_1603) &
                        &+ 1)) * x_inl1_sz_ez((x_w3_1605) + 1, (x_w2_1604) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_cb215)) then
            allocate(x_cb215(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        else if (size(x_cb215, 1) /= (np_particles) .or. size(x_cb215, 2) /= (((o - gal) + 1)) .or. size(x_cb215, 3) &
        &/= ((o + 1)) .or. size(x_cb215, 4) /= ((o + 1))) then
            deallocate(x_cb215)
            allocate(x_cb215(np_particles, ((o - gal) + 1), (o + 1), (o + 1)))
        end if
        do x_w0_1606 = 0, ((o + 1)) - 1
            do x_w1_1607 = 0, ((o + 1)) - 1
                do x_w2_1608 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1609 = 0, (np_particles) - 1
                        x_cb215((x_w3_1609) + 1, (x_w2_1608) + 1, (x_w1_1607) + 1, (x_w0_1606) + 1) = &
                        &(x_inl58_weight((x_w3_1609) + 1, (x_w2_1608) + 1, (x_w1_1607) + 1, (x_w0_1606) + 1) * &
                        &x_inl58_gathered((x_w3_1609) + 1, (x_w2_1608) + 1, (x_w1_1607) + 1, (x_w0_1606) + 1))
                    end do
                end do
            end do
        end do
        ! numpy: np.sum(__cb215, axis=(0, 1, 2))
        do x_ax0_1610 = 0, (np_particles) - 1
            x_cb216((x_ax0_1610) + 1) = 0.0_c_double
            do x_rd0_1611 = 0, ((o + 1)) - 1
                do x_rd1_1612 = 0, ((o + 1)) - 1
                    do x_rd2_1613 = 0, (((o - gal) + 1)) - 1
                        x_cb216((x_ax0_1610) + 1) = (x_cb216((x_ax0_1610) + 1) + x_cb215((x_ax0_1610) + 1, &
                        &(x_rd2_1613) + 1, (x_rd1_1612) + 1, (x_rd0_1611) + 1))
                    end do
                end do
            end do
        end do
        do x_w0_1614 = 0, (np_particles) - 1
            x_hcall31((x_w0_1614) + 1) = x_cb216((x_w0_1614) + 1)
        end do
        do x_w0_1615 = 0, (np_particles) - 1
            Ezp((x_w0_1615) + 1) = Ezp((x_w0_1615) + 1) + (x_hcall31((x_w0_1615) + 1))
        end do
        if (.not. allocated(x_cb217)) then
            allocate(x_cb217(((o - gal) + 1)))
        else if (size(x_cb217, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb217)
            allocate(x_cb217(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb217)) then
            allocate(x_cb217(((o - gal) + 1)))
        else if (size(x_cb217, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb217)
            allocate(x_cb217(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bz)
        do x_i_1616 = 0, (x_inl1_n_sx_bz) - 1
            x_cb217((x_i_1616) + 1) = x_i_1616
        end do
        if (.not. allocated(x_inl59_tx)) then
            allocate(x_inl59_tx(((o - gal) + 1)))
        else if (size(x_inl59_tx, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl59_tx)
            allocate(x_inl59_tx(((o - gal) + 1)))
        end if
        do x_w0_1617 = 0, (x_inl1_n_sx_bz) - 1
            x_inl59_tx((x_w0_1617) + 1) = x_cb217((x_w0_1617) + 1)
        end do
        if (.not. allocated(x_cb218)) then
            allocate(x_cb218(((o - gal) + 1)))
        else if (size(x_cb218, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb218)
            allocate(x_cb218(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb218)) then
            allocate(x_cb218(((o - gal) + 1)))
        else if (size(x_cb218, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb218)
            allocate(x_cb218(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sy_bz)
        do x_i_1618 = 0, (x_inl1_n_sy_bz) - 1
            x_cb218((x_i_1618) + 1) = x_i_1618
        end do
        if (.not. allocated(x_inl59_ty)) then
            allocate(x_inl59_ty(((o - gal) + 1)))
        else if (size(x_inl59_ty, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl59_ty)
            allocate(x_inl59_ty(((o - gal) + 1)))
        end if
        do x_w0_1619 = 0, (x_inl1_n_sy_bz) - 1
            x_inl59_ty((x_w0_1619) + 1) = x_cb218((x_w0_1619) + 1)
        end do
        if (.not. allocated(x_cb219)) then
            allocate(x_cb219((o + 1)))
        else if (size(x_cb219, 1) /= ((o + 1))) then
            deallocate(x_cb219)
            allocate(x_cb219((o + 1)))
        end if
        if (.not. allocated(x_cb219)) then
            allocate(x_cb219((o + 1)))
        else if (size(x_cb219, 1) /= ((o + 1))) then
            deallocate(x_cb219)
            allocate(x_cb219((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bz)
        do x_i_1620 = 0, (x_inl1_n_sz_bz) - 1
            x_cb219((x_i_1620) + 1) = x_i_1620
        end do
        if (.not. allocated(x_inl59_tz)) then
            allocate(x_inl59_tz((o + 1)))
        else if (size(x_inl59_tz, 1) /= ((o + 1))) then
            deallocate(x_inl59_tz)
            allocate(x_inl59_tz((o + 1)))
        end if
        do x_w0_1621 = 0, (x_inl1_n_sz_bz) - 1
            x_inl59_tz((x_w0_1621) + 1) = x_cb219((x_w0_1621) + 1)
        end do
        if (.not. allocated(x_inl59_ix)) then
            allocate(x_inl59_ix(np_particles, 1, 1, ((o - gal) + 1)))
        else if (size(x_inl59_ix, 1) /= (np_particles) .or. size(x_inl59_ix, 2) /= (1) .or. size(x_inl59_ix, 3) /= (1) &
        &.or. size(x_inl59_ix, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl59_ix)
            allocate(x_inl59_ix(np_particles, 1, 1, ((o - gal) + 1)))
        end if
        do si0_l1622 = 0, (((o - gal) + 1)) - 1
            do si1_l1623 = 0, (1) - 1
                do si2_l1624 = 0, (1) - 1
                    do si3_l1625 = 0, (np_particles) - 1
                        x_inl59_ix((si3_l1625) + 1, (si2_l1624) + 1, (si1_l1623) + 1, (si0_l1622) + 1) = ((x_inl1_lox &
                        &+ x_inl1_j_bz((si3_l1625) + 1)) + x_inl59_tx((si0_l1622) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_iy)) then
            allocate(x_inl59_iy(np_particles, 1, ((o - gal) + 1), 1))
        else if (size(x_inl59_iy, 1) /= (np_particles) .or. size(x_inl59_iy, 2) /= (1) .or. size(x_inl59_iy, 3) /= &
        &(((o - gal) + 1)) .or. size(x_inl59_iy, 4) /= (1)) then
            deallocate(x_inl59_iy)
            allocate(x_inl59_iy(np_particles, 1, ((o - gal) + 1), 1))
        end if
        do si0_l1626 = 0, (1) - 1
            do si1_l1627 = 0, (((o - gal) + 1)) - 1
                do si2_l1628 = 0, (1) - 1
                    do si3_l1629 = 0, (np_particles) - 1
                        x_inl59_iy((si3_l1629) + 1, (si2_l1628) + 1, (si1_l1627) + 1, (si0_l1626) + 1) = ((x_inl1_loy &
                        &+ x_inl1_k_bz((si3_l1629) + 1)) + x_inl59_ty((si1_l1627) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_iz)) then
            allocate(x_inl59_iz(np_particles, (o + 1), 1, 1))
        else if (size(x_inl59_iz, 1) /= (np_particles) .or. size(x_inl59_iz, 2) /= ((o + 1)) .or. size(x_inl59_iz, 3) &
        &/= (1) .or. size(x_inl59_iz, 4) /= (1)) then
            deallocate(x_inl59_iz)
            allocate(x_inl59_iz(np_particles, (o + 1), 1, 1))
        end if
        do si0_l1630 = 0, (1) - 1
            do si1_l1631 = 0, (1) - 1
                do si2_l1632 = 0, ((o + 1)) - 1
                    do si3_l1633 = 0, (np_particles) - 1
                        x_inl59_iz((si3_l1633) + 1, (si2_l1632) + 1, (si1_l1631) + 1, (si0_l1630) + 1) = ((x_inl1_loz &
                        &+ x_inl1_l_bz((si3_l1633) + 1)) + x_inl59_tz((si2_l1632) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_ix_b)) then
            allocate(x_inl59_ix_b(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl59_ix_b, 1) /= (np_particles) .or. size(x_inl59_ix_b, 2) /= ((o + 1)) .or. &
        &size(x_inl59_ix_b, 3) /= (((o - gal) + 1)) .or. size(x_inl59_ix_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl59_ix_b)
            allocate(x_inl59_ix_b(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l1634 = 0, (((o - gal) + 1)) - 1
            do si1_l1635 = 0, (((o - gal) + 1)) - 1
                do si2_l1636 = 0, ((o + 1)) - 1
                    do si3_l1637 = 0, (np_particles) - 1
                        x_inl59_ix_b((si3_l1637) + 1, (si2_l1636) + 1, (si1_l1635) + 1, (si0_l1634) + 1) = &
                        &x_inl59_ix((si3_l1637) + 1, (0) + 1, (0) + 1, (si0_l1634) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_iy_b)) then
            allocate(x_inl59_iy_b(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl59_iy_b, 1) /= (np_particles) .or. size(x_inl59_iy_b, 2) /= ((o + 1)) .or. &
        &size(x_inl59_iy_b, 3) /= (((o - gal) + 1)) .or. size(x_inl59_iy_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl59_iy_b)
            allocate(x_inl59_iy_b(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l1638 = 0, (((o - gal) + 1)) - 1
            do si1_l1639 = 0, (((o - gal) + 1)) - 1
                do si2_l1640 = 0, ((o + 1)) - 1
                    do si3_l1641 = 0, (np_particles) - 1
                        x_inl59_iy_b((si3_l1641) + 1, (si2_l1640) + 1, (si1_l1639) + 1, (si0_l1638) + 1) = &
                        &x_inl59_iy((si3_l1641) + 1, (0) + 1, (si1_l1639) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_iz_b)) then
            allocate(x_inl59_iz_b(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl59_iz_b, 1) /= (np_particles) .or. size(x_inl59_iz_b, 2) /= ((o + 1)) .or. &
        &size(x_inl59_iz_b, 3) /= (((o - gal) + 1)) .or. size(x_inl59_iz_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl59_iz_b)
            allocate(x_inl59_iz_b(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do si0_l1642 = 0, (((o - gal) + 1)) - 1
            do si1_l1643 = 0, (((o - gal) + 1)) - 1
                do si2_l1644 = 0, ((o + 1)) - 1
                    do si3_l1645 = 0, (np_particles) - 1
                        x_inl59_iz_b((si3_l1645) + 1, (si2_l1644) + 1, (si1_l1643) + 1, (si0_l1642) + 1) = &
                        &x_inl59_iz((si3_l1645) + 1, (si2_l1644) + 1, (0) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_gathered)) then
            allocate(x_inl59_gathered(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl59_gathered, 1) /= (np_particles) .or. size(x_inl59_gathered, 2) /= ((o + 1)) .or. &
        &size(x_inl59_gathered, 3) /= (((o - gal) + 1)) .or. size(x_inl59_gathered, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl59_gathered)
            allocate(x_inl59_gathered(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_1646 = 0, (((o - gal) + 1)) - 1
            do x_w1_1647 = 0, (((o - gal) + 1)) - 1
                do x_w2_1648 = 0, ((o + 1)) - 1
                    do x_w3_1649 = 0, (np_particles) - 1
                        x_inl59_gathered((x_w3_1649) + 1, (x_w2_1648) + 1, (x_w1_1647) + 1, (x_w0_1646) + 1) = &
                        &bz_arr((0) + 1, (x_inl59_iz_b((x_w3_1649) + 1, (x_w2_1648) + 1, (x_w1_1647) + 1, (x_w0_1646) &
                        &+ 1)) + 1, (x_inl59_iy_b((x_w3_1649) + 1, (x_w2_1648) + 1, (x_w1_1647) + 1, (x_w0_1646) + 1)) &
                        &+ 1, (x_inl59_ix_b((x_w3_1649) + 1, (x_w2_1648) + 1, (x_w1_1647) + 1, (x_w0_1646) + 1)) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl59_weight)) then
            allocate(x_inl59_weight(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_inl59_weight, 1) /= (np_particles) .or. size(x_inl59_weight, 2) /= ((o + 1)) .or. &
        &size(x_inl59_weight, 3) /= (((o - gal) + 1)) .or. size(x_inl59_weight, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl59_weight)
            allocate(x_inl59_weight(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_1650 = 0, (((o - gal) + 1)) - 1
            do x_w1_1651 = 0, (((o - gal) + 1)) - 1
                do x_w2_1652 = 0, ((o + 1)) - 1
                    do x_w3_1653 = 0, (np_particles) - 1
                        x_inl59_weight((x_w3_1653) + 1, (x_w2_1652) + 1, (x_w1_1651) + 1, (x_w0_1650) + 1) = &
                        &((x_inl1_sx_bz((x_w3_1653) + 1, (x_w0_1650) + 1) * x_inl1_sy_bz((x_w3_1653) + 1, (x_w1_1651) &
                        &+ 1)) * x_inl1_sz_bz((x_w3_1653) + 1, (x_w2_1652) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_cb220)) then
            allocate(x_cb220(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        else if (size(x_cb220, 1) /= (np_particles) .or. size(x_cb220, 2) /= ((o + 1)) .or. size(x_cb220, 3) /= (((o - &
        &gal) + 1)) .or. size(x_cb220, 4) /= (((o - gal) + 1))) then
            deallocate(x_cb220)
            allocate(x_cb220(np_particles, (o + 1), ((o - gal) + 1), ((o - gal) + 1)))
        end if
        do x_w0_1654 = 0, (((o - gal) + 1)) - 1
            do x_w1_1655 = 0, (((o - gal) + 1)) - 1
                do x_w2_1656 = 0, ((o + 1)) - 1
                    do x_w3_1657 = 0, (np_particles) - 1
                        x_cb220((x_w3_1657) + 1, (x_w2_1656) + 1, (x_w1_1655) + 1, (x_w0_1654) + 1) = &
                        &(x_inl59_weight((x_w3_1657) + 1, (x_w2_1656) + 1, (x_w1_1655) + 1, (x_w0_1654) + 1) * &
                        &x_inl59_gathered((x_w3_1657) + 1, (x_w2_1656) + 1, (x_w1_1655) + 1, (x_w0_1654) + 1))
                    end do
                end do
            end do
        end do
        ! numpy: np.sum(__cb220, axis=(0, 1, 2))
        do x_ax0_1658 = 0, (np_particles) - 1
            x_cb221((x_ax0_1658) + 1) = 0.0_c_double
            do x_rd0_1659 = 0, (((o - gal) + 1)) - 1
                do x_rd1_1660 = 0, (((o - gal) + 1)) - 1
                    do x_rd2_1661 = 0, ((o + 1)) - 1
                        x_cb221((x_ax0_1658) + 1) = (x_cb221((x_ax0_1658) + 1) + x_cb220((x_ax0_1658) + 1, &
                        &(x_rd2_1661) + 1, (x_rd1_1660) + 1, (x_rd0_1659) + 1))
                    end do
                end do
            end do
        end do
        do x_w0_1662 = 0, (np_particles) - 1
            x_hcall32((x_w0_1662) + 1) = x_cb221((x_w0_1662) + 1)
        end do
        do x_w0_1663 = 0, (np_particles) - 1
            Bzp((x_w0_1663) + 1) = Bzp((x_w0_1663) + 1) + (x_hcall32((x_w0_1663) + 1))
        end do
        if (.not. allocated(x_cb222)) then
            allocate(x_cb222(((o - gal) + 1)))
        else if (size(x_cb222, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb222)
            allocate(x_cb222(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb222)) then
            allocate(x_cb222(((o - gal) + 1)))
        else if (size(x_cb222, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb222)
            allocate(x_cb222(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_by)
        do x_i_1664 = 0, (x_inl1_n_sx_by) - 1
            x_cb222((x_i_1664) + 1) = x_i_1664
        end do
        if (.not. allocated(x_inl60_tx)) then
            allocate(x_inl60_tx(((o - gal) + 1)))
        else if (size(x_inl60_tx, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl60_tx)
            allocate(x_inl60_tx(((o - gal) + 1)))
        end if
        do x_w0_1665 = 0, (x_inl1_n_sx_by) - 1
            x_inl60_tx((x_w0_1665) + 1) = x_cb222((x_w0_1665) + 1)
        end do
        if (.not. allocated(x_cb223)) then
            allocate(x_cb223((o + 1)))
        else if (size(x_cb223, 1) /= ((o + 1))) then
            deallocate(x_cb223)
            allocate(x_cb223((o + 1)))
        end if
        if (.not. allocated(x_cb223)) then
            allocate(x_cb223((o + 1)))
        else if (size(x_cb223, 1) /= ((o + 1))) then
            deallocate(x_cb223)
            allocate(x_cb223((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sy_by)
        do x_i_1666 = 0, (x_inl1_n_sy_by) - 1
            x_cb223((x_i_1666) + 1) = x_i_1666
        end do
        if (.not. allocated(x_inl60_ty)) then
            allocate(x_inl60_ty((o + 1)))
        else if (size(x_inl60_ty, 1) /= ((o + 1))) then
            deallocate(x_inl60_ty)
            allocate(x_inl60_ty((o + 1)))
        end if
        do x_w0_1667 = 0, (x_inl1_n_sy_by) - 1
            x_inl60_ty((x_w0_1667) + 1) = x_cb223((x_w0_1667) + 1)
        end do
        if (.not. allocated(x_cb224)) then
            allocate(x_cb224(((o - gal) + 1)))
        else if (size(x_cb224, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb224)
            allocate(x_cb224(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb224)) then
            allocate(x_cb224(((o - gal) + 1)))
        else if (size(x_cb224, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb224)
            allocate(x_cb224(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_by)
        do x_i_1668 = 0, (x_inl1_n_sz_by) - 1
            x_cb224((x_i_1668) + 1) = x_i_1668
        end do
        if (.not. allocated(x_inl60_tz)) then
            allocate(x_inl60_tz(((o - gal) + 1)))
        else if (size(x_inl60_tz, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl60_tz)
            allocate(x_inl60_tz(((o - gal) + 1)))
        end if
        do x_w0_1669 = 0, (x_inl1_n_sz_by) - 1
            x_inl60_tz((x_w0_1669) + 1) = x_cb224((x_w0_1669) + 1)
        end do
        if (.not. allocated(x_inl60_ix)) then
            allocate(x_inl60_ix(np_particles, 1, 1, ((o - gal) + 1)))
        else if (size(x_inl60_ix, 1) /= (np_particles) .or. size(x_inl60_ix, 2) /= (1) .or. size(x_inl60_ix, 3) /= (1) &
        &.or. size(x_inl60_ix, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl60_ix)
            allocate(x_inl60_ix(np_particles, 1, 1, ((o - gal) + 1)))
        end if
        do si0_l1670 = 0, (((o - gal) + 1)) - 1
            do si1_l1671 = 0, (1) - 1
                do si2_l1672 = 0, (1) - 1
                    do si3_l1673 = 0, (np_particles) - 1
                        x_inl60_ix((si3_l1673) + 1, (si2_l1672) + 1, (si1_l1671) + 1, (si0_l1670) + 1) = ((x_inl1_lox &
                        &+ x_inl1_j_by((si3_l1673) + 1)) + x_inl60_tx((si0_l1670) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_iy)) then
            allocate(x_inl60_iy(np_particles, 1, (o + 1), 1))
        else if (size(x_inl60_iy, 1) /= (np_particles) .or. size(x_inl60_iy, 2) /= (1) .or. size(x_inl60_iy, 3) /= ((o &
        &+ 1)) .or. size(x_inl60_iy, 4) /= (1)) then
            deallocate(x_inl60_iy)
            allocate(x_inl60_iy(np_particles, 1, (o + 1), 1))
        end if
        do si0_l1674 = 0, (1) - 1
            do si1_l1675 = 0, ((o + 1)) - 1
                do si2_l1676 = 0, (1) - 1
                    do si3_l1677 = 0, (np_particles) - 1
                        x_inl60_iy((si3_l1677) + 1, (si2_l1676) + 1, (si1_l1675) + 1, (si0_l1674) + 1) = ((x_inl1_loy &
                        &+ x_inl1_k_by((si3_l1677) + 1)) + x_inl60_ty((si1_l1675) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_iz)) then
            allocate(x_inl60_iz(np_particles, ((o - gal) + 1), 1, 1))
        else if (size(x_inl60_iz, 1) /= (np_particles) .or. size(x_inl60_iz, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl60_iz, 3) /= (1) .or. size(x_inl60_iz, 4) /= (1)) then
            deallocate(x_inl60_iz)
            allocate(x_inl60_iz(np_particles, ((o - gal) + 1), 1, 1))
        end if
        do si0_l1678 = 0, (1) - 1
            do si1_l1679 = 0, (1) - 1
                do si2_l1680 = 0, (((o - gal) + 1)) - 1
                    do si3_l1681 = 0, (np_particles) - 1
                        x_inl60_iz((si3_l1681) + 1, (si2_l1680) + 1, (si1_l1679) + 1, (si0_l1678) + 1) = ((x_inl1_loz &
                        &+ x_inl1_l_by((si3_l1681) + 1)) + x_inl60_tz((si2_l1680) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_ix_b)) then
            allocate(x_inl60_ix_b(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl60_ix_b, 1) /= (np_particles) .or. size(x_inl60_ix_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl60_ix_b, 3) /= ((o + 1)) .or. size(x_inl60_ix_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl60_ix_b)
            allocate(x_inl60_ix_b(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        end if
        do si0_l1682 = 0, (((o - gal) + 1)) - 1
            do si1_l1683 = 0, ((o + 1)) - 1
                do si2_l1684 = 0, (((o - gal) + 1)) - 1
                    do si3_l1685 = 0, (np_particles) - 1
                        x_inl60_ix_b((si3_l1685) + 1, (si2_l1684) + 1, (si1_l1683) + 1, (si0_l1682) + 1) = &
                        &x_inl60_ix((si3_l1685) + 1, (0) + 1, (0) + 1, (si0_l1682) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_iy_b)) then
            allocate(x_inl60_iy_b(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl60_iy_b, 1) /= (np_particles) .or. size(x_inl60_iy_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl60_iy_b, 3) /= ((o + 1)) .or. size(x_inl60_iy_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl60_iy_b)
            allocate(x_inl60_iy_b(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        end if
        do si0_l1686 = 0, (((o - gal) + 1)) - 1
            do si1_l1687 = 0, ((o + 1)) - 1
                do si2_l1688 = 0, (((o - gal) + 1)) - 1
                    do si3_l1689 = 0, (np_particles) - 1
                        x_inl60_iy_b((si3_l1689) + 1, (si2_l1688) + 1, (si1_l1687) + 1, (si0_l1686) + 1) = &
                        &x_inl60_iy((si3_l1689) + 1, (0) + 1, (si1_l1687) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_iz_b)) then
            allocate(x_inl60_iz_b(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl60_iz_b, 1) /= (np_particles) .or. size(x_inl60_iz_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl60_iz_b, 3) /= ((o + 1)) .or. size(x_inl60_iz_b, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl60_iz_b)
            allocate(x_inl60_iz_b(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        end if
        do si0_l1690 = 0, (((o - gal) + 1)) - 1
            do si1_l1691 = 0, ((o + 1)) - 1
                do si2_l1692 = 0, (((o - gal) + 1)) - 1
                    do si3_l1693 = 0, (np_particles) - 1
                        x_inl60_iz_b((si3_l1693) + 1, (si2_l1692) + 1, (si1_l1691) + 1, (si0_l1690) + 1) = &
                        &x_inl60_iz((si3_l1693) + 1, (si2_l1692) + 1, (0) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_gathered)) then
            allocate(x_inl60_gathered(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl60_gathered, 1) /= (np_particles) .or. size(x_inl60_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl60_gathered, 3) /= ((o + 1)) .or. size(x_inl60_gathered, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl60_gathered)
            allocate(x_inl60_gathered(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_1694 = 0, (((o - gal) + 1)) - 1
            do x_w1_1695 = 0, ((o + 1)) - 1
                do x_w2_1696 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1697 = 0, (np_particles) - 1
                        x_inl60_gathered((x_w3_1697) + 1, (x_w2_1696) + 1, (x_w1_1695) + 1, (x_w0_1694) + 1) = &
                        &by_arr((0) + 1, (x_inl60_iz_b((x_w3_1697) + 1, (x_w2_1696) + 1, (x_w1_1695) + 1, (x_w0_1694) &
                        &+ 1)) + 1, (x_inl60_iy_b((x_w3_1697) + 1, (x_w2_1696) + 1, (x_w1_1695) + 1, (x_w0_1694) + 1)) &
                        &+ 1, (x_inl60_ix_b((x_w3_1697) + 1, (x_w2_1696) + 1, (x_w1_1695) + 1, (x_w0_1694) + 1)) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl60_weight)) then
            allocate(x_inl60_weight(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_inl60_weight, 1) /= (np_particles) .or. size(x_inl60_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl60_weight, 3) /= ((o + 1)) .or. size(x_inl60_weight, 4) /= (((o - gal) + 1))) then
            deallocate(x_inl60_weight)
            allocate(x_inl60_weight(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_1698 = 0, (((o - gal) + 1)) - 1
            do x_w1_1699 = 0, ((o + 1)) - 1
                do x_w2_1700 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1701 = 0, (np_particles) - 1
                        x_inl60_weight((x_w3_1701) + 1, (x_w2_1700) + 1, (x_w1_1699) + 1, (x_w0_1698) + 1) = &
                        &((x_inl1_sx_by((x_w3_1701) + 1, (x_w0_1698) + 1) * x_inl1_sy_by((x_w3_1701) + 1, (x_w1_1699) &
                        &+ 1)) * x_inl1_sz_by((x_w3_1701) + 1, (x_w2_1700) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_cb225)) then
            allocate(x_cb225(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        else if (size(x_cb225, 1) /= (np_particles) .or. size(x_cb225, 2) /= (((o - gal) + 1)) .or. size(x_cb225, 3) &
        &/= ((o + 1)) .or. size(x_cb225, 4) /= (((o - gal) + 1))) then
            deallocate(x_cb225)
            allocate(x_cb225(np_particles, ((o - gal) + 1), (o + 1), ((o - gal) + 1)))
        end if
        do x_w0_1702 = 0, (((o - gal) + 1)) - 1
            do x_w1_1703 = 0, ((o + 1)) - 1
                do x_w2_1704 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1705 = 0, (np_particles) - 1
                        x_cb225((x_w3_1705) + 1, (x_w2_1704) + 1, (x_w1_1703) + 1, (x_w0_1702) + 1) = &
                        &(x_inl60_weight((x_w3_1705) + 1, (x_w2_1704) + 1, (x_w1_1703) + 1, (x_w0_1702) + 1) * &
                        &x_inl60_gathered((x_w3_1705) + 1, (x_w2_1704) + 1, (x_w1_1703) + 1, (x_w0_1702) + 1))
                    end do
                end do
            end do
        end do
        ! numpy: np.sum(__cb225, axis=(0, 1, 2))
        do x_ax0_1706 = 0, (np_particles) - 1
            x_cb226((x_ax0_1706) + 1) = 0.0_c_double
            do x_rd0_1707 = 0, (((o - gal) + 1)) - 1
                do x_rd1_1708 = 0, ((o + 1)) - 1
                    do x_rd2_1709 = 0, (((o - gal) + 1)) - 1
                        x_cb226((x_ax0_1706) + 1) = (x_cb226((x_ax0_1706) + 1) + x_cb225((x_ax0_1706) + 1, &
                        &(x_rd2_1709) + 1, (x_rd1_1708) + 1, (x_rd0_1707) + 1))
                    end do
                end do
            end do
        end do
        do x_w0_1710 = 0, (np_particles) - 1
            x_hcall33((x_w0_1710) + 1) = x_cb226((x_w0_1710) + 1)
        end do
        do x_w0_1711 = 0, (np_particles) - 1
            Byp((x_w0_1711) + 1) = Byp((x_w0_1711) + 1) + (x_hcall33((x_w0_1711) + 1))
        end do
        if (.not. allocated(x_cb227)) then
            allocate(x_cb227((o + 1)))
        else if (size(x_cb227, 1) /= ((o + 1))) then
            deallocate(x_cb227)
            allocate(x_cb227((o + 1)))
        end if
        if (.not. allocated(x_cb227)) then
            allocate(x_cb227((o + 1)))
        else if (size(x_cb227, 1) /= ((o + 1))) then
            deallocate(x_cb227)
            allocate(x_cb227((o + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sx_bx)
        do x_i_1712 = 0, (x_inl1_n_sx_bx) - 1
            x_cb227((x_i_1712) + 1) = x_i_1712
        end do
        if (.not. allocated(x_inl61_tx)) then
            allocate(x_inl61_tx((o + 1)))
        else if (size(x_inl61_tx, 1) /= ((o + 1))) then
            deallocate(x_inl61_tx)
            allocate(x_inl61_tx((o + 1)))
        end if
        do x_w0_1713 = 0, (x_inl1_n_sx_bx) - 1
            x_inl61_tx((x_w0_1713) + 1) = x_cb227((x_w0_1713) + 1)
        end do
        if (.not. allocated(x_cb228)) then
            allocate(x_cb228(((o - gal) + 1)))
        else if (size(x_cb228, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb228)
            allocate(x_cb228(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb228)) then
            allocate(x_cb228(((o - gal) + 1)))
        else if (size(x_cb228, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb228)
            allocate(x_cb228(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sy_bx)
        do x_i_1714 = 0, (x_inl1_n_sy_bx) - 1
            x_cb228((x_i_1714) + 1) = x_i_1714
        end do
        if (.not. allocated(x_inl61_ty)) then
            allocate(x_inl61_ty(((o - gal) + 1)))
        else if (size(x_inl61_ty, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl61_ty)
            allocate(x_inl61_ty(((o - gal) + 1)))
        end if
        do x_w0_1715 = 0, (x_inl1_n_sy_bx) - 1
            x_inl61_ty((x_w0_1715) + 1) = x_cb228((x_w0_1715) + 1)
        end do
        if (.not. allocated(x_cb229)) then
            allocate(x_cb229(((o - gal) + 1)))
        else if (size(x_cb229, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb229)
            allocate(x_cb229(((o - gal) + 1)))
        end if
        if (.not. allocated(x_cb229)) then
            allocate(x_cb229(((o - gal) + 1)))
        else if (size(x_cb229, 1) /= (((o - gal) + 1))) then
            deallocate(x_cb229)
            allocate(x_cb229(((o - gal) + 1)))
        end if
        ! numpy: np.arange(__inl1_n_sz_bx)
        do x_i_1716 = 0, (x_inl1_n_sz_bx) - 1
            x_cb229((x_i_1716) + 1) = x_i_1716
        end do
        if (.not. allocated(x_inl61_tz)) then
            allocate(x_inl61_tz(((o - gal) + 1)))
        else if (size(x_inl61_tz, 1) /= (((o - gal) + 1))) then
            deallocate(x_inl61_tz)
            allocate(x_inl61_tz(((o - gal) + 1)))
        end if
        do x_w0_1717 = 0, (x_inl1_n_sz_bx) - 1
            x_inl61_tz((x_w0_1717) + 1) = x_cb229((x_w0_1717) + 1)
        end do
        if (.not. allocated(x_inl61_ix)) then
            allocate(x_inl61_ix(np_particles, 1, 1, (o + 1)))
        else if (size(x_inl61_ix, 1) /= (np_particles) .or. size(x_inl61_ix, 2) /= (1) .or. size(x_inl61_ix, 3) /= (1) &
        &.or. size(x_inl61_ix, 4) /= ((o + 1))) then
            deallocate(x_inl61_ix)
            allocate(x_inl61_ix(np_particles, 1, 1, (o + 1)))
        end if
        do si0_l1718 = 0, ((o + 1)) - 1
            do si1_l1719 = 0, (1) - 1
                do si2_l1720 = 0, (1) - 1
                    do si3_l1721 = 0, (np_particles) - 1
                        x_inl61_ix((si3_l1721) + 1, (si2_l1720) + 1, (si1_l1719) + 1, (si0_l1718) + 1) = ((x_inl1_lox &
                        &+ x_inl1_j_bx((si3_l1721) + 1)) + x_inl61_tx((si0_l1718) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_iy)) then
            allocate(x_inl61_iy(np_particles, 1, ((o - gal) + 1), 1))
        else if (size(x_inl61_iy, 1) /= (np_particles) .or. size(x_inl61_iy, 2) /= (1) .or. size(x_inl61_iy, 3) /= &
        &(((o - gal) + 1)) .or. size(x_inl61_iy, 4) /= (1)) then
            deallocate(x_inl61_iy)
            allocate(x_inl61_iy(np_particles, 1, ((o - gal) + 1), 1))
        end if
        do si0_l1722 = 0, (1) - 1
            do si1_l1723 = 0, (((o - gal) + 1)) - 1
                do si2_l1724 = 0, (1) - 1
                    do si3_l1725 = 0, (np_particles) - 1
                        x_inl61_iy((si3_l1725) + 1, (si2_l1724) + 1, (si1_l1723) + 1, (si0_l1722) + 1) = ((x_inl1_loy &
                        &+ x_inl1_k_bx((si3_l1725) + 1)) + x_inl61_ty((si1_l1723) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_iz)) then
            allocate(x_inl61_iz(np_particles, ((o - gal) + 1), 1, 1))
        else if (size(x_inl61_iz, 1) /= (np_particles) .or. size(x_inl61_iz, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl61_iz, 3) /= (1) .or. size(x_inl61_iz, 4) /= (1)) then
            deallocate(x_inl61_iz)
            allocate(x_inl61_iz(np_particles, ((o - gal) + 1), 1, 1))
        end if
        do si0_l1726 = 0, (1) - 1
            do si1_l1727 = 0, (1) - 1
                do si2_l1728 = 0, (((o - gal) + 1)) - 1
                    do si3_l1729 = 0, (np_particles) - 1
                        x_inl61_iz((si3_l1729) + 1, (si2_l1728) + 1, (si1_l1727) + 1, (si0_l1726) + 1) = ((x_inl1_loz &
                        &+ x_inl1_l_bx((si3_l1729) + 1)) + x_inl61_tz((si2_l1728) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_ix_b)) then
            allocate(x_inl61_ix_b(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl61_ix_b, 1) /= (np_particles) .or. size(x_inl61_ix_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl61_ix_b, 3) /= (((o - gal) + 1)) .or. size(x_inl61_ix_b, 4) /= ((o + 1))) then
            deallocate(x_inl61_ix_b)
            allocate(x_inl61_ix_b(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        end if
        do si0_l1730 = 0, ((o + 1)) - 1
            do si1_l1731 = 0, (((o - gal) + 1)) - 1
                do si2_l1732 = 0, (((o - gal) + 1)) - 1
                    do si3_l1733 = 0, (np_particles) - 1
                        x_inl61_ix_b((si3_l1733) + 1, (si2_l1732) + 1, (si1_l1731) + 1, (si0_l1730) + 1) = &
                        &x_inl61_ix((si3_l1733) + 1, (0) + 1, (0) + 1, (si0_l1730) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_iy_b)) then
            allocate(x_inl61_iy_b(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl61_iy_b, 1) /= (np_particles) .or. size(x_inl61_iy_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl61_iy_b, 3) /= (((o - gal) + 1)) .or. size(x_inl61_iy_b, 4) /= ((o + 1))) then
            deallocate(x_inl61_iy_b)
            allocate(x_inl61_iy_b(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        end if
        do si0_l1734 = 0, ((o + 1)) - 1
            do si1_l1735 = 0, (((o - gal) + 1)) - 1
                do si2_l1736 = 0, (((o - gal) + 1)) - 1
                    do si3_l1737 = 0, (np_particles) - 1
                        x_inl61_iy_b((si3_l1737) + 1, (si2_l1736) + 1, (si1_l1735) + 1, (si0_l1734) + 1) = &
                        &x_inl61_iy((si3_l1737) + 1, (0) + 1, (si1_l1735) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_iz_b)) then
            allocate(x_inl61_iz_b(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl61_iz_b, 1) /= (np_particles) .or. size(x_inl61_iz_b, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl61_iz_b, 3) /= (((o - gal) + 1)) .or. size(x_inl61_iz_b, 4) /= ((o + 1))) then
            deallocate(x_inl61_iz_b)
            allocate(x_inl61_iz_b(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        end if
        do si0_l1738 = 0, ((o + 1)) - 1
            do si1_l1739 = 0, (((o - gal) + 1)) - 1
                do si2_l1740 = 0, (((o - gal) + 1)) - 1
                    do si3_l1741 = 0, (np_particles) - 1
                        x_inl61_iz_b((si3_l1741) + 1, (si2_l1740) + 1, (si1_l1739) + 1, (si0_l1738) + 1) = &
                        &x_inl61_iz((si3_l1741) + 1, (si2_l1740) + 1, (0) + 1, (0) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_gathered)) then
            allocate(x_inl61_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl61_gathered, 1) /= (np_particles) .or. size(x_inl61_gathered, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl61_gathered, 3) /= (((o - gal) + 1)) .or. size(x_inl61_gathered, 4) /= ((o + 1))) then
            deallocate(x_inl61_gathered)
            allocate(x_inl61_gathered(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_1742 = 0, ((o + 1)) - 1
            do x_w1_1743 = 0, (((o - gal) + 1)) - 1
                do x_w2_1744 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1745 = 0, (np_particles) - 1
                        x_inl61_gathered((x_w3_1745) + 1, (x_w2_1744) + 1, (x_w1_1743) + 1, (x_w0_1742) + 1) = &
                        &bx_arr((0) + 1, (x_inl61_iz_b((x_w3_1745) + 1, (x_w2_1744) + 1, (x_w1_1743) + 1, (x_w0_1742) &
                        &+ 1)) + 1, (x_inl61_iy_b((x_w3_1745) + 1, (x_w2_1744) + 1, (x_w1_1743) + 1, (x_w0_1742) + 1)) &
                        &+ 1, (x_inl61_ix_b((x_w3_1745) + 1, (x_w2_1744) + 1, (x_w1_1743) + 1, (x_w0_1742) + 1)) + 1)
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_inl61_weight)) then
            allocate(x_inl61_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_inl61_weight, 1) /= (np_particles) .or. size(x_inl61_weight, 2) /= (((o - gal) + 1)) .or. &
        &size(x_inl61_weight, 3) /= (((o - gal) + 1)) .or. size(x_inl61_weight, 4) /= ((o + 1))) then
            deallocate(x_inl61_weight)
            allocate(x_inl61_weight(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_1746 = 0, ((o + 1)) - 1
            do x_w1_1747 = 0, (((o - gal) + 1)) - 1
                do x_w2_1748 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1749 = 0, (np_particles) - 1
                        x_inl61_weight((x_w3_1749) + 1, (x_w2_1748) + 1, (x_w1_1747) + 1, (x_w0_1746) + 1) = &
                        &((x_inl1_sx_bx((x_w3_1749) + 1, (x_w0_1746) + 1) * x_inl1_sy_bx((x_w3_1749) + 1, (x_w1_1747) &
                        &+ 1)) * x_inl1_sz_bx((x_w3_1749) + 1, (x_w2_1748) + 1))
                    end do
                end do
            end do
        end do
        if (.not. allocated(x_cb230)) then
            allocate(x_cb230(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        else if (size(x_cb230, 1) /= (np_particles) .or. size(x_cb230, 2) /= (((o - gal) + 1)) .or. size(x_cb230, 3) &
        &/= (((o - gal) + 1)) .or. size(x_cb230, 4) /= ((o + 1))) then
            deallocate(x_cb230)
            allocate(x_cb230(np_particles, ((o - gal) + 1), ((o - gal) + 1), (o + 1)))
        end if
        do x_w0_1750 = 0, ((o + 1)) - 1
            do x_w1_1751 = 0, (((o - gal) + 1)) - 1
                do x_w2_1752 = 0, (((o - gal) + 1)) - 1
                    do x_w3_1753 = 0, (np_particles) - 1
                        x_cb230((x_w3_1753) + 1, (x_w2_1752) + 1, (x_w1_1751) + 1, (x_w0_1750) + 1) = &
                        &(x_inl61_weight((x_w3_1753) + 1, (x_w2_1752) + 1, (x_w1_1751) + 1, (x_w0_1750) + 1) * &
                        &x_inl61_gathered((x_w3_1753) + 1, (x_w2_1752) + 1, (x_w1_1751) + 1, (x_w0_1750) + 1))
                    end do
                end do
            end do
        end do
        ! numpy: np.sum(__cb230, axis=(0, 1, 2))
        do x_ax0_1754 = 0, (np_particles) - 1
            x_cb231((x_ax0_1754) + 1) = 0.0_c_double
            do x_rd0_1755 = 0, ((o + 1)) - 1
                do x_rd1_1756 = 0, (((o - gal) + 1)) - 1
                    do x_rd2_1757 = 0, (((o - gal) + 1)) - 1
                        x_cb231((x_ax0_1754) + 1) = (x_cb231((x_ax0_1754) + 1) + x_cb230((x_ax0_1754) + 1, &
                        &(x_rd2_1757) + 1, (x_rd1_1756) + 1, (x_rd0_1755) + 1))
                    end do
                end do
            end do
        end do
        do x_w0_1758 = 0, (np_particles) - 1
            x_hcall34((x_w0_1758) + 1) = x_cb231((x_w0_1758) + 1)
        end do
        do x_w0_1759 = 0, (np_particles) - 1
            Bxp((x_w0_1759) + 1) = Bxp((x_w0_1759) + 1) + (x_hcall34((x_w0_1759) + 1))
        end do
    end if

end subroutine warpx_field_gather_fp64
