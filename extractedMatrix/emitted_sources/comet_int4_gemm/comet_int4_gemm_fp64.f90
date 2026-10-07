! hpcagent_bench-autogen -- generated from comet_int4_gemm_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine comet_int4_gemm_fp64(codes_left, codes_right, out, num_field, num_vector) bind(C, &
&name="comet_int4_gemm_fp64")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), value, intent(in) :: num_field
    integer(c_int64_t), value, intent(in) :: num_vector
    integer(c_int8_t), intent(in) :: codes_left(num_field, num_vector)
    integer(c_int8_t), intent(in) :: codes_right(num_field, num_vector)
    integer(c_int32_t), intent(inout) :: out(2, 2, num_vector, num_vector)
    integer(c_int64_t) :: si0_l13, si0_l20, si0_l27, si0_l34, si1_l14, si1_l21, si1_l28, si1_l35, x_i_10, x_i_17, &
    &x_i_24, x_i_31, x_j_11, x_j_18, x_j_25, x_j_32, x_l_12, x_l_19, x_l_26, x_l_33, x_t0_15, x_t0_22, x_t0_29, &
    &x_t0_8, x_t1_16, x_t1_23, x_t1_30, x_t1_9, x_w0_0, x_w0_2, x_w0_4, x_w0_6, x_w1_1, x_w1_3, x_w1_5, x_w1_7
    real(c_double) :: x_cb1(num_vector, num_field)
    real(c_double) :: x_mm2(num_vector, num_vector)
    real(c_double) :: x_cb3(num_vector, num_field)
    real(c_double) :: x_mm4(num_vector, num_vector)
    real(c_double) :: x_cb5(num_vector, num_field)
    real(c_double) :: x_mm6(num_vector, num_vector)
    real(c_double) :: x_cb7(num_vector, num_field)
    real(c_double) :: x_mm8(num_vector, num_vector)
    real(c_double) :: li1(num_field, num_vector)
    real(c_double) :: li0(num_field, num_vector)
    real(c_double) :: rj1(num_field, num_vector)
    real(c_double) :: rj0(num_field, num_vector)
    do x_w0_0 = 0, (num_vector) - 1
        do x_w1_1 = 0, (num_field) - 1
            li1((x_w1_1) + 1, (x_w0_0) + 1) = (INT(IAND(INT(codes_left((x_w1_1) + 1, (x_w0_0) + 1), c_int64_t), &
            &1_c_int64_t), c_int32_t) + INT(IAND(SHIFTA(INT(codes_left((x_w1_1) + 1, (x_w0_0) + 1), c_int64_t), &
            &1_c_int64_t), 1_c_int64_t), c_int32_t))
        end do
    end do
    do x_w0_2 = 0, (num_vector) - 1
        do x_w1_3 = 0, (num_field) - 1
            li0((x_w1_3) + 1, (x_w0_2) + 1) = (2 - li1((x_w1_3) + 1, (x_w0_2) + 1))
        end do
    end do
    do x_w0_4 = 0, (num_vector) - 1
        do x_w1_5 = 0, (num_field) - 1
            rj1((x_w1_5) + 1, (x_w0_4) + 1) = (INT(IAND(INT(codes_right((x_w1_5) + 1, (x_w0_4) + 1), c_int64_t), &
            &1_c_int64_t), c_int32_t) + INT(IAND(SHIFTA(INT(codes_right((x_w1_5) + 1, (x_w0_4) + 1), c_int64_t), &
            &1_c_int64_t), 1_c_int64_t), c_int32_t))
        end do
    end do
    do x_w0_6 = 0, (num_vector) - 1
        do x_w1_7 = 0, (num_field) - 1
            rj0((x_w1_7) + 1, (x_w0_6) + 1) = (2 - rj1((x_w1_7) + 1, (x_w0_6) + 1))
        end do
    end do
    ! numpy: np.transpose(rj0)
    do x_t0_8 = 0, (num_vector) - 1
        do x_t1_9 = 0, (num_field) - 1
            x_cb1((x_t0_8) + 1, (x_t1_9) + 1) = rj0((x_t1_9) + 1, (x_t0_8) + 1)
        end do
    end do
    do x_i_10 = 0, (num_vector) - 1
        do x_j_11 = 0, (num_vector) - 1
            x_mm2((x_j_11) + 1, (x_i_10) + 1) = 0.0_c_double
            do x_l_12 = 0, (num_field) - 1
                x_mm2((x_j_11) + 1, (x_i_10) + 1) = x_mm2((x_j_11) + 1, (x_i_10) + 1) + ((li0((x_l_12) + 1, (x_i_10) + &
                &1) * x_cb1((x_j_11) + 1, (x_l_12) + 1)))
            end do
        end do
    end do
    do si0_l13 = 0, (num_vector) - 1
        do si1_l14 = 0, (num_vector) - 1
            out((0) + 1, (0) + 1, (si1_l14) + 1, (si0_l13) + 1) = x_mm2((si1_l14) + 1, (si0_l13) + 1)
        end do
    end do
    ! numpy: np.transpose(rj1)
    do x_t0_15 = 0, (num_vector) - 1
        do x_t1_16 = 0, (num_field) - 1
            x_cb3((x_t0_15) + 1, (x_t1_16) + 1) = rj1((x_t1_16) + 1, (x_t0_15) + 1)
        end do
    end do
    do x_i_17 = 0, (num_vector) - 1
        do x_j_18 = 0, (num_vector) - 1
            x_mm4((x_j_18) + 1, (x_i_17) + 1) = 0.0_c_double
            do x_l_19 = 0, (num_field) - 1
                x_mm4((x_j_18) + 1, (x_i_17) + 1) = x_mm4((x_j_18) + 1, (x_i_17) + 1) + ((li0((x_l_19) + 1, (x_i_17) + &
                &1) * x_cb3((x_j_18) + 1, (x_l_19) + 1)))
            end do
        end do
    end do
    do si0_l20 = 0, (num_vector) - 1
        do si1_l21 = 0, (num_vector) - 1
            out((1) + 1, (0) + 1, (si1_l21) + 1, (si0_l20) + 1) = x_mm4((si1_l21) + 1, (si0_l20) + 1)
        end do
    end do
    ! numpy: np.transpose(rj0)
    do x_t0_22 = 0, (num_vector) - 1
        do x_t1_23 = 0, (num_field) - 1
            x_cb5((x_t0_22) + 1, (x_t1_23) + 1) = rj0((x_t1_23) + 1, (x_t0_22) + 1)
        end do
    end do
    do x_i_24 = 0, (num_vector) - 1
        do x_j_25 = 0, (num_vector) - 1
            x_mm6((x_j_25) + 1, (x_i_24) + 1) = 0.0_c_double
            do x_l_26 = 0, (num_field) - 1
                x_mm6((x_j_25) + 1, (x_i_24) + 1) = x_mm6((x_j_25) + 1, (x_i_24) + 1) + ((li1((x_l_26) + 1, (x_i_24) + &
                &1) * x_cb5((x_j_25) + 1, (x_l_26) + 1)))
            end do
        end do
    end do
    do si0_l27 = 0, (num_vector) - 1
        do si1_l28 = 0, (num_vector) - 1
            out((0) + 1, (1) + 1, (si1_l28) + 1, (si0_l27) + 1) = x_mm6((si1_l28) + 1, (si0_l27) + 1)
        end do
    end do
    ! numpy: np.transpose(rj1)
    do x_t0_29 = 0, (num_vector) - 1
        do x_t1_30 = 0, (num_field) - 1
            x_cb7((x_t0_29) + 1, (x_t1_30) + 1) = rj1((x_t1_30) + 1, (x_t0_29) + 1)
        end do
    end do
    do x_i_31 = 0, (num_vector) - 1
        do x_j_32 = 0, (num_vector) - 1
            x_mm8((x_j_32) + 1, (x_i_31) + 1) = 0.0_c_double
            do x_l_33 = 0, (num_field) - 1
                x_mm8((x_j_32) + 1, (x_i_31) + 1) = x_mm8((x_j_32) + 1, (x_i_31) + 1) + ((li1((x_l_33) + 1, (x_i_31) + &
                &1) * x_cb7((x_j_32) + 1, (x_l_33) + 1)))
            end do
        end do
    end do
    do si0_l34 = 0, (num_vector) - 1
        do si1_l35 = 0, (num_vector) - 1
            out((1) + 1, (1) + 1, (si1_l35) + 1, (si0_l34) + 1) = x_mm8((si1_l35) + 1, (si0_l34) + 1)
        end do
    end do

end subroutine comet_int4_gemm_fp64
