#ifndef CLASSES_HH
#define CLASSES_HH

// using namespace std;
#include<iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>


namespace IPL
{
    enum class typeExp {
        empty_astnode,
        seq_astnode,
        assignS_astnode,
        if_astnode,
        while_astnode,
        for_astnode,
        return_astnode,
        proccall_astnode,
        identifier_astnode,
        arrayref_astnode,
        member_astnode,
        arrow_astnode,
        op_binary_astnode,
        op_unary_astnode,
        assignE_astnode,
        funcall_astnode,
        int_astnode,
        float_astnode,
        string_astnode
    };

    enum class op_binary_type {
        OR_OP,
        AND_OP,
        EQ_OP_INT,
        NE_OP_INT,
        LT_OP_INT,
        LE_OP_INT,
        GT_OP_INT,
        GE_OP_INT,
        EQ_OP_FLOAT,
        NE_OP_FLOAT,
        LT_OP_FLOAT,
        LE_OP_FLOAT,
        GT_OP_FLOAT,
        GE_OP_FLOAT,
        PLUS_INT,
        MINUS_INT,
        MULT_INT,
        DIV_INT,
        PLUS_FLOAT,
        MINUS_FLOAT,
        MULT_FLOAT,
        DIV_FLOAT
    };

    enum class op_unary_type {
        TO_INT,
        TO_FLOAT,
        UMINUS,
        NOT,
        ADDRESS,
        DEREF,
        PP
    };

    static const char *op_unary_type_string[] = {
        "TO_INT",
        "TO_FLOAT",
        "UMINUS",
        "NOT",
        "ADDRESS",
        "DEREF",
        "PP"
    };

    static const char *op_binary_type_string[] = {

        "OR_OP","AND_OP","EQ_OP_INT","NE_OP_INT","LT_OP_INT","LE_OP_INT","GT_OP_INT","GE_OP_INT",
        "EQ_OP_FLOAT","NE_OP_FLOAT","LT_OP_FLOAT","LE_OP_FLOAT","GT_OP_FLOAT","GE_OP_FLOAT",
        "PLUS_INT","MINUS_INT","MULT_INT","DIV_INT","PLUS_FLOAT","MINUS_FLOAT","MULT_FLOAT","DIV_FLOAT"

    };



    class abstract_astnode {
        public:
            virtual ~abstract_astnode() {};
            //virtual void print() = 0;
            enum typeExp astnode_type;
            int l = -1; //seti-ullman labelling
            virtual void gencode(int b) = 0;
    };

    class statement_astnode : public abstract_astnode {
        public:
            virtual ~statement_astnode() {}
            std::vector<int> next;
    };

    class expression_astnode : public abstract_astnode {
        public:
            virtual ~expression_astnode() {}
            std::string exp_type_;
            std::string get_dereferenced_type();
            bool is_bool = false;
            bool fall = true;
            std::vector<int> truelist;
            std::vector<int> falselist;
            int startline;
            int endline;

    };

    class reference_astnode : public expression_astnode {
        public:
            virtual ~reference_astnode() {}
    };

    class empty_astnode : public statement_astnode {
        public:
            empty_astnode();
            //void print();
            void gencode(int b){};
    };

    class seq_astnode : public statement_astnode {
        public:
            std::vector<statement_astnode*> statements_;
            seq_astnode();
            void add_statement(statement_astnode* statement);
            //void print();
            void gencode(int b);
    };

    class assignS_astnode : public statement_astnode {
        public:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
        
            assignS_astnode(expression_astnode* expression1, expression_astnode* expression2);
            //void print();
            void gencode(int b);
    };

    class if_astnode : public statement_astnode {
        public:
            expression_astnode* condition_;
            statement_astnode* then_;
            statement_astnode* else_;
        
            if_astnode(expression_astnode* Condition, statement_astnode* Then, statement_astnode* Else);
            //void print();
            void gencode(int b);
    };

    class while_astnode : public statement_astnode {
        public:
            expression_astnode* condition_;
            statement_astnode* body_;
        
            while_astnode(expression_astnode* condition, statement_astnode* body);
            //void print();
            void gencode(int b);
    };

    class for_astnode : public statement_astnode {
        public:
            expression_astnode* init_;
            expression_astnode* condition_;
            expression_astnode* update_;
            statement_astnode* body_;
        
            for_astnode(expression_astnode* init, expression_astnode* condition, expression_astnode* update, statement_astnode* body);
            //void print();
            void gencode(int b);
    };

    class return_astnode : public statement_astnode {
        public:
            expression_astnode* expression_;
        
            return_astnode(expression_astnode* expression);
            //void print();
            void gencode(int b);
    };

    class proccall_astnode : public statement_astnode {
        public:
            std::string name_;
            std::vector<expression_astnode*> arguments_;
            proccall_astnode(std::string name);
            void add_argument(expression_astnode* argument);
            //void print();
            void gencode(int b);
    };
    
    class op_binary_astnode : public expression_astnode {
        public:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
            enum op_binary_type op_;
        
            op_binary_astnode(expression_astnode* expression1, expression_astnode* expression2, enum op_binary_type op);
            //void print();
            void gencode(int b);
    };

    class op_unary_astnode : public expression_astnode {
        public:
            expression_astnode* expression_;
            enum op_unary_type op_;
        
            op_unary_astnode(expression_astnode* expression, enum op_unary_type op);
            //void print();
            void gencode(int b);
    };

    class int_astnode : public expression_astnode {
        public:
            int value_;
        
            int_astnode(int value);
            //void print();
            void gencode(int b);
    };

    class float_astnode : public expression_astnode {
        public:
            float value_;
        
            float_astnode(float value);
            //void print();
            void gencode(int b){};
    };

    class string_astnode : public expression_astnode {
        public:
            std::string value_;
            string_astnode(std::string value);
            //void print();
            void gencode(int b){};
            std::string str_label;
    };

    class assignE_astnode : public expression_astnode {
        public:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
        
            assignE_astnode(expression_astnode* expression1, expression_astnode* expression2);
            //void print();
            void gencode(int b);
            expression_astnode* get_expression1();
            expression_astnode* get_expression2();
            
    };

    class funcall_astnode : public expression_astnode {
        public:
            std::string name_;
            std::vector<expression_astnode*> arguments_;
        
            funcall_astnode(std::string name);
            void add_argument(expression_astnode* argument);
            //void print();
            void gencode(int b);
    };

    class identifier_astnode : public reference_astnode {
        public:
            std::string name_;
        
            identifier_astnode(std::string name);
            //void print();
            void gencode(int b);
            
    };

    class member_astnode : public reference_astnode {
        public:
            expression_astnode* expression_;
            identifier_astnode* name_;
        
            member_astnode(expression_astnode* expression, identifier_astnode* name);
            //void print();
            void gencode(int b);
    };

    class arrayref_astnode : public reference_astnode {
        public:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
        
            arrayref_astnode(expression_astnode* expression1, expression_astnode* expression2);
            //void print();
            void gencode(int b);
    };

    class arrow_astnode : public reference_astnode {
        public:
            expression_astnode* expression_;
            identifier_astnode* name_;
        
            arrow_astnode(expression_astnode* expression, identifier_astnode* name);
            //void print();
            void gencode(int b);
    };


    enum class SymbolType {
        Var,
        Const,
        Func,
        Struct
    };

    enum class Scope {
        Global,
        Local,
        Param
    };






    class LocalSymbolTable_Entry
    {
    public:
        std::string lst_entry_name_;
        SymbolType lst_entry_type_;
        Scope lst_entry_scope_;
        std::string lst_entry_sub_type_;
        int lst_entry_size_;
        int lst_entry_offset_;

        LocalSymbolTable_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset);
       
        Scope get_lst_entry_scope();
        std::string get_lst_entry_type_name();
        std::string get_lst_entry_scope_name();
       
        int get_lst_entry_size();
        
        void set_lst_entry_name(std::string name);
      
        void set_lst_entry_sub_type(std::string sub_type);
      
        std::string get_dereferenced_type();
    };

    class LocalSymbolTable
    {
    public:
        std::vector<LocalSymbolTable_Entry *> lst_entries_;

        LocalSymbolTable();
        bool add_lst_entry(LocalSymbolTable_Entry *LocalSymbolTable_Entry);
        LocalSymbolTable_Entry *get_lst_entry_by_name(std::string name);
        int get_size();
        //void print();
        std::vector<LocalSymbolTable_Entry *> get_lst_entries();


    };


    class GlobalSymbolTable_Entry
    {
    public:
        std::string gst_entry_name_;
        SymbolType gst_entry_type_;
        Scope gst_entry_scope_;
        std::string gst_entry_sub_type_;
        int gst_entry_size_;
        int gst_entry_offset_;
        LocalSymbolTable *lst_;
        abstract_astnode* ast_;

        GlobalSymbolTable_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset);
        GlobalSymbolTable_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset, LocalSymbolTable *lst);
        SymbolType get_gst_entry_type();
        std::string get_gst_entry_type_name();
        std::string get_gst_entry_scope_name();
        int get_gst_entry_size();
        LocalSymbolTable *get_lst();
        void set_gst_entry_type(SymbolType type);
        void set_gst_entry_size(int size);
        void set_lst(LocalSymbolTable *lst);
        void set_ast(abstract_astnode* ast);
        abstract_astnode* get_ast();

    };


    //vector comparator

    

    

    class GlobalSymbolTable
    {
    public:
        std::vector<GlobalSymbolTable_Entry *> gst_entries_;

        GlobalSymbolTable();
        bool add_GlobalSymbolTable_entry(GlobalSymbolTable_Entry *GlobalSymbolTable_Entry);
        GlobalSymbolTable_Entry *get_GlobalSymbolTable_entry_by_name(std::string name);
        void print();
        std::vector<GlobalSymbolTable_Entry *> get_gst_entries();
        GlobalSymbolTable_Entry *get_gst_entry_by_lst(LocalSymbolTable *lst);

    };

    class Type_Specifier
    {
    public:
        std::string type_specifier_type_;

        Type_Specifier();
        Type_Specifier(std::string type);
        int get_type_specifier_size(GlobalSymbolTable *gst); 
    };

    class Fun_Declarator
    {
    public:
        std::string fun_decl_name_;
    
        Fun_Declarator();
        Fun_Declarator(std::string name);
    };

    class Declaration_List
    {
    public:
        std::vector<LocalSymbolTable_Entry *> lst_entries_;
    
        Declaration_List();
        bool add_lst_entry(LocalSymbolTable_Entry *lst_entry);
        std::vector<LocalSymbolTable_Entry *> get_lst_entries();
    };
    
    class Declarator
    {
    public:
        std::string decl_stars_;
        std::string decl_name_;
        int decl_size_;
    
        Declarator();
        Declarator(std::string name);
        Declarator(std::string name, int size);
        std::string get_decl_name();
        std::string get_decl_stars();
        void set_decl_stars(std::string stars);
    };

    class Declarator_List
    {
    public:
        std::vector<Declarator> decl_list_declarators_;
     
        Declarator_List();
        void add_decl_list_declarator(Declarator declerator);
        std::vector<Declarator> get_decl_list_declarators();
    }; 

    class Parameter_List
    {
    public:
        std::stack<LocalSymbolTable_Entry *> param_list_parameters_;
    
        Parameter_List();
        void add_param_list_parameter(LocalSymbolTable_Entry *parameter);
        LocalSymbolTable_Entry * pop();
    };
     
    class Compound_Statement {
        public:
            std::vector<LocalSymbolTable_Entry *> lst_entries_;
            abstract_astnode* ast_;
        
            Compound_Statement();
            bool add_lst_entry(LocalSymbolTable_Entry * lst_entry);
            std::vector<LocalSymbolTable_Entry *> get_lst_entries();
            void set_ast(abstract_astnode* ast);
            abstract_astnode* get_ast();
    };

    class Expression_List {
        public:
            std::vector<expression_astnode*> expressions_;
        
            Expression_List();
            void add_expression(expression_astnode* expression);
            std::vector<expression_astnode*> get_expressions();
    };
   

}
#endif
