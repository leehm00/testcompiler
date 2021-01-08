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