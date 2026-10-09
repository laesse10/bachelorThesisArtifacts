! hpcagent_bench-autogen -- generated from spgemm_hash_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine spgemm_hash_fp32(A_indices, A_indptr, B_indices, B_indptr, C_indices, C_indptr, K, M, N, nnz_A, nnz_B, &
&nnz_C_cap) bind(C, name="spgemm_hash_fp32")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), value, intent(in) :: K
    integer(c_int64_t), value, intent(in) :: M
    integer(c_int64_t), value, intent(in) :: N
    integer(c_int64_t), value, intent(in) :: nnz_A
    integer(c_int64_t), value, intent(in) :: nnz_B
    integer(c_int64_t), value, intent(in) :: nnz_C_cap
    integer(c_int64_t), intent(in) :: A_indices(nnz_A)
    integer(c_int64_t), intent(in) :: A_indptr((M + 1))
    integer(c_int64_t), intent(in) :: B_indices(nnz_B)
    integer(c_int64_t), intent(in) :: B_indptr((K + 1))
    integer(c_int64_t), intent(inout) :: C_indices(nnz_C_cap)
    integer(c_int64_t), intent(inout) :: C_indptr((M + 1))
    integer(c_int64_t) :: b_l12, b_l14, b_l2, b_l4, f_k_l10, f_k_l20, i_l0, i_l11, i_l13, i_l16, i_l3, i_l6, j_l1, &
    &j_l19, j_l9, r_l15, r_l17, r_l5, r_l7, t_l18, t_l21, t_l8
    integer(c_int64_t) :: running
    integer(c_int64_t) :: products
    integer(c_int64_t) :: chosen
    integer(c_int64_t) :: row
    integer(c_int64_t) :: a_col
    integer(c_int64_t) :: ts
    integer(c_int64_t) :: distinct
    integer(c_int64_t) :: base
    integer(c_int64_t) :: count
    integer(c_int64_t) :: b_col
    integer(c_int64_t) :: slot
    integer(c_int64_t) :: probing
    integer(c_int64_t) :: held
    integer(c_int64_t) :: prod(M)
    integer(c_int64_t) :: row_bin(M)
    integer(c_int64_t) :: row_nnz(M)
    integer(c_int64_t) :: bin_size(8)
    integer(c_int64_t) :: bin_offset(8)
    integer(c_int64_t) :: rows_in_bins(M)
    integer(c_int64_t) :: table(4096)
    prod = 0
    row_bin = 0
    row_nnz = 0
    bin_size = 0
    bin_offset = 0
    rows_in_bins = 0
    do i_l0 = 0, (M) - 1
        products = 0
        do j_l1 = A_indptr((i_l0) + 1), (A_indptr(((i_l0 + 1)) + 1)) - 1
            a_col = A_indices((j_l1) + 1)
            products = (products + (B_indptr(((a_col + 1)) + 1) - B_indptr((a_col) + 1)))
        end do
        if ((products > N)) then
            products = N
        end if
        prod((i_l0) + 1) = products
    end do
    do b_l2 = 0, (8) - 1
        bin_size((b_l2) + 1) = 0
    end do
    do i_l3 = 0, (M) - 1
        call x_select_bin(chosen, REAL(prod((i_l3) + 1), c_float))
        row_bin((i_l3) + 1) = chosen
        if ((chosen >= 0)) then
            bin_size((chosen) + 1) = (bin_size((chosen) + 1) + 1)
        end if
    end do
    running = 0
    do b_l4 = 0, (8) - 1
        bin_offset((b_l4) + 1) = running
        running = (running + bin_size((b_l4) + 1))
        bin_size((b_l4) + 1) = 0
    end do
    do r_l5 = 0, (M) - 1
        rows_in_bins((r_l5) + 1) = (-1)
    end do
    do i_l6 = 0, (M) - 1
        chosen = row_bin((i_l6) + 1)
        if ((chosen >= 0)) then
            rows_in_bins(((bin_offset((chosen) + 1) + bin_size((chosen) + 1))) + 1) = i_l6
            bin_size((chosen) + 1) = (bin_size((chosen) + 1) + 1)
        end if
    end do
    do r_l7 = 0, (M) - 1
        row = rows_in_bins((r_l7) + 1)
        if ((row >= 0)) then
            call x_table_size(ts, INT(row_bin((row) + 1), c_int64_t))
            do t_l8 = 0, (ts) - 1
                table((t_l8) + 1) = N
            end do
            distinct = 0
            do j_l9 = A_indptr((row) + 1), (A_indptr(((row + 1)) + 1)) - 1
                a_col = A_indices((j_l9) + 1)
                do f_k_l10 = B_indptr((a_col) + 1), (B_indptr(((a_col + 1)) + 1)) - 1
                    b_col = B_indices((f_k_l10) + 1)
                    slot = MODULO((b_col * 107), ts)
                    probing = 1
                    do while ((probing == 1))
                        held = table((slot) + 1)
                        if ((held == b_col)) then
                            probing = 0
                        else if ((held == N)) then
                            table((slot) + 1) = b_col
                            distinct = (distinct + 1)
                            probing = 0
                        else
                            slot = (slot + 1)
                            if ((slot == ts)) then
                                slot = 0
                            end if
                        end if
                    end do
                end do
            end do
            row_nnz((row) + 1) = distinct
        end if
    end do
    running = 0
    do i_l11 = 0, (M) - 1
        C_indptr((i_l11) + 1) = running
        running = (running + row_nnz((i_l11) + 1))
    end do
    C_indptr((M) + 1) = running
    do b_l12 = 0, (8) - 1
        bin_size((b_l12) + 1) = 0
    end do
    do i_l13 = 0, (M) - 1
        call x_select_bin__s2(chosen, REAL(row_nnz((i_l13) + 1), c_float))
        row_bin((i_l13) + 1) = chosen
        if ((chosen >= 0)) then
            bin_size((chosen) + 1) = (bin_size((chosen) + 1) + 1)
        end if
    end do
    running = 0
    do b_l14 = 0, (8) - 1
        bin_offset((b_l14) + 1) = running
        running = (running + bin_size((b_l14) + 1))
        bin_size((b_l14) + 1) = 0
    end do
    do r_l15 = 0, (M) - 1
        rows_in_bins((r_l15) + 1) = (-1)
    end do
    do i_l16 = 0, (M) - 1
        chosen = row_bin((i_l16) + 1)
        if ((chosen >= 0)) then
            rows_in_bins(((bin_offset((chosen) + 1) + bin_size((chosen) + 1))) + 1) = i_l16
            bin_size((chosen) + 1) = (bin_size((chosen) + 1) + 1)
        end if
    end do
    do r_l17 = 0, (M) - 1
        row = rows_in_bins((r_l17) + 1)
        if ((row >= 0)) then
            call x_table_size(ts, INT(row_bin((row) + 1), c_int64_t))
            do t_l18 = 0, (ts) - 1
                table((t_l18) + 1) = N
            end do
            do j_l19 = A_indptr((row) + 1), (A_indptr(((row + 1)) + 1)) - 1
                a_col = A_indices((j_l19) + 1)
                do f_k_l20 = B_indptr((a_col) + 1), (B_indptr(((a_col + 1)) + 1)) - 1
                    b_col = B_indices((f_k_l20) + 1)
                    slot = MODULO((b_col * 107), ts)
                    probing = 1
                    do while ((probing == 1))
                        held = table((slot) + 1)
                        if ((held == b_col)) then
                            probing = 0
                        else if ((held == N)) then
                            table((slot) + 1) = b_col
                            probing = 0
                        else
                            slot = (slot + 1)
                            if ((slot == ts)) then
                                slot = 0
                            end if
                        end if
                    end do
                end do
            end do
            call x_bitonic_sort(table, INT(ts, c_int64_t))
            base = C_indptr((row) + 1)
            count = (C_indptr(((row + 1)) + 1) - base)
            do t_l21 = 0, (count) - 1
                C_indices(((base + t_l21)) + 1) = table((t_l21) + 1)
            end do
        end if
    end do
contains
    subroutine x_select_bin(hret_, size)
        use, intrinsic :: iso_c_binding
        real(c_float), value, intent(in) :: size
        integer(c_int64_t), intent(out) :: hret_
        integer(c_int64_t) :: b_l0
        integer(c_int64_t) :: chosen
        integer(c_int64_t) :: low
        integer(c_int64_t) :: high
            chosen = (-1)
            low = 0
            high = 32
            do b_l0 = 0, (8) - 1
                if (((size > low) .AND. (size <= high))) then
                    chosen = b_l0
                end if
                low = high
                high = (high * 2)
            end do
            hret_ = chosen
            return
    end subroutine x_select_bin
    subroutine x_table_size(hret_, b)
        use, intrinsic :: iso_c_binding
        integer(c_int64_t), value, intent(in) :: b
        integer(c_int64_t), intent(out) :: hret_
        integer(c_int64_t) :: x__0
        integer(c_int64_t) :: ts
            ts = 32
            do x__0 = 0, (b) - 1
                ts = (ts * 2)
            end do
            hret_ = ts
            return
    end subroutine x_table_size
    subroutine x_bitonic_sort(key, n)
        use, intrinsic :: iso_c_binding
        integer(c_int64_t), value, intent(in) :: n
        integer(c_int64_t), intent(inout) :: key(4096)
        integer(c_int64_t) :: idx_l0, idx_l1
        integer(c_int64_t) :: size
        integer(c_int64_t) :: stride
        integer(c_int64_t) :: pos
        integer(c_int64_t) :: left
        integer(c_int64_t) :: right
        integer(c_int64_t) :: ascending
        integer(c_int64_t) :: greater
            size = 2
            do while ((size < n))
                stride = npb_floordiv_i(INT(size, c_int64_t), INT(2, c_int64_t))
                do while ((stride > 0))
                    do idx_l0 = 0, (npb_floordiv_i(INT(n, c_int64_t), INT(2, c_int64_t))) - 1
                        ascending = 1
                        if ((MODULO(npb_floordiv_i(INT(idx_l0, c_int64_t), INT(npb_floordiv_i(INT(size, c_int64_t), &
                        &INT(2, c_int64_t)), c_int64_t)), 2_c_int64_t) == 1)) then
                            ascending = 0
                        end if
                        pos = ((2 * idx_l0) - MODULO(idx_l0, stride))
                        left = key((pos) + 1)
                        right = key(((pos + stride)) + 1)
                        greater = 0
                        if ((left > right)) then
                            greater = 1
                        end if
                        if ((greater == ascending)) then
                            key((pos) + 1) = right
                            key(((pos + stride)) + 1) = left
                        end if
                    end do
                    stride = npb_floordiv_i(INT(stride, c_int64_t), INT(2, c_int64_t))
                end do
                size = (size * 2)
            end do
            stride = npb_floordiv_i(INT(n, c_int64_t), INT(2, c_int64_t))
            do while ((stride > 0))
                do idx_l1 = 0, (npb_floordiv_i(INT(n, c_int64_t), INT(2, c_int64_t))) - 1
                    pos = ((2 * idx_l1) - MODULO(idx_l1, stride))
                    left = key((pos) + 1)
                    right = key(((pos + stride)) + 1)
                    if ((left > right)) then
                        key((pos) + 1) = right
                        key(((pos + stride)) + 1) = left
                    end if
                end do
                stride = npb_floordiv_i(INT(stride, c_int64_t), INT(2, c_int64_t))
            end do
    end subroutine x_bitonic_sort
    subroutine x_select_bin__s2(hret_, size)
        use, intrinsic :: iso_c_binding
        real(c_float), value, intent(in) :: size
        integer(c_int64_t), intent(out) :: hret_
        integer(c_int64_t) :: b_l0
        integer(c_int64_t) :: chosen
        integer(c_int64_t) :: low
        integer(c_int64_t) :: high
            chosen = (-1)
            low = 0
            high = 32
            do b_l0 = 0, (8) - 1
                if (((size > low) .AND. (size <= high))) then
                    chosen = b_l0
                end if
                low = high
                high = (high * 2)
            end do
            hret_ = chosen
            return
    end subroutine x_select_bin__s2

    elemental function npb_floordiv_i(a, b) result(r)
        integer(c_int64_t), intent(in) :: a, b
        integer(c_int64_t) :: r
        r = a / b - merge(1_c_int64_t, 0_c_int64_t, (mod(a, b) /= 0_c_int64_t) .and. ((a < 0_c_int64_t) .neqv. (b < &
        &0_c_int64_t)))
    end function npb_floordiv_i

end subroutine spgemm_hash_fp32
