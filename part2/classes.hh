#ifndef CLASSES_HH
#define CLASSES_HH

// using namespace std;
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

    const std::map<op_binary_type,std::string> op_binary_map = {
        {op_binary_type::OR_OP, "OR_OP"},
        {op_binary_type::AND_OP, "AND_OP"},
        {op_binary_type::EQ_OP_INT, "EQ_OP_INT"},
        {op_binary_type::NE_OP_INT, "NE_OP_INT"},
        {op_binary_type::LT_OP_INT, "LT_OP_INT"},
        {op_binary_type::LE_OP_INT, "LE_OP_INT"},
        {op_binary_type::GT_OP_INT, "GT_OP_INT"},
        {op_binary_type::GE_OP_INT, "GE_OP_INT"},
        {op_binary_type::EQ_OP_FLOAT, "EQ_OP_FLOAT"},
        {op_binary_type::NE_OP_FLOAT, "NE_OP_FLOAT"},
        {op_binary_type::LT_OP_FLOAT, "LT_OP_FLOAT"},
        {op_binary_type::LE_OP_FLOAT, "LE_OP_FLOAT"},
        {op_binary_type::GT_OP_FLOAT, "GT_OP_FLOAT"},
        {op_binary_type::GE_OP_FLOAT, "GE_OP_FLOAT"},
        {op_binary_type::PLUS_INT, "PLUS_INT"},
        {op_binary_type::MINUS_INT, "MINUS_INT"},
        {op_binary_type::MULT_INT, "MULT_INT"},
        {op_binary_type::DIV_INT, "DIV_INT"},
        {op_binary_type::PLUS_FLOAT, "PLUS_FLOAT"},
        {op_binary_type::MINUS_FLOAT, "MINUS_FLOAT"},
        {op_binary_type::MULT_FLOAT, "MULT_FLOAT"},
        {op_binary_type::DIV_FLOAT, "DIV_FLOAT"}
    };
    
    const std::map<op_unary_type,std::string> op_unary_map = {
        {op_unary_type::TO_INT, "TO_INT"},
        {op_unary_type::TO_FLOAT, "TO_FLOAT"},
        {op_unary_type::UMINUS, "UMINUS"},
        {op_unary_type::NOT, "NOT"},
        {op_unary_type::ADDRESS, "ADDRESS"},
        {op_unary_type::DEREF, "DEREF"},
        {op_unary_type::PP, "PP"}
    };


    class abstract_astnode {
        public:
            virtual ~abstract_astnode() {}
            virtual void print() = 0;
            enum typeExp astnode_type;
    };

    class statement_astnode : public abstract_astnode {
        public:
            virtual ~statement_astnode() {}
    };

    class expression_astnode : public abstract_astnode {
        public:
            virtual ~expression_astnode() {}
            std::string exp_type_;
            bool is_lvalue;
            std::string get_dereferenced_type();
            int cvalue;
    };

    class reference_astnode : public expression_astnode {
        public:
            virtual ~reference_astnode() {}
    };

    class empty_astnode : public statement_astnode {
        public:
            empty_astnode();
            void print();
    };

    class seq_astnode : public statement_astnode {
        private:
            std::vector<statement_astnode*> statements_;
        public:
            seq_astnode();
            void add_statement(statement_astnode* statement);
            void print();
    };

    class assignS_astnode : public statement_astnode {
        private:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
        public:
            assignS_astnode(expression_astnode* expression1, expression_astnode* expression2);
            void print();
    };

    class if_astnode : public statement_astnode {
        private:
            expression_astnode* condition_;
            statement_astnode* then_;
            statement_astnode* else_;
        public:
            if_astnode(expression_astnode* Condition, statement_astnode* Then, statement_astnode* Else);
            void print();
    };

    class while_astnode : public statement_astnode {
        private:
            expression_astnode* condition_;
            statement_astnode* body_;
        public:
            while_astnode(expression_astnode* condition, statement_astnode* body);
            void print();
    };

    class for_astnode : public statement_astnode {
        private:
            expression_astnode* init_;
            expression_astnode* condition_;
            expression_astnode* update_;
            statement_astnode* body_;
        public:
            for_astnode(expression_astnode* init, expression_astnode* condition, expression_astnode* update, statement_astnode* body);
            void print();
    };

    class return_astnode : public statement_astnode {
        private:
            expression_astnode* expression_;
        public:
            return_astnode(expression_astnode* expression);
            void print();
    };

    class proccall_astnode : public statement_astnode {
        private:
            std::string name_;
            std::vector<expression_astnode*> arguments_;
        public:
            proccall_astnode(std::string name);
            void add_argument(expression_astnode* argument);
            void print();
    };
    
    class op_binary_astnode : public expression_astnode {
        private:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
            enum op_binary_type op_;
        public:
            op_binary_astnode(expression_astnode* expression1, expression_astnode* expression2, enum op_binary_type op);
            void print();
    };

    class op_unary_astnode : public expression_astnode {
        private:
            expression_astnode* expression_;
            enum op_unary_type op_;
        public:
            op_unary_astnode(expression_astnode* expression, enum op_unary_type op);
            void print();
    };

    class int_astnode : public expression_astnode {
        private:
            int value_;
        public:
            int_astnode(int value);
            void print();
    };

    class float_astnode : public expression_astnode {
        private:
            float value_;
        public:
            float_astnode(float value);
            void print();
    };

    class string_astnode : public expression_astnode {
        private:
            std::string value_;
        public:
            string_astnode(std::string value);
            void print();
    };

    class assignE_astnode : public expression_astnode {
        private:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
        public:
            assignE_astnode(expression_astnode* expression1, expression_astnode* expression2);
            void print();
            expression_astnode* get_expression1();
            expression_astnode* get_expression2();
    };

    class funcall_astnode : public expression_astnode {
        private:
            std::string name_;
            std::vector<expression_astnode*> arguments_;
        public:
            funcall_astnode(std::string name);
            void add_argument(expression_astnode* argument);
            void print();
    };

    class identifier_astnode : public reference_astnode {
        private:
            std::string name_;
        public:
            identifier_astnode(std::string name);
            void print();
            
    };

    class member_astnode : public reference_astnode {
        private:
            expression_astnode* expression_;
            identifier_astnode* name_;
        public:
            member_astnode(expression_astnode* expression, identifier_astnode* name);
            void print();
    };

    class arrayref_astnode : public reference_astnode {
        private:
            expression_astnode* expression1_;
            expression_astnode* expression2_;
        public:
            arrayref_astnode(expression_astnode* expression1, expression_astnode* expression2);
            void print();
    };

    class arrow_astnode : public reference_astnode {
        private:
            expression_astnode* expression_;
            identifier_astnode* name_;
        public:
            arrow_astnode(expression_astnode* expression, identifier_astnode* name);
            void print();
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

    class LST_Entry
    {
    private:
        std::string name_;
        SymbolType type_;
        Scope scope_;
        std::string sub_type_;
        int size_;
        int offset_;

    public:
        LST_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset);
        std::string get_name();
        SymbolType get_type();
        Scope get_scope();
        std::string get_type_name();
        std::string get_scope_name();
        std::string get_sub_type();
        int get_size();
        int get_offset();
        void set_name(std::string name);
        void set_type(SymbolType type);
        void set_scope(Scope scope);
        void set_sub_type(std::string sub_type);
        void set_size(int size);
        void set_offset(int offset);
        std::string get_dereferenced_type();
    };

    class LST
    {
    private:
        std::vector<LST_Entry *> lst_entries_;

    public:
        LST();
        bool add_lst_entry(LST_Entry *LST_Entry);
        LST_Entry *get_lst_entry_by_name(std::string name);
        int get_size();
        void print();
        std::vector<LST_Entry *> get_lst_entries();


    };


    class GST_Entry
    {
    private:
        std::string name_;
        SymbolType type_;
        Scope scope_;
        std::string sub_type_;
        int size_;
        int offset_;
        LST *lst_;
        abstract_astnode* ast_;

    public:
        GST_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset);
        GST_Entry(std::string name, SymbolType type, Scope scope, std::string sub_type, int size, int offset, LST *lst);
        std::string get_name();
        SymbolType get_type();
        Scope get_scope();
        std::string get_type_name();
        std::string get_scope_name();
        std::string get_sub_type();
        int get_size();
        int get_offset();
        LST *get_lst();
        void set_name(std::string name);
        void set_type(SymbolType type);
        void set_scope(Scope scope);
        void set_sub_type(std::string sub_type);
        void set_size(int size);
        void set_offset(int offset);
        void set_lst(LST *lst);
        void set_ast(abstract_astnode* ast);
        abstract_astnode* get_ast();

    };


    //vector comparator

    

    

    class GST
    {
    private:
        std::vector<GST_Entry *> gst_entries_;
    public:
        GST();
        bool add_GST_entry(GST_Entry *GST_Entry);
        GST_Entry *get_gst_entry_by_name(std::string name);
        void print();
        std::vector<GST_Entry *> get_gst_entries();
        GST_Entry *get_gst_entry_by_lst(LST *lst);

    };

    class Type_Specifier
    {
    private:
        std::string type_;
    
    public: 
        Type_Specifier();
        Type_Specifier(std::string type);
        std::string get_type();
        void set_type(std::string type);
        int get_size(GST *gst);
    };

    class Fun_Declarator
    {
    private:
        std::string name_;
    
    public: 
        Fun_Declarator();
        Fun_Declarator(std::string name);
        void set_name(std::string name);
        std::string get_name();
    };

    class Declaration_List
    {
    private:
        std::vector<LST_Entry *> lst_entries_;
    
    public: 
        Declaration_List();
        bool add_lst_entry(LST_Entry *lst_entry);
        std::vector<LST_Entry *> get_lst_entries();
    };
    
    class Declarator
    {
    private:
        std::string stars_;
        std::string name_;
        int size_;
    
    public: 
        Declarator();
        Declarator(std::string name);
        Declarator(std::string name, int size);
        std::string get_name();
        int get_size();
        std::string get_stars();
        void set_name(std::string name);
        void set_size(int size);
        void set_stars(std::string stars);
    };

    class Declarator_List
    {
    private:
        std::vector<Declarator> declarators_;
    
    public: 
        Declarator_List();
        void add_declarator(Declarator declerator);
        std::vector<Declarator> get_declarators();
    }; 

    class Parameter_List
    {
    private:
        std::stack<LST_Entry *> parameters_;
    
    public: 
        Parameter_List();
        void add_parameter(LST_Entry *parameter);
        LST_Entry * pop();
    };
     
    class Compound_Statement {
        private:
            std::vector<LST_Entry *> lst_entries_;
            abstract_astnode* ast_;
        public:
            Compound_Statement();
            bool add_lst_entry(LST_Entry * lst_entry);
            std::vector<LST_Entry *> get_lst_entries();
            void set_ast(abstract_astnode* ast);
            abstract_astnode* get_ast();
    };

    class Expression_List {
        private:
            std::vector<expression_astnode*> expressions_;
        public:
            Expression_List();
            void add_expression(expression_astnode* expression);
            std::vector<expression_astnode*> get_expressions();
    };
   

}
#endif
