! hpcagent_bench-autogen -- generated from warpx_esirkepov_deposition_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine warpx_esirkepov_deposition_fp32(Jx, Jy, Jz, dinv, ion_lev, lo, reduced_particle_shape_mask, uxp, uyp, uzp, &
&wp, xp, xyzmin, yp, zp, depos_order, do_ionization, dt, enable_reduced_shape, geom, n_rz_azimuthal_modes, ncells, &
&np_particles, q, relative_time) bind(C, name="warpx_esirkepov_deposition_fp32")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), value, intent(in) :: depos_order
    integer(c_int64_t), value, intent(in) :: do_ionization
    integer(c_int64_t), value, intent(in) :: enable_reduced_shape
    integer(c_int64_t), value :: geom
    integer(c_int64_t), value, intent(in) :: n_rz_azimuthal_modes
    integer(c_int64_t), value, intent(in) :: ncells
    integer(c_int64_t), value, intent(in) :: np_particles
    real(c_float), intent(inout) :: Jx(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), ((ncells &
    &+ (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    real(c_float), intent(inout) :: Jy(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), ((ncells &
    &+ (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    real(c_float), intent(inout) :: Jz(((2 * n_rz_azimuthal_modes) - 1), ((ncells + (2 * depos_order)) + 6), ((ncells &
    &+ (2 * depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    real(c_float), intent(in) :: dinv(3)
    integer(c_int32_t), intent(in) :: ion_lev(np_particles)
    integer(c_int32_t), intent(in) :: lo(3)
    integer(c_int32_t), intent(in) :: reduced_particle_shape_mask(((ncells + (2 * depos_order)) + 6), ((ncells + (2 * &
    &depos_order)) + 6), ((ncells + (2 * depos_order)) + 6))
    real(c_float), intent(in) :: uxp(np_particles)
    real(c_float), intent(in) :: uyp(np_particles)
    real(c_float), intent(in) :: uzp(np_particles)
    real(c_float), intent(in) :: wp(np_particles)
    real(c_float), intent(in) :: xp(np_particles)
    real(c_float), intent(in) :: xyzmin(3)
    real(c_float), intent(in) :: yp(np_particles)
    real(c_float), intent(in) :: zp(np_particles)
    real(c_float), value, intent(in) :: dt
    real(c_float), value, intent(in) :: q
    real(c_float), value, intent(in) :: relative_time
    integer(c_int64_t) :: imode_l474, imode_l490, imode_l509, ip_l443, si0_l118, si0_l122, si0_l123, si0_l127, &
    &si0_l128, si0_l129, si0_l133, si0_l134, si0_l135, si0_l136, si0_l142, si0_l143, si0_l144, si0_l145, si0_l146, &
    &si0_l221, si0_l225, si0_l226, si0_l230, si0_l231, si0_l232, si0_l236, si0_l237, si0_l238, si0_l239, si0_l245, &
    &si0_l246, si0_l247, si0_l248, si0_l249, si0_l324, si0_l328, si0_l329, si0_l333, si0_l334, si0_l335, si0_l339, &
    &si0_l340, si0_l341, si0_l342, si0_l348, si0_l349, si0_l350, si0_l351, si0_l352, si0_l448, si0_l455, si0_l462, &
    &si0_l470, si0_l475, si0_l477, si0_l481, si0_l496, si0_l498, si0_l505, si0_l510, si0_l512, si0_l515, si0_l516, &
    &si0_l519, si0_l522, si0_l524, si0_l525, si1_l449, si1_l456, si1_l463, si1_l471, si1_l476, si1_l478, si1_l482, &
    &si1_l497, si1_l499, si1_l506, si1_l511, si1_l513, si2_l450, si2_l457, si2_l464, x_cs0_447, x_cs0_454, x_cs0_461, &
    &x_cs0_466, x_cs0_501, x_cs0_518, x_cs0_521, x_i_151, x_i_192, x_i_202, x_i_254, x_i_295, x_i_305, x_i_357, &
    &x_i_398, x_i_408, x_i_487, x_r0_0, x_r0_13, x_r0_15, x_r0_17, x_r0_19, x_r0_212, x_r0_216, x_r0_23, x_r0_25, &
    &x_r0_27, x_r0_29, x_r0_31, x_r0_315, x_r0_319, x_r0_33, x_r0_35, x_r0_37, x_r0_418, x_r0_422, x_r0_431, x_r0_433, &
    &x_r0_435, x_r0_437, x_r0_439, x_r0_441, x_r0_58, x_r0_60, x_r0_62, x_r0_64, x_r0_66, x_r0_68, x_r0_70, x_r0_72, &
    &x_r1_213, x_r1_217, x_r1_316, x_r1_320, x_r1_419, x_r1_423, x_sc0_155, x_sc0_160, x_sc0_161, x_sc0_166, &
    &x_sc0_167, x_sc0_168, x_sc0_173, x_sc0_174, x_sc0_175, x_sc0_176, x_sc0_183, x_sc0_184, x_sc0_185, x_sc0_186, &
    &x_sc0_187, x_sc0_197, x_sc0_198, x_sc0_207, x_sc0_208, x_sc0_258, x_sc0_263, x_sc0_264, x_sc0_269, x_sc0_270, &
    &x_sc0_271, x_sc0_276, x_sc0_277, x_sc0_278, x_sc0_279, x_sc0_286, x_sc0_287, x_sc0_288, x_sc0_289, x_sc0_290, &
    &x_sc0_300, x_sc0_301, x_sc0_310, x_sc0_311, x_sc0_361, x_sc0_366, x_sc0_367, x_sc0_372, x_sc0_373, x_sc0_374, &
    &x_sc0_379, x_sc0_380, x_sc0_381, x_sc0_382, x_sc0_389, x_sc0_390, x_sc0_391, x_sc0_392, x_sc0_393, x_sc0_403, &
    &x_sc0_404, x_sc0_413, x_sc0_414, x_w0_1, x_w0_10, x_w0_100, x_w0_101, x_w0_102, x_w0_103, x_w0_104, x_w0_105, &
    &x_w0_106, x_w0_107, x_w0_108, x_w0_109, x_w0_11, x_w0_110, x_w0_111, x_w0_112, x_w0_113, x_w0_114, x_w0_115, &
    &x_w0_116, x_w0_117, x_w0_119, x_w0_12, x_w0_120, x_w0_121, x_w0_124, x_w0_125, x_w0_126, x_w0_130, x_w0_131, &
    &x_w0_132, x_w0_137, x_w0_138, x_w0_139, x_w0_14, x_w0_140, x_w0_141, x_w0_147, x_w0_148, x_w0_150, x_w0_152, &
    &x_w0_153, x_w0_154, x_w0_156, x_w0_157, x_w0_158, x_w0_159, x_w0_16, x_w0_162, x_w0_163, x_w0_164, x_w0_165, &
    &x_w0_169, x_w0_170, x_w0_171, x_w0_172, x_w0_177, x_w0_178, x_w0_179, x_w0_18, x_w0_180, x_w0_181, x_w0_182, &
    &x_w0_188, x_w0_189, x_w0_191, x_w0_193, x_w0_194, x_w0_195, x_w0_196, x_w0_199, x_w0_2, x_w0_20, x_w0_200, &
    &x_w0_203, x_w0_204, x_w0_205, x_w0_206, x_w0_209, x_w0_21, x_w0_210, x_w0_214, x_w0_218, x_w0_22, x_w0_220, &
    &x_w0_222, x_w0_223, x_w0_224, x_w0_227, x_w0_228, x_w0_229, x_w0_233, x_w0_234, x_w0_235, x_w0_24, x_w0_240, &
    &x_w0_241, x_w0_242, x_w0_243, x_w0_244, x_w0_250, x_w0_251, x_w0_253, x_w0_255, x_w0_256, x_w0_257, x_w0_259, &
    &x_w0_26, x_w0_260, x_w0_261, x_w0_262, x_w0_265, x_w0_266, x_w0_267, x_w0_268, x_w0_272, x_w0_273, x_w0_274, &
    &x_w0_275, x_w0_28, x_w0_280, x_w0_281, x_w0_282, x_w0_283, x_w0_284, x_w0_285, x_w0_291, x_w0_292, x_w0_294, &
    &x_w0_296, x_w0_297, x_w0_298, x_w0_299, x_w0_3, x_w0_30, x_w0_302, x_w0_303, x_w0_306, x_w0_307, x_w0_308, &
    &x_w0_309, x_w0_312, x_w0_313, x_w0_317, x_w0_32, x_w0_321, x_w0_323, x_w0_325, x_w0_326, x_w0_327, x_w0_330, &
    &x_w0_331, x_w0_332, x_w0_336, x_w0_337, x_w0_338, x_w0_34, x_w0_343, x_w0_344, x_w0_345, x_w0_346, x_w0_347, &
    &x_w0_353, x_w0_354, x_w0_356, x_w0_358, x_w0_359, x_w0_36, x_w0_360, x_w0_362, x_w0_363, x_w0_364, x_w0_365, &
    &x_w0_368, x_w0_369, x_w0_370, x_w0_371, x_w0_375, x_w0_376, x_w0_377, x_w0_378, x_w0_38, x_w0_383, x_w0_384, &
    &x_w0_385, x_w0_386, x_w0_387, x_w0_388, x_w0_39, x_w0_394, x_w0_395, x_w0_397, x_w0_399, x_w0_4, x_w0_40, &
    &x_w0_400, x_w0_401, x_w0_402, x_w0_405, x_w0_406, x_w0_409, x_w0_41, x_w0_410, x_w0_411, x_w0_412, x_w0_415, &
    &x_w0_416, x_w0_42, x_w0_420, x_w0_424, x_w0_426, x_w0_427, x_w0_428, x_w0_429, x_w0_43, x_w0_430, x_w0_432, &
    &x_w0_434, x_w0_436, x_w0_438, x_w0_44, x_w0_440, x_w0_442, x_w0_444, x_w0_446, x_w0_45, x_w0_451, x_w0_453, &
    &x_w0_458, x_w0_46, x_w0_460, x_w0_465, x_w0_467, x_w0_468, x_w0_47, x_w0_472, x_w0_479, x_w0_48, x_w0_483, &
    &x_w0_485, x_w0_488, x_w0_489, x_w0_49, x_w0_491, x_w0_493, x_w0_495, x_w0_5, x_w0_50, x_w0_500, x_w0_502, &
    &x_w0_503, x_w0_507, x_w0_51, x_w0_514, x_w0_517, x_w0_52, x_w0_520, x_w0_523, x_w0_53, x_w0_54, x_w0_55, x_w0_56, &
    &x_w0_57, x_w0_59, x_w0_6, x_w0_61, x_w0_63, x_w0_65, x_w0_67, x_w0_69, x_w0_7, x_w0_71, x_w0_73, x_w0_74, &
    &x_w0_75, x_w0_76, x_w0_77, x_w0_78, x_w0_79, x_w0_8, x_w0_80, x_w0_81, x_w0_82, x_w0_83, x_w0_84, x_w0_85, &
    &x_w0_86, x_w0_87, x_w0_88, x_w0_89, x_w0_9, x_w0_90, x_w0_91, x_w0_92, x_w0_93, x_w0_94, x_w0_95, x_w0_96, &
    &x_w0_97, x_w0_98, x_w0_99, x_w1_149, x_w1_190, x_w1_201, x_w1_211, x_w1_215, x_w1_219, x_w1_252, x_w1_293, &
    &x_w1_304, x_w1_314, x_w1_318, x_w1_322, x_w1_355, x_w1_396, x_w1_407, x_w1_417, x_w1_421, x_w1_425, x_w1_445, &
    &x_w1_452, x_w1_459, x_w1_469, x_w1_473, x_w1_480, x_w1_484, x_w1_486, x_w1_492, x_w1_494, x_w1_504, x_w1_508
    integer(c_int64_t) :: o
    integer(c_int64_t) :: n_modes
    integer(c_int64_t) :: do_ion
    logical(c_bool) :: reduce_enabled
    logical(c_bool) :: rz_modes
    real(c_float) :: dinvx
    real(c_float) :: dinvy
    real(c_float) :: dinvz
    real(c_float) :: xmin
    real(c_float) :: ymin
    real(c_float) :: zmin
    integer(c_int64_t) :: lox
    integer(c_int64_t) :: loy
    integer(c_int64_t) :: loz
    real(c_float) :: invvol
    real(c_float) :: invdtd_x
    real(c_float) :: invdtd_y
    real(c_float) :: invdtd_z
    real(c_float) :: half_dt_step
    integer(c_int64_t) :: half
    integer(c_int64_t) :: width
    real(c_float) :: wqi
    integer(c_int64_t) :: i0
    integer(c_int64_t) :: i1
    integer(c_int64_t) :: j0
    integer(c_int64_t) :: j1
    integer(c_int64_t) :: k0
    integer(c_int64_t) :: k1
    integer(c_int64_t) :: ib
    integer(c_int64_t) :: jb
    integer(c_int64_t) :: kb
    integer(c_int64_t) :: i0y
    integer(c_int64_t) :: i1y
    integer(c_int64_t) :: j0y
    integer(c_int64_t) :: j1y
    integer(c_int64_t) :: k0y
    integer(c_int64_t) :: k1y
    integer(c_int64_t) :: i0z
    integer(c_int64_t) :: i1z
    integer(c_int64_t) :: j0z
    integer(c_int64_t) :: j1z
    integer(c_int64_t) :: k0z
    integer(c_int64_t) :: k1z
    real(c_float) :: x_ifexp0
    real(c_float) :: x_ifexp1
    real(c_float) :: x_ifexp2
    real(c_float) :: x_ifexp3
    integer(c_int64_t) :: x_ifexp26
    integer(c_int64_t) :: x_ifexp27
    integer(c_int64_t) :: x_ifexp28
    integer(c_int64_t) :: x_ifexp29
    integer(c_int64_t) :: x_ifexp30
    integer(c_int64_t) :: x_ifexp31
    real(c_float) :: x_ifexp4
    real(c_float) :: x_ifexp5
    real(c_float) :: x_ifexp6
    real(c_float) :: x_ifexp7
    real(c_float) :: x_ifexp8
    real(c_float) :: x_ifexp9
    real(c_float) :: x_ifexp10
    real(c_float) :: x_ifexp11
    real(c_float) :: x_ifexp12
    real(c_float) :: x_ifexp13
    real(c_float) :: x_ifexp14
    real(c_float) :: x_ifexp15
    real(c_float) :: x_ifexp16
    real(c_float) :: x_ifexp17
    real(c_float) :: x_ifexp18
    real(c_float) :: x_ifexp19
    real(c_float) :: xy_mid_re
    real(c_float) :: xy_mid_im
    real(c_float) :: xy_new_re
    real(c_float) :: xy_new_im
    real(c_float) :: xy_old_re
    real(c_float) :: xy_old_im
    integer(c_int64_t) :: i0x
    integer(c_int64_t) :: i1x
    real(c_float) :: x_ifexp20
    real(c_float) :: x_ifexp21
    real(c_float) :: x_ifexp22
    real(c_float) :: x_ifexp23
    real(c_float) :: x_ifexp24
    real(c_float) :: x_ifexp25
    real(c_float) :: nxt_mid_re
    real(c_float) :: nxt_mid_im
    real(c_float) :: nxt_new_re
    real(c_float) :: nxt_new_im
    real(c_float) :: nxt_old_re
    real(c_float) :: nxt_old_im
    real(c_float) :: x_new(np_particles)
    real(c_float) :: x_old(np_particles)
    real(c_float) :: y_new(np_particles)
    real(c_float) :: y_old(np_particles)
    real(c_float) :: z_new(np_particles)
    real(c_float) :: z_old(np_particles)
    real(c_float) :: vx(np_particles)
    real(c_float) :: vy(np_particles)
    real(c_float) :: vz(np_particles)
    real(c_float) :: xy_new0_re(np_particles)
    real(c_float) :: xy_mid0_re(np_particles)
    real(c_float) :: xy_old0_re(np_particles)
    real(c_float) :: xy_new0_im(np_particles)
    real(c_float) :: xy_mid0_im(np_particles)
    real(c_float) :: xy_old0_im(np_particles)
    integer(c_int32_t) :: reduce_shape_old(np_particles)
    integer(c_int32_t) :: reduce_shape_new(np_particles)
    integer(c_int64_t) :: i_new(np_particles)
    real(c_float), allocatable :: x_inl11_sx(:, :)
    real(c_float), allocatable :: x_inl12_sx(:, :)
    real(c_float), allocatable :: x_inl13_sx(:, :)
    real(c_float), allocatable :: x_inl14_sx(:, :)
    real(c_float), allocatable :: x_inl15_sx(:, :)
    real(c_float), allocatable :: x_inl16_sx(:, :)
    real(c_float), allocatable :: x_inl17_sx(:, :)
    real(c_float), allocatable :: x_inl18_sx(:, :)
    real(c_float), allocatable :: x_inl19_sx(:, :)
    real(c_float), allocatable :: x_inl20_sx(:, :)
    real(c_float), allocatable :: x_inl21_sx(:, :)
    real(c_float), allocatable :: x_inl22_sx(:, :)
    integer(c_int64_t) :: dil(np_particles)
    real(c_float) :: x_cb1(np_particles)
    real(c_float) :: x_cb2(np_particles)
    real(c_float) :: x_cb3(np_particles)
    real(c_float) :: x_cb4(np_particles)
    real(c_float) :: x_cb5(np_particles)
    real(c_float) :: x_cb6(np_particles)
    real(c_float) :: x_cb7(np_particles)
    real(c_float) :: x_cb8(np_particles)
    real(c_float) :: x_cb9(np_particles)
    real(c_float) :: x_cb10(np_particles)
    real(c_float) :: x_cb11(np_particles)
    real(c_float) :: x_cb12(np_particles)
    real(c_float) :: x_cb13(np_particles)
    real(c_float) :: x_cb14(np_particles)
    real(c_float) :: x_cb15(np_particles)
    real(c_float) :: x_cb16(np_particles)
    real(c_float) :: x_cb17(np_particles)
    real(c_float) :: x_cb18(np_particles)
    real(c_float) :: x_cb19(np_particles)
    real(c_float) :: x_cb20(np_particles)
    real(c_float) :: x_cb21(np_particles)
    integer(c_int64_t) :: x_cb22(np_particles)
    integer(c_int64_t) :: x_cb23(np_particles)
    integer(c_int64_t) :: x_cb24(np_particles)
    real(c_float), allocatable :: x_cb25(:, :)
    real(c_float), allocatable :: x_cb26(:, :)
    integer(c_int64_t) :: x_cb27(np_particles)
    integer(c_int64_t) :: x_cb28(np_particles)
    integer(c_int64_t) :: x_cb29(np_particles)
    real(c_float), allocatable :: x_cb30(:, :)
    real(c_float), allocatable :: x_cb31(:, :)
    integer(c_int64_t) :: x_cb32(np_particles)
    integer(c_int64_t) :: x_cb33(np_particles)
    integer(c_int64_t) :: x_cb34(np_particles)
    real(c_float), allocatable :: x_cb35(:, :)
    real(c_float), allocatable :: x_cb36(:, :)
    integer(c_int64_t) :: x_cb37(np_particles)
    integer(c_int64_t) :: x_cb38(np_particles)
    integer(c_int64_t) :: x_cb39(np_particles)
    integer(c_int64_t) :: x_cb40(np_particles)
    integer(c_int64_t) :: x_cb41(np_particles)
    integer(c_int64_t) :: x_cb42(np_particles)
    real(c_float), allocatable :: x_cb43(:)
    real(c_float), allocatable :: x_cb45(:)
    real(c_float), allocatable :: x_cb47(:)
    real(c_float), allocatable :: x_cb49(:)
    integer(c_int64_t), allocatable :: x_cb51(:)
    real(c_float), allocatable :: x_cb52(:)
    real(c_float), allocatable :: x_cb54(:)
    real(c_float), allocatable :: x_cb56(:)
    real(c_float), allocatable :: cum_x(:)
    real(c_float), allocatable :: cum_y(:)
    real(c_float), allocatable :: cum_z(:)
    real(c_float), allocatable :: cum_x__v1(:)
    real(c_float) :: gaminv(np_particles)
    real(c_float) :: wq(np_particles)
    real(c_float) :: xp_new(np_particles)
    real(c_float) :: yp_new(np_particles)
    real(c_float) :: xp_mid(np_particles)
    real(c_float) :: yp_mid(np_particles)
    real(c_float) :: xp_old(np_particles)
    real(c_float) :: yp_old(np_particles)
    real(c_float) :: x_inl1_denom_safe(np_particles)
    real(c_float) :: costheta_mid(np_particles)
    real(c_float) :: x_inl2_denom_safe(np_particles)
    real(c_float) :: sintheta_mid(np_particles)
    real(c_float) :: x_inl3_denom_safe(np_particles)
    real(c_float) :: costheta_new(np_particles)
    real(c_float) :: x_inl4_denom_safe(np_particles)
    real(c_float) :: sintheta_new(np_particles)
    real(c_float) :: x_inl5_denom_safe(np_particles)
    real(c_float) :: costheta_old(np_particles)
    real(c_float) :: x_inl6_denom_safe(np_particles)
    real(c_float) :: sintheta_old(np_particles)
    real(c_float) :: zp_new(np_particles)
    real(c_float) :: zp_mid(np_particles)
    real(c_float) :: zp_old(np_particles)
    real(c_float) :: rp_mid(np_particles)
    real(c_float) :: x_inl7_denom_safe(np_particles)
    real(c_float) :: x_inl8_denom_safe(np_particles)
    real(c_float) :: x_inl9_denom_safe(np_particles)
    real(c_float) :: cosphi_mid(np_particles)
    real(c_float) :: x_inl10_denom_safe(np_particles)
    real(c_float) :: sinphi_mid(np_particles)
    integer(c_int64_t) :: i_old(np_particles)
    integer(c_int64_t) :: j_new(np_particles)
    integer(c_int64_t) :: j_old(np_particles)
    integer(c_int64_t) :: k_new(np_particles)
    integer(c_int64_t) :: k_old(np_particles)
    integer(c_int64_t) :: x_inl11_idx(np_particles)
    real(c_float) :: x_inl11_xint(np_particles)
    real(c_float) :: x_inl11_sm(np_particles)
    real(c_float) :: x_inl11_sp(np_particles)
    real(c_float), allocatable :: sx_new(:, :)
    integer(c_int64_t) :: x_inl12_rows(np_particles)
    integer(c_int64_t) :: x_inl12_i_shift(np_particles)
    integer(c_int64_t) :: x_inl12_idx(np_particles)
    real(c_float) :: x_inl12_xint(np_particles)
    real(c_float) :: x_inl12_sm(np_particles)
    real(c_float) :: x_inl12_sp(np_particles)
    real(c_float), allocatable :: sx_old(:, :)
    integer(c_int64_t) :: x_inl13_rows(np_particles)
    integer(c_int64_t) :: x_inl13_i_shift(np_particles)
    real(c_float) :: x_inl13_xint(np_particles)
    integer(c_int64_t) :: x_inl13_idx(np_particles)
    real(c_float), allocatable :: ov_new(:, :)
    integer(c_int64_t) :: x_inl14_rows(np_particles)
    integer(c_int64_t) :: x_inl14_i_shift(np_particles)
    real(c_float) :: x_inl14_xint(np_particles)
    integer(c_int64_t) :: x_inl14_idx(np_particles)
    real(c_float), allocatable :: ov_old(:, :)
    integer(c_int64_t) :: x_inl15_idx(np_particles)
    real(c_float) :: x_inl15_xint(np_particles)
    real(c_float) :: x_inl15_sm(np_particles)
    real(c_float) :: x_inl15_sp(np_particles)
    real(c_float), allocatable :: sy_new(:, :)
    integer(c_int64_t) :: x_inl16_rows(np_particles)
    integer(c_int64_t) :: x_inl16_i_shift(np_particles)
    integer(c_int64_t) :: x_inl16_idx(np_particles)
    real(c_float) :: x_inl16_xint(np_particles)
    real(c_float) :: x_inl16_sm(np_particles)
    real(c_float) :: x_inl16_sp(np_particles)
    real(c_float), allocatable :: sy_old(:, :)
    integer(c_int64_t) :: x_inl17_rows(np_particles)
    integer(c_int64_t) :: x_inl17_i_shift(np_particles)
    real(c_float) :: x_inl17_xint(np_particles)
    integer(c_int64_t) :: x_inl17_idx(np_particles)
    integer(c_int64_t) :: x_inl18_rows(np_particles)
    integer(c_int64_t) :: x_inl18_i_shift(np_particles)
    real(c_float) :: x_inl18_xint(np_particles)
    integer(c_int64_t) :: x_inl18_idx(np_particles)
    integer(c_int64_t) :: x_inl19_idx(np_particles)
    real(c_float) :: x_inl19_xint(np_particles)
    real(c_float) :: x_inl19_sm(np_particles)
    real(c_float) :: x_inl19_sp(np_particles)
    real(c_float), allocatable :: sz_new(:, :)
    integer(c_int64_t) :: x_inl20_rows(np_particles)
    integer(c_int64_t) :: x_inl20_i_shift(np_particles)
    integer(c_int64_t) :: x_inl20_idx(np_particles)
    real(c_float) :: x_inl20_xint(np_particles)
    real(c_float) :: x_inl20_sm(np_particles)
    real(c_float) :: x_inl20_sp(np_particles)
    real(c_float), allocatable :: sz_old(:, :)
    integer(c_int64_t) :: x_inl21_rows(np_particles)
    integer(c_int64_t) :: x_inl21_i_shift(np_particles)
    real(c_float) :: x_inl21_xint(np_particles)
    integer(c_int64_t) :: x_inl21_idx(np_particles)
    integer(c_int64_t) :: x_inl22_rows(np_particles)
    integer(c_int64_t) :: x_inl22_i_shift(np_particles)
    real(c_float) :: x_inl22_xint(np_particles)
    integer(c_int64_t) :: x_inl22_idx(np_particles)
    integer(c_int64_t) :: diu(np_particles)
    integer(c_int64_t) :: djl(np_particles)
    integer(c_int64_t) :: dju(np_particles)
    integer(c_int64_t) :: dkl(np_particles)
    integer(c_int64_t) :: dku(np_particles)
    real(c_float), allocatable :: gx(:, :)
    real(c_float), allocatable :: gy(:, :)
    real(c_float), allocatable :: gz(:, :)
    real(c_float), allocatable :: zavg_x(:)
    real(c_float), allocatable :: sdxi(:, :)
    real(c_float), allocatable :: djr(:, :)
    real(c_float), allocatable :: sdyj(:, :)
    real(c_float), allocatable :: a_re(:, :)
    real(c_float), allocatable :: b_re(:, :)
    integer(c_int64_t), allocatable :: i_local(:)
    real(c_float), allocatable :: neg2coef(:)
    real(c_float), allocatable :: sum_re(:, :)
    real(c_float), allocatable :: sum_im(:, :)
    real(c_float), allocatable :: coef_m(:)
    real(c_float), allocatable :: xavg_z(:)
    real(c_float), allocatable :: sdzk(:, :)
    real(c_float), allocatable :: djz(:, :)
    real(c_float), allocatable :: zavg(:)
    real(c_float), allocatable :: xavg(:)
    real(c_float) :: rp_new(np_particles)
    real(c_float) :: rp_old(np_particles)
    real(c_float) :: rpxy_mid(np_particles)
    integer(c_int64_t) :: fx_o(np_particles)
    integer(c_int64_t) :: fy_o(np_particles)
    integer(c_int64_t) :: fz_o(np_particles)
    integer(c_int64_t) :: fx_n(np_particles)
    integer(c_int64_t) :: fy_n(np_particles)
    integer(c_int64_t) :: fz_n(np_particles)
    integer(c_int64_t) :: x_inl11_j(np_particles)
    integer(c_int64_t) :: x_inl12_i(np_particles)
    integer(c_int64_t) :: x_inl13_i(np_particles)
    integer(c_int64_t) :: x_inl14_i(np_particles)
    integer(c_int64_t) :: x_inl15_j(np_particles)
    integer(c_int64_t) :: x_inl16_i(np_particles)
    integer(c_int64_t) :: x_inl17_i(np_particles)
    integer(c_int64_t) :: x_inl18_i(np_particles)
    integer(c_int64_t) :: x_inl19_j(np_particles)
    integer(c_int64_t) :: x_inl20_i(np_particles)
    integer(c_int64_t) :: x_inl21_i(np_particles)
    integer(c_int64_t) :: x_inl22_i(np_particles)
    o = INT(depos_order, c_int64_t)
    geom = INT(geom, c_int64_t)
    n_modes = INT(n_rz_azimuthal_modes, c_int64_t)
    do_ion = INT(do_ionization, c_int64_t)
    reduce_enabled = ((INT(enable_reduced_shape, c_int64_t) /= 0) .AND. (o > 1))
    rz_modes = ((geom == 2) .AND. (n_modes > 1))
    dinvx = dinv((0) + 1)
    dinvy = dinv((1) + 1)
    dinvz = dinv((2) + 1)
    xmin = xyzmin((0) + 1)
    ymin = xyzmin((1) + 1)
    zmin = xyzmin((2) + 1)
    lox = INT(INT(lo((0) + 1), c_int64_t), c_int64_t)
    loy = INT(INT(lo((1) + 1), c_int64_t), c_int64_t)
    loz = INT(INT(lo((2) + 1), c_int64_t), c_int64_t)
    invvol = ((dinvx * dinvy) * dinvz)
    invdtd_x = (((1.0_c_float / dt) * dinvy) * dinvz)
    invdtd_y = (((1.0_c_float / dt) * dinvx) * dinvz)
    invdtd_z = (((1.0_c_float / dt) * dinvx) * dinvy)
    ! numpy: np.sqrt(1.0 + (uxp * uxp + uyp * uyp + uzp * uzp) * 1.1126500560536185e-17)
    do x_r0_0 = 0, (np_particles) - 1
        x_cb1((x_r0_0) + 1) = SQRT((1.0_c_float + ((((uxp((x_r0_0) + 1) * uxp((x_r0_0) + 1)) + (uyp((x_r0_0) + 1) * &
        &uyp((x_r0_0) + 1))) + (uzp((x_r0_0) + 1) * uzp((x_r0_0) + 1))) * 1.1126500560536185e-17_c_float)))
    end do
    do x_w0_1 = 0, (np_particles) - 1
        gaminv((x_w0_1) + 1) = (1.0_c_float / x_cb1((x_w0_1) + 1))
    end do
    do x_w0_2 = 0, (np_particles) - 1
        wq((x_w0_2) + 1) = (q * wp((x_w0_2) + 1))
    end do
    if ((do_ion /= 0)) then
        do x_w0_3 = 0, (np_particles) - 1
            wq((x_w0_3) + 1) = (wq((x_w0_3) + 1) * INT(ion_lev((x_w0_3) + 1), c_int64_t))
        end do
    end if
    half_dt_step = (relative_time + (0.5_c_float * dt))
    x_new = 0
    x_old = 0
    y_new = 0
    y_old = 0
    z_new = 0
    z_old = 0
    vx = 0
    vy = 0
    vz = 0
    xy_new0_re = 0
    xy_mid0_re = 0
    xy_old0_re = 0
    xy_new0_im = 0
    xy_mid0_im = 0
    xy_old0_im = 0
    if (((geom == 2) .OR. (geom == 4))) then
        do x_w0_4 = 0, (np_particles) - 1
            xp_new((x_w0_4) + 1) = (xp((x_w0_4) + 1) + ((half_dt_step * uxp((x_w0_4) + 1)) * gaminv((x_w0_4) + 1)))
        end do
        do x_w0_5 = 0, (np_particles) - 1
            yp_new((x_w0_5) + 1) = (yp((x_w0_5) + 1) + ((half_dt_step * uyp((x_w0_5) + 1)) * gaminv((x_w0_5) + 1)))
        end do
        do x_w0_6 = 0, (np_particles) - 1
            xp_mid((x_w0_6) + 1) = (xp_new((x_w0_6) + 1) - (((0.5_c_float * dt) * uxp((x_w0_6) + 1)) * gaminv((x_w0_6) &
            &+ 1)))
        end do
        do x_w0_7 = 0, (np_particles) - 1
            yp_mid((x_w0_7) + 1) = (yp_new((x_w0_7) + 1) - (((0.5_c_float * dt) * uyp((x_w0_7) + 1)) * gaminv((x_w0_7) &
            &+ 1)))
        end do
        do x_w0_8 = 0, (np_particles) - 1
            xp_old((x_w0_8) + 1) = (xp_new((x_w0_8) + 1) - ((dt * uxp((x_w0_8) + 1)) * gaminv((x_w0_8) + 1)))
        end do
        do x_w0_9 = 0, (np_particles) - 1
            yp_old((x_w0_9) + 1) = (yp_new((x_w0_9) + 1) - ((dt * uyp((x_w0_9) + 1)) * gaminv((x_w0_9) + 1)))
        end do
        do x_w0_10 = 0, (np_particles) - 1
            rp_new((x_w0_10) + 1) = hypot(xp_new((x_w0_10) + 1), yp_new((x_w0_10) + 1))
        end do
        do x_w0_11 = 0, (np_particles) - 1
            rp_mid((x_w0_11) + 1) = hypot(xp_mid((x_w0_11) + 1), yp_mid((x_w0_11) + 1))
        end do
        do x_w0_12 = 0, (np_particles) - 1
            rp_old((x_w0_12) + 1) = hypot(xp_old((x_w0_12) + 1), yp_old((x_w0_12) + 1))
        end do
        ! numpy: np.where(rp_mid > 0.0, rp_mid, 1.0)
        do x_r0_13 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_13) + 1) > 0.0_c_float)) then
                x_ifexp0 = rp_mid((x_r0_13) + 1)
            else
                x_ifexp0 = 1.0_c_float
            end if
            x_cb2((x_r0_13) + 1) = x_ifexp0
        end do
        do x_w0_14 = 0, (np_particles) - 1
            x_inl1_denom_safe((x_w0_14) + 1) = x_cb2((x_w0_14) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, xp_mid / __inl1_denom_safe, 1.0)
        do x_r0_15 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_15) + 1) > 0.0_c_float)) then
                x_ifexp1 = (xp_mid((x_r0_15) + 1) / x_inl1_denom_safe((x_r0_15) + 1))
            else
                x_ifexp1 = 1.0_c_float
            end if
            x_cb3((x_r0_15) + 1) = x_ifexp1
        end do
        do x_w0_16 = 0, (np_particles) - 1
            costheta_mid((x_w0_16) + 1) = x_cb3((x_w0_16) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, rp_mid, 1.0)
        do x_r0_17 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_17) + 1) > 0.0_c_float)) then
                x_ifexp2 = rp_mid((x_r0_17) + 1)
            else
                x_ifexp2 = 1.0_c_float
            end if
            x_cb4((x_r0_17) + 1) = x_ifexp2
        end do
        do x_w0_18 = 0, (np_particles) - 1
            x_inl2_denom_safe((x_w0_18) + 1) = x_cb4((x_w0_18) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, yp_mid / __inl2_denom_safe, 0.0)
        do x_r0_19 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_19) + 1) > 0.0_c_float)) then
                x_ifexp3 = (yp_mid((x_r0_19) + 1) / x_inl2_denom_safe((x_r0_19) + 1))
            else
                x_ifexp3 = 0.0_c_float
            end if
            x_cb5((x_r0_19) + 1) = x_ifexp3
        end do
        do x_w0_20 = 0, (np_particles) - 1
            sintheta_mid((x_w0_20) + 1) = x_cb5((x_w0_20) + 1)
        end do
        do x_w0_21 = 0, (np_particles) - 1
            x_new((x_w0_21) + 1) = ((rp_new((x_w0_21) + 1) - xmin) * dinvx)
        end do
        do x_w0_22 = 0, (np_particles) - 1
            x_old((x_w0_22) + 1) = ((rp_old((x_w0_22) + 1) - xmin) * dinvx)
        end do
        if ((geom == 2)) then
            ! numpy: np.where(rp_new > 0.0, rp_new, 1.0)
            do x_r0_23 = 0, (np_particles) - 1
                if ((rp_new((x_r0_23) + 1) > 0.0_c_float)) then
                    x_ifexp4 = rp_new((x_r0_23) + 1)
                else
                    x_ifexp4 = 1.0_c_float
                end if
                x_cb6((x_r0_23) + 1) = x_ifexp4
            end do
            do x_w0_24 = 0, (np_particles) - 1
                x_inl3_denom_safe((x_w0_24) + 1) = x_cb6((x_w0_24) + 1)
            end do
            ! numpy: np.where(rp_new > 0.0, xp_new / __inl3_denom_safe, 1.0)
            do x_r0_25 = 0, (np_particles) - 1
                if ((rp_new((x_r0_25) + 1) > 0.0_c_float)) then
                    x_ifexp5 = (xp_new((x_r0_25) + 1) / x_inl3_denom_safe((x_r0_25) + 1))
                else
                    x_ifexp5 = 1.0_c_float
                end if
                x_cb7((x_r0_25) + 1) = x_ifexp5
            end do
            do x_w0_26 = 0, (np_particles) - 1
                costheta_new((x_w0_26) + 1) = x_cb7((x_w0_26) + 1)
            end do
            ! numpy: np.where(rp_new > 0.0, rp_new, 1.0)
            do x_r0_27 = 0, (np_particles) - 1
                if ((rp_new((x_r0_27) + 1) > 0.0_c_float)) then
                    x_ifexp6 = rp_new((x_r0_27) + 1)
                else
                    x_ifexp6 = 1.0_c_float
                end if
                x_cb8((x_r0_27) + 1) = x_ifexp6
            end do
            do x_w0_28 = 0, (np_particles) - 1
                x_inl4_denom_safe((x_w0_28) + 1) = x_cb8((x_w0_28) + 1)
            end do
            ! numpy: np.where(rp_new > 0.0, yp_new / __inl4_denom_safe, 0.0)
            do x_r0_29 = 0, (np_particles) - 1
                if ((rp_new((x_r0_29) + 1) > 0.0_c_float)) then
                    x_ifexp7 = (yp_new((x_r0_29) + 1) / x_inl4_denom_safe((x_r0_29) + 1))
                else
                    x_ifexp7 = 0.0_c_float
                end if
                x_cb9((x_r0_29) + 1) = x_ifexp7
            end do
            do x_w0_30 = 0, (np_particles) - 1
                sintheta_new((x_w0_30) + 1) = x_cb9((x_w0_30) + 1)
            end do
            ! numpy: np.where(rp_old > 0.0, rp_old, 1.0)
            do x_r0_31 = 0, (np_particles) - 1
                if ((rp_old((x_r0_31) + 1) > 0.0_c_float)) then
                    x_ifexp8 = rp_old((x_r0_31) + 1)
                else
                    x_ifexp8 = 1.0_c_float
                end if
                x_cb10((x_r0_31) + 1) = x_ifexp8
            end do
            do x_w0_32 = 0, (np_particles) - 1
                x_inl5_denom_safe((x_w0_32) + 1) = x_cb10((x_w0_32) + 1)
            end do
            ! numpy: np.where(rp_old > 0.0, xp_old / __inl5_denom_safe, 1.0)
            do x_r0_33 = 0, (np_particles) - 1
                if ((rp_old((x_r0_33) + 1) > 0.0_c_float)) then
                    x_ifexp9 = (xp_old((x_r0_33) + 1) / x_inl5_denom_safe((x_r0_33) + 1))
                else
                    x_ifexp9 = 1.0_c_float
                end if
                x_cb11((x_r0_33) + 1) = x_ifexp9
            end do
            do x_w0_34 = 0, (np_particles) - 1
                costheta_old((x_w0_34) + 1) = x_cb11((x_w0_34) + 1)
            end do
            ! numpy: np.where(rp_old > 0.0, rp_old, 1.0)
            do x_r0_35 = 0, (np_particles) - 1
                if ((rp_old((x_r0_35) + 1) > 0.0_c_float)) then
                    x_ifexp10 = rp_old((x_r0_35) + 1)
                else
                    x_ifexp10 = 1.0_c_float
                end if
                x_cb12((x_r0_35) + 1) = x_ifexp10
            end do
            do x_w0_36 = 0, (np_particles) - 1
                x_inl6_denom_safe((x_w0_36) + 1) = x_cb12((x_w0_36) + 1)
            end do
            ! numpy: np.where(rp_old > 0.0, yp_old / __inl6_denom_safe, 0.0)
            do x_r0_37 = 0, (np_particles) - 1
                if ((rp_old((x_r0_37) + 1) > 0.0_c_float)) then
                    x_ifexp11 = (yp_old((x_r0_37) + 1) / x_inl6_denom_safe((x_r0_37) + 1))
                else
                    x_ifexp11 = 0.0_c_float
                end if
                x_cb13((x_r0_37) + 1) = x_ifexp11
            end do
            do x_w0_38 = 0, (np_particles) - 1
                sintheta_old((x_w0_38) + 1) = x_cb13((x_w0_38) + 1)
            end do
            do x_w0_39 = 0, (np_particles) - 1
                xy_new0_re((x_w0_39) + 1) = costheta_new((x_w0_39) + 1)
            end do
            do x_w0_40 = 0, (np_particles) - 1
                xy_new0_im((x_w0_40) + 1) = sintheta_new((x_w0_40) + 1)
            end do
            do x_w0_41 = 0, (np_particles) - 1
                xy_mid0_re((x_w0_41) + 1) = costheta_mid((x_w0_41) + 1)
            end do
            do x_w0_42 = 0, (np_particles) - 1
                xy_mid0_im((x_w0_42) + 1) = sintheta_mid((x_w0_42) + 1)
            end do
            do x_w0_43 = 0, (np_particles) - 1
                xy_old0_re((x_w0_43) + 1) = costheta_old((x_w0_43) + 1)
            end do
            do x_w0_44 = 0, (np_particles) - 1
                xy_old0_im((x_w0_44) + 1) = sintheta_old((x_w0_44) + 1)
            end do
        end if
    else if ((geom == 5)) then
        do x_w0_45 = 0, (np_particles) - 1
            xp_new((x_w0_45) + 1) = (xp((x_w0_45) + 1) + ((half_dt_step * uxp((x_w0_45) + 1)) * gaminv((x_w0_45) + 1)))
        end do
        do x_w0_46 = 0, (np_particles) - 1
            yp_new((x_w0_46) + 1) = (yp((x_w0_46) + 1) + ((half_dt_step * uyp((x_w0_46) + 1)) * gaminv((x_w0_46) + 1)))
        end do
        do x_w0_47 = 0, (np_particles) - 1
            zp_new((x_w0_47) + 1) = (zp((x_w0_47) + 1) + ((half_dt_step * uzp((x_w0_47) + 1)) * gaminv((x_w0_47) + 1)))
        end do
        do x_w0_48 = 0, (np_particles) - 1
            xp_mid((x_w0_48) + 1) = (xp_new((x_w0_48) + 1) - (((0.5_c_float * dt) * uxp((x_w0_48) + 1)) * &
            &gaminv((x_w0_48) + 1)))
        end do
        do x_w0_49 = 0, (np_particles) - 1
            yp_mid((x_w0_49) + 1) = (yp_new((x_w0_49) + 1) - (((0.5_c_float * dt) * uyp((x_w0_49) + 1)) * &
            &gaminv((x_w0_49) + 1)))
        end do
        do x_w0_50 = 0, (np_particles) - 1
            zp_mid((x_w0_50) + 1) = (zp_new((x_w0_50) + 1) - (((0.5_c_float * dt) * uzp((x_w0_50) + 1)) * &
            &gaminv((x_w0_50) + 1)))
        end do
        do x_w0_51 = 0, (np_particles) - 1
            xp_old((x_w0_51) + 1) = (xp_new((x_w0_51) + 1) - ((dt * uxp((x_w0_51) + 1)) * gaminv((x_w0_51) + 1)))
        end do
        do x_w0_52 = 0, (np_particles) - 1
            yp_old((x_w0_52) + 1) = (yp_new((x_w0_52) + 1) - ((dt * uyp((x_w0_52) + 1)) * gaminv((x_w0_52) + 1)))
        end do
        do x_w0_53 = 0, (np_particles) - 1
            zp_old((x_w0_53) + 1) = (zp_new((x_w0_53) + 1) - ((dt * uzp((x_w0_53) + 1)) * gaminv((x_w0_53) + 1)))
        end do
        do x_w0_54 = 0, (np_particles) - 1
            rpxy_mid((x_w0_54) + 1) = hypot(xp_mid((x_w0_54) + 1), yp_mid((x_w0_54) + 1))
        end do
        do x_w0_55 = 0, (np_particles) - 1
            rp_new((x_w0_55) + 1) = SQRT((((xp_new((x_w0_55) + 1) * xp_new((x_w0_55) + 1)) + (yp_new((x_w0_55) + 1) * &
            &yp_new((x_w0_55) + 1))) + (zp_new((x_w0_55) + 1) * zp_new((x_w0_55) + 1))))
        end do
        do x_w0_56 = 0, (np_particles) - 1
            rp_old((x_w0_56) + 1) = SQRT((((xp_old((x_w0_56) + 1) * xp_old((x_w0_56) + 1)) + (yp_old((x_w0_56) + 1) * &
            &yp_old((x_w0_56) + 1))) + (zp_old((x_w0_56) + 1) * zp_old((x_w0_56) + 1))))
        end do
        do x_w0_57 = 0, (np_particles) - 1
            rp_mid((x_w0_57) + 1) = ((rp_new((x_w0_57) + 1) + rp_old((x_w0_57) + 1)) * 0.5_c_float)
        end do
        ! numpy: np.where(rpxy_mid > 0.0, rpxy_mid, 1.0)
        do x_r0_58 = 0, (np_particles) - 1
            if ((rpxy_mid((x_r0_58) + 1) > 0.0_c_float)) then
                x_ifexp12 = rpxy_mid((x_r0_58) + 1)
            else
                x_ifexp12 = 1.0_c_float
            end if
            x_cb14((x_r0_58) + 1) = x_ifexp12
        end do
        do x_w0_59 = 0, (np_particles) - 1
            x_inl7_denom_safe((x_w0_59) + 1) = x_cb14((x_w0_59) + 1)
        end do
        ! numpy: np.where(rpxy_mid > 0.0, xp_mid / __inl7_denom_safe, 1.0)
        do x_r0_60 = 0, (np_particles) - 1
            if ((rpxy_mid((x_r0_60) + 1) > 0.0_c_float)) then
                x_ifexp13 = (xp_mid((x_r0_60) + 1) / x_inl7_denom_safe((x_r0_60) + 1))
            else
                x_ifexp13 = 1.0_c_float
            end if
            x_cb15((x_r0_60) + 1) = x_ifexp13
        end do
        do x_w0_61 = 0, (np_particles) - 1
            costheta_mid((x_w0_61) + 1) = x_cb15((x_w0_61) + 1)
        end do
        ! numpy: np.where(rpxy_mid > 0.0, rpxy_mid, 1.0)
        do x_r0_62 = 0, (np_particles) - 1
            if ((rpxy_mid((x_r0_62) + 1) > 0.0_c_float)) then
                x_ifexp14 = rpxy_mid((x_r0_62) + 1)
            else
                x_ifexp14 = 1.0_c_float
            end if
            x_cb16((x_r0_62) + 1) = x_ifexp14
        end do
        do x_w0_63 = 0, (np_particles) - 1
            x_inl8_denom_safe((x_w0_63) + 1) = x_cb16((x_w0_63) + 1)
        end do
        ! numpy: np.where(rpxy_mid > 0.0, yp_mid / __inl8_denom_safe, 0.0)
        do x_r0_64 = 0, (np_particles) - 1
            if ((rpxy_mid((x_r0_64) + 1) > 0.0_c_float)) then
                x_ifexp15 = (yp_mid((x_r0_64) + 1) / x_inl8_denom_safe((x_r0_64) + 1))
            else
                x_ifexp15 = 0.0_c_float
            end if
            x_cb17((x_r0_64) + 1) = x_ifexp15
        end do
        do x_w0_65 = 0, (np_particles) - 1
            sintheta_mid((x_w0_65) + 1) = x_cb17((x_w0_65) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, rp_mid, 1.0)
        do x_r0_66 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_66) + 1) > 0.0_c_float)) then
                x_ifexp16 = rp_mid((x_r0_66) + 1)
            else
                x_ifexp16 = 1.0_c_float
            end if
            x_cb18((x_r0_66) + 1) = x_ifexp16
        end do
        do x_w0_67 = 0, (np_particles) - 1
            x_inl9_denom_safe((x_w0_67) + 1) = x_cb18((x_w0_67) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, rpxy_mid / __inl9_denom_safe, 1.0)
        do x_r0_68 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_68) + 1) > 0.0_c_float)) then
                x_ifexp17 = (rpxy_mid((x_r0_68) + 1) / x_inl9_denom_safe((x_r0_68) + 1))
            else
                x_ifexp17 = 1.0_c_float
            end if
            x_cb19((x_r0_68) + 1) = x_ifexp17
        end do
        do x_w0_69 = 0, (np_particles) - 1
            cosphi_mid((x_w0_69) + 1) = x_cb19((x_w0_69) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, rp_mid, 1.0)
        do x_r0_70 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_70) + 1) > 0.0_c_float)) then
                x_ifexp18 = rp_mid((x_r0_70) + 1)
            else
                x_ifexp18 = 1.0_c_float
            end if
            x_cb20((x_r0_70) + 1) = x_ifexp18
        end do
        do x_w0_71 = 0, (np_particles) - 1
            x_inl10_denom_safe((x_w0_71) + 1) = x_cb20((x_w0_71) + 1)
        end do
        ! numpy: np.where(rp_mid > 0.0, zp_mid / __inl10_denom_safe, 0.0)
        do x_r0_72 = 0, (np_particles) - 1
            if ((rp_mid((x_r0_72) + 1) > 0.0_c_float)) then
                x_ifexp19 = (zp_mid((x_r0_72) + 1) / x_inl10_denom_safe((x_r0_72) + 1))
            else
                x_ifexp19 = 0.0_c_float
            end if
            x_cb21((x_r0_72) + 1) = x_ifexp19
        end do
        do x_w0_73 = 0, (np_particles) - 1
            sinphi_mid((x_w0_73) + 1) = x_cb21((x_w0_73) + 1)
        end do
        do x_w0_74 = 0, (np_particles) - 1
            x_new((x_w0_74) + 1) = ((rp_new((x_w0_74) + 1) - xmin) * dinvx)
        end do
        do x_w0_75 = 0, (np_particles) - 1
            x_old((x_w0_75) + 1) = ((rp_old((x_w0_75) + 1) - xmin) * dinvx)
        end do
    else if ((geom /= 0)) then
        do x_w0_76 = 0, (np_particles) - 1
            x_new((x_w0_76) + 1) = (((xp((x_w0_76) + 1) - xmin) + ((half_dt_step * uxp((x_w0_76) + 1)) * &
            &gaminv((x_w0_76) + 1))) * dinvx)
        end do
        do x_w0_77 = 0, (np_particles) - 1
            x_old((x_w0_77) + 1) = (x_new((x_w0_77) + 1) - (((dt * dinvx) * uxp((x_w0_77) + 1)) * gaminv((x_w0_77) + &
            &1)))
        end do
    end if
    if ((geom == 3)) then
        do x_w0_78 = 0, (np_particles) - 1
            y_new((x_w0_78) + 1) = (((yp((x_w0_78) + 1) - ymin) + ((half_dt_step * uyp((x_w0_78) + 1)) * &
            &gaminv((x_w0_78) + 1))) * dinvy)
        end do
        do x_w0_79 = 0, (np_particles) - 1
            y_old((x_w0_79) + 1) = (y_new((x_w0_79) + 1) - (((dt * dinvy) * uyp((x_w0_79) + 1)) * gaminv((x_w0_79) + &
            &1)))
        end do
    end if
    if (((geom /= 4) .AND. (geom /= 5))) then
        do x_w0_80 = 0, (np_particles) - 1
            z_new((x_w0_80) + 1) = (((zp((x_w0_80) + 1) - zmin) + ((half_dt_step * uzp((x_w0_80) + 1)) * &
            &gaminv((x_w0_80) + 1))) * dinvz)
        end do
        do x_w0_81 = 0, (np_particles) - 1
            z_old((x_w0_81) + 1) = (z_new((x_w0_81) + 1) - (((dt * dinvz) * uzp((x_w0_81) + 1)) * gaminv((x_w0_81) + &
            &1)))
        end do
    end if
    reduce_shape_old = 0
    reduce_shape_new = 0
    if (reduce_enabled) then
        if ((geom == 3)) then
            do x_w0_82 = 0, (np_particles) - 1
                fx_o((x_w0_82) + 1) = INT((aint(x_old((x_w0_82) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_old((x_w0_82) + 1)) < aint(x_old((x_w0_82) + 1)))), c_int64_t)
            end do
            do x_w0_83 = 0, (np_particles) - 1
                fy_o((x_w0_83) + 1) = INT((aint(y_old((x_w0_83) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(y_old((x_w0_83) + 1)) < aint(y_old((x_w0_83) + 1)))), c_int64_t)
            end do
            do x_w0_84 = 0, (np_particles) - 1
                fz_o((x_w0_84) + 1) = INT((aint(z_old((x_w0_84) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_old((x_w0_84) + 1)) < aint(z_old((x_w0_84) + 1)))), c_int64_t)
            end do
            do x_w0_85 = 0, (np_particles) - 1
                fx_n((x_w0_85) + 1) = INT((aint(x_new((x_w0_85) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_new((x_w0_85) + 1)) < aint(x_new((x_w0_85) + 1)))), c_int64_t)
            end do
            do x_w0_86 = 0, (np_particles) - 1
                fy_n((x_w0_86) + 1) = INT((aint(y_new((x_w0_86) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(y_new((x_w0_86) + 1)) < aint(y_new((x_w0_86) + 1)))), c_int64_t)
            end do
            do x_w0_87 = 0, (np_particles) - 1
                fz_n((x_w0_87) + 1) = INT((aint(z_new((x_w0_87) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_new((x_w0_87) + 1)) < aint(z_new((x_w0_87) + 1)))), c_int64_t)
            end do
            do x_w0_88 = 0, (np_particles) - 1
                reduce_shape_old((x_w0_88) + 1) = INT(reduced_particle_shape_mask(((loz + fz_o((x_w0_88) + 1))) + 1, &
                &((loy + fy_o((x_w0_88) + 1))) + 1, ((lox + fx_o((x_w0_88) + 1))) + 1), c_int64_t)
            end do
            do x_w0_89 = 0, (np_particles) - 1
                reduce_shape_new((x_w0_89) + 1) = INT(reduced_particle_shape_mask(((loz + fz_n((x_w0_89) + 1))) + 1, &
                &((loy + fy_n((x_w0_89) + 1))) + 1, ((lox + fx_n((x_w0_89) + 1))) + 1), c_int64_t)
            end do
        else if (((geom == 1) .OR. (geom == 2))) then
            do x_w0_90 = 0, (np_particles) - 1
                fx_o((x_w0_90) + 1) = INT((aint(x_old((x_w0_90) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_old((x_w0_90) + 1)) < aint(x_old((x_w0_90) + 1)))), c_int64_t)
            end do
            do x_w0_91 = 0, (np_particles) - 1
                fz_o((x_w0_91) + 1) = INT((aint(z_old((x_w0_91) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_old((x_w0_91) + 1)) < aint(z_old((x_w0_91) + 1)))), c_int64_t)
            end do
            do x_w0_92 = 0, (np_particles) - 1
                fx_n((x_w0_92) + 1) = INT((aint(x_new((x_w0_92) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_new((x_w0_92) + 1)) < aint(x_new((x_w0_92) + 1)))), c_int64_t)
            end do
            do x_w0_93 = 0, (np_particles) - 1
                fz_n((x_w0_93) + 1) = INT((aint(z_new((x_w0_93) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_new((x_w0_93) + 1)) < aint(z_new((x_w0_93) + 1)))), c_int64_t)
            end do
            do x_w0_94 = 0, (np_particles) - 1
                reduce_shape_old((x_w0_94) + 1) = INT(reduced_particle_shape_mask((0) + 1, ((loy + fz_o((x_w0_94) + &
                &1))) + 1, ((lox + fx_o((x_w0_94) + 1))) + 1), c_int64_t)
            end do
            do x_w0_95 = 0, (np_particles) - 1
                reduce_shape_new((x_w0_95) + 1) = INT(reduced_particle_shape_mask((0) + 1, ((loy + fz_n((x_w0_95) + &
                &1))) + 1, ((lox + fx_n((x_w0_95) + 1))) + 1), c_int64_t)
            end do
        else if (((geom == 4) .OR. (geom == 5))) then
            do x_w0_96 = 0, (np_particles) - 1
                fx_o((x_w0_96) + 1) = INT((aint(x_old((x_w0_96) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_old((x_w0_96) + 1)) < aint(x_old((x_w0_96) + 1)))), c_int64_t)
            end do
            do x_w0_97 = 0, (np_particles) - 1
                fx_n((x_w0_97) + 1) = INT((aint(x_new((x_w0_97) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_new((x_w0_97) + 1)) < aint(x_new((x_w0_97) + 1)))), c_int64_t)
            end do
            do x_w0_98 = 0, (np_particles) - 1
                reduce_shape_old((x_w0_98) + 1) = INT(reduced_particle_shape_mask((0) + 1, (0) + 1, ((lox + &
                &fx_o((x_w0_98) + 1))) + 1), c_int64_t)
            end do
            do x_w0_99 = 0, (np_particles) - 1
                reduce_shape_new((x_w0_99) + 1) = INT(reduced_particle_shape_mask((0) + 1, (0) + 1, ((lox + &
                &fx_n((x_w0_99) + 1))) + 1), c_int64_t)
            end do
        else
            do x_w0_100 = 0, (np_particles) - 1
                fz_o((x_w0_100) + 1) = INT((aint(z_old((x_w0_100) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_old((x_w0_100) + 1)) < aint(z_old((x_w0_100) + 1)))), c_int64_t)
            end do
            do x_w0_101 = 0, (np_particles) - 1
                fz_n((x_w0_101) + 1) = INT((aint(z_new((x_w0_101) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_new((x_w0_101) + 1)) < aint(z_new((x_w0_101) + 1)))), c_int64_t)
            end do
            do x_w0_102 = 0, (np_particles) - 1
                reduce_shape_old((x_w0_102) + 1) = INT(reduced_particle_shape_mask((0) + 1, (0) + 1, ((lox + &
                &fz_o((x_w0_102) + 1))) + 1), c_int64_t)
            end do
            do x_w0_103 = 0, (np_particles) - 1
                reduce_shape_new((x_w0_103) + 1) = INT(reduced_particle_shape_mask((0) + 1, (0) + 1, ((lox + &
                &fz_n((x_w0_103) + 1))) + 1), c_int64_t)
            end do
        end if
    end if
    if ((geom == 2)) then
        do x_w0_104 = 0, (np_particles) - 1
            vy((x_w0_104) + 1) = ((((-(uxp((x_w0_104) + 1))) * sintheta_mid((x_w0_104) + 1)) + (uyp((x_w0_104) + 1) * &
            &costheta_mid((x_w0_104) + 1))) * gaminv((x_w0_104) + 1))
        end do
    else if ((geom == 1)) then
        do x_w0_105 = 0, (np_particles) - 1
            vy((x_w0_105) + 1) = (uyp((x_w0_105) + 1) * gaminv((x_w0_105) + 1))
        end do
    else if ((geom == 0)) then
        do x_w0_106 = 0, (np_particles) - 1
            vx((x_w0_106) + 1) = (uxp((x_w0_106) + 1) * gaminv((x_w0_106) + 1))
        end do
        do x_w0_107 = 0, (np_particles) - 1
            vy((x_w0_107) + 1) = (uyp((x_w0_107) + 1) * gaminv((x_w0_107) + 1))
        end do
    else if ((geom == 4)) then
        do x_w0_108 = 0, (np_particles) - 1
            vy((x_w0_108) + 1) = ((((-(uxp((x_w0_108) + 1))) * sintheta_mid((x_w0_108) + 1)) + (uyp((x_w0_108) + 1) * &
            &costheta_mid((x_w0_108) + 1))) * gaminv((x_w0_108) + 1))
        end do
        do x_w0_109 = 0, (np_particles) - 1
            vz((x_w0_109) + 1) = (uzp((x_w0_109) + 1) * gaminv((x_w0_109) + 1))
        end do
    else if ((geom == 5)) then
        do x_w0_110 = 0, (np_particles) - 1
            vy((x_w0_110) + 1) = ((((-(uxp((x_w0_110) + 1))) * sintheta_mid((x_w0_110) + 1)) + (uyp((x_w0_110) + 1) * &
            &costheta_mid((x_w0_110) + 1))) * gaminv((x_w0_110) + 1))
        end do
        do x_w0_111 = 0, (np_particles) - 1
            vz((x_w0_111) + 1) = ((((((-(uxp((x_w0_111) + 1))) * costheta_mid((x_w0_111) + 1)) * sinphi_mid((x_w0_111) &
            &+ 1)) - ((uyp((x_w0_111) + 1) * sintheta_mid((x_w0_111) + 1)) * sinphi_mid((x_w0_111) + 1))) + &
            &(uzp((x_w0_111) + 1) * cosphi_mid((x_w0_111) + 1))) * gaminv((x_w0_111) + 1))
        end do
    end if
    half = npb_floordiv_i(INT(o, c_int64_t), INT(2, c_int64_t))
    width = (o + 3)
    i_new = 0
    do x_w0_112 = 0, (np_particles) - 1
        i_old((x_w0_112) + 1) = i_new((x_w0_112) + 1)
    end do
    do x_w0_113 = 0, (np_particles) - 1
        j_new((x_w0_113) + 1) = i_new((x_w0_113) + 1)
    end do
    do x_w0_114 = 0, (np_particles) - 1
        j_old((x_w0_114) + 1) = i_new((x_w0_114) + 1)
    end do
    do x_w0_115 = 0, (np_particles) - 1
        k_new((x_w0_115) + 1) = i_new((x_w0_115) + 1)
    end do
    do x_w0_116 = 0, (np_particles) - 1
        k_old((x_w0_116) + 1) = i_new((x_w0_116) + 1)
    end do
    if ((geom /= 0)) then
        if (.not. allocated(x_inl11_sx)) then
            allocate(x_inl11_sx(width, np_particles))
        else if (size(x_inl11_sx, 1) /= (width) .or. size(x_inl11_sx, 2) /= (np_particles)) then
            deallocate(x_inl11_sx)
            allocate(x_inl11_sx(width, np_particles))
        end if
        x_inl11_sx = 0
        if ((o == 0)) then
            do x_w0_117 = 0, (np_particles) - 1
                x_inl11_j((x_w0_117) + 1) = INT(AINT((x_new((x_w0_117) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do si0_l118 = 0, (np_particles) - 1
                x_inl11_sx((1) + 1, (si0_l118) + 1) = 1.0_c_float
            end do
            do x_w0_119 = 0, (np_particles) - 1
                x_inl11_idx((x_w0_119) + 1) = x_inl11_j((x_w0_119) + 1)
            end do
        else if ((o == 1)) then
            do x_w0_120 = 0, (np_particles) - 1
                x_inl11_j((x_w0_120) + 1) = INT(AINT(x_new((x_w0_120) + 1)), c_int64_t)
            end do
            do x_w0_121 = 0, (np_particles) - 1
                x_inl11_xint((x_w0_121) + 1) = (x_new((x_w0_121) + 1) - x_inl11_j((x_w0_121) + 1))
            end do
            do si0_l122 = 0, (np_particles) - 1
                x_inl11_sx((1) + 1, (si0_l122) + 1) = (1.0_c_float - x_inl11_xint((si0_l122) + 1))
            end do
            do si0_l123 = 0, (np_particles) - 1
                x_inl11_sx((2) + 1, (si0_l123) + 1) = x_inl11_xint((si0_l123) + 1)
            end do
            do x_w0_124 = 0, (np_particles) - 1
                x_inl11_idx((x_w0_124) + 1) = x_inl11_j((x_w0_124) + 1)
            end do
        else if ((o == 2)) then
            do x_w0_125 = 0, (np_particles) - 1
                x_inl11_j((x_w0_125) + 1) = INT(AINT((x_new((x_w0_125) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_126 = 0, (np_particles) - 1
                x_inl11_xint((x_w0_126) + 1) = (x_new((x_w0_126) + 1) - x_inl11_j((x_w0_126) + 1))
            end do
            do si0_l127 = 0, (np_particles) - 1
                x_inl11_sx((1) + 1, (si0_l127) + 1) = ((0.5_c_float * (0.5_c_float - x_inl11_xint((si0_l127) + 1))) * &
                &(0.5_c_float - x_inl11_xint((si0_l127) + 1)))
            end do
            do si0_l128 = 0, (np_particles) - 1
                x_inl11_sx((2) + 1, (si0_l128) + 1) = (0.75_c_float - (x_inl11_xint((si0_l128) + 1) * &
                &x_inl11_xint((si0_l128) + 1)))
            end do
            do si0_l129 = 0, (np_particles) - 1
                x_inl11_sx((3) + 1, (si0_l129) + 1) = ((0.5_c_float * (0.5_c_float + x_inl11_xint((si0_l129) + 1))) * &
                &(0.5_c_float + x_inl11_xint((si0_l129) + 1)))
            end do
            do x_w0_130 = 0, (np_particles) - 1
                x_inl11_idx((x_w0_130) + 1) = (x_inl11_j((x_w0_130) + 1) - 1)
            end do
        else if ((o == 3)) then
            do x_w0_131 = 0, (np_particles) - 1
                x_inl11_j((x_w0_131) + 1) = INT(AINT(x_new((x_w0_131) + 1)), c_int64_t)
            end do
            do x_w0_132 = 0, (np_particles) - 1
                x_inl11_xint((x_w0_132) + 1) = (x_new((x_w0_132) + 1) - x_inl11_j((x_w0_132) + 1))
            end do
            do si0_l133 = 0, (np_particles) - 1
                x_inl11_sx((1) + 1, (si0_l133) + 1) = (((0.16666666666666666_c_float * (1.0_c_float - &
                &x_inl11_xint((si0_l133) + 1))) * (1.0_c_float - x_inl11_xint((si0_l133) + 1))) * (1.0_c_float - &
                &x_inl11_xint((si0_l133) + 1)))
            end do
            do si0_l134 = 0, (np_particles) - 1
                x_inl11_sx((2) + 1, (si0_l134) + 1) = ((2.0_c_float / 3.0_c_float) - ((x_inl11_xint((si0_l134) + 1) * &
                &x_inl11_xint((si0_l134) + 1)) * (1.0_c_float - (x_inl11_xint((si0_l134) + 1) / 2.0_c_float))))
            end do
            do si0_l135 = 0, (np_particles) - 1
                x_inl11_sx((3) + 1, (si0_l135) + 1) = ((2.0_c_float / 3.0_c_float) - (((1.0_c_float - &
                &x_inl11_xint((si0_l135) + 1)) * (1.0_c_float - x_inl11_xint((si0_l135) + 1))) * (1.0_c_float - &
                &(0.5_c_float * (1.0_c_float - x_inl11_xint((si0_l135) + 1))))))
            end do
            do si0_l136 = 0, (np_particles) - 1
                x_inl11_sx((4) + 1, (si0_l136) + 1) = (((0.16666666666666666_c_float * x_inl11_xint((si0_l136) + 1)) * &
                &x_inl11_xint((si0_l136) + 1)) * x_inl11_xint((si0_l136) + 1))
            end do
            do x_w0_137 = 0, (np_particles) - 1
                x_inl11_idx((x_w0_137) + 1) = (x_inl11_j((x_w0_137) + 1) - 1)
            end do
        else
            do x_w0_138 = 0, (np_particles) - 1
                x_inl11_j((x_w0_138) + 1) = INT(AINT((x_new((x_w0_138) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_139 = 0, (np_particles) - 1
                x_inl11_xint((x_w0_139) + 1) = (x_new((x_w0_139) + 1) - x_inl11_j((x_w0_139) + 1))
            end do
            do x_w0_140 = 0, (np_particles) - 1
                x_inl11_sm((x_w0_140) + 1) = (0.5_c_float - x_inl11_xint((x_w0_140) + 1))
            end do
            do x_w0_141 = 0, (np_particles) - 1
                x_inl11_sp((x_w0_141) + 1) = (0.5_c_float + x_inl11_xint((x_w0_141) + 1))
            end do
            do si0_l142 = 0, (np_particles) - 1
                x_inl11_sx((1) + 1, (si0_l142) + 1) = (((((1.0_c_float / 24.0_c_float) * x_inl11_sm((si0_l142) + 1)) * &
                &x_inl11_sm((si0_l142) + 1)) * x_inl11_sm((si0_l142) + 1)) * x_inl11_sm((si0_l142) + 1))
            end do
            do si0_l143 = 0, (np_particles) - 1
                x_inl11_sx((2) + 1, (si0_l143) + 1) = ((1.0_c_float / 24.0_c_float) * ((4.75_c_float - (11.0_c_float * &
                &x_inl11_xint((si0_l143) + 1))) + (((4.0_c_float * x_inl11_xint((si0_l143) + 1)) * &
                &x_inl11_xint((si0_l143) + 1)) * ((1.5_c_float + x_inl11_xint((si0_l143) + 1)) - &
                &(x_inl11_xint((si0_l143) + 1) * x_inl11_xint((si0_l143) + 1))))))
            end do
            do si0_l144 = 0, (np_particles) - 1
                x_inl11_sx((3) + 1, (si0_l144) + 1) = ((1.0_c_float / 24.0_c_float) * (14.375_c_float + (((6.0_c_float &
                &* x_inl11_xint((si0_l144) + 1)) * x_inl11_xint((si0_l144) + 1)) * ((x_inl11_xint((si0_l144) + 1) * &
                &x_inl11_xint((si0_l144) + 1)) - 2.5_c_float))))
            end do
            do si0_l145 = 0, (np_particles) - 1
                x_inl11_sx((4) + 1, (si0_l145) + 1) = ((1.0_c_float / 24.0_c_float) * ((4.75_c_float + (11.0_c_float * &
                &x_inl11_xint((si0_l145) + 1))) + (((4.0_c_float * x_inl11_xint((si0_l145) + 1)) * &
                &x_inl11_xint((si0_l145) + 1)) * ((1.5_c_float - x_inl11_xint((si0_l145) + 1)) - &
                &(x_inl11_xint((si0_l145) + 1) * x_inl11_xint((si0_l145) + 1))))))
            end do
            do si0_l146 = 0, (np_particles) - 1
                x_inl11_sx((5) + 1, (si0_l146) + 1) = (((((1.0_c_float / 24.0_c_float) * x_inl11_sp((si0_l146) + 1)) * &
                &x_inl11_sp((si0_l146) + 1)) * x_inl11_sp((si0_l146) + 1)) * x_inl11_sp((si0_l146) + 1))
            end do
            do x_w0_147 = 0, (np_particles) - 1
                x_inl11_idx((x_w0_147) + 1) = (x_inl11_j((x_w0_147) + 1) - 2)
            end do
        end if
        if (.not. allocated(sx_new)) then
            allocate(sx_new(width, np_particles))
        else if (size(sx_new, 1) /= (width) .or. size(sx_new, 2) /= (np_particles)) then
            deallocate(sx_new)
            allocate(sx_new(width, np_particles))
        end if
        do x_w0_148 = 0, (np_particles) - 1
            do x_w1_149 = 0, (width) - 1
                sx_new((x_w1_149) + 1, (x_w0_148) + 1) = x_inl11_sx((x_w1_149) + 1, (x_w0_148) + 1)
            end do
        end do
        do x_w0_150 = 0, (np_particles) - 1
            i_new((x_w0_150) + 1) = x_inl11_idx((x_w0_150) + 1)
        end do
        if (.not. allocated(x_inl12_sx)) then
            allocate(x_inl12_sx(width, np_particles))
        else if (size(x_inl12_sx, 1) /= (width) .or. size(x_inl12_sx, 2) /= (np_particles)) then
            deallocate(x_inl12_sx)
            allocate(x_inl12_sx(width, np_particles))
        end if
        x_inl12_sx = 0
        ! numpy: np.arange(np_particles)
        do x_i_151 = 0, (np_particles) - 1
            x_cb22((x_i_151) + 1) = x_i_151
        end do
        do x_w0_152 = 0, (np_particles) - 1
            x_inl12_rows((x_w0_152) + 1) = x_cb22((x_w0_152) + 1)
        end do
        if ((o == 0)) then
            do x_w0_153 = 0, (np_particles) - 1
                x_inl12_i((x_w0_153) + 1) = INT((aint((x_old((x_w0_153) + 1) + 0.5_c_float)) - merge(1.0_c_float, &
                &0.0_c_float, ((x_old((x_w0_153) + 1) + 0.5_c_float)) < aint((x_old((x_w0_153) + 1) + 0.5_c_float)))), &
                &c_int64_t)
            end do
            do x_w0_154 = 0, (np_particles) - 1
                x_inl12_i_shift((x_w0_154) + 1) = (x_inl12_i((x_w0_154) + 1) - i_new((x_w0_154) + 1))
            end do
            do x_sc0_155 = 0, (np_particles) - 1
                x_inl12_sx(((1 + x_inl12_i_shift((x_sc0_155) + 1))) + 1, (x_inl12_rows((x_sc0_155) + 1)) + 1) = &
                &1.0_c_float
            end do
            do x_w0_156 = 0, (np_particles) - 1
                x_inl12_idx((x_w0_156) + 1) = x_inl12_i((x_w0_156) + 1)
            end do
        else if ((o == 1)) then
            do x_w0_157 = 0, (np_particles) - 1
                x_inl12_i((x_w0_157) + 1) = INT((aint(x_old((x_w0_157) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_old((x_w0_157) + 1)) < aint(x_old((x_w0_157) + 1)))), c_int64_t)
            end do
            do x_w0_158 = 0, (np_particles) - 1
                x_inl12_i_shift((x_w0_158) + 1) = (x_inl12_i((x_w0_158) + 1) - i_new((x_w0_158) + 1))
            end do
            do x_w0_159 = 0, (np_particles) - 1
                x_inl12_xint((x_w0_159) + 1) = (x_old((x_w0_159) + 1) - x_inl12_i((x_w0_159) + 1))
            end do
            do x_sc0_160 = 0, (np_particles) - 1
                x_inl12_sx(((1 + x_inl12_i_shift((x_sc0_160) + 1))) + 1, (x_inl12_rows((x_sc0_160) + 1)) + 1) = &
                &(1.0_c_float - x_inl12_xint((x_sc0_160) + 1))
            end do
            do x_sc0_161 = 0, (np_particles) - 1
                x_inl12_sx(((2 + x_inl12_i_shift((x_sc0_161) + 1))) + 1, (x_inl12_rows((x_sc0_161) + 1)) + 1) = &
                &x_inl12_xint((x_sc0_161) + 1)
            end do
            do x_w0_162 = 0, (np_particles) - 1
                x_inl12_idx((x_w0_162) + 1) = x_inl12_i((x_w0_162) + 1)
            end do
        else if ((o == 2)) then
            do x_w0_163 = 0, (np_particles) - 1
                x_inl12_i((x_w0_163) + 1) = INT(AINT((x_old((x_w0_163) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_164 = 0, (np_particles) - 1
                x_inl12_i_shift((x_w0_164) + 1) = (x_inl12_i((x_w0_164) + 1) - (i_new((x_w0_164) + 1) + 1))
            end do
            do x_w0_165 = 0, (np_particles) - 1
                x_inl12_xint((x_w0_165) + 1) = (x_old((x_w0_165) + 1) - x_inl12_i((x_w0_165) + 1))
            end do
            do x_sc0_166 = 0, (np_particles) - 1
                x_inl12_sx(((1 + x_inl12_i_shift((x_sc0_166) + 1))) + 1, (x_inl12_rows((x_sc0_166) + 1)) + 1) = &
                &((0.5_c_float * (0.5_c_float - x_inl12_xint((x_sc0_166) + 1))) * (0.5_c_float - &
                &x_inl12_xint((x_sc0_166) + 1)))
            end do
            do x_sc0_167 = 0, (np_particles) - 1
                x_inl12_sx(((2 + x_inl12_i_shift((x_sc0_167) + 1))) + 1, (x_inl12_rows((x_sc0_167) + 1)) + 1) = &
                &(0.75_c_float - (x_inl12_xint((x_sc0_167) + 1) * x_inl12_xint((x_sc0_167) + 1)))
            end do
            do x_sc0_168 = 0, (np_particles) - 1
                x_inl12_sx(((3 + x_inl12_i_shift((x_sc0_168) + 1))) + 1, (x_inl12_rows((x_sc0_168) + 1)) + 1) = &
                &((0.5_c_float * (0.5_c_float + x_inl12_xint((x_sc0_168) + 1))) * (0.5_c_float + &
                &x_inl12_xint((x_sc0_168) + 1)))
            end do
            do x_w0_169 = 0, (np_particles) - 1
                x_inl12_idx((x_w0_169) + 1) = (x_inl12_i((x_w0_169) + 1) - 1)
            end do
        else if ((o == 3)) then
            do x_w0_170 = 0, (np_particles) - 1
                x_inl12_i((x_w0_170) + 1) = INT(AINT(x_old((x_w0_170) + 1)), c_int64_t)
            end do
            do x_w0_171 = 0, (np_particles) - 1
                x_inl12_i_shift((x_w0_171) + 1) = (x_inl12_i((x_w0_171) + 1) - (i_new((x_w0_171) + 1) + 1))
            end do
            do x_w0_172 = 0, (np_particles) - 1
                x_inl12_xint((x_w0_172) + 1) = (x_old((x_w0_172) + 1) - x_inl12_i((x_w0_172) + 1))
            end do
            do x_sc0_173 = 0, (np_particles) - 1
                x_inl12_sx(((1 + x_inl12_i_shift((x_sc0_173) + 1))) + 1, (x_inl12_rows((x_sc0_173) + 1)) + 1) = &
                &(((0.16666666666666666_c_float * (1.0_c_float - x_inl12_xint((x_sc0_173) + 1))) * (1.0_c_float - &
                &x_inl12_xint((x_sc0_173) + 1))) * (1.0_c_float - x_inl12_xint((x_sc0_173) + 1)))
            end do
            do x_sc0_174 = 0, (np_particles) - 1
                x_inl12_sx(((2 + x_inl12_i_shift((x_sc0_174) + 1))) + 1, (x_inl12_rows((x_sc0_174) + 1)) + 1) = &
                &((2.0_c_float / 3.0_c_float) - ((x_inl12_xint((x_sc0_174) + 1) * x_inl12_xint((x_sc0_174) + 1)) * &
                &(1.0_c_float - (x_inl12_xint((x_sc0_174) + 1) / 2.0_c_float))))
            end do
            do x_sc0_175 = 0, (np_particles) - 1
                x_inl12_sx(((3 + x_inl12_i_shift((x_sc0_175) + 1))) + 1, (x_inl12_rows((x_sc0_175) + 1)) + 1) = &
                &((2.0_c_float / 3.0_c_float) - (((1.0_c_float - x_inl12_xint((x_sc0_175) + 1)) * (1.0_c_float - &
                &x_inl12_xint((x_sc0_175) + 1))) * (1.0_c_float - (0.5_c_float * (1.0_c_float - &
                &x_inl12_xint((x_sc0_175) + 1))))))
            end do
            do x_sc0_176 = 0, (np_particles) - 1
                x_inl12_sx(((4 + x_inl12_i_shift((x_sc0_176) + 1))) + 1, (x_inl12_rows((x_sc0_176) + 1)) + 1) = &
                &(((0.16666666666666666_c_float * x_inl12_xint((x_sc0_176) + 1)) * x_inl12_xint((x_sc0_176) + 1)) * &
                &x_inl12_xint((x_sc0_176) + 1))
            end do
            do x_w0_177 = 0, (np_particles) - 1
                x_inl12_idx((x_w0_177) + 1) = (x_inl12_i((x_w0_177) + 1) - 1)
            end do
        else
            do x_w0_178 = 0, (np_particles) - 1
                x_inl12_i((x_w0_178) + 1) = INT(AINT((x_old((x_w0_178) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_179 = 0, (np_particles) - 1
                x_inl12_i_shift((x_w0_179) + 1) = (x_inl12_i((x_w0_179) + 1) - (i_new((x_w0_179) + 1) + 2))
            end do
            do x_w0_180 = 0, (np_particles) - 1
                x_inl12_xint((x_w0_180) + 1) = (x_old((x_w0_180) + 1) - x_inl12_i((x_w0_180) + 1))
            end do
            do x_w0_181 = 0, (np_particles) - 1
                x_inl12_sm((x_w0_181) + 1) = (0.5_c_float - x_inl12_xint((x_w0_181) + 1))
            end do
            do x_w0_182 = 0, (np_particles) - 1
                x_inl12_sp((x_w0_182) + 1) = (0.5_c_float + x_inl12_xint((x_w0_182) + 1))
            end do
            do x_sc0_183 = 0, (np_particles) - 1
                x_inl12_sx(((1 + x_inl12_i_shift((x_sc0_183) + 1))) + 1, (x_inl12_rows((x_sc0_183) + 1)) + 1) = &
                &(((((1.0_c_float / 24.0_c_float) * x_inl12_sm((x_sc0_183) + 1)) * x_inl12_sm((x_sc0_183) + 1)) * &
                &x_inl12_sm((x_sc0_183) + 1)) * x_inl12_sm((x_sc0_183) + 1))
            end do
            do x_sc0_184 = 0, (np_particles) - 1
                x_inl12_sx(((2 + x_inl12_i_shift((x_sc0_184) + 1))) + 1, (x_inl12_rows((x_sc0_184) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * ((4.75_c_float - (11.0_c_float * x_inl12_xint((x_sc0_184) + 1))) + &
                &(((4.0_c_float * x_inl12_xint((x_sc0_184) + 1)) * x_inl12_xint((x_sc0_184) + 1)) * ((1.5_c_float + &
                &x_inl12_xint((x_sc0_184) + 1)) - (x_inl12_xint((x_sc0_184) + 1) * x_inl12_xint((x_sc0_184) + 1))))))
            end do
            do x_sc0_185 = 0, (np_particles) - 1
                x_inl12_sx(((3 + x_inl12_i_shift((x_sc0_185) + 1))) + 1, (x_inl12_rows((x_sc0_185) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * (14.375_c_float + (((6.0_c_float * x_inl12_xint((x_sc0_185) + 1)) * &
                &x_inl12_xint((x_sc0_185) + 1)) * ((x_inl12_xint((x_sc0_185) + 1) * x_inl12_xint((x_sc0_185) + 1)) - &
                &2.5_c_float))))
            end do
            do x_sc0_186 = 0, (np_particles) - 1
                x_inl12_sx(((4 + x_inl12_i_shift((x_sc0_186) + 1))) + 1, (x_inl12_rows((x_sc0_186) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * ((4.75_c_float + (11.0_c_float * x_inl12_xint((x_sc0_186) + 1))) + &
                &(((4.0_c_float * x_inl12_xint((x_sc0_186) + 1)) * x_inl12_xint((x_sc0_186) + 1)) * ((1.5_c_float - &
                &x_inl12_xint((x_sc0_186) + 1)) - (x_inl12_xint((x_sc0_186) + 1) * x_inl12_xint((x_sc0_186) + 1))))))
            end do
            do x_sc0_187 = 0, (np_particles) - 1
                x_inl12_sx(((5 + x_inl12_i_shift((x_sc0_187) + 1))) + 1, (x_inl12_rows((x_sc0_187) + 1)) + 1) = &
                &(((((1.0_c_float / 24.0_c_float) * x_inl12_sp((x_sc0_187) + 1)) * x_inl12_sp((x_sc0_187) + 1)) * &
                &x_inl12_sp((x_sc0_187) + 1)) * x_inl12_sp((x_sc0_187) + 1))
            end do
            do x_w0_188 = 0, (np_particles) - 1
                x_inl12_idx((x_w0_188) + 1) = (x_inl12_i((x_w0_188) + 1) - 2)
            end do
        end if
        if (.not. allocated(sx_old)) then
            allocate(sx_old(width, np_particles))
        else if (size(sx_old, 1) /= (width) .or. size(sx_old, 2) /= (np_particles)) then
            deallocate(sx_old)
            allocate(sx_old(width, np_particles))
        end if
        do x_w0_189 = 0, (np_particles) - 1
            do x_w1_190 = 0, (width) - 1
                sx_old((x_w1_190) + 1, (x_w0_189) + 1) = x_inl12_sx((x_w1_190) + 1, (x_w0_189) + 1)
            end do
        end do
        do x_w0_191 = 0, (np_particles) - 1
            i_old((x_w0_191) + 1) = x_inl12_idx((x_w0_191) + 1)
        end do
        if (reduce_enabled) then
            if (.not. allocated(x_inl13_sx)) then
                allocate(x_inl13_sx(width, np_particles))
            else if (size(x_inl13_sx, 1) /= (width) .or. size(x_inl13_sx, 2) /= (np_particles)) then
                deallocate(x_inl13_sx)
                allocate(x_inl13_sx(width, np_particles))
            end if
            x_inl13_sx = 0
            ! numpy: np.arange(np_particles)
            do x_i_192 = 0, (np_particles) - 1
                x_cb23((x_i_192) + 1) = x_i_192
            end do
            do x_w0_193 = 0, (np_particles) - 1
                x_inl13_rows((x_w0_193) + 1) = x_cb23((x_w0_193) + 1)
            end do
            do x_w0_194 = 0, (np_particles) - 1
                x_inl13_i((x_w0_194) + 1) = INT((aint(x_new((x_w0_194) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_new((x_w0_194) + 1)) < aint(x_new((x_w0_194) + 1)))), c_int64_t)
            end do
            do x_w0_195 = 0, (np_particles) - 1
                x_inl13_i_shift((x_w0_195) + 1) = (x_inl13_i((x_w0_195) + 1) - (i_new((x_w0_195) + 1) + half))
            end do
            do x_w0_196 = 0, (np_particles) - 1
                x_inl13_xint((x_w0_196) + 1) = (x_new((x_w0_196) + 1) - x_inl13_i((x_w0_196) + 1))
            end do
            do x_sc0_197 = 0, (np_particles) - 1
                x_inl13_sx((((half + 1) + x_inl13_i_shift((x_sc0_197) + 1))) + 1, (x_inl13_rows((x_sc0_197) + 1)) + 1) &
                &= (1.0_c_float - x_inl13_xint((x_sc0_197) + 1))
            end do
            do x_sc0_198 = 0, (np_particles) - 1
                x_inl13_sx((((half + 2) + x_inl13_i_shift((x_sc0_198) + 1))) + 1, (x_inl13_rows((x_sc0_198) + 1)) + 1) &
                &= x_inl13_xint((x_sc0_198) + 1)
            end do
            do x_w0_199 = 0, (np_particles) - 1
                x_inl13_idx((x_w0_199) + 1) = x_inl13_i((x_w0_199) + 1)
            end do
            if (.not. allocated(ov_new)) then
                allocate(ov_new(width, np_particles))
            else if (size(ov_new, 1) /= (width) .or. size(ov_new, 2) /= (np_particles)) then
                deallocate(ov_new)
                allocate(ov_new(width, np_particles))
            end if
            do x_w0_200 = 0, (np_particles) - 1
                do x_w1_201 = 0, (width) - 1
                    ov_new((x_w1_201) + 1, (x_w0_200) + 1) = x_inl13_sx((x_w1_201) + 1, (x_w0_200) + 1)
                end do
            end do
            if (.not. allocated(x_inl14_sx)) then
                allocate(x_inl14_sx(width, np_particles))
            else if (size(x_inl14_sx, 1) /= (width) .or. size(x_inl14_sx, 2) /= (np_particles)) then
                deallocate(x_inl14_sx)
                allocate(x_inl14_sx(width, np_particles))
            end if
            x_inl14_sx = 0
            ! numpy: np.arange(np_particles)
            do x_i_202 = 0, (np_particles) - 1
                x_cb24((x_i_202) + 1) = x_i_202
            end do
            do x_w0_203 = 0, (np_particles) - 1
                x_inl14_rows((x_w0_203) + 1) = x_cb24((x_w0_203) + 1)
            end do
            do x_w0_204 = 0, (np_particles) - 1
                x_inl14_i((x_w0_204) + 1) = INT((aint(x_old((x_w0_204) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(x_old((x_w0_204) + 1)) < aint(x_old((x_w0_204) + 1)))), c_int64_t)
            end do
            do x_w0_205 = 0, (np_particles) - 1
                x_inl14_i_shift((x_w0_205) + 1) = (x_inl14_i((x_w0_205) + 1) - (i_new((x_w0_205) + 1) + half))
            end do
            do x_w0_206 = 0, (np_particles) - 1
                x_inl14_xint((x_w0_206) + 1) = (x_old((x_w0_206) + 1) - x_inl14_i((x_w0_206) + 1))
            end do
            do x_sc0_207 = 0, (np_particles) - 1
                x_inl14_sx((((half + 1) + x_inl14_i_shift((x_sc0_207) + 1))) + 1, (x_inl14_rows((x_sc0_207) + 1)) + 1) &
                &= (1.0_c_float - x_inl14_xint((x_sc0_207) + 1))
            end do
            do x_sc0_208 = 0, (np_particles) - 1
                x_inl14_sx((((half + 2) + x_inl14_i_shift((x_sc0_208) + 1))) + 1, (x_inl14_rows((x_sc0_208) + 1)) + 1) &
                &= x_inl14_xint((x_sc0_208) + 1)
            end do
            do x_w0_209 = 0, (np_particles) - 1
                x_inl14_idx((x_w0_209) + 1) = x_inl14_i((x_w0_209) + 1)
            end do
            if (.not. allocated(ov_old)) then
                allocate(ov_old(width, np_particles))
            else if (size(ov_old, 1) /= (width) .or. size(ov_old, 2) /= (np_particles)) then
                deallocate(ov_old)
                allocate(ov_old(width, np_particles))
            end if
            do x_w0_210 = 0, (np_particles) - 1
                do x_w1_211 = 0, (width) - 1
                    ov_old((x_w1_211) + 1, (x_w0_210) + 1) = x_inl14_sx((x_w1_211) + 1, (x_w0_210) + 1)
                end do
            end do
            if (.not. allocated(x_cb25)) then
                allocate(x_cb25(width, np_particles))
            else if (size(x_cb25, 1) /= (width) .or. size(x_cb25, 2) /= (np_particles)) then
                deallocate(x_cb25)
                allocate(x_cb25(width, np_particles))
            end if
            ! numpy: np.where((reduce_shape_new != 0)[:, None], ov_new, sx_new)
            do x_r0_212 = 0, (np_particles) - 1
                do x_r1_213 = 0, (width) - 1
                    if ((reduce_shape_new((x_r0_212) + 1) /= 0)) then
                        x_ifexp20 = ov_new((x_r1_213) + 1, (x_r0_212) + 1)
                    else
                        x_ifexp20 = sx_new((x_r1_213) + 1, (x_r0_212) + 1)
                    end if
                    x_cb25((x_r1_213) + 1, (x_r0_212) + 1) = x_ifexp20
                end do
            end do
            if (.not. allocated(sx_new)) then
                allocate(sx_new(width, np_particles))
            else if (size(sx_new, 1) /= (width) .or. size(sx_new, 2) /= (np_particles)) then
                deallocate(sx_new)
                allocate(sx_new(width, np_particles))
            end if
            do x_w0_214 = 0, (np_particles) - 1
                do x_w1_215 = 0, (width) - 1
                    sx_new((x_w1_215) + 1, (x_w0_214) + 1) = x_cb25((x_w1_215) + 1, (x_w0_214) + 1)
                end do
            end do
            if (.not. allocated(x_cb26)) then
                allocate(x_cb26(width, np_particles))
            else if (size(x_cb26, 1) /= (width) .or. size(x_cb26, 2) /= (np_particles)) then
                deallocate(x_cb26)
                allocate(x_cb26(width, np_particles))
            end if
            ! numpy: np.where((reduce_shape_old != 0)[:, None], ov_old, sx_old)
            do x_r0_216 = 0, (np_particles) - 1
                do x_r1_217 = 0, (width) - 1
                    if ((reduce_shape_old((x_r0_216) + 1) /= 0)) then
                        x_ifexp21 = ov_old((x_r1_217) + 1, (x_r0_216) + 1)
                    else
                        x_ifexp21 = sx_old((x_r1_217) + 1, (x_r0_216) + 1)
                    end if
                    x_cb26((x_r1_217) + 1, (x_r0_216) + 1) = x_ifexp21
                end do
            end do
            if (.not. allocated(sx_old)) then
                allocate(sx_old(width, np_particles))
            else if (size(sx_old, 1) /= (width) .or. size(sx_old, 2) /= (np_particles)) then
                deallocate(sx_old)
                allocate(sx_old(width, np_particles))
            end if
            do x_w0_218 = 0, (np_particles) - 1
                do x_w1_219 = 0, (width) - 1
                    sx_old((x_w1_219) + 1, (x_w0_218) + 1) = x_cb26((x_w1_219) + 1, (x_w0_218) + 1)
                end do
            end do
        end if
    end if
    if ((geom == 3)) then
        if (.not. allocated(x_inl15_sx)) then
            allocate(x_inl15_sx(width, np_particles))
        else if (size(x_inl15_sx, 1) /= (width) .or. size(x_inl15_sx, 2) /= (np_particles)) then
            deallocate(x_inl15_sx)
            allocate(x_inl15_sx(width, np_particles))
        end if
        x_inl15_sx = 0
        if ((o == 0)) then
            do x_w0_220 = 0, (np_particles) - 1
                x_inl15_j((x_w0_220) + 1) = INT(AINT((y_new((x_w0_220) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do si0_l221 = 0, (np_particles) - 1
                x_inl15_sx((1) + 1, (si0_l221) + 1) = 1.0_c_float
            end do
            do x_w0_222 = 0, (np_particles) - 1
                x_inl15_idx((x_w0_222) + 1) = x_inl15_j((x_w0_222) + 1)
            end do
        else if ((o == 1)) then
            do x_w0_223 = 0, (np_particles) - 1
                x_inl15_j((x_w0_223) + 1) = INT(AINT(y_new((x_w0_223) + 1)), c_int64_t)
            end do
            do x_w0_224 = 0, (np_particles) - 1
                x_inl15_xint((x_w0_224) + 1) = (y_new((x_w0_224) + 1) - x_inl15_j((x_w0_224) + 1))
            end do
            do si0_l225 = 0, (np_particles) - 1
                x_inl15_sx((1) + 1, (si0_l225) + 1) = (1.0_c_float - x_inl15_xint((si0_l225) + 1))
            end do
            do si0_l226 = 0, (np_particles) - 1
                x_inl15_sx((2) + 1, (si0_l226) + 1) = x_inl15_xint((si0_l226) + 1)
            end do
            do x_w0_227 = 0, (np_particles) - 1
                x_inl15_idx((x_w0_227) + 1) = x_inl15_j((x_w0_227) + 1)
            end do
        else if ((o == 2)) then
            do x_w0_228 = 0, (np_particles) - 1
                x_inl15_j((x_w0_228) + 1) = INT(AINT((y_new((x_w0_228) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_229 = 0, (np_particles) - 1
                x_inl15_xint((x_w0_229) + 1) = (y_new((x_w0_229) + 1) - x_inl15_j((x_w0_229) + 1))
            end do
            do si0_l230 = 0, (np_particles) - 1
                x_inl15_sx((1) + 1, (si0_l230) + 1) = ((0.5_c_float * (0.5_c_float - x_inl15_xint((si0_l230) + 1))) * &
                &(0.5_c_float - x_inl15_xint((si0_l230) + 1)))
            end do
            do si0_l231 = 0, (np_particles) - 1
                x_inl15_sx((2) + 1, (si0_l231) + 1) = (0.75_c_float - (x_inl15_xint((si0_l231) + 1) * &
                &x_inl15_xint((si0_l231) + 1)))
            end do
            do si0_l232 = 0, (np_particles) - 1
                x_inl15_sx((3) + 1, (si0_l232) + 1) = ((0.5_c_float * (0.5_c_float + x_inl15_xint((si0_l232) + 1))) * &
                &(0.5_c_float + x_inl15_xint((si0_l232) + 1)))
            end do
            do x_w0_233 = 0, (np_particles) - 1
                x_inl15_idx((x_w0_233) + 1) = (x_inl15_j((x_w0_233) + 1) - 1)
            end do
        else if ((o == 3)) then
            do x_w0_234 = 0, (np_particles) - 1
                x_inl15_j((x_w0_234) + 1) = INT(AINT(y_new((x_w0_234) + 1)), c_int64_t)
            end do
            do x_w0_235 = 0, (np_particles) - 1
                x_inl15_xint((x_w0_235) + 1) = (y_new((x_w0_235) + 1) - x_inl15_j((x_w0_235) + 1))
            end do
            do si0_l236 = 0, (np_particles) - 1
                x_inl15_sx((1) + 1, (si0_l236) + 1) = (((0.16666666666666666_c_float * (1.0_c_float - &
                &x_inl15_xint((si0_l236) + 1))) * (1.0_c_float - x_inl15_xint((si0_l236) + 1))) * (1.0_c_float - &
                &x_inl15_xint((si0_l236) + 1)))
            end do
            do si0_l237 = 0, (np_particles) - 1
                x_inl15_sx((2) + 1, (si0_l237) + 1) = ((2.0_c_float / 3.0_c_float) - ((x_inl15_xint((si0_l237) + 1) * &
                &x_inl15_xint((si0_l237) + 1)) * (1.0_c_float - (x_inl15_xint((si0_l237) + 1) / 2.0_c_float))))
            end do
            do si0_l238 = 0, (np_particles) - 1
                x_inl15_sx((3) + 1, (si0_l238) + 1) = ((2.0_c_float / 3.0_c_float) - (((1.0_c_float - &
                &x_inl15_xint((si0_l238) + 1)) * (1.0_c_float - x_inl15_xint((si0_l238) + 1))) * (1.0_c_float - &
                &(0.5_c_float * (1.0_c_float - x_inl15_xint((si0_l238) + 1))))))
            end do
            do si0_l239 = 0, (np_particles) - 1
                x_inl15_sx((4) + 1, (si0_l239) + 1) = (((0.16666666666666666_c_float * x_inl15_xint((si0_l239) + 1)) * &
                &x_inl15_xint((si0_l239) + 1)) * x_inl15_xint((si0_l239) + 1))
            end do
            do x_w0_240 = 0, (np_particles) - 1
                x_inl15_idx((x_w0_240) + 1) = (x_inl15_j((x_w0_240) + 1) - 1)
            end do
        else
            do x_w0_241 = 0, (np_particles) - 1
                x_inl15_j((x_w0_241) + 1) = INT(AINT((y_new((x_w0_241) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_242 = 0, (np_particles) - 1
                x_inl15_xint((x_w0_242) + 1) = (y_new((x_w0_242) + 1) - x_inl15_j((x_w0_242) + 1))
            end do
            do x_w0_243 = 0, (np_particles) - 1
                x_inl15_sm((x_w0_243) + 1) = (0.5_c_float - x_inl15_xint((x_w0_243) + 1))
            end do
            do x_w0_244 = 0, (np_particles) - 1
                x_inl15_sp((x_w0_244) + 1) = (0.5_c_float + x_inl15_xint((x_w0_244) + 1))
            end do
            do si0_l245 = 0, (np_particles) - 1
                x_inl15_sx((1) + 1, (si0_l245) + 1) = (((((1.0_c_float / 24.0_c_float) * x_inl15_sm((si0_l245) + 1)) * &
                &x_inl15_sm((si0_l245) + 1)) * x_inl15_sm((si0_l245) + 1)) * x_inl15_sm((si0_l245) + 1))
            end do
            do si0_l246 = 0, (np_particles) - 1
                x_inl15_sx((2) + 1, (si0_l246) + 1) = ((1.0_c_float / 24.0_c_float) * ((4.75_c_float - (11.0_c_float * &
                &x_inl15_xint((si0_l246) + 1))) + (((4.0_c_float * x_inl15_xint((si0_l246) + 1)) * &
                &x_inl15_xint((si0_l246) + 1)) * ((1.5_c_float + x_inl15_xint((si0_l246) + 1)) - &
                &(x_inl15_xint((si0_l246) + 1) * x_inl15_xint((si0_l246) + 1))))))
            end do
            do si0_l247 = 0, (np_particles) - 1
                x_inl15_sx((3) + 1, (si0_l247) + 1) = ((1.0_c_float / 24.0_c_float) * (14.375_c_float + (((6.0_c_float &
                &* x_inl15_xint((si0_l247) + 1)) * x_inl15_xint((si0_l247) + 1)) * ((x_inl15_xint((si0_l247) + 1) * &
                &x_inl15_xint((si0_l247) + 1)) - 2.5_c_float))))
            end do
            do si0_l248 = 0, (np_particles) - 1
                x_inl15_sx((4) + 1, (si0_l248) + 1) = ((1.0_c_float / 24.0_c_float) * ((4.75_c_float + (11.0_c_float * &
                &x_inl15_xint((si0_l248) + 1))) + (((4.0_c_float * x_inl15_xint((si0_l248) + 1)) * &
                &x_inl15_xint((si0_l248) + 1)) * ((1.5_c_float - x_inl15_xint((si0_l248) + 1)) - &
                &(x_inl15_xint((si0_l248) + 1) * x_inl15_xint((si0_l248) + 1))))))
            end do
            do si0_l249 = 0, (np_particles) - 1
                x_inl15_sx((5) + 1, (si0_l249) + 1) = (((((1.0_c_float / 24.0_c_float) * x_inl15_sp((si0_l249) + 1)) * &
                &x_inl15_sp((si0_l249) + 1)) * x_inl15_sp((si0_l249) + 1)) * x_inl15_sp((si0_l249) + 1))
            end do
            do x_w0_250 = 0, (np_particles) - 1
                x_inl15_idx((x_w0_250) + 1) = (x_inl15_j((x_w0_250) + 1) - 2)
            end do
        end if
        if (.not. allocated(sy_new)) then
            allocate(sy_new(width, np_particles))
        else if (size(sy_new, 1) /= (width) .or. size(sy_new, 2) /= (np_particles)) then
            deallocate(sy_new)
            allocate(sy_new(width, np_particles))
        end if
        do x_w0_251 = 0, (np_particles) - 1
            do x_w1_252 = 0, (width) - 1
                sy_new((x_w1_252) + 1, (x_w0_251) + 1) = x_inl15_sx((x_w1_252) + 1, (x_w0_251) + 1)
            end do
        end do
        do x_w0_253 = 0, (np_particles) - 1
            j_new((x_w0_253) + 1) = x_inl15_idx((x_w0_253) + 1)
        end do
        if (.not. allocated(x_inl16_sx)) then
            allocate(x_inl16_sx(width, np_particles))
        else if (size(x_inl16_sx, 1) /= (width) .or. size(x_inl16_sx, 2) /= (np_particles)) then
            deallocate(x_inl16_sx)
            allocate(x_inl16_sx(width, np_particles))
        end if
        x_inl16_sx = 0
        ! numpy: np.arange(np_particles)
        do x_i_254 = 0, (np_particles) - 1
            x_cb27((x_i_254) + 1) = x_i_254
        end do
        do x_w0_255 = 0, (np_particles) - 1
            x_inl16_rows((x_w0_255) + 1) = x_cb27((x_w0_255) + 1)
        end do
        if ((o == 0)) then
            do x_w0_256 = 0, (np_particles) - 1
                x_inl16_i((x_w0_256) + 1) = INT((aint((y_old((x_w0_256) + 1) + 0.5_c_float)) - merge(1.0_c_float, &
                &0.0_c_float, ((y_old((x_w0_256) + 1) + 0.5_c_float)) < aint((y_old((x_w0_256) + 1) + 0.5_c_float)))), &
                &c_int64_t)
            end do
            do x_w0_257 = 0, (np_particles) - 1
                x_inl16_i_shift((x_w0_257) + 1) = (x_inl16_i((x_w0_257) + 1) - j_new((x_w0_257) + 1))
            end do
            do x_sc0_258 = 0, (np_particles) - 1
                x_inl16_sx(((1 + x_inl16_i_shift((x_sc0_258) + 1))) + 1, (x_inl16_rows((x_sc0_258) + 1)) + 1) = &
                &1.0_c_float
            end do
            do x_w0_259 = 0, (np_particles) - 1
                x_inl16_idx((x_w0_259) + 1) = x_inl16_i((x_w0_259) + 1)
            end do
        else if ((o == 1)) then
            do x_w0_260 = 0, (np_particles) - 1
                x_inl16_i((x_w0_260) + 1) = INT((aint(y_old((x_w0_260) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(y_old((x_w0_260) + 1)) < aint(y_old((x_w0_260) + 1)))), c_int64_t)
            end do
            do x_w0_261 = 0, (np_particles) - 1
                x_inl16_i_shift((x_w0_261) + 1) = (x_inl16_i((x_w0_261) + 1) - j_new((x_w0_261) + 1))
            end do
            do x_w0_262 = 0, (np_particles) - 1
                x_inl16_xint((x_w0_262) + 1) = (y_old((x_w0_262) + 1) - x_inl16_i((x_w0_262) + 1))
            end do
            do x_sc0_263 = 0, (np_particles) - 1
                x_inl16_sx(((1 + x_inl16_i_shift((x_sc0_263) + 1))) + 1, (x_inl16_rows((x_sc0_263) + 1)) + 1) = &
                &(1.0_c_float - x_inl16_xint((x_sc0_263) + 1))
            end do
            do x_sc0_264 = 0, (np_particles) - 1
                x_inl16_sx(((2 + x_inl16_i_shift((x_sc0_264) + 1))) + 1, (x_inl16_rows((x_sc0_264) + 1)) + 1) = &
                &x_inl16_xint((x_sc0_264) + 1)
            end do
            do x_w0_265 = 0, (np_particles) - 1
                x_inl16_idx((x_w0_265) + 1) = x_inl16_i((x_w0_265) + 1)
            end do
        else if ((o == 2)) then
            do x_w0_266 = 0, (np_particles) - 1
                x_inl16_i((x_w0_266) + 1) = INT(AINT((y_old((x_w0_266) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_267 = 0, (np_particles) - 1
                x_inl16_i_shift((x_w0_267) + 1) = (x_inl16_i((x_w0_267) + 1) - (j_new((x_w0_267) + 1) + 1))
            end do
            do x_w0_268 = 0, (np_particles) - 1
                x_inl16_xint((x_w0_268) + 1) = (y_old((x_w0_268) + 1) - x_inl16_i((x_w0_268) + 1))
            end do
            do x_sc0_269 = 0, (np_particles) - 1
                x_inl16_sx(((1 + x_inl16_i_shift((x_sc0_269) + 1))) + 1, (x_inl16_rows((x_sc0_269) + 1)) + 1) = &
                &((0.5_c_float * (0.5_c_float - x_inl16_xint((x_sc0_269) + 1))) * (0.5_c_float - &
                &x_inl16_xint((x_sc0_269) + 1)))
            end do
            do x_sc0_270 = 0, (np_particles) - 1
                x_inl16_sx(((2 + x_inl16_i_shift((x_sc0_270) + 1))) + 1, (x_inl16_rows((x_sc0_270) + 1)) + 1) = &
                &(0.75_c_float - (x_inl16_xint((x_sc0_270) + 1) * x_inl16_xint((x_sc0_270) + 1)))
            end do
            do x_sc0_271 = 0, (np_particles) - 1
                x_inl16_sx(((3 + x_inl16_i_shift((x_sc0_271) + 1))) + 1, (x_inl16_rows((x_sc0_271) + 1)) + 1) = &
                &((0.5_c_float * (0.5_c_float + x_inl16_xint((x_sc0_271) + 1))) * (0.5_c_float + &
                &x_inl16_xint((x_sc0_271) + 1)))
            end do
            do x_w0_272 = 0, (np_particles) - 1
                x_inl16_idx((x_w0_272) + 1) = (x_inl16_i((x_w0_272) + 1) - 1)
            end do
        else if ((o == 3)) then
            do x_w0_273 = 0, (np_particles) - 1
                x_inl16_i((x_w0_273) + 1) = INT(AINT(y_old((x_w0_273) + 1)), c_int64_t)
            end do
            do x_w0_274 = 0, (np_particles) - 1
                x_inl16_i_shift((x_w0_274) + 1) = (x_inl16_i((x_w0_274) + 1) - (j_new((x_w0_274) + 1) + 1))
            end do
            do x_w0_275 = 0, (np_particles) - 1
                x_inl16_xint((x_w0_275) + 1) = (y_old((x_w0_275) + 1) - x_inl16_i((x_w0_275) + 1))
            end do
            do x_sc0_276 = 0, (np_particles) - 1
                x_inl16_sx(((1 + x_inl16_i_shift((x_sc0_276) + 1))) + 1, (x_inl16_rows((x_sc0_276) + 1)) + 1) = &
                &(((0.16666666666666666_c_float * (1.0_c_float - x_inl16_xint((x_sc0_276) + 1))) * (1.0_c_float - &
                &x_inl16_xint((x_sc0_276) + 1))) * (1.0_c_float - x_inl16_xint((x_sc0_276) + 1)))
            end do
            do x_sc0_277 = 0, (np_particles) - 1
                x_inl16_sx(((2 + x_inl16_i_shift((x_sc0_277) + 1))) + 1, (x_inl16_rows((x_sc0_277) + 1)) + 1) = &
                &((2.0_c_float / 3.0_c_float) - ((x_inl16_xint((x_sc0_277) + 1) * x_inl16_xint((x_sc0_277) + 1)) * &
                &(1.0_c_float - (x_inl16_xint((x_sc0_277) + 1) / 2.0_c_float))))
            end do
            do x_sc0_278 = 0, (np_particles) - 1
                x_inl16_sx(((3 + x_inl16_i_shift((x_sc0_278) + 1))) + 1, (x_inl16_rows((x_sc0_278) + 1)) + 1) = &
                &((2.0_c_float / 3.0_c_float) - (((1.0_c_float - x_inl16_xint((x_sc0_278) + 1)) * (1.0_c_float - &
                &x_inl16_xint((x_sc0_278) + 1))) * (1.0_c_float - (0.5_c_float * (1.0_c_float - &
                &x_inl16_xint((x_sc0_278) + 1))))))
            end do
            do x_sc0_279 = 0, (np_particles) - 1
                x_inl16_sx(((4 + x_inl16_i_shift((x_sc0_279) + 1))) + 1, (x_inl16_rows((x_sc0_279) + 1)) + 1) = &
                &(((0.16666666666666666_c_float * x_inl16_xint((x_sc0_279) + 1)) * x_inl16_xint((x_sc0_279) + 1)) * &
                &x_inl16_xint((x_sc0_279) + 1))
            end do
            do x_w0_280 = 0, (np_particles) - 1
                x_inl16_idx((x_w0_280) + 1) = (x_inl16_i((x_w0_280) + 1) - 1)
            end do
        else
            do x_w0_281 = 0, (np_particles) - 1
                x_inl16_i((x_w0_281) + 1) = INT(AINT((y_old((x_w0_281) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_282 = 0, (np_particles) - 1
                x_inl16_i_shift((x_w0_282) + 1) = (x_inl16_i((x_w0_282) + 1) - (j_new((x_w0_282) + 1) + 2))
            end do
            do x_w0_283 = 0, (np_particles) - 1
                x_inl16_xint((x_w0_283) + 1) = (y_old((x_w0_283) + 1) - x_inl16_i((x_w0_283) + 1))
            end do
            do x_w0_284 = 0, (np_particles) - 1
                x_inl16_sm((x_w0_284) + 1) = (0.5_c_float - x_inl16_xint((x_w0_284) + 1))
            end do
            do x_w0_285 = 0, (np_particles) - 1
                x_inl16_sp((x_w0_285) + 1) = (0.5_c_float + x_inl16_xint((x_w0_285) + 1))
            end do
            do x_sc0_286 = 0, (np_particles) - 1
                x_inl16_sx(((1 + x_inl16_i_shift((x_sc0_286) + 1))) + 1, (x_inl16_rows((x_sc0_286) + 1)) + 1) = &
                &(((((1.0_c_float / 24.0_c_float) * x_inl16_sm((x_sc0_286) + 1)) * x_inl16_sm((x_sc0_286) + 1)) * &
                &x_inl16_sm((x_sc0_286) + 1)) * x_inl16_sm((x_sc0_286) + 1))
            end do
            do x_sc0_287 = 0, (np_particles) - 1
                x_inl16_sx(((2 + x_inl16_i_shift((x_sc0_287) + 1))) + 1, (x_inl16_rows((x_sc0_287) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * ((4.75_c_float - (11.0_c_float * x_inl16_xint((x_sc0_287) + 1))) + &
                &(((4.0_c_float * x_inl16_xint((x_sc0_287) + 1)) * x_inl16_xint((x_sc0_287) + 1)) * ((1.5_c_float + &
                &x_inl16_xint((x_sc0_287) + 1)) - (x_inl16_xint((x_sc0_287) + 1) * x_inl16_xint((x_sc0_287) + 1))))))
            end do
            do x_sc0_288 = 0, (np_particles) - 1
                x_inl16_sx(((3 + x_inl16_i_shift((x_sc0_288) + 1))) + 1, (x_inl16_rows((x_sc0_288) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * (14.375_c_float + (((6.0_c_float * x_inl16_xint((x_sc0_288) + 1)) * &
                &x_inl16_xint((x_sc0_288) + 1)) * ((x_inl16_xint((x_sc0_288) + 1) * x_inl16_xint((x_sc0_288) + 1)) - &
                &2.5_c_float))))
            end do
            do x_sc0_289 = 0, (np_particles) - 1
                x_inl16_sx(((4 + x_inl16_i_shift((x_sc0_289) + 1))) + 1, (x_inl16_rows((x_sc0_289) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * ((4.75_c_float + (11.0_c_float * x_inl16_xint((x_sc0_289) + 1))) + &
                &(((4.0_c_float * x_inl16_xint((x_sc0_289) + 1)) * x_inl16_xint((x_sc0_289) + 1)) * ((1.5_c_float - &
                &x_inl16_xint((x_sc0_289) + 1)) - (x_inl16_xint((x_sc0_289) + 1) * x_inl16_xint((x_sc0_289) + 1))))))
            end do
            do x_sc0_290 = 0, (np_particles) - 1
                x_inl16_sx(((5 + x_inl16_i_shift((x_sc0_290) + 1))) + 1, (x_inl16_rows((x_sc0_290) + 1)) + 1) = &
                &(((((1.0_c_float / 24.0_c_float) * x_inl16_sp((x_sc0_290) + 1)) * x_inl16_sp((x_sc0_290) + 1)) * &
                &x_inl16_sp((x_sc0_290) + 1)) * x_inl16_sp((x_sc0_290) + 1))
            end do
            do x_w0_291 = 0, (np_particles) - 1
                x_inl16_idx((x_w0_291) + 1) = (x_inl16_i((x_w0_291) + 1) - 2)
            end do
        end if
        if (.not. allocated(sy_old)) then
            allocate(sy_old(width, np_particles))
        else if (size(sy_old, 1) /= (width) .or. size(sy_old, 2) /= (np_particles)) then
            deallocate(sy_old)
            allocate(sy_old(width, np_particles))
        end if
        do x_w0_292 = 0, (np_particles) - 1
            do x_w1_293 = 0, (width) - 1
                sy_old((x_w1_293) + 1, (x_w0_292) + 1) = x_inl16_sx((x_w1_293) + 1, (x_w0_292) + 1)
            end do
        end do
        do x_w0_294 = 0, (np_particles) - 1
            j_old((x_w0_294) + 1) = x_inl16_idx((x_w0_294) + 1)
        end do
        if (reduce_enabled) then
            if (.not. allocated(x_inl17_sx)) then
                allocate(x_inl17_sx(width, np_particles))
            else if (size(x_inl17_sx, 1) /= (width) .or. size(x_inl17_sx, 2) /= (np_particles)) then
                deallocate(x_inl17_sx)
                allocate(x_inl17_sx(width, np_particles))
            end if
            x_inl17_sx = 0
            ! numpy: np.arange(np_particles)
            do x_i_295 = 0, (np_particles) - 1
                x_cb28((x_i_295) + 1) = x_i_295
            end do
            do x_w0_296 = 0, (np_particles) - 1
                x_inl17_rows((x_w0_296) + 1) = x_cb28((x_w0_296) + 1)
            end do
            do x_w0_297 = 0, (np_particles) - 1
                x_inl17_i((x_w0_297) + 1) = INT((aint(y_new((x_w0_297) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(y_new((x_w0_297) + 1)) < aint(y_new((x_w0_297) + 1)))), c_int64_t)
            end do
            do x_w0_298 = 0, (np_particles) - 1
                x_inl17_i_shift((x_w0_298) + 1) = (x_inl17_i((x_w0_298) + 1) - (j_new((x_w0_298) + 1) + half))
            end do
            do x_w0_299 = 0, (np_particles) - 1
                x_inl17_xint((x_w0_299) + 1) = (y_new((x_w0_299) + 1) - x_inl17_i((x_w0_299) + 1))
            end do
            do x_sc0_300 = 0, (np_particles) - 1
                x_inl17_sx((((half + 1) + x_inl17_i_shift((x_sc0_300) + 1))) + 1, (x_inl17_rows((x_sc0_300) + 1)) + 1) &
                &= (1.0_c_float - x_inl17_xint((x_sc0_300) + 1))
            end do
            do x_sc0_301 = 0, (np_particles) - 1
                x_inl17_sx((((half + 2) + x_inl17_i_shift((x_sc0_301) + 1))) + 1, (x_inl17_rows((x_sc0_301) + 1)) + 1) &
                &= x_inl17_xint((x_sc0_301) + 1)
            end do
            do x_w0_302 = 0, (np_particles) - 1
                x_inl17_idx((x_w0_302) + 1) = x_inl17_i((x_w0_302) + 1)
            end do
            if (.not. allocated(ov_new)) then
                allocate(ov_new(width, np_particles))
            else if (size(ov_new, 1) /= (width) .or. size(ov_new, 2) /= (np_particles)) then
                deallocate(ov_new)
                allocate(ov_new(width, np_particles))
            end if
            do x_w0_303 = 0, (np_particles) - 1
                do x_w1_304 = 0, (width) - 1
                    ov_new((x_w1_304) + 1, (x_w0_303) + 1) = x_inl17_sx((x_w1_304) + 1, (x_w0_303) + 1)
                end do
            end do
            if (.not. allocated(x_inl18_sx)) then
                allocate(x_inl18_sx(width, np_particles))
            else if (size(x_inl18_sx, 1) /= (width) .or. size(x_inl18_sx, 2) /= (np_particles)) then
                deallocate(x_inl18_sx)
                allocate(x_inl18_sx(width, np_particles))
            end if
            x_inl18_sx = 0
            ! numpy: np.arange(np_particles)
            do x_i_305 = 0, (np_particles) - 1
                x_cb29((x_i_305) + 1) = x_i_305
            end do
            do x_w0_306 = 0, (np_particles) - 1
                x_inl18_rows((x_w0_306) + 1) = x_cb29((x_w0_306) + 1)
            end do
            do x_w0_307 = 0, (np_particles) - 1
                x_inl18_i((x_w0_307) + 1) = INT((aint(y_old((x_w0_307) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(y_old((x_w0_307) + 1)) < aint(y_old((x_w0_307) + 1)))), c_int64_t)
            end do
            do x_w0_308 = 0, (np_particles) - 1
                x_inl18_i_shift((x_w0_308) + 1) = (x_inl18_i((x_w0_308) + 1) - (j_new((x_w0_308) + 1) + half))
            end do
            do x_w0_309 = 0, (np_particles) - 1
                x_inl18_xint((x_w0_309) + 1) = (y_old((x_w0_309) + 1) - x_inl18_i((x_w0_309) + 1))
            end do
            do x_sc0_310 = 0, (np_particles) - 1
                x_inl18_sx((((half + 1) + x_inl18_i_shift((x_sc0_310) + 1))) + 1, (x_inl18_rows((x_sc0_310) + 1)) + 1) &
                &= (1.0_c_float - x_inl18_xint((x_sc0_310) + 1))
            end do
            do x_sc0_311 = 0, (np_particles) - 1
                x_inl18_sx((((half + 2) + x_inl18_i_shift((x_sc0_311) + 1))) + 1, (x_inl18_rows((x_sc0_311) + 1)) + 1) &
                &= x_inl18_xint((x_sc0_311) + 1)
            end do
            do x_w0_312 = 0, (np_particles) - 1
                x_inl18_idx((x_w0_312) + 1) = x_inl18_i((x_w0_312) + 1)
            end do
            if (.not. allocated(ov_old)) then
                allocate(ov_old(width, np_particles))
            else if (size(ov_old, 1) /= (width) .or. size(ov_old, 2) /= (np_particles)) then
                deallocate(ov_old)
                allocate(ov_old(width, np_particles))
            end if
            do x_w0_313 = 0, (np_particles) - 1
                do x_w1_314 = 0, (width) - 1
                    ov_old((x_w1_314) + 1, (x_w0_313) + 1) = x_inl18_sx((x_w1_314) + 1, (x_w0_313) + 1)
                end do
            end do
            if (.not. allocated(x_cb30)) then
                allocate(x_cb30(width, np_particles))
            else if (size(x_cb30, 1) /= (width) .or. size(x_cb30, 2) /= (np_particles)) then
                deallocate(x_cb30)
                allocate(x_cb30(width, np_particles))
            end if
            ! numpy: np.where((reduce_shape_new != 0)[:, None], ov_new, sy_new)
            do x_r0_315 = 0, (np_particles) - 1
                do x_r1_316 = 0, (width) - 1
                    if ((reduce_shape_new((x_r0_315) + 1) /= 0)) then
                        x_ifexp22 = ov_new((x_r1_316) + 1, (x_r0_315) + 1)
                    else
                        x_ifexp22 = sy_new((x_r1_316) + 1, (x_r0_315) + 1)
                    end if
                    x_cb30((x_r1_316) + 1, (x_r0_315) + 1) = x_ifexp22
                end do
            end do
            if (.not. allocated(sy_new)) then
                allocate(sy_new(width, np_particles))
            else if (size(sy_new, 1) /= (width) .or. size(sy_new, 2) /= (np_particles)) then
                deallocate(sy_new)
                allocate(sy_new(width, np_particles))
            end if
            do x_w0_317 = 0, (np_particles) - 1
                do x_w1_318 = 0, (width) - 1
                    sy_new((x_w1_318) + 1, (x_w0_317) + 1) = x_cb30((x_w1_318) + 1, (x_w0_317) + 1)
                end do
            end do
            if (.not. allocated(x_cb31)) then
                allocate(x_cb31(width, np_particles))
            else if (size(x_cb31, 1) /= (width) .or. size(x_cb31, 2) /= (np_particles)) then
                deallocate(x_cb31)
                allocate(x_cb31(width, np_particles))
            end if
            ! numpy: np.where((reduce_shape_old != 0)[:, None], ov_old, sy_old)
            do x_r0_319 = 0, (np_particles) - 1
                do x_r1_320 = 0, (width) - 1
                    if ((reduce_shape_old((x_r0_319) + 1) /= 0)) then
                        x_ifexp23 = ov_old((x_r1_320) + 1, (x_r0_319) + 1)
                    else
                        x_ifexp23 = sy_old((x_r1_320) + 1, (x_r0_319) + 1)
                    end if
                    x_cb31((x_r1_320) + 1, (x_r0_319) + 1) = x_ifexp23
                end do
            end do
            if (.not. allocated(sy_old)) then
                allocate(sy_old(width, np_particles))
            else if (size(sy_old, 1) /= (width) .or. size(sy_old, 2) /= (np_particles)) then
                deallocate(sy_old)
                allocate(sy_old(width, np_particles))
            end if
            do x_w0_321 = 0, (np_particles) - 1
                do x_w1_322 = 0, (width) - 1
                    sy_old((x_w1_322) + 1, (x_w0_321) + 1) = x_cb31((x_w1_322) + 1, (x_w0_321) + 1)
                end do
            end do
        end if
    end if
    if (((geom /= 4) .AND. (geom /= 5))) then
        if (.not. allocated(x_inl19_sx)) then
            allocate(x_inl19_sx(width, np_particles))
        else if (size(x_inl19_sx, 1) /= (width) .or. size(x_inl19_sx, 2) /= (np_particles)) then
            deallocate(x_inl19_sx)
            allocate(x_inl19_sx(width, np_particles))
        end if
        x_inl19_sx = 0
        if ((o == 0)) then
            do x_w0_323 = 0, (np_particles) - 1
                x_inl19_j((x_w0_323) + 1) = INT(AINT((z_new((x_w0_323) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do si0_l324 = 0, (np_particles) - 1
                x_inl19_sx((1) + 1, (si0_l324) + 1) = 1.0_c_float
            end do
            do x_w0_325 = 0, (np_particles) - 1
                x_inl19_idx((x_w0_325) + 1) = x_inl19_j((x_w0_325) + 1)
            end do
        else if ((o == 1)) then
            do x_w0_326 = 0, (np_particles) - 1
                x_inl19_j((x_w0_326) + 1) = INT(AINT(z_new((x_w0_326) + 1)), c_int64_t)
            end do
            do x_w0_327 = 0, (np_particles) - 1
                x_inl19_xint((x_w0_327) + 1) = (z_new((x_w0_327) + 1) - x_inl19_j((x_w0_327) + 1))
            end do
            do si0_l328 = 0, (np_particles) - 1
                x_inl19_sx((1) + 1, (si0_l328) + 1) = (1.0_c_float - x_inl19_xint((si0_l328) + 1))
            end do
            do si0_l329 = 0, (np_particles) - 1
                x_inl19_sx((2) + 1, (si0_l329) + 1) = x_inl19_xint((si0_l329) + 1)
            end do
            do x_w0_330 = 0, (np_particles) - 1
                x_inl19_idx((x_w0_330) + 1) = x_inl19_j((x_w0_330) + 1)
            end do
        else if ((o == 2)) then
            do x_w0_331 = 0, (np_particles) - 1
                x_inl19_j((x_w0_331) + 1) = INT(AINT((z_new((x_w0_331) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_332 = 0, (np_particles) - 1
                x_inl19_xint((x_w0_332) + 1) = (z_new((x_w0_332) + 1) - x_inl19_j((x_w0_332) + 1))
            end do
            do si0_l333 = 0, (np_particles) - 1
                x_inl19_sx((1) + 1, (si0_l333) + 1) = ((0.5_c_float * (0.5_c_float - x_inl19_xint((si0_l333) + 1))) * &
                &(0.5_c_float - x_inl19_xint((si0_l333) + 1)))
            end do
            do si0_l334 = 0, (np_particles) - 1
                x_inl19_sx((2) + 1, (si0_l334) + 1) = (0.75_c_float - (x_inl19_xint((si0_l334) + 1) * &
                &x_inl19_xint((si0_l334) + 1)))
            end do
            do si0_l335 = 0, (np_particles) - 1
                x_inl19_sx((3) + 1, (si0_l335) + 1) = ((0.5_c_float * (0.5_c_float + x_inl19_xint((si0_l335) + 1))) * &
                &(0.5_c_float + x_inl19_xint((si0_l335) + 1)))
            end do
            do x_w0_336 = 0, (np_particles) - 1
                x_inl19_idx((x_w0_336) + 1) = (x_inl19_j((x_w0_336) + 1) - 1)
            end do
        else if ((o == 3)) then
            do x_w0_337 = 0, (np_particles) - 1
                x_inl19_j((x_w0_337) + 1) = INT(AINT(z_new((x_w0_337) + 1)), c_int64_t)
            end do
            do x_w0_338 = 0, (np_particles) - 1
                x_inl19_xint((x_w0_338) + 1) = (z_new((x_w0_338) + 1) - x_inl19_j((x_w0_338) + 1))
            end do
            do si0_l339 = 0, (np_particles) - 1
                x_inl19_sx((1) + 1, (si0_l339) + 1) = (((0.16666666666666666_c_float * (1.0_c_float - &
                &x_inl19_xint((si0_l339) + 1))) * (1.0_c_float - x_inl19_xint((si0_l339) + 1))) * (1.0_c_float - &
                &x_inl19_xint((si0_l339) + 1)))
            end do
            do si0_l340 = 0, (np_particles) - 1
                x_inl19_sx((2) + 1, (si0_l340) + 1) = ((2.0_c_float / 3.0_c_float) - ((x_inl19_xint((si0_l340) + 1) * &
                &x_inl19_xint((si0_l340) + 1)) * (1.0_c_float - (x_inl19_xint((si0_l340) + 1) / 2.0_c_float))))
            end do
            do si0_l341 = 0, (np_particles) - 1
                x_inl19_sx((3) + 1, (si0_l341) + 1) = ((2.0_c_float / 3.0_c_float) - (((1.0_c_float - &
                &x_inl19_xint((si0_l341) + 1)) * (1.0_c_float - x_inl19_xint((si0_l341) + 1))) * (1.0_c_float - &
                &(0.5_c_float * (1.0_c_float - x_inl19_xint((si0_l341) + 1))))))
            end do
            do si0_l342 = 0, (np_particles) - 1
                x_inl19_sx((4) + 1, (si0_l342) + 1) = (((0.16666666666666666_c_float * x_inl19_xint((si0_l342) + 1)) * &
                &x_inl19_xint((si0_l342) + 1)) * x_inl19_xint((si0_l342) + 1))
            end do
            do x_w0_343 = 0, (np_particles) - 1
                x_inl19_idx((x_w0_343) + 1) = (x_inl19_j((x_w0_343) + 1) - 1)
            end do
        else
            do x_w0_344 = 0, (np_particles) - 1
                x_inl19_j((x_w0_344) + 1) = INT(AINT((z_new((x_w0_344) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_345 = 0, (np_particles) - 1
                x_inl19_xint((x_w0_345) + 1) = (z_new((x_w0_345) + 1) - x_inl19_j((x_w0_345) + 1))
            end do
            do x_w0_346 = 0, (np_particles) - 1
                x_inl19_sm((x_w0_346) + 1) = (0.5_c_float - x_inl19_xint((x_w0_346) + 1))
            end do
            do x_w0_347 = 0, (np_particles) - 1
                x_inl19_sp((x_w0_347) + 1) = (0.5_c_float + x_inl19_xint((x_w0_347) + 1))
            end do
            do si0_l348 = 0, (np_particles) - 1
                x_inl19_sx((1) + 1, (si0_l348) + 1) = (((((1.0_c_float / 24.0_c_float) * x_inl19_sm((si0_l348) + 1)) * &
                &x_inl19_sm((si0_l348) + 1)) * x_inl19_sm((si0_l348) + 1)) * x_inl19_sm((si0_l348) + 1))
            end do
            do si0_l349 = 0, (np_particles) - 1
                x_inl19_sx((2) + 1, (si0_l349) + 1) = ((1.0_c_float / 24.0_c_float) * ((4.75_c_float - (11.0_c_float * &
                &x_inl19_xint((si0_l349) + 1))) + (((4.0_c_float * x_inl19_xint((si0_l349) + 1)) * &
                &x_inl19_xint((si0_l349) + 1)) * ((1.5_c_float + x_inl19_xint((si0_l349) + 1)) - &
                &(x_inl19_xint((si0_l349) + 1) * x_inl19_xint((si0_l349) + 1))))))
            end do
            do si0_l350 = 0, (np_particles) - 1
                x_inl19_sx((3) + 1, (si0_l350) + 1) = ((1.0_c_float / 24.0_c_float) * (14.375_c_float + (((6.0_c_float &
                &* x_inl19_xint((si0_l350) + 1)) * x_inl19_xint((si0_l350) + 1)) * ((x_inl19_xint((si0_l350) + 1) * &
                &x_inl19_xint((si0_l350) + 1)) - 2.5_c_float))))
            end do
            do si0_l351 = 0, (np_particles) - 1
                x_inl19_sx((4) + 1, (si0_l351) + 1) = ((1.0_c_float / 24.0_c_float) * ((4.75_c_float + (11.0_c_float * &
                &x_inl19_xint((si0_l351) + 1))) + (((4.0_c_float * x_inl19_xint((si0_l351) + 1)) * &
                &x_inl19_xint((si0_l351) + 1)) * ((1.5_c_float - x_inl19_xint((si0_l351) + 1)) - &
                &(x_inl19_xint((si0_l351) + 1) * x_inl19_xint((si0_l351) + 1))))))
            end do
            do si0_l352 = 0, (np_particles) - 1
                x_inl19_sx((5) + 1, (si0_l352) + 1) = (((((1.0_c_float / 24.0_c_float) * x_inl19_sp((si0_l352) + 1)) * &
                &x_inl19_sp((si0_l352) + 1)) * x_inl19_sp((si0_l352) + 1)) * x_inl19_sp((si0_l352) + 1))
            end do
            do x_w0_353 = 0, (np_particles) - 1
                x_inl19_idx((x_w0_353) + 1) = (x_inl19_j((x_w0_353) + 1) - 2)
            end do
        end if
        if (.not. allocated(sz_new)) then
            allocate(sz_new(width, np_particles))
        else if (size(sz_new, 1) /= (width) .or. size(sz_new, 2) /= (np_particles)) then
            deallocate(sz_new)
            allocate(sz_new(width, np_particles))
        end if
        do x_w0_354 = 0, (np_particles) - 1
            do x_w1_355 = 0, (width) - 1
                sz_new((x_w1_355) + 1, (x_w0_354) + 1) = x_inl19_sx((x_w1_355) + 1, (x_w0_354) + 1)
            end do
        end do
        do x_w0_356 = 0, (np_particles) - 1
            k_new((x_w0_356) + 1) = x_inl19_idx((x_w0_356) + 1)
        end do
        if (.not. allocated(x_inl20_sx)) then
            allocate(x_inl20_sx(width, np_particles))
        else if (size(x_inl20_sx, 1) /= (width) .or. size(x_inl20_sx, 2) /= (np_particles)) then
            deallocate(x_inl20_sx)
            allocate(x_inl20_sx(width, np_particles))
        end if
        x_inl20_sx = 0
        ! numpy: np.arange(np_particles)
        do x_i_357 = 0, (np_particles) - 1
            x_cb32((x_i_357) + 1) = x_i_357
        end do
        do x_w0_358 = 0, (np_particles) - 1
            x_inl20_rows((x_w0_358) + 1) = x_cb32((x_w0_358) + 1)
        end do
        if ((o == 0)) then
            do x_w0_359 = 0, (np_particles) - 1
                x_inl20_i((x_w0_359) + 1) = INT((aint((z_old((x_w0_359) + 1) + 0.5_c_float)) - merge(1.0_c_float, &
                &0.0_c_float, ((z_old((x_w0_359) + 1) + 0.5_c_float)) < aint((z_old((x_w0_359) + 1) + 0.5_c_float)))), &
                &c_int64_t)
            end do
            do x_w0_360 = 0, (np_particles) - 1
                x_inl20_i_shift((x_w0_360) + 1) = (x_inl20_i((x_w0_360) + 1) - k_new((x_w0_360) + 1))
            end do
            do x_sc0_361 = 0, (np_particles) - 1
                x_inl20_sx(((1 + x_inl20_i_shift((x_sc0_361) + 1))) + 1, (x_inl20_rows((x_sc0_361) + 1)) + 1) = &
                &1.0_c_float
            end do
            do x_w0_362 = 0, (np_particles) - 1
                x_inl20_idx((x_w0_362) + 1) = x_inl20_i((x_w0_362) + 1)
            end do
        else if ((o == 1)) then
            do x_w0_363 = 0, (np_particles) - 1
                x_inl20_i((x_w0_363) + 1) = INT((aint(z_old((x_w0_363) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_old((x_w0_363) + 1)) < aint(z_old((x_w0_363) + 1)))), c_int64_t)
            end do
            do x_w0_364 = 0, (np_particles) - 1
                x_inl20_i_shift((x_w0_364) + 1) = (x_inl20_i((x_w0_364) + 1) - k_new((x_w0_364) + 1))
            end do
            do x_w0_365 = 0, (np_particles) - 1
                x_inl20_xint((x_w0_365) + 1) = (z_old((x_w0_365) + 1) - x_inl20_i((x_w0_365) + 1))
            end do
            do x_sc0_366 = 0, (np_particles) - 1
                x_inl20_sx(((1 + x_inl20_i_shift((x_sc0_366) + 1))) + 1, (x_inl20_rows((x_sc0_366) + 1)) + 1) = &
                &(1.0_c_float - x_inl20_xint((x_sc0_366) + 1))
            end do
            do x_sc0_367 = 0, (np_particles) - 1
                x_inl20_sx(((2 + x_inl20_i_shift((x_sc0_367) + 1))) + 1, (x_inl20_rows((x_sc0_367) + 1)) + 1) = &
                &x_inl20_xint((x_sc0_367) + 1)
            end do
            do x_w0_368 = 0, (np_particles) - 1
                x_inl20_idx((x_w0_368) + 1) = x_inl20_i((x_w0_368) + 1)
            end do
        else if ((o == 2)) then
            do x_w0_369 = 0, (np_particles) - 1
                x_inl20_i((x_w0_369) + 1) = INT(AINT((z_old((x_w0_369) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_370 = 0, (np_particles) - 1
                x_inl20_i_shift((x_w0_370) + 1) = (x_inl20_i((x_w0_370) + 1) - (k_new((x_w0_370) + 1) + 1))
            end do
            do x_w0_371 = 0, (np_particles) - 1
                x_inl20_xint((x_w0_371) + 1) = (z_old((x_w0_371) + 1) - x_inl20_i((x_w0_371) + 1))
            end do
            do x_sc0_372 = 0, (np_particles) - 1
                x_inl20_sx(((1 + x_inl20_i_shift((x_sc0_372) + 1))) + 1, (x_inl20_rows((x_sc0_372) + 1)) + 1) = &
                &((0.5_c_float * (0.5_c_float - x_inl20_xint((x_sc0_372) + 1))) * (0.5_c_float - &
                &x_inl20_xint((x_sc0_372) + 1)))
            end do
            do x_sc0_373 = 0, (np_particles) - 1
                x_inl20_sx(((2 + x_inl20_i_shift((x_sc0_373) + 1))) + 1, (x_inl20_rows((x_sc0_373) + 1)) + 1) = &
                &(0.75_c_float - (x_inl20_xint((x_sc0_373) + 1) * x_inl20_xint((x_sc0_373) + 1)))
            end do
            do x_sc0_374 = 0, (np_particles) - 1
                x_inl20_sx(((3 + x_inl20_i_shift((x_sc0_374) + 1))) + 1, (x_inl20_rows((x_sc0_374) + 1)) + 1) = &
                &((0.5_c_float * (0.5_c_float + x_inl20_xint((x_sc0_374) + 1))) * (0.5_c_float + &
                &x_inl20_xint((x_sc0_374) + 1)))
            end do
            do x_w0_375 = 0, (np_particles) - 1
                x_inl20_idx((x_w0_375) + 1) = (x_inl20_i((x_w0_375) + 1) - 1)
            end do
        else if ((o == 3)) then
            do x_w0_376 = 0, (np_particles) - 1
                x_inl20_i((x_w0_376) + 1) = INT(AINT(z_old((x_w0_376) + 1)), c_int64_t)
            end do
            do x_w0_377 = 0, (np_particles) - 1
                x_inl20_i_shift((x_w0_377) + 1) = (x_inl20_i((x_w0_377) + 1) - (k_new((x_w0_377) + 1) + 1))
            end do
            do x_w0_378 = 0, (np_particles) - 1
                x_inl20_xint((x_w0_378) + 1) = (z_old((x_w0_378) + 1) - x_inl20_i((x_w0_378) + 1))
            end do
            do x_sc0_379 = 0, (np_particles) - 1
                x_inl20_sx(((1 + x_inl20_i_shift((x_sc0_379) + 1))) + 1, (x_inl20_rows((x_sc0_379) + 1)) + 1) = &
                &(((0.16666666666666666_c_float * (1.0_c_float - x_inl20_xint((x_sc0_379) + 1))) * (1.0_c_float - &
                &x_inl20_xint((x_sc0_379) + 1))) * (1.0_c_float - x_inl20_xint((x_sc0_379) + 1)))
            end do
            do x_sc0_380 = 0, (np_particles) - 1
                x_inl20_sx(((2 + x_inl20_i_shift((x_sc0_380) + 1))) + 1, (x_inl20_rows((x_sc0_380) + 1)) + 1) = &
                &((2.0_c_float / 3.0_c_float) - ((x_inl20_xint((x_sc0_380) + 1) * x_inl20_xint((x_sc0_380) + 1)) * &
                &(1.0_c_float - (x_inl20_xint((x_sc0_380) + 1) / 2.0_c_float))))
            end do
            do x_sc0_381 = 0, (np_particles) - 1
                x_inl20_sx(((3 + x_inl20_i_shift((x_sc0_381) + 1))) + 1, (x_inl20_rows((x_sc0_381) + 1)) + 1) = &
                &((2.0_c_float / 3.0_c_float) - (((1.0_c_float - x_inl20_xint((x_sc0_381) + 1)) * (1.0_c_float - &
                &x_inl20_xint((x_sc0_381) + 1))) * (1.0_c_float - (0.5_c_float * (1.0_c_float - &
                &x_inl20_xint((x_sc0_381) + 1))))))
            end do
            do x_sc0_382 = 0, (np_particles) - 1
                x_inl20_sx(((4 + x_inl20_i_shift((x_sc0_382) + 1))) + 1, (x_inl20_rows((x_sc0_382) + 1)) + 1) = &
                &(((0.16666666666666666_c_float * x_inl20_xint((x_sc0_382) + 1)) * x_inl20_xint((x_sc0_382) + 1)) * &
                &x_inl20_xint((x_sc0_382) + 1))
            end do
            do x_w0_383 = 0, (np_particles) - 1
                x_inl20_idx((x_w0_383) + 1) = (x_inl20_i((x_w0_383) + 1) - 1)
            end do
        else
            do x_w0_384 = 0, (np_particles) - 1
                x_inl20_i((x_w0_384) + 1) = INT(AINT((z_old((x_w0_384) + 1) + 0.5_c_float)), c_int64_t)
            end do
            do x_w0_385 = 0, (np_particles) - 1
                x_inl20_i_shift((x_w0_385) + 1) = (x_inl20_i((x_w0_385) + 1) - (k_new((x_w0_385) + 1) + 2))
            end do
            do x_w0_386 = 0, (np_particles) - 1
                x_inl20_xint((x_w0_386) + 1) = (z_old((x_w0_386) + 1) - x_inl20_i((x_w0_386) + 1))
            end do
            do x_w0_387 = 0, (np_particles) - 1
                x_inl20_sm((x_w0_387) + 1) = (0.5_c_float - x_inl20_xint((x_w0_387) + 1))
            end do
            do x_w0_388 = 0, (np_particles) - 1
                x_inl20_sp((x_w0_388) + 1) = (0.5_c_float + x_inl20_xint((x_w0_388) + 1))
            end do
            do x_sc0_389 = 0, (np_particles) - 1
                x_inl20_sx(((1 + x_inl20_i_shift((x_sc0_389) + 1))) + 1, (x_inl20_rows((x_sc0_389) + 1)) + 1) = &
                &(((((1.0_c_float / 24.0_c_float) * x_inl20_sm((x_sc0_389) + 1)) * x_inl20_sm((x_sc0_389) + 1)) * &
                &x_inl20_sm((x_sc0_389) + 1)) * x_inl20_sm((x_sc0_389) + 1))
            end do
            do x_sc0_390 = 0, (np_particles) - 1
                x_inl20_sx(((2 + x_inl20_i_shift((x_sc0_390) + 1))) + 1, (x_inl20_rows((x_sc0_390) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * ((4.75_c_float - (11.0_c_float * x_inl20_xint((x_sc0_390) + 1))) + &
                &(((4.0_c_float * x_inl20_xint((x_sc0_390) + 1)) * x_inl20_xint((x_sc0_390) + 1)) * ((1.5_c_float + &
                &x_inl20_xint((x_sc0_390) + 1)) - (x_inl20_xint((x_sc0_390) + 1) * x_inl20_xint((x_sc0_390) + 1))))))
            end do
            do x_sc0_391 = 0, (np_particles) - 1
                x_inl20_sx(((3 + x_inl20_i_shift((x_sc0_391) + 1))) + 1, (x_inl20_rows((x_sc0_391) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * (14.375_c_float + (((6.0_c_float * x_inl20_xint((x_sc0_391) + 1)) * &
                &x_inl20_xint((x_sc0_391) + 1)) * ((x_inl20_xint((x_sc0_391) + 1) * x_inl20_xint((x_sc0_391) + 1)) - &
                &2.5_c_float))))
            end do
            do x_sc0_392 = 0, (np_particles) - 1
                x_inl20_sx(((4 + x_inl20_i_shift((x_sc0_392) + 1))) + 1, (x_inl20_rows((x_sc0_392) + 1)) + 1) = &
                &((1.0_c_float / 24.0_c_float) * ((4.75_c_float + (11.0_c_float * x_inl20_xint((x_sc0_392) + 1))) + &
                &(((4.0_c_float * x_inl20_xint((x_sc0_392) + 1)) * x_inl20_xint((x_sc0_392) + 1)) * ((1.5_c_float - &
                &x_inl20_xint((x_sc0_392) + 1)) - (x_inl20_xint((x_sc0_392) + 1) * x_inl20_xint((x_sc0_392) + 1))))))
            end do
            do x_sc0_393 = 0, (np_particles) - 1
                x_inl20_sx(((5 + x_inl20_i_shift((x_sc0_393) + 1))) + 1, (x_inl20_rows((x_sc0_393) + 1)) + 1) = &
                &(((((1.0_c_float / 24.0_c_float) * x_inl20_sp((x_sc0_393) + 1)) * x_inl20_sp((x_sc0_393) + 1)) * &
                &x_inl20_sp((x_sc0_393) + 1)) * x_inl20_sp((x_sc0_393) + 1))
            end do
            do x_w0_394 = 0, (np_particles) - 1
                x_inl20_idx((x_w0_394) + 1) = (x_inl20_i((x_w0_394) + 1) - 2)
            end do
        end if
        if (.not. allocated(sz_old)) then
            allocate(sz_old(width, np_particles))
        else if (size(sz_old, 1) /= (width) .or. size(sz_old, 2) /= (np_particles)) then
            deallocate(sz_old)
            allocate(sz_old(width, np_particles))
        end if
        do x_w0_395 = 0, (np_particles) - 1
            do x_w1_396 = 0, (width) - 1
                sz_old((x_w1_396) + 1, (x_w0_395) + 1) = x_inl20_sx((x_w1_396) + 1, (x_w0_395) + 1)
            end do
        end do
        do x_w0_397 = 0, (np_particles) - 1
            k_old((x_w0_397) + 1) = x_inl20_idx((x_w0_397) + 1)
        end do
        if (reduce_enabled) then
            if (.not. allocated(x_inl21_sx)) then
                allocate(x_inl21_sx(width, np_particles))
            else if (size(x_inl21_sx, 1) /= (width) .or. size(x_inl21_sx, 2) /= (np_particles)) then
                deallocate(x_inl21_sx)
                allocate(x_inl21_sx(width, np_particles))
            end if
            x_inl21_sx = 0
            ! numpy: np.arange(np_particles)
            do x_i_398 = 0, (np_particles) - 1
                x_cb33((x_i_398) + 1) = x_i_398
            end do
            do x_w0_399 = 0, (np_particles) - 1
                x_inl21_rows((x_w0_399) + 1) = x_cb33((x_w0_399) + 1)
            end do
            do x_w0_400 = 0, (np_particles) - 1
                x_inl21_i((x_w0_400) + 1) = INT((aint(z_new((x_w0_400) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_new((x_w0_400) + 1)) < aint(z_new((x_w0_400) + 1)))), c_int64_t)
            end do
            do x_w0_401 = 0, (np_particles) - 1
                x_inl21_i_shift((x_w0_401) + 1) = (x_inl21_i((x_w0_401) + 1) - (k_new((x_w0_401) + 1) + half))
            end do
            do x_w0_402 = 0, (np_particles) - 1
                x_inl21_xint((x_w0_402) + 1) = (z_new((x_w0_402) + 1) - x_inl21_i((x_w0_402) + 1))
            end do
            do x_sc0_403 = 0, (np_particles) - 1
                x_inl21_sx((((half + 1) + x_inl21_i_shift((x_sc0_403) + 1))) + 1, (x_inl21_rows((x_sc0_403) + 1)) + 1) &
                &= (1.0_c_float - x_inl21_xint((x_sc0_403) + 1))
            end do
            do x_sc0_404 = 0, (np_particles) - 1
                x_inl21_sx((((half + 2) + x_inl21_i_shift((x_sc0_404) + 1))) + 1, (x_inl21_rows((x_sc0_404) + 1)) + 1) &
                &= x_inl21_xint((x_sc0_404) + 1)
            end do
            do x_w0_405 = 0, (np_particles) - 1
                x_inl21_idx((x_w0_405) + 1) = x_inl21_i((x_w0_405) + 1)
            end do
            if (.not. allocated(ov_new)) then
                allocate(ov_new(width, np_particles))
            else if (size(ov_new, 1) /= (width) .or. size(ov_new, 2) /= (np_particles)) then
                deallocate(ov_new)
                allocate(ov_new(width, np_particles))
            end if
            do x_w0_406 = 0, (np_particles) - 1
                do x_w1_407 = 0, (width) - 1
                    ov_new((x_w1_407) + 1, (x_w0_406) + 1) = x_inl21_sx((x_w1_407) + 1, (x_w0_406) + 1)
                end do
            end do
            if (.not. allocated(x_inl22_sx)) then
                allocate(x_inl22_sx(width, np_particles))
            else if (size(x_inl22_sx, 1) /= (width) .or. size(x_inl22_sx, 2) /= (np_particles)) then
                deallocate(x_inl22_sx)
                allocate(x_inl22_sx(width, np_particles))
            end if
            x_inl22_sx = 0
            ! numpy: np.arange(np_particles)
            do x_i_408 = 0, (np_particles) - 1
                x_cb34((x_i_408) + 1) = x_i_408
            end do
            do x_w0_409 = 0, (np_particles) - 1
                x_inl22_rows((x_w0_409) + 1) = x_cb34((x_w0_409) + 1)
            end do
            do x_w0_410 = 0, (np_particles) - 1
                x_inl22_i((x_w0_410) + 1) = INT((aint(z_old((x_w0_410) + 1)) - merge(1.0_c_float, 0.0_c_float, &
                &(z_old((x_w0_410) + 1)) < aint(z_old((x_w0_410) + 1)))), c_int64_t)
            end do
            do x_w0_411 = 0, (np_particles) - 1
                x_inl22_i_shift((x_w0_411) + 1) = (x_inl22_i((x_w0_411) + 1) - (k_new((x_w0_411) + 1) + half))
            end do
            do x_w0_412 = 0, (np_particles) - 1
                x_inl22_xint((x_w0_412) + 1) = (z_old((x_w0_412) + 1) - x_inl22_i((x_w0_412) + 1))
            end do
            do x_sc0_413 = 0, (np_particles) - 1
                x_inl22_sx((((half + 1) + x_inl22_i_shift((x_sc0_413) + 1))) + 1, (x_inl22_rows((x_sc0_413) + 1)) + 1) &
                &= (1.0_c_float - x_inl22_xint((x_sc0_413) + 1))
            end do
            do x_sc0_414 = 0, (np_particles) - 1
                x_inl22_sx((((half + 2) + x_inl22_i_shift((x_sc0_414) + 1))) + 1, (x_inl22_rows((x_sc0_414) + 1)) + 1) &
                &= x_inl22_xint((x_sc0_414) + 1)
            end do
            do x_w0_415 = 0, (np_particles) - 1
                x_inl22_idx((x_w0_415) + 1) = x_inl22_i((x_w0_415) + 1)
            end do
            if (.not. allocated(ov_old)) then
                allocate(ov_old(width, np_particles))
            else if (size(ov_old, 1) /= (width) .or. size(ov_old, 2) /= (np_particles)) then
                deallocate(ov_old)
                allocate(ov_old(width, np_particles))
            end if
            do x_w0_416 = 0, (np_particles) - 1
                do x_w1_417 = 0, (width) - 1
                    ov_old((x_w1_417) + 1, (x_w0_416) + 1) = x_inl22_sx((x_w1_417) + 1, (x_w0_416) + 1)
                end do
            end do
            if (.not. allocated(x_cb35)) then
                allocate(x_cb35(width, np_particles))
            else if (size(x_cb35, 1) /= (width) .or. size(x_cb35, 2) /= (np_particles)) then
                deallocate(x_cb35)
                allocate(x_cb35(width, np_particles))
            end if
            ! numpy: np.where((reduce_shape_new != 0)[:, None], ov_new, sz_new)
            do x_r0_418 = 0, (np_particles) - 1
                do x_r1_419 = 0, (width) - 1
                    if ((reduce_shape_new((x_r0_418) + 1) /= 0)) then
                        x_ifexp24 = ov_new((x_r1_419) + 1, (x_r0_418) + 1)
                    else
                        x_ifexp24 = sz_new((x_r1_419) + 1, (x_r0_418) + 1)
                    end if
                    x_cb35((x_r1_419) + 1, (x_r0_418) + 1) = x_ifexp24
                end do
            end do
            if (.not. allocated(sz_new)) then
                allocate(sz_new(width, np_particles))
            else if (size(sz_new, 1) /= (width) .or. size(sz_new, 2) /= (np_particles)) then
                deallocate(sz_new)
                allocate(sz_new(width, np_particles))
            end if
            do x_w0_420 = 0, (np_particles) - 1
                do x_w1_421 = 0, (width) - 1
                    sz_new((x_w1_421) + 1, (x_w0_420) + 1) = x_cb35((x_w1_421) + 1, (x_w0_420) + 1)
                end do
            end do
            if (.not. allocated(x_cb36)) then
                allocate(x_cb36(width, np_particles))
            else if (size(x_cb36, 1) /= (width) .or. size(x_cb36, 2) /= (np_particles)) then
                deallocate(x_cb36)
                allocate(x_cb36(width, np_particles))
            end if
            ! numpy: np.where((reduce_shape_old != 0)[:, None], ov_old, sz_old)
            do x_r0_422 = 0, (np_particles) - 1
                do x_r1_423 = 0, (width) - 1
                    if ((reduce_shape_old((x_r0_422) + 1) /= 0)) then
                        x_ifexp25 = ov_old((x_r1_423) + 1, (x_r0_422) + 1)
                    else
                        x_ifexp25 = sz_old((x_r1_423) + 1, (x_r0_422) + 1)
                    end if
                    x_cb36((x_r1_423) + 1, (x_r0_422) + 1) = x_ifexp25
                end do
            end do
            if (.not. allocated(sz_old)) then
                allocate(sz_old(width, np_particles))
            else if (size(sz_old, 1) /= (width) .or. size(sz_old, 2) /= (np_particles)) then
                deallocate(sz_old)
                allocate(sz_old(width, np_particles))
            end if
            do x_w0_424 = 0, (np_particles) - 1
                do x_w1_425 = 0, (width) - 1
                    sz_old((x_w1_425) + 1, (x_w0_424) + 1) = x_cb36((x_w1_425) + 1, (x_w0_424) + 1)
                end do
            end do
        end if
    end if
    dil = 1
    do x_w0_426 = 0, (np_particles) - 1
        diu((x_w0_426) + 1) = dil((x_w0_426) + 1)
    end do
    do x_w0_427 = 0, (np_particles) - 1
        djl((x_w0_427) + 1) = dil((x_w0_427) + 1)
    end do
    do x_w0_428 = 0, (np_particles) - 1
        dju((x_w0_428) + 1) = dil((x_w0_428) + 1)
    end do
    do x_w0_429 = 0, (np_particles) - 1
        dkl((x_w0_429) + 1) = dil((x_w0_429) + 1)
    end do
    do x_w0_430 = 0, (np_particles) - 1
        dku((x_w0_430) + 1) = dil((x_w0_430) + 1)
    end do
    if ((geom /= 0)) then
        ! numpy: np.where(i_old < i_new, 0, 1)
        do x_r0_431 = 0, (np_particles) - 1
            if ((i_old((x_r0_431) + 1) < i_new((x_r0_431) + 1))) then
                x_ifexp26 = 0
            else
                x_ifexp26 = 1
            end if
            x_cb37((x_r0_431) + 1) = x_ifexp26
        end do
        do x_w0_432 = 0, (np_particles) - 1
            dil((x_w0_432) + 1) = x_cb37((x_w0_432) + 1)
        end do
        ! numpy: np.where(i_old > i_new, 0, 1)
        do x_r0_433 = 0, (np_particles) - 1
            if ((i_old((x_r0_433) + 1) > i_new((x_r0_433) + 1))) then
                x_ifexp27 = 0
            else
                x_ifexp27 = 1
            end if
            x_cb38((x_r0_433) + 1) = x_ifexp27
        end do
        do x_w0_434 = 0, (np_particles) - 1
            diu((x_w0_434) + 1) = x_cb38((x_w0_434) + 1)
        end do
    end if
    if ((geom == 3)) then
        ! numpy: np.where(j_old < j_new, 0, 1)
        do x_r0_435 = 0, (np_particles) - 1
            if ((j_old((x_r0_435) + 1) < j_new((x_r0_435) + 1))) then
                x_ifexp28 = 0
            else
                x_ifexp28 = 1
            end if
            x_cb39((x_r0_435) + 1) = x_ifexp28
        end do
        do x_w0_436 = 0, (np_particles) - 1
            djl((x_w0_436) + 1) = x_cb39((x_w0_436) + 1)
        end do
        ! numpy: np.where(j_old > j_new, 0, 1)
        do x_r0_437 = 0, (np_particles) - 1
            if ((j_old((x_r0_437) + 1) > j_new((x_r0_437) + 1))) then
                x_ifexp29 = 0
            else
                x_ifexp29 = 1
            end if
            x_cb40((x_r0_437) + 1) = x_ifexp29
        end do
        do x_w0_438 = 0, (np_particles) - 1
            dju((x_w0_438) + 1) = x_cb40((x_w0_438) + 1)
        end do
    end if
    if (((geom /= 4) .AND. (geom /= 5))) then
        ! numpy: np.where(k_old < k_new, 0, 1)
        do x_r0_439 = 0, (np_particles) - 1
            if ((k_old((x_r0_439) + 1) < k_new((x_r0_439) + 1))) then
                x_ifexp30 = 0
            else
                x_ifexp30 = 1
            end if
            x_cb41((x_r0_439) + 1) = x_ifexp30
        end do
        do x_w0_440 = 0, (np_particles) - 1
            dkl((x_w0_440) + 1) = x_cb41((x_w0_440) + 1)
        end do
        ! numpy: np.where(k_old > k_new, 0, 1)
        do x_r0_441 = 0, (np_particles) - 1
            if ((k_old((x_r0_441) + 1) > k_new((x_r0_441) + 1))) then
                x_ifexp31 = 0
            else
                x_ifexp31 = 1
            end if
            x_cb42((x_r0_441) + 1) = x_ifexp31
        end do
        do x_w0_442 = 0, (np_particles) - 1
            dku((x_w0_442) + 1) = x_cb42((x_w0_442) + 1)
        end do
    end if
    do ip_l443 = 0, (np_particles) - 1
        wqi = wq((ip_l443) + 1)
        if ((geom == 3)) then
            i0 = INT(dil((ip_l443) + 1), c_int64_t)
            i1 = ((o + 2) - INT(diu((ip_l443) + 1), c_int64_t))
            j0 = INT(djl((ip_l443) + 1), c_int64_t)
            j1 = ((o + 3) - INT(dju((ip_l443) + 1), c_int64_t))
            k0 = INT(dkl((ip_l443) + 1), c_int64_t)
            k1 = ((o + 3) - INT(dku((ip_l443) + 1), c_int64_t))
            ib = (INT(i_new((ip_l443) + 1), c_int64_t) - 1)
            jb = (INT(j_new((ip_l443) + 1), c_int64_t) - 1)
            kb = (INT(k_new((ip_l443) + 1), c_int64_t) - 1)
            if (.not. allocated(gx)) then
                allocate(gx((k1 - k0), (j1 - j0)))
            else if (size(gx, 1) /= ((k1 - k0)) .or. size(gx, 2) /= ((j1 - j0))) then
                deallocate(gx)
                allocate(gx((k1 - k0), (j1 - j0)))
            end if
            do x_w0_444 = 0, ((j1 - j0)) - 1
                do x_w1_445 = 0, ((k1 - k0)) - 1
                    gx((x_w1_445) + 1, (x_w0_444) + 1) = ((0.3333333333333333_c_float * ((sy_new(((x_w0_444 + (j0 - &
                    &0))) + 1, (ip_l443) + 1) * sz_new(((x_w1_445 + (k0 - 0))) + 1, (ip_l443) + 1)) + &
                    &(sy_old(((x_w0_444 + (j0 - 0))) + 1, (ip_l443) + 1) * sz_old(((x_w1_445 + (k0 - 0))) + 1, &
                    &(ip_l443) + 1)))) + (0.16666666666666666_c_float * ((sy_new(((x_w0_444 + (j0 - 0))) + 1, &
                    &(ip_l443) + 1) * sz_old(((x_w1_445 + (k0 - 0))) + 1, (ip_l443) + 1)) + (sy_old(((x_w0_444 + (j0 - &
                    &0))) + 1, (ip_l443) + 1) * sz_new(((x_w1_445 + (k0 - 0))) + 1, (ip_l443) + 1)))))
                end do
            end do
            if (.not. allocated(x_cb43)) then
                allocate(x_cb43((i1 - i0)))
            else if (size(x_cb43, 1) /= ((i1 - i0))) then
                deallocate(x_cb43)
                allocate(x_cb43((i1 - i0)))
            end if
            if (.not. allocated(x_cb43)) then
                allocate(x_cb43((i1 - i0)))
            else if (size(x_cb43, 1) /= ((i1 - i0))) then
                deallocate(x_cb43)
                allocate(x_cb43((i1 - i0)))
            end if
            do x_w0_446 = 0, ((i1 - i0)) - 1
                x_cb43((x_w0_446) + 1) = ((wqi * invdtd_x) * (sx_old(((x_w0_446 + (i0 - 0))) + 1, (ip_l443) + 1) - &
                &sx_new(((x_w0_446 + (i0 - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_x)) then
                allocate(cum_x((i1 - i0)))
            else if (size(cum_x, 1) /= ((i1 - i0))) then
                deallocate(cum_x)
                allocate(cum_x((i1 - i0)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_x * (sx_old[ip, i0:i1] - sx_new[ip, i0:i1]))
            cum_x((0) + 1) = x_cb43((0) + 1)
            do x_cs0_447 = 1, ((i1 - i0)) - 1
                cum_x((x_cs0_447) + 1) = (cum_x(((x_cs0_447 - 1)) + 1) + x_cb43((x_cs0_447) + 1))
            end do
            do si0_l448 = ((lox + ib) + i0), (((lox + ib) + i1)) - 1
                do si1_l449 = ((loy + jb) + j0), (((loy + jb) + j1)) - 1
                    do si2_l450 = ((loz + kb) + k0), (((loz + kb) + k1)) - 1
                        Jx((0) + 1, (si2_l450) + 1, (si1_l449) + 1, (si0_l448) + 1) = Jx((0) + 1, (si2_l450) + 1, &
                        &(si1_l449) + 1, (si0_l448) + 1) + ((cum_x(((si0_l448 + (0 - ((lox + ib) + i0)))) + 1) * &
                        &gx(((si2_l450 + (0 - ((loz + kb) + k0)))) + 1, ((si1_l449 + (0 - ((loy + jb) + j0)))) + 1)))
                    end do
                end do
            end do
            i0y = INT(dil((ip_l443) + 1), c_int64_t)
            i1y = ((o + 3) - INT(diu((ip_l443) + 1), c_int64_t))
            j0y = INT(djl((ip_l443) + 1), c_int64_t)
            j1y = ((o + 2) - INT(dju((ip_l443) + 1), c_int64_t))
            k0y = INT(dkl((ip_l443) + 1), c_int64_t)
            k1y = ((o + 3) - INT(dku((ip_l443) + 1), c_int64_t))
            if (.not. allocated(gy)) then
                allocate(gy((k1y - k0y), (i1y - i0y)))
            else if (size(gy, 1) /= ((k1y - k0y)) .or. size(gy, 2) /= ((i1y - i0y))) then
                deallocate(gy)
                allocate(gy((k1y - k0y), (i1y - i0y)))
            end if
            do x_w0_451 = 0, ((i1y - i0y)) - 1
                do x_w1_452 = 0, ((k1y - k0y)) - 1
                    gy((x_w1_452) + 1, (x_w0_451) + 1) = ((0.3333333333333333_c_float * ((sx_new(((x_w0_451 + (i0y - &
                    &0))) + 1, (ip_l443) + 1) * sz_new(((x_w1_452 + (k0y - 0))) + 1, (ip_l443) + 1)) + &
                    &(sx_old(((x_w0_451 + (i0y - 0))) + 1, (ip_l443) + 1) * sz_old(((x_w1_452 + (k0y - 0))) + 1, &
                    &(ip_l443) + 1)))) + (0.16666666666666666_c_float * ((sx_new(((x_w0_451 + (i0y - 0))) + 1, &
                    &(ip_l443) + 1) * sz_old(((x_w1_452 + (k0y - 0))) + 1, (ip_l443) + 1)) + (sx_old(((x_w0_451 + (i0y &
                    &- 0))) + 1, (ip_l443) + 1) * sz_new(((x_w1_452 + (k0y - 0))) + 1, (ip_l443) + 1)))))
                end do
            end do
            if (.not. allocated(x_cb45)) then
                allocate(x_cb45((j1y - j0y)))
            else if (size(x_cb45, 1) /= ((j1y - j0y))) then
                deallocate(x_cb45)
                allocate(x_cb45((j1y - j0y)))
            end if
            if (.not. allocated(x_cb45)) then
                allocate(x_cb45((j1y - j0y)))
            else if (size(x_cb45, 1) /= ((j1y - j0y))) then
                deallocate(x_cb45)
                allocate(x_cb45((j1y - j0y)))
            end if
            do x_w0_453 = 0, ((j1y - j0y)) - 1
                x_cb45((x_w0_453) + 1) = ((wqi * invdtd_y) * (sy_old(((x_w0_453 + (j0y - 0))) + 1, (ip_l443) + 1) - &
                &sy_new(((x_w0_453 + (j0y - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_y)) then
                allocate(cum_y((j1y - j0y)))
            else if (size(cum_y, 1) /= ((j1y - j0y))) then
                deallocate(cum_y)
                allocate(cum_y((j1y - j0y)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_y * (sy_old[ip, j0y:j1y] - sy_new[ip, j0y:j1y]))
            cum_y((0) + 1) = x_cb45((0) + 1)
            do x_cs0_454 = 1, ((j1y - j0y)) - 1
                cum_y((x_cs0_454) + 1) = (cum_y(((x_cs0_454 - 1)) + 1) + x_cb45((x_cs0_454) + 1))
            end do
            do si0_l455 = ((lox + ib) + i0y), (((lox + ib) + i1y)) - 1
                do si1_l456 = ((loy + jb) + j0y), (((loy + jb) + j1y)) - 1
                    do si2_l457 = ((loz + kb) + k0y), (((loz + kb) + k1y)) - 1
                        Jy((0) + 1, (si2_l457) + 1, (si1_l456) + 1, (si0_l455) + 1) = Jy((0) + 1, (si2_l457) + 1, &
                        &(si1_l456) + 1, (si0_l455) + 1) + ((gy(((si2_l457 + (0 - ((loz + kb) + k0y)))) + 1, &
                        &((si0_l455 + (0 - ((lox + ib) + i0y)))) + 1) * cum_y(((si1_l456 + (0 - ((loy + jb) + j0y)))) &
                        &+ 1)))
                    end do
                end do
            end do
            i0z = INT(dil((ip_l443) + 1), c_int64_t)
            i1z = ((o + 3) - INT(diu((ip_l443) + 1), c_int64_t))
            j0z = INT(djl((ip_l443) + 1), c_int64_t)
            j1z = ((o + 3) - INT(dju((ip_l443) + 1), c_int64_t))
            k0z = INT(dkl((ip_l443) + 1), c_int64_t)
            k1z = ((o + 2) - INT(dku((ip_l443) + 1), c_int64_t))
            if (.not. allocated(gz)) then
                allocate(gz((j1z - j0z), (i1z - i0z)))
            else if (size(gz, 1) /= ((j1z - j0z)) .or. size(gz, 2) /= ((i1z - i0z))) then
                deallocate(gz)
                allocate(gz((j1z - j0z), (i1z - i0z)))
            end if
            do x_w0_458 = 0, ((i1z - i0z)) - 1
                do x_w1_459 = 0, ((j1z - j0z)) - 1
                    gz((x_w1_459) + 1, (x_w0_458) + 1) = ((0.3333333333333333_c_float * ((sx_new(((x_w0_458 + (i0z - &
                    &0))) + 1, (ip_l443) + 1) * sy_new(((x_w1_459 + (j0z - 0))) + 1, (ip_l443) + 1)) + &
                    &(sx_old(((x_w0_458 + (i0z - 0))) + 1, (ip_l443) + 1) * sy_old(((x_w1_459 + (j0z - 0))) + 1, &
                    &(ip_l443) + 1)))) + (0.16666666666666666_c_float * ((sx_new(((x_w0_458 + (i0z - 0))) + 1, &
                    &(ip_l443) + 1) * sy_old(((x_w1_459 + (j0z - 0))) + 1, (ip_l443) + 1)) + (sx_old(((x_w0_458 + (i0z &
                    &- 0))) + 1, (ip_l443) + 1) * sy_new(((x_w1_459 + (j0z - 0))) + 1, (ip_l443) + 1)))))
                end do
            end do
            if (.not. allocated(x_cb47)) then
                allocate(x_cb47((k1z - k0z)))
            else if (size(x_cb47, 1) /= ((k1z - k0z))) then
                deallocate(x_cb47)
                allocate(x_cb47((k1z - k0z)))
            end if
            if (.not. allocated(x_cb47)) then
                allocate(x_cb47((k1z - k0z)))
            else if (size(x_cb47, 1) /= ((k1z - k0z))) then
                deallocate(x_cb47)
                allocate(x_cb47((k1z - k0z)))
            end if
            do x_w0_460 = 0, ((k1z - k0z)) - 1
                x_cb47((x_w0_460) + 1) = ((wqi * invdtd_z) * (sz_old(((x_w0_460 + (k0z - 0))) + 1, (ip_l443) + 1) - &
                &sz_new(((x_w0_460 + (k0z - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_z)) then
                allocate(cum_z((k1z - k0z)))
            else if (size(cum_z, 1) /= ((k1z - k0z))) then
                deallocate(cum_z)
                allocate(cum_z((k1z - k0z)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_z * (sz_old[ip, k0z:k1z] - sz_new[ip, k0z:k1z]))
            cum_z((0) + 1) = x_cb47((0) + 1)
            do x_cs0_461 = 1, ((k1z - k0z)) - 1
                cum_z((x_cs0_461) + 1) = (cum_z(((x_cs0_461 - 1)) + 1) + x_cb47((x_cs0_461) + 1))
            end do
            do si0_l462 = ((lox + ib) + i0z), (((lox + ib) + i1z)) - 1
                do si1_l463 = ((loy + jb) + j0z), (((loy + jb) + j1z)) - 1
                    do si2_l464 = ((loz + kb) + k0z), (((loz + kb) + k1z)) - 1
                        Jz((0) + 1, (si2_l464) + 1, (si1_l463) + 1, (si0_l462) + 1) = Jz((0) + 1, (si2_l464) + 1, &
                        &(si1_l463) + 1, (si0_l462) + 1) + ((gz(((si1_l463 + (0 - ((loy + jb) + j0z)))) + 1, &
                        &((si0_l462 + (0 - ((lox + ib) + i0z)))) + 1) * cum_z(((si2_l464 + (0 - ((loz + kb) + k0z)))) &
                        &+ 1)))
                    end do
                end do
            end do
        else if (((geom == 1) .OR. (geom == 2))) then
            i0 = INT(dil((ip_l443) + 1), c_int64_t)
            i1 = ((o + 2) - INT(diu((ip_l443) + 1), c_int64_t))
            k0 = INT(dkl((ip_l443) + 1), c_int64_t)
            k1 = ((o + 3) - INT(dku((ip_l443) + 1), c_int64_t))
            ib = (INT(i_new((ip_l443) + 1), c_int64_t) - 1)
            kb = (INT(k_new((ip_l443) + 1), c_int64_t) - 1)
            if (.not. allocated(x_cb49)) then
                allocate(x_cb49((i1 - i0)))
            else if (size(x_cb49, 1) /= ((i1 - i0))) then
                deallocate(x_cb49)
                allocate(x_cb49((i1 - i0)))
            end if
            if (.not. allocated(x_cb49)) then
                allocate(x_cb49((i1 - i0)))
            else if (size(x_cb49, 1) /= ((i1 - i0))) then
                deallocate(x_cb49)
                allocate(x_cb49((i1 - i0)))
            end if
            do x_w0_465 = 0, ((i1 - i0)) - 1
                x_cb49((x_w0_465) + 1) = ((wqi * invdtd_x) * (sx_old(((x_w0_465 + (i0 - 0))) + 1, (ip_l443) + 1) - &
                &sx_new(((x_w0_465 + (i0 - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_x)) then
                allocate(cum_x((i1 - i0)))
            else if (size(cum_x, 1) /= ((i1 - i0))) then
                deallocate(cum_x)
                allocate(cum_x((i1 - i0)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_x * (sx_old[ip, i0:i1] - sx_new[ip, i0:i1]))
            cum_x((0) + 1) = x_cb49((0) + 1)
            do x_cs0_466 = 1, ((i1 - i0)) - 1
                cum_x((x_cs0_466) + 1) = (cum_x(((x_cs0_466 - 1)) + 1) + x_cb49((x_cs0_466) + 1))
            end do
            if (.not. allocated(zavg_x)) then
                allocate(zavg_x((k1 - k0)))
            else if (size(zavg_x, 1) /= ((k1 - k0))) then
                deallocate(zavg_x)
                allocate(zavg_x((k1 - k0)))
            end if
            do x_w0_467 = 0, ((k1 - k0)) - 1
                zavg_x((x_w0_467) + 1) = (0.5_c_float * (sz_new(((x_w0_467 + (k0 - 0))) + 1, (ip_l443) + 1) + &
                &sz_old(((x_w0_467 + (k0 - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(sdxi)) then
                allocate(sdxi((k1 - k0), (i1 - i0)))
            else if (size(sdxi, 1) /= ((k1 - k0)) .or. size(sdxi, 2) /= ((i1 - i0))) then
                deallocate(sdxi)
                allocate(sdxi((k1 - k0), (i1 - i0)))
            end if
            do x_w0_468 = 0, ((i1 - i0)) - 1
                do x_w1_469 = 0, ((k1 - k0)) - 1
                    sdxi((x_w1_469) + 1, (x_w0_468) + 1) = (cum_x((x_w0_468) + 1) * zavg_x((x_w1_469) + 1))
                end do
            end do
            do si0_l470 = ((lox + ib) + i0), (((lox + ib) + i1)) - 1
                do si1_l471 = ((loy + kb) + k0), (((loy + kb) + k1)) - 1
                    Jx((0) + 1, (0) + 1, (si1_l471) + 1, (si0_l470) + 1) = Jx((0) + 1, (0) + 1, (si1_l471) + 1, &
                    &(si0_l470) + 1) + (sdxi(((si1_l471 - ((loy + kb) + k0))) + 1, ((si0_l470 - ((lox + ib) + i0))) + &
                    &1))
                end do
            end do
            if (rz_modes) then
                if (.not. allocated(djr)) then
                    allocate(djr((k1 - k0), (i1 - i0)))
                else if (size(djr, 1) /= ((k1 - k0)) .or. size(djr, 2) /= ((i1 - i0))) then
                    deallocate(djr)
                    allocate(djr((k1 - k0), (i1 - i0)))
                end if
                do x_w0_472 = 0, ((i1 - i0)) - 1
                    do x_w1_473 = 0, ((k1 - k0)) - 1
                        djr((x_w1_473) + 1, (x_w0_472) + 1) = (2.0_c_float * sdxi((x_w1_473) + 1, (x_w0_472) + 1))
                    end do
                end do
                xy_mid_re = xy_mid0_re((ip_l443) + 1)
                xy_mid_im = xy_mid0_im((ip_l443) + 1)
                do imode_l474 = 1, (n_modes) - 1
                    do si0_l475 = ((lox + ib) + i0), (((lox + ib) + i1)) - 1
                        do si1_l476 = ((loy + kb) + k0), (((loy + kb) + k1)) - 1
                            Jx((((2 * imode_l474) - 1)) + 1, (0) + 1, (si1_l476) + 1, (si0_l475) + 1) = Jx((((2 * &
                            &imode_l474) - 1)) + 1, (0) + 1, (si1_l476) + 1, (si0_l475) + 1) + ((djr(((si1_l476 - &
                            &((loy + kb) + k0))) + 1, ((si0_l475 - ((lox + ib) + i0))) + 1) * xy_mid_re))
                        end do
                    end do
                    do si0_l477 = ((lox + ib) + i0), (((lox + ib) + i1)) - 1
                        do si1_l478 = ((loy + kb) + k0), (((loy + kb) + k1)) - 1
                            Jx(((2 * imode_l474)) + 1, (0) + 1, (si1_l478) + 1, (si0_l477) + 1) = Jx(((2 * &
                            &imode_l474)) + 1, (0) + 1, (si1_l478) + 1, (si0_l477) + 1) + ((djr(((si1_l478 - ((loy + &
                            &kb) + k0))) + 1, ((si0_l477 - ((lox + ib) + i0))) + 1) * xy_mid_im))
                        end do
                    end do
                    nxt_mid_re = ((xy_mid_re * xy_mid0_re((ip_l443) + 1)) - (xy_mid_im * xy_mid0_im((ip_l443) + 1)))
                    nxt_mid_im = ((xy_mid_re * xy_mid0_im((ip_l443) + 1)) + (xy_mid_im * xy_mid0_re((ip_l443) + 1)))
                    xy_mid_re = nxt_mid_re
                    xy_mid_im = nxt_mid_im
                end do
            end if
            i0y = INT(dil((ip_l443) + 1), c_int64_t)
            i1y = ((o + 3) - INT(diu((ip_l443) + 1), c_int64_t))
            k0y = INT(dkl((ip_l443) + 1), c_int64_t)
            k1y = ((o + 3) - INT(dku((ip_l443) + 1), c_int64_t))
            if (.not. allocated(sdyj)) then
                allocate(sdyj((k1y - k0y), (i1y - i0y)))
            else if (size(sdyj, 1) /= ((k1y - k0y)) .or. size(sdyj, 2) /= ((i1y - i0y))) then
                deallocate(sdyj)
                allocate(sdyj((k1y - k0y), (i1y - i0y)))
            end if
            do x_w0_479 = 0, ((i1y - i0y)) - 1
                do x_w1_480 = 0, ((k1y - k0y)) - 1
                    sdyj((x_w1_480) + 1, (x_w0_479) + 1) = (((wqi * vy((ip_l443) + 1)) * invvol) * &
                    &((0.3333333333333333_c_float * ((sx_new(((i0y + x_w0_479)) + 1, (ip_l443) + 1) * sz_new(((k0y + &
                    &x_w1_480)) + 1, (ip_l443) + 1)) + (sx_old(((i0y + x_w0_479)) + 1, (ip_l443) + 1) * sz_old(((k0y + &
                    &x_w1_480)) + 1, (ip_l443) + 1)))) + (0.16666666666666666_c_float * ((sx_new(((i0y + x_w0_479)) + &
                    &1, (ip_l443) + 1) * sz_old(((k0y + x_w1_480)) + 1, (ip_l443) + 1)) + (sx_old(((i0y + x_w0_479)) + &
                    &1, (ip_l443) + 1) * sz_new(((k0y + x_w1_480)) + 1, (ip_l443) + 1))))))
                end do
            end do
            do si0_l481 = ((lox + ib) + i0y), (((lox + ib) + i1y)) - 1
                do si1_l482 = ((loy + kb) + k0y), (((loy + kb) + k1y)) - 1
                    Jy((0) + 1, (0) + 1, (si1_l482) + 1, (si0_l481) + 1) = Jy((0) + 1, (0) + 1, (si1_l482) + 1, &
                    &(si0_l481) + 1) + (sdyj(((si1_l482 - ((loy + kb) + k0y))) + 1, ((si0_l481 - ((lox + ib) + i0y))) &
                    &+ 1))
                end do
            end do
            if (rz_modes) then
                if (.not. allocated(a_re)) then
                    allocate(a_re((k1y - k0y), (i1y - i0y)))
                else if (size(a_re, 1) /= ((k1y - k0y)) .or. size(a_re, 2) /= ((i1y - i0y))) then
                    deallocate(a_re)
                    allocate(a_re((k1y - k0y), (i1y - i0y)))
                end if
                do x_w0_483 = 0, ((i1y - i0y)) - 1
                    do x_w1_484 = 0, ((k1y - k0y)) - 1
                        a_re((x_w1_484) + 1, (x_w0_483) + 1) = (sx_new(((i0y + x_w0_483)) + 1, (ip_l443) + 1) * &
                        &sz_new(((k0y + x_w1_484)) + 1, (ip_l443) + 1))
                    end do
                end do
                if (.not. allocated(b_re)) then
                    allocate(b_re((k1y - k0y), (i1y - i0y)))
                else if (size(b_re, 1) /= ((k1y - k0y)) .or. size(b_re, 2) /= ((i1y - i0y))) then
                    deallocate(b_re)
                    allocate(b_re((k1y - k0y), (i1y - i0y)))
                end if
                do x_w0_485 = 0, ((i1y - i0y)) - 1
                    do x_w1_486 = 0, ((k1y - k0y)) - 1
                        b_re((x_w1_486) + 1, (x_w0_485) + 1) = (sx_old(((i0y + x_w0_485)) + 1, (ip_l443) + 1) * &
                        &sz_old(((k0y + x_w1_486)) + 1, (ip_l443) + 1))
                    end do
                end do
                if (.not. allocated(x_cb51)) then
                    allocate(x_cb51((i1y - i0y)))
                else if (size(x_cb51, 1) /= ((i1y - i0y))) then
                    deallocate(x_cb51)
                    allocate(x_cb51((i1y - i0y)))
                end if
                if (.not. allocated(x_cb51)) then
                    allocate(x_cb51((i1y - i0y)))
                else if (size(x_cb51, 1) /= ((i1y - i0y))) then
                    deallocate(x_cb51)
                    allocate(x_cb51((i1y - i0y)))
                end if
                ! numpy: np.arange(i0y, i1y)
                do x_i_487 = 0, ((i1y - i0y)) - 1
                    x_cb51((x_i_487) + 1) = (i0y + x_i_487)
                end do
                if (.not. allocated(i_local)) then
                    allocate(i_local((i1y - i0y)))
                else if (size(i_local, 1) /= ((i1y - i0y))) then
                    deallocate(i_local)
                    allocate(i_local((i1y - i0y)))
                end if
                do x_w0_488 = 0, ((i1y - i0y)) - 1
                    i_local((x_w0_488) + 1) = (ib + x_cb51((x_w0_488) + 1))
                end do
                if (.not. allocated(neg2coef)) then
                    allocate(neg2coef((i1y - i0y)))
                else if (size(neg2coef, 1) /= ((i1y - i0y))) then
                    deallocate(neg2coef)
                    allocate(neg2coef((i1y - i0y)))
                end if
                do x_w0_489 = 0, ((i1y - i0y)) - 1
                    neg2coef((x_w0_489) + 1) = ((((-(2.0_c_float)) * (i_local((x_w0_489) + 1) + (xmin * dinvx))) * &
                    &wqi) * invdtd_x)
                end do
                xy_new_re = xy_new0_re((ip_l443) + 1)
                xy_new_im = xy_new0_im((ip_l443) + 1)
                xy_mid_re = xy_mid0_re((ip_l443) + 1)
                xy_mid_im = xy_mid0_im((ip_l443) + 1)
                xy_old_re = xy_old0_re((ip_l443) + 1)
                xy_old_im = xy_old0_im((ip_l443) + 1)
                do imode_l490 = 1, (n_modes) - 1
                    if (.not. allocated(sum_re)) then
                        allocate(sum_re((k1y - k0y), (i1y - i0y)))
                    else if (size(sum_re, 1) /= ((k1y - k0y)) .or. size(sum_re, 2) /= ((i1y - i0y))) then
                        deallocate(sum_re)
                        allocate(sum_re((k1y - k0y), (i1y - i0y)))
                    end if
                    do x_w0_491 = 0, ((i1y - i0y)) - 1
                        do x_w1_492 = 0, ((k1y - k0y)) - 1
                            sum_re((x_w1_492) + 1, (x_w0_491) + 1) = ((a_re((x_w1_492) + 1, (x_w0_491) + 1) * &
                            &(xy_new_re - xy_mid_re)) + (b_re((x_w1_492) + 1, (x_w0_491) + 1) * (xy_mid_re - &
                            &xy_old_re)))
                        end do
                    end do
                    if (.not. allocated(sum_im)) then
                        allocate(sum_im((k1y - k0y), (i1y - i0y)))
                    else if (size(sum_im, 1) /= ((k1y - k0y)) .or. size(sum_im, 2) /= ((i1y - i0y))) then
                        deallocate(sum_im)
                        allocate(sum_im((k1y - k0y), (i1y - i0y)))
                    end if
                    do x_w0_493 = 0, ((i1y - i0y)) - 1
                        do x_w1_494 = 0, ((k1y - k0y)) - 1
                            sum_im((x_w1_494) + 1, (x_w0_493) + 1) = ((a_re((x_w1_494) + 1, (x_w0_493) + 1) * &
                            &(xy_new_im - xy_mid_im)) + (b_re((x_w1_494) + 1, (x_w0_493) + 1) * (xy_mid_im - &
                            &xy_old_im)))
                        end do
                    end do
                    if (.not. allocated(coef_m)) then
                        allocate(coef_m((i1y - i0y)))
                    else if (size(coef_m, 1) /= ((i1y - i0y))) then
                        deallocate(coef_m)
                        allocate(coef_m((i1y - i0y)))
                    end if
                    do x_w0_495 = 0, ((i1y - i0y)) - 1
                        coef_m((x_w0_495) + 1) = (neg2coef((x_w0_495) + 1) / REAL(imode_l490, c_double))
                    end do
                    do si0_l496 = ((lox + ib) + i0y), (((lox + ib) + i1y)) - 1
                        do si1_l497 = ((loy + kb) + k0y), (((loy + kb) + k1y)) - 1
                            Jy((((2 * imode_l490) - 1)) + 1, (0) + 1, (si1_l497) + 1, (si0_l496) + 1) = Jy((((2 * &
                            &imode_l490) - 1)) + 1, (0) + 1, (si1_l497) + 1, (si0_l496) + 1) + ((coef_m(((si0_l496 + &
                            &(0 - ((lox + ib) + i0y)))) + 1) * (-(sum_im(((si1_l497 - ((loy + kb) + k0y))) + 1, &
                            &((si0_l496 - ((lox + ib) + i0y))) + 1)))))
                        end do
                    end do
                    do si0_l498 = ((lox + ib) + i0y), (((lox + ib) + i1y)) - 1
                        do si1_l499 = ((loy + kb) + k0y), (((loy + kb) + k1y)) - 1
                            Jy(((2 * imode_l490)) + 1, (0) + 1, (si1_l499) + 1, (si0_l498) + 1) = Jy(((2 * &
                            &imode_l490)) + 1, (0) + 1, (si1_l499) + 1, (si0_l498) + 1) + ((coef_m(((si0_l498 + (0 - &
                            &((lox + ib) + i0y)))) + 1) * sum_re(((si1_l499 - ((loy + kb) + k0y))) + 1, ((si0_l498 - &
                            &((lox + ib) + i0y))) + 1)))
                        end do
                    end do
                    nxt_new_re = ((xy_new_re * xy_new0_re((ip_l443) + 1)) - (xy_new_im * xy_new0_im((ip_l443) + 1)))
                    nxt_new_im = ((xy_new_re * xy_new0_im((ip_l443) + 1)) + (xy_new_im * xy_new0_re((ip_l443) + 1)))
                    nxt_mid_re = ((xy_mid_re * xy_mid0_re((ip_l443) + 1)) - (xy_mid_im * xy_mid0_im((ip_l443) + 1)))
                    nxt_mid_im = ((xy_mid_re * xy_mid0_im((ip_l443) + 1)) + (xy_mid_im * xy_mid0_re((ip_l443) + 1)))
                    nxt_old_re = ((xy_old_re * xy_old0_re((ip_l443) + 1)) - (xy_old_im * xy_old0_im((ip_l443) + 1)))
                    nxt_old_im = ((xy_old_re * xy_old0_im((ip_l443) + 1)) + (xy_old_im * xy_old0_re((ip_l443) + 1)))
                    xy_new_re = nxt_new_re
                    xy_new_im = nxt_new_im
                    xy_mid_re = nxt_mid_re
                    xy_mid_im = nxt_mid_im
                    xy_old_re = nxt_old_re
                    xy_old_im = nxt_old_im
                end do
            end if
            i0z = INT(dil((ip_l443) + 1), c_int64_t)
            i1z = ((o + 3) - INT(diu((ip_l443) + 1), c_int64_t))
            k0z = INT(dkl((ip_l443) + 1), c_int64_t)
            k1z = ((o + 2) - INT(dku((ip_l443) + 1), c_int64_t))
            if (.not. allocated(x_cb52)) then
                allocate(x_cb52((k1z - k0z)))
            else if (size(x_cb52, 1) /= ((k1z - k0z))) then
                deallocate(x_cb52)
                allocate(x_cb52((k1z - k0z)))
            end if
            if (.not. allocated(x_cb52)) then
                allocate(x_cb52((k1z - k0z)))
            else if (size(x_cb52, 1) /= ((k1z - k0z))) then
                deallocate(x_cb52)
                allocate(x_cb52((k1z - k0z)))
            end if
            do x_w0_500 = 0, ((k1z - k0z)) - 1
                x_cb52((x_w0_500) + 1) = ((wqi * invdtd_z) * (sz_old(((x_w0_500 + (k0z - 0))) + 1, (ip_l443) + 1) - &
                &sz_new(((x_w0_500 + (k0z - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_z)) then
                allocate(cum_z((k1z - k0z)))
            else if (size(cum_z, 1) /= ((k1z - k0z))) then
                deallocate(cum_z)
                allocate(cum_z((k1z - k0z)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_z * (sz_old[ip, k0z:k1z] - sz_new[ip, k0z:k1z]))
            cum_z((0) + 1) = x_cb52((0) + 1)
            do x_cs0_501 = 1, ((k1z - k0z)) - 1
                cum_z((x_cs0_501) + 1) = (cum_z(((x_cs0_501 - 1)) + 1) + x_cb52((x_cs0_501) + 1))
            end do
            if (.not. allocated(xavg_z)) then
                allocate(xavg_z((i1z - i0z)))
            else if (size(xavg_z, 1) /= ((i1z - i0z))) then
                deallocate(xavg_z)
                allocate(xavg_z((i1z - i0z)))
            end if
            do x_w0_502 = 0, ((i1z - i0z)) - 1
                xavg_z((x_w0_502) + 1) = (0.5_c_float * (sx_new(((x_w0_502 + (i0z - 0))) + 1, (ip_l443) + 1) + &
                &sx_old(((x_w0_502 + (i0z - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(sdzk)) then
                allocate(sdzk((k1z - k0z), (i1z - i0z)))
            else if (size(sdzk, 1) /= ((k1z - k0z)) .or. size(sdzk, 2) /= ((i1z - i0z))) then
                deallocate(sdzk)
                allocate(sdzk((k1z - k0z), (i1z - i0z)))
            end if
            do x_w0_503 = 0, ((i1z - i0z)) - 1
                do x_w1_504 = 0, ((k1z - k0z)) - 1
                    sdzk((x_w1_504) + 1, (x_w0_503) + 1) = (xavg_z((x_w0_503) + 1) * cum_z((x_w1_504) + 1))
                end do
            end do
            do si0_l505 = ((lox + ib) + i0z), (((lox + ib) + i1z)) - 1
                do si1_l506 = ((loy + kb) + k0z), (((loy + kb) + k1z)) - 1
                    Jz((0) + 1, (0) + 1, (si1_l506) + 1, (si0_l505) + 1) = Jz((0) + 1, (0) + 1, (si1_l506) + 1, &
                    &(si0_l505) + 1) + (sdzk(((si1_l506 - ((loy + kb) + k0z))) + 1, ((si0_l505 - ((lox + ib) + i0z))) &
                    &+ 1))
                end do
            end do
            if (rz_modes) then
                if (.not. allocated(djz)) then
                    allocate(djz((k1z - k0z), (i1z - i0z)))
                else if (size(djz, 1) /= ((k1z - k0z)) .or. size(djz, 2) /= ((i1z - i0z))) then
                    deallocate(djz)
                    allocate(djz((k1z - k0z), (i1z - i0z)))
                end if
                do x_w0_507 = 0, ((i1z - i0z)) - 1
                    do x_w1_508 = 0, ((k1z - k0z)) - 1
                        djz((x_w1_508) + 1, (x_w0_507) + 1) = (2.0_c_float * sdzk((x_w1_508) + 1, (x_w0_507) + 1))
                    end do
                end do
                xy_mid_re = xy_mid0_re((ip_l443) + 1)
                xy_mid_im = xy_mid0_im((ip_l443) + 1)
                do imode_l509 = 1, (n_modes) - 1
                    do si0_l510 = ((lox + ib) + i0z), (((lox + ib) + i1z)) - 1
                        do si1_l511 = ((loy + kb) + k0z), (((loy + kb) + k1z)) - 1
                            Jz((((2 * imode_l509) - 1)) + 1, (0) + 1, (si1_l511) + 1, (si0_l510) + 1) = Jz((((2 * &
                            &imode_l509) - 1)) + 1, (0) + 1, (si1_l511) + 1, (si0_l510) + 1) + ((djz(((si1_l511 - &
                            &((loy + kb) + k0z))) + 1, ((si0_l510 - ((lox + ib) + i0z))) + 1) * xy_mid_re))
                        end do
                    end do
                    do si0_l512 = ((lox + ib) + i0z), (((lox + ib) + i1z)) - 1
                        do si1_l513 = ((loy + kb) + k0z), (((loy + kb) + k1z)) - 1
                            Jz(((2 * imode_l509)) + 1, (0) + 1, (si1_l513) + 1, (si0_l512) + 1) = Jz(((2 * &
                            &imode_l509)) + 1, (0) + 1, (si1_l513) + 1, (si0_l512) + 1) + ((djz(((si1_l513 - ((loy + &
                            &kb) + k0z))) + 1, ((si0_l512 - ((lox + ib) + i0z))) + 1) * xy_mid_im))
                        end do
                    end do
                    nxt_mid_re = ((xy_mid_re * xy_mid0_re((ip_l443) + 1)) - (xy_mid_im * xy_mid0_im((ip_l443) + 1)))
                    nxt_mid_im = ((xy_mid_re * xy_mid0_im((ip_l443) + 1)) + (xy_mid_im * xy_mid0_re((ip_l443) + 1)))
                    xy_mid_re = nxt_mid_re
                    xy_mid_im = nxt_mid_im
                end do
            end if
        else if ((geom == 0)) then
            k0 = INT(dkl((ip_l443) + 1), c_int64_t)
            k1 = ((o + 3) - INT(dku((ip_l443) + 1), c_int64_t))
            kb = (INT(k_new((ip_l443) + 1), c_int64_t) - 1)
            if (.not. allocated(zavg)) then
                allocate(zavg((k1 - k0)))
            else if (size(zavg, 1) /= ((k1 - k0))) then
                deallocate(zavg)
                allocate(zavg((k1 - k0)))
            end if
            do x_w0_514 = 0, ((k1 - k0)) - 1
                zavg((x_w0_514) + 1) = (0.5_c_float * (sz_old(((x_w0_514 + (k0 - 0))) + 1, (ip_l443) + 1) + &
                &sz_new(((x_w0_514 + (k0 - 0))) + 1, (ip_l443) + 1)))
            end do
            do si0_l515 = ((lox + kb) + k0), (((lox + kb) + k1)) - 1
                Jx((0) + 1, (0) + 1, (0) + 1, (si0_l515) + 1) = Jx((0) + 1, (0) + 1, (0) + 1, (si0_l515) + 1) + &
                &((((wqi * vx((ip_l443) + 1)) * invvol) * zavg(((si0_l515 - ((lox + kb) + k0))) + 1)))
            end do
            do si0_l516 = ((lox + kb) + k0), (((lox + kb) + k1)) - 1
                Jy((0) + 1, (0) + 1, (0) + 1, (si0_l516) + 1) = Jy((0) + 1, (0) + 1, (0) + 1, (si0_l516) + 1) + &
                &((((wqi * vy((ip_l443) + 1)) * invvol) * zavg(((si0_l516 - ((lox + kb) + k0))) + 1)))
            end do
            k0z = INT(dkl((ip_l443) + 1), c_int64_t)
            k1z = ((o + 2) - INT(dku((ip_l443) + 1), c_int64_t))
            if (.not. allocated(x_cb54)) then
                allocate(x_cb54((k1z - k0z)))
            else if (size(x_cb54, 1) /= ((k1z - k0z))) then
                deallocate(x_cb54)
                allocate(x_cb54((k1z - k0z)))
            end if
            if (.not. allocated(x_cb54)) then
                allocate(x_cb54((k1z - k0z)))
            else if (size(x_cb54, 1) /= ((k1z - k0z))) then
                deallocate(x_cb54)
                allocate(x_cb54((k1z - k0z)))
            end if
            do x_w0_517 = 0, ((k1z - k0z)) - 1
                x_cb54((x_w0_517) + 1) = ((wqi * invdtd_z) * (sz_old(((x_w0_517 + (k0z - 0))) + 1, (ip_l443) + 1) - &
                &sz_new(((x_w0_517 + (k0z - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_z)) then
                allocate(cum_z((k1z - k0z)))
            else if (size(cum_z, 1) /= ((k1z - k0z))) then
                deallocate(cum_z)
                allocate(cum_z((k1z - k0z)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_z * (sz_old[ip, k0z:k1z] - sz_new[ip, k0z:k1z]))
            cum_z((0) + 1) = x_cb54((0) + 1)
            do x_cs0_518 = 1, ((k1z - k0z)) - 1
                cum_z((x_cs0_518) + 1) = (cum_z(((x_cs0_518 - 1)) + 1) + x_cb54((x_cs0_518) + 1))
            end do
            do si0_l519 = ((lox + kb) + k0z), (((lox + kb) + k1z)) - 1
                Jz((0) + 1, (0) + 1, (0) + 1, (si0_l519) + 1) = Jz((0) + 1, (0) + 1, (0) + 1, (si0_l519) + 1) + &
                &(cum_z(((si0_l519 - ((lox + kb) + k0z))) + 1))
            end do
        else
            i0x = INT(dil((ip_l443) + 1), c_int64_t)
            i1x = ((o + 2) - INT(diu((ip_l443) + 1), c_int64_t))
            ib = (INT(i_new((ip_l443) + 1), c_int64_t) - 1)
            if (.not. allocated(x_cb56)) then
                allocate(x_cb56((i1x - i0x)))
            else if (size(x_cb56, 1) /= ((i1x - i0x))) then
                deallocate(x_cb56)
                allocate(x_cb56((i1x - i0x)))
            end if
            if (.not. allocated(x_cb56)) then
                allocate(x_cb56((i1x - i0x)))
            else if (size(x_cb56, 1) /= ((i1x - i0x))) then
                deallocate(x_cb56)
                allocate(x_cb56((i1x - i0x)))
            end if
            do x_w0_520 = 0, ((i1x - i0x)) - 1
                x_cb56((x_w0_520) + 1) = ((wqi * invdtd_x) * (sx_old(((x_w0_520 + (i0x - 0))) + 1, (ip_l443) + 1) - &
                &sx_new(((x_w0_520 + (i0x - 0))) + 1, (ip_l443) + 1)))
            end do
            if (.not. allocated(cum_x__v1)) then
                allocate(cum_x__v1((i1x - i0x)))
            else if (size(cum_x__v1, 1) /= ((i1x - i0x))) then
                deallocate(cum_x__v1)
                allocate(cum_x__v1((i1x - i0x)))
            end if
            ! numpy: np.cumsum(wqi * invdtd_x * (sx_old[ip, i0x:i1x] - sx_new[ip, i0x:i1x]))
            cum_x__v1((0) + 1) = x_cb56((0) + 1)
            do x_cs0_521 = 1, ((i1x - i0x)) - 1
                cum_x__v1((x_cs0_521) + 1) = (cum_x__v1(((x_cs0_521 - 1)) + 1) + x_cb56((x_cs0_521) + 1))
            end do
            do si0_l522 = ((lox + ib) + i0x), (((lox + ib) + i1x)) - 1
                Jx((0) + 1, (0) + 1, (0) + 1, (si0_l522) + 1) = Jx((0) + 1, (0) + 1, (0) + 1, (si0_l522) + 1) + &
                &(cum_x__v1(((si0_l522 - ((lox + ib) + i0x))) + 1))
            end do
            i0 = INT(dil((ip_l443) + 1), c_int64_t)
            i1 = ((o + 3) - INT(diu((ip_l443) + 1), c_int64_t))
            if (.not. allocated(xavg)) then
                allocate(xavg((i1 - i0)))
            else if (size(xavg, 1) /= ((i1 - i0))) then
                deallocate(xavg)
                allocate(xavg((i1 - i0)))
            end if
            do x_w0_523 = 0, ((i1 - i0)) - 1
                xavg((x_w0_523) + 1) = (0.5_c_float * (sx_old(((x_w0_523 + (i0 - 0))) + 1, (ip_l443) + 1) + &
                &sx_new(((x_w0_523 + (i0 - 0))) + 1, (ip_l443) + 1)))
            end do
            do si0_l524 = ((lox + ib) + i0), (((lox + ib) + i1)) - 1
                Jy((0) + 1, (0) + 1, (0) + 1, (si0_l524) + 1) = Jy((0) + 1, (0) + 1, (0) + 1, (si0_l524) + 1) + &
                &((((wqi * vy((ip_l443) + 1)) * invvol) * xavg(((si0_l524 - ((lox + ib) + i0))) + 1)))
            end do
            do si0_l525 = ((lox + ib) + i0), (((lox + ib) + i1)) - 1
                Jz((0) + 1, (0) + 1, (0) + 1, (si0_l525) + 1) = Jz((0) + 1, (0) + 1, (0) + 1, (si0_l525) + 1) + &
                &((((wqi * vz((ip_l443) + 1)) * invvol) * xavg(((si0_l525 - ((lox + ib) + i0))) + 1)))
            end do
        end if
    end do
contains

    elemental function npb_floordiv_i(a, b) result(r)
        integer(c_int64_t), intent(in) :: a, b
        integer(c_int64_t) :: r
        r = a / b - merge(1_c_int64_t, 0_c_int64_t, (mod(a, b) /= 0_c_int64_t) .and. ((a < 0_c_int64_t) .neqv. (b < &
        &0_c_int64_t)))
    end function npb_floordiv_i

end subroutine warpx_esirkepov_deposition_fp32
