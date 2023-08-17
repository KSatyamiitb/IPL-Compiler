%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%locations

%define parse.trace

%code requires{
   namespace IPL {
        class Scanner;
   }

   #include "classes.hh"
   #include "location.hh"

  // # ifndef YY_NULLPTR
  // #  if defined __cplusplus && 201103L <= __cplusplus
  // #   define YY_NULLPTR nullptr
  // #  else
  // #   define YY_NULLPTR 0
  // #  endif
  // # endif

}

%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } STRING_LITERAL
%printer { std::cerr << $$; } OTHERS
%printer { std::cerr << $$; } STRUCT
%printer { std::cerr << $$; } VOID
%printer { std::cerr << $$; } INT
%printer { std::cerr << $$; } FLOAT
%printer { std::cerr << $$; } WHILE
%printer { std::cerr << $$; } FOR
%printer { std::cerr << $$; } IF
%printer { std::cerr << $$; } ELSE
%printer { std::cerr << $$; } GE_OP
%printer { std::cerr << $$; } LE_OP
%printer { std::cerr << $$; } EQ_OP
%printer { std::cerr << $$; } NE_OP
%printer { std::cerr << $$; } INC_OP
%printer { std::cerr << $$; } OR_OP
%printer { std::cerr << $$; } AND_OP
%printer { std::cerr << $$; } PTR_OP


%parse-param { Scanner  &scanner  }
%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include <string>
   #include <vector>
   
   
   #include "scanner.hh"
   #include "classes.hh"

#undef yylex
#define yylex IPL::Parser::scanner.yylex
using namespace IPL;
GST* gst = new GST();
LST* current_lst = NULL;
int current_offset = 0;

std::map<std::string , std::string> predefined {
    {"printf", "void"},
    {"scanf", "void"}
};

bool contain(std::string str, std::string c){
    return (str.find(c) != std::string::npos);
}
std::string get_referenced_type_from_string(std::string exp_type_){
    // if &, then add one star
    if (!contain(exp_type_ , "[")){
        // Case 1 not an array 
        return (exp_type_ + "*");
    }
    else{
        // Case 2 an array 
        if (!contain(exp_type_ , "(")){
            // Case 2a not a pointer to an array 
            return exp_type_.substr(0,exp_type_.find("[")) + "(*)" + exp_type_.substr(exp_type_.find("["));
        }
        else{
            // Case 2b already a pointer to an array 
            return exp_type_.substr(0,exp_type_.find("(")+1) + "*" + exp_type_.substr(exp_type_.find("(")+1);    
        }
    }
}

}


%define api.value.type variant
%define parse.assert

%start complete_code



%token '\n' '\t'
%token <std::string> IDENTIFIER
%token <std::string> INT_CONSTANT
%token <std::string> FLOAT_CONSTANT
%token <std::string> STRING_LITERAL
%token <std::string> OTHERS
%token <std::string> STRUCT
%token <std::string> VOID
%token <std::string> INT
%token <std::string> FLOAT
%token <std::string> WHILE
%token <std::string> FOR
%token <std::string> RETURN
%token <std::string> IF
%token <std::string> ELSE
%token <std::string> GE_OP
%token <std::string> LE_OP
%token <std::string> EQ_OP
%token <std::string> NE_OP
%token <std::string> INC_OP
%token <std::string> OR_OP
%token <std::string> AND_OP
%token <std::string> PTR_OP
%token  '*' '/' '+' '-' '=' '>' '<' '[' ']' '{' '}' '(' ')' '&' '.' ',' ':' ';' '!'

%nterm <int> complete_code translation_unit struct_specifier function_definition 
%nterm <Type_Specifier*> type_specifier 
%nterm <Fun_Declarator*> fun_declarator 
%nterm <Declaration_List*> parameter_declaration declaration_list declaration  
%nterm <Parameter_List*> parameter_list
%nterm <Declarator*> declarator_arr declarator 
%nterm <Declarator_List*> declarator_list
%nterm <Compound_Statement*> compound_statement
%nterm <seq_astnode*> statement_list
%nterm <statement_astnode*> statement
%nterm <proccall_astnode*> procedure_call
%nterm <statement_astnode*> selection_statement iteration_statement
%nterm <assignE_astnode*> assignment_expression
%nterm <assignS_astnode*> assignment_statement
%nterm <expression_astnode*> expression logical_and_expression equality_expression
%nterm <expression_astnode*> relational_expression additive_expression unary_expression multiplicative_expression postfix_expression primary_expression
%nterm <Expression_List*> expression_list
%nterm <op_unary_type> unary_operator


%%
complete_code: translation_unit {gst->print();}

translation_unit: 
struct_specifier 
| function_definition
| translation_unit struct_specifier
| translation_unit function_definition

struct_specifier: 
STRUCT IDENTIFIER {
	LST* lst = new LST();
	bool success = gst->add_GST_entry(new GST_Entry("struct " + $2,SymbolType::Struct,Scope::Global,"-",0,0,lst));
	if(!success) {
        error(@1, "The function \"" + $2 + "\" has a previous defination");
    }
    current_lst = lst;
	current_offset = 0;
} '{' declaration_list '}' ';'{
    GST_Entry* gst_entry_ = gst->get_gst_entry_by_name("struct " + $2);
	gst_entry_->set_size(gst_entry_->get_lst()->get_size());
}

function_definition: 
type_specifier fun_declarator {
    int type_specifier_size = $1->get_size(gst);
    if(type_specifier_size == -1){
        error(@1, $1->get_type() + " not declared");
    }
	std::string name = $2->get_name();
	GST_Entry* entry = gst->get_gst_entry_by_name(name);
	std::string type = $1->get_type();
	entry->set_sub_type(type);
	current_lst = entry->get_lst();
    current_offset = 0;
} compound_statement {
	GST_Entry* gst_entry_ = gst->get_gst_entry_by_name ($2->get_name()); 
    gst_entry_->set_ast( $4->get_ast() );
}

type_specifier: 
VOID {$$ = new Type_Specifier("void");}
| INT {$$ = new Type_Specifier("int");}
| FLOAT {$$ = new Type_Specifier("float");}
| STRUCT IDENTIFIER {$$ = new Type_Specifier("struct "+$2);}

fun_declarator: 
IDENTIFIER '(' {
	LST* lst = new LST();
	bool success = gst->add_GST_entry(new GST_Entry($1,SymbolType::Func,Scope::Global,"-",0,0,lst));
	if(!success) {
       error(@2, "The function \"" + $1 + "\" has a previous defination");
    }
    current_lst = lst;
	current_offset = 12;
} parameter_list {
	Parameter_List* parameter_list_ = $4;
	LST_Entry* lst_entry_ = parameter_list_->pop(); 
	while(lst_entry_ != NULL) {
		lst_entry_->set_offset (current_offset); 
		current_offset += lst_entry_->get_size(); 
		lst_entry_ = parameter_list_->pop();
	}
} ')' { 
    $$ = new Fun_Declarator($1);
}
| IDENTIFIER '(' ')' { 
	LST* lst = new LST();
	bool success = gst->add_GST_entry(new GST_Entry($1,SymbolType::Func,Scope::Global,"-",0,0,lst));
    if(!success) {
       error(@1, "The function \"" + $1 + "\" has a previous defination");
    } 
    current_lst = lst;
    current_offset = 0;
    $$ = new Fun_Declarator($1); 
}

parameter_list: 
parameter_declaration {
    std::vector<LST_Entry*> lst_entries = $1->get_lst_entries();
	$$ = new Parameter_List();
    for (unsigned int i = 0; i < lst_entries.size(); i++) {
        $$->add_parameter(lst_entries[i]);
    }
}
| parameter_list ',' parameter_declaration
{
	std::vector<LST_Entry*> lst_entries = $3->get_lst_entries();
	$$ = $1;
    for (unsigned int i = 0; i < lst_entries.size(); i++) {
        $$->add_parameter(lst_entries[i]);
    }
}

parameter_declaration: 
type_specifier declarator {
    $$ = new Declaration_List();
	int type_specifier_size = $1->get_size(gst);
    if(type_specifier_size == -1){
        error(@1, $1->get_type() + " not declared");
    }
    std::vector<Declarator> declarators = {*$2};
    for (unsigned int i = 0; i < declarators.size(); i++) {
        if($1->get_type() == "void" && declarators[i].get_stars()[0] != '*')
        {
             error(@2, "Cannot declare variable of type \"void\"");
        }
        int size = type_specifier_size;
        if(declarators[i].get_stars()[0] == '*')
        {
            size = 4;
            
        }
        LST_Entry* temp = new LST_Entry(declarators[i].get_name(), SymbolType::Var, Scope::Param, $1->get_type()+declarators[i].get_stars(), declarators[i].get_size()*(size), 0);
        bool success1  = current_lst->add_lst_entry(temp);
        bool success2 = $$->add_lst_entry(temp);
        if(!success1 || !success2) {
             error(@1, "\"" + declarators[i].get_name() + "\" has a previous declaration");
        }
    }
    
}

declaration_list:
declaration {
    $$ = $1;
}
| declaration_list declaration {
    $$ = $1;
    std::vector<LST_Entry *> lst_entries_ = $2->get_lst_entries();
    for ( unsigned int i = 0; i < lst_entries_.size(); i++)
    {
        $$->add_lst_entry(lst_entries_[i]);
    }
}

declaration: 
type_specifier declarator_list ';' {
    $$ = new Declaration_List();
	int type_specifier_size = $1->get_size(gst);
    if(type_specifier_size == -1){
        error(@1, $1->get_type() + " not declared");
    }
    std::vector<Declarator> declarators = $2->get_declarators();
    for (unsigned int i = 0; i < declarators.size(); i++) {
        if($1->get_type() == "void" && declarators[i].get_stars()[0] != '*')
        {
              error(@2, "Cannot declare variable of type \"void\"");
        }
        int size = type_specifier_size;
        if(declarators[i].get_stars()[0] == '*')
        {
            size = 4;
            
        }
        LST_Entry* temp = new LST_Entry(declarators[i].get_name(), SymbolType::Var, Scope::Local, $1->get_type()+declarators[i].get_stars(), declarators[i].get_size()*(size), current_offset);
        bool success1  = current_lst->add_lst_entry(temp);
        bool success2 = $$->add_lst_entry(temp);
        if(!success1 || !success2) {
             error(@1, "\"" + declarators[i].get_name() + "\" has a previous declaration");
        }
		current_offset += declarators[i].get_size()*(size);
    }

}

declarator_list: 
declarator {
    $$ = new Declarator_List();
    $$->add_declarator(*$1);
}
| declarator_list ',' declarator {
    $$ = $1;
    $$->add_declarator(*$3);
}

declarator_arr: 
IDENTIFIER {
    $$ = new Declarator($1,1);
}
| declarator_arr '[' INT_CONSTANT ']' {
    $$ = $1;
    $$->set_size(stoi($3)*$$->get_size());
    $$->set_stars($$->get_stars()+"["+$3+"]");
}

declarator: 
declarator_arr {
    $$ = $1;
}
| '*' declarator {
    $$ = $2;
    $$->set_stars("*"+$$->get_stars());
}

compound_statement: 
'{' '}' { $$ = new Compound_Statement(); }
| '{' statement_list '}' { 
    $$ = new Compound_Statement();
    $$->set_ast($2);
}
| '{' declaration_list '}' {
    current_offset = 0;  
    $$ = new Compound_Statement();
    std::vector<LST_Entry *> lst_entries_ = $2->get_lst_entries();
    for ( unsigned int i = 0; i < lst_entries_.size(); i++)
    {
        //std::cout << lst_entries_[i]->get_name();
        current_offset -= lst_entries_[i]->get_size();
        lst_entries_[i]->set_offset(current_offset);
        $$->add_lst_entry(lst_entries_[i]);
    }

}
| '{' declaration_list statement_list '}' {
    current_offset = 0;
    $$ = new Compound_Statement();
    $$->set_ast($3);
    std::vector<LST_Entry *> lst_entries_ = $2->get_lst_entries();
    for ( unsigned int i = 0; i < lst_entries_.size(); i++)
    {
        //std::cout << lst_entries_[i]->get_name();
        current_offset -= lst_entries_[i]->get_size();
        lst_entries_[i]->set_offset(current_offset);
        $$->add_lst_entry(lst_entries_[i]);
    }

}

statement_list: 
statement {
    $$ = new seq_astnode();
    $$->add_statement($1);

}
| statement_list statement{
    $$ = $1;
    $$->add_statement($2);
}

statement: 
';' { $$ = new empty_astnode(); }
| '{' statement_list '}' { $$ = $2; }
| selection_statement { $$ = $1; }
| iteration_statement { $$ = $1; }
| assignment_statement { $$ = $1; }
| procedure_call { $$ = $1; }
| RETURN expression ';' { 
   GST_Entry * gst_entry = gst->get_gst_entry_by_lst(current_lst);
   if(gst_entry==nullptr){
       error(@2, "Cannot return from a non-function");
   }
   else if(gst_entry->get_sub_type() == "int" && $2->exp_type_ == "float"){
      op_unary_astnode* temp = new op_unary_astnode($2,op_unary_type::TO_INT);
      $$ = new return_astnode(temp); 
   }
   else if(gst_entry->get_sub_type() == "float" && $2->exp_type_== "int"){
      op_unary_astnode* temp = new op_unary_astnode($2,op_unary_type::TO_FLOAT);
      $$ = new return_astnode(temp); 
   }
   else if(gst_entry->get_sub_type() != $2->exp_type_){
        error(@2, "Cannot return " + $2->exp_type_ + " from a " + gst_entry->get_sub_type() + " function");
   }
   else{
      $$ = new return_astnode($2); 
   }
}

assignment_expression: 
unary_expression '=' expression { 
    if($1->is_lvalue){
        if($1->exp_type_ == "int" && $3->exp_type_ == "float"){
            $$ = new assignE_astnode($1,new op_unary_astnode($3,op_unary_type::TO_INT)); 
        }
        else if($1->exp_type_ == "float" && $3->exp_type_ == "int"){
            $$ = new assignE_astnode($1,new op_unary_astnode($3,op_unary_type::TO_FLOAT)); 
        }
        else{
            if($1->get_dereferenced_type() != "" && !contain($1->exp_type_ , "[")){
                if($3->exp_type_.substr(0,5) == "void*" && !contain($3->exp_type_.substr(5) , "*")){
                    $$ = new assignE_astnode($1,$3); 
                }
                else if($3->astnode_type == typeExp::int_astnode){
                    expression_astnode* temp = $3;
                    int val = temp->cvalue;
                    if(val==0){
                        $$ = new assignE_astnode($1,$3);
                    }
                    else{
                         error(@2, "Cannot assign " + $3->exp_type_ + " to " + $1->exp_type_);
                    }
                }
                else if($1->get_dereferenced_type() == "void"){
                    if($3->get_dereferenced_type() != ""){
                        $$ = new assignE_astnode($1,$3);
                    }
                    else{
                         error(@2, "Cannot assign " + $3->exp_type_ + " to " + $1->exp_type_);
                    }
                }
                else if($1->get_dereferenced_type() == "void*"){
                    if($3->get_dereferenced_type() == "void" || $3->get_dereferenced_type() == "void*"){
                        $$ = new assignE_astnode($1,$3);
                    }
                    else{
                         error(@2, "Cannot assign " + $3->exp_type_ + " to " + $1->exp_type_);
                    }
                }
                else{
                    if($1->get_dereferenced_type() == $3->get_dereferenced_type()){
                        $$ = new assignE_astnode($1,$3); 
                    }
                    else{
                         error(@2, "Cannot assign " + $3->exp_type_ + " to " + $1->exp_type_);
                    }
                }
            }
            else if($1->exp_type_ != $3->exp_type_  || contain($1->exp_type_ , "[")){
                 error(@2, "Cannot assign " + $3->exp_type_ + " to " + $1->exp_type_);
            }
            else {
                $$ = new assignE_astnode($1,$3); 
            }
        }
    }
    else{
         error(@2, "Cannot assign to a non-lvalue");
    }
}

assignment_statement: 
assignment_expression ';' { $$ = new assignS_astnode($1->get_expression1(), $1->get_expression2()); }

procedure_call: 
IDENTIFIER '(' ')' ';' { 
    if(predefined.find($1) != predefined.end()){
      $$ = new proccall_astnode($1);
    }
    else{
        GST_Entry* gst_entry = gst->get_gst_entry_by_name($1);
        if(gst_entry == nullptr){
             error(@1, "Identifier: " + $1  + " not declared");
        }
        LST* lst = gst_entry->get_lst();
        LST* params = new LST();
        for(auto entry : lst->get_lst_entries()){
            if(entry->get_scope() == Scope::Param){
                bool success = params->add_lst_entry(entry);
                if(!success){
                     error(@1, "\"" + entry->get_name() + "\" has a previous declaration");
                }
            }
        }
        if(params->get_lst_entries().size() != 0){
             error(@1, "Proc call " + $1 + " requires "+ std::to_string(params->get_lst_entries().size()) +" parameters");
        }
        $$ = new proccall_astnode($1); 
    }
    
    
}
| IDENTIFIER '(' expression_list ')' ';' {
    if(predefined.find($1) != predefined.end()){
      std::vector<expression_astnode*> expressions = $3->get_expressions();
      $$ = new proccall_astnode($1);
      for (unsigned int i = 0; i < expressions.size(); i++) {
         $$->add_argument(expressions[i]);
      }
    }
    else{ 
        GST_Entry* gst_entry = gst->get_gst_entry_by_name($1);
        if(gst_entry == nullptr){
             error(@1, "Identifier: " + $1  + " not declared");
        }
        LST* lst = gst_entry->get_lst();
        LST* params = new LST();
        for(auto entry : lst->get_lst_entries()){
            if(entry->get_scope() == Scope::Param){
                bool success = params->add_lst_entry(entry);
                if(!success){
                     error(@1, "\"" + entry->get_name() + "\" has a previous declaration");
                }
            }
        }
        std::vector<LST_Entry*> param_entries = params->get_lst_entries();    
        std::vector<expression_astnode*> expressions = $3->get_expressions();
        if(param_entries.size() != expressions.size()){
              error(@1, "Proc call " + $1 + " requires "+ std::to_string(param_entries.size()) +" parameters. Received " + std::to_string(expressions.size()) + " parameters");
        }
        proccall_astnode* output = new proccall_astnode($1);
        for (unsigned int i = 0; i < expressions.size(); i++) {
            if(param_entries[i]->get_sub_type() == "int" && expressions[i]->exp_type_ == "float"){
                op_unary_astnode* temp = new op_unary_astnode(expressions[i],op_unary_type::TO_INT);
                output->add_argument(temp);
            }
            else if(param_entries[i]->get_sub_type() == "float" && expressions[i]->exp_type_ == "int"){
                op_unary_astnode* temp = new op_unary_astnode(expressions[i],op_unary_type::TO_FLOAT);
                output->add_argument(temp);
            }
            else if((param_entries[i]->get_dereferenced_type() == "void"|| expressions[i]->get_dereferenced_type() == param_entries[i]->get_dereferenced_type()) && expressions[i]->get_dereferenced_type() != ""){
                output->add_argument(expressions[i]);
            }
            else if(param_entries[i]->get_dereferenced_type() != "" && (expressions[i]->get_dereferenced_type() == "void"|| expressions[i]->get_dereferenced_type() == param_entries[i]->get_dereferenced_type())){
                output->add_argument(expressions[i]);
            }

            else if(param_entries[i]->get_sub_type() != expressions[i]->exp_type_){
                  error(@1, "Proc call " + $1 + " requires parameter of type " + param_entries[i]->get_sub_type() + ". Received type: " + expressions[i]->exp_type_);
            }
            else{
                output->add_argument(expressions[i]);
            }
        }
        $$ = output;
    }

}

expression: 
logical_and_expression { $$ = $1; }
| expression OR_OP logical_and_expression {

    if( ($1->exp_type_.substr(0,6) == "struct" && $1->get_dereferenced_type() == "") || ($3->exp_type_.substr(0,6) == "struct" && $3->get_dereferenced_type() == "") ){
        error(@1, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$ = new op_binary_astnode($1, $3, op_binary_type::OR_OP); 
    $$->exp_type_ = "int";
    $$->is_lvalue = false;
}

logical_and_expression: 
equality_expression { $$ = $1; }
| logical_and_expression AND_OP equality_expression { 

    if( ($1->exp_type_.substr(0,6) == "struct" && $1->get_dereferenced_type() == "") || ($3->exp_type_.substr(0,6) == "struct" && $3->get_dereferenced_type() == "") ){
        error(@1, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$ = new op_binary_astnode($1, $3, op_binary_type::AND_OP); 
    $$->exp_type_ = "int";
    $$->is_lvalue = false;
}

equality_expression: 
relational_expression { $$ = $1; }
| equality_expression EQ_OP relational_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::EQ_OP_INT); 
    
    std::string eq_exp_type = $1->exp_type_;
    std::string rel_type = $3->exp_type_;
    if( (eq_exp_type.compare("float") == 0) && (rel_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::EQ_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (eq_exp_type.compare("float") == 0) && (rel_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::EQ_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (eq_exp_type.compare("int") == 0) && (rel_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::EQ_OP_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (eq_exp_type.compare("int") == 0) && (rel_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::EQ_OP_INT);
        $$->exp_type_ = "int";
    }
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::EQ_OP_INT);
        $$->exp_type_ = "int";
    }  
    else{
        error(@2, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
}
| equality_expression NE_OP relational_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::NE_OP_INT); 
    
    std::string eq_exp_type = $1->exp_type_;
    std::string rel_type = $3->exp_type_;
    if( (eq_exp_type.compare("float") == 0) && (rel_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::NE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (eq_exp_type.compare("float") == 0) && (rel_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::NE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (eq_exp_type.compare("int") == 0) && (rel_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::NE_OP_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (eq_exp_type.compare("int") == 0) && (rel_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::NE_OP_INT);
        $$->exp_type_ = "int";
    }
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::NE_OP_INT);
        $$->exp_type_ = "int";
    } 
    else{
        error(@2, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    } 
    $$->is_lvalue = false;
}

relational_expression: 
additive_expression { $$ = $1; }
| relational_expression '<' additive_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::LT_OP_INT); 
    std::string rel_type = $1->exp_type_;
    std::string add_type = $3->exp_type_;
    if( (rel_type.compare("float") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::LT_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("float") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::LT_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::LT_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::LT_OP_INT);
        $$->exp_type_ = "int";
    }
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::LT_OP_INT);
        $$->exp_type_ = "int";
    } 
    else{
        error(@2, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
} 
| relational_expression '>' additive_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::GT_OP_INT); 
    
    std::string rel_type = $1->exp_type_;
    std::string add_type = $3->exp_type_;
    if( (rel_type.compare("float") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::GT_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("float") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::GT_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::GT_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::GT_OP_INT);
        $$->exp_type_ = "int";
    } 
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::GT_OP_INT);
        $$->exp_type_ = "int";
    } 
    else{
        error(@2, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
} 
| relational_expression LE_OP additive_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::LE_OP_INT); 

    std::string rel_type = $1->exp_type_;
    std::string add_type = $3->exp_type_;
    if( (rel_type.compare("float") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::LE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("float") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::LE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::LE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::LE_OP_INT);
        $$->exp_type_ = "int";
    } 
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::LE_OP_INT);
        $$->exp_type_ = "int";
    } 
    else{
        error(@2, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
    
}
| relational_expression GE_OP additive_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::GE_OP_INT); 
    
    std::string rel_type = $1->exp_type_;
    std::string add_type = $3->exp_type_;
    if( (rel_type.compare("float") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::GE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("float") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::GE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::GE_OP_FLOAT);
        $$->exp_type_ = "int";
    }
    else if( (rel_type.compare("int") == 0) && (add_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::GE_OP_INT);
        $$->exp_type_ = "int";
    } 
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::GE_OP_INT);
        $$->exp_type_ = "int";
    } 
    else{
        error(@2, "Invalid operands types . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }   
    $$->is_lvalue = false;
}

additive_expression: 
multiplicative_expression { $$ = $1; }
| additive_expression '+' multiplicative_expression { 
    //$$ = new op_binary_astnode($1, $3, op_binary_type::PLUS_INT);

    std::string add_type = $1->exp_type_;
    std::string mul_type = $3->exp_type_;
    if( (add_type.compare("float") == 0) && (mul_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::PLUS_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (add_type.compare("float") == 0) && (mul_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::PLUS_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (add_type.compare("int") == 0) && (mul_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::PLUS_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (add_type.compare("int") == 0) && (mul_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::PLUS_INT);
        $$->exp_type_ = "int";
    }
    else if( ($1->get_dereferenced_type() != "") && (mul_type.compare("int") == 0) ){ //array and int //pointer and int
        $$ = new op_binary_astnode($1, $3, op_binary_type::PLUS_INT);
        $$->exp_type_ = get_referenced_type_from_string($1->get_dereferenced_type());
    }
    else if( ($3->get_dereferenced_type() != "") && (add_type.compare("int") == 0) ){ //int and array //int and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::PLUS_INT);
        $$->exp_type_ = get_referenced_type_from_string($3->get_dereferenced_type());
    }
    else
    {
          error(@2, "Invalid operands . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
    
}
| additive_expression '-' multiplicative_expression {
    $$ = new op_binary_astnode($1, $3, op_binary_type::MINUS_INT); 
    
    std::string add_type = $1->exp_type_;
    std::string mul_type = $3->exp_type_;
    if( (add_type.compare("float") == 0) && (mul_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::MINUS_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (add_type.compare("float") == 0) && (mul_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::MINUS_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (add_type.compare("int") == 0) && (mul_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::MINUS_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (add_type.compare("int") == 0) && (mul_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::MINUS_INT);
        $$->exp_type_ = "int";
    }
    else if( ($1->get_dereferenced_type() != "") && (mul_type.compare("int") == 0) ){ //array and int //pointer and int
        $$ = new op_binary_astnode($1, $3, op_binary_type::MINUS_INT);
        $$->exp_type_ = get_referenced_type_from_string($1->get_dereferenced_type());
    }
    else if( ($1->get_dereferenced_type() != "") && ($3->get_dereferenced_type() != "") && ($1->get_dereferenced_type() == $3->get_dereferenced_type()) ){ //array and array //pointer and pointer
        $$ = new op_binary_astnode($1, $3, op_binary_type::MINUS_INT);
        $$->exp_type_ = "int";
    } 
    else{
          error(@2, "Invalid operands . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
}

unary_expression: 
postfix_expression { $$ = $1; }
| unary_operator unary_expression { 
    $$ = new op_unary_astnode($2, $1); 
    switch($1){
        case op_unary_type::NOT:
        {
            if ($2->exp_type_ != "int" && $2->exp_type_ != "float" && $2->get_dereferenced_type() == ""){
                  error(@2, "Operand of unary "+ op_unary_map.at($1) +" should be an int or float or pointer. Received type: " + $2->exp_type_);
            }
            $$->exp_type_ = "int";
            $$->is_lvalue = false;
            break;
        }
        case op_unary_type::TO_INT:
        {
            if ($2->exp_type_ != "int" && $2->exp_type_ != "float"){
                  error(@2, "Operand of unary "+ op_unary_map.at($1) +" should be an int or float. Received type: " + $2->exp_type_);
            }
            $$->exp_type_ = "int";
            $$->is_lvalue = true;
            break;
        }
        case op_unary_type::TO_FLOAT:
        {
            if ($2->exp_type_ != "int" && $2->exp_type_ != "float"){
                  error(@2, "Operand of unary "+ op_unary_map.at($1) +" should be an int or float. Received type: " + $2->exp_type_);
            }
            $$->exp_type_ = "float";
            $$->is_lvalue = true;
            break;
        }
        case op_unary_type::PP:
        {
            // PP not defined in grammar
        }
        case op_unary_type::UMINUS:
        {
            if ($2->exp_type_ != "int" && $2->exp_type_ != "float"){
                  error(@2, "Operand of unary "+ op_unary_map.at($1) +" should be an int or float. Received type: " + $2->exp_type_);
            }
            $$->exp_type_ = $2->exp_type_;
            $$->is_lvalue = false;
            break;
        }
        case op_unary_type::DEREF:
        {
            std::string der = $2->get_dereferenced_type();
            if(der == "")
            {
                  error(@2, "Operand of unary * should be a pointer. Received type: " + $2->exp_type_);
            }
            else
            {
                $$->exp_type_ = der;
            }
            $$->is_lvalue = true;
            break;
        }
        case op_unary_type::ADDRESS:
        {
            if(!$2->is_lvalue){
                error(@2, "Operand of & should  have lvalue");
            }
            std::string ref = get_referenced_type_from_string($2->exp_type_);
            $$->exp_type_ = ref;
            $$->is_lvalue = false;
            break;
        }
        default:
        {
              error(@2, "Invalid unary operator");
        }
    }
}

multiplicative_expression: 
unary_expression { $$ = $1; }
| multiplicative_expression '*' unary_expression {
    std::string mul_type = $1->exp_type_;
    std::string un_type = $3->exp_type_;
    if( (mul_type.compare("float") == 0) && (un_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::MULT_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (mul_type.compare("float") == 0) && (un_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::MULT_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (mul_type.compare("int") == 0) && (un_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::MULT_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (mul_type.compare("int") == 0) && (un_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::MULT_INT);
        $$->exp_type_ = "int";
    }
    else{
        error(@2, "Invalid operands . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
}
| multiplicative_expression '/' unary_expression {
    std::string mul_type = $1->exp_type_;
    std::string un_type = $3->exp_type_;
    if( (mul_type.compare("float") == 0) && (un_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::DIV_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (mul_type.compare("float") == 0) && (un_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, new op_unary_astnode($3, op_unary_type::TO_FLOAT), op_binary_type::DIV_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (mul_type.compare("int") == 0) && (un_type.compare("float") == 0) )
    {
        $$ = new op_binary_astnode(new op_unary_astnode($1, op_unary_type::TO_FLOAT), $3, op_binary_type::DIV_FLOAT);
        $$->exp_type_ = "float";
    }
    else if( (mul_type.compare("int") == 0) && (un_type.compare("int") == 0) )
    {
        $$ = new op_binary_astnode($1, $3, op_binary_type::DIV_INT);
        $$->exp_type_ = "int";
    }
    else{
        error(@2, "Invalid operands . Type 1: " + $1->exp_type_ + " Type 2: " + $3->exp_type_);
    }
    $$->is_lvalue = false;
}

postfix_expression: 
primary_expression { $$ = $1; }
| postfix_expression '[' expression ']' {
    std::string deref_type = $1->get_dereferenced_type();
    if (deref_type==""){
          error(@2, "First Operand of [] should be a pointer or array. Received type: " + $1->exp_type_);
    }
    else if($3->exp_type_ != "int"){
          error(@2, "Second Operand of [] should be an int. Received type: " + $3->exp_type_);
    }
    $$ = new arrayref_astnode($1, $3);
    $$->exp_type_ = deref_type;
    $$->is_lvalue = true; 
}
| IDENTIFIER '(' ')' { 
    if(predefined.find($1) != predefined.end()){
      $$ = new funcall_astnode($1); 
      $$->exp_type_ = predefined.at($1);
      $$->is_lvalue = false;
    }
    else{
        GST_Entry* gst_entry = gst->get_gst_entry_by_name($1);
        if(gst_entry == nullptr){
              error(@1, "Identifier: " + $1  + " not declared");
        }
        LST* lst = gst_entry->get_lst();
        LST* params = new LST();
        for(auto entry : lst->get_lst_entries()){
            if(entry->get_scope() == Scope::Param){
                bool success = params->add_lst_entry(entry);
                if(!success){
                      error(@1, "\"" + entry->get_name() + "\" has a previous declaration");
                }
            }
        }
        if(params->get_lst_entries().size() != 0){
              error(@1, "Function " + $1 + " requires "+ std::to_string(params->get_lst_entries().size()) +" parameters");
        }
        $$ = new funcall_astnode($1);
        $$->exp_type_ = gst_entry->get_sub_type();
        $$->is_lvalue = false;
    }
}
| IDENTIFIER '(' expression_list ')' {
    if(predefined.find($1) != predefined.end()){
        std::vector<expression_astnode*> expressions = $3->get_expressions();
        funcall_astnode* output = new funcall_astnode($1); 
        for (unsigned int i = 0; i < expressions.size(); i++) {
            output->add_argument(expressions[i]);
        }
        $$ = output;
        $$->exp_type_ = predefined.at($1);
        $$->is_lvalue = false;
    }
    else{
        GST_Entry* gst_entry = gst->get_gst_entry_by_name($1);
        if(gst_entry == nullptr){
              error(@1, "Identifier: " + $1  + " not declared");
        }
        LST* lst = gst_entry->get_lst();
        LST* params = new LST();
        for(auto entry : lst->get_lst_entries()){
            if(entry->get_scope() == Scope::Param){
                bool success = params->add_lst_entry(entry);
                if(!success){
                      error(@1, "\"" + entry->get_name() + "\" has a previous declaration");
                }
            }
        }    
        std::vector<LST_Entry*> param_entries = params->get_lst_entries();    
        std::vector<expression_astnode*> expressions = $3->get_expressions();
        if(param_entries.size() != expressions.size()){
              error(@1, "Function " + $1 + " requires "+ std::to_string(param_entries.size()) +" parameters. Received " + std::to_string(expressions.size()) + " parameters");
        }
        funcall_astnode* output = new funcall_astnode($1);
        for (unsigned int i = 0; i < expressions.size(); i++) {
            if(param_entries[i]->get_sub_type() == "int" && expressions[i]->exp_type_ == "float"){
                op_unary_astnode* temp = new op_unary_astnode(expressions[i],op_unary_type::TO_INT);
                output->add_argument(temp);
            }
            else if(param_entries[i]->get_sub_type() == "float" && expressions[i]->exp_type_ == "int"){
                op_unary_astnode* temp = new op_unary_astnode(expressions[i],op_unary_type::TO_FLOAT);
                output->add_argument(temp);
            }
            else if((param_entries[i]->get_dereferenced_type() == "void"|| expressions[i]->get_dereferenced_type() == param_entries[i]->get_dereferenced_type()) && expressions[i]->get_dereferenced_type() != ""){
                output->add_argument(expressions[i]);
            }
            else if(param_entries[i]->get_dereferenced_type() != "" && (expressions[i]->get_dereferenced_type() == "void"|| expressions[i]->get_dereferenced_type() == param_entries[i]->get_dereferenced_type())){
                output->add_argument(expressions[i]);
            }

            else if(param_entries[i]->get_sub_type() != expressions[i]->exp_type_){
                  error(@1, "Function " + $1 + " requires parameter of type " + param_entries[i]->get_sub_type() + ". Received type: " + expressions[i]->exp_type_);
            }
            else{
                output->add_argument(expressions[i]);
            }
        }
        $$ = output;
        $$->exp_type_ = gst_entry->get_sub_type();
        $$->is_lvalue = false;
    }
}
| postfix_expression '.' IDENTIFIER {
    if($1->exp_type_.substr(0,6) != "struct" || $1->get_dereferenced_type() != ""){
          error(@2, "First Operand of . should be a struct. Received type: " + $1->exp_type_);
    }
    $$ = new member_astnode($1, new identifier_astnode($3));
    //$1 has exp_type_ of "struct x"
    //$3 is name of member variable
    GST_Entry* gst_entry = gst->get_gst_entry_by_name($1->exp_type_);
    if(gst_entry == nullptr){
          error(@1, "Identifier: " + $1->exp_type_  + " not declared");
    }
    LST * struct_lst = gst_entry->get_lst();
    LST_Entry* lst_entry = struct_lst->get_lst_entry_by_name($3);
    if(lst_entry == nullptr){
          error(@1, $1->exp_type_  + " has no member " + $3);
    }    
    $$->exp_type_ = lst_entry->get_sub_type();
    $$->is_lvalue = true;
}
| postfix_expression PTR_OP IDENTIFIER {
    if($1->get_dereferenced_type().substr(0,6) == "struct" && $1->get_dereferenced_type() != ""){
        $$ = new arrow_astnode($1, new identifier_astnode($3));
       
        //$1 has exp_type_ of "struct x* or struct x[2]"
        //$3 is name of member variable
       
        std::string str = $1->exp_type_;
        std::string name;
        if ( contain(str , "*") && ( ! contain(str.substr(str.find("*") + 1) , "*") ) && (!contain(str , "[")) )
        {
            name = $1->exp_type_.substr(0,$1->exp_type_.find("*"));
        }
        else if ( contain(str , "[") && ( ! contain(str.substr(str.find("]")) , "[") ) && (!contain(str , "*")) )
        {
            name = $1->exp_type_.substr(0,$1->exp_type_.find("["));
        }
        else
        { 
              error(@2, "First Operand of -> should be a pointer to a struct. Received type: " + $1->exp_type_);
        }
        GST_Entry* gst_entry = gst->get_gst_entry_by_name(name);
        if(gst_entry == nullptr){
              error(@1, "Identifier: " + name  + " not declared");
        }
        LST * struct_lst = gst_entry->get_lst();
        LST_Entry* lst_entry = struct_lst->get_lst_entry_by_name($3);
        if(lst_entry == nullptr){
              error(@1, name + " has no member " + $3);
        }
        $$->exp_type_ = lst_entry->get_sub_type();
        $$->is_lvalue = true;
    }
    else{
          error(@2, "First Operand of -> should be a pointer to a struct. Received type: " + $1->exp_type_);
    }

}
| postfix_expression INC_OP {
    if($1->is_lvalue == true){
        if($1->exp_type_ != "int" && $1->exp_type_ != "float" && $1->get_dereferenced_type() == "" && contain($1->exp_type_ , "[") && !contain($1->exp_type_ , "(")){
              error(@2, "Operand of ++ should be an int, float or pointer. Received type: " + $1->exp_type_);
        }
        $$ = new op_unary_astnode($1,op_unary_type::PP); 
        $$->exp_type_ = $1->exp_type_;
        $$->is_lvalue = false;
    }
    else{
          error(@2, "Operand of ++ should be an lvalue");
    }
}

primary_expression: 
IDENTIFIER { 
    $$ = new identifier_astnode($1);
    LST_Entry* lst_entry = current_lst->get_lst_entry_by_name($1);
    if(lst_entry == nullptr){
        error(@1, "Variable \"" + $1 + "\" not declared");
    }
    else{
        $$->exp_type_ = lst_entry->get_sub_type();
    }
    $$->is_lvalue = true;
}
| INT_CONSTANT { 
    $$  = new int_astnode(stoi($1)); 
    $$->is_lvalue = false;
    $$->cvalue = stoi($1);
}
| FLOAT_CONSTANT { 
    $$  = new float_astnode(stof($1)); 
    $$->is_lvalue = false;
}
| STRING_LITERAL { 
    $$  = new string_astnode($1); 
    $$->is_lvalue = false;
}
| '(' expression ')' { $$ = $2; }

expression_list: 
expression { $$ = new Expression_List(); $$->add_expression($1); }
| expression_list ',' expression { $$ = $1; $$->add_expression($3); }

unary_operator: 
'-' { $$ = op_unary_type::UMINUS; }
| '!' { $$ = op_unary_type::NOT; }
| '&' { $$ = op_unary_type::ADDRESS; }
| '*' { $$ = op_unary_type::DEREF; }

selection_statement: 
IF '(' expression ')' statement ELSE statement { $$ = new if_astnode($3, $5, $7); }

iteration_statement: 
WHILE '(' expression ')' statement { $$ = new while_astnode($3, $5); }
| FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement {
    expression_astnode* e = $3;
    expression_astnode* e2 = $7;
    $$ = new for_astnode(e, $5, e2, $9);

}

%%

void IPL::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cout << "Error at line " << l.begin.line << ": " << err_message << "\n";
   exit(1);
}

