; ModuleID = 's2710_hoist.c'
source_filename = "s2710_hoist.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable vscale_range(1,16)
define void @tsvc_2_s2710_fp64(ptr noalias noundef captures(none) %a, ptr noalias noundef captures(none) %b, ptr noalias noundef captures(none) %c, ptr noalias noundef readonly captures(none) %d, ptr noalias noundef readonly captures(none) %e, ptr noalias noundef readonly captures(none) %x, i64 noundef %LEN_1D) local_unnamed_addr #0 {
entry:
  %cmp93 = icmp sgt i64 %LEN_1D, 0
  br i1 %cmp93, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %0 = load double, ptr %x, align 8, !tbaa !10
  %.fr = freeze double %0
  %cmp28 = fcmp nsz contract ogt double %.fr, 0.000000e+00
  %cmp8 = icmp samesign ugt i64 %LEN_1D, 10
  %1 = tail call i64 @llvm.vscale.i64()
  %2 = shl nuw nsw i64 %1, 1
  br i1 %cmp8, label %for.body.lr.ph.split.us, label %for.body.lr.ph.split

for.body.lr.ph.split.us:                          ; preds = %for.body.lr.ph
  %min.iters.check170 = icmp ult i64 %LEN_1D, %2
  br i1 %cmp28, label %for.body.us.us.preheader, label %for.body.us.preheader

for.body.us.preheader:                            ; preds = %for.body.lr.ph.split.us
  br i1 %min.iters.check170, label %for.body.us.preheader188, label %vector.ph158

vector.ph158:                                     ; preds = %for.body.us.preheader
  %3 = tail call i64 @llvm.vscale.i64()
  %4 = shl nuw nsw i64 %3, 1
  %.not = sub nsw i64 0, %4
  %n.vec = and i64 %LEN_1D, %.not
  br label %vector.body159

vector.body159:                                   ; preds = %vector.body159, %vector.ph158
  %index160 = phi i64 [ 0, %vector.ph158 ], [ %index.next167, %vector.body159 ]
  %5 = getelementptr double, ptr %a, i64 %index160
  %wide.load = load <vscale x 2 x double>, ptr %5, align 8, !tbaa !10
  %6 = getelementptr double, ptr %b, i64 %index160
  %wide.load161 = load <vscale x 2 x double>, ptr %6, align 8, !tbaa !10
  %7 = fcmp nsz contract ogt <vscale x 2 x double> %wide.load, %wide.load161
  %8 = getelementptr double, ptr %c, i64 %index160
  %9 = xor <vscale x 2 x i1> %7, splat (i1 true)
  %10 = getelementptr double, ptr %e, i64 %index160
  %wide.masked.load162 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %10, <vscale x 2 x i1> %9, <vscale x 2 x double> poison), !tbaa !10
  %11 = fmul nsz contract <vscale x 2 x double> %wide.masked.load162, %wide.masked.load162
  %12 = fadd nsz contract <vscale x 2 x double> %wide.load, %11
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %12, ptr align 8 %6, <vscale x 2 x i1> %9), !tbaa !10
  %wide.load163 = load <vscale x 2 x double>, ptr %8, align 8, !tbaa !10
  %13 = fadd nsz contract <vscale x 2 x double> %11, %wide.load163
  %14 = getelementptr double, ptr %d, i64 %index160
  %wide.masked.load164 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %14, <vscale x 2 x i1> %7, <vscale x 2 x double> poison), !tbaa !10
  %15 = fmul nsz contract <vscale x 2 x double> %wide.load161, %wide.masked.load164
  %16 = fadd nsz contract <vscale x 2 x double> %wide.load, %15
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %16, ptr align 8 %5, <vscale x 2 x i1> %7), !tbaa !10
  %wide.load165 = load <vscale x 2 x double>, ptr %8, align 8, !tbaa !10
  %17 = fmul nsz contract <vscale x 2 x double> %wide.masked.load164, %wide.masked.load164
  %18 = fadd nsz contract <vscale x 2 x double> %17, %wide.load165
  %predphi166 = select <vscale x 2 x i1> %7, <vscale x 2 x double> %18, <vscale x 2 x double> %13
  store <vscale x 2 x double> %predphi166, ptr %8, align 8, !tbaa !10
  %index.next167 = add nuw i64 %index160, %4
  %19 = icmp eq i64 %index.next167, %n.vec
  br i1 %19, label %middle.block168, label %vector.body159, !llvm.loop !12

middle.block168:                                  ; preds = %vector.body159
  %cmp.n = icmp eq i64 %LEN_1D, %n.vec
  br i1 %cmp.n, label %for.cond.cleanup, label %for.body.us.preheader188

for.body.us.preheader188:                         ; preds = %for.body.us.preheader, %middle.block168
  %i.094.us.ph = phi i64 [ 0, %for.body.us.preheader ], [ %n.vec, %middle.block168 ]
  br label %for.body.us

for.body.us.us.preheader:                         ; preds = %for.body.lr.ph.split.us
  br i1 %min.iters.check170, label %for.body.us.us.preheader187, label %vector.ph171

vector.ph171:                                     ; preds = %for.body.us.us.preheader
  %20 = tail call i64 @llvm.vscale.i64()
  %21 = shl nuw nsw i64 %20, 1
  %.not186 = sub nsw i64 0, %21
  %n.vec173 = and i64 %LEN_1D, %.not186
  br label %vector.body174

vector.body174:                                   ; preds = %vector.body174, %vector.ph171
  %index175 = phi i64 [ 0, %vector.ph171 ], [ %index.next182, %vector.body174 ]
  %22 = getelementptr double, ptr %a, i64 %index175
  %wide.load176 = load <vscale x 2 x double>, ptr %22, align 8, !tbaa !10
  %23 = getelementptr double, ptr %b, i64 %index175
  %wide.load177 = load <vscale x 2 x double>, ptr %23, align 8, !tbaa !10
  %24 = fcmp nsz contract ogt <vscale x 2 x double> %wide.load176, %wide.load177
  %25 = xor <vscale x 2 x i1> %24, splat (i1 true)
  %26 = getelementptr double, ptr %e, i64 %index175
  %wide.masked.load178 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %26, <vscale x 2 x i1> %25, <vscale x 2 x double> poison), !tbaa !10
  %27 = fmul nsz contract <vscale x 2 x double> %wide.masked.load178, %wide.masked.load178
  %28 = fadd nsz contract <vscale x 2 x double> %wide.load176, %27
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %28, ptr align 8 %23, <vscale x 2 x i1> %25), !tbaa !10
  %29 = getelementptr double, ptr %d, i64 %index175
  %wide.masked.load179 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %29, <vscale x 2 x i1> %25, <vscale x 2 x double> poison), !tbaa !10
  %30 = fmul nsz contract <vscale x 2 x double> %wide.masked.load179, %wide.masked.load179
  %31 = fadd nsz contract <vscale x 2 x double> %wide.load176, %30
  %32 = getelementptr double, ptr %c, i64 %index175
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %31, ptr align 8 %32, <vscale x 2 x i1> %25), !tbaa !10
  %wide.masked.load180 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %29, <vscale x 2 x i1> %24, <vscale x 2 x double> poison), !tbaa !10
  %33 = fmul nsz contract <vscale x 2 x double> %wide.load177, %wide.masked.load180
  %34 = fadd nsz contract <vscale x 2 x double> %wide.load176, %33
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %34, ptr align 8 %22, <vscale x 2 x i1> %24), !tbaa !10
  %wide.masked.load181 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %32, <vscale x 2 x i1> %24, <vscale x 2 x double> poison), !tbaa !10
  %35 = fmul nsz contract <vscale x 2 x double> %wide.masked.load180, %wide.masked.load180
  %36 = fadd nsz contract <vscale x 2 x double> %35, %wide.masked.load181
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %36, ptr align 8 %32, <vscale x 2 x i1> %24), !tbaa !10
  %index.next182 = add nuw i64 %index175, %21
  %37 = icmp eq i64 %index.next182, %n.vec173
  br i1 %37, label %middle.block183, label %vector.body174, !llvm.loop !16

middle.block183:                                  ; preds = %vector.body174
  %cmp.n184 = icmp eq i64 %LEN_1D, %n.vec173
  br i1 %cmp.n184, label %for.cond.cleanup, label %for.body.us.us.preheader187

for.body.us.us.preheader187:                      ; preds = %for.body.us.us.preheader, %middle.block183
  %i.094.us.us.ph = phi i64 [ 0, %for.body.us.us.preheader ], [ %n.vec173, %middle.block183 ]
  br label %for.body.us.us

for.body.us.us:                                   ; preds = %for.body.us.us.preheader187, %for.inc.us.us
  %i.094.us.us = phi i64 [ %inc.us.us, %for.inc.us.us ], [ %i.094.us.us.ph, %for.body.us.us.preheader187 ]
  %arrayidx1.us.us = getelementptr inbounds nuw double, ptr %a, i64 %i.094.us.us
  %38 = load double, ptr %arrayidx1.us.us, align 8, !tbaa !10
  %arrayidx2.us.us = getelementptr inbounds nuw double, ptr %b, i64 %i.094.us.us
  %39 = load double, ptr %arrayidx2.us.us, align 8, !tbaa !10
  %cmp3.us.us = fcmp nsz contract ogt double %38, %39
  br i1 %cmp3.us.us, label %if.then.us.us, label %if.else21.us.us

if.else21.us.us:                                  ; preds = %for.body.us.us
  %arrayidx23.us.us = getelementptr inbounds nuw double, ptr %e, i64 %i.094.us.us
  %40 = load double, ptr %arrayidx23.us.us, align 8, !tbaa !10
  %mul25.us.us = fmul nsz contract double %40, %40
  %add26.us.us = fadd nsz contract double %38, %mul25.us.us
  store double %add26.us.us, ptr %arrayidx2.us.us, align 8, !tbaa !10
  %arrayidx31.us.us = getelementptr inbounds nuw double, ptr %d, i64 %i.094.us.us
  %41 = load double, ptr %arrayidx31.us.us, align 8, !tbaa !10
  %mul33.us.us = fmul nsz contract double %41, %41
  %add34.us.us = fadd nsz contract double %38, %mul33.us.us
  %arrayidx35.us.us = getelementptr inbounds nuw double, ptr %c, i64 %i.094.us.us
  store double %add34.us.us, ptr %arrayidx35.us.us, align 8, !tbaa !10
  br label %for.inc.us.us

if.then.us.us:                                    ; preds = %for.body.us.us
  %arrayidx6.us.us = getelementptr inbounds nuw double, ptr %d, i64 %i.094.us.us
  %42 = load double, ptr %arrayidx6.us.us, align 8, !tbaa !10
  %mul.us.us = fmul nsz contract double %39, %42
  %add.us.us = fadd nsz contract double %38, %mul.us.us
  store double %add.us.us, ptr %arrayidx1.us.us, align 8, !tbaa !10
  %arrayidx10.us.us = getelementptr inbounds nuw double, ptr %c, i64 %i.094.us.us
  %43 = load double, ptr %arrayidx10.us.us, align 8, !tbaa !10
  %mul13.us.us = fmul nsz contract double %42, %42
  %add14.us.us = fadd nsz contract double %mul13.us.us, %43
  store double %add14.us.us, ptr %arrayidx10.us.us, align 8, !tbaa !10
  br label %for.inc.us.us

for.inc.us.us:                                    ; preds = %if.then.us.us, %if.else21.us.us
  %inc.us.us = add nuw nsw i64 %i.094.us.us, 1
  %exitcond122.not = icmp eq i64 %inc.us.us, %LEN_1D
  br i1 %exitcond122.not, label %for.cond.cleanup, label %for.body.us.us, !llvm.loop !17

for.body.us:                                      ; preds = %for.body.us.preheader188, %for.inc.us
  %i.094.us = phi i64 [ %inc.us, %for.inc.us ], [ %i.094.us.ph, %for.body.us.preheader188 ]
  %arrayidx1.us = getelementptr inbounds nuw double, ptr %a, i64 %i.094.us
  %44 = load double, ptr %arrayidx1.us, align 8, !tbaa !10
  %arrayidx2.us = getelementptr inbounds nuw double, ptr %b, i64 %i.094.us
  %45 = load double, ptr %arrayidx2.us, align 8, !tbaa !10
  %cmp3.us = fcmp nsz contract ogt double %44, %45
  %arrayidx10.us = getelementptr inbounds nuw double, ptr %c, i64 %i.094.us
  br i1 %cmp3.us, label %if.then.us, label %if.else21.us

if.else21.us:                                     ; preds = %for.body.us
  %arrayidx23.us = getelementptr inbounds nuw double, ptr %e, i64 %i.094.us
  %46 = load double, ptr %arrayidx23.us, align 8, !tbaa !10
  %mul25.us = fmul nsz contract double %46, %46
  %add26.us = fadd nsz contract double %44, %mul25.us
  store double %add26.us, ptr %arrayidx2.us, align 8, !tbaa !10
  %47 = load double, ptr %arrayidx10.us, align 8, !tbaa !10
  %add41.us = fadd nsz contract double %mul25.us, %47
  br label %for.inc.us

if.then.us:                                       ; preds = %for.body.us
  %arrayidx6.us = getelementptr inbounds nuw double, ptr %d, i64 %i.094.us
  %48 = load double, ptr %arrayidx6.us, align 8, !tbaa !10
  %mul.us = fmul nsz contract double %45, %48
  %add.us = fadd nsz contract double %44, %mul.us
  store double %add.us, ptr %arrayidx1.us, align 8, !tbaa !10
  %49 = load double, ptr %arrayidx10.us, align 8, !tbaa !10
  %mul13.us = fmul nsz contract double %48, %48
  %add14.us = fadd nsz contract double %mul13.us, %49
  br label %for.inc.us

for.inc.us:                                       ; preds = %if.then.us, %if.else21.us
  %add14.us.sink = phi double [ %add14.us, %if.then.us ], [ %add41.us, %if.else21.us ]
  store double %add14.us.sink, ptr %arrayidx10.us, align 8, !tbaa !10
  %inc.us = add nuw nsw i64 %i.094.us, 1
  %exitcond121.not = icmp eq i64 %inc.us, %LEN_1D
  br i1 %exitcond121.not, label %for.cond.cleanup, label %for.body.us, !llvm.loop !18

for.body.lr.ph.split:                             ; preds = %for.body.lr.ph
  %active.lane.mask.entry145 = tail call <vscale x 2 x i1> @llvm.get.active.lane.mask.nxv2i1.i64(i64 0, i64 %LEN_1D)
  br i1 %cmp28, label %vector.body146, label %vector.body

vector.body:                                      ; preds = %for.body.lr.ph.split, %vector.body
  %index = phi i64 [ %index.next, %vector.body ], [ 0, %for.body.lr.ph.split ]
  %active.lane.mask = phi <vscale x 2 x i1> [ %active.lane.mask.next, %vector.body ], [ %active.lane.mask.entry145, %for.body.lr.ph.split ]
  %50 = getelementptr double, ptr %a, i64 %index
  %wide.masked.load = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %50, <vscale x 2 x i1> %active.lane.mask, <vscale x 2 x double> poison), !tbaa !10
  %51 = getelementptr double, ptr %b, i64 %index
  %wide.masked.load139 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %51, <vscale x 2 x i1> %active.lane.mask, <vscale x 2 x double> poison), !tbaa !10
  %52 = fcmp nsz contract ogt <vscale x 2 x double> %wide.masked.load, %wide.masked.load139
  %53 = xor <vscale x 2 x i1> %52, splat (i1 true)
  %54 = select <vscale x 2 x i1> %active.lane.mask, <vscale x 2 x i1> %53, <vscale x 2 x i1> zeroinitializer
  %55 = getelementptr double, ptr %e, i64 %index
  %wide.masked.load140 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %55, <vscale x 2 x i1> %54, <vscale x 2 x double> poison), !tbaa !10
  %56 = fmul nsz contract <vscale x 2 x double> %wide.masked.load140, %wide.masked.load140
  %57 = fadd nsz contract <vscale x 2 x double> %wide.masked.load, %56
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %57, ptr align 8 %51, <vscale x 2 x i1> %54), !tbaa !10
  %58 = getelementptr double, ptr %c, i64 %index
  %wide.masked.load141 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %58, <vscale x 2 x i1> %54, <vscale x 2 x double> poison), !tbaa !10
  %59 = fadd nsz contract <vscale x 2 x double> %56, %wide.masked.load141
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %59, ptr align 8 %58, <vscale x 2 x i1> %54), !tbaa !10
  %60 = select <vscale x 2 x i1> %active.lane.mask, <vscale x 2 x i1> %52, <vscale x 2 x i1> zeroinitializer
  %61 = getelementptr double, ptr %d, i64 %index
  %wide.masked.load142 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %61, <vscale x 2 x i1> %60, <vscale x 2 x double> poison), !tbaa !10
  %62 = fmul nsz contract <vscale x 2 x double> %wide.masked.load139, %wide.masked.load142
  %63 = fadd nsz contract <vscale x 2 x double> %wide.masked.load, %62
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %63, ptr align 8 %50, <vscale x 2 x i1> %60), !tbaa !10
  %wide.masked.load143 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %55, <vscale x 2 x i1> %60, <vscale x 2 x double> poison), !tbaa !10
  %64 = fmul nsz contract <vscale x 2 x double> %wide.masked.load142, %wide.masked.load143
  %65 = fadd nsz contract <vscale x 2 x double> %64, splat (double 1.000000e+00)
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %65, ptr align 8 %58, <vscale x 2 x i1> %60), !tbaa !10
  %index.next = add i64 %index, %2
  %active.lane.mask.next = tail call <vscale x 2 x i1> @llvm.get.active.lane.mask.nxv2i1.i64(i64 %index.next, i64 %LEN_1D)
  %66 = extractelement <vscale x 2 x i1> %active.lane.mask.next, i64 0
  br i1 %66, label %vector.body, label %for.cond.cleanup, !llvm.loop !19

vector.body146:                                   ; preds = %for.body.lr.ph.split, %vector.body146
  %index147 = phi i64 [ %index.next155, %vector.body146 ], [ 0, %for.body.lr.ph.split ]
  %active.lane.mask148 = phi <vscale x 2 x i1> [ %active.lane.mask.next156, %vector.body146 ], [ %active.lane.mask.entry145, %for.body.lr.ph.split ]
  %67 = getelementptr double, ptr %a, i64 %index147
  %wide.masked.load149 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %67, <vscale x 2 x i1> %active.lane.mask148, <vscale x 2 x double> poison), !tbaa !10
  %68 = getelementptr double, ptr %b, i64 %index147
  %wide.masked.load150 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %68, <vscale x 2 x i1> %active.lane.mask148, <vscale x 2 x double> poison), !tbaa !10
  %69 = fcmp nsz contract ogt <vscale x 2 x double> %wide.masked.load149, %wide.masked.load150
  %70 = xor <vscale x 2 x i1> %69, splat (i1 true)
  %71 = select <vscale x 2 x i1> %active.lane.mask148, <vscale x 2 x i1> %70, <vscale x 2 x i1> zeroinitializer
  %72 = getelementptr double, ptr %e, i64 %index147
  %wide.masked.load151 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %72, <vscale x 2 x i1> %71, <vscale x 2 x double> poison), !tbaa !10
  %73 = fmul nsz contract <vscale x 2 x double> %wide.masked.load151, %wide.masked.load151
  %74 = fadd nsz contract <vscale x 2 x double> %wide.masked.load149, %73
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %74, ptr align 8 %68, <vscale x 2 x i1> %71), !tbaa !10
  %75 = getelementptr double, ptr %d, i64 %index147
  %wide.masked.load152 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %75, <vscale x 2 x i1> %71, <vscale x 2 x double> poison), !tbaa !10
  %76 = fmul nsz contract <vscale x 2 x double> %wide.masked.load152, %wide.masked.load152
  %77 = fadd nsz contract <vscale x 2 x double> %wide.masked.load149, %76
  %78 = select <vscale x 2 x i1> %active.lane.mask148, <vscale x 2 x i1> %69, <vscale x 2 x i1> zeroinitializer
  %wide.masked.load153 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %75, <vscale x 2 x i1> %78, <vscale x 2 x double> poison), !tbaa !10
  %79 = fmul nsz contract <vscale x 2 x double> %wide.masked.load150, %wide.masked.load153
  %80 = fadd nsz contract <vscale x 2 x double> %wide.masked.load149, %79
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %80, ptr align 8 %67, <vscale x 2 x i1> %78), !tbaa !10
  %wide.masked.load154 = tail call <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr align 8 %72, <vscale x 2 x i1> %78, <vscale x 2 x double> poison), !tbaa !10
  %81 = fmul nsz contract <vscale x 2 x double> %wide.masked.load153, %wide.masked.load154
  %82 = fadd nsz contract <vscale x 2 x double> %81, splat (double 1.000000e+00)
  %predphi = select <vscale x 2 x i1> %69, <vscale x 2 x double> %82, <vscale x 2 x double> %77
  %83 = getelementptr inbounds nuw double, ptr %c, i64 %index147
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %predphi, ptr align 8 %83, <vscale x 2 x i1> %active.lane.mask148), !tbaa !10
  %index.next155 = add i64 %index147, %2
  %active.lane.mask.next156 = tail call <vscale x 2 x i1> @llvm.get.active.lane.mask.nxv2i1.i64(i64 %index.next155, i64 %LEN_1D)
  %84 = extractelement <vscale x 2 x i1> %active.lane.mask.next156, i64 0
  br i1 %84, label %vector.body146, label %for.cond.cleanup, !llvm.loop !20

for.cond.cleanup:                                 ; preds = %vector.body, %vector.body146, %for.inc.us, %for.inc.us.us, %middle.block168, %middle.block183, %entry
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.vscale.i64() #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <vscale x 2 x i1> @llvm.get.active.lane.mask.nxv2i1.i64(i64, i64) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: read)
declare <vscale x 2 x double> @llvm.masked.load.nxv2f64.p0(ptr captures(none), <vscale x 2 x i1>, <vscale x 2 x double>) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: write)
declare void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double>, ptr captures(none), <vscale x 2 x i1>) #3

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable vscale_range(1,16) "frame-pointer"="non-leaf-no-reserve" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="neoverse-v2" "target-features"="+bf16,+bti,+ccidx,+complxnum,+crc,+dit,+dotprod,+ete,+flagm,+fp-armv8,+fp16fml,+fpac,+fullfp16,+i8mm,+jsconv,+lse,+mte,+neon,+pauth,+perfmon,+predres,+rand,+ras,+rcpc,+rdm,+sb,+spe,+ssbs,+sve,+sve-bitperm,+sve2,+trbe,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+v9a,-fmv" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: read) }
attributes #3 = { nocallback nofree nosync nounwind willreturn memory(argmem: write) }

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
!12 = distinct !{!12, !13, !14, !15}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!"llvm.loop.isvectorized", i32 1}
!15 = !{!"llvm.loop.unroll.runtime.disable"}
!16 = distinct !{!16, !13, !14, !15}
!17 = distinct !{!17, !13, !15, !14}
!18 = distinct !{!18, !13, !15, !14}
!19 = distinct !{!19, !13, !14, !15}
!20 = distinct !{!20, !13, !14, !15}
