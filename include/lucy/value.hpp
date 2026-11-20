#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
namespace lucy {
struct Function; struct Class; struct Instance; struct Environment; struct NativeObject; struct Nil {}; struct Value;
using Array=std::vector<Value>; using Map=std::unordered_map<std::string,Value>;
struct Value {
 using FunctionPtr=std::shared_ptr<Function>; using ClassPtr=std::shared_ptr<Class>; using InstancePtr=std::shared_ptr<Instance>; using MapPtr=std::shared_ptr<Map>; using ArrayPtr=std::shared_ptr<Array>; using NativePtr=std::shared_ptr<NativeObject>;
 std::variant<Nil,bool,long long,double,std::string,ArrayPtr,MapPtr,FunctionPtr,ClassPtr,InstancePtr,NativePtr> data;
 Value():data(Nil{}){} Value(Nil v):data(v){} Value(bool v):data(v){} Value(long long v):data(v){} Value(int v):data((long long)v){} Value(double v):data(v){} Value(std::string v):data(std::move(v)){} Value(const char*v):data(std::string(v)){} Value(Array v):data(std::make_shared<Array>(std::move(v))){} Value(Map v):data(std::make_shared<Map>(std::move(v))){} Value(ArrayPtr v):data(std::move(v)){} Value(MapPtr v):data(std::move(v)){} Value(FunctionPtr v):data(std::move(v)){} Value(ClassPtr v):data(std::move(v)){} Value(InstancePtr v):data(std::move(v)){} Value(NativePtr v):data(std::move(v)){}
 bool is_truthy()const; std::string to_string()const; std::string type_name()const; bool is_number()const;
};
}
