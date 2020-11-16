; ModuleID = 'while.c'
source_filename = "while.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 10, i32* %2, align 4;a=10赋值操作
  store i32 0, i32* %3, align 4;i=0赋值操作
  br label %4;跳转到while条件判断

4:                                                ; preds = %7, %0循环判断i<10是否成立
  %5 = load i32, i32* %3, align 4;取i的值
  %6 = icmp slt i32 %5, 10;比较取值
  br i1 %6, label %7, label %12;满足则执行while内内容(label7)for则跳出循环

7:                                                ; preds = %4在i<10成立的情况下,跳转到这个label,执行本语句块
  %8 = load i32, i32* %3, align 4;取i的值
  %9 = add nsw i32 %8, 1;执行i+1
  store i32 %9, i32* %3, align 4;存回i
  %10 = load i32, i32* %2, align 4
  %11 = add nsw i32 %10, %9;执行a+i
  store i32 %11, i32* %2, align 4;存回a
  br label %4;每次执行结束跳回label4重新判断

12:                                               ; preds = %4跳出循环之后执行的return a
  %13 = load i32, i32* %2, align 4
  ret i32 %13
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
