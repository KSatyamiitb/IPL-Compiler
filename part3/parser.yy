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
GlobalSymbolTable* gst = new GlobalSymbolTable();
LocalSymbolTable* current_lst = NULL;
int current_offset = 0;

std::map<std::string , std::string> predefined {
    {"printf", "void"}
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

%start program



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

%nterm <int> program translation_unit struct_specifier function_definition 
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
program: translation_unit {gst->print();}

translation_unit: 
struct_specifier 
| function_definition
| translation_unit struct_specifier
| translation_unit function_definition

struct_specifier: 
STRUCT IDENTIFIER { //matches
	LocalSymbolTable* lst = new LocalSymbolTable();
    std::string na = "struct " + $2;
	gst->add_GlobalSymbolTable_entry(new GlobalSymbolTable_Entry(na,SymbolType::Struct,Scope::Global,"-",0,0,lst));
    current_offset = 0;
    current_lst = lst;
} '{' declaration_list '}' ';'{
    std::string na = "struct " + $2;
    GlobalSymbolTable_Entry* gst_entry_ = gst->get_GlobalSymbolTable_entry_by_name(na);
	gst_entry_->gst_entry_size_ = gst_entry_->get_lst()->get_size();
}

function_definition: //matches
type_specifier fun_declarator {
    //int type_specifier_size = $1->get_size(gst);
	std::string name = $2->fun_decl_name_;
	GlobalSymbolTable_Entry* entry = gst->get_GlobalSymbolTable_entry_by_name(name);
	std::string type = $1->type_specifier_type_;
	entry->gst_entry_sub_type_ = type;
	current_offset = 0;
    current_lst = entry->get_lst();
} compound_statement {
    std::string na = $2->fun_decl_name_;
	GlobalSymbolTable_Entry* gst_entry_ = gst->get_GlobalSymbolTable_entry_by_name (na); 
    gst_entry_->set_ast( $4->get_ast() );
    gst_entry_->gst_entry_size_ = (gst_entry_->get_lst()->get_size());
}

type_specifier: 
VOID {$$ = new Type_Specifier("void");}
| INT {$$ = new Type_Specifier("int");}
| FLOAT {$$ = new Type_Specifier("float");}
| STRUCT IDENTIFIER {$$ = new Type_Specifier("struct "+$2);}

fun_declarator: //matches
IDENTIFIER '(' {
	LocalSymbolTable* lst = new LocalSymbolTable();
    GlobalSymbolTable_Entry *temp_gst = new GlobalSymbolTable_Entry($1,SymbolType::Func,Scope::Global,"-",0,0,lst);
    gst->add_GlobalSymbolTable_entry(temp_gst);
    current_offset = 12;
    current_lst = lst;

} parameter_list {
	Parameter_List* parameter_list_ = $4;
	LocalSymbolTable_Entry* lst_entry_ = parameter_list_->pop(); 
	while(lst_entry_ != NULL) {
		lst_entry_->lst_entry_offset_ = (current_offset); 
		current_offset = current_offset + lst_entry_->lst_entry_size_; 
		lst_entry_ = parameter_list_->pop();
	}
} ')' { 
    $$ = new Fun_Declarator($1);
}
| IDENTIFIER '(' ')' { 
	LocalSymbolTable* lst = new LocalSymbolTable();
    GlobalSymbolTable_Entry *temp_gst = new GlobalSymbolTable_Entry($1,SymbolType::Func,Scope::Global,"-",0,0,lst);
	gst->add_GlobalSymbolTable_entry(temp_gst);
    current_offset = 0;
    current_lst = lst;
    
    $$ = new Fun_Declarator($1); 
}

parameter_list: 
parameter_declaration {
    std::vector<LocalSymbolTable_Entry*> lst_entries = $1->get_lst_entries();
	$$ = new Parameter_List();
    for (unsigned int i = 0; i < lst_entries.size(); i++) {
        $$->add_param_list_parameter(lst_entries[i]);
    }
}
| parameter_list ',' parameter_declaration
{
	std::vector<LocalSymbolTable_Entry*> lst_entries = $3->get_lst_entries();
	$$ = $1;
    for (unsigned int i = 0; i < lst_entries.size(); i++) {
        $$->add_param_list_parameter(lst_entries[i]);
    }
}

parameter_declaration: 
type_specifier declarator {
    $$ = new Declaration_List();
	int type_specifier_size = $1->get_type_specifier_size(gst);
    std::vector<Declarator> declarators = {*$2};
    for (unsigned int i = 0; i < declarators.size(); i++) {
        int size = type_specifier_size;
        if(declarators[i].decl_stars_[0] == '*')
        {
            size = 4;
        }
        int s = declarators[i].decl_size_*(size);
        std::string n = $1->type_specifier_type_+declarators[i].decl_stars_;
        std::string name = declarators[i].decl_name_;
        LocalSymbolTable_Entry* temp = new LocalSymbolTable_Entry(name, SymbolType::Var, Scope::Param, n, s, 0);
        current_lst->add_lst_entry(temp);
        $$->add_lst_entry(temp);
    }
    
}

declaration_list:
declaration {
    $$ = $1;
}
| declaration_list declaration {
    $$ = $1;
    std::vector<LocalSymbolTable_Entry *> lst_entries_ = $2->get_lst_entries();
    for ( unsigned int i = 0; i < lst_entries_.size(); i++)
    {
        $$->add_lst_entry(lst_entries_[i]);
    }
}

declaration: 
type_specifier declarator_list ';' {
    $$ = new Declaration_List();
	int type_specifier_size = $1->get_type_specifier_size(gst);
    std::vector<Declarator> declarators = $2->get_decl_list_declarators();
    for (unsigned int i = 0; i < declarators.size(); i++) {

        int size = type_specifier_size;
        if(declarators[i].decl_stars_[0] == '*')
        {
            size = 4;
            
        }
        int s = declarators[i].decl_size_*(size);
        std::string m = $1->type_specifier_type_+declarators[i].decl_stars_;
        std::string name = declarators[i].decl_name_;
        LocalSymbolTable_Entry* temp = new LocalSymbolTable_Entry(name, SymbolType::Var, Scope::Local, m, s, current_offset);
        current_lst->add_lst_entry(temp);
        $$->add_lst_entry(temp);
		current_offset = current_offset + declarators[i].decl_size_*(size);
    }

}

declarator_list: 
declarator {
    $$ = new Declarator_List();
    $$->add_decl_list_declarator(*$1);
}
| declarator_list ',' declarator {
    $$ = $1;
    $$->add_decl_list_declarator(*$3);
}

declarator_arr: 
IDENTIFIER {
    $$ = new Declarator($1,1);
}
| declarator_arr '[' INT_CONSTANT ']' {
    $$ = $1;
    $$->decl_size_ = stoi($3)*$$->decl_size_;
    $$->decl_stars_ = $$->decl_stars_+"["+$3+"]";
}

declarator: 
declarator_arr {
    $$ = $1;
}
| '*' declarator {
    $$ = $2;
    $$->decl_stars_ = "*"+$$->decl_stars_;
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
    std::vector<LocalSymbolTable_Entry *> lst_entries_ = $2->get_lst_entries();
    for ( unsigned int i = 0; i < lst_entries_.size(); i++)
    {
        current_offset = current_offset - lst_entries_[i]->lst_entry_size_;
        lst_entries_[i]->lst_entry_offset_ = (current_offset);
        $$->add_lst_entry(lst_entries_[i]);
    }

}
| '{' declaration_list statement_list '}' {
    current_offset = 0;
    $$ = new Compound_Statement();
    $$->set_ast($3);
    std::vector<LocalSymbolTable_Entry *> lst_entries_ = $2->get_lst_entries();
    for ( unsigned int i = 0; i < lst_entries_.size(); i++)
    {
        current_offset = current_offset - lst_entries_[i]->lst_entry_size_;
        lst_entries_[i]->lst_entry_offset_ = (current_offset);
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
    $$ = new return_astnode($2); 
}

assignment_expression: 
unary_expression '=' expression { 
    $$ = new assignE_astnode($1,$3); 

}

assignment_statement: 
assignment_expression ';' { $$ = new assignS_astnode($1->get_expression1(), $1->get_expression2()); }

procedure_call: 
IDENTIFIER '(' ')' ';' { 
    GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name($1);
    LocalSymbolTable* lst = gst_entry->get_lst();
    LocalSymbolTable* params = new LocalSymbolTable();
    for(auto entry : lst->get_lst_entries()){
        if(entry->lst_entry_scope_ == Scope::Param){
            params->add_lst_entry(entry);
        }
    }
    $$ = new proccall_astnode($1);  
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
        GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name($1);
        LocalSymbolTable* lst = gst_entry->get_lst();
        LocalSymbolTable* params = new LocalSymbolTable();
        for(auto entry : lst->get_lst_entries()){
            if(entry->lst_entry_scope_ == Scope::Param){
                params->add_lst_entry(entry);
            }
        }
        std::vector<LocalSymbolTable_Entry*> param_entries = params->get_lst_entries();    
        std::vector<expression_astnode*> expressions = $3->get_expressions();
        proccall_astnode* output = new proccall_astnode($1);
        for (unsigned int i = 0; i < expressions.size(); i++) {
            output->add_argument(expressions[i]);
        }
        $$ = output;
    }

}

expression: 
logical_and_expression { $$ = $1; }
| expression OR_OP logical_and_expression {
    $$ = new op_binary_astnode($1, $3, op_binary_type::OR_OP);
    $$->exp_type_ = "int";
}
logical_and_expression: 
equality_expression { $$ = $1; }
| logical_and_expression AND_OP equality_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::AND_OP);
    $$->exp_type_ = "int";
}

equality_expression: 
relational_expression { $$ = $1; }
| equality_expression EQ_OP relational_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::EQ_OP_INT); 
    $$->exp_type_ = "int";
}
| equality_expression NE_OP relational_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::NE_OP_INT); 
    $$->exp_type_ = "int";
}

relational_expression: 
additive_expression { $$ = $1; }
| relational_expression '<' additive_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::LT_OP_INT); 
    $$->exp_type_ = "int";
} 
| relational_expression '>' additive_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::GT_OP_INT); 
    $$->exp_type_ = "int";
} 
| relational_expression LE_OP additive_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::LE_OP_INT); 
    $$->exp_type_ = "int";
    
}
| relational_expression GE_OP additive_expression { 
    $$ = new op_binary_astnode($1, $3, op_binary_type::GE_OP_INT); 
    $$->exp_type_ = "int";
}

additive_expression: 
multiplicative_expression { $$ = $1; }
| additive_expression '+' multiplicative_expression { 
    std::string add_type = $1->exp_type_;
    std::string mul_type = $3->exp_type_;
    
    if( (add_type.compare("int") == 0) && (mul_type.compare("int") == 0) )
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
}
| additive_expression '-' multiplicative_expression {
    std::string add_type = $1->exp_type_;
    std::string mul_type = $3->exp_type_;
    if( (add_type.compare("int") == 0) && (mul_type.compare("int") == 0) )
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
}

unary_expression: 
postfix_expression { $$ = $1; }
| unary_operator unary_expression { 
    $$ = new op_unary_astnode($2, $1); 

    switch($1){
        case op_unary_type::DEREF:
        {
            std::string der = $2->get_dereferenced_type();
            $$->exp_type_ = der;
            break;
        }
        case op_unary_type::ADDRESS:
        {
            std::string ref = get_referenced_type_from_string($2->exp_type_);
            $$->exp_type_ = ref;
            break;
        }
        default:
        {
            $$->exp_type_ = "int";
        }
    }

}

multiplicative_expression: 
unary_expression { $$ = $1; }
| multiplicative_expression '*' unary_expression {
    $$ = new op_binary_astnode($1, $3, op_binary_type::MULT_INT);
    $$->exp_type_ = "int";
}
| multiplicative_expression '/' unary_expression {
    $$ = new op_binary_astnode($1, $3, op_binary_type::DIV_INT);
    $$->exp_type_ = "int";
}

postfix_expression: 
primary_expression { $$ = $1; }
| postfix_expression '[' expression ']' {
    $$ = new arrayref_astnode($1, $3);
    std::string deref_type = $1->get_dereferenced_type();
    $$->exp_type_ = deref_type;
}
| IDENTIFIER '(' ')' { 
    GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name($1);
    LocalSymbolTable* lst = gst_entry->get_lst();
    LocalSymbolTable* params = new LocalSymbolTable();
    for(auto entry : lst->get_lst_entries()){
        if(entry->lst_entry_scope_ == Scope::Param){
            params->add_lst_entry(entry);
        }
    }
    $$ = new funcall_astnode($1);
    $$->exp_type_ = gst_entry->gst_entry_sub_type_;
}
| IDENTIFIER '(' expression_list ')' {
    GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name($1);
    LocalSymbolTable* lst = gst_entry->get_lst();
    LocalSymbolTable* params = new LocalSymbolTable();
    for(auto entry : lst->get_lst_entries()){
        if(entry->lst_entry_scope_ == Scope::Param){
            params->add_lst_entry(entry);
        }
    }    
    std::vector<LocalSymbolTable_Entry*> param_entries = params->get_lst_entries();    
    std::vector<expression_astnode*> expressions = $3->get_expressions();
    funcall_astnode* output = new funcall_astnode($1);
    for (unsigned int i = 0; i < expressions.size(); i++) {
        output->add_argument(expressions[i]);
    }
    $$ = output;
    $$->exp_type_ = gst_entry->gst_entry_sub_type_;
}
| postfix_expression '.' IDENTIFIER {
    $$ = new member_astnode($1, new identifier_astnode($3));

    GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name($1->exp_type_);
    LocalSymbolTable * struct_lst = gst_entry->get_lst();
    LocalSymbolTable_Entry* lst_entry = struct_lst->get_lst_entry_by_name($3);
    $$->exp_type_ = lst_entry->lst_entry_sub_type_;


}
| postfix_expression PTR_OP IDENTIFIER {
        $$ = new arrow_astnode($1, new identifier_astnode($3));

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
        GlobalSymbolTable_Entry* gst_entry = gst->get_GlobalSymbolTable_entry_by_name(name);
        LocalSymbolTable * struct_lst = gst_entry->get_lst();
        LocalSymbolTable_Entry* lst_entry = struct_lst->get_lst_entry_by_name($3);
        $$->exp_type_ = lst_entry->lst_entry_sub_type_;

}
| postfix_expression INC_OP {
        $$ = new op_unary_astnode($1,op_unary_type::PP);
        $$->exp_type_ = $1->exp_type_; 
}

primary_expression: 
IDENTIFIER { 
    $$ = new identifier_astnode($1);
    LocalSymbolTable_Entry* lst_entry = current_lst->get_lst_entry_by_name($1);
    $$->exp_type_ = lst_entry->lst_entry_sub_type_;
}
| INT_CONSTANT { 
    $$  = new int_astnode(stoi($1)); 
}
| FLOAT_CONSTANT { 
    $$  = new float_astnode(stof($1)); 
}
| STRING_LITERAL { 
    $$  = new string_astnode($1); 
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

