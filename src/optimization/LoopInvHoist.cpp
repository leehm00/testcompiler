#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

void LoopInvHoist::run()
{
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();

    // 接下来由你来补充啦！
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
                if (type == 0 || type == 1 ) {
                    for (auto op : instr->get_operands()) {
                        donotdelete.push_back(op);
                    }
                    continue;
                }
                else if (type == 10 || type == 15 || type == 16 || type == 17) {
                    continue;
                }
                leftoplist.push_back(instr);
                for (auto op : instr->get_operands())
                    rightoplist.push_back(op);
            }
            leftoplist.reverse();
            rightoplist.reverse();

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
        }
    }

}
