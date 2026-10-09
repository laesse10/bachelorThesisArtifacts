! hpcagent_bench-autogen -- generated from warpx_boris_push_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine warpx_boris_push_fp32(Bx, By, Bz, Ex, Ey, Ez, ux, uy, uz, m, momentum_push_type, np_particles, q) bind(C, &
&name="warpx_boris_push_fp32")
    use, intrinsic :: iso_c_binding
    real(c_float), parameter :: dt = 1e-13_8
    integer(c_int64_t), value, intent(in) :: momentum_push_type
    integer(c_int64_t), value, intent(in) :: np_particles
    real(c_float), intent(in) :: Bx(np_particles)
    real(c_float), intent(in) :: By(np_particles)
    real(c_float), intent(in) :: Bz(np_particles)
    real(c_float), intent(in) :: Ex(np_particles)
    real(c_float), intent(in) :: Ey(np_particles)
    real(c_float), intent(in) :: Ez(np_particles)
    real(c_float), intent(inout) :: ux(np_particles)
    real(c_float), intent(inout) :: uy(np_particles)
    real(c_float), intent(inout) :: uz(np_particles)
    real(c_float), value, intent(in) :: m
    real(c_float), value, intent(in) :: q
    integer(c_int64_t) :: x_r0_10, x_r0_12, x_r0_3, x_w0_0, x_w0_1, x_w0_11, x_w0_13, x_w0_14, x_w0_15, x_w0_16, &
    &x_w0_17, x_w0_18, x_w0_19, x_w0_2, x_w0_20, x_w0_21, x_w0_22, x_w0_23, x_w0_24, x_w0_25, x_w0_26, x_w0_27, &
    &x_w0_28, x_w0_29, x_w0_4, x_w0_5, x_w0_6, x_w0_7, x_w0_8, x_w0_9
    integer(c_int64_t) :: mpt
    real(c_float) :: x_inl1_econst
    real(c_float) :: x_inl1_inv_c2
    real(c_float) :: x_ifexp0
    real(c_float) :: x_ifexp1
    real(c_float) :: x_cb1(np_particles)
    real(c_float) :: x_cb2(np_particles)
    real(c_float) :: x_cb3(np_particles)
    real(c_float) :: x_inl1_inv_gamma(np_particles)
    real(c_float) :: x_inl1_tx(np_particles)
    real(c_float) :: x_inl1_ty(np_particles)
    real(c_float) :: x_inl1_tz(np_particles)
    real(c_float) :: x_inl1_tsq(np_particles)
    logical(c_bool) :: x_inl1_has_field(np_particles)
    real(c_float) :: x_inl1_safe_tsq(np_particles)
    real(c_float) :: x_inl1_factor(np_particles)
    real(c_float) :: x_inl1_tsqi(np_particles)
    real(c_float) :: x_inl1_sx(np_particles)
    real(c_float) :: x_inl1_sy(np_particles)
    real(c_float) :: x_inl1_sz(np_particles)
    real(c_float) :: x_inl1_ux_p(np_particles)
    real(c_float) :: x_inl1_uy_p(np_particles)
    real(c_float) :: x_inl1_uz_p(np_particles)
    mpt = INT(momentum_push_type, c_int64_t)
    x_inl1_econst = (((0.5_c_float * q) * dt) / m)
    if (((mpt == 1) .OR. (mpt == 0))) then
        do x_w0_0 = 0, (np_particles) - 1
            ux((x_w0_0) + 1) = ux((x_w0_0) + 1) + ((x_inl1_econst * Ex((x_w0_0) + 1)))
        end do
        do x_w0_1 = 0, (np_particles) - 1
            uy((x_w0_1) + 1) = uy((x_w0_1) + 1) + ((x_inl1_econst * Ey((x_w0_1) + 1)))
        end do
        do x_w0_2 = 0, (np_particles) - 1
            uz((x_w0_2) + 1) = uz((x_w0_2) + 1) + ((x_inl1_econst * Ez((x_w0_2) + 1)))
        end do
    end if
    x_inl1_inv_c2 = 1.1126500560536185e-17_c_float
    ! numpy: np.sqrt(1.0 + (ux * ux + uy * uy + uz * uz) * __inl1_inv_c2)
    do x_r0_3 = 0, (np_particles) - 1
        x_cb1((x_r0_3) + 1) = SQRT((1.0_c_float + ((((ux((x_r0_3) + 1) * ux((x_r0_3) + 1)) + (uy((x_r0_3) + 1) * &
        &uy((x_r0_3) + 1))) + (uz((x_r0_3) + 1) * uz((x_r0_3) + 1))) * x_inl1_inv_c2)))
    end do
    do x_w0_4 = 0, (np_particles) - 1
        x_inl1_inv_gamma((x_w0_4) + 1) = (1.0_c_float / x_cb1((x_w0_4) + 1))
    end do
    do x_w0_5 = 0, (np_particles) - 1
        x_inl1_tx((x_w0_5) + 1) = ((x_inl1_econst * x_inl1_inv_gamma((x_w0_5) + 1)) * Bx((x_w0_5) + 1))
    end do
    do x_w0_6 = 0, (np_particles) - 1
        x_inl1_ty((x_w0_6) + 1) = ((x_inl1_econst * x_inl1_inv_gamma((x_w0_6) + 1)) * By((x_w0_6) + 1))
    end do
    do x_w0_7 = 0, (np_particles) - 1
        x_inl1_tz((x_w0_7) + 1) = ((x_inl1_econst * x_inl1_inv_gamma((x_w0_7) + 1)) * Bz((x_w0_7) + 1))
    end do
    if (((mpt == 1) .OR. (mpt == 2))) then
        do x_w0_8 = 0, (np_particles) - 1
            x_inl1_tsq((x_w0_8) + 1) = (((x_inl1_tx((x_w0_8) + 1) * x_inl1_tx((x_w0_8) + 1)) + (x_inl1_ty((x_w0_8) + &
            &1) * x_inl1_ty((x_w0_8) + 1))) + (x_inl1_tz((x_w0_8) + 1) * x_inl1_tz((x_w0_8) + 1)))
        end do
        do x_w0_9 = 0, (np_particles) - 1
            x_inl1_has_field((x_w0_9) + 1) = (x_inl1_tsq((x_w0_9) + 1) > 0.0_c_float)
        end do
        ! numpy: np.where(__inl1_has_field, __inl1_tsq, 1.0)
        do x_r0_10 = 0, (np_particles) - 1
            if (x_inl1_has_field((x_r0_10) + 1)) then
                x_ifexp0 = x_inl1_tsq((x_r0_10) + 1)
            else
                x_ifexp0 = 1.0_c_float
            end if
            x_cb2((x_r0_10) + 1) = x_ifexp0
        end do
        do x_w0_11 = 0, (np_particles) - 1
            x_inl1_safe_tsq((x_w0_11) + 1) = x_cb2((x_w0_11) + 1)
        end do
        ! numpy: np.where(__inl1_has_field, (sqrt(1.0 + __inl1_tsq) - 1.0) / __inl1_safe_tsq, 0.5)
        do x_r0_12 = 0, (np_particles) - 1
            if (x_inl1_has_field((x_r0_12) + 1)) then
                x_ifexp1 = ((SQRT((1.0_c_float + x_inl1_tsq((x_r0_12) + 1))) - 1.0_c_float) / &
                &x_inl1_safe_tsq((x_r0_12) + 1))
            else
                x_ifexp1 = 0.5_c_float
            end if
            x_cb3((x_r0_12) + 1) = x_ifexp1
        end do
        do x_w0_13 = 0, (np_particles) - 1
            x_inl1_factor((x_w0_13) + 1) = x_cb3((x_w0_13) + 1)
        end do
        do x_w0_14 = 0, (np_particles) - 1
            x_inl1_tx((x_w0_14) + 1) = x_inl1_tx((x_w0_14) + 1) * (x_inl1_factor((x_w0_14) + 1))
        end do
        do x_w0_15 = 0, (np_particles) - 1
            x_inl1_ty((x_w0_15) + 1) = x_inl1_ty((x_w0_15) + 1) * (x_inl1_factor((x_w0_15) + 1))
        end do
        do x_w0_16 = 0, (np_particles) - 1
            x_inl1_tz((x_w0_16) + 1) = x_inl1_tz((x_w0_16) + 1) * (x_inl1_factor((x_w0_16) + 1))
        end do
    end if
    do x_w0_17 = 0, (np_particles) - 1
        x_inl1_tsqi((x_w0_17) + 1) = (2.0_c_float / (((1.0_c_float + (x_inl1_tx((x_w0_17) + 1) * x_inl1_tx((x_w0_17) + &
        &1))) + (x_inl1_ty((x_w0_17) + 1) * x_inl1_ty((x_w0_17) + 1))) + (x_inl1_tz((x_w0_17) + 1) * &
        &x_inl1_tz((x_w0_17) + 1))))
    end do
    do x_w0_18 = 0, (np_particles) - 1
        x_inl1_sx((x_w0_18) + 1) = (x_inl1_tx((x_w0_18) + 1) * x_inl1_tsqi((x_w0_18) + 1))
    end do
    do x_w0_19 = 0, (np_particles) - 1
        x_inl1_sy((x_w0_19) + 1) = (x_inl1_ty((x_w0_19) + 1) * x_inl1_tsqi((x_w0_19) + 1))
    end do
    do x_w0_20 = 0, (np_particles) - 1
        x_inl1_sz((x_w0_20) + 1) = (x_inl1_tz((x_w0_20) + 1) * x_inl1_tsqi((x_w0_20) + 1))
    end do
    do x_w0_21 = 0, (np_particles) - 1
        x_inl1_ux_p((x_w0_21) + 1) = ((ux((x_w0_21) + 1) + (uy((x_w0_21) + 1) * x_inl1_tz((x_w0_21) + 1))) - &
        &(uz((x_w0_21) + 1) * x_inl1_ty((x_w0_21) + 1)))
    end do
    do x_w0_22 = 0, (np_particles) - 1
        x_inl1_uy_p((x_w0_22) + 1) = ((uy((x_w0_22) + 1) + (uz((x_w0_22) + 1) * x_inl1_tx((x_w0_22) + 1))) - &
        &(ux((x_w0_22) + 1) * x_inl1_tz((x_w0_22) + 1)))
    end do
    do x_w0_23 = 0, (np_particles) - 1
        x_inl1_uz_p((x_w0_23) + 1) = ((uz((x_w0_23) + 1) + (ux((x_w0_23) + 1) * x_inl1_ty((x_w0_23) + 1))) - &
        &(uy((x_w0_23) + 1) * x_inl1_tx((x_w0_23) + 1)))
    end do
    do x_w0_24 = 0, (np_particles) - 1
        ux((x_w0_24) + 1) = ux((x_w0_24) + 1) + (((x_inl1_uy_p((x_w0_24) + 1) * x_inl1_sz((x_w0_24) + 1)) - &
        &(x_inl1_uz_p((x_w0_24) + 1) * x_inl1_sy((x_w0_24) + 1))))
    end do
    do x_w0_25 = 0, (np_particles) - 1
        uy((x_w0_25) + 1) = uy((x_w0_25) + 1) + (((x_inl1_uz_p((x_w0_25) + 1) * x_inl1_sx((x_w0_25) + 1)) - &
        &(x_inl1_ux_p((x_w0_25) + 1) * x_inl1_sz((x_w0_25) + 1))))
    end do
    do x_w0_26 = 0, (np_particles) - 1
        uz((x_w0_26) + 1) = uz((x_w0_26) + 1) + (((x_inl1_ux_p((x_w0_26) + 1) * x_inl1_sy((x_w0_26) + 1)) - &
        &(x_inl1_uy_p((x_w0_26) + 1) * x_inl1_sx((x_w0_26) + 1))))
    end do
    if (((mpt == 2) .OR. (mpt == 0))) then
        do x_w0_27 = 0, (np_particles) - 1
            ux((x_w0_27) + 1) = ux((x_w0_27) + 1) + ((x_inl1_econst * Ex((x_w0_27) + 1)))
        end do
        do x_w0_28 = 0, (np_particles) - 1
            uy((x_w0_28) + 1) = uy((x_w0_28) + 1) + ((x_inl1_econst * Ey((x_w0_28) + 1)))
        end do
        do x_w0_29 = 0, (np_particles) - 1
            uz((x_w0_29) + 1) = uz((x_w0_29) + 1) + ((x_inl1_econst * Ez((x_w0_29) + 1)))
        end do
    end if

end subroutine warpx_boris_push_fp32
