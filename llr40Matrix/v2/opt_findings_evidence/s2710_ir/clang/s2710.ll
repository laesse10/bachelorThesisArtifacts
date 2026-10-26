; ModuleID = 's2710.c'
source_filename = "s2710.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable vscale_range(1,16)
define void @tsvc_2_s2710_fp64(ptr noalias noundef captures(none) %a, ptr noalias noundef captures(none) %b, ptr noalias noundef captures(none) %c, ptr noalias noundef readonly captures(none) %d, ptr noalias noundef readonly captures(none) %e, ptr noalias noundef readonly captures(none) %x, i64 noundef %LEN_1D) local_unnamed_addr #0 {
entry:
  %cmp93 = icmp sgt i64 %LEN_1D, 0
  br i1 %cmp93, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp7 = icmp samesign ugt i64 %LEN_1D, 10
  br i1 %cmp7, label %for.body.us, label %for.body

for.body.us:                                      ; preds = %for.body.lr.ph, %for.inc.us
  %i.094.us = phi i64 [ %inc.us, %for.inc.us ], [ 0, %for.body.lr.ph ]
  %arrayidx.us = getelementptr inbounds nuw double, ptr %a, i64 %i.094.us
  %0 = load double, ptr %arrayidx.us, align 8, !tbaa !10
  %arrayidx1.us = getelementptr inbounds nuw double, ptr %b, i64 %i.094.us
  %1 = load double, ptr %arrayidx1.us, align 8, !tbaa !10
  %cmp2.us = fcmp nsz contract ogt double %0, %1
  br i1 %cmp2.us, label %if.then.us, label %if.else20.us

if.else20.us:                                     ; preds = %for.body.us
  %arrayidx22.us = getelementptr inbounds nuw double, ptr %e, i64 %i.094.us
  %2 = load double, ptr %arrayidx22.us, align 8, !tbaa !10
  %mul24.us = fmul nsz contract double %2, %2
  %add25.us = fadd nsz contract double %0, %mul24.us
  store double %add25.us, ptr %arrayidx1.us, align 8, !tbaa !10
  %3 = load double, ptr %x, align 8, !tbaa !10
  %cmp28.us = fcmp nsz contract ogt double %3, 0.000000e+00
  br i1 %cmp28.us, label %if.then29.us, label %if.else36.us

if.else36.us:                                     ; preds = %if.else20.us
  %arrayidx37.us = getelementptr inbounds nuw double, ptr %c, i64 %i.094.us
  %4 = load double, ptr %arrayidx37.us, align 8, !tbaa !10
  %add41.us = fadd nsz contract double %mul24.us, %4
  store double %add41.us, ptr %arrayidx37.us, align 8, !tbaa !10
  br label %for.inc.us

if.then29.us:                                     ; preds = %if.else20.us
  %arrayidx31.us = getelementptr inbounds nuw double, ptr %d, i64 %i.094.us
  %5 = load double, ptr %arrayidx31.us, align 8, !tbaa !10
  %mul33.us = fmul nsz contract double %5, %5
  %add34.us = fadd nsz contract double %0, %mul33.us
  %arrayidx35.us = getelementptr inbounds nuw double, ptr %c, i64 %i.094.us
  store double %add34.us, ptr %arrayidx35.us, align 8, !tbaa !10
  br label %for.inc.us

if.then.us:                                       ; preds = %for.body.us
  %arrayidx5.us = getelementptr inbounds nuw double, ptr %d, i64 %i.094.us
  %6 = load double, ptr %arrayidx5.us, align 8, !tbaa !10
  %mul.us = fmul nsz contract double %1, %6
  %add.us = fadd nsz contract double %0, %mul.us
  store double %add.us, ptr %arrayidx.us, align 8, !tbaa !10
  %arrayidx9.us = getelementptr inbounds nuw double, ptr %c, i64 %i.094.us
  %7 = load double, ptr %arrayidx9.us, align 8, !tbaa !10
  %mul12.us = fmul nsz contract double %6, %6
  %add13.us = fadd nsz contract double %mul12.us, %7
  store double %add13.us, ptr %arrayidx9.us, align 8, !tbaa !10
  br label %for.inc.us

for.inc.us:                                       ; preds = %if.then.us, %if.then29.us, %if.else36.us
  %inc.us = add nuw nsw i64 %i.094.us, 1
  %exitcond96.not = icmp eq i64 %inc.us, %LEN_1D
  br i1 %exitcond96.not, label %for.cond.cleanup, label %for.body.us, !llvm.loop !12

for.cond.cleanup:                                 ; preds = %for.inc, %for.inc.us, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.094 = phi i64 [ %inc, %for.inc ], [ 0, %for.body.lr.ph ]
  %arrayidx = getelementptr inbounds nuw double, ptr %a, i64 %i.094
  %8 = load double, ptr %arrayidx, align 8, !tbaa !10
  %arrayidx1 = getelementptr inbounds nuw double, ptr %b, i64 %i.094
  %9 = load double, ptr %arrayidx1, align 8, !tbaa !10
  %cmp2 = fcmp nsz contract ogt double %8, %9
  br i1 %cmp2, label %if.then, label %if.else20

if.then:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds nuw double, ptr %d, i64 %i.094
  %10 = load double, ptr %arrayidx5, align 8, !tbaa !10
  %mul = fmul nsz contract double %9, %10
  %add = fadd nsz contract double %8, %mul
  store double %add, ptr %arrayidx, align 8, !tbaa !10
  %arrayidx16 = getelementptr inbounds nuw double, ptr %e, i64 %i.094
  %11 = load double, ptr %arrayidx16, align 8, !tbaa !10
  %mul17 = fmul nsz contract double %10, %11
  %add18 = fadd nsz contract double %mul17, 1.000000e+00
  %arrayidx19 = getelementptr inbounds nuw double, ptr %c, i64 %i.094
  store double %add18, ptr %arrayidx19, align 8, !tbaa !10
  br label %for.inc

if.else20:                                        ; preds = %for.body
  %arrayidx22 = getelementptr inbounds nuw double, ptr %e, i64 %i.094
  %12 = load double, ptr %arrayidx22, align 8, !tbaa !10
  %mul24 = fmul nsz contract double %12, %12
  %add25 = fadd nsz contract double %8, %mul24
  store double %add25, ptr %arrayidx1, align 8, !tbaa !10
  %13 = load double, ptr %x, align 8, !tbaa !10
  %cmp28 = fcmp nsz contract ogt double %13, 0.000000e+00
  br i1 %cmp28, label %if.then29, label %if.else36

if.then29:                                        ; preds = %if.else20
  %arrayidx31 = getelementptr inbounds nuw double, ptr %d, i64 %i.094
  %14 = load double, ptr %arrayidx31, align 8, !tbaa !10
  %mul33 = fmul nsz contract double %14, %14
  %add34 = fadd nsz contract double %8, %mul33
  %arrayidx35 = getelementptr inbounds nuw double, ptr %c, i64 %i.094
  store double %add34, ptr %arrayidx35, align 8, !tbaa !10
  br label %for.inc

if.else36:                                        ; preds = %if.else20
  %arrayidx37 = getelementptr inbounds nuw double, ptr %c, i64 %i.094
  %15 = load double, ptr %arrayidx37, align 8, !tbaa !10
  %add41 = fadd nsz contract double %mul24, %15
  store double %add41, ptr %arrayidx37, align 8, !tbaa !10
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else36, %if.then29
  %inc = add nuw nsw i64 %i.094, 1
  %exitcond.not = icmp eq i64 %inc, %LEN_1D
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !12
}

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable vscale_range(1,16) "frame-pointer"="non-leaf-no-reserve" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="neoverse-v2" "target-features"="+bf16,+bti,+ccidx,+complxnum,+crc,+dit,+dotprod,+ete,+flagm,+fp-armv8,+fp16fml,+fpac,+fullfp16,+i8mm,+jsconv,+lse,+mte,+neon,+pauth,+perfmon,+predres,+rand,+ras,+rcpc,+rdm,+sb,+spe,+ssbs,+sve,+sve-bitperm,+sve2,+trbe,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+v9a,-fmv" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}
!llvm.errno.tbaa = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 4}
!5 = !{!"Homebrew clang version 22.1.3"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"double", !8, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
