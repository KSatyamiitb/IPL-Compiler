%{
// This part goes directly into the generated parser

#include "scanner.hh"
#include "parser.tab.hh"

// YY_DECL is bound to the type of the scanner. The default is 
// #define YY_DECL int Scanner::yylex()
// which is generated anyway.

//If you want the type of the generated scanner to be different, do this

#undef  YY_DECL
#define YY_DECL int IPL::Scanner::yylex( IPL::Parser::semantic_type * const lval, IPL::Parser::location_type *location )

//We want these to be the first action after a token is recognized. 

#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

// token_type is a short hand for IPL::Parser::token

// using token_type = IPL::Parser::token;
%}


/* %option debug */
%option nodefault
%option yyclass="Scanner"
%option noyywrap
%option c++

%%

%{ /** Code executed at the beginning of yylex **/

      yylval = lval;

%}


[ \t]                            
"\n"  
\/\*.*?\*\/

\"([^\"\\]|\\n|\\t|\\\\|\\\")*\"          {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::STRING_LITERAL;
}

[[:digit:]]+\.[[:digit:]]* {   
       yylval->emplace< std::string >( YYText() );
       return IPL::Parser::token::FLOAT_CONSTANT;
}

[[:digit:]]+\.[[:digit:]]*[eE][+-]?[[:digit:]]+ {   
       yylval->emplace< std::string >( YYText() );
       return IPL::Parser::token::FLOAT_CONSTANT;
}

\.[[:digit:]]+ {   
       yylval->emplace< std::string >( YYText() );
       return IPL::Parser::token::FLOAT_CONSTANT;
}

\.[[:digit:]]+[eE][+-]?[[:digit:]]+ {   
       yylval->emplace< std::string >( YYText() );
       return IPL::Parser::token::FLOAT_CONSTANT;
}

[1-9][[:digit:]]* { 
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::INT_CONSTANT;
}
0[bB][01]* { 
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::INT_CONSTANT;
}
0[0-7]* { 
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::INT_CONSTANT;
}
0[xX][0-9a-fA-F]*  { 
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::INT_CONSTANT;
}

"void"                    {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::VOID;    
} 

"int"                   {   
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::INT;
}

"float"            {   
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::FLOAT;
}

"struct"            {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::STRUCT;
}

"for"            {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::FOR;
}

"return"            {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::RETURN;
}


"while"            {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::WHILE;
}

"if"            {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::IF;
}

"else"            {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::ELSE;
}

[_a-zA-Z][_0-9a-zA-Z]*         {   
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::IDENTIFIER;
}

">="   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::GE_OP;
}

"<="   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::LE_OP;
}

"=="   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::EQ_OP;
}

"!="   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::NE_OP;
}

"++"   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::INC_OP;
}

"&&"   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::AND_OP;
}

"||"   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::OR_OP;
}

"->"   {
      yylval->emplace< std::string >( YYText() );
      return IPL::Parser::token::PTR_OP;
}

"*"   {return '*';}
"/"   {return '/';}
"+"   {return '+';}
"-"   {return '-';}
"="   {return '=';}
">"   {return '>';}
"<"   {return '<';}
"["   {return '[';}
"]"   {return ']';}
"{"   {return '{';}
"}"   {return '}';}
"("   {return '(';}
")"   {return ')';}
"&"   {return '&';}
"."   {return '.';}
","   {return ',';}
";"   {return ';';}
":"   {return ':';}
"!"   {return '!';}
.     {yylval->emplace< std::string >( YYText() );return IPL::Parser::token::OTHERS;}

<<EOF>>         yyterminate();
%%
