#pragma once
#include <stdio.h>
#include <cassert>
#include <stdlib.h>
#include "../list/list.hpp"
#include "../stack/stack.hpp"
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include "table_name.h"
#include "../hash_map/hash_map.hpp"

const long long maximum_of_fractional_part = 16777216; //24 bit meaning 2^24

enum Registers{
    RAX,
    RBX,
    RCX,
    RDX,
    RSP,
    RBP,
    RDI,
    RSI,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15
};


enum Node_type{
    GARBAGE = 0,
    SEPARATOR_OP,
    SEPARATOR_CL,
    END_LINE,
    DECLARATOR,
    VARIABLE,
    ASSIGNMENT,
    LOOP,
    CONDITION,
    ELSE,
    STANDART_FUNCTION,
    READ,
    NUMBER,
    BRACKET_OP,
    BRACKET_CL,
    BINARY_OP,
    LOGIC_OP,
    LOGIC,
    UNARY_OP,
    UNARY,
    RETURN,
    LINKER,
    END,
    COMMA,
    FUNCTION_DECLARATOR,
    FUNCTION_CALL,
    VARIABLE_DECLARATOR,
    WINDOW,
    DRAW,
    ERROR
};

enum Math_operators{
    PLUS =  '+',
    MINUS = '-',
    DIV =   '/',
    MUL =   '*',
    POW =   '^'
};

enum Logical_operators{
    EQUAL       = '=' + '=',
    NOT_EQUAL   = '!' + '=',
    ABOVE       = '>',
    ABOVE_EQUAL = '>' + '=',
    BELOW       = '<',
    BELOW_EQUAL = '<' + '='
};

enum Logic{
    AND,
    OR
};

enum FUNCTIONS{
    WRITE,
    PUT_PIXEL,
    ABS,
    SIN,
    COS,
    SQRT

};

struct Lexer_Node{
    const char* line;
    int str_size;
    Node_type type;
    double value;
};
const int max_buffer_size = 65536;

const int node_types_count = 38;
const Lexer_Node node_types[] = {
    {"("            , 1, BRACKET_OP                     },
    {")"            , 1, BRACKET_CL                     },
    {"{"            , 1, SEPARATOR_OP                   },
    {"}"            , 1, SEPARATOR_CL                   },
    {"Let"          , 3, DECLARATOR                     },
    {";"            , 1, END_LINE                       },
    {"While"        , 5, LOOP                           },
    {"Assuming_that",13, CONDITION                      },
    {"Read"         , 4, READ,                          },
    {"Write"        , 5, STANDART_FUNCTION, WRITE       },
    {"+="           , 2, UNARY_OP         , PLUS        },
    {"-="           , 2, UNARY_OP         , MINUS       },
    {"/="           , 2, UNARY_OP         , DIV         },
    {"*="           , 2, UNARY_OP         , MUL         },
    {"+"            , 1, BINARY_OP        , PLUS        },
    {"-"            , 1, BINARY_OP        , MINUS       },
    {"/"            , 1, BINARY_OP        , DIV         },
    {"*"            , 1, BINARY_OP        , MUL         },
    {"^"            , 1, BINARY_OP        , POW         },
    {"=="           , 2, LOGIC_OP         , EQUAL       },
    {"!="           , 2, LOGIC_OP         , NOT_EQUAL   },
    {"<="           , 2, LOGIC_OP         , BELOW_EQUAL },
    {">="           , 2, LOGIC_OP         , ABOVE_EQUAL },
    {"<"            , 1, LOGIC_OP         , BELOW       },
    {">"            , 1, LOGIC_OP         , ABOVE       },
    {"="            , 1, ASSIGNMENT                     },
    {"Return"       , 6, RETURN                         },
    {"Otherwise"    , 9, ELSE                           },
    {","            , 1, COMMA                          },
    {"&&"           , 2, LOGIC            , AND         },
    {"||"           , 2, LOGIC            , OR          },
    {"Window"       , 6, WINDOW                         },
    {"Pixel"        , 5, STANDART_FUNCTION, PUT_PIXEL   },
    {"Redraw"       , 6, DRAW,                          },
    {"Abs"          , 3, STANDART_FUNCTION, ABS         },
    {"Sin"          , 3, STANDART_FUNCTION, SIN         },
    {"Cos"          , 3, STANDART_FUNCTION, COS         },
    {"Sqrt"         , 4, STANDART_FUNCTION, SQRT        }


};



struct AST_tree{
    struct Tree_Node{
        Node_type type;
        union{
            char* line;
            long long value;
        }u;
        size_t left;
        size_t right;
    };


    struct Error{
        size_t pos;
        char symbol;
    };

    void create_assemble_file(const char* file);

    void JIT_compile();
    void create_nasm_file(const char* file);

    void execute_JIT_compiled_buffer();

    void destructor();
    void lexical_analysis(const char* file);
    void dump_list(const char* file);
    void build_AST();
    void dump_tree(const char* file);
    void save_tree(const char* file);
    void load_tree(const char* file);

    void print_types(){
        for(int i = 1; i <= this->list->size; i++){
            printf("%d ", this->list->get_value_by_index(i).type);
        }
    }



    private:
    List<Tree_Node>* list;
    List<char*>* generated_labels;

    char* jit_buffer;

    struct label_pair{
        const char* label;
        char* RIP;
        size_t op_code_size;
    };

    List<label_pair>* labels_to_fill;

    Hash_map* label_table;

    size_t root;

    size_t curr_label;

    size_t current;

    Name_pages_list* scope;

    Tree_Node get_node(size_t index);

    void do_tree_print(size_t index, FILE* fp);


    bool is_type(Node_type type);

    bool is_value(long long val);

    Node_type get_type();

    long long get_value();

    void require(size_t line, Node_type type);


    void left_connection(size_t son, size_t parent);

    void right_connection(size_t son, size_t parent);

    void change_type(size_t index, Node_type type);

    void change_value(size_t index, long long val);


    size_t Get_grammar();

    size_t Get_programm();

    size_t Get_func_definition();

    size_t Get_func_variables();

    size_t Get_variable();

    size_t Get_variable_declaration();

    size_t Get_operations();

    size_t parse_variable_statement();

    size_t Get_link_statement();

    size_t Get_loop();

    size_t Get_condition();

    size_t copy_with_str(size_t index);

    size_t Get_unary_op();

    size_t Get_assignment();

    size_t Get_standart_function();

    size_t Get_return();

    size_t Get_expression();

    size_t Get_logic_expression();

    size_t Get_part_of_expression();

    size_t Get_term();

    size_t Get_power();

    size_t Get_unar();

    size_t Get_primary_expression();

    size_t Get_read();

    size_t Get_function_call();

    size_t Get_expression_args();

    size_t Get_window();

    size_t Get_draw();


    size_t push_variable(char* curr);

    size_t push_value(char* curr);

    void print_errors(Stack_t<Error>* st);

    size_t file_size(const char* file);

    bool is_garbage(char c);

    size_t var_len(char* line);

    bool isnumber(char* line);
    
    void node_print(FILE* fp, Tree_Node* nd);


//////////////////////////////////////////////////////////////////////////////////////
/////////////////   GENERATE  NASM ASSEMBLER COMMANDS    ///////////////////////////////////

    size_t nasm_generate_code(char* line);

    size_t nasm_generate_default_return(char* line);

    size_t nasm_generate_body_code(size_t index, char* line);

    void nasm_get_function_arguments(size_t index);

    size_t nasm_generate_function_code(size_t index, char* line);

    size_t nasm_generate_statement_code(size_t index, char* line); 

    size_t nasm_generate_expression(size_t index, char* line);

    size_t nasm_generate_assignment(size_t index, char* line);

    size_t nasm_print_logical_operator(size_t index, char* line);

    size_t nasm_print_binary_operator(size_t index, char* line);

    size_t nasm_generate_return(size_t index, char* line);

    size_t nasm_generate_standart_function(size_t index, char* line);

    size_t nasm_generate_write(size_t index, char* line);

    size_t nasm_generate_function_call(size_t index, char* line);

    size_t nasm_generate_func_arguments(size_t index, char* line);

    size_t nasm_generate_variable_declaration(size_t index, char* line);

    size_t nasm_generate_condition(size_t index, char* line);

    size_t nasm_generate_body(size_t index, char* line);

    size_t nasm_generate_loop(size_t index, char* line);

    size_t nasm_print_logic(size_t index, char* line);

    size_t nasm_get_func_argc(size_t index);

    size_t nasm_generate_window(size_t index, char* line);

    size_t nasm_generate_draw(size_t index, char* line);

    size_t nasm_generate_pixel(size_t index, char* line);

    size_t nasm_generate_sin(size_t index, char* line);

    size_t nasm_generate_cos(size_t index, char* line);

    size_t nasm_generate_abs(size_t index, char* line);

    size_t nasm_generate_label(Tree_Node node, char* line);
    
    size_t nasm_generate_sqrt(size_t index, char* line);

//////////////////////////////////////////////////////////////////////////////////////
/////////////////   GENERATE x86 ELF EXECUTABLE    ///////////////////////////////////

    size_t x86_generate_code(char* line);

    size_t x86_generate_default_return(char* line);

    size_t x86_generate_body_code(size_t index, char* line);

    void x86_get_function_arguments(size_t index);

    size_t x86_generate_function_code(size_t index, char* line);

    size_t x86_generate_statement_code(size_t index, char* line); 

    size_t x86_generate_expression(size_t index, char* line);

    size_t x86_generate_assignment(size_t index, char* line);

    size_t x86_print_logical_operator(size_t index, char* line);

    size_t x86_print_binary_operator(size_t index, char* line);

    size_t x86_generate_return(size_t index, char* line);

    size_t x86_generate_standart_function(size_t index, char* line);

    size_t x86_generate_write(size_t index, char* line);

    size_t x86_generate_function_call(size_t index, char* line);

    size_t x86_generate_func_arguments(size_t index, char* line);

    size_t x86_generate_variable_declaration(size_t index, char* line);

    size_t x86_generate_condition(size_t index, char* line);

    size_t x86_generate_body(size_t index, char* line);

    size_t x86_generate_loop(size_t index, char* line);

    size_t x86_print_logic(size_t index, char* line);

    size_t x86_get_func_argc(size_t index);

    size_t x86_generate_window(size_t index, char* line);

    size_t x86_generate_draw(size_t index, char* line);

    size_t x86_generate_pixel(size_t index, char* line);

    size_t x86_generate_sin(size_t index, char* line);

    size_t x86_generate_cos(size_t index, char* line);

    size_t x86_generate_abs(size_t index, char* line);

    void x86_generate_label(Tree_Node node, char* line);

    void x86_fill_labels();

    void fill_x_bytes(int x, long long a, char* line);

    size_t x86_load_printf(char* line);

    size_t x86_load_scanf(char* line);


    size_t x86_gen_push(char* line, Registers reg);

    size_t x86_gen_pop(char* line, Registers reg);

    size_t x86_gen_ret(char* line);

    size_t x86_gen_call(char* line);

    size_t x86_gen_mov_rbp_rsp(char* line);//    = mov rbp, rsp

    size_t x86_gen_mov_rsp_rbp(char* line);//    = mov rsp, rbp

    size_t x86_gen_mov_var_rax(char* line, int var_offset);//    = mov [rbp + var_offset], rax

    size_t x86_gen_sub_rsp_8(char*line);

    size_t x86_gen_mov_rax_const(char* line, long long value);

    size_t x86_gen_mov_rdi_rax(char* line);

};




