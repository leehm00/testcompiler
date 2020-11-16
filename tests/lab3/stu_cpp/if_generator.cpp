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
    auto module = new Module("If");//module name是If
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    Type *FloatType = Type::get_float_type(module);
    //开始定义main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);//默认return 0
    auto aAlloca = builder->create_alloca(FloatType);//在内存中分配a的位置
    builder->create_store(CONST_FP(5.555), aAlloca);//赋值a = 5.555
    auto aLoad = builder->create_load(aAlloca);//取出参数a的值
    auto fcmp = builder->create_fcmp_gt(aLoad, CONST_FP(1.0));//if的判断,a>1
    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);//条件成立的分支
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);//条件不成立的分支
    auto retA = BasicBlock::create(module, "", mainFun);//返回语句
    auto br = builder->create_cond_br(fcmp, trueBB, falseBB);//根据fcmp的值跳转
    //条件成立的分支
    builder->set_insert_point(trueBB);
    builder->create_store(CONST_INT(233), retAlloca);//修改返回值为233
    builder->create_br(retA);//跳转到返回语句
    //条件不成立的分支
    builder->set_insert_point(falseBB);
    builder->create_br(retA);//对返回值不做修改,直接返回到返回语句
    //返回语句
    builder->set_insert_point(retA);
    auto call = builder->create_load(retAlloca);
    builder->create_ret(call);
    //打印结果
    std::cout << module->print();
    delete module;
    return 0;
}
