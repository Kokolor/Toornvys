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
define noundef i32 @main() local_unnamed_addr #0 {
entry:
  %0 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 17)
  %1 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @0, i32 7)
  ret i32 7
}

attributes #0 = { nofree nounwind }
