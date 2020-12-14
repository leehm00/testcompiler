#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())
#define CONST_INT(num) \
    ConstantInt::get((int)num, module.get())

// You can define global variables here
// to store state

enum var_op {
      LOAD,
      STORE
};
std::string valuename;
CminusType expression_return_type;
Function* now_function;
std::vector<CminusType> returnstack;
BasicBlock* return_BasicBlock;
BasicBlock* curr_block;
// the return basicblock, to make code inside function know where to jump to return
BasicBlock* return_block;
// the return value Alloca
Value* return_alloca;
// for BasicBlock::Create to get function ref.
Function* curr_func;
// record the expression, to be used in while, if and return statements
Value* expression;
// record whether a return statement is encountered. If return is found, following code should be ignored to avoid IR problem.
bool is_returned = false;
bool is_returned_record = false;
int label_cnt = 0;
// show what to do in syntax_var
var_op curr_op = LOAD;
bool created_ret_BasicBlock;
bool returned_statement;
bool use_var;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    for(auto declaration:node.declarations){
        declaration->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {
    if(node.type != TYPE_VOID){
        if(node.type == TYPE_FLOAT){
            auto float_alloca = builder->create_alloca(Type::get_float_type(module.get()));
            builder->create_store(CONST_FP(node.f_val),float_alloca);
            auto floatnum = builder->create_load(float_alloca);
            valuename = "floatnum";
            scope.push("floatnum",floatnum);
            expression_return_type = TYPE_FLOAT;
            expression = ConstantInt::get(node.f_val,module.get());
    }
    else {
        expression = ConstantInt::get(node.i_val,module.get());
        auto int_alloca = builder->create_alloca(Type::get_int32_type(module.get()));
        builder->create_store(CONST_INT(node.i_val),int_alloca);
        auto intnum = builder->create_load(int_alloca);
        valuename = "intnum";
        scope.push("intnum",intnum);
        expression_return_type = TYPE_INT;
    }  
    }
    else {
        std::cout<<"num must be an int or a float";
        exit(0);
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    
    Module *m = module.get(); 
    auto typeint1 = Type::get_int1_type(m); 
    auto typeint32 = Type::get_int32_type(m); 
    auto typefloat = Type::get_float_type(m); 
    auto typeptrint32 = Type::get_int32_ptr_type(m); 
    auto typeptrfloat = Type::get_float_ptr_type(m);
    //全局变量
    if(scope.in_global()){
        GlobalVariable* gvar;
        if(node.num != nullptr){
            node.num->accept(*this);
            if(node.type == TYPE_FLOAT){
                Type *float_type = Type::get_float_type(module.get());
                auto init = ConstantZero::get(float_type,module.get());
                auto *arrayType = ArrayType::get(float_type,(node.num->i_val));
                auto arrayname = GlobalVariable::create(node.id,module.get(),arrayType,false,init);
                scope.push(node.id,arrayname);
                ConstantZero* constarr = ConstantZero::get(Type::get_label_type(module.get()),module.get());
  		        // gvar = new GlobalVariable(*module,
  		        // 		arrType,
  		        // 		false,
  		        // 		GlobalValue::CommonLinkage,
  		        // 		node.id);
                gvar = GlobalVariable::create(node.id,module.get(),Type::get_label_type(module.get()),false,constarr);
                 
            }else{
                Type *int_type = Type::get_int32_type(module.get());
                auto init = ConstantZero::get(int_type, module.get());
                auto *arrayType = ArrayType::get(int_type,(node.num->i_val));
                auto arrayname = GlobalVariable::create(node.id,module.get(),arrayType,false,init);
                scope.push(node.id,arrayname);
                ConstantZero* constarr = ConstantZero::get(Type::get_label_type(module.get()),module.get());
  		        // gvar = new GlobalVariable(*module,
  		        // 		arrType,
  		        // 		false,
  		        // 		GlobalValue::CommonLinkage,
  		        // 		node.id);
                gvar = GlobalVariable::create(node.id,module.get(),Type::get_label_type(module.get()),false,constarr);
                
                
            }
        }
        else if(node.type == TYPE_FLOAT){
            Type *floatnum = Type::get_float_type(module.get());
            auto init = ConstantZero::get(floatnum, module.get());
            auto globalvar = GlobalVariable::create(node.id,module.get(),floatnum,false,init);
            ConstantFP* const_float = ConstantFP::get(0,module.get());//姑且
            scope.push(node.id,globalvar);
            gvar = GlobalVariable::create(node.id,module.get(),Type::get_float_type(module.get()),false,const_float);
            
        }
        else if(node.type == TYPE_INT){
            Type *int_type = Type::get_int32_type(module.get());
            auto init = ConstantZero::get(int_type, module.get());
            auto globalvar = GlobalVariable::create(node.id,module.get(),int_type,false,init);
            ConstantInt* const_int = ConstantInt::get(0, module.get());//
            scope.push(node.id,globalvar);
            gvar = GlobalVariable::create(node.id,module.get(),Type::get_int32_type(module.get()),false,const_int);
            
        }
    }
    //局部变量声明
    else{
        //数组
        if(node.num != nullptr){
            node.num->accept(*this);
            
                if(node.type == TYPE_FLOAT){
                    Type *float_type = Type::get_float_type(module.get());
                    auto *arrayType = ArrayType::get(float_type,(node.num->i_val));
                    auto arrayname = builder->create_alloca(arrayType);
                    scope.push(node.id,arrayname);
                }
                else{
                    Type *int_type = Type::get_int32_type(module.get());
                    auto *arrayType = ArrayType::get(int_type,(node.num->i_val));
                    auto arrayname = builder->create_alloca(arrayType);
                    scope.push(node.id,arrayname);
                }
        }//num
        else if(node.type == TYPE_FLOAT){
            Type *float_type = Type::get_float_type(module.get());
            auto localfloat = builder->create_alloca(float_type);
            scope.push(node.id,localfloat);
        }
        else if(node.type == TYPE_INT){
            Type *int_type = Type::get_int32_type(module.get());
            auto localint = builder->create_alloca(int_type);
            scope.push(node.id,localint);
        }
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    //储存参数
    std::vector<Type* > params;
    
        for(std::shared_ptr<ASTParam> param: node.params){
            if(param->isarray){
                if(param->type == TYPE_FLOAT){
                    
                    params.push_back(Type::get_float_ptr_type(module.get()));
                }
                else if(param->type == TYPE_INT){
                    
                    params.push_back(Type::get_int32_ptr_type(module.get()));
                }
                
            }
            else if(param->type == TYPE_FLOAT){
                params.push_back(Type::get_float_type(module.get()));
            }
            else if(param->type == TYPE_INT){
                params.push_back(Type::get_int32_type(module.get()));
            }
        }
    
    //函数分析
    
    returnstack.push_back(node.type);
    Type *function_return_type;
    label_cnt = 0;
    switch (node.type)
    {
    case TYPE_VOID:
        function_return_type = Type::get_void_type(module.get());
        break;
    case TYPE_FLOAT:
        function_return_type = Type::get_float_type(module.get());
        break;
    case TYPE_INT:
        function_return_type = Type::get_int32_type(module.get());
        break;
    default:
        std::cout<<"function return type error";
        exit(0);
        break;
    }
    auto function_type = FunctionType::get(function_return_type,params);
    auto Fun = Function::create(function_type,node.id,module.get());
    
    scope.push(node.id,Fun);
    scope.enter();
    Value* return_alloca;
    now_function = Fun;
    returned_statement = false;
    auto BasicBlock_ = BasicBlock::create(module.get(),"entry",Fun);
    builder->set_insert_point(BasicBlock_);
    if(node.type == TYPE_VOID);else{
        return_alloca = builder->create_alloca(function_return_type);
        scope.push("return_alloca",return_alloca);
    }
    for(auto param: node.params){
        param->accept(*this);
    }
    std::vector<Value *> args;
    auto arg = Fun->arg_begin();
    while (arg != Fun->arg_end())
    {
        args.push_back(*arg);
        arg++;
    }
    int i = 0;
    while (i < args.size())
    {
        builder->create_store(args[i], scope.find(node.params[i]->id));
        i++;
    }
    created_ret_BasicBlock = false;
    node.compound_stmt->accept(*this);
    BasicBlock* ret_BaisicBlock;
    if(created_ret_BasicBlock == true){
        ret_BaisicBlock = return_BasicBlock;
    }
    else{
        ret_BaisicBlock = BasicBlock::create(module.get(),"ret_BaisicBlock",Fun);
    }
    builder->set_insert_point(ret_BaisicBlock);
    if(node.type != TYPE_VOID){
        LoadInst* retLoad = builder->create_load(return_alloca);
        builder->create_ret(retLoad);
    }
    else{
        builder->create_void_ret();
    }
    scope.exit();
    returnstack.pop_back();
}

void CminusfBuilder::visit(ASTParam &node) {
    Type* params_type;
    if(node.type == TYPE_VOID){
        return;
    }
    if(node.isarray){
        if(node.type == TYPE_FLOAT){
            params_type = Type::get_float_ptr_type(module.get());
        }
        else if(node.type == TYPE_INT){
            params_type = Type::get_int32_ptr_type(module.get());
        }
    }
    else if(node.type == TYPE_FLOAT){
        params_type = Type::get_float_type(module.get());
    }
    else if(node.type == TYPE_INT){
        params_type = Type::get_int32_type(module.get());
    }
    
    AllocaInst* paramAlloca = builder->create_alloca(params_type);
    scope.push(node.id,paramAlloca);
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    scope.enter();
    for (auto var_decl: node.local_declarations) {
        var_decl->accept(*this);
		if (var_decl->type == TYPE_VOID) {
			std::cout << "Error: no void type variable or array is allowed!" << std::endl;
		}
		// array declaration
		if (var_decl->num != nullptr&&0) {

			auto arrType = ArrayType::get(IntegerType::get(32,module.get()), var_decl->num->i_val);//wrong ,what should element be?
			auto arrptr = builder->create_alloca(arrType);
			scope.push(var_decl->id, arrptr);
		}
		// normal variable declaration
		else if (var_decl->num == nullptr&&0) {
			auto var = builder->create_alloca(Type::get_int32_type(module.get()));
			scope.push(var_decl->id, var);
		}
	}
    is_returned = false;
    for(std::shared_ptr<ASTStatement> stmt:node.statement_list){
        stmt->accept(*this);
        if (is_returned)
			break;
    }
    is_returned_record = is_returned;
	is_returned = false;
    scope.exit();
 }

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    if(node.expression != nullptr){
        node.expression->accept(*this);
    }
 }

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    
    curr_op = LOAD;
    char labelname[100];
    Value* expr1;
    scope.enter();
    BasicBlock* trueBasicBlock = BasicBlock::create(module.get(),"trueBasicBlock",now_function);
    BasicBlock* falseBasicBlock = BasicBlock::create(module.get(),"falseBasicBlock",now_function);
    node.expression->accept(*this);
    
    Value* compare = scope.find(valuename);
    builder->create_cond_br(compare,trueBasicBlock,falseBasicBlock);
    scope.exit();
    if (expression->get_type() == Type::get_int32_type(module.get())&&0) {
        expr1 = builder->create_icmp_ne(expression, ConstantInt::get(0,module.get()));
    }
    else if ((expression->get_type() != Type::get_int32_type(module.get())&&0)) {
        expr1 = expression;
    }
    // record the current block, for add the br later
    BasicBlock* orig_block = curr_block;
    // create the conditional jump CondBr
    // if-statement
    label_cnt++;
    int label_now = label_cnt;
    curr_block =  trueBasicBlock;
    BasicBlock* trueBB_location;
    BasicBlock* falseBB_location;
    
    if(node.else_statement != nullptr){
        BasicBlock* next_BasicBlock;
        builder->set_insert_point(trueBasicBlock);
        scope.enter();
        returned_statement = false;
        node.if_statement->accept(*this);
        bool return1 = returned_statement;
        scope.exit();
        if(returned_statement);else{
            next_BasicBlock = BasicBlock::create(module.get(),"next_BasicBlock",now_function);
            builder->create_br(next_BasicBlock);
        }
        builder->set_insert_point(falseBasicBlock);
        scope.enter();
        returned_statement = false;
        node.else_statement->accept(*this);
        bool return2 = returned_statement;
        bool trueBB_returned;
        bool falseBB_returned;
        trueBB_location = curr_block;
        trueBB_returned = is_returned_record;
        scope.exit();
        if(!return1 && return2){
            next_BasicBlock = BasicBlock::create(module.get(),"next_BasicBlock",now_function);
            builder->create_br(next_BasicBlock);
        }
        else if(!return1 && !return2){
            builder->create_br(next_BasicBlock);
        }
        if(!return1 || !return2){
            builder->set_insert_point(next_BasicBlock);
        }
        returned_statement = return1 && return2;
        
    }
    else{
        builder->set_insert_point(trueBasicBlock);
        scope.enter();
        node.if_statement->accept(*this);
        scope.exit();
        builder->create_br(falseBasicBlock);
        builder->set_insert_point(falseBasicBlock);
    }
 }

void CminusfBuilder::visit(ASTIterationStmt &node) {
    BasicBlock* judgeBasicBlock = BasicBlock::create(module.get(),"judgeBasicBlock",now_function);
    BasicBlock* trueBasicBlock = BasicBlock::create(module.get(),"trueBasicBlock",now_function);
    BasicBlock* falseBasicBlock = BasicBlock::create(module.get(),"falseBasicBlock",now_function);
    label_cnt++;
    int label_now = label_cnt;
  	char labelname[100];
    //auto startBB = BasicBlock::create(module.get(), labelname, curr_func);
    //curr_block = startBB;
  	curr_op = LOAD;
    scope.enter();
    builder->create_br(judgeBasicBlock);
    builder->set_insert_point(judgeBasicBlock);
    node.expression->accept(*this);
    Value* compare = scope.find(valuename);
    builder->create_cond_br(compare,trueBasicBlock,falseBasicBlock);
    builder->set_insert_point(trueBasicBlock);
    scope.exit();
    scope.enter();
    builder->create_br(judgeBasicBlock);
    builder->set_insert_point(falseBasicBlock);
    node.statement->accept(*this);
    scope.exit();
    Value* expr1;
    if (expression->get_type() == Type::get_int32_type(module.get())&&0) {
  		expr1 = builder->create_icmp_ne(expression, ConstantInt::get(0,module.get()));
  	}
  	else if (expression->get_type() != Type::get_int32_type(module.get())&&0){
  		expr1 = expression;
  	}
    //auto endBB = BasicBlock::create(module.get(), labelname, curr_func);
 }

void CminusfBuilder::visit(ASTReturnStmt &node) {
    if(created_ret_BasicBlock == true);else{
        auto ret_BaisicBlock = BasicBlock::create(module.get(),"ret_BaisicBlock",now_function);
        return_BasicBlock = ret_BaisicBlock;
        created_ret_BasicBlock = true;
    }
    Module *m = module.get(); 
    auto typeint1 = Type::get_int1_type(m); 
    auto typeint32 = Type::get_int32_type(m); 
    auto typefloat = Type::get_float_type(m); 
    auto typeptrint32 = Type::get_int32_ptr_type(m); 
    auto typeptrfloat = Type::get_float_ptr_type(m);
    if(node.expression == nullptr){
        returned_statement = true;
        builder->create_br(return_BasicBlock);
        return;
    }
    else{
        scope.enter();
        curr_op = LOAD;
		Value* retVal;
        node.expression->accept(*this);
        if(returnstack.back() == TYPE_FLOAT && expression_return_type == TYPE_FLOAT||returnstack.back() == TYPE_INT && expression_return_type == TYPE_INT){
            Value* ReturnValue = scope.find(valuename);
            Value* return_alloca = scope.find("return_alloca");
            builder->create_store(ReturnValue,return_alloca);
            returned_statement = true;
            builder->create_br(return_BasicBlock);
        }
        
        else{
            
        
        if (expression->get_type() == Type::get_int1_type(module.get())) {
			// cast i1 boolean true or false result to i32 0 or 1


			auto retCast = builder->create_zext(expression,Type::get_int32_type(module.get()));
      retVal = retCast;

			//builder->CreateRet(retCast);
		}
		else if (expression->get_type() == Type::get_int32_type(module.get())) {
			retVal = expression;
			//builder->create_ret(expression);
		}
		else {
			std::cout << "Error: unknown expression return type!" << std::endl;
		}
        }
    }
 }

void CminusfBuilder::visit(ASTVar &node) {
    label_cnt++;
    switch (curr_op)
    {
    case LOAD:
        if(node.expression != nullptr){
        Value* idalloca = scope.find(node.id);
        LoadInst* loadid = builder->create_load(idalloca);
        scope.enter();
        node.expression->accept(*this);
        Value* expressionalloca = scope.find(valuename);
        LoadInst* loadexpression = builder->create_load(expressionalloca);
        scope.exit();
        GetElementPtrInst* assign_gep = builder->create_gep(loadid,{loadexpression});
        scope.push("assign_gep",assign_gep);
        valuename = "assign_gep";
        valuename = node.id;
        Value* var = scope.find(node.id);
        if(var->get_type() == Type::get_float_type(module.get())){
            expression_return_type = TYPE_FLOAT;
        }
        else{
            expression_return_type = TYPE_INT;
        }
        }
        else{
        valuename ="varvalue";
        Value* var = scope.find(node.id);
        LoadInst* loadvar = builder->create_load(var);
        scope.push("varvalue",loadvar);
        if(loadvar->get_type()->is_float_type()){
            expression_return_type = TYPE_FLOAT;
        }
        if(loadvar->get_type()->is_integer_type()){
            expression_return_type = TYPE_INT;
        }
        else{
            std::cout<<"variable has a incorrect type";
            exit(0);
        }
        }
        break;
    case STORE:
        if (node.expression == nullptr) {
        // variable
        auto alloca = scope.find(node.id);
        Value* expr;
        if(expression->get_type() == Type::get_int1_type(module.get())) {
          expr = builder->create_zext(expression, Type::get_int32_type(module.get()));
        }
        else {
          expr = expression;
        }
        builder->create_store(expr, alloca);
        expression = expr;
      }
      else{
        curr_op = LOAD;
        auto rhs = expression;
        node.expression->accept(*this);
        Value* expr;
        if(expression->get_type() == Type::get_int1_type(module.get())) {
          expr = builder->create_zext(expression, Type::get_int32_type(module.get()));
        }
        else {
          expr = expression;
        }

        // check if array index is negative
        auto neg = builder->create_icmp_lt(expression, ConstantInt::get(0,module.get()));
        char labelname[100];
        label_cnt++;
        sprintf(labelname, "arr_neg_%d", label_cnt);
        auto arrnegBB = BasicBlock::create(module.get(), labelname, curr_func);
        sprintf(labelname, "arr_ok_%d", label_cnt);
        auto arrokBB = BasicBlock::create(module.get(), labelname, curr_func);
        builder->create_cond_br(neg, arrnegBB, arrokBB);
        builder->set_insert_point(arrnegBB);
        std::vector<Value*> argdum;
        builder->create_call(scope.find("neg_idx_except"), argdum);
        // add this just to make llvm happy, actually program will abort in call
        builder->create_br(arrokBB);
        builder->set_insert_point(arrokBB);
        curr_block = arrokBB;

        auto alloca = scope.find(node.id);
        if (alloca->get_type() == PointerType::get_int32_ptr_type(module.get())) {
          // array passed by reference, treat as pointer
          // function parameter makes it pointer of pointer, so load first
          auto arrptr = builder->create_load(PointerType::get_int32_ptr_type(module.get()), alloca);
          std::vector<Value *> idx;
          idx.push_back(expr);
          auto gep = builder->create_gep(arrptr, idx);
          builder->create_store(rhs, gep);
          expression = expr;

        }
        else {
          // local array or global array, type of which is [100 x i32]* like
          std::vector<Value *> idx;
          idx.push_back(ConstantInt::get(0,module.get()));
          idx.push_back(expr);
          auto gep = builder->create_gep( alloca, idx);
          builder->create_store(rhs, gep);
          expression = expr;
        }
      }
        break;
    default:
        std::cout << "ERROR: wrong var op!" << std::endl;
        
        break;
    }
    
 }
void CminusfBuilder::visit(ASTAssignExpression &node) {
    Module *m = module.get(); 
    auto typeint1 = Type::get_int1_type(m); 
    auto typeint32 = Type::get_int32_type(m); 
    auto typefloat = Type::get_float_type(m); 
    auto typeptrint32 = Type::get_int32_ptr_type(m); 
    auto typeptrfloat = Type::get_float_ptr_type(m);
    scope.enter();
    curr_op = LOAD;
    node.var->accept(*this);
    Value* assignalloca = scope.find(node.var->id);
    CminusType assign_type = expression_return_type;
    node.expression->accept(*this);
    Value* loadvalue = scope.find(valuename);
    CminusType valuetype = expression_return_type;
    Value *left = loadvalue;
    if(assign_type == TYPE_INT && valuetype == TYPE_FLOAT){
        loadvalue = builder->create_fptosi(loadvalue,Type::get_int32_type(module.get()));
    }
    if(assign_type == TYPE_FLOAT && valuetype == TYPE_INT){
        loadvalue = builder->create_sitofp(loadvalue,Type::get_float_type(module.get()));
    }
    if (loadvalue->get_type() == typeint32 &&0&& left->get_type() == typefloat) { 

        loadvalue = builder->create_sitofp(loadvalue, typefloat); 

    } 
    if (loadvalue->get_type() == typefloat && left->get_type() == typeint32&&0) { 

        loadvalue = builder->create_fptosi(loadvalue, typeint32); 

    }
    builder->create_store(loadvalue,assignalloca);
    scope.push("assignvalue",loadvalue);
    valuename = "assignvalue";
    expression_return_type = expression_return_type;
    
 }

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    
    if(node.additive_expression_r == nullptr){
        scope.enter();
        node.additive_expression_l->accept(*this);
        Value* value = scope.find(valuename);
        scope.exit();
        valuename = "value";
        scope.push("value",value);
        expression_return_type = expression_return_type;
    }
    else{
        Module *m = module.get(); 
        auto typeint1 = Type::get_int1_type(m); 
        auto typeint32 = Type::get_int32_type(m); 
        auto typefloat = Type::get_float_type(m); 
        auto typeptrint32 = Type::get_int32_ptr_type(m); 
        auto typeptrfloat = Type::get_float_ptr_type(m);
        scope.enter();
        node.additive_expression_l->accept(*this);
        Value* loadleft = scope.find(valuename);
        CminusType left_type = expression_return_type;
        scope.exit();
        scope.enter();
        node.additive_expression_r->accept(*this);
        Value* loadright = scope.find(valuename);
        auto right_type = expression_return_type;
        scope.exit();
        Value* lhs = expression;
		curr_op = LOAD;
		Value* rhs = expression;
        if(left_type == TYPE_INT && right_type == TYPE_INT){
            Value* icmp;
            switch (node.op)
            {
            case OP_LT:{
                icmp = builder->create_icmp_lt(loadleft,loadright);
                break;
            }
            case OP_LE:{
                icmp = builder->create_icmp_le(loadleft,loadright);
                break;
            }
            case OP_GE:{
                icmp = builder->create_icmp_ge(loadleft,loadright);
                break;
            }
            case OP_GT:{
                icmp = builder->create_icmp_gt(loadleft,loadright);
                break;
            }
            case OP_EQ:{
                icmp = builder->create_icmp_eq(loadleft,loadright);
                break;
            }
            case OP_NEQ:{
                icmp = builder->create_icmp_ne(loadleft,loadright);
                break;
            }
            default:
                std::cout<<"compare erorr";
                exit(0);
                break;
            }
            valuename = "icmp";
            scope.push("icmp",icmp);
            expression_return_type = TYPE_INT;
        }
        else{
            if(left_type == TYPE_FLOAT){
                builder->create_sitofp(loadleft,Type::get_float_type(module.get()));
            }
            else if(right_type == TYPE_FLOAT){
                builder->create_sitofp(loadright,Type::get_float_type(module.get()));
            }
            Value* fcmp;
            switch (node.op)
            {
            
            case OP_LT:{
                fcmp = builder->create_fcmp_lt(loadleft,loadright);
                break;
            }
            case OP_LE:{
                fcmp = builder->create_fcmp_le(loadleft,loadright);
                break;
            }
            case OP_GE:{
                fcmp = builder->create_fcmp_ge(loadleft,loadright);
                break;
            }
            case OP_GT:{
                fcmp = builder->create_fcmp_gt(loadleft,loadright);
                break;
            }
            case OP_EQ:{
                fcmp = builder->create_fcmp_eq(loadleft,loadright);
                break;
            }
            case OP_NEQ:{
                fcmp = builder->create_fcmp_ne(loadleft,loadright);
                break;
            }
            default:
                std::cout<<"compare erorr";
                exit(0);
                break;
            }
            valuename = "fcmp";
            scope.push("fcmp",fcmp);
            expression_return_type = TYPE_FLOAT;
        }
    }
 }

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    if(node.additive_expression == nullptr){
        curr_op = LOAD;
        scope.enter();
        node.term->accept(*this);
        Value* termAlloca = scope.find(valuename);
        scope.exit();
        valuename = "termAlloca";
        scope.push("termAlloca",termAlloca);
        expression_return_type = expression_return_type;
    }
    else{ 
        scope.enter();
        node.additive_expression->accept(*this);
        Value* loadleft = scope.find(valuename);
        CminusType left_type = expression_return_type;
        scope.exit();
        scope.enter();
        node.term->accept(*this);
        Value* loadright = scope.find(valuename);
        CminusType right_type = expression_return_type;
        scope.exit();
        curr_op = LOAD;
		Value* lhs = expression;
		curr_op = LOAD;
		Value* rhs = expression;
        if(left_type == TYPE_INT && right_type == TYPE_INT){
            if(node.op == OP_MINUS){
                auto isub = builder->create_isub(loadleft,loadright);
                valuename = "isub";
                scope.push("isub",isub);
            }
            else if(node.op == OP_PLUS){
                auto iadd = builder->create_iadd(loadleft,loadright);
                valuename = "iadd";
                scope.push("iadd",iadd);
            } 
            expression_return_type = TYPE_INT;
        }
        else{
            if(right_type == TYPE_INT){
                builder->create_sitofp(loadright,Type::get_float_type(module.get()));
            }
            else if(left_type == TYPE_INT){
                builder->create_sitofp(loadleft,Type::get_float_type(module.get()));
            }
            if(node.op == OP_MINUS){
                auto fsub = builder->create_fsub(loadleft,loadright);
                valuename = "fsub";
                scope.push("fsub",fsub);
            }
            else if(node.op == OP_PLUS){
                auto fadd = builder->create_fadd(loadleft,loadright);
                valuename = "fadd";
                scope.push("fadd",fadd);
            }
            expression_return_type = TYPE_FLOAT;
        }
    }
 }

void CminusfBuilder::visit(ASTTerm &node) {
    if(node.term == nullptr){
        scope.enter();
        curr_op = LOAD;
        node.factor->accept(*this);
        auto loadfractor = scope.find(valuename);
        scope.exit();
        valuename = "loadfractor";
        scope.push("loadfractor",loadfractor);
        expression_return_type = expression_return_type;
    }
    else{
        Value* lhs = expression;
		curr_op = LOAD;
        scope.enter();
        node.term->accept(*this);
        auto loadleft = scope.find(valuename);
        auto left_type = expression_return_type;
        scope.exit();
        scope.enter();
        node.factor->accept(*this);
        auto loadright = scope.find(valuename);
        auto right_type = expression_return_type;
        scope.exit();
        if(left_type == TYPE_INT && right_type == TYPE_INT){
            if(node.op == OP_DIV){
                auto isdiv = builder->create_isdiv(loadleft,loadright);
                valuename = "isdiv";
                scope.push("isdiv",isdiv);
            }
            else if(node.op == OP_MUL){
                auto imul = builder->create_imul(loadleft,loadright);
                valuename = "imul";
                scope.push("imul",imul);
            }else
            {
                std::cout << "Term error" << std::endl;
                exit(0);
            }
            expression_return_type = TYPE_INT;
        }
        else{
            if(left_type == TYPE_INT){
                builder->create_sitofp(loadleft,Type::get_float_type(module.get()));
            }
            else if(right_type == TYPE_INT){
                builder->create_sitofp(loadright,Type::get_float_type(module.get()));
            }
            if(node.op == OP_MUL){
                auto fmul = builder->create_fmul(loadleft,loadright);
                valuename = "fmul";
                scope.push("fmul",fmul);
            }
            else if(node.op == OP_DIV){
                auto fdiv = builder->create_fdiv(loadleft,loadright);
                valuename = "fdiv";
                scope.push("fdiv",fdiv);
            }else
            {
                std::cout << "Term error" << std::endl;
                exit(0);
            }
            
            expression_return_type = TYPE_FLOAT;
        }
    }
 }

void CminusfBuilder::visit(ASTCall &node) {
    int i = 0;
    Module *m = module.get(); 
    auto typeint1 = Type::get_int1_type(m); 
    auto typeint32 = Type::get_int32_type(m); 
    auto typefloat = Type::get_float_type(m); 
    auto typeptrint32 = Type::get_int32_ptr_type(m); 
    auto typeptrfloat = Type::get_float_ptr_type(m);
    Value* function = scope.find(node.id);
    auto fun = function->get_type();
    auto callFun = static_cast <FunctionType*> (fun);
    Function* Fun = dynamic_cast<Function *>(function);
    if (function == nullptr) {
        std::cout << "ERROR: Unknown function: " << node.id << std::endl;
        exit(1);
    }
    std::vector<Value *> args; 
    for(auto arg:node.args){
        scope.enter();
        arg->accept(*this);
        auto argLoad = scope.find(valuename);
        args.push_back(argLoad);
        scope.exit();
        if (function->get_type() == typeint32 && callFun->get_param_type(i) == typefloat && 0) {
            std::cout << "expressionint" << std::endl; 
            function = builder->create_sitofp(function, typefloat); 
            std::cout << "callFunﬂoat" << std::endl; 
        }
        if (function->get_type() == typefloat && callFun->get_param_type(i) == typeint32 && 0) { 

            std::cout << "expressionﬂoat" << std::endl; 

            function = builder->create_fptosi(function, typeint32); 

            std::cout << "callFunint" << std::endl; 

        }   
    }
    auto call = builder->create_call(function,args);
    auto function_typepe = function->get_type();
    if(function_typepe->is_integer_type()){
        expression_return_type = TYPE_INT;
    }
    else if(function_typepe->is_float_type()){
        expression_return_type = TYPE_FLOAT;
    }
    else if(function_typepe->is_void_type()){
        expression_return_type = TYPE_VOID;
    }
    valuename = "call";
    scope.push("call",call);
 }