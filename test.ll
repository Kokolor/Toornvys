; ModuleID = 'main_module'
source_filename = "main_module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nofree nounwind
define void @printInt(i32 %0) local_unnamed_addr #0 {
entry:
  %1 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 %0)
  ret void
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #0

; Function Attrs: nofree nounwind
define i32 @add(i32 %test, i32 %x, i32 %y) local_unnamed_addr #0 {
entry:
  %0 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 17)
  %addtmp = add i32 %y, %x
  ret i32 %addtmp
}

; Function Attrs: nofree nounwind
define void @modify(ptr nocapture %val) local_unnamed_addr #0 {
entry:
  %deref = load i32, ptr %val, align 4
  %addtmp = add i32 %deref, 5
  store i32 %addtmp, ptr %val, align 4
  %0 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 %addtmp)
  ret void
}

; Function Attrs: nofree nounwind
define noundef i32 @main() local_unnamed_addr #0 {
entry:
  %0 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 17)
  %1 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 7)
  %2 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 30)
  %3 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 12)
  %4 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 12)
  ret i32 12
}

attributes #0 = { nofree nounwind }
