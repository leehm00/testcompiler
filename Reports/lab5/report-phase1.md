# Lab5 实验报告-阶段一

小组成员 

| 姓名   | 学号       |
| ------ | ---------- |
| 李泓民 | PB18071495 |
| 单航   | PB18111747 |

## 实验要求

请按照自己的理解，写明本次实验需要干什么

阅读关于`LoopSearch`和`Mem2Reg`部分的说明，掌握如何去调用`LightIR`的接口来开发优化Pass，最后该部分需要在实验报告中写下**思考题**的答案。

## 思考题
### LoopSearch
1. 循环的入口如何确定？循环的入口的数量可能超过1嘛？

用图强连通分量搜索算法找到每个强连通图的循环基本点。

不可能。

2. 简述一下算法怎么解决循环嵌套的情况。

先找到所有的循环，BasicBlock* get_loop_base(BBset_t **loop*)函数的loop2base用来保存base，BBset_t \*get_inner_loop(BasicBlock\* *bb*)得到嵌套深度最深的loop（即得到bb所在最低层次的loop），std::unordered_set<BBset_t *> get_loops_in_func(Function *f)得到函数 f 里的所有循环，然后将循环建立一个树（多个循环就建立森林），解决嵌套。

### Mem2reg
1. 请简述支配边界的概念。

直观理解就是当前结点所能支配的边界（不包括该边界）。

当一个点A到点B在控制流程图中，如果A没有直接支配着B但是支配着一个B的前驱，则节点B就是点A的支配边界（有可能点A是点B的前置程序，那么，因为任何一个点都支配着自己，点A也支配着自己，所以点A也是点B的支配边界），从A的观点来看，还有点在其他没有经过A的控制路径，可以使他们更早出现。

2. 请简述`phi`节点的概念，与其存在的意义。

SSA中由于一个变量只能被赋值一次，最后要用到之前多次赋值的变量时通过添加一个`phi`节点，选择该使用哪个版本的该变量。

如a赋值两次就有a1和a2，后面执行如果有`b=a`这样的用到a的值的时候就要使用`phi`节点选择使用a1还是a2。

就是用来解决SSA单次赋值的控制流的问题的。

3. 请描述`Mem2Reg Pass`执行前后的`ir`的变化, 简述一下。

   > before `Mem2Reg`：
   >
   > ```c
   > ; ModuleID = 'cminus'
   > source_filename = "complex3.cminus"
   > 
   > declare i32 @input()
   > 
   > declare void @output(i32)
   > 
   > declare void @outputFloat(float)
   > 
   > declare void @neg_idx_except()
   > 
   > define i32 @gcd(i32 %arg0, i32 %arg1) {
   > label_entry:
   >   %op2 = alloca i32
   >   store i32 %arg0, i32* %op2
   >   %op3 = alloca i32
   >   store i32 %arg1, i32* %op3
   >   %op4 = load i32, i32* %op3
   >   %op5 = icmp eq i32 %op4, 0
   >   %op6 = zext i1 %op5 to i32
   >   %op7 = icmp ne i32 %op6, 0
   >   br i1 %op7, label %label8, label %label10
   > label8:                                                ; preds = %label_entry
   >   %op9 = load i32, i32* %op2
   >   ret i32 %op9
   > label10:                                                ; preds = %label_entry
   >   %op11 = load i32, i32* %op3
   >   %op12 = load i32, i32* %op2
   >   %op13 = load i32, i32* %op2
   >   %op14 = load i32, i32* %op3
   >   %op15 = sdiv i32 %op13, %op14
   >   %op16 = load i32, i32* %op3
   >   %op17 = mul i32 %op15, %op16
   >   %op18 = sub i32 %op12, %op17
   >   %op19 = call i32 @gcd(i32 %op11, i32 %op18)
   >   ret i32 %op19
   > }
   > define void @main() {
   > label_entry:
   >   %op0 = alloca i32
   >   %op1 = alloca i32
   >   %op2 = alloca i32
   >   %op3 = call i32 @input()
   >   store i32 %op3, i32* %op0
   >   %op4 = call i32 @input()
   >   store i32 %op4, i32* %op1
   >   %op5 = load i32, i32* %op0
   >   %op6 = load i32, i32* %op1
   >   %op7 = icmp slt i32 %op5, %op6
   >   %op8 = zext i1 %op7 to i32
   >   %op9 = icmp ne i32 %op8, 0
   >   br i1 %op9, label %label10, label %label14
   > label10:                                                ; preds = %label_entry
   >   %op11 = load i32, i32* %op0
   >   store i32 %op11, i32* %op2
   >   %op12 = load i32, i32* %op1
   >   store i32 %op12, i32* %op0
   >   %op13 = load i32, i32* %op2
   >   store i32 %op13, i32* %op1
   >   br label %label14
   > label14:                                                ; preds = %label_entry, %label10
   >   %op15 = load i32, i32* %op0
   >   %op16 = load i32, i32* %op1
   >   %op17 = call i32 @gcd(i32 %op15, i32 %op16)
   >   store i32 %op17, i32* %op2
   >   %op18 = load i32, i32* %op2
   >   call void @output(i32 %op18)
   >   ret void
   > }
   > ```
   >
   > After `Mem2Reg`：
   >
   > ```c
   > ; ModuleID = 'cminus'
   > source_filename = "complex3.cminus"
   > 
   > declare i32 @input()
   > 
   > declare void @output(i32)
   > 
   > declare void @outputFloat(float)
   > 
   > declare void @neg_idx_except()
   > 
   > define i32 @gcd(i32 %arg0, i32 %arg1) {
   > label_entry:
   >   %op5 = icmp eq i32 %arg1, 0
   >   %op6 = zext i1 %op5 to i32
   >   %op7 = icmp ne i32 %op6, 0
   >   br i1 %op7, label %label8, label %label10
   > label8:                                                ; preds = %label_entry
   >   ret i32 %arg0
   > label10:                                                ; preds = %label_entry
   >   %op15 = sdiv i32 %arg0, %arg1
   >   %op17 = mul i32 %op15, %arg1
   >   %op18 = sub i32 %arg0, %op17
   >   %op19 = call i32 @gcd(i32 %arg1, i32 %op18)
   >   ret i32 %op19
   > }
   > define void @main() {
   > label_entry:
   >   %op3 = call i32 @input()
   >   %op4 = call i32 @input()
   >   %op7 = icmp slt i32 %op3, %op4
   >   %op8 = zext i1 %op7 to i32
   >   %op9 = icmp ne i32 %op8, 0
   >   br i1 %op9, label %label10, label %label14
   > label10:                                                ; preds = %label_entry
   >   br label %label14
   > label14:                                                ; preds = %label_entry, %label10
   >   %op19 = phi i32 [ %op3, %label10 ], [ undef, %label_entry ]
   >   %op20 = phi i32 [ %op4, %label_entry ], [ %op3, %label10 ]
   >   %op21 = phi i32 [ %op3, %label_entry ], [ %op4, %label10 ]
   >   %op17 = call i32 @gcd(i32 %op21, i32 %op20)
   >   call void @output(i32 %op17)
   >   ret void
   > }
   > ```

使用SSA，插入了phi节点，还执行了相关的优化，删除了多余的语句(如alloca这些)。

4. 在放置phi节点的时候，算法是如何利用支配树的信息的？

算法得到了所有的全局变量和对应的节点，使用支配边界的信息来放置phi节点。

5. 算法是如何选择`value`(变量最新的值)来替换`load`指令的？（描述数据结构与维护方法）

维护一个全局变量，先将最新的对应左值压入栈中，然后用栈顶代替原值。

### 代码阅读总结

此次实验有什么收获

和课上的理论知识结合，支配和支配边界这些，理解了相应的在编译器的执行和实现。

学习了如何去调用`LightIR`的接口来开发优化Pass:

- LoopSearch是后续的循环不变式外提优化的基础。
- `Mem2Reg Pass`构造了LLVM IR 的SSA格式(静态单赋值格式)。

### 实验反馈 （可选 不会评分）

对本次实验的建议

其实在wiki上的SSA解释可以搬下来，还有一篇博客写的也不错https://blog.csdn.net/liumf2005/article/details/8690377

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
