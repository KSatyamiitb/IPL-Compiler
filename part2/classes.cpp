#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include "classes.hh"
#include <algorithm>

int space = 0;


namespace IPL
{

    bool compareLST_Entry( LST_Entry* l1, LST_Entry* l2){
        return (l1->get_name() < l2->get_name());
    }

    bool compareGST_Entry( GST_Entry* g1, GST_Entry* g2){
        return (g1->get_name() < g2->get_name());
    }

    bool contain(std::string str, std::string c){
        return (str.find(c) != std::string::npos);
    }

    empty_astnode::empty_astnode() {
        astnode_type = typeExp::empty_astnode;
    }

    void empty_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"empty\"\n";
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

    void seq_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"seq\": [\n";
        for (unsigned int i = 0; i < statements_.size(); i++) {
            if(i == 0)
            {
            space++;
            }
            if(statements_[i]->astnode_type != typeExp::empty_astnode){
                std::cout << std::string(space, '\t') << "{\n";
                statements_[i]->print();
                std::cout << std::string(--space, '\t') << "}";
            }
            else{
                std::cout << std::string(space, '\t') << "\n";
                statements_[i]->print();
                --space;
            }
            if( i != statements_.size() - 1)
            {
                std::cout << ",\n";
            }
            else
            {
                std::cout << "\n";
                space--;
            }
        }
        std::cout << std::string(space, '\t') << "]\n";
    }

    void seq_astnode::add_statement(statement_astnode* statement) {
        statements_.push_back(statement);
    }

    assignS_astnode::assignS_astnode(expression_astnode* expression1, expression_astnode* expression2) {
        astnode_type = typeExp::assignS_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
    }

    void assignS_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"assignS\": {\n";
        std::cout << std::string(++space, '\t') << "\"left\": {\n";
        expression1_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"right\": {\n";
        expression2_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }

    if_astnode::if_astnode(expression_astnode* Condition, statement_astnode* Then, statement_astnode* Else) {
        astnode_type = typeExp::if_astnode;
        condition_ = Condition;
        then_ = Then;
        else_ = Else;
    }

    void if_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"if\": {\n";
        std::cout << std::string(++space, '\t') << "\"cond\": {\n";
        condition_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        if(then_->astnode_type != typeExp::empty_astnode){
            std::cout << std::string(space, '\t') << "\"then\": {\n";
            then_->print();
            std::cout << std::string(--space, '\t') << "},\n";
        }
        else{
            std::cout << std::string(space, '\t') << "\"then\": \n";
            then_->print(); 
            std::cout << std::string(--space, '\t') << ",\n";
        }
        if(else_->astnode_type != typeExp::empty_astnode){
            std::cout << std::string(space, '\t') << "\"else\": {\n";
            else_->print();
            std::cout << std::string(--space, '\t') << "}\n";
        }
        else{
            std::cout << std::string(space, '\t') << "\"else\": \n";
            else_->print();   
            std::cout << std::string(--space, '\t') << "\n";
        }
        std::cout << std::string(--space, '\t') << "}\n";
    }

    while_astnode::while_astnode(expression_astnode* condition, statement_astnode* body) {
        astnode_type = typeExp::while_astnode;
        condition_ = condition;
        body_ = body;
    }

    void while_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"while\": {\n";
        std::cout << std::string(++space, '\t') << "\"cond\": {\n";
        condition_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        if(body_->astnode_type != typeExp::empty_astnode){
            std::cout << std::string(space, '\t') << "\"stmt\": {\n";
            body_->print();
            std::cout << std::string(--space, '\t') << "}\n";
        }
        else{
            std::cout << std::string(space, '\t') << "\"stmt\": \n";
            body_->print(); 
            std::cout << std::string(--space, '\t') << "\n";
        }
        std::cout << std::string(--space, '\t') << "}\n";
    }

    for_astnode::for_astnode(expression_astnode* init, expression_astnode* condition, expression_astnode* update, statement_astnode* body) {
        astnode_type = typeExp::for_astnode;
        init_ = init;
        condition_ = condition;
        update_ = update;
        body_ = body;
    }

    void for_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"for\": {\n";
        std::cout << std::string(++space, '\t') << "\"init\": {\n";
        init_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"guard\": {\n";
        condition_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"step\": {\n";
        update_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        if(body_->astnode_type != typeExp::empty_astnode){
            std::cout << std::string(space, '\t') << "\"body\": {\n";
            body_->print();
            std::cout << std::string(--space, '\t') << "}\n";
        }
        else{
            std::cout << std::string(space, '\t') << "\"body\": \n";
            body_->print(); 
            std::cout << std::string(--space, '\t') << "\n";
        }
        std::cout << std::string(--space, '\t') << "}\n";
    }

    return_astnode::return_astnode(expression_astnode* expression) {
        astnode_type = typeExp::return_astnode;
        expression_ = expression;
    }

    void return_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"return\": {\n";
        expression_->print();       
        std::cout << std::string(--space, '\t') << "}\n";
    }

    proccall_astnode::proccall_astnode(std::string name) {
        astnode_type = typeExp::proccall_astnode;
        name_ = name;
        arguments_ = std::vector<expression_astnode*>();
    }

    void proccall_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"proccall\": {\n";
        std::cout << std::string(++space, '\t') << "\"fname\": {\n";
        std::cout << std::string(++space, '\t') << "\"identifier\": \"" << name_ <<"\"\n";
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"params\": [\n";
        for (unsigned int i = 0; i < arguments_.size(); i++) {
            if(i == 0)
            {
            space++;
            }
            std::cout << std::string(space, '\t') << "{\n";
            arguments_[i]->print();
            std::cout << std::string(--space, '\t') << "}";
            if( i != arguments_.size() - 1)
            {
                std::cout << ",\n";
            }
            else
            {
                std::cout << "\n";
                space--;
            }
        }
        std::cout << std::string(space, '\t') << "]\n";
        std::cout << std::string(--space, '\t') << "}\n";
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

    void op_binary_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"op_binary\": {\n";
        std::cout << std::string(++space, '\t') << "\"op\": \"" << op_binary_map.at(op_) << "\",\n";
        std::cout << std::string(space, '\t') << "\"left\": {\n";
        expression1_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"right\": {\n";
        expression2_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }
    
    op_unary_astnode::op_unary_astnode(expression_astnode* expression, enum op_unary_type op) {
        astnode_type = typeExp::op_unary_astnode;
        expression_ = expression;
        op_ = op;
    }

    void op_unary_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"op_unary\": {\n";
        std::cout << std::string(++space, '\t') << "\"op\": \"" << op_unary_map.at(op_) << "\",\n";
        std::cout << std::string(space, '\t') << "\"child\": {\n";
        expression_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }

    int_astnode::int_astnode(int value) {
        astnode_type = typeExp::int_astnode;
        value_ = value;
        exp_type_ = "int";
    }

    void int_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"intconst\": " << value_ << "\n";
    }

    float_astnode::float_astnode(float value) {
        astnode_type = typeExp::float_astnode;
        value_ = value;
        exp_type_ = "float";
    }

    void float_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"floatconst\": " << value_ << "\n";
    }

    string_astnode::string_astnode(std::string value) {
        astnode_type = typeExp::string_astnode;
        value_ = value;
        exp_type_ = "string";
    }

    void string_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"stringconst\": " << value_ << "\n";
    }

    assignE_astnode::assignE_astnode(expression_astnode* expression1, expression_astnode* expression2) {
        astnode_type = typeExp::assignE_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
    }

    void assignE_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"assignE\": {\n";
        std::cout << std::string(++space, '\t') << "\"left\": {\n";
        expression1_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"right\": {\n";
        expression2_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }

    expression_astnode* assignE_astnode::get_expression1() {
        return expression1_;
    }

    expression_astnode* assignE_astnode::get_expression2() {
        return expression2_;
    }

    funcall_astnode::funcall_astnode(std::string name) {
        astnode_type = typeExp::funcall_astnode;
        name_ = name;
        arguments_ = std::vector<expression_astnode*>();
    }

    void funcall_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"funcall\": {\n";
        std::cout << std::string(++space, '\t') << "\"fname\": {\n";
        std::cout << std::string(++space, '\t') << "\"identifier\": \"" << name_ <<"\"\n";
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"params\": [\n";
        for (unsigned int i = 0; i < arguments_.size(); i++) {
            if(i == 0)
            {
            space++;
            }
            std::cout << std::string(space, '\t') << "{\n";
            arguments_[i]->print();
            std::cout << std::string(--space, '\t') << "}";
            if( i != arguments_.size() - 1)
            {
                std::cout << ",\n";
            }
            else
            {
                std::cout << "\n";
                space--;
            }
        }
        std::cout << std::string(space, '\t') << "]\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }

    void funcall_astnode::add_argument(expression_astnode* argument) {
        arguments_.push_back(argument);
    }


    identifier_astnode::identifier_astnode(std::string name) {
        astnode_type = typeExp::identifier_astnode;
        name_ = name;
    }

    void identifier_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"identifier\": \"" << name_ << "\"\n";
    }

    member_astnode::member_astnode(expression_astnode* expression, identifier_astnode* name) {
        astnode_type = typeExp::member_astnode;
        expression_ = expression;
        name_ = name;
    }

    void member_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"member\": {\n";
        std::cout << std::string(++space, '\t') << "\"struct\": {\n";
        expression_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"field\": {\n";
        name_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }

    arrayref_astnode::arrayref_astnode(expression_astnode* expression1, expression_astnode* expression2) {
        astnode_type = typeExp::arrayref_astnode;
        expression1_ = expression1;
        expression2_ = expression2;
    }

    void arrayref_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"arrayref\": {\n";
        std::cout << std::string(++space, '\t') << "\"array\": {\n";
        expression1_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"index\": {\n";
        expression2_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }

    arrow_astnode::arrow_astnode(expression_astnode* expression, identifier_astnode* name) {
        astnode_type = typeExp::arrow_astnode;
        expression_ = expression;
        name_ = name;
    }

    void arrow_astnode::print() {
        std::cout << std::string(++space, '\t') << "\"arrow\": {\n";
        std::cout << std::string(++space, '\t') << "\"pointer\": {\n";
        expression_->print();
        std::cout << std::string(--space, '\t') << "},\n";
        std::cout << std::string(space, '\t') << "\"field\": {\n";
        name_->print();
        std::cout << std::string(--space, '\t') << "}\n";
        std::cout << std::string(--space, '\t') << "}\n";
    }


    LST_Entry::LST_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset)
    {
        name_ = name;
        type_ = type;
        scope_ = scope;
        sub_type_ = sub_type;
        size_ = size;
        offset_ = offset;
    }

    std::string LST_Entry::get_name()
    {
        return name_;
    }

    SymbolType LST_Entry::get_type()
    {
        return type_;
    }

    Scope LST_Entry::get_scope()
    {
        return scope_;
    }

    std::string LST_Entry::get_type_name()
    {
        switch(type_)
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

    std::string LST_Entry::get_scope_name()
    {
        switch(scope_)
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

    std::string LST_Entry::get_sub_type()
    {
        return sub_type_;
    }

    int LST_Entry::get_size()
    {
        return size_;
    }

    int LST_Entry::get_offset()
    {
        return offset_;
    }

    void LST_Entry::set_name(std::string name)
    {
        name_ = name;
    }

    void LST_Entry::set_type(SymbolType type)
    {
        type_= type;
    }

    void LST_Entry::set_scope(Scope scope)
    {
        scope_ = scope;
    }

    void LST_Entry::set_sub_type(std::string sub_type)
    {
        sub_type_= sub_type;
    }

    void LST_Entry::set_size(int size)
    {
        size_ = size;
    }

    void LST_Entry::set_offset(int offset)
    {
        offset_ = offset;
    }

    std::string LST_Entry::get_dereferenced_type(){
        //if star, then remove one star
        if (!contain(sub_type_ , "[")){
            // Case 1 not associated to array 
            if(!contain(sub_type_ , "*")){
                return ""; //ERROR CASE
            }
            else{
                return sub_type_.substr(0,sub_type_.find("*")) + sub_type_.substr(sub_type_.find("*") + 1);
            }
        }
        else{
            if(!contain(sub_type_ , "(")){
                // Case 2a an array itself
                return sub_type_.substr(0,sub_type_.find("[")) + sub_type_.substr(sub_type_.find("]") + 1);
            }
            else{
                // Case 2b associated with pointer to array
                if (sub_type_.find(')')-sub_type_.find('(') == 2){
                    // Case 2bi a single pointer to an array 
                    return sub_type_.substr(0,sub_type_.find("(")) + sub_type_.substr(sub_type_.find(")")+1);
                }
                else{
                    // Case 2bii multi pointer to an array 
                    return sub_type_.substr(0,sub_type_.find("(")+1) + sub_type_.substr(sub_type_.find("(")+2);    
                }
            }
        }
    }

    LST::LST()
    {
        lst_entries_ = std::vector<LST_Entry *>();
    }

    LST_Entry *LST::get_lst_entry_by_name(std::string name)
    {
        for(unsigned int i = 0; i <lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->get_name() == name)
            {
                return lst_entries_[i];
            }
        }
        return nullptr;
    }

    bool LST::add_lst_entry(LST_Entry *entry)
    {
        LST_Entry* lst_entry = this->get_lst_entry_by_name(entry->get_name());
        if(lst_entry != nullptr && lst_entry->get_type() == entry->get_type()){
            return false;
        }
        lst_entries_.push_back(entry);
        return true;
    }

    int LST::get_size()
    {
        int total_size = 0;
        for(unsigned int i = 0; i < lst_entries_.size(); i++)
        {
            total_size += lst_entries_[i]->get_size();
        }
        return total_size;
    }

    std::vector<LST_Entry *> LST::get_lst_entries()
    {
        return lst_entries_;
    }

    void LST::print()
    {

        sort(lst_entries_.begin(), lst_entries_.end(), compareLST_Entry);

        std::cout << "[\n";
        for(unsigned int i = 0; i < lst_entries_.size(); i++)
        {
            if( i == 0)
            {
                space++;
            }
            std::cout << std::string(space,'\t') << "[\n";
            std::cout << std::string(++space,'\t') << "\"" << lst_entries_[i]->get_name() << "\",\n";
            std::cout << std::string(space,'\t') << "\"" << lst_entries_[i]->get_type_name() << "\",\n";
            std::cout << std::string(space,'\t') << "\"" << lst_entries_[i]->get_scope_name() << "\",\n";
            std::cout << std::string(space,'\t') << lst_entries_[i]->get_size() << ",\n";
            std::cout << std::string(space,'\t') << lst_entries_[i]->get_offset() << ",\n";
            std::cout << std::string(space,'\t') << "\"" << lst_entries_[i]->get_sub_type() << "\"\n";
            std::cout << std::string(--space,'\t') << "]";
            if( i != lst_entries_.size() - 1)
            {
                std::cout << ",\n";
            }
            else
            {
                std::cout << "\n";
                space--;
            }
        }
        std::cout << std::string(space,'\t') << "]";
    }
    
    GST_Entry::GST_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset)
    {
        name_ = name;
        type_ = type;
        scope_ = scope;
        sub_type_ = sub_type;
        size_ = size;
        offset_ = offset;
        lst_ = NULL;
        ast_ = NULL;
    }

    GST_Entry::GST_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset, LST *lst)
    {
        name_ = name;
        type_ = type;
        scope_ = scope;
        sub_type_= sub_type;
        size_ = size;
        offset_ = offset;
        lst_ = lst;
        ast_ = NULL;
    }

    std::string GST_Entry::get_name()
    {
        return name_;
    }

    SymbolType GST_Entry::get_type()
    {
        return type_;
    }

    Scope GST_Entry::get_scope()
    {
        return scope_;
    }

    std::string GST_Entry::get_type_name()
    {
        switch(type_)
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

    std::string GST_Entry::get_scope_name()
    {
        switch(scope_)
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

    std::string GST_Entry::get_sub_type()
    {
        return sub_type_;
    }

    int GST_Entry::get_size()
    {
        return size_;
    }

    int GST_Entry::get_offset()
    {
        return offset_;
    }

    LST *GST_Entry::get_lst()
    {
        return lst_;
    }

    void GST_Entry::set_name(std::string name)
    {
        name_ = name;
    }

    void GST_Entry::set_type(SymbolType type)
    {
        type_= type;
    }

    void GST_Entry::set_scope(Scope scope)
    {
        scope_ = scope;
    }

    void GST_Entry::set_sub_type(std::string sub_type)
    {
        sub_type_= sub_type;
    }

    void GST_Entry::set_size(int size)
    {
        size_ = size;
    }

    void GST_Entry::set_offset(int offset)
    {
        offset_ = offset;
    }

    void GST_Entry::set_lst(LST *lst)
    {
        lst_ = lst;
    }

    void GST_Entry::set_ast(abstract_astnode* ast)
    {
        ast_ = ast;
    }

    abstract_astnode* GST_Entry::get_ast()
    {
        return ast_;
    }

    GST::GST()
    {
        gst_entries_ = std::vector<GST_Entry *>();
    }

    GST_Entry *GST::get_gst_entry_by_name(std::string name)
    {
        for(unsigned int i = 0; i < gst_entries_.size(); i++)
        {
            if(gst_entries_[i]->get_name() == name)
            {
                return gst_entries_[i];
            }
        }
        return nullptr;
    }

    bool GST::add_GST_entry(GST_Entry *entry)
    {
        GST_Entry* gst_entry = this->get_gst_entry_by_name(entry->get_name());
        if(gst_entry != nullptr && gst_entry->get_type() == entry->get_type()){
            return false;
        }
        gst_entries_.push_back(entry);
        return true;
    }

    std::vector<GST_Entry *> GST::get_gst_entries()
    {
        return gst_entries_;
    }

    GST_Entry * GST::get_gst_entry_by_lst(LST *lst)
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

    void GST::print()
    {

        sort(gst_entries_.begin(), gst_entries_.end(), compareGST_Entry);

        //first symbols
        std::cout<< "{\n" << std::string(++space,'\t') << "\"globalST\": [\n";
        for(unsigned int i = 0; i < gst_entries_.size(); i++)
        {
            if( i == 0)
            {
                space++;
            }
            std::cout << std::string(space,'\t') << "[\n";
            std::cout << std::string(++space,'\t') << "\"" << gst_entries_[i]->get_name() << "\",\n";
            std::cout << std::string(space,'\t') << "\"" << gst_entries_[i]->get_type_name() << "\",\n";
            std::cout << std::string(space,'\t') << "\"" << gst_entries_[i]->get_scope_name() << "\",\n";
            std::cout << std::string(space,'\t') << gst_entries_[i]->get_size() << ",\n";
            if(gst_entries_[i]->get_type() == SymbolType::Struct)
            {
                std::cout << std::string(space,'\t') << "\"-\"" << ",\n";
            }
            else
            {
                std::cout << std::string(space,'\t') << gst_entries_[i]->get_offset() << ",\n";
            }
            
            std::cout << std::string(space,'\t') << "\"" << gst_entries_[i]->get_sub_type() << "\"\n";
            std::cout << std::string(--space,'\t') << "]";
            if( i != (gst_entries_.size() - 1))
            {
                std::cout << ",\n";
            }
            else
            {
                std::cout << "\n";
                space--;
            }
        }
        std::cout << std::string(space,'\t') << "],\n";

        //then structs
        
        std::cout << std::string(space,'\t') << "\"structs\": [\n";
        bool is_any_struct = false;
        for(unsigned int i = 0; i < gst_entries_.size(); i++)
        {
            if(gst_entries_[i]->get_type() == SymbolType::Struct)
            {
                if( is_any_struct == true)
                {
                    std::cout << ",\n"; 
                }
                if( is_any_struct == false)
                {
                    space++;
                    is_any_struct = true;   
                }
                std::cout << std::string(space,'\t') << "{\n";
                std::cout << std::string(++space,'\t') << "\"name\": \"" << gst_entries_[i]->get_name() << "\",\n";       
                std::cout << std::string(space,'\t') << "\"localST\": ";         
                gst_entries_[i]->get_lst()->print();
                std::cout << "\n";
                std::cout << std::string(--space,'\t') << "}";
            }
        }
        if( is_any_struct == true)
        {
            std::cout << "\n";
            space--;
        }
        std::cout << std::string(space,'\t') << "],\n";

        //then functions
        
        std::cout << std::string(space,'\t') << "\"functions\": [\n";
        bool is_any_fun = false;
        for(unsigned int i = 0; i < gst_entries_.size(); i++)
        {
            if(gst_entries_[i]->get_type() == SymbolType::Func)
            {
                if( is_any_fun == true)
                {
                    std::cout << ",\n"; 
                }
                if( is_any_fun == false)
                {
                    space++;
                    is_any_fun = true;
                }
                std::cout << std::string(space,'\t') << "{\n";
                std::cout << std::string(++space,'\t') << "\"name\": \"" << gst_entries_[i]->get_name() << "\",\n";       
                std::cout << std::string(space,'\t') << "\"localST\": ";         
                gst_entries_[i]->get_lst()->print();
                std::cout << ",\n";
                std::cout << std::string(space,'\t') << "\"ast\": {\n";
                gst_entries_[i]->get_ast()->print();
                std::cout << std::string(--space,'\t')  << "}\n";
                std::cout << std::string(--space,'\t') << "}";
            }
        }
        if( is_any_fun == true)
        {
            std::cout << "\n";
            space--;
        }
        std::cout << std::string(space,'\t') << "]\n";
        std::cout << std::string(--space,'\t') << "}";       
    }

    
    Type_Specifier::Type_Specifier()
    {
        type_ = "";
    }

    Type_Specifier::Type_Specifier(std::string type)
    {
        type_ = type;
    }

    std::string Type_Specifier::get_type()
    {
        return type_;
    }

    void Type_Specifier::set_type(std::string type)
    {
        type_ = type;
    }

    int Type_Specifier::get_size(GST *gst)
    {
        if (type_ == "int")
        {
            return 4;
        }
        else if (type_ == "float")
        {
            return 4;
        }
        else if (type_ == "void")
        {
            return 0;
        }
        else if (type_.compare(0, 6, "struct") == 0)
        {
            GST_Entry* gst_entry = gst->get_gst_entry_by_name(type_);
            if(gst_entry == nullptr){
                return -1;
            }
            return gst_entry->get_size();
        }
        return -1;
    }

    Fun_Declarator::Fun_Declarator()
    {
        name_ = "";
    }

    Fun_Declarator::Fun_Declarator(std::string name)
    {
        name_ = name;
    }

    std::string Fun_Declarator::get_name()
    {
        return name_;
    }

    void Fun_Declarator::set_name(std::string name)
    {
        name_ = name;
    }

    Declaration_List::Declaration_List()
    {
        lst_entries_ = std::vector<LST_Entry *>();
    }

    bool Declaration_List::add_lst_entry(LST_Entry *lst_entry)
    {
        for(unsigned int i = 0; i <lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->get_name() == lst_entry->get_name() && lst_entry->get_type() == lst_entries_[i]->get_type())
            {
                return false;
            }
        }
        lst_entries_.push_back(lst_entry);
        return true;

    }

    std::vector<LST_Entry *> Declaration_List::get_lst_entries()
    {
        return lst_entries_;
    }


    Declarator::Declarator()
    {
        stars_ = "";
        name_ = "";
        size_ = 1;
    }

    Declarator::Declarator(std::string name)
    {
        stars_ = "";
        name_ = name;
        size_ = 1;
    }

    Declarator::Declarator(std::string name, int size)
    {
        stars_ = "";
        name_ = name;
        this->size_ = size;
    }

    std::string Declarator::get_name()
    {
        return name_;
    }

    std::string Declarator::get_stars()
    {
        return stars_;
    }

    int Declarator::get_size()
    {
        return size_;
    }

    void Declarator::set_name(std::string name)
    {
        name_ = name;
    }

    void Declarator::set_stars(std::string stars)
    {
        stars_ = stars;
    }

    void Declarator::set_size(int size)
    {
        this->size_ = size;
    }

    Declarator_List::Declarator_List()
    {
        declarators_ = std::vector<Declarator>();
    }

    void Declarator_List::add_declarator(Declarator declarator)
    {
        declarators_.push_back(declarator);
    }

    std::vector<Declarator> Declarator_List::get_declarators()
    {
        return declarators_;
    }

    Parameter_List::Parameter_List()
    {
        parameters_ = std::stack<LST_Entry *>();
    }

    void Parameter_List::add_parameter(LST_Entry *paramter)
    {
        parameters_.push(paramter);
    }

    LST_Entry* Parameter_List::pop()
    {
        if(parameters_.empty())
        {
            return nullptr;
        }
        LST_Entry* temp = parameters_.top();
        parameters_.pop();
        return temp;
    }

    Compound_Statement::Compound_Statement() {
        lst_entries_ = std::vector<LST_Entry *>();
        ast_ = new seq_astnode();
    }

    bool Compound_Statement::add_lst_entry(LST_Entry * lst_entry) {
        for(unsigned int i = 0; i <lst_entries_.size(); i++)
        {
            if(lst_entries_[i]->get_name() == lst_entry->get_name() && lst_entry->get_type() == lst_entries_[i]->get_type())
            {
                return false;
            }
        }
        lst_entries_.push_back(lst_entry);
        return true;
    }

    std::vector<LST_Entry *> Compound_Statement::get_lst_entries() {
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
