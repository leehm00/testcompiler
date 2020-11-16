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
    auto module = new Module("Fun");//module name是Fun
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    //开始定义callee函数
    std::vector<Type *> Ints(1, Int32Type);
    auto calleeFunTy = FunctionType::get(Int32Type, Ints);
    auto calleeFun = Function::create(calleeFunTy, "callee", module);
    auto bbcallee = BasicBlock::create(module, "entry", calleeFun);

    builder->set_insert_point(bbcallee);

    auto retcalleeAlloca = builder->create_alloca(Int32Type);
    auto aAlloca = builder->create_alloca(Int32Type);
    //callee函数的参数定义和存放
    std::vector<Value *> args;
    for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
        args.push_back(*arg);
    }
    builder->create_store(CONST_INT(0), retcalleeAlloca);
    builder->create_store(args[0], aAlloca);
    //进入callee函数体开始执行
    auto aLoad = builder->create_load(aAlloca);
    auto mul = builder->create_imul(aLoad, CONST_INT(2));//计算2*a
    builder->create_store(mul, retcalleeAlloca);
    //返回值设定为2*a的结果
    auto retLoad = builder->create_load(retcalleeAlloca);
    builder->create_ret(retLoad);
    //开始定义main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    //默认return 0
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);
    //调用函数
    auto constAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(110), constAlloca);
    auto constLoad = builder->create_load(constAlloca);
    auto call = builder->create_call(calleeFun, {constLoad});
    //最后的返回语句
    builder->create_ret(call);
    //打印结果
    std::cout << module->print();
    delete module;
    return 0;
}
