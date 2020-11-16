#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
    auto module = new Module("While");//module name是While
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    //开始定义main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);//默认return 0
    auto aAlloca = builder->create_alloca(Int32Type);//分配a的空间
    auto iAlloca = builder->create_alloca(Int32Type);//分配i的空间
    builder->create_store(CONST_INT(10), aAlloca);//赋值a = 10
    builder->create_store(CONST_INT(0), iAlloca);//赋值i = 0

    auto judge_i = BasicBlock::create(module, "judge_i", mainFun);
    auto true_i = BasicBlock::create(module, "true_i", mainFun);//条件成立
    auto retA = BasicBlock::create(module, "retA", mainFun);//不成立直接返回
    builder->create_br(judge_i);
    //while判断条件
    builder->set_insert_point(judge_i);
    auto iLoad = builder->create_load(iAlloca);
    auto icmp = builder->create_icmp_lt(iLoad, CONST_INT(10));
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
    //返回语句,返回值是a
    builder->set_insert_point(retA);
    auto call = builder->create_load(aAlloca);
    builder->create_ret(call);
    //打印结果
    std::cout << module->print();
    delete module;
    return 0;
}
