#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "classes.hh"
#include <algorithm>


bool found = false;
bool is_it_main = false;
int space = 0;
int printf_count = 0; 
int label_count = 2; //starts from 2 for some reason
int reg_size = 6;
int regi = 0;
static const char *reg[] = {"%ecx","%ebx","%edi","%esi","%eax","%edx"};
IPL::LocalSymbolTable* current;
IPL::GlobalSymbolTable* global;


static const char *tfall[] = {"jne","je","jge","jg","jle","jl"};
static const char *ffall[] = {"je","jne","jl","jle","jg","jge"};

std::vector<std::string> code_array;

namespace IPL
{
    void label(abstract_astnode* A){
        if(A->l != -1) return;
        else if(A->astnode_type == typeExp::seq_astnode){
            seq_astnode* X = dynamic_cast<seq_astnode*>(A);
            int m = 0;
            for (unsigned int i = 0; i < X->statements_.size(); i++) {
                label(X->statements_[i]);
                m = std::max(m,X->statements_[i]->l);
            }
            X->l = m;
        }
        else if(A->astnode_type == typeExp::assignS_astnode){
            assignS_astnode* X = dynamic_cast<assignS_astnode*>(A);
            label(X->expression1_);
            label(X->expression2_);
            if(X->expression1_->l == 0) X->expression1_->l++;
            if(X->expression1_->l == X->expression2_->l) { X->l = X->expression1_->l + 1;}
            else X->l = std::max(X->expression1_->l , X->expression2_->l);
            bool b = X->expression2_->exp_type_.substr(0,6) == "struct" && X->expression2_->get_dereferenced_type() == "";
            if(b) X->l++;
            
        }
        else if(A->astnode_type == typeExp::op_binary_astnode){
            op_binary_astnode* X = dynamic_cast<op_binary_astnode*>(A);
            label(X->expression1_);
            label(X->expression2_);
            if(X->expression1_->l == 0) X->expression1_->l++;
            if(X->op_ == op_binary_type::DIV_INT){
                if(X->expression1_->l == X->expression2_->l) { X->l = std::max(X->expression1_->l + 1, 4);}
                else X->l = std::max(std::max(X->expression1_->l , X->expression2_->l), 4);
            }
            else{
                if(X->expression1_->l == X->expression2_->l) { X->l = X->expression1_->l + 1;}
                else X->l = std::max(X->expression1_->l , X->expression2_->l);
            }
        }
        else if(A->astnode_type == typeExp::op_unary_astnode){
            op_unary_astnode* X = dynamic_cast<op_unary_astnode*>(A);
            label(X->expression_);
            X->l = X->expression_->l+1;
        }
        else if(A->astnode_type == typeExp::assignE_astnode){
            assignE_astnode* X = dynamic_cast<assignE_astnode*>(A);
            label(X->expression1_);
            label(X->expression2_);
            if(X->expression1_->l == 0) X->expression1_->l++;
            if(X->expression1_->l == X->expression2_->l) { X->l = X->expression1_->l + 1;}
            else X->l = std::max(X->expression1_->l , X->expression2_->l) ;
        }
        else if(A->astnode_type == typeExp::funcall_astnode){
            funcall_astnode* X = dynamic_cast<funcall_astnode*>(A);
            int m = 0;
            for (unsigned int i = 0; i < X->arguments_.size(); i++) {
                label(X->arguments_[i]);
                if(X->arguments_[i]->l==0) X->arguments_[i]->l++;
                m = std::max(m,X->arguments_[i]->l);
            }
            m = std::max(m, global->get_GlobalSymbolTable_entry_by_name(X->name_)->get_ast()->l);
            X->l = m;
        }
        else if(A->astnode_type == typeExp::proccall_astnode){
            proccall_astnode* X = dynamic_cast<proccall_astnode*>(A);
            int m = 0;
            for (unsigned int i = 0; i < X->arguments_.size(); i++) {
                label(X->arguments_[i]);
                if(X->arguments_[i]->l==0) X->arguments_[i]->l++;
                m = std::max(m,X->arguments_[i]->l);
            }
            if(X->name_ != "printf"){
                m = std::max(m, global->get_GlobalSymbolTable_entry_by_name(X->name_)->get_ast()->l);
            }
            X->l = m;
        }
        else if(A->astnode_type == typeExp::member_astnode){
            member_astnode* X = dynamic_cast<member_astnode*>(A);
            label(X->expression_);
            if(X->expression_->l == 0) X->expression_->l++;
            X->l = X->expression_->l + 1;
        }
        else if(A->astnode_type == typeExp::arrayref_astnode){
            arrayref_astnode* X = dynamic_cast<arrayref_astnode*>(A);
            label(X->expression1_);
            label(X->expression2_);
            if(X->expression1_->l == 0) X->expression1_->l++;
            if(X->expression1_->l == X->expression2_->l) { X->l = X->expression1_->l + 1;}
            else X->l = std::max(X->expression1_->l , X->expression2_->l) ;
        }
        else if(A->astnode_type == typeExp::arrow_astnode){
            arrow_astnode* X = dynamic_cast<arrow_astnode*>(A);
            label(X->expression_);
            if(X->expression_->l == 0) X->expression_->l++;
            X->l = X->expression_->l + 1;
        }
        else if(A->astnode_type == typeExp::if_astnode){
            int m = 0;
            if_astnode* X = dynamic_cast<if_astnode*>(A);
            label(X->condition_);
            m = std::max(m,X->condition_->l);
            label(X->then_);
            m = std::max(m,X->then_->l);
            label(X->else_);
            m = std::max(m,X->else_->l);
            X->l = m;
        }
        else if(A->astnode_type == typeExp::while_astnode){
            int m = 0;
            while_astnode* X = dynamic_cast<while_astnode*>(A);
            label(X->condition_);
            m = std::max(m,X->condition_->l);
            label(X->body_);
            m = std::max(m,X->body_->l);
            X->l = m;
        }
        else if(A->astnode_type == typeExp::for_astnode){
            int m = 0;
            for_astnode* X = dynamic_cast<for_astnode*>(A);
            label(X->init_);
            m = std::max(m,X->init_->l);
            label(X->condition_);
            m = std::max(m,X->condition_->l);
            label(X->update_);
            m = std::max(m,X->update_->l);
            label(X->body_);
            m = std::max(m,X->body_->l);
            X->l = m;
        }
        else if(A->astnode_type == typeExp::return_astnode){
            return_astnode* X = dynamic_cast<return_astnode*>(A);
            label(X->expression_);
            if(X->expression_->l == 0) X->expression_->l++;
            X->l = X->expression_->l;
        }
        else if(A->astnode_type == typeExp::empty_astnode){
            A->l = 0;
        }
        else if(A->astnode_type == typeExp::identifier_astnode){
            A->l = 0;
        }
        else if(A->astnode_type == typeExp::int_astnode){
            A->l = 1;
        }
        else if(A->astnode_type == typeExp::string_astnode){
            A->l = 1;
        }
        return;
    }

    void gen(abstract_astnode* A){
        if(A->astnode_type == typeExp::empty_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::seq_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::assignS_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::if_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::while_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::for_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::return_astnode){
            A->gencode(0);
        }
        else if(A->astnode_type == typeExp::proccall_astnode){
            A->gencode(0);
        }
        return;
    }


    void are_there_printf(abstract_astnode* S){

        label(S);

        if(S->astnode_type == typeExp::seq_astnode){
            seq_astnode *A = dynamic_cast<seq_astnode*>(S);
            for (unsigned int i = 0; i < A->statements_.size(); i++) {
                are_there_printf(A->statements_[i]);
            }
        }
        else if(S->astnode_type == typeExp::if_astnode){
            if_astnode *A = dynamic_cast<if_astnode*>(S);
            are_there_printf(A->then_);
            are_there_printf(A->else_);
        }
        else if(S->astnode_type == typeExp::while_astnode){
            while_astnode *A = dynamic_cast<while_astnode*>(S);
            are_there_printf(A->body_);
        }
        else if(S->astnode_type == typeExp::for_astnode){
            for_astnode *A = dynamic_cast<for_astnode*>(S);
            are_there_printf(A->body_);
        }
        else if(S->astnode_type == typeExp::proccall_astnode){
            proccall_astnode *A = dynamic_cast<proccall_astnode*>(S);
            if(A->name_ == "printf"){
                if(found == false){
                    //std::cout<<"    .section    .rodata\n";
                    code_array.push_back("\t.section\t.rodata\n");
                    found = true;
                }
                for (unsigned int i = 0; i < A->arguments_.size(); i++) {
                    if(A->arguments_[i]->astnode_type == typeExp::string_astnode){
                        code_array.push_back(".LC" + std::to_string(printf_count) + ":\n");
                        dynamic_cast<string_astnode*>(A->arguments_[i])->str_label = "$.LC"+std::to_string(printf_count++);
                        //std::cout<<"    .string:    "<<dynamic_cast<string_astnode*>(P->arguments_[0])->value_<<"\n";
                        code_array.push_back("\t.string\t" + dynamic_cast<string_astnode*>(A->arguments_[i])->value_ + "\n");
                    }
                }
            }
        }
        return;
    }

    void swap(){
        const char* temp = reg[regi];
        reg[regi] = reg[regi+1];
        reg[regi+1] = temp;
        return;
    }

    std::vector<int> merge(std::vector<int> v1, std::vector<int> v2){
        v1.insert(v1.end(), v2.begin(), v2.end());
        return v1;
    }

    void backpatch(std::vector<int> v, std::string s){
        for(auto element : v){
            code_array[element].append(s);
        }
    }

    int nextinstr(){
        return code_array.size();
    }

    bool compareLocalSymbolTable_Entry( LocalSymbolTable_Entry* l1, LocalSymbolTable_Entry* l2){
        return (l1->lst_entry_name_ < l2->lst_entry_name_);
    }

    bool compareGlobalSymbolTable_Entry( GlobalSymbolTable_Entry* g1, GlobalSymbolTable_Entry* g2){
        return (g1->gst_entry_name_ < g2->gst_entry_name_);
    }

    bool contain(std::string str, std::string c){
        return (str.find(c) != std::string::npos);
    }

    empty_astnode::empty_astnode() {
        astnode_type = typeExp::empty_astnode;
    }


    int get_data_size(std::string type){
        // std::cout<<type<<"\n";

        if (!contain(type , "*")){
            //check for basic type
            int start = type.find("[");
            std::string basic_type = type.substr(0,start);
            int size;
            if(basic_type == "int"){
                size = 4;
            }
            else{
                GlobalSymbolTable_Entry* gst_entry = global->get_GlobalSymbolTable_entry_by_name(basic_type);
                size = gst_entry->gst_entry_size_;
            }
            while(start != std::string::npos){
                //return size of basic type
                int end = type.find("]", start+1);
                size = size * (std::stoi(type.substr(start+1,end-start-1)));
                start = type.find("[", end);
            }
            return size;
        }
        else{
            if(!contain(type , "(")){
                //size of basic type is 4
                int size = 4;
                int start = type.find("[");
                while(start != std::string::npos){
                    //return size of basic type
                    int end = type.find("]", start+1);
                    size = size * (std::stoi(type.substr(start+1,end-start-1)));
                    start = type.find("[", end);
                }
                return size;
            }
            else{
                return 4;
            }
        }
        // std::cout<<"000000000000000000000000\n";
        return 1;
    }

    std::string expression_astnode::get_dereferenced_type(){
        //if star, then remove one star
        if (!contain(exp_type_ , "[")){
            // Case 1 not associated to array 
            if(!contain(exp_type_ , "*")){
                return ""; //ERROR CASE
            }
            else{
                return exp_type_.substr(0,exp_type_.find("*")) + exp_type_.substr(exp_type_.find("*") + 1);
            }
        }
        else{
            if(!contain(exp_type_ , "(")){
                // Case 2a an array itself
                return exp_type_.substr(0,exp_type_.find("[")) + exp_type_.substr(exp_type_.find("]") + 1);
            }
            else{
                // Case 2b associated with pointer to array
                if (exp_type_.find(')')-exp_type_.find('(') == 2){
                    // Case 2bi a single pointer to an array 
                    return exp_type_.substr(0,exp_type_.find("(")) + exp_type_.substr(exp_type_.find(")")+1);
                }
                else{
                    // Case 2bii multi pointer to an array 
                    return exp_type_.substr(0,exp_type_.find("(")+1) + exp_type_.substr(exp_type_.find("(")+2);    
                }
            }
        }
    }

    seq_astnode::seq_astnode() {
        astnode_type = typeExp::seq_astnode;
    }


    void seq_astnode::add_statement(statement_astnode* statement) {
        statements_.push_back(statement);
    }


    void seq_astnode::gencode(int b){
        for (unsigned int i = 0; i < statements_.size(); i++) {
            gen(statements_[i]);
            if(statements_[i]->next.size()>0 && i != statements_.size()-1){
                std::string label = ".L" + std::to_string(label_count);
                label_count++;
                code_array.push_back(label + std::string(":\n"));
                backpatch(statements_[i]->next, label + std::string("\n"));
            }
            if(statements_[i]->next.size()>0 && i == statements_.size()-1){
                next = merge(next, statements_[i]->next);
            }
        }        
    }

    assignS_astnode::assignS_astnode(expression_astnode* expression1, expression_astnode* expression2) {
        astnode_type = typeExp::assignS_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
    }


    void assignS_astnode::gencode(int b){
        int n1 = expression1_->l, n2 = expression2_->l;

        if(n2 == 0){
            
            expression1_->gencode(0);
            
            int size = get_data_size(expression2_->exp_type_);
            int offsetR = current->get_lst_entry_by_name( dynamic_cast<identifier_astnode*>(expression2_)->name_)->lst_entry_offset_;
            bool b = expression2_->exp_type_.substr(0,6) == "struct" && expression2_->get_dereferenced_type() == "";
            if(b){
                for(int i = 0; i < size/4; i++){
                    code_array.push_back("\tmovl\t" + std::to_string(offsetR + (i+4)) + "(%ebp),\t"+ std::string(reg[regi+1]) + "\n");
                    code_array.push_back("\tmovl\t" + std::string(reg[regi+1]) +",\t"+ std::to_string(i*4) + "(" + std::string(reg[regi]) + ")\n");
                }
            }
            else{
                code_array.push_back("\tmovl\t" + std::to_string(offsetR) + "(%ebp),\t"+ std::string(reg[regi+1]) + "\n");
                code_array.push_back("\tmovl\t" + std::string(reg[regi+1]) +",\t("+ std::string(reg[regi]) + ")\n");
            }
        }
        else if(n1 < n2 && n1 < reg_size){
            swap();
            bool b = expression2_->exp_type_.substr(0,6) == "struct" && expression2_->get_dereferenced_type() == "";
            if(b){expression2_->gencode(0);}
            else {expression2_->gencode(1);}
            regi++;
            expression1_->gencode(0);
            if(b){
                int size = get_data_size(expression2_->exp_type_);
                for(int i = 0; i < size/4; i++){
                    code_array.push_back("\tmovl\t" + std::to_string(i*4) + "(" + reg[regi-1 ]+ "),\t"+ std::string(reg[regi+1]) + "\n");
                    code_array.push_back("\tmovl\t" + std::string(reg[regi+1]) +",\t"+ std::to_string(i*4) + "(" + std::string(reg[regi]) + ")\n");
                }
            }
            else {
                std::string t = std::string(reg[regi-1]);
                code_array.push_back("\tmovl\t" + std::string(std::string(reg[regi-1])) + ",\t(" + std::string(std::string(reg[regi])) + ")\n");
            }
            regi--;
            swap();
        }
        else if(n2 <= n1 && n2 < reg_size){
            expression1_->gencode(0);
            regi++;

            bool b = expression2_->exp_type_.substr(0,6) == "struct" && expression2_->get_dereferenced_type() == "";
            if(b){expression2_->gencode(0);}
            else {expression2_->gencode(1);}
            if(b){
                int size = get_data_size(expression2_->exp_type_);
                for(int i = 0; i < size/4; i++){
                    code_array.push_back("\tmovl\t" + std::to_string(i*4) + "(" + reg[regi]+ "),\t"+ std::string(reg[regi+1]) + "\n");
                    code_array.push_back("\tmovl\t" + std::string(reg[regi+1]) +",\t"+ std::to_string(i*4) + "(" + std::string(reg[regi-1]) + ")\n");
                }
            }
            else {
                code_array.push_back("\tmovl\t" + std::string(reg[regi]) + ",\t(" + std::string(reg[regi-1]) + ")\n");
            }
            
            regi--;
        }
        else if(n2 >= reg_size){
            bool b = expression2_->exp_type_.substr(0,6) == "struct" && expression2_->get_dereferenced_type() == "";
            if(b){expression2_->gencode(0);}
            else {expression2_->gencode(1);}
            code_array.push_back("\tpushl\t%" + std::string(reg[regi]) + "\n");
            
            expression1_->gencode(0);
            if(b){
                int size = get_data_size(expression2_->exp_type_);
                code_array.push_back("\tmovl\t(%esp),\t" + std::string(reg[regi+1]) + "\n");
                for(int i = 0; i < size/4; i++){
                    code_array.push_back("\tmovl\t" + std::to_string(i*4) + "(" + reg[regi+1 ]+ "),\t"+ std::string(reg[regi+2]) + "\n");
                    code_array.push_back("\tmovl\t" + std::string(reg[regi+2]) +",\t"+ std::to_string(i*4) + "(" + std::string(reg[regi]) + ")\n");
                }
            }
            else {
                code_array.push_back("\tmovl\t(%esp),\t(" + std::string(reg[regi]) + ")\n");
            }
            
            code_array.push_back("\tsubl\t$4,\t%esp\n");
        }

    }

    if_astnode::if_astnode(expression_astnode* Condition, statement_astnode* Then, statement_astnode* Else) {
        astnode_type = typeExp::if_astnode;
        condition_ = Condition;
        then_ = Then;
        else_ = Else;
    }


    void if_astnode::gencode(int b) {
        condition_->fall = true;
        condition_->is_bool = true;
        condition_->gencode(1);
        if(condition_->truelist.size()>0){
            std::string label = ".L" + std::to_string(label_count);
            label_count++;
            code_array.push_back(label + std::string(":\n"));
            backpatch(condition_->truelist, label+ std::string("\n"));
        }
        
        gen(then_);
        int temp = nextinstr();
        code_array.push_back(std::string("\t") + std::string("jmp\t"));
        then_->next.push_back(temp);

        std::string label = ".L" + std::to_string(label_count);
        label_count++;
        code_array.push_back(label + std::string(":\n"));
        backpatch(condition_->falselist, label+ std::string("\n"));

        gen(else_);

        next = merge(then_->next, else_->next);
    }


    while_astnode::while_astnode(expression_astnode* condition, statement_astnode* body) {
        astnode_type = typeExp::while_astnode;
        condition_ = condition;
        body_ = body;
    }


    void while_astnode::gencode(int b) {
        int start = nextinstr();
        code_array.push_back(std::string("\t") + std::string("jmp\t"));

        std::string body_label = ".L" + std::to_string(label_count);
        label_count++;
        code_array.push_back(body_label + std::string(":\n"));

        gen(body_);

        std::string cond_label = ".L" + std::to_string(label_count);
        label_count++;
        code_array.push_back(cond_label + std::string(":\n"));

        code_array[start].append(cond_label + std::string("\n"));

        condition_->fall = false;
        condition_->is_bool = true;
        condition_->gencode(1);
        backpatch(condition_->truelist, body_label+ std::string("\n"));
        backpatch(body_->next, cond_label+ std::string("\n"));
        next = merge(next, condition_->falselist);
    }

    for_astnode::for_astnode(expression_astnode* init, expression_astnode* condition, expression_astnode* update, statement_astnode* body) {
        astnode_type = typeExp::for_astnode;
        init_ = init;
        condition_ = condition;
        update_ = update;
        body_ = body;
    }


    void for_astnode::gencode(int b) {

        init_->gencode(1);

        std::string cond_label = ".L" + std::to_string(label_count);
        label_count++;
        code_array.push_back(cond_label+ std::string(":\n"));
        
        condition_->fall = true;
        condition_->is_bool = true;
        condition_->gencode(1);

        if(condition_->truelist.size()>1){
            std::string label = ".L" + std::to_string(label_count);
            label_count++;
            code_array.push_back(label + std::string(":\n"));
            backpatch(condition_->truelist, label+ std::string("\n"));
        }

        gen(body_);

        if(body_->next.size()>0){
            std::string label = ".L" + std::to_string(label_count);
            label_count++;
            code_array.push_back(label + std::string(":\n"));
            backpatch(body_->next, label+ std::string("\n"));
        }

        update_->gencode(1);

        int temp = nextinstr();
        code_array.push_back(std::string("\t") + std::string("jmp\t"));
        code_array[temp].append(cond_label+ std::string("\n"));

        next = merge(next, condition_->falselist);
        
    }

    return_astnode::return_astnode(expression_astnode* expression) {
        astnode_type = typeExp::return_astnode;
        expression_ = expression;
    }

    void return_astnode::gencode(int b){
        bool s = expression_->exp_type_.substr(0,6) == "struct" && expression_->get_dereferenced_type() == "";
        if(s){expression_->gencode(0);}
        else {expression_->gencode(1);}
        code_array.push_back("\tmovl\t"+std::string(reg[regi])+",\t8(%ebp)\n");
        
        code_array.push_back("\tmovl\t%ebp,\t%esp\n");
        if(is_it_main){
            code_array.push_back("\tmovl\t$0,\t%eax\n");
        }
        code_array.push_back("\tleave\n\tret\n");
        if(is_it_main){
            is_it_main = false;
        }

        
    }

    
    proccall_astnode::proccall_astnode(std::string name) {
        astnode_type = typeExp::proccall_astnode;
        name_ = name;
        arguments_ = std::vector<expression_astnode*>();
    }

    void proccall_astnode::gencode(int b){
      
        if(name_ == "printf"){
            for (unsigned int j = 0 ; j < arguments_.size();  j++) {
                int i = arguments_.size()-1 - j;
                if(arguments_[i]->astnode_type == typeExp::string_astnode){
                    code_array.push_back("\tpushl\t" + dynamic_cast<string_astnode*>(arguments_[i])->str_label + "\n");
                }
                else{
                    arguments_[i]->gencode(1);
                    code_array.push_back("\tpushl\t" + std::string(reg[regi]) + "\n");
                }
            }
            code_array.push_back("\tcall\t" + name_ + "\n");
            return;
        }
        for (unsigned int i = 0; i < arguments_.size(); i++) {
            bool s = arguments_[i]->exp_type_.substr(0,6) == "struct" && arguments_[i]->get_dereferenced_type() == "";
            if(s){arguments_[i]->gencode(0);}
            else {
                if(!contain(arguments_[i]->exp_type_,"(") && contain(arguments_[i]->exp_type_,"[")){
                    arguments_[i]->gencode(0);
                }
                else arguments_[i]->gencode(1);
            }
            if(s){
                int size = get_data_size(arguments_[i]->exp_type_);
                for(int i = (size/4)-1; i >= 0; i--){
                    code_array.push_back("\tpushl\t" + std::to_string((i*4)) + "(" + std::string(reg[regi]) + ")\n");
                }
            }
            else {
                code_array.push_back("\tpushl\t" + std::string(reg[regi]) + "\n");
            }
        }
        code_array.push_back("\tsubl\t$4,\t%esp\n");
       
        code_array.push_back("\tcall\t" + name_ + "\n");
        int byte_jump = 4;
        for (unsigned int i = 0; i < arguments_.size(); i++) {
            byte_jump +=  get_data_size(arguments_[i]->exp_type_);
        }
        code_array.push_back("\taddl\t$"+std::to_string(byte_jump)+",\t%esp\n");
    
    }


    void proccall_astnode::add_argument(expression_astnode* argument) {
        arguments_.push_back(argument);
    }

    op_binary_astnode::op_binary_astnode(expression_astnode* expression1, expression_astnode* expression2, enum op_binary_type op) {
        astnode_type = typeExp::op_binary_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
        op_ = op;
    }


    void rel_bool(std::string a, std::string b, op_binary_astnode *B){
        if(int(B->op_) < 8){
            code_array.push_back("\tcmpl\t"+a+",\t"+b+"\n");
            if(B->fall){
                B->falselist.push_back(nextinstr());
                code_array.push_back("\t"+std::string(tfall[int(B->op_)-2])+"\t");
            }
            else{
                B->truelist.push_back(nextinstr());
                code_array.push_back("\t"+std::string(ffall[int(B->op_)-2])+"\t");
            }
            if(!B->is_bool){
                //it is a value
                //add code for value
                if(B->fall){
                    if(B->truelist.size()>0){
                        std::string label_start = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label_start + std::string(":\n"));
                        backpatch(B->truelist, label_start+ std::string("\n"));
                    }

                    code_array.push_back(std::string("\tmovl\t")+std::string("$1,\t")+std::string(b)+std::string("\n")); //change this

                    int temp = nextinstr();
                    code_array.push_back(std::string("\t") + std::string("jmp\t"));
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(B->falselist, label + std::string("\n"));

                    code_array.push_back(std::string("\tmovl\t")+std::string("$0,\t")+std::string(b) + std::string("\n")); //change this
                    std::string label2 = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label2 + std::string(":\n"));
                    code_array[temp].append(label2+ std::string("\n"));
                    
                    //check if val is in top register
                }
                else{
                    //ask if this is same
                    if(B->falselist.size()>0){
                        std::string label_start = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label_start + std::string(":\n"));
                        backpatch(B->falselist, label_start + std::string("\n"));
                    }
                    code_array.push_back(std::string("\tmovl\t")+std::string("$0,\t") + std::string(b) + std::string("\n")); //change this

                    int temp = nextinstr();
                    code_array.push_back(std::string("\t") + std::string("jmp\t"));
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(B->truelist, label + std::string("\n"));

                    code_array.push_back(std::string("\tmovl\t") + std::string("$1,\t") + std::string(b) + std::string("\n")); //change this
                    std::string label2 = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label2 + std::string(":\n"));
                    code_array[temp].append(label2+ std::string("\n"));
                }

            }
        }
        else{
            std::string opr;
            if(B->op_ == op_binary_type::PLUS_INT) { opr = "addl";}
            else if(B->op_ == op_binary_type::MINUS_INT) { opr = "subl";}
            else if(B->op_ == op_binary_type::MULT_INT) { opr = "imull";}
            else if(B->op_ == op_binary_type::DIV_INT) { opr = "idivl";}
            if(opr != "idivl"){
                if(opr == "addl"){
                    if(B->expression1_->get_dereferenced_type() != "" && B->expression2_->exp_type_ == "int"){
                        if(contain(a,"(")){
                            code_array.push_back("\tmovl\t" + a +",\t"+ std::string(reg[regi+1]) +"\n");
                            code_array.push_back("\timull\t$" + std::to_string(get_data_size(B->expression1_->get_dereferenced_type())) +",\t" + reg[regi+1] + "\n");
                            code_array.push_back("\t"+ opr + "\t" + reg[regi+1] +",\t" + b + "\n");
                        }
                        else{
                            code_array.push_back("\timull\t$" + std::to_string(get_data_size(B->expression1_->get_dereferenced_type())) +",\t" + a + "\n");
                            code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
                        }
                    }
                    else if(B->expression2_->get_dereferenced_type() != "" && B->expression1_->exp_type_ == "int"){
                        if(contain(a,"(")){
                            code_array.push_back("\tmovl\t" + a +",\t"+ std::string(reg[regi+1]) +"\n");
                            code_array.push_back("\timull\t$" + std::to_string(get_data_size(B->expression2_->get_dereferenced_type())) +",\t" + reg[regi+1] + "\n");
                            code_array.push_back("\t"+ opr + "\t" + reg[regi+1] +",\t" + b + "\n");
                        }
                        else{
                            code_array.push_back("\timull\t$" + std::to_string(get_data_size(B->expression2_->get_dereferenced_type())) +",\t" + a + "\n");
                            code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
                        }
                    }
                    else {
                        code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
                    }
                }
                else if(opr == "subl"){
                    if(B->expression1_->get_dereferenced_type() != "" && B->expression2_->exp_type_ == "int"){
                        if(contain(a,"(")){
                            code_array.push_back("\tmovl\t" + a +",\t"+ std::string(reg[regi+1]) +"\n");
                            code_array.push_back("\timull\t$" + std::to_string(get_data_size(B->expression1_->get_dereferenced_type())) +",\t" + reg[regi+1] + "\n");
                            code_array.push_back("\t"+ opr + "\t" + reg[regi+1] +",\t" + b + "\n");
                        }
                        else{
                            code_array.push_back("\timull\t$" + std::to_string(get_data_size(B->expression1_->get_dereferenced_type())) +",\t" + a + "\n");
                            code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
                        }
                    }
                    else if(B->expression1_->get_dereferenced_type() != "" && B->expression2_->get_dereferenced_type() != "" && (B->expression2_->get_dereferenced_type() == B->expression1_->get_dereferenced_type())){
                        code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
                        code_array.push_back("\tmovl\t" + b +",\t%eax\n");
                        code_array.push_back("\tmovl\t$" + std::to_string(get_data_size(B->expression2_->get_dereferenced_type())) +",\t"+ std::string(reg[regi]) +"\n");
                        code_array.push_back("\tcltd\n");
                        code_array.push_back("\tidivl\t" + std::string(reg[regi]) +"\n");
                        code_array.push_back("\tmovl\t%eax,\t"+ b +"\n");
                    }
                    else {
                        code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
                    }
                }
                else code_array.push_back("\t"+ opr + "\t" + a +",\t" + b + "\n");
            }
            else{
                code_array.push_back("\tmovl\t" + b +",\t%eax\n");
                if(contain(a,"(")){
                    code_array.push_back("\tmovl\t" + a +",\t"+ std::string(reg[regi]) +"\n");
                    code_array.push_back("\tcltd\n");
                    code_array.push_back("\tidivl\t" + std::string(reg[regi]) +"\n");
                }
                else{
                    code_array.push_back("\tcltd\n");
                    code_array.push_back("\tidivl\t" + a +"\n");
                }
                code_array.push_back("\tmovl\t%eax,\t"+ b +"\n");
                
            }
            if(B->is_bool){
                code_array.push_back("\ttestl\t" + b + "," + b +"\n");
                if(B->fall){
                    B->falselist.push_back(nextinstr());
                    code_array.push_back(std::string("\t") + std::string("je\t"));
                }
                else{
                    B->truelist.push_back(nextinstr());
                    code_array.push_back(std::string("\t") + std::string("jne\t"));
                }
            }
        }
    }
    
    void op_binary_astnode::gencode(int b){

        if(op_ == op_binary_type::OR_OP || op_ == op_binary_type::AND_OP) {
            // left and then right
            //set both as bool
            expression1_->is_bool = true;
            expression2_->is_bool = true;
            if(op_ == op_binary_type::OR_OP){
                expression1_->fall = false;
                expression2_->fall = fall;
                expression1_->gencode(1); //1 is for value right?
                if(expression1_->falselist.size()>0){
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(expression1_->falselist, label + std::string("\n"));
                }
                expression2_->gencode(1);
                truelist = merge(expression1_->truelist, expression2_->truelist);
                falselist = expression2_->falselist;
            }
            else{
                expression1_->fall = true;
                expression2_->fall = fall;
                expression1_->gencode(1); //1 is for value right?
                if(expression1_->truelist.size()>0){
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(expression1_->truelist, label + std::string("\n"));
                }
                expression2_->gencode(1);
                falselist = merge(expression1_->falselist, expression2_->falselist);
                truelist = expression2_->truelist;
            }
            if(fall){
                if(!is_bool){
                    if(truelist.size()>0){
                        std::string label = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label + std::string(":\n"));
                        backpatch(truelist, label + std::string("\n"));
                    }
                    code_array.push_back(std::string("\tmovl\t") + std::string("$1,\t") + std::string(reg[regi]) + std::string("\n")); //change this

                    int temp = nextinstr();
                    code_array.push_back(std::string("\t") + std::string("jmp\t"));
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(falselist, label + std::string("\n"));

                    code_array.push_back(std::string("\tmovl\t") + std::string("$0,\t") + std::string(reg[regi]) + std::string("\n")); //change this
                    std::string label2 = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label2 + std::string(":\n"));
                    code_array[temp].append(label2 + std::string("\n"));
                }

            }
            else{
                if(!is_bool){
                    if(falselist.size()>0){
                        std::string label = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label + std::string(":\n"));
                        backpatch(falselist, label + std::string("\n"));
                    }
                    code_array.push_back(std::string("\tmovl\t") + std::string("$0,\t") + std::string(reg[regi]) + std::string("\n") ); //change this

                    int temp = nextinstr();
                    code_array.push_back(std::string("\t") + std::string("jmp\t"));
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(truelist, label + std::string("\n"));

                    code_array.push_back(std::string("\tmovl\t") + std::string("$1,\t") + std::string(reg[regi]) + std::string("\n")); //change this
                    std::string label2 = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label2 + std::string(":\n"));
                    code_array[temp].append(label2 + std::string("\n"));
                }
            }

        }
        else{
            expression1_->is_bool = false;
            expression2_->is_bool = false;
            //code_array.push_back("in binary with " + a + "\n");
            if(b == 1){
                int n1 = expression1_->l, n2 = expression2_->l;

                if(n2 == 0){
                    expression1_->gencode(1);
                    int offsetR = current->get_lst_entry_by_name( dynamic_cast<identifier_astnode*>(expression2_)->name_)->lst_entry_offset_;
                    
                    rel_bool(std::to_string(offsetR) + "(%ebp)", std::string(reg[regi]), this);
                    
                }
                else if(n1 < n2 && n1 < reg_size){
                    swap();
                    
                    expression2_->gencode(1);
                    regi++;
                   
                    expression1_->gencode(1);
                   
                    rel_bool(std::string(reg[regi-1]), std::string(reg[regi]), this);
                  
                    regi--;
                    swap();
                }
                else if(n2 <= n1 && n2 < reg_size){
                    expression1_->gencode(1);
                    regi++;
                    expression2_->gencode(1);

                    rel_bool(std::string(reg[regi]), std::string(reg[regi-1]), this);
                    
                    regi--;
                }
                else if(n2 >= reg_size){
                    
                    expression2_->gencode(1);
                    code_array.push_back("\tpushl\t%" + std::string(reg[regi]) + "\n");
                    expression1_->gencode(1);
                    
                    rel_bool("(%esp)", std::string(reg[regi]), this);
                   
                    code_array.push_back("\tsubl\t$4,\t%esp\n");
                }
            }
        }
    }

    op_unary_astnode::op_unary_astnode(expression_astnode* expression, enum op_unary_type op) {
        astnode_type = typeExp::op_unary_astnode;
        expression_ = expression;
        op_ = op;
    }

    void op_unary_astnode::gencode(int b){

       
        if (op_ == op_unary_type::DEREF){
            
            expression_->gencode(1);
            if(b==1){
                code_array.push_back("\tmovl\t(" + std::string(reg[regi]) + "),\t" + std::string(reg[regi]) + "\n");
            }
        } 
        else if (op_ == op_unary_type::ADDRESS){
            expression_->gencode(0);
        }
        else if (op_ == op_unary_type::PP){
            swap();
            expression_->gencode(0);
            code_array.push_back("\tmovl\t("+std::string(reg[regi])+"),\t"+ std::string(reg[regi+1]) + "\n");
            code_array.push_back("\taddl\t$1,\t("+ std::string(reg[regi]) + ")\n");
            swap();
        }
        else if (op_ == op_unary_type::UMINUS){
            swap();
            expression_->gencode(1);
            swap();
            code_array.push_back("\tmovl\t$0,\t" + std::string(reg[regi]) + "\n");
            code_array.push_back("\tsubl\t" + std::string(reg[regi+1]) + ",\t" + std::string(reg[regi]) + "\n");
        }
        else if (op_ == op_unary_type::NOT){

            std::string a;
            if(expression_->l == 0){
                int offset = current->get_lst_entry_by_name( dynamic_cast<identifier_astnode*>(expression_)->name_)->lst_entry_offset_;
                code_array.push_back("\tmovl\t" + std::to_string(offset) + "(%ebp),\t"+ std::string(reg[regi]) + "\n");
                a =  std::string(reg[regi]);
            }
            else{
                expression_->gencode(1);
                a = std::string(reg[regi]);
            }
            code_array.push_back("\tcmpl\t$0,\t"+a+"\n");
            if(fall){
                falselist.push_back(nextinstr());
                code_array.push_back("\tjne\t");
            }
            else{
                truelist.push_back(nextinstr());
                code_array.push_back("\tje\t");
            }
            if(!is_bool){
                if(fall){
                    if(truelist.size()>0){
                        std::string label_start = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label_start + std::string(":\n"));
                        backpatch(truelist, label_start+ std::string("\n"));
                    }
                    code_array.push_back(std::string("\tmovl\t")+std::string("$1,\t")+std::string(a)+std::string("\n")); //change this

                    int temp = nextinstr();
                    code_array.push_back(std::string("\t") + std::string("jmp\t"));
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(falselist, label + std::string("\n"));

                    code_array.push_back(std::string("\tmovl\t")+std::string("$0,\t")+std::string(a) + std::string("\n")); //change this
                    std::string label2 = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label2 + std::string(":\n"));
                    code_array[temp].append(label2+ std::string("\n"));
                    
                    //check if val is in top register
                }
                else{
                    //ask if this is same
                    if(falselist.size()>0){
                        std::string label_start = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label_start + std::string(":\n"));
                        backpatch(falselist, label_start + std::string("\n"));
                    }
                    code_array.push_back(std::string("\tmovl\t")+std::string("$0,\t") + std::string(a) + std::string("\n")); //change this

                    int temp = nextinstr();
                    code_array.push_back(std::string("\t") + std::string("jmp\t"));
                    std::string label = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label + std::string(":\n"));
                    backpatch(truelist, label + std::string("\n"));

                    code_array.push_back(std::string("\tmovl\t") + std::string("$1,\t") + std::string(a) + std::string("\n")); //change this
                    std::string label2 = ".L" + std::to_string(label_count);
                    label_count++;
                    code_array.push_back(label2 + std::string(":\n"));
                    code_array[temp].append(label2+ std::string("\n"));
                }

            }
        }
    }

    int_astnode::int_astnode(int value) {
        astnode_type = typeExp::int_astnode;
        value_ = value;
        exp_type_ = "int";
    }

    void int_astnode::gencode(int b){
        if( b == 0){}
        else {
            //std::cout<<"    movl    $"<< value_ <<"    "<<std::string(reg[regi])<<"\n";
            code_array.push_back("\t movl\t$" + std::to_string(value_) + ",\t" + std::string(reg[regi]) + "\n");
        }
    }

    float_astnode::float_astnode(float value) {
        astnode_type = typeExp::float_astnode;
        value_ = value;
        exp_type_ = "float";
    }

    string_astnode::string_astnode(std::string value) {
        astnode_type = typeExp::string_astnode;
        value_ = value;
        exp_type_ = "string";
    }

    assignE_astnode::assignE_astnode(expression_astnode* expression1, expression_astnode* expression2) {
        astnode_type = typeExp::assignE_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
    }

    expression_astnode* assignE_astnode::get_expression1() {
        return expression1_;
    }

    expression_astnode* assignE_astnode::get_expression2() {
        return expression2_;
    }

    void assignE_astnode::gencode(int b){ //to be used in for loop
        assignS_astnode* temp = new assignS_astnode(expression1_,expression2_);
        temp->gencode(0);
        code_array.push_back("\tmovl\t(" + std::string(reg[regi]) + "),\t" + std::string(reg[regi]) + "\n");
    }

    funcall_astnode::funcall_astnode(std::string name) {
        astnode_type = typeExp::funcall_astnode;
        name_ = name;
        arguments_ = std::vector<expression_astnode*>();
    }

    void funcall_astnode::gencode(int b){
        for (unsigned int i = 0; i < arguments_.size(); i++) {
            bool s = arguments_[i]->exp_type_.substr(0,6) == "struct" && arguments_[i]->get_dereferenced_type() == "";
            if(s){arguments_[i]->gencode(0);}
            else {
                if(!contain(arguments_[i]->exp_type_,"(") && contain(arguments_[i]->exp_type_,"[")){
                    arguments_[i]->gencode(0);
                }
                else arguments_[i]->gencode(1);
            }
            if(s){
                int size = get_data_size(arguments_[i]->exp_type_);
                for(int i = (size/4)-1; i >= 0; i--){
                    code_array.push_back("\tpushl\t" + std::to_string((i*4)) + "(" + std::string(reg[regi]) + ")\n");
                }
            }
            else {
                code_array.push_back("\tpushl\t" + std::string(reg[regi]) + "\n");
            }
        }
        code_array.push_back("\tsubl\t$4,\t%esp\n");
        code_array.push_back("\tcall\t" + name_ + "\n");
        if(b==0){
            bool is_str = exp_type_.substr(0,6) == "struct" && get_dereferenced_type() == "";
            if(is_str){
                code_array.push_back("\tmovl\t(%esp),\t" + std::string(reg[regi]) + "\n");
            }
            else {
                code_array.push_back("\tmovl\t%esp,\t" + std::string(reg[regi]) + "\n");
            }
            
        }
        else{
            code_array.push_back("\tmovl\t(%esp),\t" + std::string(reg[regi]) + "\n");
        }
        int byte_jump = 4;
        for (unsigned int i = 0; i < arguments_.size(); i++) {
            byte_jump +=  get_data_size(arguments_[i]->exp_type_);
        }
        code_array.push_back("\taddl\t$"+std::to_string(byte_jump)+",\t%esp\n");
    }

    void funcall_astnode::add_argument(expression_astnode* argument) {
        arguments_.push_back(argument);
    }


    identifier_astnode::identifier_astnode(std::string name) {
        astnode_type = typeExp::identifier_astnode;
        name_ = name;
    }


    void ref_bool(std::string a, std::string b, expression_astnode *B){
        if(B->is_bool){
            code_array.push_back("\ttestl\t" + b + "," + b +"\n");
            if(B->fall){
                B->falselist.push_back(nextinstr());
                code_array.push_back(std::string("\t") + std::string("je\t"));
            }
            else{
                B->truelist.push_back(nextinstr());
                code_array.push_back(std::string("\t") + std::string("jne\t"));
            }
        }
    }

    void identifier_astnode::gencode(int b){
        int offset = current->get_lst_entry_by_name(name_)->lst_entry_offset_;
        if( b == 0){
            //std::cout<<"    leal    "<< offset <<"(%ebp)    "<<std::string(reg[regi])<<"\n";
            code_array.push_back("\tleal\t" +  std::to_string(offset) + "(%ebp),\t" + std::string(reg[regi]) + "\n");
        }
        else {
            //std::cout<<"    movl    "<< offset <<"(%ebp)    "<<std::string(reg[regi])<<"\n";
            code_array.push_back("\tmovl\t" + std::to_string(offset) + "(%ebp),\t" + std::string(reg[regi]) + "\n");
            ref_bool(std::to_string(offset) + "(%ebp)", std::string(reg[regi]), this);
                    
        }
    }

    member_astnode::member_astnode(expression_astnode* expression, identifier_astnode* name) {
        astnode_type = typeExp::member_astnode;
        expression_ = expression;
        name_ = name;
    }

    void member_astnode::gencode(int b){
        expression_->gencode(0);
        int offset = global->get_GlobalSymbolTable_entry_by_name(expression_->exp_type_)->get_lst()->get_lst_entry_by_name(dynamic_cast<identifier_astnode*>(name_)->name_)->lst_entry_offset_;
        //std::cout<<"    addl    $"<< offset <<"    "<<std::string(reg[regi])<<"\n";
        code_array.push_back("\taddl\t$" + std::to_string(offset) + ",\t" + std::string(reg[regi]) + "\n");
        if(b == 1){
            //std::cout<<"    movl    ("<< std::string(reg[regi]) <<")    "<<std::string(reg[regi])<<"\n";
            code_array.push_back("\tmovl\t(" + std::string(reg[regi]) + "),\t" + std::string(reg[regi]) + "\n");
            ref_bool(std::string(reg[regi]), std::string(reg[regi]), this);
        }
    }

    arrayref_astnode::arrayref_astnode(expression_astnode* expression1, expression_astnode* expression2) {
        astnode_type = typeExp::arrayref_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
    }

    void arrayref_astnode::gencode(int b){

        int n1 = expression1_->l, n2 = expression2_->l;
        int exp1_b = 0;
        if(expression1_->astnode_type == typeExp::identifier_astnode){
            LocalSymbolTable_Entry* X = current->get_lst_entry_by_name(dynamic_cast<identifier_astnode*>(expression1_)->name_);
            if(contain(X->lst_entry_sub_type_,"*") || (X->lst_entry_scope_ == Scope::Param && contain(X->lst_entry_sub_type_,"["))){
                exp1_b = 1;
            }
        }

        if(n2 == 0){
            expression1_->gencode(exp1_b);
            regi++;
            int offset = current->get_lst_entry_by_name( dynamic_cast<identifier_astnode*>(expression2_)->name_)->lst_entry_offset_;
            code_array.push_back("\tmovl\t" +  std::to_string(offset) + "(%ebp),\t" + std::string(reg[regi]) + "\n");
            code_array.push_back("\timull\t$" + std::to_string(get_data_size(exp_type_)) + ",\t" + std::string(reg[regi]) + "\n");
            code_array.push_back("\taddl\t" + std::string(reg[regi]) + ",\t" + std::string(reg[regi-1]) + "\n");
            regi--;
        }
        else if(n1 < n2 && n1 < reg_size){
            swap();
            expression2_->gencode(1);
            code_array.push_back("\timull\t$" + std::to_string(get_data_size(exp_type_)) + ",\t" + std::string(reg[regi]) + "\n");
            regi++;
            expression1_->gencode(exp1_b);
            code_array.push_back("\taddl\t" + std::string(reg[regi-1]) + ",\t" + std::string(reg[regi]) + "\n");
            regi--;
            swap();
        }
        else if(n2 <= n1 && n2 < reg_size){
            expression1_->gencode(exp1_b);
            regi++;
            expression2_->gencode(1);
            code_array.push_back("\timull\t$" + std::to_string(get_data_size(exp_type_)) + ",\t" + std::string(reg[regi]) + "\n");
            code_array.push_back("\taddl\t" + std::string(reg[regi]) + ",\t" + std::string(reg[regi-1]) + "\n");
            regi--;
        }
        else if(n2 >= reg_size){
            expression2_->gencode(1);
            code_array.push_back("\timull\t$" + std::to_string(get_data_size(exp_type_)) +",\t" + std::string(reg[regi]) + "\n");
            code_array.push_back("\tpushl\t%" + std::string(reg[regi]) + "\n");
            expression1_->gencode(exp1_b);
            code_array.push_back("\taddl\t(%esp),\t" + std::string(reg[regi]) + "\n");
            code_array.push_back("\tsubl\t$4,\t%esp\n");
        }
        if(b == 1){
            code_array.push_back("\tmovl\t(" + std::string(reg[regi]) +"),\t" + std::string(reg[regi]) + "\n");
            ref_bool(std::string(reg[regi]), std::string(reg[regi]), this);
        }
    }

    arrow_astnode::arrow_astnode(expression_astnode* expression, identifier_astnode* name) {
        astnode_type = typeExp::arrow_astnode;
        expression_ = expression;
        name_ = name;
    }

    void arrow_astnode::gencode(int b){

        
        expression_->gencode(1);
        int offset = global->get_GlobalSymbolTable_entry_by_name(expression_->get_dereferenced_type())->get_lst()->get_lst_entry_by_name(dynamic_cast<identifier_astnode*>(name_)->name_)->lst_entry_offset_;
        code_array.push_back("\taddl\t$" + std::to_string(offset) + ",\t" + std::string(reg[regi]) + "\n");
        if(b == 1){
            code_array.push_back("\tmovl\t(" + std::string(reg[regi]) + "),\t"+ std::string(reg[regi]) + "\n");
            ref_bool(std::string(reg[regi]), std::string(reg[regi]), this);
        }
    }

    LocalSymbolTable_Entry::LocalSymbolTable_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset)
    {   
        lst_entry_scope_ = scope;
        lst_entry_sub_type_ = sub_type;
        lst_entry_offset_ = offset;
        lst_entry_type_ = type;
        lst_entry_name_ = name;
        lst_entry_size_ = size;
        
    }

    Scope LocalSymbolTable_Entry::get_lst_entry_scope()
    {
        return lst_entry_scope_;
    }

    std::string LocalSymbolTable_Entry::get_lst_entry_type_name()
    {
        switch(lst_entry_type_)
        {
        case SymbolType::Var:
            return "var";
        case SymbolType::Const:
            return "const";
        case SymbolType::Func:
            return "func";
        case SymbolType::Struct:
            return "struct";
        default:
            return "unknown";
        }
    }

    std::string LocalSymbolTable_Entry::get_lst_entry_scope_name()
    {
        switch(lst_entry_scope_)
        {
        case Scope::Global:
            return "global";
        case Scope::Local:
            return "local";
        case Scope::Param:
            return "param";
        default:
            return "unknown";
        }
    }

    int LocalSymbolTable_Entry::get_lst_entry_size()
    {
        return lst_entry_size_;
    }


    void LocalSymbolTable_Entry::set_lst_entry_name(std::string name)
    {
        lst_entry_name_ = name;
    }


    void LocalSymbolTable_Entry::set_lst_entry_sub_type(std::string sub_type)
    {
        lst_entry_sub_type_= sub_type;
    }

    std::string LocalSymbolTable_Entry::get_dereferenced_type(){
        //if star, then remove one star
        if (!contain(lst_entry_sub_type_ , "[")){
            // Case 1 not associated to array 
            if(!contain(lst_entry_sub_type_ , "*")){
                return ""; //ERROR CASE
            }
            else{
                return lst_entry_sub_type_.substr(0,lst_entry_sub_type_.find("*")) + lst_entry_sub_type_.substr(lst_entry_sub_type_.find("*") + 1);
            }
        }
        else{
            if(!contain(lst_entry_sub_type_ , "(")){
                // Case 2a an array itself
                return lst_entry_sub_type_.substr(0,lst_entry_sub_type_.find("[")) + lst_entry_sub_type_.substr(lst_entry_sub_type_.find("]") + 1);
            }
            else{
                // Case 2b associated with pointer to array
                if (lst_entry_sub_type_.find(')')-lst_entry_sub_type_.find('(') == 2){
                    // Case 2bi a single pointer to an array 
                    return lst_entry_sub_type_.substr(0,lst_entry_sub_type_.find("(")) + lst_entry_sub_type_.substr(lst_entry_sub_type_.find(")")+1);
                }
                else{
                    // Case 2bii multi pointer to an array 
                    return lst_entry_sub_type_.substr(0,lst_entry_sub_type_.find("(")+1) + lst_entry_sub_type_.substr(lst_entry_sub_type_.find("(")+2);    
                }
            }
        }
    }

    LocalSymbolTable::LocalSymbolTable()
    {
        lst_entries_ = std::vector<LocalSymbolTable_Entry *>();
    }

    LocalSymbolTable_Entry *LocalSymbolTable::get_lst_entry_by_name(std::string name)
    {
        for(unsigned int i = 0; i <lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->lst_entry_name_ == name)
            {
                return lst_entries_[i];
            }
        }
        return nullptr;
    }

    bool LocalSymbolTable::add_lst_entry(LocalSymbolTable_Entry *entry)
    {
        LocalSymbolTable_Entry* lst_entry = this->get_lst_entry_by_name(entry->lst_entry_name_);
        if(lst_entry != nullptr && lst_entry->lst_entry_type_ == entry->lst_entry_type_){
            return false;
        }
        lst_entries_.push_back(entry);
        return true;
    }

    int LocalSymbolTable::get_size()
    {
        int total_size = 0;
        for(unsigned int i = 0; i < lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->get_lst_entry_scope_name() == "local") total_size += lst_entries_[i]->lst_entry_size_;
        }
        return total_size;
    }

    std::vector<LocalSymbolTable_Entry *> LocalSymbolTable::get_lst_entries()
    {
        return lst_entries_;
    }

    
    GlobalSymbolTable_Entry::GlobalSymbolTable_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset)
    {
        gst_entry_name_ = name;
        gst_entry_type_ = type;
        gst_entry_scope_ = scope;
        gst_entry_sub_type_ = sub_type;
        gst_entry_size_ = size;
        gst_entry_offset_ = offset;
        lst_ = NULL;
        ast_ = NULL;
    }

    GlobalSymbolTable_Entry::GlobalSymbolTable_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset, LocalSymbolTable *lst)
    {
        gst_entry_name_ = name;
        gst_entry_type_ = type;
        gst_entry_scope_ = scope;
        gst_entry_sub_type_= sub_type;
        gst_entry_size_ = size;
        gst_entry_offset_ = offset;
        lst_ = lst;
        ast_ = NULL;
    }

    SymbolType GlobalSymbolTable_Entry::get_gst_entry_type()
    {
        return gst_entry_type_;
    }

    std::string GlobalSymbolTable_Entry::get_gst_entry_type_name()
    {
        switch(gst_entry_type_)
        {
        case SymbolType::Var:
            return "var";
        case SymbolType::Const:
            return "const";
        case SymbolType::Func:
            return "fun";
        case SymbolType::Struct:
            return "struct";
        default:
            return "unknown";
        }
    }

    std::string GlobalSymbolTable_Entry::get_gst_entry_scope_name()
    {
        switch(gst_entry_scope_)
        {
        case Scope::Global:
            return "global";
        case Scope::Local:
            return "local";
        case Scope::Param:
            return "param";
        default:
            return "unknown";
        }
    }

    int GlobalSymbolTable_Entry::get_gst_entry_size()
    {
        return gst_entry_size_;
    }

    LocalSymbolTable *GlobalSymbolTable_Entry::get_lst()
    {
        return lst_;
    }

    void GlobalSymbolTable_Entry::set_gst_entry_type(SymbolType type)
    {
        gst_entry_type_= type;
    }


    void GlobalSymbolTable_Entry::set_gst_entry_size(int size)
    {
        gst_entry_size_ = size;
    }

    void GlobalSymbolTable_Entry::set_lst(LocalSymbolTable *lst)
    {
        lst_ = lst;
    }

    void GlobalSymbolTable_Entry::set_ast(abstract_astnode* ast)
    {
        ast_ = ast;
    }

    abstract_astnode* GlobalSymbolTable_Entry::get_ast()
    {
        return ast_;
    }

    GlobalSymbolTable::GlobalSymbolTable()
    {
        gst_entries_ = std::vector<GlobalSymbolTable_Entry *>();
    }

    GlobalSymbolTable_Entry *GlobalSymbolTable::get_GlobalSymbolTable_entry_by_name(std::string name)
    {
        for(unsigned int i = 0; i < gst_entries_.size(); i++)
        {
            if(gst_entries_[i]->gst_entry_name_ == name)
            {
                return gst_entries_[i];
            }
        }
        return nullptr;
    }

    bool GlobalSymbolTable::add_GlobalSymbolTable_entry(GlobalSymbolTable_Entry *entry)
    {
        GlobalSymbolTable_Entry* gst_entry = this->get_GlobalSymbolTable_entry_by_name(entry->gst_entry_name_);
        if(gst_entry != nullptr && gst_entry->get_gst_entry_type() == entry->get_gst_entry_type()){
            return false;
        }
        gst_entries_.push_back(entry);
        return true;
    }

    std::vector<GlobalSymbolTable_Entry *> GlobalSymbolTable::get_gst_entries()
    {
        return gst_entries_;
    }

    GlobalSymbolTable_Entry * GlobalSymbolTable::get_gst_entry_by_lst(LocalSymbolTable *lst)
    {
        for(unsigned int i =0; i < gst_entries_.size(); i++)
        {
            if(gst_entries_[i]->get_lst() == lst)
            {
                return gst_entries_[i];
            }
        }
        return nullptr;
    }



    void GlobalSymbolTable::print()
    { 
        global = this;

        code_array.push_back("\t.file\t\"a.c\" \n\t.text \n");

        for(unsigned int i = 0; i < gst_entries_.size(); i++)
        {
            if(gst_entries_[i]->gst_entry_type_ == SymbolType::Func)
            {
                current = gst_entries_[i]->get_lst();
                found = false;
                are_there_printf(gst_entries_[i]->get_ast());
                if(found == true) {
                    code_array.push_back("\t.text\n");
                }
                found = false;

                code_array.push_back("\t.globl\t" + gst_entries_[i]->gst_entry_name_ + "\n\t.type\t" + gst_entries_[i]->gst_entry_name_ + ", @function\n");
                code_array.push_back(gst_entries_[i]->gst_entry_name_ + ":\n\tpushl\t%ebp\n\tmovl\t%esp,\t%ebp\n");
                if(gst_entries_[i]->gst_entry_size_ != 0) {
                    code_array.push_back("\tsubl\t$" + std::to_string(gst_entries_[i]->gst_entry_size_) + ",\t%esp\n");
                }
                if(gst_entries_[i]->gst_entry_name_ == "main"){
                    is_it_main = true;
                }
                gen(gst_entries_[i]->get_ast());
                if(gst_entries_[i]->gst_entry_sub_type_ == "void"){
                    if(dynamic_cast<statement_astnode*>(gst_entries_[i]->get_ast())->next.size() > 0){
                        std::string label = ".L" + std::to_string(label_count);
                        label_count++;
                        code_array.push_back(label + std::string(":\n"));
                        backpatch(dynamic_cast<statement_astnode*>(gst_entries_[i]->get_ast())->next, label+ std::string("\n"));
                    }
                    code_array.push_back("\tleave\n\tret\n");
                }
                
                code_array.push_back("\t.size\t" + gst_entries_[i]->gst_entry_name_ + ",\t.-" + gst_entries_[i]->gst_entry_name_ + "\n");

            }
        }

        code_array.push_back("\t.ident\t\"GCC: (Ubuntu 8.1.0-9ubuntu1~16.04.york1) 8.1.0\" \n\t.section\t.note.GNU-stack,\"\",@progbits\n");

        for(auto element : code_array){
            std::cout<<element;
        }
        
    }

    
    Type_Specifier::Type_Specifier()
    {
        type_specifier_type_ = "";
    }

    Type_Specifier::Type_Specifier(std::string type)
    {
        type_specifier_type_ = type;
    }

    int Type_Specifier::get_type_specifier_size(GlobalSymbolTable *gst)
    {
        if (type_specifier_type_ == "int")
        {
            return 4;
        }
        else if (type_specifier_type_ == "float")
        {
            return 4;
        }
        else if (type_specifier_type_ == "void")
        {
            return 0;
        }
        else if (type_specifier_type_.compare(0, 6, "struct") == 0)
        {
            GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name(type_specifier_type_);
            if(gst_entry == nullptr){
                return -1;
            }
            return gst_entry->get_gst_entry_size();
        }
        return -1;
    }

    Fun_Declarator::Fun_Declarator()
    {
        fun_decl_name_ = "";
    }

    Fun_Declarator::Fun_Declarator(std::string name)
    {
        fun_decl_name_ = name;
    }


    Declaration_List::Declaration_List()
    {
        lst_entries_ = std::vector<LocalSymbolTable_Entry *>();
    }

    bool Declaration_List::add_lst_entry(LocalSymbolTable_Entry *lst_entry)
    {
        for(unsigned int i = 0; i <lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->lst_entry_name_ == lst_entry->lst_entry_name_ && lst_entry->lst_entry_type_ == lst_entries_[i]->lst_entry_type_)
            {
                return false;
            }
        }
        lst_entries_.push_back(lst_entry);
        return true;

    }

    std::vector<LocalSymbolTable_Entry *> Declaration_List::get_lst_entries()
    {
        return lst_entries_;
    }


    Declarator::Declarator()
    {
        decl_stars_ = "";
        decl_name_ = "";
        decl_size_ = 1;
    }

    Declarator::Declarator(std::string name)
    {
        decl_stars_ = "";
        decl_name_ = name;
        decl_size_ = 1;
    }

    Declarator::Declarator(std::string name, int size)
    {
        decl_stars_ = "";
        decl_name_ = name;
        this->decl_size_ = size;
    }

    std::string Declarator::get_decl_name()
    {
        return decl_name_;
    }

    std::string Declarator::get_decl_stars()
    {
        return decl_stars_;
    }


    void Declarator::set_decl_stars(std::string stars)
    {
        decl_stars_ = stars;
    }


    Declarator_List::Declarator_List()
    {
        decl_list_declarators_ = std::vector<Declarator>();
    }

    void Declarator_List::add_decl_list_declarator(Declarator declarator)
    {
        decl_list_declarators_.push_back(declarator);
    }

    std::vector<Declarator> Declarator_List::get_decl_list_declarators()
    {
        return decl_list_declarators_;
    }

    Parameter_List::Parameter_List()
    {
        param_list_parameters_ = std::stack<LocalSymbolTable_Entry *>();
    }

    void Parameter_List::add_param_list_parameter(LocalSymbolTable_Entry *paramter)
    {
        param_list_parameters_.push(paramter);
    }

    LocalSymbolTable_Entry* Parameter_List::pop()
    {
        if(param_list_parameters_.empty())
        {
            return nullptr;
        }
        LocalSymbolTable_Entry* temp = param_list_parameters_.top();
        param_list_parameters_.pop();
        return temp;
    }

    Compound_Statement::Compound_Statement() {
        lst_entries_ = std::vector<LocalSymbolTable_Entry *>();
        ast_ = new seq_astnode();
    }

    bool Compound_Statement::add_lst_entry(LocalSymbolTable_Entry * lst_entry) {
        for(unsigned int i = 0; i <lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->lst_entry_name_ == lst_entry->lst_entry_name_ && lst_entry->lst_entry_type_ == lst_entries_[i]->lst_entry_type_)
            {
                return false;
            }
        }
        lst_entries_.push_back(lst_entry);
        return true;
    }

    std::vector<LocalSymbolTable_Entry *> Compound_Statement::get_lst_entries() {
        return lst_entries_;
    }
    void Compound_Statement::set_ast(abstract_astnode* ast) {
        ast_ = ast;
    }
    abstract_astnode* Compound_Statement::get_ast() {
        return ast_;
    }

    Expression_List::Expression_List() {
        expressions_ = std::vector<expression_astnode*>();
    }
    void Expression_List::add_expression(expression_astnode* expression) {
        expressions_.push_back(expression);
    }
    std::vector<expression_astnode*> Expression_List::get_expressions() {
        return expressions_;
    }
}
