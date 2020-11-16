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
    auto module = new Module("Assign");//module name为Assign
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    //进入main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);//默认return 0
    //在内存中为a分配空间
    auto *arrayType = ArrayType::get(Int32Type, 10);
    auto aArray = builder->create_alloca(arrayType);
    //为a[0]赋值10
    auto A_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), A_gep);
    //为a[1]赋值,计算了a[0]*2
    auto A1_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(0)});
    auto a0Load = builder->create_load(A1_gep);//取a[0]值
    auto mul = builder->create_imul(a0Load, CONST_INT(2));//a[0]*2
    auto A2_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(1)});
    builder->create_store(mul, A2_gep);//存a[1]值
    auto A3_gep = builder->create_gep(aArray, {CONST_INT(0), CONST_INT(1)});
    auto call = builder->create_load(A3_gep);//取出了a[1]值,返回值修改为a[1]
    builder->create_ret(call);//返回语句
    //打印结果
    std::cout << module->print();
    delete module;
    return 0;
}
