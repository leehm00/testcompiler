# Lab5 实验报告

小组成员  

| 姓名   | 学号       |
| ------ | ---------- |
| 李泓民 | PB18071495 |
| 单航   | PB18111747 |

## 实验要求

请按照自己的理解，写明本次实验需要干什么

在理解SSA（静态单赋值）格式的基础上，实现三个简单的块内优化Pass与分析Pass：常量传播，循环不变式外提，活跃变量分析。

- **常量传播**:能够实现在编译优化阶段，能够计算出结果的变量，就直接替换为常量
- **循环不变式外提**:要能够实现将与循环无关的表达式提取到循环的外面。不用考虑数组，与全局变量。
- **活跃变量分析**:能够实现分析bb块的入口和出口的活跃变量。

## 实验难点

实验中遇到哪些挑战

- **常量传播**：因为基本上上过课，有了理论知识，然后才来做实验的，所以其实理解算法的工作原理不是特别难，基本上按照书上的算法实现就可以了
- **循环不变式外提**：写在后面的设计里面了，主要就是记录分析的顺序遇到了问题，不过还好解决了。
- **活跃变量分析**：最开始liveout的变量迭代万之后加入livein里面，他们出去是活跃的，在后继的in里面也应该是活跃的，迭代的时候加入就会出问题。

## 实验设计

* 常量传播
    实现思路：对于每个运算表达式判断是不是常量。
    
    ConstantFP \*cast_constantfp(Value \*\*value\*)和ConstantInt \*cast_constantint(Value \*\*value\*)两个函数判断是否是常量，ConstantInt *ConstFolder::compute返回整形值的常数折叠实现，ConstantFP *ConstFolder::computefp返回浮点值的常数折叠实现。
    
    具体的run过程就是遍历每一条指令，对于运算指令，判断是否是常量，用前面的函数就可以判断了，如果是常量就直接替代这些常量出现的地方，直接用常量的计算结果替代。
    
    
    
    相应代码：.hpp文件
    
    ```c++
    #ifndef CONSTPROPAGATION_HPP
    #define CONSTPROPAGATION_HPP
    #include "PassManager.hpp"
    #include "Constant.h"
    #include "Instruction.h"
    #include "Module.h"
    
    #include "Value.h"
    #include "IRBuilder.h"
    #include <vector>
    #include <stack>
    #include <unordered_map>
    
    // tips: 用来判断value是否为ConstantFP/ConstantInt
    ConstantFP* cast_constantfp(Value *value);
    ConstantInt* cast_constantint(Value *value);
    
    
    // tips: ConstFloder类
    
    class ConstFolder
    {
    public:
        ConstFolder(Module *m) : module_(m) {}
        ConstantInt *compute(
            Instruction::OpID op,
            ConstantInt *value1,
            ConstantInt *value2);
        ConstantFP *computefp(
            Instruction::OpID op,
            ConstantFP *value1,
            ConstantFP *value2);
        // ...
    private:
        Module *module_;
    };
    
    class ConstPropagation : public Pass
    {
    public:
        ConstPropagation(Module *m) : Pass(m) {}
        void run();
    };
    
    #endif
    ```
    
    .cpp文件（其实就是先判断是什么运算，然后做出相应的替换工作，很多都是对于不同运算符的重复）
    
    ```c++
    #include "ConstPropagation.hpp"
    #include "logging.hpp"
    
    // 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
    // 当然如果同学们有更好的方式，不强求使用下面这种方式
    ConstantInt *ConstFolder::compute(
        Instruction::OpID op,
        ConstantInt *leftvalue,
        ConstantInt *rightvalue)
    {
    
        int c_leftvalue = leftvalue->get_value();
        int c_rightvalue = rightvalue->get_value();
        switch (op)
        {
        case Instruction::add:
            return ConstantInt::get(c_leftvalue + c_rightvalue, module_);
    
            break;
        case Instruction::sub:
            return ConstantInt::get(c_leftvalue - c_rightvalue, module_);
            break;
        case Instruction::mul:
            return ConstantInt::get(c_leftvalue * c_rightvalue, module_);
            break;
        case Instruction::sdiv:
            return ConstantInt::get((int)(c_leftvalue / c_rightvalue), module_);
            break;
        default:
            return nullptr;
            break;
        }
    }
    //TODO:修改float实现
    ConstantFP *ConstFolder::computefp(
        Instruction::OpID op,
        ConstantFP *leftvalue,
        ConstantFP *rightvalue)
    {
    
        int c_leftvalue = leftvalue->get_value();
        int c_rightvalue = rightvalue->get_value();
        switch (op)
        {
        case Instruction::fadd:
            return ConstantFP::get(c_leftvalue + c_rightvalue, module_);
    
            break;
        case Instruction::fsub:
            return ConstantFP::get(c_leftvalue - c_rightvalue, module_);
            break;
        case Instruction::fmul:
            return ConstantFP::get(c_leftvalue * c_rightvalue, module_);
            break;
        case Instruction::fdiv:
            return ConstantFP::get((float)(c_leftvalue / c_rightvalue), module_);
            break;
        default:
            return nullptr;
            break;
        }
    }
    
    // 用来判断value是否为ConstantFP，如果不是则会返回nullptr
    ConstantFP *cast_constantfp(Value *value)
    {
        auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
        if (constant_fp_ptr)
        {
            return constant_fp_ptr;
        }
        else
        {
            return nullptr;
        }
    }
    ConstantInt *cast_constantint(Value *value)
    {
        auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
        if (constant_int_ptr)
        {
            return constant_int_ptr;
        }
        else
        {
            return nullptr;
        }
    }
    
    
    void ConstPropagation::run()
    {
        std::vector<Instruction *> wait_delete;
        for (auto f : m_->get_functions())
        {
    
            auto func_ = f;
            for (auto bb : func_->get_basic_blocks())
            {
                for (auto instr : bb->get_instructions())
                {
                    if (instr->is_add()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantint(leftvalue)!=nullptr&&cast_constantint(rightvalue)!=nullptr){
                        auto const_leftvalue=cast_constantint(leftvalue);
                        auto const_rightvalue=cast_constantint(rightvalue);
                        auto constfolder=ConstFolder(m_);
                        auto replacement=constfolder.compute(Instruction::add,const_leftvalue,const_rightvalue);
                        int y=ConstantInt::get_value(replacement);
                        instr->replace_all_use_with(replacement);
                        wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_mul()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantint(leftvalue)!=nullptr&&cast_constantint(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantint(leftvalue);
                            auto const_rightvalue=cast_constantint(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.compute(Instruction::mul,const_leftvalue,const_rightvalue);
                            int y=ConstantInt::get_value(replacement);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
    
                    }
                    else if (instr->is_sub()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantint(leftvalue)!=nullptr&&cast_constantint(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantint(leftvalue);
                            auto const_rightvalue=cast_constantint(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.compute(Instruction::sub,const_leftvalue,const_rightvalue);
                            int y=ConstantInt::get_value(replacement);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_div()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantint(leftvalue)!=nullptr&&cast_constantint(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantint(leftvalue);
                            auto const_rightvalue=cast_constantint(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.compute(Instruction::sdiv,const_leftvalue,const_rightvalue);
                            int y=ConstantInt::get_value(replacement);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_fadd()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantfp(leftvalue)!=nullptr&&cast_constantfp(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantfp(leftvalue);
                            auto const_rightvalue=cast_constantfp(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.computefp(Instruction::fadd,const_leftvalue,const_rightvalue);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_fsub()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantfp(leftvalue)!=nullptr&&cast_constantfp(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantfp(leftvalue);
                            auto const_rightvalue=cast_constantfp(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.computefp(Instruction::fsub,const_leftvalue,const_rightvalue);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_fmul()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantfp(leftvalue)!=nullptr&&cast_constantfp(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantfp(leftvalue);
                            auto const_rightvalue=cast_constantfp(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.computefp(Instruction::fmul,const_leftvalue,const_rightvalue);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
                    }
                    else if (instr->is_fdiv()) {
                        auto leftvalue = instr->get_operand(0);
                        auto rightvalue = instr->get_operand(1);
                        if(cast_constantfp(leftvalue)!=nullptr&&cast_constantfp(rightvalue)!=nullptr){
                            auto const_leftvalue=cast_constantfp(leftvalue);
                            auto const_rightvalue=cast_constantfp(rightvalue);
                            auto constfolder=ConstFolder(m_);
                            auto replacement=constfolder.computefp(Instruction::fdiv,const_leftvalue,const_rightvalue);
                            instr->replace_all_use_with(replacement);
                            wait_delete.push_back(instr);
                        }
                    }
                }
            }
        }
        for (auto f : m_->get_functions())
        {
            auto func_ = f;
            for (auto bb : func_->get_basic_blocks())
            {
                for ( auto instr : wait_delete)
                {
                    bb->delete_instr(instr);
                }
            }
        }
    }
    ```
    
    优化前后的IR对比（举一个例子）并辅以简单说明：
    
    对于下面的程序：
    
    ```c
    int a;
    void main(void){
        int i;
    
        a=(1<2)+2;
        i=0;
        while(i<100000000)
        {
            if(a+(3.0*1.23456*5.73478*2.3333*4.3673/6.34636)/3-2*1.23456*5.73478*2.3333*4.3673*6.34636)
            {
                a=a+1;
            }
            i=i+1;
        }
        output(a*2+5);
        return ;
    }
    ```
    
    可以看到主要的想要优化的地方就是if（）里面一坨，直接根据优化的结果，在最后得到了优化的结果，对于这部分的处理是
    
    ```c
    label9:                                                ; preds = %label4
      %op10 = load i32, i32* @a
      %op18 = sitofp i32 %op10 to float
      %op19 = fadd float %op18, 0x4018000000000000
      %op26 = fsub float %op19, 0x407e000000000000
      %op27 = fcmp une float %op26,0x0
      br i1 %op27, label %label32, label %label35
    ```
    
    用clang得到的.ll文件是：
    
    ```c
    5:                                                ; preds = %2
      %6 = load i32, i32* @a, align 4
      %7 = sitofp i32 %6 to double
      %8 = fadd double %7, 0x4026BC7789C7F051
      %9 = fsub double %8, 0x408C9DD64BCAC5EF
      %10 = fcmp une double %9, 0.000000e+00
      br i1 %10, label %11, label %14
    ```
    
    是直接计算好之后再进行比较。
    
    而去掉优化的话，输出的是：
    
    ```c
    label9:                                                ; preds = %label4
      %op10 = load i32, i32* @a
      %op11 = fmul float 0x4008000000000000, 0x3ff3c0c200000000
      %op12 = fmul float 0x4008000000000000, 0x4016f06a20000000
      %op13 = fmul float 0x402e000000000000, 0x4002aa9940000000
      %op14 = fmul float 0x403e000000000000, 0x4011781d80000000
      %op15 = fdiv float 0x405e000000000000, 0x401962ac40000000
      %op16 = sitofp i32 3 to float
      %op17 = fdiv float 0x4034000000000000, %op16
      %op18 = sitofp i32 %op10 to float
      %op19 = fadd float %op18, %op17
      %op20 = sitofp i32 2 to float
      %op21 = fmul float %op20, 0x3ff3c0c200000000
      %op22 = fmul float %op21, 0x4016f06a20000000
      %op23 = fmul float %op22, 0x4002aa9940000000
      %op24 = fmul float %op23, 0x4011781d80000000
      %op25 = fmul float %op24, 0x401962ac40000000
      %op26 = fsub float %op19, %op25
      %op27 = fcmp une float %op26,0x0
      br i1 %op27, label %label32, label %label35
    ```
    
    是依次把浮点数都计算出来的。


* 循环不变式外提
    实现思路：先找到最底层循环，完成指令外提后找外一层循环，由于BBset的顺序问题，无法保证完全外提，可以递归来写（虽然没有这样实现），如果右侧op集合内有未在左侧op集合中出现的op_not_in，那么op_not_in在左侧的表达式是循环不变式，外提后应再次遍历继续外提。注意在删除相应的指令时，操作数是phi指令不能删除。
    
* 相应代码：最外面就是套了两个循环，对所有的bb块都要执行相应的操作。

    - 第一部分定义了两个集合，存储指令的顺序，避免外提时出错，最后得到翻转的指令列表，用来倒序遍历删除指令。

    ```c++
    bool is_invariant = true;
        m_->set_print_name();
        for (auto loopbb = loop_searcher.begin(); loopbb != loop_searcher.end(); loopbb++) {
            auto lbsp = **loopbb;
            for(auto loop_bb : lbsp)
            {
                std::list<Value *> rightoplist;
                std::list<Value *> leftoplist;
                std::list<Value *> instrlist;
                std::list<Value *> toeraselist;
                std::list<Value *> donotdelete;
    
                for (auto instr : loop_bb->get_instructions()) {
                    Instruction::OpID type = instr->get_instr_type();
                    if (type == Instruction::ret || type == Instruction::br ) {
                        for (auto op : instr->get_operands()) {
                            donotdelete.push_back(op);
                        }
                        continue;
                    }
                    else if (type == Instruction::alloca || type == Instruction::phi || type == Instruction::call || type == Instruction::getelementptr) {
                        continue;
                    }
                    leftoplist.push_back(instr);
                    for (auto op : instr->get_operands())
                        rightoplist.push_back(op);
                }
                leftoplist.reverse();
                rightoplist.reverse();
    ```

    - 第二部分主要是对指令进行删除，通过erase来控制是否要删除，当所有指令集不为空并且可能有要外提的指令执行循环的代码。

      ```c++
      bool erase;
                  for (auto lp = leftoplist.begin(); lp != leftoplist.end();) {
                      auto l = *lp;
                      if (find(donotdelete.begin(), donotdelete.end(), l) != donotdelete.end()) {
                          for (auto op : dynamic_cast<Instruction *>(l)->get_operands()) {
                              donotdelete.push_back(op);
                          }
                          lp++;
                          continue;
                      }
                      if (find(rightoplist.begin(), rightoplist.end(), l) == rightoplist.end())
                      {
                          erase = true;
                          auto l_ins = dynamic_cast<Instruction *>(l);
                          for (auto op : l_ins->get_operands()) {
                              if (find(rightoplist.begin(), rightoplist.end(), op) != rightoplist.end()) {
                                  if (op->get_name() != "")
                                  {
                                      if (dynamic_cast<Instruction *>(op)->is_phi())
                                              erase = false;
                                      else
                                           toeraselist.push_back(op);
                                  }
                              }
                          }
                          if (erase) {
                              instrlist.push_back(l);
                              leftoplist.erase(lp++);
                              for (auto op : toeraselist) {
                                  if (find(rightoplist.begin(), rightoplist.end(), op) != rightoplist.end()) {
                                      rightoplist.erase(find(rightoplist.begin(), rightoplist.end(), op));
                                  }
                              }
                              toeraselist.clear();
                          }
                      }
                      if (!erase) {
                          lp++;
                      }
                  }
      ```

    - 第三部分，获取precBB列表，提到标号最小的Prec，即列表中第一个BB，还要将前BB的br指令和ret指令先拿出来，将需要外提的指令外提后再加入前BB的末尾。

      ```c++
      instrlist.reverse();
                  leftoplist.clear();
                  rightoplist.clear();
                  donotdelete.clear();
                  auto precs = loop_bb->get_pre_basic_blocks();
                  auto prec = *precs.begin();
      
                  std::list<Instruction *> prec_br_instrs;
                  for (auto prec_instr : prec->get_instructions()) {
                      if (prec_instr->is_br() || prec_instr->is_ret())
                          prec_br_instrs.push_back(prec_instr);
                  }
                  for(auto pbi : prec_br_instrs)
                      prec->delete_instr(pbi);
                  for (auto dins : instrlist) {
                      auto d_ins = dynamic_cast<Instruction *>(dins);
                      prec->add_instruction(d_ins);
                      loop_bb->delete_instr(d_ins);
                  }
                  for(auto pbi : prec_br_instrs)
                      prec->add_instruction(pbi);
      ```

    优化前后的IR对比（举一个例子）并辅以简单说明：

    - 使用的cminus文件是：

      ```
      void main(void){
          int i;
          int j;
          int k;
          int o;
          int p;
          int q;
          int a;
          int ret;
      
          a = 2;
      
          i = 0;
          while(i<1000000)
          {
              j = 0;
              while(j<2)
              {
                  k = 0;
                  while(k<2)
                  {
                      o = 0;
                      while(o<2)
                      {
                          p = 0;
                          while(p<2)
                          {
                              q = 0;
                              while(q<2)
                              {
                                  if( a > 1 )
                                  {
                                      j = j+1;
                                  }
                                  ret = (a*a*a*a*a*a*a*a*a*a)/a/a/a/a/a/a/a/a/a/a;
                                  q=q+1;
                              }
                              p=p+1;
                          }
                          o=o+1;
                      }
                      k=k+1;
                  }
                  j=j+1;
              }
              i=i+1;
          }
      	output(ret);
          return ;
      }
      ```

    - 使用优化提出了循环不变量之后：将这里的一串乘2都提出来了，在前面就计算了。

      ```c
      label13:                                                ; preds = %label8
        %op71 = mul i32 2, 2
        %op73 = mul i32 %op71, 2
        %op75 = mul i32 %op73, 2
        %op77 = mul i32 %op75, 2
        %op79 = mul i32 %op77, 2
        %op81 = mul i32 %op79, 2
        %op83 = mul i32 %op81, 2
        %op85 = mul i32 %op83, 2
        %op87 = mul i32 %op85, 2
        %op89 = sdiv i32 %op87, 2
        %op91 = sdiv i32 %op89, 2
        %op93 = sdiv i32 %op91, 2
        %op95 = sdiv i32 %op93, 2
        %op97 = sdiv i32 %op95, 2
        %op99 = sdiv i32 %op97, 2
        %op101 = sdiv i32 %op99, 2
        %op103 = sdiv i32 %op101, 2
        %op105 = sdiv i32 %op103, 2
        %op107 = sdiv i32 %op105, 2
        br label %label16
      ```

    - 而不做优化的输出是：（从标号看出来区别，前面优化之后在很外层就计算了，这里还是在最里层计算的）

      ```c
      label68:                                                ; preds = %label57, %label65
        %op141 = phi i32 [ %op140, %label57 ], [ %op67, %label65 ]
        %op71 = mul i32 2, 2
        %op73 = mul i32 %op71, 2
        %op75 = mul i32 %op73, 2
        %op77 = mul i32 %op75, 2
        %op79 = mul i32 %op77, 2
        %op81 = mul i32 %op79, 2
        %op83 = mul i32 %op81, 2
        %op85 = mul i32 %op83, 2
        %op87 = mul i32 %op85, 2
        %op89 = sdiv i32 %op87, 2
        %op91 = sdiv i32 %op89, 2
        %op93 = sdiv i32 %op91, 2
        %op95 = sdiv i32 %op93, 2
        %op97 = sdiv i32 %op95, 2
        %op99 = sdiv i32 %op97, 2
        %op101 = sdiv i32 %op99, 2
        %op103 = sdiv i32 %op101, 2
        %op105 = sdiv i32 %op103, 2
        %op107 = sdiv i32 %op105, 2
        %op109 = add i32 %op139, 1
        br label %label52
      ```

* 活跃变量分析
    实现思路：直接正常处理除了`phi`指令之外的所有指令，单独对`phi`指令的左值进行活跃性分析。单独处理后继块中含`phi`指令的块，将右值加入`OUT`集合与后继的`IN`集合中。
    相应的代码：遇到`phi`指令只将左值根据使用情况加入`def`集合中，而不去处理右值。其他指令中，只要作为左值出现就加入`alldef`集合，只要作为右值出现就加入`alluse`集合。然后在遇到指令时对左值在`alluse`集合中查找，如果不在的话就加入`def`中，对右值在`alldef`集合中查找，如果不在的话就加入`use`中。
    
    ```c++
    #include "ActiveVars.hpp"
    
    // 用来判断value是否为ConstantFP，如果不是则会返回nullptr
    ConstantFP *cast_constantf(Value *value)
    {
        auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
        if (constant_fp_ptr)
        {
            return constant_fp_ptr;
        }
        else
        {
            return nullptr;
        }
    }
    ConstantInt *cast_constanti(Value *value)
    {
        auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
        if (constant_int_ptr)
        {
            return constant_int_ptr;
        }
        else
        {
            return nullptr;
        }
    }
    bool is_var(Value* oper){
        return !(cast_constantf(oper)||cast_constanti(oper));
    }
    void ActiveVars::run()
    {
        std::ofstream output_active_vars;
        output_active_vars.open("active_vars.json", std::ios::out);
        output_active_vars << "[";
        for (auto &func : this->m_->get_functions()) {
            if (func->get_basic_blocks().empty()) {
                continue;
            }
            else
            {
                func_ = func;  
    
                func_->set_instr_name();
                live_in.clear();
                live_out.clear();
                
                // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内
                std::map<BasicBlock *, std::set<Value *>>lazyin;
                for (auto bb : func_->get_basic_blocks())
                {
                    for (auto instr : bb->get_instructions()){
                    if (instr->is_phi())
                        {
                            if (alluse.find(instr) == alluse.end())
                                def[bb].insert(instr);
                            alldef.insert(instr);
                            for (auto oper : instr->get_operands())
                            {
                                if (is_var(oper))
                                {
                                    alluse.insert(oper);
                                }
                            }
                        }
    					else
                        {
                            if (alluse.find(instr) == alluse.end())
                                def[bb].insert(instr);
                            for (auto oper : instr->get_operands())
                            {
                                if (alldef.find(oper) == alldef.end() && is_var(oper))
                                    use[bb].insert(oper);
                            }
                            alldef.insert(instr);
                            for (auto oper : instr->get_operands())
                            {
                                if (is_var(oper))
                                {
                                    alluse.insert(oper);
                                }
                            }
                        }
                    
    					for (auto succ : bb->get_succ_basic_blocks())
                        {
                            alldef.clear();
                            for (auto instr : succ->get_instructions())
                            {
                                if (instr->is_phi())
                                {
                                    auto vec = instr->get_operands();
                                    for (auto i = 0; i < vec.size(); i += 2)
                                    {
                                        if (dynamic_cast<BasicBlock *>(vec[i + 1]) == bb && dynamic_cast<ConstantInt *>(vec[i]) == nullptr && dynamic_cast<ConstantFP *>(vec[i]) == nullptr)
                                        {
                                            if (alldef.find(vec[i]) == alldef.end())
                                            {
                                                live_out[bb].insert(vec[i]);
                                                lazyin[succ].insert(vec[i]);
                                            }
                                        }
                                    }
                                }
                                alldef.insert(instr);
                            }
                        }
                    }
                    for (auto bb : func->get_basic_blocks())
                    {
                        need_iterate = false;
                        if (live_in[bb] != tempin[bb])
                        {
                            need_iterate = true;
                            break;
                        }
                    }
                }
                for (auto bb : func->get_basic_blocks())
                {
                    set_union(live_in[bb].begin(), live_in[bb].end(), lazyin[bb].begin(), lazyin[bb].end(), inserter(live_in[bb], live_in[bb].begin()));
                }
                output_active_vars << print();
                output_active_vars << ",";
            }
        }
        output_active_vars << "]";
        output_active_vars.close();
        return ;
    }
    
    std::string ActiveVars::print()
    {
        std::string active_vars;
        active_vars +=  "{\n";
        active_vars +=  "\"function\": \"";
        active_vars +=  func_->get_name();
        active_vars +=  "\",\n";
    
        active_vars +=  "\"live_in\": {\n";
        for (auto &p : live_in) {
            if (p.second.size() == 0) {
                continue;
            } else {
                active_vars +=  "  \"";
                active_vars +=  p.first->get_name();
                active_vars +=  "\": [" ;
                for (auto &v : p.second) {
                    active_vars +=  "\"%";
                    active_vars +=  v->get_name();
                    active_vars +=  "\",";
                }
                active_vars += "]" ;
                active_vars += ",\n";   
            }
        }
        active_vars += "\n";
        active_vars +=  "    },\n";
        
        active_vars +=  "\"live_out\": {\n";
        for (auto &p : live_out) {
            if (p.second.size() == 0) {
                continue;
            } else {
                active_vars +=  "  \"";
                active_vars +=  p.first->get_name();
                active_vars +=  "\": [" ;
                for (auto &v : p.second) {
                    active_vars +=  "\"%";
                    active_vars +=  v->get_name();
                    active_vars +=  "\",";
                }
                active_vars += "]";
                active_vars += ",\n";
            }
        }
        active_vars += "\n";
        active_vars += "    }\n";
    
        active_vars += "}\n";
        active_vars += "\n";
        return active_vars;
    }
    ```
    
    

### 实验总结

此次实验有什么收获

收获了如何进行常量传播，循环不变式外提，活跃变量分析的优化，对于课堂上的知识有了进一步的了解

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
