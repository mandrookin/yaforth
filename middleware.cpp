#include "yaforth.h"

static std::vector<record_t>                      constants;
static std::vector<record_t>                      global_variables;
static function_list_t                            global_functions;
static function_t*                                current_function;
static std::vector<record_t>                      builtin_code;

const char*  find_variable_by_address(word_t counter)
{
    const char* name = nullptr;
    for (const auto  & v : global_variables)
    {
        if(counter == v.ADDR)
        {
            name = v.NAME.c_str();
            break;
        }
    }
    return name;
}

bool try_register_local_string(std::string &name, uint32_t value)
{
    bool result = current_function != nullptr;
    if (result) {
        if (current_function->strings.count(name) > 0)
            throw "Duplicated value detected";
        current_function->strings[name] = value;
    }
    return result;
}

function_list_t& get_program()
{
    return global_functions;
}

function_t* lookup_current_function(void)
{
    return current_function;
}

bool reset_current_function()
{
    bool status = current_function != nullptr;
    current_function = nullptr;
    return status;
}

void define_constant(record_t * co) {
    co->LOCAL_INDEX = constants.size();
    constants.push_back(*co);
}

void define_global_variable(record_t* co) {
    co->LOCAL_INDEX = global_variables.size();
    global_variables.push_back(*co);
}

void define_function(record_t* co)
{
    co->LOCAL_INDEX = global_functions.size();
    function_t function;
    function.function = *co;
    global_functions.push_back(function);
    current_function = (function_t*)  &global_functions.back();
}

void register_code(record_t* co) {
    if (co->TYPE == builtin) {
        co->LOCAL_INDEX = builtin_code.size();
        builtin_code.push_back(*co);
    }
    else if (current_function) {
        co->LOCAL_INDEX = current_function->code.size();
        current_function->code.push_back(*co);
    }
    else {
        printf("%s\n", co->NAME.c_str());
    }
}
