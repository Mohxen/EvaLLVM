; ModuleID = 'simple_module'
source_filename = "simple_module"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @add(i32 %a, i32 %b) {
entry:
  %addtmp = add i32 %a, %b
  ret i32 %addtmp
}

define i32 @main() {
entry:
  %sum = call i32 @add(i32 10, i32 20)
  %casted = bitcast [4 x i8]* @.str to i8*
  call i32 (i8*, ...) @printf(i8* %casted, i32 %sum)
  ret i32 0
}

