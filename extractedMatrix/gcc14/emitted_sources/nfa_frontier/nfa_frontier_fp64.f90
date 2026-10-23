! hpcagent_bench-autogen -- generated from nfa_frontier_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
subroutine nfa_frontier_fp64(activation_counts, col_idx, comp_ptr, is_report, report_counts, row_ptr, start_idx, &
&start_ptr, start_sod, stream, symbol_cols, C, NE, NS, NSTART, T) bind(C, name="nfa_frontier_fp64")
    use, intrinsic :: iso_c_binding
    integer(c_int64_t), value, intent(in) :: C
    integer(c_int64_t), value, intent(in) :: NE
    integer(c_int64_t), value, intent(in) :: NS
    integer(c_int64_t), value, intent(in) :: NSTART
    integer(c_int64_t), value, intent(in) :: T
    integer(c_int64_t), intent(inout) :: activation_counts(NS)
    integer(c_int64_t), intent(in) :: col_idx(NE)
    integer(c_int64_t), intent(in) :: comp_ptr((C + 1))
    integer(c_int8_t), intent(in) :: is_report(NS)
    integer(c_int64_t), intent(inout) :: report_counts(C)
    integer(c_int64_t), intent(in) :: row_ptr((NS + 1))
    integer(c_int64_t), intent(in) :: start_idx(NSTART)
    integer(c_int64_t), intent(in) :: start_ptr((C + 1))
    integer(c_int8_t), intent(in) :: start_sod(NSTART)
    integer(c_int64_t), intent(in) :: stream(T)
    integer(c_int8_t), intent(in) :: symbol_cols(256, NS)
    integer(c_int64_t) :: e_l5, f_c_l0, f_t_l2, k_l1, k_l3, k_l4, k_l6
    integer(c_int64_t) :: n_front
    integer(c_int64_t) :: reports
    integer(c_int64_t) :: s
    integer(c_int64_t) :: eod
    integer(c_int64_t) :: n_match
    integer(c_int64_t) :: child
    integer(c_int64_t) :: enabled(NS)
    integer(c_int64_t) :: frontier(NS)
    integer(c_int64_t) :: matched(NS)
    enabled = 0
    frontier = 0
    matched = 0
    do f_c_l0 = 0, (C) - 1
        n_front = 0
        do k_l1 = start_ptr((f_c_l0) + 1), (start_ptr(((f_c_l0 + 1)) + 1)) - 1
            s = start_idx((k_l1) + 1)
            if ((enabled((s) + 1) == 0)) then
                enabled((s) + 1) = 1
                frontier(((comp_ptr((f_c_l0) + 1) + n_front)) + 1) = s
                n_front = n_front + (1)
            end if
        end do
        reports = INT(0, c_int64_t)
        do f_t_l2 = 0, (T) - 1
            eod = 0
            if ((f_t_l2 == (T - 1))) then
                eod = 1
            else if ((stream((f_t_l2) + 1) == 10)) then
                eod = 1
            end if
            n_match = 0
            do k_l3 = 0, (n_front) - 1
                s = frontier(((comp_ptr((f_c_l0) + 1) + k_l3)) + 1)
                if ((iand(INT(symbol_cols((stream((f_t_l2) + 1)) + 1, (s) + 1), c_int64_t), 255_c_int64_t) /= 0)) then
                    matched(((comp_ptr((f_c_l0) + 1) + n_match)) + 1) = s
                    n_match = n_match + (1)
                    activation_counts((s) + 1) = activation_counts((s) + 1) + (1)
                    if ((iand(INT(is_report((s) + 1), c_int64_t), 255_c_int64_t) /= 0)) then
                        reports = reports + (1)
                    end if
                end if
                enabled((s) + 1) = 0
            end do
            n_front = 0
            do k_l4 = 0, (n_match) - 1
                s = matched(((comp_ptr((f_c_l0) + 1) + k_l4)) + 1)
                do e_l5 = row_ptr((s) + 1), (row_ptr(((s + 1)) + 1)) - 1
                    child = col_idx((e_l5) + 1)
                    if ((enabled((child) + 1) == 0)) then
                        enabled((child) + 1) = 1
                        frontier(((comp_ptr((f_c_l0) + 1) + n_front)) + 1) = child
                        n_front = n_front + (1)
                    end if
                end do
            end do
            do k_l6 = start_ptr((f_c_l0) + 1), (start_ptr(((f_c_l0 + 1)) + 1)) - 1
                if (((iand(INT(start_sod((k_l6) + 1), c_int64_t), 255_c_int64_t) == 0) .OR. (eod == 1))) then
                    s = start_idx((k_l6) + 1)
                    if ((enabled((s) + 1) == 0)) then
                        enabled((s) + 1) = 1
                        frontier(((comp_ptr((f_c_l0) + 1) + n_front)) + 1) = s
                        n_front = n_front + (1)
                    end if
                end if
            end do
        end do
        report_counts((f_c_l0) + 1) = reports
    end do

end subroutine nfa_frontier_fp64
