! hpcagent_bench-autogen -- generated from quatrex_rgf_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine quatrex_rgf_fp32(a_diag, a_lower, a_upper, sigma_greater_diag, sigma_greater_upper, sigma_lesser_diag, &
&sigma_lesser_upper, x_greater_diag, x_greater_lower, x_greater_upper, x_lesser_diag, x_lesser_lower, x_lesser_upper, &
&x_retarded_diag, BS, NB, NE) bind(C, name="quatrex_rgf_fp32")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), value, intent(in) :: BS
    integer(c_int64_t), value, intent(in) :: NB
    integer(c_int64_t), value, intent(in) :: NE
    complex(c_float_complex), intent(in) :: a_diag(BS, BS, NB, NE)
    complex(c_float_complex), intent(in) :: a_lower(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(in) :: a_upper(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(in) :: sigma_greater_diag(BS, BS, NB, NE)
    complex(c_float_complex), intent(in) :: sigma_greater_upper(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(in) :: sigma_lesser_diag(BS, BS, NB, NE)
    complex(c_float_complex), intent(in) :: sigma_lesser_upper(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(inout) :: x_greater_diag(BS, BS, NB, NE)
    complex(c_float_complex), intent(inout) :: x_greater_lower(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(inout) :: x_greater_upper(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(inout) :: x_lesser_diag(BS, BS, NB, NE)
    complex(c_float_complex), intent(inout) :: x_lesser_lower(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(inout) :: x_lesser_upper(BS, BS, (NB - 1), NE)
    complex(c_float_complex), intent(inout) :: x_retarded_diag(BS, BS, NB, NE)
    integer(c_int64_t) :: e_l0, i_l144, i_l37, si0_l1, si0_l102, si0_l104, si0_l116, si0_l126, si0_l134, si0_l145, &
    &si0_l15, si0_l154, si0_l156, si0_l165, si0_l167, si0_l176, si0_l178, si0_l187, si0_l198, si0_l200, si0_l206, &
    &si0_l211, si0_l222, si0_l226, si0_l237, si0_l241, si0_l258, si0_l260, si0_l266, si0_l271, si0_l282, si0_l286, &
    &si0_l297, si0_l301, si0_l312, si0_l38, si0_l47, si0_l52, si0_l66, si0_l75, si0_l77, si0_l89, si1_l103, si1_l105, &
    &si1_l117, si1_l124, si1_l127, si1_l13, si1_l135, si1_l146, si1_l155, si1_l157, si1_l16, si1_l166, si1_l168, &
    &si1_l177, si1_l179, si1_l188, si1_l199, si1_l2, si1_l201, si1_l207, si1_l212, si1_l223, si1_l227, si1_l238, &
    &si1_l239, si1_l242, si1_l259, si1_l261, si1_l267, si1_l27, si1_l272, si1_l283, si1_l287, si1_l298, si1_l299, &
    &si1_l302, si1_l313, si1_l314, si1_l35, si1_l39, si1_l48, si1_l53, si1_l64, si1_l67, si1_l76, si1_l78, si1_l90, &
    &si1_l97, si2_l125, si2_l132, si2_l14, si2_l140, si2_l142, si2_l224, si2_l232, si2_l240, si2_l247, si2_l28, &
    &si2_l284, si2_l292, si2_l300, si2_l307, si2_l315, si2_l316, si2_l36, si2_l65, si2_l98, si3_l133, si3_l141, &
    &si3_l143, si3_l225, si3_l233, si3_l248, si3_l285, si3_l293, si3_l308, si3_l317, x_i_113, x_i_118, x_i_121, &
    &x_i_184, x_i_195, x_i_219, x_i_234, x_i_24, x_i_255, x_i_279, x_i_294, x_i_32, x_i_86, x_i_91, x_i_94, &
    &x_inv_c_10, x_inv_c_58, x_inv_c_59, x_inv_c_61, x_inv_c_7, x_inv_c_8, x_inv_i_3, x_inv_i_54, x_inv_j_4, &
    &x_inv_j_55, x_inv_k_5, x_inv_k_56, x_inv_r_57, x_inv_r_6, x_inv_r_60, x_inv_r_9, x_j_114, x_j_119, x_j_122, &
    &x_j_185, x_j_196, x_j_220, x_j_235, x_j_25, x_j_256, x_j_280, x_j_295, x_j_33, x_j_87, x_j_92, x_j_95, x_l_115, &
    &x_l_120, x_l_123, x_l_186, x_l_197, x_l_221, x_l_236, x_l_257, x_l_26, x_l_281, x_l_296, x_l_34, x_l_88, x_l_93, &
    &x_l_96, x_mmi12_72, x_mmi14_83, x_mmi18_99, x_mmi20_110, x_mmi27_151, x_mmi29_162, x_mmi31_173, x_mmi34_189, &
    &x_mmi35_192, x_mmi38_208, x_mmi39_213, x_mmi3_21, x_mmi40_216, x_mmi45_249, x_mmi46_252, x_mmi49_268, &
    &x_mmi50_273, x_mmi51_276, x_mmi56_309, x_mmi5_29, x_mmi8_44, x_mmi9_49, x_mmj12_73, x_mmj14_84, x_mmj18_100, &
    &x_mmj20_111, x_mmj27_152, x_mmj29_163, x_mmj31_174, x_mmj34_190, x_mmj35_193, x_mmj38_209, x_mmj39_214, &
    &x_mmj3_22, x_mmj40_217, x_mmj45_250, x_mmj46_253, x_mmj49_269, x_mmj50_274, x_mmj51_277, x_mmj56_310, x_mmj5_30, &
    &x_mmj8_45, x_mmj9_50, x_mml12_74, x_mml14_85, x_mml18_101, x_mml20_112, x_mml27_153, x_mml29_164, x_mml31_175, &
    &x_mml34_191, x_mml35_194, x_mml38_210, x_mml39_215, x_mml3_23, x_mml40_218, x_mml45_251, x_mml46_254, &
    &x_mml49_270, x_mml50_275, x_mml51_278, x_mml56_311, x_mml5_31, x_mml8_46, x_mml9_51, x_t0_106, x_t0_128, &
    &x_t0_136, x_t0_147, x_t0_158, x_t0_169, x_t0_17, x_t0_180, x_t0_202, x_t0_228, x_t0_243, x_t0_262, x_t0_288, &
    &x_t0_303, x_t0_40, x_t0_68, x_t0_79, x_t1_107, x_t1_129, x_t1_137, x_t1_148, x_t1_159, x_t1_170, x_t1_18, &
    &x_t1_181, x_t1_203, x_t1_229, x_t1_244, x_t1_263, x_t1_289, x_t1_304, x_t1_41, x_t1_69, x_t1_80, x_w0_108, &
    &x_w0_11, x_w0_130, x_w0_138, x_w0_149, x_w0_160, x_w0_171, x_w0_182, x_w0_19, x_w0_204, x_w0_230, x_w0_245, &
    &x_w0_264, x_w0_290, x_w0_305, x_w0_42, x_w0_62, x_w0_70, x_w0_81, x_w1_109, x_w1_12, x_w1_131, x_w1_139, &
    &x_w1_150, x_w1_161, x_w1_172, x_w1_183, x_w1_20, x_w1_205, x_w1_231, x_w1_246, x_w1_265, x_w1_291, x_w1_306, &
    &x_w1_43, x_w1_63, x_w1_71, x_w1_82
    integer(c_int64_t) :: x_inv_p
    complex(c_float_complex) :: x_inv_factor
    integer(c_int64_t) :: j
    complex(c_float_complex) :: x_inv_tmp
    real(c_float) :: x_ifexp0
    real(c_float) :: x_ifexp1
    complex(c_float_complex) :: xr_d(BS, BS, NB)
    complex(c_float_complex) :: xl_d(BS, BS, NB)
    complex(c_float_complex) :: xg_d(BS, BS, NB)
    complex(c_float_complex) :: m(BS, BS)
    complex(c_float_complex) :: dag(BS, BS)
    complex(c_float_complex) :: a_ji_dag(BS, BS)
    complex(c_float_complex) :: xr_jj_dag(BS, BS)
    complex(c_float_complex) :: t1(BS, BS)
    complex(c_float_complex) :: t2(BS, BS)
    complex(c_float_complex) :: t3(BS, BS)
    complex(c_float_complex) :: xr_ii_a_ij(BS, BS)
    complex(c_float_complex) :: xr_jj_a_ji(BS, BS)
    complex(c_float_complex) :: xr_ii_a_ij_xr_jj(BS, BS)
    complex(c_float_complex) :: xr_ii_a_ij_xr_jj_a_ji(BS, BS)
    complex(c_float_complex) :: a_ij_dag_xr_ii_dag(BS, BS)
    complex(c_float_complex) :: a_ji_dag_xr_jj_dag(BS, BS)
    complex(c_float_complex) :: xr_jj_dag_a_ij_dag_xr_ii_dag(BS, BS)
    complex(c_float_complex) :: temp_1x(BS, BS)
    complex(c_float_complex) :: temp_2x(BS, BS)
    complex(c_float_complex) :: cj(BS, BS)
    complex(c_float_complex) :: x_cb1(BS, BS)
    complex(c_float_complex) :: x_cb2(BS, BS)
    complex(c_float_complex) :: x_mm3(BS, BS)
    complex(c_float_complex) :: x_mm4(BS, BS)
    complex(c_float_complex) :: x_mm5(BS, BS)
    complex(c_float_complex) :: x_mm6(BS, BS)
    complex(c_float_complex) :: x_cb7(BS, BS)
    complex(c_float_complex) :: x_mm8(BS, BS)
    complex(c_float_complex) :: x_mm9(BS, BS)
    complex(c_float_complex) :: x_cb10(BS, BS)
    complex(c_float_complex) :: x_cb11(BS, BS)
    complex(c_float_complex) :: x_mm12(BS, BS)
    complex(c_float_complex) :: x_cb13(BS, BS)
    complex(c_float_complex) :: x_mm14(BS, BS)
    complex(c_float_complex) :: x_mm15(BS, BS)
    complex(c_float_complex) :: x_mm16(BS, BS)
    complex(c_float_complex) :: x_mm17(BS, BS)
    complex(c_float_complex) :: x_mm18(BS, BS)
    complex(c_float_complex) :: x_cb19(BS, BS)
    complex(c_float_complex) :: x_mm20(BS, BS)
    complex(c_float_complex) :: x_mm21(BS, BS)
    complex(c_float_complex) :: x_mm22(BS, BS)
    complex(c_float_complex) :: x_mm23(BS, BS)
    complex(c_float_complex) :: x_cb24(BS, BS)
    complex(c_float_complex) :: x_cb25(BS, BS)
    complex(c_float_complex) :: x_cb26(BS, BS)
    complex(c_float_complex) :: x_mm27(BS, BS)
    complex(c_float_complex) :: x_cb28(BS, BS)
    complex(c_float_complex) :: x_mm29(BS, BS)
    complex(c_float_complex) :: x_cb30(BS, BS)
    complex(c_float_complex) :: x_mm31(BS, BS)
    complex(c_float_complex) :: x_cb32(BS, BS)
    complex(c_float_complex) :: x_mm33(BS, BS)
    complex(c_float_complex) :: x_mm34(BS, BS)
    complex(c_float_complex) :: x_mm35(BS, BS)
    complex(c_float_complex) :: x_mm36(BS, BS)
    complex(c_float_complex) :: x_cb37(BS, BS)
    complex(c_float_complex) :: x_mm38(BS, BS)
    complex(c_float_complex) :: x_mm39(BS, BS)
    complex(c_float_complex) :: x_mm40(BS, BS)
    complex(c_float_complex) :: x_mm41(BS, BS)
    complex(c_float_complex) :: x_cb42(BS, BS)
    complex(c_float_complex) :: x_mm43(BS, BS)
    complex(c_float_complex) :: x_cb44(BS, BS)
    complex(c_float_complex) :: x_mm45(BS, BS)
    complex(c_float_complex) :: x_mm46(BS, BS)
    complex(c_float_complex) :: x_mm47(BS, BS)
    complex(c_float_complex) :: x_cb48(BS, BS)
    complex(c_float_complex) :: x_mm49(BS, BS)
    complex(c_float_complex) :: x_mm50(BS, BS)
    complex(c_float_complex) :: x_mm51(BS, BS)
    complex(c_float_complex) :: x_mm52(BS, BS)
    complex(c_float_complex) :: x_cb53(BS, BS)
    complex(c_float_complex) :: x_mm54(BS, BS)
    complex(c_float_complex) :: x_cb55(BS, BS)
    complex(c_float_complex) :: x_mm56(BS, BS)
    complex(c_float_complex) :: x_inv_aw0(BS, BS)
    complex(c_float_complex) :: x_inv_aw1(BS, BS)
    complex(c_float_complex) :: xr(BS, BS)
    xr_d = 0
    xl_d = 0
    xg_d = 0
    m = 0
    dag = 0
    a_ji_dag = 0
    xr_jj_dag = 0
    t1 = 0
    t2 = 0
    t3 = 0
    xr_ii_a_ij = 0
    xr_jj_a_ji = 0
    xr_ii_a_ij_xr_jj = 0
    xr_ii_a_ij_xr_jj_a_ji = 0
    a_ij_dag_xr_ii_dag = 0
    a_ji_dag_xr_jj_dag = 0
    xr_jj_dag_a_ij_dag_xr_ii_dag = 0
    temp_1x = 0
    temp_2x = 0
    cj = 0
    do e_l0 = 0, (NE) - 1
        do si0_l1 = 0, (BS) - 1
            do si1_l2 = 0, (BS) - 1
                m((si1_l2) + 1, (si0_l1) + 1) = a_diag((si1_l2) + 1, (si0_l1) + 1, (0) + 1, (e_l0) + 1)
            end do
        end do
        ! numpy: np.linalg.inv(m)
        do x_inv_i_3 = 0, (BS) - 1
            do x_inv_j_4 = 0, (BS) - 1
                x_inv_aw0((x_inv_j_4) + 1, (x_inv_i_3) + 1) = m((x_inv_j_4) + 1, (x_inv_i_3) + 1)
                if ((x_inv_i_3 == x_inv_j_4)) then
                    x_ifexp0 = 1.0_c_float
                else
                    x_ifexp0 = 0.0_c_float
                end if
                x_cb1((x_inv_j_4) + 1, (x_inv_i_3) + 1) = x_ifexp0
            end do
        end do
        do x_inv_k_5 = 0, (BS) - 1
            x_inv_p = x_inv_k_5
            do x_inv_r_6 = (x_inv_k_5 + 1), (BS) - 1
                if ((ABS(x_inv_aw0((x_inv_k_5) + 1, (x_inv_r_6) + 1)) > ABS(x_inv_aw0((x_inv_k_5) + 1, (x_inv_p) + &
                &1)))) then
                    x_inv_p = x_inv_r_6
                end if
            end do
            do x_inv_c_7 = 0, (BS) - 1
                x_inv_tmp = x_inv_aw0((x_inv_c_7) + 1, (x_inv_k_5) + 1)
                x_inv_aw0((x_inv_c_7) + 1, (x_inv_k_5) + 1) = x_inv_aw0((x_inv_c_7) + 1, (x_inv_p) + 1)
                x_inv_aw0((x_inv_c_7) + 1, (x_inv_p) + 1) = x_inv_tmp
                x_inv_tmp = x_cb1((x_inv_c_7) + 1, (x_inv_k_5) + 1)
                x_cb1((x_inv_c_7) + 1, (x_inv_k_5) + 1) = x_cb1((x_inv_c_7) + 1, (x_inv_p) + 1)
                x_cb1((x_inv_c_7) + 1, (x_inv_p) + 1) = x_inv_tmp
            end do
            x_inv_factor = x_inv_aw0((x_inv_k_5) + 1, (x_inv_k_5) + 1)
            do x_inv_c_8 = 0, (BS) - 1
                x_cb1((x_inv_c_8) + 1, (x_inv_k_5) + 1) = (x_cb1((x_inv_c_8) + 1, (x_inv_k_5) + 1) / x_inv_factor)
                x_inv_aw0((x_inv_c_8) + 1, (x_inv_k_5) + 1) = (x_inv_aw0((x_inv_c_8) + 1, (x_inv_k_5) + 1) / &
                &x_inv_factor)
            end do
            do x_inv_r_9 = 0, (BS) - 1
                if ((x_inv_r_9 /= x_inv_k_5)) then
                    x_inv_factor = x_inv_aw0((x_inv_k_5) + 1, (x_inv_r_9) + 1)
                    do x_inv_c_10 = 0, (BS) - 1
                        x_cb1((x_inv_c_10) + 1, (x_inv_r_9) + 1) = (x_cb1((x_inv_c_10) + 1, (x_inv_r_9) + 1) - &
                        &(x_inv_factor * x_cb1((x_inv_c_10) + 1, (x_inv_k_5) + 1)))
                        x_inv_aw0((x_inv_c_10) + 1, (x_inv_r_9) + 1) = (x_inv_aw0((x_inv_c_10) + 1, (x_inv_r_9) + 1) - &
                        &(x_inv_factor * x_inv_aw0((x_inv_c_10) + 1, (x_inv_k_5) + 1)))
                    end do
                end if
            end do
        end do
        do x_w0_11 = 0, (BS) - 1
            do x_w1_12 = 0, (BS) - 1
                xr((x_w1_12) + 1, (x_w0_11) + 1) = x_cb1((x_w1_12) + 1, (x_w0_11) + 1)
            end do
        end do
        do si1_l13 = 0, (BS) - 1
            do si2_l14 = 0, (BS) - 1
                xr_d((si2_l14) + 1, (si1_l13) + 1, (0) + 1) = xr((si2_l14) + 1, (si1_l13) + 1)
            end do
        end do
        do si0_l15 = 0, (BS) - 1
            do si1_l16 = 0, (BS) - 1
                cj((si1_l16) + 1, (si0_l15) + 1) = CONJG(xr((si1_l16) + 1, (si0_l15) + 1))
            end do
        end do
        ! numpy: np.transpose(cj)
        do x_t0_17 = 0, (BS) - 1
            do x_t1_18 = 0, (BS) - 1
                x_cb2((x_t0_17) + 1, (x_t1_18) + 1) = cj((x_t1_18) + 1, (x_t0_17) + 1)
            end do
        end do
        do x_w0_19 = 0, (BS) - 1
            do x_w1_20 = 0, (BS) - 1
                xr_jj_dag((x_w1_20) + 1, (x_w0_19) + 1) = x_cb2((x_w1_20) + 1, (x_w0_19) + 1)
            end do
        end do
        do x_mmi3_21 = 0, (BS) - 1
            do x_mmj3_22 = 0, (BS) - 1
                x_mm3((x_mmj3_22) + 1, (x_mmi3_21) + 1) = 0.0_c_float
                do x_mml3_23 = 0, (BS) - 1
                    x_mm3((x_mmj3_22) + 1, (x_mmi3_21) + 1) = x_mm3((x_mmj3_22) + 1, (x_mmi3_21) + 1) + &
                    &((xr((x_mml3_23) + 1, (x_mmi3_21) + 1) * sigma_lesser_diag((x_mmj3_22) + 1, (x_mml3_23) + 1, (0) &
                    &+ 1, (e_l0) + 1)))
                end do
            end do
        end do
        do x_i_24 = 0, (BS) - 1
            do x_j_25 = 0, (BS) - 1
                x_mm4((x_j_25) + 1, (x_i_24) + 1) = 0.0_c_float
                do x_l_26 = 0, (BS) - 1
                    x_mm4((x_j_25) + 1, (x_i_24) + 1) = x_mm4((x_j_25) + 1, (x_i_24) + 1) + ((x_mm3((x_l_26) + 1, &
                    &(x_i_24) + 1) * xr_jj_dag((x_j_25) + 1, (x_l_26) + 1)))
                end do
            end do
        end do
        do si1_l27 = 0, (BS) - 1
            do si2_l28 = 0, (BS) - 1
                xl_d((si2_l28) + 1, (si1_l27) + 1, (0) + 1) = x_mm4((si2_l28) + 1, (si1_l27) + 1)
            end do
        end do
        do x_mmi5_29 = 0, (BS) - 1
            do x_mmj5_30 = 0, (BS) - 1
                x_mm5((x_mmj5_30) + 1, (x_mmi5_29) + 1) = 0.0_c_float
                do x_mml5_31 = 0, (BS) - 1
                    x_mm5((x_mmj5_30) + 1, (x_mmi5_29) + 1) = x_mm5((x_mmj5_30) + 1, (x_mmi5_29) + 1) + &
                    &((xr((x_mml5_31) + 1, (x_mmi5_29) + 1) * sigma_greater_diag((x_mmj5_30) + 1, (x_mml5_31) + 1, (0) &
                    &+ 1, (e_l0) + 1)))
                end do
            end do
        end do
        do x_i_32 = 0, (BS) - 1
            do x_j_33 = 0, (BS) - 1
                x_mm6((x_j_33) + 1, (x_i_32) + 1) = 0.0_c_float
                do x_l_34 = 0, (BS) - 1
                    x_mm6((x_j_33) + 1, (x_i_32) + 1) = x_mm6((x_j_33) + 1, (x_i_32) + 1) + ((x_mm5((x_l_34) + 1, &
                    &(x_i_32) + 1) * xr_jj_dag((x_j_33) + 1, (x_l_34) + 1)))
                end do
            end do
        end do
        do si1_l35 = 0, (BS) - 1
            do si2_l36 = 0, (BS) - 1
                xg_d((si2_l36) + 1, (si1_l35) + 1, (0) + 1) = x_mm6((si2_l36) + 1, (si1_l35) + 1)
            end do
        end do
        do i_l37 = 0, ((NB - 1)) - 1
            j = (i_l37 + 1)
            do si0_l38 = 0, (BS) - 1
                do si1_l39 = 0, (BS) - 1
                    cj((si1_l39) + 1, (si0_l38) + 1) = CONJG(a_lower((si1_l39) + 1, (si0_l38) + 1, (i_l37) + 1, (e_l0) &
                    &+ 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_40 = 0, (BS) - 1
                do x_t1_41 = 0, (BS) - 1
                    x_cb7((x_t0_40) + 1, (x_t1_41) + 1) = cj((x_t1_41) + 1, (x_t0_40) + 1)
                end do
            end do
            do x_w0_42 = 0, (BS) - 1
                do x_w1_43 = 0, (BS) - 1
                    a_ji_dag((x_w1_43) + 1, (x_w0_42) + 1) = x_cb7((x_w1_43) + 1, (x_w0_42) + 1)
                end do
            end do
            do x_mmi8_44 = 0, (BS) - 1
                do x_mmj8_45 = 0, (BS) - 1
                    x_mm8((x_mmj8_45) + 1, (x_mmi8_44) + 1) = 0.0_c_float
                    do x_mml8_46 = 0, (BS) - 1
                        x_mm8((x_mmj8_45) + 1, (x_mmi8_44) + 1) = x_mm8((x_mmj8_45) + 1, (x_mmi8_44) + 1) + &
                        &((a_lower((x_mml8_46) + 1, (x_mmi8_44) + 1, (i_l37) + 1, (e_l0) + 1) * xr_d((x_mmj8_45) + 1, &
                        &(x_mml8_46) + 1, (i_l37) + 1)))
                    end do
                end do
            end do
            do si0_l47 = 0, (BS) - 1
                do si1_l48 = 0, (BS) - 1
                    t1((si1_l48) + 1, (si0_l47) + 1) = x_mm8((si1_l48) + 1, (si0_l47) + 1)
                end do
            end do
            do x_mmi9_49 = 0, (BS) - 1
                do x_mmj9_50 = 0, (BS) - 1
                    x_mm9((x_mmj9_50) + 1, (x_mmi9_49) + 1) = 0.0_c_float
                    do x_mml9_51 = 0, (BS) - 1
                        x_mm9((x_mmj9_50) + 1, (x_mmi9_49) + 1) = x_mm9((x_mmj9_50) + 1, (x_mmi9_49) + 1) + &
                        &((t1((x_mml9_51) + 1, (x_mmi9_49) + 1) * a_upper((x_mmj9_50) + 1, (x_mml9_51) + 1, (i_l37) + &
                        &1, (e_l0) + 1)))
                    end do
                end do
            end do
            do si0_l52 = 0, (BS) - 1
                do si1_l53 = 0, (BS) - 1
                    m((si1_l53) + 1, (si0_l52) + 1) = (a_diag((si1_l53) + 1, (si0_l52) + 1, (j) + 1, (e_l0) + 1) - &
                    &x_mm9((si1_l53) + 1, (si0_l52) + 1))
                end do
            end do
            ! numpy: np.linalg.inv(m)
            do x_inv_i_54 = 0, (BS) - 1
                do x_inv_j_55 = 0, (BS) - 1
                    x_inv_aw1((x_inv_j_55) + 1, (x_inv_i_54) + 1) = m((x_inv_j_55) + 1, (x_inv_i_54) + 1)
                    if ((x_inv_i_54 == x_inv_j_55)) then
                        x_ifexp1 = 1.0_c_float
                    else
                        x_ifexp1 = 0.0_c_float
                    end if
                    x_cb10((x_inv_j_55) + 1, (x_inv_i_54) + 1) = x_ifexp1
                end do
            end do
            do x_inv_k_56 = 0, (BS) - 1
                x_inv_p = x_inv_k_56
                do x_inv_r_57 = (x_inv_k_56 + 1), (BS) - 1
                    if ((ABS(x_inv_aw1((x_inv_k_56) + 1, (x_inv_r_57) + 1)) > ABS(x_inv_aw1((x_inv_k_56) + 1, &
                    &(x_inv_p) + 1)))) then
                        x_inv_p = x_inv_r_57
                    end if
                end do
                do x_inv_c_58 = 0, (BS) - 1
                    x_inv_tmp = x_inv_aw1((x_inv_c_58) + 1, (x_inv_k_56) + 1)
                    x_inv_aw1((x_inv_c_58) + 1, (x_inv_k_56) + 1) = x_inv_aw1((x_inv_c_58) + 1, (x_inv_p) + 1)
                    x_inv_aw1((x_inv_c_58) + 1, (x_inv_p) + 1) = x_inv_tmp
                    x_inv_tmp = x_cb10((x_inv_c_58) + 1, (x_inv_k_56) + 1)
                    x_cb10((x_inv_c_58) + 1, (x_inv_k_56) + 1) = x_cb10((x_inv_c_58) + 1, (x_inv_p) + 1)
                    x_cb10((x_inv_c_58) + 1, (x_inv_p) + 1) = x_inv_tmp
                end do
                x_inv_factor = x_inv_aw1((x_inv_k_56) + 1, (x_inv_k_56) + 1)
                do x_inv_c_59 = 0, (BS) - 1
                    x_cb10((x_inv_c_59) + 1, (x_inv_k_56) + 1) = (x_cb10((x_inv_c_59) + 1, (x_inv_k_56) + 1) / &
                    &x_inv_factor)
                    x_inv_aw1((x_inv_c_59) + 1, (x_inv_k_56) + 1) = (x_inv_aw1((x_inv_c_59) + 1, (x_inv_k_56) + 1) / &
                    &x_inv_factor)
                end do
                do x_inv_r_60 = 0, (BS) - 1
                    if ((x_inv_r_60 /= x_inv_k_56)) then
                        x_inv_factor = x_inv_aw1((x_inv_k_56) + 1, (x_inv_r_60) + 1)
                        do x_inv_c_61 = 0, (BS) - 1
                            x_cb10((x_inv_c_61) + 1, (x_inv_r_60) + 1) = (x_cb10((x_inv_c_61) + 1, (x_inv_r_60) + 1) - &
                            &(x_inv_factor * x_cb10((x_inv_c_61) + 1, (x_inv_k_56) + 1)))
                            x_inv_aw1((x_inv_c_61) + 1, (x_inv_r_60) + 1) = (x_inv_aw1((x_inv_c_61) + 1, (x_inv_r_60) &
                            &+ 1) - (x_inv_factor * x_inv_aw1((x_inv_c_61) + 1, (x_inv_k_56) + 1)))
                        end do
                    end if
                end do
            end do
            do x_w0_62 = 0, (BS) - 1
                do x_w1_63 = 0, (BS) - 1
                    xr((x_w1_63) + 1, (x_w0_62) + 1) = x_cb10((x_w1_63) + 1, (x_w0_62) + 1)
                end do
            end do
            do si1_l64 = 0, (BS) - 1
                do si2_l65 = 0, (BS) - 1
                    xr_d((si2_l65) + 1, (si1_l64) + 1, (j) + 1) = xr((si2_l65) + 1, (si1_l64) + 1)
                end do
            end do
            do si0_l66 = 0, (BS) - 1
                do si1_l67 = 0, (BS) - 1
                    cj((si1_l67) + 1, (si0_l66) + 1) = CONJG(xr((si1_l67) + 1, (si0_l66) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_68 = 0, (BS) - 1
                do x_t1_69 = 0, (BS) - 1
                    x_cb11((x_t0_68) + 1, (x_t1_69) + 1) = cj((x_t1_69) + 1, (x_t0_68) + 1)
                end do
            end do
            do x_w0_70 = 0, (BS) - 1
                do x_w1_71 = 0, (BS) - 1
                    xr_jj_dag((x_w1_71) + 1, (x_w0_70) + 1) = x_cb11((x_w1_71) + 1, (x_w0_70) + 1)
                end do
            end do
            do x_mmi12_72 = 0, (BS) - 1
                do x_mmj12_73 = 0, (BS) - 1
                    x_mm12((x_mmj12_73) + 1, (x_mmi12_72) + 1) = 0.0_c_float
                    do x_mml12_74 = 0, (BS) - 1
                        x_mm12((x_mmj12_73) + 1, (x_mmi12_72) + 1) = x_mm12((x_mmj12_73) + 1, (x_mmi12_72) + 1) + &
                        &((t1((x_mml12_74) + 1, (x_mmi12_72) + 1) * sigma_lesser_upper((x_mmj12_73) + 1, (x_mml12_74) &
                        &+ 1, (i_l37) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do si0_l75 = 0, (BS) - 1
                do si1_l76 = 0, (BS) - 1
                    t2((si1_l76) + 1, (si0_l75) + 1) = x_mm12((si1_l76) + 1, (si0_l75) + 1)
                end do
            end do
            do si0_l77 = 0, (BS) - 1
                do si1_l78 = 0, (BS) - 1
                    cj((si1_l78) + 1, (si0_l77) + 1) = CONJG(t2((si1_l78) + 1, (si0_l77) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_79 = 0, (BS) - 1
                do x_t1_80 = 0, (BS) - 1
                    x_cb13((x_t0_79) + 1, (x_t1_80) + 1) = cj((x_t1_80) + 1, (x_t0_79) + 1)
                end do
            end do
            do x_w0_81 = 0, (BS) - 1
                do x_w1_82 = 0, (BS) - 1
                    dag((x_w1_82) + 1, (x_w0_81) + 1) = x_cb13((x_w1_82) + 1, (x_w0_81) + 1)
                end do
            end do
            do x_mmi14_83 = 0, (BS) - 1
                do x_mmj14_84 = 0, (BS) - 1
                    x_mm14((x_mmj14_84) + 1, (x_mmi14_83) + 1) = 0.0_c_float
                    do x_mml14_85 = 0, (BS) - 1
                        x_mm14((x_mmj14_84) + 1, (x_mmi14_83) + 1) = x_mm14((x_mmj14_84) + 1, (x_mmi14_83) + 1) + &
                        &((a_lower((x_mml14_85) + 1, (x_mmi14_83) + 1, (i_l37) + 1, (e_l0) + 1) * xl_d((x_mmj14_84) + &
                        &1, (x_mml14_85) + 1, (i_l37) + 1)))
                    end do
                end do
            end do
            do x_i_86 = 0, (BS) - 1
                do x_j_87 = 0, (BS) - 1
                    x_mm15((x_j_87) + 1, (x_i_86) + 1) = 0.0_c_float
                    do x_l_88 = 0, (BS) - 1
                        x_mm15((x_j_87) + 1, (x_i_86) + 1) = x_mm15((x_j_87) + 1, (x_i_86) + 1) + ((x_mm14((x_l_88) + &
                        &1, (x_i_86) + 1) * a_ji_dag((x_j_87) + 1, (x_l_88) + 1)))
                    end do
                end do
            end do
            do si0_l89 = 0, (BS) - 1
                do si1_l90 = 0, (BS) - 1
                    t3((si1_l90) + 1, (si0_l89) + 1) = (((sigma_lesser_diag((si1_l90) + 1, (si0_l89) + 1, (j) + 1, &
                    &(e_l0) + 1) + x_mm15((si1_l90) + 1, (si0_l89) + 1)) + dag((si1_l90) + 1, (si0_l89) + 1)) - &
                    &t2((si1_l90) + 1, (si0_l89) + 1))
                end do
            end do
            do x_i_91 = 0, (BS) - 1
                do x_j_92 = 0, (BS) - 1
                    x_mm16((x_j_92) + 1, (x_i_91) + 1) = 0.0_c_float
                    do x_l_93 = 0, (BS) - 1
                        x_mm16((x_j_92) + 1, (x_i_91) + 1) = x_mm16((x_j_92) + 1, (x_i_91) + 1) + ((xr((x_l_93) + 1, &
                        &(x_i_91) + 1) * t3((x_j_92) + 1, (x_l_93) + 1)))
                    end do
                end do
            end do
            do x_i_94 = 0, (BS) - 1
                do x_j_95 = 0, (BS) - 1
                    x_mm17((x_j_95) + 1, (x_i_94) + 1) = 0.0_c_float
                    do x_l_96 = 0, (BS) - 1
                        x_mm17((x_j_95) + 1, (x_i_94) + 1) = x_mm17((x_j_95) + 1, (x_i_94) + 1) + ((x_mm16((x_l_96) + &
                        &1, (x_i_94) + 1) * xr_jj_dag((x_j_95) + 1, (x_l_96) + 1)))
                    end do
                end do
            end do
            do si1_l97 = 0, (BS) - 1
                do si2_l98 = 0, (BS) - 1
                    xl_d((si2_l98) + 1, (si1_l97) + 1, (j) + 1) = x_mm17((si2_l98) + 1, (si1_l97) + 1)
                end do
            end do
            do x_mmi18_99 = 0, (BS) - 1
                do x_mmj18_100 = 0, (BS) - 1
                    x_mm18((x_mmj18_100) + 1, (x_mmi18_99) + 1) = 0.0_c_float
                    do x_mml18_101 = 0, (BS) - 1
                        x_mm18((x_mmj18_100) + 1, (x_mmi18_99) + 1) = x_mm18((x_mmj18_100) + 1, (x_mmi18_99) + 1) + &
                        &((t1((x_mml18_101) + 1, (x_mmi18_99) + 1) * sigma_greater_upper((x_mmj18_100) + 1, &
                        &(x_mml18_101) + 1, (i_l37) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do si0_l102 = 0, (BS) - 1
                do si1_l103 = 0, (BS) - 1
                    t2((si1_l103) + 1, (si0_l102) + 1) = x_mm18((si1_l103) + 1, (si0_l102) + 1)
                end do
            end do
            do si0_l104 = 0, (BS) - 1
                do si1_l105 = 0, (BS) - 1
                    cj((si1_l105) + 1, (si0_l104) + 1) = CONJG(t2((si1_l105) + 1, (si0_l104) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_106 = 0, (BS) - 1
                do x_t1_107 = 0, (BS) - 1
                    x_cb19((x_t0_106) + 1, (x_t1_107) + 1) = cj((x_t1_107) + 1, (x_t0_106) + 1)
                end do
            end do
            do x_w0_108 = 0, (BS) - 1
                do x_w1_109 = 0, (BS) - 1
                    dag((x_w1_109) + 1, (x_w0_108) + 1) = x_cb19((x_w1_109) + 1, (x_w0_108) + 1)
                end do
            end do
            do x_mmi20_110 = 0, (BS) - 1
                do x_mmj20_111 = 0, (BS) - 1
                    x_mm20((x_mmj20_111) + 1, (x_mmi20_110) + 1) = 0.0_c_float
                    do x_mml20_112 = 0, (BS) - 1
                        x_mm20((x_mmj20_111) + 1, (x_mmi20_110) + 1) = x_mm20((x_mmj20_111) + 1, (x_mmi20_110) + 1) + &
                        &((a_lower((x_mml20_112) + 1, (x_mmi20_110) + 1, (i_l37) + 1, (e_l0) + 1) * xg_d((x_mmj20_111) &
                        &+ 1, (x_mml20_112) + 1, (i_l37) + 1)))
                    end do
                end do
            end do
            do x_i_113 = 0, (BS) - 1
                do x_j_114 = 0, (BS) - 1
                    x_mm21((x_j_114) + 1, (x_i_113) + 1) = 0.0_c_float
                    do x_l_115 = 0, (BS) - 1
                        x_mm21((x_j_114) + 1, (x_i_113) + 1) = x_mm21((x_j_114) + 1, (x_i_113) + 1) + &
                        &((x_mm20((x_l_115) + 1, (x_i_113) + 1) * a_ji_dag((x_j_114) + 1, (x_l_115) + 1)))
                    end do
                end do
            end do
            do si0_l116 = 0, (BS) - 1
                do si1_l117 = 0, (BS) - 1
                    t3((si1_l117) + 1, (si0_l116) + 1) = (((sigma_greater_diag((si1_l117) + 1, (si0_l116) + 1, (j) + &
                    &1, (e_l0) + 1) + x_mm21((si1_l117) + 1, (si0_l116) + 1)) + dag((si1_l117) + 1, (si0_l116) + 1)) - &
                    &t2((si1_l117) + 1, (si0_l116) + 1))
                end do
            end do
            do x_i_118 = 0, (BS) - 1
                do x_j_119 = 0, (BS) - 1
                    x_mm22((x_j_119) + 1, (x_i_118) + 1) = 0.0_c_float
                    do x_l_120 = 0, (BS) - 1
                        x_mm22((x_j_119) + 1, (x_i_118) + 1) = x_mm22((x_j_119) + 1, (x_i_118) + 1) + ((xr((x_l_120) + &
                        &1, (x_i_118) + 1) * t3((x_j_119) + 1, (x_l_120) + 1)))
                    end do
                end do
            end do
            do x_i_121 = 0, (BS) - 1
                do x_j_122 = 0, (BS) - 1
                    x_mm23((x_j_122) + 1, (x_i_121) + 1) = 0.0_c_float
                    do x_l_123 = 0, (BS) - 1
                        x_mm23((x_j_122) + 1, (x_i_121) + 1) = x_mm23((x_j_122) + 1, (x_i_121) + 1) + &
                        &((x_mm22((x_l_123) + 1, (x_i_121) + 1) * xr_jj_dag((x_j_122) + 1, (x_l_123) + 1)))
                    end do
                end do
            end do
            do si1_l124 = 0, (BS) - 1
                do si2_l125 = 0, (BS) - 1
                    xg_d((si2_l125) + 1, (si1_l124) + 1, (j) + 1) = x_mm23((si2_l125) + 1, (si1_l124) + 1)
                end do
            end do
        end do
        do si0_l126 = 0, (BS) - 1
            do si1_l127 = 0, (BS) - 1
                cj((si1_l127) + 1, (si0_l126) + 1) = CONJG(xl_d((si1_l127) + 1, (si0_l126) + 1, ((NB - 1)) + 1))
            end do
        end do
        ! numpy: np.transpose(cj)
        do x_t0_128 = 0, (BS) - 1
            do x_t1_129 = 0, (BS) - 1
                x_cb24((x_t0_128) + 1, (x_t1_129) + 1) = cj((x_t1_129) + 1, (x_t0_128) + 1)
            end do
        end do
        do x_w0_130 = 0, (BS) - 1
            do x_w1_131 = 0, (BS) - 1
                dag((x_w1_131) + 1, (x_w0_130) + 1) = x_cb24((x_w1_131) + 1, (x_w0_130) + 1)
            end do
        end do
        do si2_l132 = 0, (BS) - 1
            do si3_l133 = 0, (BS) - 1
                x_lesser_diag((si3_l133) + 1, (si2_l132) + 1, ((NB - 1)) + 1, (e_l0) + 1) = (0.5_c_float * &
                &(xl_d((si3_l133) + 1, (si2_l132) + 1, ((NB - 1)) + 1) - dag((si3_l133) + 1, (si2_l132) + 1)))
            end do
        end do
        do si0_l134 = 0, (BS) - 1
            do si1_l135 = 0, (BS) - 1
                cj((si1_l135) + 1, (si0_l134) + 1) = CONJG(xg_d((si1_l135) + 1, (si0_l134) + 1, ((NB - 1)) + 1))
            end do
        end do
        ! numpy: np.transpose(cj)
        do x_t0_136 = 0, (BS) - 1
            do x_t1_137 = 0, (BS) - 1
                x_cb25((x_t0_136) + 1, (x_t1_137) + 1) = cj((x_t1_137) + 1, (x_t0_136) + 1)
            end do
        end do
        do x_w0_138 = 0, (BS) - 1
            do x_w1_139 = 0, (BS) - 1
                dag((x_w1_139) + 1, (x_w0_138) + 1) = x_cb25((x_w1_139) + 1, (x_w0_138) + 1)
            end do
        end do
        do si2_l140 = 0, (BS) - 1
            do si3_l141 = 0, (BS) - 1
                x_greater_diag((si3_l141) + 1, (si2_l140) + 1, ((NB - 1)) + 1, (e_l0) + 1) = (0.5_c_float * &
                &(xg_d((si3_l141) + 1, (si2_l140) + 1, ((NB - 1)) + 1) - dag((si3_l141) + 1, (si2_l140) + 1)))
            end do
        end do
        do si2_l142 = 0, (BS) - 1
            do si3_l143 = 0, (BS) - 1
                x_retarded_diag((si3_l143) + 1, (si2_l142) + 1, ((NB - 1)) + 1, (e_l0) + 1) = xr_d((si3_l143) + 1, &
                &(si2_l142) + 1, ((NB - 1)) + 1)
            end do
        end do
        do i_l144 = (NB - 2), ((-1)) + 1, (-1)
            j = (i_l144 + 1)
            do si0_l145 = 0, (BS) - 1
                do si1_l146 = 0, (BS) - 1
                    cj((si1_l146) + 1, (si0_l145) + 1) = CONJG(xr_d((si1_l146) + 1, (si0_l145) + 1, (j) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_147 = 0, (BS) - 1
                do x_t1_148 = 0, (BS) - 1
                    x_cb26((x_t0_147) + 1, (x_t1_148) + 1) = cj((x_t1_148) + 1, (x_t0_147) + 1)
                end do
            end do
            do x_w0_149 = 0, (BS) - 1
                do x_w1_150 = 0, (BS) - 1
                    xr_jj_dag((x_w1_150) + 1, (x_w0_149) + 1) = x_cb26((x_w1_150) + 1, (x_w0_149) + 1)
                end do
            end do
            do x_mmi27_151 = 0, (BS) - 1
                do x_mmj27_152 = 0, (BS) - 1
                    x_mm27((x_mmj27_152) + 1, (x_mmi27_151) + 1) = 0.0_c_float
                    do x_mml27_153 = 0, (BS) - 1
                        x_mm27((x_mmj27_152) + 1, (x_mmi27_151) + 1) = x_mm27((x_mmj27_152) + 1, (x_mmi27_151) + 1) + &
                        &((xr_d((x_mml27_153) + 1, (x_mmi27_151) + 1, (i_l144) + 1) * a_upper((x_mmj27_152) + 1, &
                        &(x_mml27_153) + 1, (i_l144) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do si0_l154 = 0, (BS) - 1
                do si1_l155 = 0, (BS) - 1
                    xr_ii_a_ij((si1_l155) + 1, (si0_l154) + 1) = x_mm27((si1_l155) + 1, (si0_l154) + 1)
                end do
            end do
            do si0_l156 = 0, (BS) - 1
                do si1_l157 = 0, (BS) - 1
                    cj((si1_l157) + 1, (si0_l156) + 1) = CONJG(xr_ii_a_ij((si1_l157) + 1, (si0_l156) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_158 = 0, (BS) - 1
                do x_t1_159 = 0, (BS) - 1
                    x_cb28((x_t0_158) + 1, (x_t1_159) + 1) = cj((x_t1_159) + 1, (x_t0_158) + 1)
                end do
            end do
            do x_w0_160 = 0, (BS) - 1
                do x_w1_161 = 0, (BS) - 1
                    a_ij_dag_xr_ii_dag((x_w1_161) + 1, (x_w0_160) + 1) = x_cb28((x_w1_161) + 1, (x_w0_160) + 1)
                end do
            end do
            do x_mmi29_162 = 0, (BS) - 1
                do x_mmj29_163 = 0, (BS) - 1
                    x_mm29((x_mmj29_163) + 1, (x_mmi29_162) + 1) = 0.0_c_float
                    do x_mml29_164 = 0, (BS) - 1
                        x_mm29((x_mmj29_163) + 1, (x_mmi29_162) + 1) = x_mm29((x_mmj29_163) + 1, (x_mmi29_162) + 1) + &
                        &((xr_d((x_mml29_164) + 1, (x_mmi29_162) + 1, (j) + 1) * a_lower((x_mmj29_163) + 1, &
                        &(x_mml29_164) + 1, (i_l144) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do si0_l165 = 0, (BS) - 1
                do si1_l166 = 0, (BS) - 1
                    xr_jj_a_ji((si1_l166) + 1, (si0_l165) + 1) = x_mm29((si1_l166) + 1, (si0_l165) + 1)
                end do
            end do
            do si0_l167 = 0, (BS) - 1
                do si1_l168 = 0, (BS) - 1
                    cj((si1_l168) + 1, (si0_l167) + 1) = CONJG(xr_jj_a_ji((si1_l168) + 1, (si0_l167) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_169 = 0, (BS) - 1
                do x_t1_170 = 0, (BS) - 1
                    x_cb30((x_t0_169) + 1, (x_t1_170) + 1) = cj((x_t1_170) + 1, (x_t0_169) + 1)
                end do
            end do
            do x_w0_171 = 0, (BS) - 1
                do x_w1_172 = 0, (BS) - 1
                    a_ji_dag_xr_jj_dag((x_w1_172) + 1, (x_w0_171) + 1) = x_cb30((x_w1_172) + 1, (x_w0_171) + 1)
                end do
            end do
            do x_mmi31_173 = 0, (BS) - 1
                do x_mmj31_174 = 0, (BS) - 1
                    x_mm31((x_mmj31_174) + 1, (x_mmi31_173) + 1) = 0.0_c_float
                    do x_mml31_175 = 0, (BS) - 1
                        x_mm31((x_mmj31_174) + 1, (x_mmi31_173) + 1) = x_mm31((x_mmj31_174) + 1, (x_mmi31_173) + 1) + &
                        &((xr_ii_a_ij((x_mml31_175) + 1, (x_mmi31_173) + 1) * xr_d((x_mmj31_174) + 1, (x_mml31_175) + &
                        &1, (j) + 1)))
                    end do
                end do
            end do
            do si0_l176 = 0, (BS) - 1
                do si1_l177 = 0, (BS) - 1
                    xr_ii_a_ij_xr_jj((si1_l177) + 1, (si0_l176) + 1) = x_mm31((si1_l177) + 1, (si0_l176) + 1)
                end do
            end do
            do si0_l178 = 0, (BS) - 1
                do si1_l179 = 0, (BS) - 1
                    cj((si1_l179) + 1, (si0_l178) + 1) = CONJG(xr_ii_a_ij_xr_jj((si1_l179) + 1, (si0_l178) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_180 = 0, (BS) - 1
                do x_t1_181 = 0, (BS) - 1
                    x_cb32((x_t0_180) + 1, (x_t1_181) + 1) = cj((x_t1_181) + 1, (x_t0_180) + 1)
                end do
            end do
            do x_w0_182 = 0, (BS) - 1
                do x_w1_183 = 0, (BS) - 1
                    xr_jj_dag_a_ij_dag_xr_ii_dag((x_w1_183) + 1, (x_w0_182) + 1) = x_cb32((x_w1_183) + 1, (x_w0_182) + &
                    &1)
                end do
            end do
            do x_i_184 = 0, (BS) - 1
                do x_j_185 = 0, (BS) - 1
                    x_mm33((x_j_185) + 1, (x_i_184) + 1) = 0.0_c_float
                    do x_l_186 = 0, (BS) - 1
                        x_mm33((x_j_185) + 1, (x_i_184) + 1) = x_mm33((x_j_185) + 1, (x_i_184) + 1) + &
                        &((xr_ii_a_ij((x_l_186) + 1, (x_i_184) + 1) * xr_jj_a_ji((x_j_185) + 1, (x_l_186) + 1)))
                    end do
                end do
            end do
            do si0_l187 = 0, (BS) - 1
                do si1_l188 = 0, (BS) - 1
                    xr_ii_a_ij_xr_jj_a_ji((si1_l188) + 1, (si0_l187) + 1) = x_mm33((si1_l188) + 1, (si0_l187) + 1)
                end do
            end do
            do x_mmi34_189 = 0, (BS) - 1
                do x_mmj34_190 = 0, (BS) - 1
                    x_mm34((x_mmj34_190) + 1, (x_mmi34_189) + 1) = 0.0_c_float
                    do x_mml34_191 = 0, (BS) - 1
                        x_mm34((x_mmj34_190) + 1, (x_mmi34_189) + 1) = x_mm34((x_mmj34_190) + 1, (x_mmi34_189) + 1) + &
                        &((xr_ii_a_ij_xr_jj_a_ji((x_mml34_191) + 1, (x_mmi34_189) + 1) * xl_d((x_mmj34_190) + 1, &
                        &(x_mml34_191) + 1, (i_l144) + 1)))
                    end do
                end do
            end do
            do x_mmi35_192 = 0, (BS) - 1
                do x_mmj35_193 = 0, (BS) - 1
                    x_mm35((x_mmj35_193) + 1, (x_mmi35_192) + 1) = 0.0_c_float
                    do x_mml35_194 = 0, (BS) - 1
                        x_mm35((x_mmj35_193) + 1, (x_mmi35_192) + 1) = x_mm35((x_mmj35_193) + 1, (x_mmi35_192) + 1) + &
                        &((xr_d((x_mml35_194) + 1, (x_mmi35_192) + 1, (i_l144) + 1) * sigma_lesser_upper((x_mmj35_193) &
                        &+ 1, (x_mml35_194) + 1, (i_l144) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do x_i_195 = 0, (BS) - 1
                do x_j_196 = 0, (BS) - 1
                    x_mm36((x_j_196) + 1, (x_i_195) + 1) = 0.0_c_float
                    do x_l_197 = 0, (BS) - 1
                        x_mm36((x_j_196) + 1, (x_i_195) + 1) = x_mm36((x_j_196) + 1, (x_i_195) + 1) + &
                        &((x_mm35((x_l_197) + 1, (x_i_195) + 1) * xr_jj_dag_a_ij_dag_xr_ii_dag((x_j_196) + 1, &
                        &(x_l_197) + 1)))
                    end do
                end do
            end do
            do si0_l198 = 0, (BS) - 1
                do si1_l199 = 0, (BS) - 1
                    t1((si1_l199) + 1, (si0_l198) + 1) = (x_mm34((si1_l199) + 1, (si0_l198) + 1) - x_mm36((si1_l199) + &
                    &1, (si0_l198) + 1))
                end do
            end do
            do si0_l200 = 0, (BS) - 1
                do si1_l201 = 0, (BS) - 1
                    cj((si1_l201) + 1, (si0_l200) + 1) = CONJG(t1((si1_l201) + 1, (si0_l200) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_202 = 0, (BS) - 1
                do x_t1_203 = 0, (BS) - 1
                    x_cb37((x_t0_202) + 1, (x_t1_203) + 1) = cj((x_t1_203) + 1, (x_t0_202) + 1)
                end do
            end do
            do x_w0_204 = 0, (BS) - 1
                do x_w1_205 = 0, (BS) - 1
                    dag((x_w1_205) + 1, (x_w0_204) + 1) = x_cb37((x_w1_205) + 1, (x_w0_204) + 1)
                end do
            end do
            do si0_l206 = 0, (BS) - 1
                do si1_l207 = 0, (BS) - 1
                    temp_1x((si1_l207) + 1, (si0_l206) + 1) = (t1((si1_l207) + 1, (si0_l206) + 1) - dag((si1_l207) + &
                    &1, (si0_l206) + 1))
                end do
            end do
            do x_mmi38_208 = 0, (BS) - 1
                do x_mmj38_209 = 0, (BS) - 1
                    x_mm38((x_mmj38_209) + 1, (x_mmi38_208) + 1) = 0.0_c_float
                    do x_mml38_210 = 0, (BS) - 1
                        x_mm38((x_mmj38_209) + 1, (x_mmi38_208) + 1) = x_mm38((x_mmj38_209) + 1, (x_mmi38_208) + 1) + &
                        &((xr_ii_a_ij((x_mml38_210) + 1, (x_mmi38_208) + 1) * xl_d((x_mmj38_209) + 1, (x_mml38_210) + &
                        &1, (j) + 1)))
                    end do
                end do
            end do
            do si0_l211 = 0, (BS) - 1
                do si1_l212 = 0, (BS) - 1
                    temp_2x((si1_l212) + 1, (si0_l211) + 1) = x_mm38((si1_l212) + 1, (si0_l211) + 1)
                end do
            end do
            do x_mmi39_213 = 0, (BS) - 1
                do x_mmj39_214 = 0, (BS) - 1
                    x_mm39((x_mmj39_214) + 1, (x_mmi39_213) + 1) = 0.0_c_float
                    do x_mml39_215 = 0, (BS) - 1
                        x_mm39((x_mmj39_214) + 1, (x_mmi39_213) + 1) = x_mm39((x_mmj39_214) + 1, (x_mmi39_213) + 1) + &
                        &((xl_d((x_mml39_215) + 1, (x_mmi39_213) + 1, (i_l144) + 1) * a_ji_dag_xr_jj_dag((x_mmj39_214) &
                        &+ 1, (x_mml39_215) + 1)))
                    end do
                end do
            end do
            do x_mmi40_216 = 0, (BS) - 1
                do x_mmj40_217 = 0, (BS) - 1
                    x_mm40((x_mmj40_217) + 1, (x_mmi40_216) + 1) = 0.0_c_float
                    do x_mml40_218 = 0, (BS) - 1
                        x_mm40((x_mmj40_217) + 1, (x_mmi40_216) + 1) = x_mm40((x_mmj40_217) + 1, (x_mmi40_216) + 1) + &
                        &((xr_d((x_mml40_218) + 1, (x_mmi40_216) + 1, (i_l144) + 1) * sigma_lesser_upper((x_mmj40_217) &
                        &+ 1, (x_mml40_218) + 1, (i_l144) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do x_i_219 = 0, (BS) - 1
                do x_j_220 = 0, (BS) - 1
                    x_mm41((x_j_220) + 1, (x_i_219) + 1) = 0.0_c_float
                    do x_l_221 = 0, (BS) - 1
                        x_mm41((x_j_220) + 1, (x_i_219) + 1) = x_mm41((x_j_220) + 1, (x_i_219) + 1) + &
                        &((x_mm40((x_l_221) + 1, (x_i_219) + 1) * xr_jj_dag((x_j_220) + 1, (x_l_221) + 1)))
                    end do
                end do
            end do
            do si0_l222 = 0, (BS) - 1
                do si1_l223 = 0, (BS) - 1
                    t2((si1_l223) + 1, (si0_l222) + 1) = (((-(temp_2x((si1_l223) + 1, (si0_l222) + 1))) - &
                    &x_mm39((si1_l223) + 1, (si0_l222) + 1)) + x_mm41((si1_l223) + 1, (si0_l222) + 1))
                end do
            end do
            do si2_l224 = 0, (BS) - 1
                do si3_l225 = 0, (BS) - 1
                    x_lesser_upper((si3_l225) + 1, (si2_l224) + 1, (i_l144) + 1, (e_l0) + 1) = t2((si3_l225) + 1, &
                    &(si2_l224) + 1)
                end do
            end do
            do si0_l226 = 0, (BS) - 1
                do si1_l227 = 0, (BS) - 1
                    cj((si1_l227) + 1, (si0_l226) + 1) = CONJG(t2((si1_l227) + 1, (si0_l226) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_228 = 0, (BS) - 1
                do x_t1_229 = 0, (BS) - 1
                    x_cb42((x_t0_228) + 1, (x_t1_229) + 1) = cj((x_t1_229) + 1, (x_t0_228) + 1)
                end do
            end do
            do x_w0_230 = 0, (BS) - 1
                do x_w1_231 = 0, (BS) - 1
                    dag((x_w1_231) + 1, (x_w0_230) + 1) = x_cb42((x_w1_231) + 1, (x_w0_230) + 1)
                end do
            end do
            do si2_l232 = 0, (BS) - 1
                do si3_l233 = 0, (BS) - 1
                    x_lesser_lower((si3_l233) + 1, (si2_l232) + 1, (i_l144) + 1, (e_l0) + 1) = (-(dag((si3_l233) + 1, &
                    &(si2_l232) + 1)))
                end do
            end do
            do x_i_234 = 0, (BS) - 1
                do x_j_235 = 0, (BS) - 1
                    x_mm43((x_j_235) + 1, (x_i_234) + 1) = 0.0_c_float
                    do x_l_236 = 0, (BS) - 1
                        x_mm43((x_j_235) + 1, (x_i_234) + 1) = x_mm43((x_j_235) + 1, (x_i_234) + 1) + &
                        &((temp_2x((x_l_236) + 1, (x_i_234) + 1) * a_ij_dag_xr_ii_dag((x_j_235) + 1, (x_l_236) + 1)))
                    end do
                end do
            end do
            do si0_l237 = 0, (BS) - 1
                do si1_l238 = 0, (BS) - 1
                    t3((si1_l238) + 1, (si0_l237) + 1) = ((xl_d((si1_l238) + 1, (si0_l237) + 1, (i_l144) + 1) + &
                    &x_mm43((si1_l238) + 1, (si0_l237) + 1)) + temp_1x((si1_l238) + 1, (si0_l237) + 1))
                end do
            end do
            do si1_l239 = 0, (BS) - 1
                do si2_l240 = 0, (BS) - 1
                    xl_d((si2_l240) + 1, (si1_l239) + 1, (i_l144) + 1) = t3((si2_l240) + 1, (si1_l239) + 1)
                end do
            end do
            do si0_l241 = 0, (BS) - 1
                do si1_l242 = 0, (BS) - 1
                    cj((si1_l242) + 1, (si0_l241) + 1) = CONJG(t3((si1_l242) + 1, (si0_l241) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_243 = 0, (BS) - 1
                do x_t1_244 = 0, (BS) - 1
                    x_cb44((x_t0_243) + 1, (x_t1_244) + 1) = cj((x_t1_244) + 1, (x_t0_243) + 1)
                end do
            end do
            do x_w0_245 = 0, (BS) - 1
                do x_w1_246 = 0, (BS) - 1
                    dag((x_w1_246) + 1, (x_w0_245) + 1) = x_cb44((x_w1_246) + 1, (x_w0_245) + 1)
                end do
            end do
            do si2_l247 = 0, (BS) - 1
                do si3_l248 = 0, (BS) - 1
                    x_lesser_diag((si3_l248) + 1, (si2_l247) + 1, (i_l144) + 1, (e_l0) + 1) = (0.5_c_float * &
                    &(t3((si3_l248) + 1, (si2_l247) + 1) - dag((si3_l248) + 1, (si2_l247) + 1)))
                end do
            end do
            do x_mmi45_249 = 0, (BS) - 1
                do x_mmj45_250 = 0, (BS) - 1
                    x_mm45((x_mmj45_250) + 1, (x_mmi45_249) + 1) = 0.0_c_float
                    do x_mml45_251 = 0, (BS) - 1
                        x_mm45((x_mmj45_250) + 1, (x_mmi45_249) + 1) = x_mm45((x_mmj45_250) + 1, (x_mmi45_249) + 1) + &
                        &((xr_ii_a_ij_xr_jj_a_ji((x_mml45_251) + 1, (x_mmi45_249) + 1) * xg_d((x_mmj45_250) + 1, &
                        &(x_mml45_251) + 1, (i_l144) + 1)))
                    end do
                end do
            end do
            do x_mmi46_252 = 0, (BS) - 1
                do x_mmj46_253 = 0, (BS) - 1
                    x_mm46((x_mmj46_253) + 1, (x_mmi46_252) + 1) = 0.0_c_float
                    do x_mml46_254 = 0, (BS) - 1
                        x_mm46((x_mmj46_253) + 1, (x_mmi46_252) + 1) = x_mm46((x_mmj46_253) + 1, (x_mmi46_252) + 1) + &
                        &((xr_d((x_mml46_254) + 1, (x_mmi46_252) + 1, (i_l144) + 1) * &
                        &sigma_greater_upper((x_mmj46_253) + 1, (x_mml46_254) + 1, (i_l144) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do x_i_255 = 0, (BS) - 1
                do x_j_256 = 0, (BS) - 1
                    x_mm47((x_j_256) + 1, (x_i_255) + 1) = 0.0_c_float
                    do x_l_257 = 0, (BS) - 1
                        x_mm47((x_j_256) + 1, (x_i_255) + 1) = x_mm47((x_j_256) + 1, (x_i_255) + 1) + &
                        &((x_mm46((x_l_257) + 1, (x_i_255) + 1) * xr_jj_dag_a_ij_dag_xr_ii_dag((x_j_256) + 1, &
                        &(x_l_257) + 1)))
                    end do
                end do
            end do
            do si0_l258 = 0, (BS) - 1
                do si1_l259 = 0, (BS) - 1
                    t1((si1_l259) + 1, (si0_l258) + 1) = (x_mm45((si1_l259) + 1, (si0_l258) + 1) - x_mm47((si1_l259) + &
                    &1, (si0_l258) + 1))
                end do
            end do
            do si0_l260 = 0, (BS) - 1
                do si1_l261 = 0, (BS) - 1
                    cj((si1_l261) + 1, (si0_l260) + 1) = CONJG(t1((si1_l261) + 1, (si0_l260) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_262 = 0, (BS) - 1
                do x_t1_263 = 0, (BS) - 1
                    x_cb48((x_t0_262) + 1, (x_t1_263) + 1) = cj((x_t1_263) + 1, (x_t0_262) + 1)
                end do
            end do
            do x_w0_264 = 0, (BS) - 1
                do x_w1_265 = 0, (BS) - 1
                    dag((x_w1_265) + 1, (x_w0_264) + 1) = x_cb48((x_w1_265) + 1, (x_w0_264) + 1)
                end do
            end do
            do si0_l266 = 0, (BS) - 1
                do si1_l267 = 0, (BS) - 1
                    temp_1x((si1_l267) + 1, (si0_l266) + 1) = (t1((si1_l267) + 1, (si0_l266) + 1) - dag((si1_l267) + &
                    &1, (si0_l266) + 1))
                end do
            end do
            do x_mmi49_268 = 0, (BS) - 1
                do x_mmj49_269 = 0, (BS) - 1
                    x_mm49((x_mmj49_269) + 1, (x_mmi49_268) + 1) = 0.0_c_float
                    do x_mml49_270 = 0, (BS) - 1
                        x_mm49((x_mmj49_269) + 1, (x_mmi49_268) + 1) = x_mm49((x_mmj49_269) + 1, (x_mmi49_268) + 1) + &
                        &((xr_ii_a_ij((x_mml49_270) + 1, (x_mmi49_268) + 1) * xg_d((x_mmj49_269) + 1, (x_mml49_270) + &
                        &1, (j) + 1)))
                    end do
                end do
            end do
            do si0_l271 = 0, (BS) - 1
                do si1_l272 = 0, (BS) - 1
                    temp_2x((si1_l272) + 1, (si0_l271) + 1) = x_mm49((si1_l272) + 1, (si0_l271) + 1)
                end do
            end do
            do x_mmi50_273 = 0, (BS) - 1
                do x_mmj50_274 = 0, (BS) - 1
                    x_mm50((x_mmj50_274) + 1, (x_mmi50_273) + 1) = 0.0_c_float
                    do x_mml50_275 = 0, (BS) - 1
                        x_mm50((x_mmj50_274) + 1, (x_mmi50_273) + 1) = x_mm50((x_mmj50_274) + 1, (x_mmi50_273) + 1) + &
                        &((xg_d((x_mml50_275) + 1, (x_mmi50_273) + 1, (i_l144) + 1) * a_ji_dag_xr_jj_dag((x_mmj50_274) &
                        &+ 1, (x_mml50_275) + 1)))
                    end do
                end do
            end do
            do x_mmi51_276 = 0, (BS) - 1
                do x_mmj51_277 = 0, (BS) - 1
                    x_mm51((x_mmj51_277) + 1, (x_mmi51_276) + 1) = 0.0_c_float
                    do x_mml51_278 = 0, (BS) - 1
                        x_mm51((x_mmj51_277) + 1, (x_mmi51_276) + 1) = x_mm51((x_mmj51_277) + 1, (x_mmi51_276) + 1) + &
                        &((xr_d((x_mml51_278) + 1, (x_mmi51_276) + 1, (i_l144) + 1) * &
                        &sigma_greater_upper((x_mmj51_277) + 1, (x_mml51_278) + 1, (i_l144) + 1, (e_l0) + 1)))
                    end do
                end do
            end do
            do x_i_279 = 0, (BS) - 1
                do x_j_280 = 0, (BS) - 1
                    x_mm52((x_j_280) + 1, (x_i_279) + 1) = 0.0_c_float
                    do x_l_281 = 0, (BS) - 1
                        x_mm52((x_j_280) + 1, (x_i_279) + 1) = x_mm52((x_j_280) + 1, (x_i_279) + 1) + &
                        &((x_mm51((x_l_281) + 1, (x_i_279) + 1) * xr_jj_dag((x_j_280) + 1, (x_l_281) + 1)))
                    end do
                end do
            end do
            do si0_l282 = 0, (BS) - 1
                do si1_l283 = 0, (BS) - 1
                    t2((si1_l283) + 1, (si0_l282) + 1) = (((-(temp_2x((si1_l283) + 1, (si0_l282) + 1))) - &
                    &x_mm50((si1_l283) + 1, (si0_l282) + 1)) + x_mm52((si1_l283) + 1, (si0_l282) + 1))
                end do
            end do
            do si2_l284 = 0, (BS) - 1
                do si3_l285 = 0, (BS) - 1
                    x_greater_upper((si3_l285) + 1, (si2_l284) + 1, (i_l144) + 1, (e_l0) + 1) = t2((si3_l285) + 1, &
                    &(si2_l284) + 1)
                end do
            end do
            do si0_l286 = 0, (BS) - 1
                do si1_l287 = 0, (BS) - 1
                    cj((si1_l287) + 1, (si0_l286) + 1) = CONJG(t2((si1_l287) + 1, (si0_l286) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_288 = 0, (BS) - 1
                do x_t1_289 = 0, (BS) - 1
                    x_cb53((x_t0_288) + 1, (x_t1_289) + 1) = cj((x_t1_289) + 1, (x_t0_288) + 1)
                end do
            end do
            do x_w0_290 = 0, (BS) - 1
                do x_w1_291 = 0, (BS) - 1
                    dag((x_w1_291) + 1, (x_w0_290) + 1) = x_cb53((x_w1_291) + 1, (x_w0_290) + 1)
                end do
            end do
            do si2_l292 = 0, (BS) - 1
                do si3_l293 = 0, (BS) - 1
                    x_greater_lower((si3_l293) + 1, (si2_l292) + 1, (i_l144) + 1, (e_l0) + 1) = (-(dag((si3_l293) + 1, &
                    &(si2_l292) + 1)))
                end do
            end do
            do x_i_294 = 0, (BS) - 1
                do x_j_295 = 0, (BS) - 1
                    x_mm54((x_j_295) + 1, (x_i_294) + 1) = 0.0_c_float
                    do x_l_296 = 0, (BS) - 1
                        x_mm54((x_j_295) + 1, (x_i_294) + 1) = x_mm54((x_j_295) + 1, (x_i_294) + 1) + &
                        &((temp_2x((x_l_296) + 1, (x_i_294) + 1) * a_ij_dag_xr_ii_dag((x_j_295) + 1, (x_l_296) + 1)))
                    end do
                end do
            end do
            do si0_l297 = 0, (BS) - 1
                do si1_l298 = 0, (BS) - 1
                    t3((si1_l298) + 1, (si0_l297) + 1) = ((xg_d((si1_l298) + 1, (si0_l297) + 1, (i_l144) + 1) + &
                    &x_mm54((si1_l298) + 1, (si0_l297) + 1)) + temp_1x((si1_l298) + 1, (si0_l297) + 1))
                end do
            end do
            do si1_l299 = 0, (BS) - 1
                do si2_l300 = 0, (BS) - 1
                    xg_d((si2_l300) + 1, (si1_l299) + 1, (i_l144) + 1) = t3((si2_l300) + 1, (si1_l299) + 1)
                end do
            end do
            do si0_l301 = 0, (BS) - 1
                do si1_l302 = 0, (BS) - 1
                    cj((si1_l302) + 1, (si0_l301) + 1) = CONJG(t3((si1_l302) + 1, (si0_l301) + 1))
                end do
            end do
            ! numpy: np.transpose(cj)
            do x_t0_303 = 0, (BS) - 1
                do x_t1_304 = 0, (BS) - 1
                    x_cb55((x_t0_303) + 1, (x_t1_304) + 1) = cj((x_t1_304) + 1, (x_t0_303) + 1)
                end do
            end do
            do x_w0_305 = 0, (BS) - 1
                do x_w1_306 = 0, (BS) - 1
                    dag((x_w1_306) + 1, (x_w0_305) + 1) = x_cb55((x_w1_306) + 1, (x_w0_305) + 1)
                end do
            end do
            do si2_l307 = 0, (BS) - 1
                do si3_l308 = 0, (BS) - 1
                    x_greater_diag((si3_l308) + 1, (si2_l307) + 1, (i_l144) + 1, (e_l0) + 1) = (0.5_c_float * &
                    &(t3((si3_l308) + 1, (si2_l307) + 1) - dag((si3_l308) + 1, (si2_l307) + 1)))
                end do
            end do
            do x_mmi56_309 = 0, (BS) - 1
                do x_mmj56_310 = 0, (BS) - 1
                    x_mm56((x_mmj56_310) + 1, (x_mmi56_309) + 1) = 0.0_c_float
                    do x_mml56_311 = 0, (BS) - 1
                        x_mm56((x_mmj56_310) + 1, (x_mmi56_309) + 1) = x_mm56((x_mmj56_310) + 1, (x_mmi56_309) + 1) + &
                        &((xr_ii_a_ij_xr_jj_a_ji((x_mml56_311) + 1, (x_mmi56_309) + 1) * xr_d((x_mmj56_310) + 1, &
                        &(x_mml56_311) + 1, (i_l144) + 1)))
                    end do
                end do
            end do
            do si0_l312 = 0, (BS) - 1
                do si1_l313 = 0, (BS) - 1
                    t3((si1_l313) + 1, (si0_l312) + 1) = (xr_d((si1_l313) + 1, (si0_l312) + 1, (i_l144) + 1) + &
                    &x_mm56((si1_l313) + 1, (si0_l312) + 1))
                end do
            end do
            do si1_l314 = 0, (BS) - 1
                do si2_l315 = 0, (BS) - 1
                    xr_d((si2_l315) + 1, (si1_l314) + 1, (i_l144) + 1) = t3((si2_l315) + 1, (si1_l314) + 1)
                end do
            end do
            do si2_l316 = 0, (BS) - 1
                do si3_l317 = 0, (BS) - 1
                    x_retarded_diag((si3_l317) + 1, (si2_l316) + 1, (i_l144) + 1, (e_l0) + 1) = t3((si3_l317) + 1, &
                    &(si2_l316) + 1)
                end do
            end do
        end do
    end do

end subroutine quatrex_rgf_fp32
