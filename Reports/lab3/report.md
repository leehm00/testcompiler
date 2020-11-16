# lab3 实验报告
学号PB18071495 姓名李泓民

## 问题1: cpp与.ll的对应
请描述你的cpp代码片段和.ll的每个BasicBlock的对应关系。描述中请附上两者代码。

这里用的是cpp直接生成的.ll文件而不是自己手写的.ll文件.

每个文件都会开始设置一个寄存器存0的值.

### assign

源文件如下:

```c
int main(){
  int a[10];
  a[0] = 10;
  a[1] = a[0] * 2;
  return a[1];
}
```

.ll文件源代码如下:

```c
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca [10 x i32]
  %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
  store i32 10, i32* %2
  %3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
  %4 = load i32, i32* %3
  %5 = mul i32 %4, 2
  %6 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
  store i32 %5, i32* %6
  %7 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
  %8 = load i32, i32* %7
  ret i32 %8
}
```

对应的语句都写到了注释里面,一般都包含了上一条注释到这一条之间的几条ll语句.因为是顺序执行没有分支跳转,所以只有一个语句块.

```c++
//进入main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
```

对应于`int main()`.

```c
define i32 @main()
```



```c++
//在内存中为a分配空间
    auto *arrayType = ArrayType::get(Int32Type, 10);
    auto aArray = builder->create_alloca(arrayType);
```

 对应于`int a[10];`.

```c
%1 = alloca [10 x i32]
```



```C++
//为a[0]赋值10
    auto A_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), A_gep);
```

对应于`a[0] = 10;`.

```c
  %0 = alloca i32
  store i32 0, i32* %0
  %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
  store i32 10, i32* %2
```



```C++
//为a[1]赋值,计算了a[0]*2
    auto A1_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(0)});
    auto a0Load = builder->create_load(A1_gep);//取a[0]值
    auto mul = builder->create_imul(a0Load, CONST_INT(2));//a[0]*2
    auto A2_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(1)});
    builder->create_store(mul, A2_gep);//存a[1]值
```

对应于`a[1] = a[0] * 2;`.

```c
  ;取a[0]值
  %3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
  %4 = load i32, i32* %3
  ;计算a[0]*2
  %5 = mul i32 %4, 2
  ;取a[1]地址并存入值
  %6 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
  store i32 %5, i32* %6
```



```C++
//返回值的块
    auto A3_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(1)});
    auto call = builder->create_load(A3_gep);//取出了a[1]值,返回值修改为a[1]
    builder->create_ret(call);//返回语句
```

对应于`return a[1];`.

```c
  %7 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
  %8 = load i32, i32* %7
  ret i32 %8
```



### fun

源文件如下:

```c
int callee(int a){
  return 2 * a;
}
int main(){
  return callee(110);
}
```

.ll源代码如下:

```c
define i32 @callee(i32 %0) {
entry:
  %1 = alloca i32
  %2 = alloca i32
  store i32 0, i32* %1
  store i32 %0, i32* %2
  %3 = load i32, i32* %2
  %4 = mul i32 %3, 2
  store i32 %4, i32* %1
  %5 = load i32, i32* %1
  ret i32 %5
}
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca i32
  store i32 110, i32* %1
  %2 = load i32, i32* %1
  %3 = call i32 @callee(i32 %2)
  ret i32 %3
}

```

也是具体每条语句对应关系写到了注释里面.

```c++
//开始定义callee函数
    std::vector<Type *> Ints(1, Int32Type);
    auto calleeFunTy = FunctionType::get(Int32Type, Ints);
    auto calleeFun = Function::create(calleeFunTy, "callee", module);
    auto bbcallee = BasicBlock::create(module, "entry", calleeFun);

    builder->set_insert_point(bbcallee);

    auto retcalleeAlloca = builder->create_alloca(Int32Type);
    auto aAlloca = builder->create_alloca(Int32Type);
```

此块对应于`int callee(int a)`,但是不包含`int a`参数.

```c
define i32 @callee(i32 %0)
```



```c++
//callee函数的参数定义和存放
    std::vector<Value *> args;
    for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
        args.push_back(*arg);
    }
    builder->create_store(CONST_INT(0), retcalleeAlloca);
    builder->create_store(args[0], aAlloca);
```

此块对应于`int a`这个参数的定义.

```c
  %1 = alloca i32
  %2 = alloca i32
  store i32 0, i32* %1
  store i32 %0, i32* %2
```



```C++
//进入callee函数体开始执行
    auto aLoad = builder->create_load(aAlloca);
    auto mul = builder->create_imul(aLoad, CONST_INT(2));//计算2*a
    builder->create_store(mul, retcalleeAlloca);
```

对应于`2 * a`计算,进入了callee函数体.

```c
  %3 = load i32, i32* %2
  %4 = mul i32 %3, 2
  store i32 %4, i32* %1
```



```c++
//返回值设定为2*a的结果
    auto retLoad = builder->create_load(retcalleeAlloca);
    builder->create_ret(retLoad);
```

对应于`return 2 * a;`,callee的返回值.

```c
  %5 = load i32, i32* %1
  ret i32 %5
```



```C++
//开始定义main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
```

对应于`int main()`.

```c
define i32 @main()
```



```C++
//调用函数
    auto constAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(110), constAlloca);
    auto constLoad = builder->create_load(constAlloca);
    auto call = builder->create_call(calleeFun, {constLoad});
```

对应于对callee函数的调用`callee(110)`.

```c
  %0 = alloca i32
  store i32 0, i32* %0
  ;存110的值
  %1 = alloca i32
  store i32 110, i32* %1
  %2 = load i32, i32* %1
  ;调用函数
  %3 = call i32 @callee(i32 %2)
```



```C++
//最后的返回语句
    builder->create_ret(call);
```

对应于`return callee(110);`中的`return`.

```c
  ret i32 %3
```



### if

源文件如下:

```c
int main(){
  float a = 5.555;
  if(a > 1)
    return 233;
  return 0;
}

```

.ll文件源代码如下:

```c
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca float
  store float 0x40163851e0000000, float* %1
  %2 = load float, float* %1
  %3 = fcmp ugt float %2,0x3ff0000000000000
  br i1 %3, label %trueBB, label %falseBB
trueBB:
  store i32 233, i32* %0
  br label %4
falseBB:
  br label %4
4:
  %5 = load i32, i32* %0
  ret i32 %5
}
```

每条语句的对应关系写在了注释里面.

```c++
  //开始定义main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
```

以上对应于`int main()`.

```c
define i32 @main() 
```



```c++
 auto aAlloca = builder->create_alloca(FloatType);//在内存中分配a的位置
    builder->create_store(CONST_FP(5.555), aAlloca);//赋值a = 5.555
```

这里开始赋值`float a = 5.555;`.

```c
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca float
  store float 0x40163851e0000000, float* %1
```



```c++
    auto aLoad = builder->create_load(aAlloca);//取出参数a的值
    auto fcmp = builder->create_fcmp_gt(aLoad, CONST_FP(1.0));//if的判断,a>1
    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);//条件成立的分支
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);//条件不成立的分支
    auto retA = BasicBlock::create(module, "", mainFun);//返回语句
    auto br = builder->create_cond_br(fcmp, trueBB, falseBB);//根据fcmp的值跳转
```

这里为`if(a > 1)`的判断,并设定了相应的条件成立和不成立语句块.

```c
  %2 = load float, float* %1
  %3 = fcmp ugt float %2,0x3ff0000000000000
  br i1 %3, label %trueBB, label %falseBB
```



```c++
//条件成立的分支
    builder->set_insert_point(trueBB);
    builder->create_store(CONST_INT(233), retAlloca);//修改返回值为233
    builder->create_br(retA);//跳转到返回语句
```

这里为条件成立执行的语句块`return 233`,修改返回值为233.(之前有默认设置返回值为0,见cpp文件)

```c
trueBB:
  store i32 233, i32* %0
  br label %4
```



```c++
//条件不成立的分支
    builder->set_insert_point(falseBB);
    builder->create_br(retA);//对返回值不做修改,直接返回到返回语句
```

这里为条件成立执行的语句块`return 0`,不修改返回值(之前有默认设置返回值为0,见cpp文件)

```c
falseBB:
  br label %4
```



```C++
//返回语句
    builder->set_insert_point(retA);
    auto call = builder->create_load(retAlloca);
    builder->create_ret(call);
```

最后的返回模块,返回233和0中的一个,根据前面的分支赋值决定.

```c
4:
  %5 = load i32, i32* %0
  ret i32 %5
```



### while

源文件如下:

```c
int main(){
  int a;
  int i;
  a = 10;
  i = 0;
  while(i < 10){
    i = i + 1;
    a = a + i;
  }
  return a;
}

```

.ll文件源代码如下:

```c
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca i32
  %2 = alloca i32
  store i32 10, i32* %1
  store i32 0, i32* %2
  br label %judge_i
judge_i:
  %3 = load i32, i32* %2
  %4 = icmp slt i32 %3, 10
  br i1 %4, label %true_i, label %retA
true_i:
  %5 = load i32, i32* %2
  %6 = add i32 %5, 1
  store i32 %6, i32* %2
  %7 = load i32, i32* %2
  %8 = load i32, i32* %1
  %9 = add i32 %7, %8
  store i32 %9, i32* %1
  br label %judge_i
retA:
  %10 = load i32, i32* %1
  ret i32 %10
}

```

每条语句的对应关系写在了注释里面.

```c++
//开始定义main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
```

以上对应于`int main()`.

```c
define i32 @main()
```



```c++
auto aAlloca = builder->create_alloca(Int32Type);//分配a的空间
    auto iAlloca = builder->create_alloca(Int32Type);//分配i的空间
    builder->create_store(CONST_INT(10), aAlloca);//赋值a = 10
    builder->create_store(CONST_INT(0), iAlloca);//赋值i = 0
```

对应于以下

```c
    int a;
    int i;
	a = 10;
  	i = 0;
```

完成了分配空间和赋值的工作.

```c
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca i32
  %2 = alloca i32
  store i32 10, i32* %1
  store i32 0, i32* %2
```



```C++
	auto judge_i = BasicBlock::create(module, "judge_i", mainFun);
    auto true_i = BasicBlock::create(module, "true_i", mainFun);//条件成立
    auto retA = BasicBlock::create(module, "retA", mainFun);//不成立直接返回
    builder->create_br(judge_i);
    //while判断条件
    builder->set_insert_point(judge_i);
    auto iLoad = builder->create_load(iAlloca);
    auto icmp = builder->create_icmp_lt(iLoad, CONST_INT(10));
```

对应于循环体的条件`while(i < 10)`,并且设定了相对应的循环体和跳出循环体的标签.

```c
  br label %judge_i
judge_i:
  %3 = load i32, i32* %2
  %4 = icmp slt i32 %3, 10
  br i1 %4, label %true_i, label %retA
```



```c
//条件成立进入循环体内部执行
    auto br = builder->create_cond_br(icmp, true_i, retA);
    builder->set_insert_point(true_i);
    auto iLoadinWhile = builder->create_load(iAlloca);//取i值
    auto iAddone = builder->create_iadd(iLoadinWhile, CONST_INT(1));//i+1
    builder->create_store(iAddone, iAlloca);//存i+1值
    auto iLoadAdda = builder->create_load(iAlloca);//取i值
    auto aLoad = builder->create_load(aAlloca);//取a值
    auto iAdda = builder->create_iadd(iLoadAdda, aLoad);//a+i
    builder->create_store(iAdda, aAlloca);//存a值
    builder->create_br(judge_i);//返回while条件判断
```

对应于以下

```c
	i = i + 1;
    a = a + i;
```

具体计算步骤在注释里面.需要注意的是最后一句`builder->create_br(judge_i);//返回while条件判断`,必须有这一条返回原来的while条件判断,对应于`br label %judge_i`.

```c
true_i:
  %5 = load i32, i32* %2
  %6 = add i32 %5, 1
  store i32 %6, i32* %2
  %7 = load i32, i32* %2
  %8 = load i32, i32* %1
  %9 = add i32 %7, %8
  store i32 %9, i32* %1
  br label %judge_i
```



```c++
//返回语句,返回值是a
    builder->set_insert_point(retA);
    auto call = builder->create_load(aAlloca);
    builder->create_ret(call);
```

对应于`return a;`,返回值改成a.

```c
retA:
  %10 = load i32, i32* %1
  ret i32 %10
```



## 问题2: Visitor Pattern

请指出visitor.cpp中，`treeVisitor.visit(exprRoot)`执行时，以下几个Node的遍历序列:numberA、numberB、exprC、exprD、exprE、numberF、exprRoot。  
序列请按如下格式指明：  
exprRoot->numberF->exprE- >exprD->numberB->numberA->exprC->numberA->numberB

## 问题3: getelementptr
请给出`IR.md`中提到的两种getelementptr用法的区别,并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 
  - `%2 = getelementptr i32, i32* %1 i32 %0` 

一个是i32数组指针,一个是i32指针.

`%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 中,[10 x i32]是计算类型,[10 x i32]\*是指针类型,%1是指针,i32 0,i32 %0是数组的偏移类型和偏移值.

`%2 = getelementptr i32, i32* %1 i32 %0` 中,i32是计算类型,i32\*是指针类型,%1 是指针.

## 实验难点
描述在实验中遇到的问题、分析和解决方案

### 其他问题

#### 关于寄存器编号的问题

##### 问题来源

注意到函数里面的寄存器分配,假如有参数%0,那么第一个变量要从%2开始.先附上问题的图片.

![](figs/1.1.png)

这样是正确的.

而如果把此处的%2改成%1就会报错.

![](figs/1.2.png)

##### 寻找标准文档

附上参考文档的原文.

> A function definition contains a list of basic blocks, forming the CFG (Control Flow Graph) for the function. Each basic block may optionally start with a label (giving the basic block a symbol table entry), contains a list of instructions, and ends with a [terminator](http://llvm.org/docs/LangRef.html#terminators) instruction (such as a branch or function return). If an explicit label name is not provided, a block is assigned an implicit numbered label, using the next value from the same counter as used for unnamed temporaries ([see above](http://llvm.org/docs/LangRef.html#identifiers)). **For example, if a function entry block does not have an explicit label, it will be assigned label “%0”, then the first unnamed temporary in that block will be “%1”, etc.** If a numeric label is explicitly specified, it must match the numeric label that would be used implicitly.

粗体的话大概是这个意思:如果函数输入块没有显式标签，则将为其分配标签“％0”，然后该块中的第一个未命名临时名称将是“％1”，依此类推.

##### 个人理解

我的理解是%0会预留给块作为隐藏的编号,在图片的例子里面有%0参数,所以将%1作为预留的编号标签,记录进入函数的位置.

##### 另外的验证

用main函数作为再次实验,一般第一个寄存器都是直接编号%1,假如从%0开始的话,也会报错

![](figs/1.3.png)

这也是因为%0被预留了.

同理,就和main函数里面是从%1寄存器开始用一样的,main一般是没有传入的参数,所以编号都是%1开始
![1.3](figs/1.3.png)
假如这里用了%0开始的话就也会报错,%0被预留了.

##### 由cpp生成的.ll文件的解释

因为用了entry标签,所以不再占用一个寄存器存放进入函数的位置.

如下:

```c
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca float
  store float 0x40163851e0000000, float* %1
  %2 = load float, float* %1
  %3 = fcmp ugt float %2,0x3ff0000000000000
  br i1 %3, label %trueBB, label %falseBB
trueBB:
  store i32 233, i32* %0
  br label %4
falseBB:
  br label %4
4:
  %5 = load i32, i32* %0
  ret i32 %5
}

```



#### gcd_array_generator.cpp中的问题

```c++
  auto x0GEP = builder->create_gep(x, {CONST_INT(0), CONST_INT(0)});  // GEP: 这里为什么是{0, 0}呢? (实验报告相关)
  builder->create_store(CONST_INT(90), x0GEP);
  auto y0GEP = builder->create_gep(y, {CONST_INT(0), CONST_INT(0)});  // GEP: 这里为什么是{0, 0}呢? (实验报告相关)
  builder->create_store(CONST_INT(18), y0GEP);

  x0GEP = builder->create_gep(x, {CONST_INT(0), CONST_INT(0)});
  y0GEP = builder->create_gep(y, {CONST_INT(0), CONST_INT(0)});
  call = builder->create_call(funArrayFun, {x0GEP, y0GEP});           // 为什么这里传的是{x0GEP, y0GEP}呢?
```

其实都相当于是一个问题,数组的0号元素的地址就是数组的首地址,x,y和x[0],y[0]的地址是相同的.

## 实验反馈
没啥大问题,其实感觉把C++的一些东西再铺垫一下比较好,看着容器就吓着了,不会用,查了好久资料丢失发现用的不是很多,就是个定义23333.