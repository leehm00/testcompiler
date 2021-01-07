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