; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@0 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  ret i32 0
}
