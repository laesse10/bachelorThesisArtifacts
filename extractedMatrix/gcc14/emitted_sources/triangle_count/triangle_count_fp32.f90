! hpcagent_bench-autogen -- generated from triangle_count_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine triangle_count_fp32(colidx, esrc, rowptr, total, NE, NV) bind(C, name="triangle_count_fp32")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), value, intent(in) :: NE
    integer(c_int64_t), value, intent(in) :: NV
    integer(c_int64_t), intent(in) :: colidx(NE)
    integer(c_int64_t), intent(in) :: esrc(NE)
    integer(c_int64_t), intent(in) :: rowptr((NV + 1))
    integer(c_int64_t), intent(inout) :: total(1)
    integer(c_int64_t) :: e_l0, i_l1
    integer(c_int64_t) :: count
    integer(c_int64_t) :: v
    integer(c_int64_t) :: u
    integer(c_int64_t) :: v_begin
    integer(c_int64_t) :: v_size
    integer(c_int64_t) :: u_begin
    integer(c_int64_t) :: u_size
    integer(c_int64_t) :: lookup_begin
    integer(c_int64_t) :: lookup_size
    integer(c_int64_t) :: search_begin
    integer(c_int64_t) :: search_size
    integer(c_int64_t) :: key
    integer(c_int64_t) :: hit
    integer(c_int64_t) :: bottom
    integer(c_int64_t) :: top
    integer(c_int64_t) :: mid
    integer(c_int64_t) :: y
    integer(c_int64_t) :: lo
    integer(c_int64_t) :: hi
    count = INT(0, c_int64_t)
    do e_l0 = 0, (NE) - 1
        v = esrc((e_l0) + 1)
        u = colidx((e_l0) + 1)
        v_begin = rowptr((v) + 1)
        v_size = (rowptr(((v + 1)) + 1) - v_begin)
        u_begin = rowptr((u) + 1)
        u_size = (rowptr(((u + 1)) + 1) - u_begin)
        if (((v_size > 0) .AND. (u_size > 0))) then
            if ((v_size > u_size)) then
                lookup_begin = u_begin
                lookup_size = u_size
                search_begin = v_begin
                search_size = v_size
            else
                lookup_begin = v_begin
                lookup_size = v_size
                search_begin = u_begin
                search_size = u_size
            end if
            do i_l1 = 0, (lookup_size) - 1
                key = colidx(((lookup_begin + i_l1)) + 1)
                hit = 0
                bottom = 0
                top = 32
                do while (((top > (bottom + 1)) .AND. (hit == 0)))
                    mid = npb_floordiv_i(INT((top + bottom), c_int64_t), INT(2, c_int64_t))
                    y = colidx(((search_begin + npb_floordiv_i(INT((mid * search_size), c_int64_t), INT(32, &
                    &c_int64_t)))) + 1)
                    if ((key == y)) then
                        hit = 1
                    else if ((key < y)) then
                        top = mid
                    else
                        bottom = mid
                    end if
                end do
                if ((hit == 0)) then
                    lo = npb_floordiv_i(INT((bottom * search_size), c_int64_t), INT(32, c_int64_t))
                    hi = (npb_floordiv_i(INT((top * search_size), c_int64_t), INT(32, c_int64_t)) - 1)
                    do while (((hi >= lo) .AND. (hit == 0)))
                        mid = npb_floordiv_i(INT((lo + hi), c_int64_t), INT(2, c_int64_t))
                        y = colidx(((search_begin + mid)) + 1)
                        if ((key == y)) then
                            hit = 1
                        else if ((key < y)) then
                            hi = (mid - 1)
                        else
                            lo = (mid + 1)
                        end if
                    end do
                end if
                count = (count + hit)
            end do
        end if
    end do
    total((0) + 1) = count
contains

    elemental function npb_floordiv_i(a, b) result(r)
        integer(c_int64_t), intent(in) :: a, b
        integer(c_int64_t) :: r
        r = a / b - merge(1_c_int64_t, 0_c_int64_t, (mod(a, b) /= 0_c_int64_t) .and. ((a < 0_c_int64_t) .neqv. (b < &
        &0_c_int64_t)))
    end function npb_floordiv_i

end subroutine triangle_count_fp32
