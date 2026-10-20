#include "lucy/value.hpp"
#include "lucy/ast.hpp"
#include <iomanip>
#include <sstream>
#include <unordered_set>
using namespace lucy;
bool Value::is_number()const{return std::holds_alternative<long long>(data)||std::holds_alternative<double>(data);}
bool Value::is_truthy()const{if(std::holds_alternative<Nil>(data))return false;if(auto p=std::get_if<bool>(&data))return *p;if(auto p=std::get_if<long long>(&data))return *p!=0;if(auto p=std::get_if<double>(&data))return *p!=0.0;if(auto p=std::get_if<std::string>(&data))return !p->empty();if(auto p=std::get_if<ArrayPtr>(&data))return p&&!(*p)->empty();if(auto p=std::get_if<MapPtr>(&data))return p&&!(*p)->empty();return true;}
namespace {
std::string value_to_string(const Value &value, std::unordered_set<const void *> &active) {
    if(std::holds_alternative<Nil>(value.data)) return "nil";
    if(auto p=std::get_if<bool>(&value.data)) return *p?"true":"false";
    if(auto p=std::get_if<long long>(&value.data)) return std::to_string(*p);
    if(auto p=std::get_if<double>(&value.data)){std::ostringstream o;o<<std::setprecision(15)<<*p;return o.str();}
    if(auto p=std::get_if<std::string>(&value.data)) return *p;
    if(auto p=std::get_if<Value::ArrayPtr>(&value.data)) {
        if(!*p) return "[]";
        const void *identity=p->get();
        if(active.count(identity)) return "[...]";
        active.insert(identity);
        std::string r="[";
        for(size_t i=0;i<(*p)->size();++i){if(i)r+=", ";r+=value_to_string((*p)->at(i),active);}
        r+="]"; active.erase(identity); return r;
    }
    if(auto p=std::get_if<Value::MapPtr>(&value.data)) {
        if(!*p) return "{}";
        const void *identity=p->get();
        if(active.count(identity)) return "{...}";
        active.insert(identity);
        std::string r="{";size_t i=0;
        for(auto&[k,v]:**p){if(i++)r+=", ";r+=k+": "+value_to_string(v,active);}
        r+="}"; active.erase(identity); return r;
    }
    if(auto p=std::get_if<Value::FunctionPtr>(&value.data)) return "<function "+(*p?(*p)->name:std::string{})+">";
    if(auto p=std::get_if<Value::ClassPtr>(&value.data)) return "<class "+(*p?(*p)->name:std::string{})+">";
    if(auto p=std::get_if<Value::InstancePtr>(&value.data)) return "<"+((*p&&(*p)->klass)?(*p)->klass->name:std::string("Object"))+" instance>";
    if(std::holds_alternative<Value::NativePtr>(value.data)) return "<native>";
    return "<value>";
}
}
std::string Value::to_string()const{std::unordered_set<const void *> active;return value_to_string(*this,active);}
std::string Value::type_name()const{if(std::holds_alternative<Nil>(data))return "nil";if(std::holds_alternative<bool>(data))return "bool";if(std::holds_alternative<long long>(data))return "int";if(std::holds_alternative<double>(data))return "double";if(std::holds_alternative<std::string>(data))return "string";if(std::holds_alternative<ArrayPtr>(data))return "array";if(std::holds_alternative<MapPtr>(data))return "map";if(std::holds_alternative<FunctionPtr>(data))return "function";if(std::holds_alternative<ClassPtr>(data))return "class";if(std::holds_alternative<InstancePtr>(data))return "instance";if(std::holds_alternative<NativePtr>(data))return "native";return "instance";}
