#include "ActiveVars.hpp"
#include "set"
#include <map>
#include"vector"
#include"iostream"
#include <unordered_map>
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
bool setcompare(std::set<Value*>iset1,std::set<Value*>iset2)
{
    if (iset1.size() != iset2.size())
        return false;
    std::set<Value *>::iterator iteration1, iteration2;
    bool flag = true;
    for (iteration1 = iset1.begin(), iteration2 = iset2.begin(); iteration1 != iset1.end(); iteration1++,iteration2++)
    {
        if (*iteration1 != *iteration2)
        {
            flag = false;
            break;
        }
    }
    return flag;
}
void setunion(std::set<Value *> &iset1, std::set<Value *> iset2,std::set<Value*> set3)
{
    for(auto item:set3)
    {

        iset2.insert(item);

    }
    iset1 = iset2;
}
void setinter(std::set<Value *> &iset1, std::set<Value *> iset2)
{
    for(auto item : iset2){
        if (iset1.find(item) != iset1.end())
        {
            iset1.erase(item);
        }
    }
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
            bool is_change = true;
            std::set<Value *> unioninstru;
            std::set<Value *> tempoutput;
            std::set<Value *> interreverse;
            std::set<Value *> touse;
            std::set<Value *> todefine;
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
                        for (int i = 0; i < instr->get_num_operand()/2; i++)
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
                            if(todefine.find(instr->get_operand(2*i)) == todefine.end())
                            {
                                if (dynamic_cast<ConstantInt *>(instr->get_operand(2 * i)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(2*i)) == nullptr)
                                {
                                    touse.insert(instr->get_operand(2 * i));

                                    allphi[bb].insert({instr->get_operand(2 * i), dynamic_cast<BasicBlock *>(instr->get_operand(2 * i + 1))});
                                }
                            }
                        }
                        if (touse.find(instr) == touse.end())
                        {
                            todefine.insert(instr);
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
                    if(instr->isBinary() || instr->is_fcmp() || instr->is_cmp() )
                    {
                        if (dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr && todefine.find(instr->get_operand(0)) == todefine.end()) //first value is a var
                        {
                            touse.insert(instr->get_operand(0));
                        }
                        if (dynamic_cast<ConstantInt *>(instr->get_operand(1)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(1)) == nullptr && todefine.find(instr->get_operand(1)) == todefine.end()) //second value is a var
                        {
                            touse.insert(instr->get_operand(1));
                        }
                        if (touse.find(instr) == touse.end())
                        {
                            todefine.insert(instr);
                        }
                    }
                    else if (instr->is_gep())
                    {
                        for (int i = 0; i < instr->get_num_operand(); i++)
                        {
                            if (dynamic_cast<ConstantInt *>(instr->get_operand(i)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(i)) == nullptr && todefine.find(instr->get_operand(i)) == todefine.end()) //second value is a var
                            {
                                touse.insert(instr->get_operand(i));
                            }
                            if (touse.find(instr) == touse.end())
                            {
                                todefine.insert(instr);
                            }
                        }

                    }
                    else if (instr->is_alloca())
                    {
                        if (touse.find(instr) == touse.end())
                        {
                            todefine.insert(instr);
                        }
                    }
                    else if (instr->is_load())
                    {
                        auto l_val = instr->get_operand(0);
                        if (todefine.find(l_val) == todefine.end())
                        {
                            touse.insert(l_val);
                        }
                        if (touse.find(instr) == touse.end())
                        {
                            todefine.insert(instr);
                        }
                    }
                    else if(instr->is_store())
                    {
                        auto l_val = static_cast<StoreInst *>(instr)->get_lval();
                        auto r_val = static_cast<StoreInst *>(instr)->get_rval();
                        if ((todefine.find(r_val) == todefine.end()) && (dynamic_cast<ConstantInt *>(r_val) == nullptr) && (dynamic_cast<ConstantFP *>(r_val) == nullptr))
                        {
                            touse.insert(r_val);
                        }
                        if ((todefine.find(l_val) == todefine.end()) && (dynamic_cast<ConstantInt *>(l_val) == nullptr))
                        {
                            touse.insert(l_val);
                        }
                    }
                    else if (instr->is_call())
                    {
                        for (int i = 1; i < instr->get_num_operand(); i++)
                        {
                            if ((todefine.find(instr->get_operand(i)) == todefine.end()) && (dynamic_cast<ConstantInt *>(instr->get_operand(i)) == nullptr) && (dynamic_cast<ConstantFP *>(instr->get_operand(i)) == nullptr))
                            {
                                touse.insert(instr->get_operand(i));
                            }

                        }
                        if ((touse.find(instr) == touse.end()) && !instr->is_void())
                        {
                            todefine.insert(instr);
                        }
                    }
                    else if (instr->is_zext() || instr->is_si2fp() || instr->is_fp2si())
                    {
                        if (todefine.find(instr->get_operand(0)) == todefine.end())
                        {
                            touse.insert(instr->get_operand(0));
                        }
                        if (touse.find(instr) == touse.end())
                        {
                            todefine.insert(instr);
                        }
                    }
                    else if (instr->is_ret())
                    {
                        if ((static_cast<ReturnInst *>(instr)->is_void_ret() == false) && (todefine.find(instr->get_operand(0)) == todefine.end()) && (dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr) && (dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr))
                        {
                            touse.insert(instr->get_operand(0));
                        }
                    }
                    else if (instr->is_br())
                    {
                        if (dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr && dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr && todefine.find(instr->get_operand(0)) == todefine.end())
                        {
                            if (static_cast<BranchInst*>(instr)->is_cond_br())
                            {
                                touse.insert(instr->get_operand(0));
                            }
                        }
                    }else if (alluse.find(instr) == alluse.end())
                            def[bb].insert(instr);
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
        maptodefine.clear();
        maptouse.clear();
        allphi.clear();
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