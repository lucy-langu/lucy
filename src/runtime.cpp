#include "lucy/runtime.hpp"
#include "lucy/lexer.hpp"
#include "lucy/parser.hpp"
#include "lucy/phase2.hpp"
#include "lucy/repl.hpp"
#include "lucy/stdlib_native.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <thread>
#include <set>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef _WIN32
#include <cstdio>
#define LUCY_POPEN _popen
#define LUCY_PCLOSE _pclose
#else
#include <cstdio>
#define LUCY_POPEN popen
#define LUCY_PCLOSE pclose
#endif
using namespace lucy;
namespace fs = std::filesystem;
void Environment::define(const std::string &n, Value v, bool c) { values_[n] = {std::move(v), c}; }
bool Environment::local(const std::string &n) const { return values_.count(n) != 0; }
bool Environment::constant(const std::string &n) const
{
    auto i = values_.find(n);
    return i != values_.end() && i->second.constant;
}
bool Environment::assign(const std::string &n, Value v)
{
    auto i = values_.find(n);
    if (i != values_.end())
    {
        if (i->second.constant)
            throw std::runtime_error("NameError: cannot assign to constant '" + n + "'");
        i->second.value = std::move(v);
        return true;
    }
    return parent_ && parent_->assign(n, std::move(v));
}
Value Environment::get(const std::string &n) const
{
    auto i = values_.find(n);
    if (i != values_.end())
        return i->second.value;
    if (parent_)
        return parent_->get(n);
    throw std::runtime_error("NameError: undefined variable '" + n + "'");
}
std::shared_ptr<Environment> Environment::root()
{
    auto r = shared_from_this();
    while (r->parent_)
        r = r->parent_;
    return r;
}
std::shared_ptr<Environment> Interpreter::make_environment(std::shared_ptr<Environment> parent)
{
    auto e = std::make_shared<Environment>(std::move(parent));
    environments_.push_back(e);
    return e;
}
static void need(size_t got, size_t n, const std::string &name)
{
    if (got != n)
        throw std::runtime_error("ArgumentError: " + name + " expects " + std::to_string(n) + " argument(s), got " + std::to_string(got));
}
static double num(const Value &v, const std::string &op)
{
    if (auto p = std::get_if<long long>(&v.data))
        return (double)*p;
    if (auto p = std::get_if<double>(&v.data))
        return *p;
    throw std::runtime_error("TypeError: operator '" + op + "' requires a number, got " + v.type_name());
}
static long long integer(const Value &v, const std::string &op)
{
    if (auto p = std::get_if<long long>(&v.data))
        return *p;
    if (auto p = std::get_if<double>(&v.data))
        return (long long)*p;
    throw std::runtime_error("TypeError: operator '" + op + "' requires an integer, got " + v.type_name());
}
Interpreter::~Interpreter()
{
    for (auto &environment : environments_)
        environment->clear();
    environments_.clear();
    globals_.reset();
    env_.reset();
}
Interpreter::Interpreter(std::vector<std::string> argv)
    : globals_(nullptr), env_(nullptr)
{
    globals_ = make_environment(nullptr);
    env_ = globals_;

    install_builtins();

    globals_->define("PI", 3.141592653589793, true);
    globals_->define("E", 2.718281828459045, true);
    globals_->define("VERSION", "1.0.1", true);

#ifdef _WIN32
    globals_->define("PLATFORM", std::string("windows"), true);
#elif __APPLE__
    globals_->define("PLATFORM", std::string("macos"), true);
#elif __linux__
    globals_->define("PLATFORM", std::string("linux"), true);
#else
    globals_->define("PLATFORM", std::string("unix"), true);
#endif

    // ARGV follows Ruby's convention: it contains arguments after the source file.
    auto arguments = std::make_shared<Array>();
    for (const auto &argument : argv)
    {
        arguments->emplace_back(argument);
    }

    // Keep both spellings backed by the same mutable array.
    globals_->define("ARGV", Value(arguments), true);
    globals_->define("argv", Value(arguments), true);
}
void Interpreter::install_builtins()
{
    builtins_["print"] = [&](const std::vector<Value> &a)
    {for(size_t i=0;i<a.size();++i){if(i)std::cout<<' ';std::cout<<a[i].to_string();}return Value{}; };
    builtins_["echo"] = [&](const std::vector<Value> &a)
    {for(size_t i=0;i<a.size();++i){if(i)std::cout<<' ';std::cout<<a[i].to_string();}std::cout<<'\n';return Value{}; };
    builtins_["input"] = [](const std::vector<Value> &a)
    {if(a.size()>1)throw std::runtime_error("ArgumentError: input expects 0 or 1 argument(s)");if(!a.empty())std::cout<<a[0].to_string();std::string s;std::getline(std::cin,s);return Value(s); };
    builtins_["len"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"len");if(auto p=std::get_if<std::string>(&a[0].data))return Value((long long)p->size());if(auto p=std::get_if<Value::ArrayPtr>(&a[0].data))return Value((long long)(*p)->size());if(auto p=std::get_if<Value::MapPtr>(&a[0].data))return Value((long long)(*p)->size());throw std::runtime_error("TypeError: len expects string, array, or map"); };
    builtins_["str"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"str");return Value(a[0].to_string()); };
    builtins_["int"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"int");if(auto p=std::get_if<long long>(&a[0].data))return Value(*p);if(auto p=std::get_if<double>(&a[0].data))return Value((long long)*p);if(auto p=std::get_if<std::string>(&a[0].data))try{return Value(std::stoll(*p));}catch(...){throw std::runtime_error("ValueError: cannot convert '"+*p+"' to int");}throw std::runtime_error("TypeError: int expects a number or numeric string"); };
    builtins_["float"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"float");if(auto p=std::get_if<double>(&a[0].data))return Value(*p);if(auto p=std::get_if<long long>(&a[0].data))return Value((double)*p);if(auto p=std::get_if<std::string>(&a[0].data))try{return Value(std::stod(*p));}catch(...){throw std::runtime_error("ValueError: cannot convert '"+*p+"' to float");}throw std::runtime_error("TypeError: float expects a number or numeric string"); };
    builtins_["type"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"type");return Value(a[0].type_name()); };
    builtins_["typeof"] = builtins_["type"];
    builtins_["range"] = [](const std::vector<Value> &a)
    {if(a.empty()||a.size()>3)throw std::runtime_error("ArgumentError: range expects 1 to 3 arguments");long long start=0,stop=0,step=1;if(a.size()==1)stop=integer(a[0],"range");else{start=integer(a[0],"range");stop=integer(a[1],"range");if(a.size()==3)step=integer(a[2],"range");}if(step==0)throw std::runtime_error("ValueError: range step cannot be zero");Array r;if(step>0)for(long long i=start;i<stop;i+=step)r.emplace_back(i);else for(long long i=start;i>stop;i+=step)r.emplace_back(i);return Value(std::move(r)); };
    builtins_["sum"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"sum");auto p=std::get_if<Value::ArrayPtr>(&a[0].data);if(!p)throw std::runtime_error("TypeError: sum expects an array");double r=0;for(auto&v:**p)r+=num(v,"sum");return Value(r); };
    builtins_["min"] = [](const std::vector<Value> &a)
    {if(a.empty())throw std::runtime_error("ArgumentError: min expects at least one argument");double r=num(a[0],"min");for(size_t i=1;i<a.size();++i)r=std::min(r,num(a[i],"min"));return Value(r); };
    builtins_["max"] = [](const std::vector<Value> &a)
    {if(a.empty())throw std::runtime_error("ArgumentError: max expects at least one argument");double r=num(a[0],"max");for(size_t i=1;i<a.size();++i)r=std::max(r,num(a[i],"max"));return Value(r); };
    builtins_["abs"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"abs");return Value(std::fabs(num(a[0],"abs"))); };
    builtins_["sqrt"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"sqrt");double x=num(a[0],"sqrt");if(x<0)throw std::runtime_error("ValueError: sqrt domain error");return Value(std::sqrt(x)); };
    for (auto [name, fn] : std::vector<std::pair<std::string, double (*)(double)>>{{"sin", std::sin}, {"cos", std::cos}, {"tan", std::tan}, {"exp", std::exp}, {"floor", std::floor}, {"ceil", std::ceil}})
        builtins_[name] = [fn, name](const std::vector<Value> &a)
        {need(a.size(),1,name);return Value(fn(num(a[0],name))); };
    builtins_["log"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"log");double x=num(a[0],"log");if(x<=0)throw std::runtime_error("ValueError: log domain error");return Value(std::log(x)); };
    builtins_["log10"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"log10");double x=num(a[0],"log10");if(x<=0)throw std::runtime_error("ValueError: log10 domain error");return Value(std::log10(x)); };
    builtins_["exp"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"exp");return Value(std::exp(num(a[0],"exp"))); };
    builtins_["pow"] = [](const std::vector<Value> &a)
    {need(a.size(),2,"pow");return Value(std::pow(num(a[0],"pow"),num(a[1],"pow"))); };
    builtins_["assert"] = [this](const std::vector<Value> &a)
    {if(a.empty()||a.size()>2)throw std::runtime_error("ArgumentError: assert expects 1 or 2 arguments");if(!a[0].is_truthy())throw std::runtime_error("AssertionError: "+(a.size()==2?a[1].to_string():"assertion failed"));return Value{}; };
    builtins_["read_file"] = [this](const std::vector<Value> &a)
    {need(a.size(),1,"read_file");return Value(read_file(std::get<std::string>(a[0].data))); };
    builtins_["write_file"] = [this](const std::vector<Value> &a)
    {need(a.size(),2,"write_file");std::ofstream f(std::get<std::string>(a[0].data));if(!f)throw std::runtime_error("IOError: cannot write file");f<<std::get<std::string>(a[1].data);return Value{}; };
    builtins_["exists"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"exists");return Value(fs::exists(std::get<std::string>(a[0].data))); };
    builtins_["cwd"] = [](const std::vector<Value> &a)
    {need(a.size(),0,"cwd");return Value(fs::current_path().string()); };
    builtins_["getenv"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"getenv");const char*p=std::getenv(std::get<std::string>(a[0].data).c_str());return p?Value(std::string(p)):Value{}; };
    builtins_["sleep"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"sleep");std::this_thread::sleep_for(std::chrono::milliseconds(integer(a[0],"sleep")));return Value{}; };
    builtins_["millis"] = [](const std::vector<Value> &a)
    {need(a.size(),0,"millis");static auto start=std::chrono::steady_clock::now();return Value((long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()); };
    builtins_["__io_write"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"IO.write");std::cout<<a[0].to_string();return a[0]; };
    builtins_["__io_read"] = [](const std::vector<Value> &a)
    {if(a.size()>1)throw std::runtime_error("ArgumentError: IO.read expects 0 or 1 argument");if(!a.empty())std::cout<<a[0].to_string();std::string s;std::getline(std::cin,s);return Value(s); };
    builtins_["__file_read"] = [this](const std::vector<Value> &a)
    {need(a.size(),1,"File.read");return Value(read_file(std::get<std::string>(a[0].data))); };
    builtins_["__file_write"] = [this](const std::vector<Value> &a)
    {need(a.size(),2,"File.write");std::ofstream f(std::get<std::string>(a[0].data),std::ios::binary|std::ios::trunc);if(!f)throw std::runtime_error("IOError: cannot write file '"+std::get<std::string>(a[0].data)+"'");f<<std::get<std::string>(a[1].data);return Value(true); };
    builtins_["__file_append"] = [this](const std::vector<Value> &a)
    {need(a.size(),2,"File.append");std::ofstream f(std::get<std::string>(a[0].data),std::ios::binary|std::ios::app);if(!f)throw std::runtime_error("IOError: cannot append file '"+std::get<std::string>(a[0].data)+"'");f<<std::get<std::string>(a[1].data);return Value(true); };
    builtins_["__file_exists"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"File.exists");return Value(fs::exists(std::get<std::string>(a[0].data))&&fs::is_regular_file(std::get<std::string>(a[0].data))); };
    builtins_["__file_size"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"File.size");auto p=std::get<std::string>(a[0].data);if(!fs::is_regular_file(p))throw std::runtime_error("IOError: not a regular file: '"+p+"'");return Value((long long)fs::file_size(p)); };
    builtins_["__file_delete"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"File.delete");auto p=std::get<std::string>(a[0].data);if(!fs::exists(p))return Value(false);if(!fs::remove(p))throw std::runtime_error("IOError: cannot delete file '"+p+"'");return Value(true); };
    builtins_["__file_lines"] = [this](const std::vector<Value> &a)
    {need(a.size(),1,"File.lines");std::istringstream in(read_file(std::get<std::string>(a[0].data)));Array r;std::string line;while(std::getline(in,line))r.emplace_back(line);return Value(std::move(r)); };
    builtins_["__file_copy"] = [](const std::vector<Value> &a)
    {need(a.size(),2,"File.copy");auto s=std::get<std::string>(a[0].data),d=std::get<std::string>(a[1].data);std::error_code ec;fs::copy_file(s,d,fs::copy_options::overwrite_existing,ec);if(ec)throw std::runtime_error("IOError: cannot copy file: "+ec.message());return Value(d); };
    builtins_["__file_move"] = [](const std::vector<Value> &a)
    {need(a.size(),2,"File.move");auto s=std::get<std::string>(a[0].data),d=std::get<std::string>(a[1].data);std::error_code ec;fs::rename(s,d,ec);if(ec)throw std::runtime_error("IOError: cannot move file: "+ec.message());return Value(d); };
    builtins_["__file_touch"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"File.touch");auto p=std::get<std::string>(a[0].data);if(!fs::exists(p)){std::ofstream f(p,std::ios::binary);if(!f)throw std::runtime_error("IOError: cannot create file '"+p+"'");}else{auto now=fs::file_time_type::clock::now();std::error_code ec;fs::last_write_time(p,now,ec);if(ec)throw std::runtime_error("IOError: cannot touch file: "+ec.message());}return Value(true); };
    builtins_["__file_write_lines"] = [this](const std::vector<Value> &a)
    {need(a.size(),2,"File.write_lines");auto p=std::get<std::string>(a[0].data);auto ap=std::get_if<Value::ArrayPtr>(&a[1].data);if(!ap)throw std::runtime_error("TypeError: File.write_lines expects an array");std::ofstream f(p,std::ios::binary|std::ios::trunc);if(!f)throw std::runtime_error("IOError: cannot write file '"+p+"'");for(size_t i=0;i<(*ap)->size();++i){if(i)f<<'\n';f<<(*ap)->at(i).to_string();}return Value(true); };

    builtins_["modules_info"] = [this](const std::vector<Value> &a)
    {
        need(a.size(), 0, "modules_info");

        auto modules = list_modules();
        Map detailed;
        for (const auto &[name, path] : modules)
        {
            detailed[name] = Value(path);
        }
        return Value(std::move(detailed));
    };
    builtins_["__path_join"] = [](const std::vector<Value> &a)
    {
        if (a.size() != 1)
            throw std::runtime_error("ArgumentError: Path.join expects one array of path parts");
        auto ap = std::get_if<Value::ArrayPtr>(&a[0].data);
        if (!ap)
            throw std::runtime_error("TypeError: Path.join expects an array");
        fs::path p;
        for (auto &v : **ap)
            p /= std::get<std::string>(v.data);
        return Value(p.lexically_normal().string());
    };
    builtins_["__path_absolute"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Path.absolute");return Value(fs::absolute(std::get<std::string>(a[0].data)).lexically_normal().string()); };
    builtins_["__path_expand"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 1, "Path.expand");
        std::string p = std::get<std::string>(a[0].data);
        if (!p.empty() && p[0] == '~')
        {
            const char *h = std::getenv("HOME");
#ifdef _WIN32
            if (!h)
                h = std::getenv("USERPROFILE");
#endif
            if (h)
                p = std::string(h) + p.substr(1);
        }
        return Value(fs::path(p).lexically_normal().string());
    };
    builtins_["__path_basename"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Path.basename");return Value(fs::path(std::get<std::string>(a[0].data)).filename().string()); };
    builtins_["__path_dirname"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Path.dirname");return Value(fs::path(std::get<std::string>(a[0].data)).parent_path().string()); };
    builtins_["__path_extname"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Path.extname");return Value(fs::path(std::get<std::string>(a[0].data)).extension().string()); };
    builtins_["__path_stem"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Path.stem");return Value(fs::path(std::get<std::string>(a[0].data)).stem().string()); };
    builtins_["__dir_pwd"] = [](const std::vector<Value> &a)
    {need(a.size(),0,"Dir.pwd");return Value(fs::current_path().string()); };
    builtins_["__dir_chdir"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.chdir");std::error_code ec;fs::current_path(std::get<std::string>(a[0].data),ec);if(ec)throw std::runtime_error("IOError: cannot change directory: "+ec.message());return Value(fs::current_path().string()); };
    builtins_["__dir_exists"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.exists");auto p=std::get<std::string>(a[0].data);return Value(fs::exists(p)&&fs::is_directory(p)); };
    builtins_["__dir_entries"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.entries");std::string p=std::get<std::string>(a[0].data);if(!fs::is_directory(p))throw std::runtime_error("IOError: not a directory: '"+p+"'");Array r;for(auto&e:fs::directory_iterator(p))r.emplace_back(e.path().filename().string());return Value(std::move(r)); };
    builtins_["__dir_files"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.files");std::string p=std::get<std::string>(a[0].data);if(!fs::is_directory(p))throw std::runtime_error("IOError: not a directory: '"+p+"'");Array r;for(auto&e:fs::directory_iterator(p))if(e.is_regular_file())r.emplace_back(e.path().filename().string());return Value(std::move(r)); };
    builtins_["__dir_dirs"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.dirs");std::string p=std::get<std::string>(a[0].data);if(!fs::is_directory(p))throw std::runtime_error("IOError: not a directory: '"+p+"'");Array r;for(auto&e:fs::directory_iterator(p))if(e.is_directory())r.emplace_back(e.path().filename().string());return Value(std::move(r)); };
    builtins_["__dir_mkdir"] = [](const std::vector<Value> &a)
    {if(a.empty()||a.size()>2)throw std::runtime_error("ArgumentError: Dir.mkdir expects 1 or 2 arguments");auto p=std::get<std::string>(a[0].data);bool parents=a.size()==2&&a[1].is_truthy();std::error_code ec;bool ok=parents?fs::create_directories(p,ec):fs::create_directory(p,ec);if(ec)throw std::runtime_error("IOError: cannot create directory: "+ec.message());return Value(ok||fs::is_directory(p)); };
    builtins_["__dir_rmdir"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.rmdir");auto p=std::get<std::string>(a[0].data);std::error_code ec;auto n=fs::remove_all(p,ec);if(ec)throw std::runtime_error("IOError: cannot remove directory: "+ec.message());return Value((long long)n); };
    builtins_["__dir_empty"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.empty?");auto p=std::get<std::string>(a[0].data);if(!fs::is_directory(p))throw std::runtime_error("IOError: not a directory: '"+p+"'");return Value(fs::directory_iterator(p)==fs::directory_iterator{}); };
    builtins_["__dir_glob"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.glob");auto pattern=std::get<std::string>(a[0].data);fs::path pp(pattern);fs::path base=pp.parent_path();if(base.empty())base=".";std::string name=pp.filename().string();std::string re="^";for(char c:name){if(c=='*')re+=".*";else if(c=='?')re+='.';else if(std::string(".^$|()[]{}+\\").find(c)!=std::string::npos){re+='\\';re+=c;}else re+=c;}re+="$";std::regex rx(re);Array r;if(fs::is_directory(base))for(auto&e:fs::directory_iterator(base))if(std::regex_match(e.path().filename().string(),rx))r.emplace_back(e.path().lexically_normal().string());return Value(std::move(r)); };
    builtins_["__dir_walk"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"Dir.walk");auto p=std::get<std::string>(a[0].data);if(!fs::is_directory(p))throw std::runtime_error("IOError: not a directory: '"+p+"'");Array r;for(auto&e:fs::recursive_directory_iterator(p))r.emplace_back(e.path().lexically_normal().string());return Value(std::move(r)); };
    builtins_["__dir_copy"] = [](const std::vector<Value> &a)
    {need(a.size(),2,"Dir.copy");auto s=std::get<std::string>(a[0].data),d=std::get<std::string>(a[1].data);std::error_code ec;fs::copy(s,d,fs::copy_options::recursive|fs::copy_options::overwrite_existing,ec);if(ec)throw std::runtime_error("IOError: cannot copy directory: "+ec.message());return Value(d); };

    builtins_["__os_env"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"OS.env");const char*p=std::getenv(std::get<std::string>(a[0].data).c_str());return p?Value(std::string(p)):Value{}; };
    builtins_["__os_setenv"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 2, "OS.setenv");
        auto k = std::get<std::string>(a[0].data), v = std::get<std::string>(a[1].data);
#ifdef _WIN32
        if (_putenv_s(k.c_str(), v.c_str()) != 0)
            throw std::runtime_error("IOError: cannot set environment variable");
#else
        if (setenv(k.c_str(), v.c_str(), 1) != 0)
            throw std::runtime_error("IOError: cannot set environment variable");
#endif
        return Value(true);
    };
    builtins_["__os_unsetenv"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 1, "OS.unsetenv");
        auto k = std::get<std::string>(a[0].data);
#ifdef _WIN32
        if (_putenv_s(k.c_str(), "") != 0)
            throw std::runtime_error("IOError: cannot unset environment variable");
#else
        if (unsetenv(k.c_str()) != 0)
            throw std::runtime_error("IOError: cannot unset environment variable");
#endif
        return Value(true);
    };
    builtins_["__os_system"] = [](const std::vector<Value> &a)
    {need(a.size(),1,"OS.system");return Value((long long)std::system(std::get<std::string>(a[0].data).c_str())); };
    builtins_["__os_pid"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 0, "OS.pid");
#ifdef _WIN32
        return Value((long long)GetCurrentProcessId());
#else
        return Value((long long)getpid());
#endif
    };
    builtins_["__os_home"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 0, "OS.home");
        const char *h = std::getenv("HOME");
#ifdef _WIN32
        if (!h)
            h = std::getenv("USERPROFILE");
#endif
        return h ? Value(std::string(h)) : Value{};
    };
    builtins_["__os_temp_dir"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 0, "OS.temp_dir");
        const char *t = std::getenv("TMPDIR");
#ifdef _WIN32
        if (!t)
            t = std::getenv("TEMP");
#endif
        return t ? Value(std::string(t)) : Value(fs::temp_directory_path().string());
    };
    builtins_["__os_command_exists"] = [](const std::vector<Value> &a)
    {
        need(a.size(), 1, "OS.command_exists");
        std::string cmd = std::get<std::string>(a[0].data);
        const char *path = std::getenv("PATH");
        if (!path)
            return Value(false);
        char sep = ':';
#ifdef _WIN32
        sep = ';';
#endif
        std::stringstream ss(path);
        std::string d;
        while (std::getline(ss, d, sep))
        {
            if (d.empty())
                d = ".";
            fs::path q = fs::path(d) / cmd;
            if (fs::exists(q) && fs::is_regular_file(q))
                return Value(true);
#ifdef _WIN32
            if (fs::exists(q.string() + ".exe"))
                return Value(true);
#endif
        }
        return Value(false);
    };
    builtins_["__os_cpu_count"] = [](const std::vector<Value> &a)
    {need(a.size(),0,"OS.cpu_count");auto n=std::thread::hardware_concurrency();return Value((long long)n); };
    builtins_["random_int"] = [](const std::vector<Value> &a)
    {need(a.size(),2,"random_int");static std::mt19937_64 g(std::random_device{}());return Value((long long)std::uniform_int_distribution<long long>(integer(a[0],"random_int"),integer(a[1],"random_int"))(g)); };
    builtins_["__kernel_send"] = [this](const std::vector<Value> &a)
    {if(a.size()<2||a.size()>3)throw std::runtime_error("ArgumentError: Kernel.send expects 2 or 3 arguments");std::string name=std::get<std::string>(a[1].data);Value member=member_get(a[0],name);std::vector<Value>args;if(a.size()==3&&std::holds_alternative<Value::ArrayPtr>(a[2].data)){args=*std::get<Value::ArrayPtr>(a[2].data);}else args=std::vector<Value>(a.begin()+2,a.end());return call(member,args); };
    builtins_["__kernel_respond_to"] = [this](const std::vector<Value> &a)
    {need(a.size(),2,"Kernel.respond_to");std::string name=std::get<std::string>(a[1].data);try{member_get(a[0],name);return Value(true);}catch(...){return Value(false);} };
    builtins_["__kernel_methods"] = [this](const std::vector<Value> &a)
    {need(a.size(),1,"Kernel.methods");Array result;if(auto i=std::get_if<Value::InstancePtr>(&a[0].data)){for(const auto&[name,value]:(*i)->fields)result.emplace_back(name);for(auto k=(*i)->klass;k;k=k->parent)for(const auto&[name,value]:k->methods)result.emplace_back(name);}else if(auto m=std::get_if<Value::MapPtr>(&a[0].data)){for(const auto&[name,value]:**m)result.emplace_back(name);}return Value(std::move(result)); };
    builtins_["__kernel_ancestors"] = [this](const std::vector<Value> &a)
    {need(a.size(),1,"Kernel.ancestors");Array result;if(auto i=std::get_if<Value::InstancePtr>(&a[0].data)){for(auto k=(*i)->klass;k;k=k->parent)result.emplace_back(k->name);}else if(auto c=std::get_if<Value::ClassPtr>(&a[0].data)){for(auto k=*c;k;k=k->parent)result.emplace_back(k->name);}return Value(std::move(result)); };
    builtins_["__kernel_superclass"] = [this](const std::vector<Value> &a)
    {need(a.size(),1,"Kernel.superclass");if(auto i=std::get_if<Value::InstancePtr>(&a[0].data)){auto p=(*i)->klass->parent;return p?Value(p->name):Value{};}if(auto c=std::get_if<Value::ClassPtr>(&a[0].data)){return (*c)->parent?Value((*c)->parent->name):Value{};}return Value{}; };
    builtins_["__kernel_global_variables"] = [this](const std::vector<Value> &)
    {Array result;for(const auto&[name,entry]:globals_->values())result.emplace_back(name);return Value(std::move(result)); };
    builtins_["__kernel_local_variables"] = [this](const std::vector<Value> &)
    {Array result;for(const auto&[name,entry]:env_->values())result.emplace_back(name);return Value(std::move(result)); };
    install_phase2_builtins(builtins_);
    install_extended_stdlib_builtins(builtins_);
}
bool Interpreter::equal(const Value &a, const Value &b) const
{
    if (a.type_name() != b.type_name() && a.is_number() && b.is_number())
        return num(a, "==") == num(b, "==");
    if (a.data.index() != b.data.index())
        return false;
    if (std::holds_alternative<Nil>(a.data))
        return true;
    if (auto x = std::get_if<bool>(&a.data))
        return *x == std::get<bool>(b.data);
    if (auto x = std::get_if<long long>(&a.data))
        return *x == std::get<long long>(b.data);
    if (auto x = std::get_if<double>(&a.data))
        return *x == std::get<double>(b.data);
    if (auto x = std::get_if<std::string>(&a.data))
        return *x == std::get<std::string>(b.data);
    return a.to_string() == b.to_string();
}
Value Interpreter::compound(const Value &a, const Token &o, const Value &b)
{
    Token q = o;
    if (q.type == TokenType::PlusEqual)
        q.type = TokenType::Plus;
    if (q.type == TokenType::MinusEqual)
        q.type = TokenType::Minus;
    if (q.type == TokenType::StarEqual)
        q.type = TokenType::Star;
    if (q.type == TokenType::SlashEqual)
        q.type = TokenType::Slash;
    if (q.type == TokenType::PercentEqual)
        q.type = TokenType::Percent;
    if (q.type == TokenType::PowerEqual)
        q.type = TokenType::Power;
    if (q.type == TokenType::BitAndEqual)
        q.type = TokenType::BitAnd;
    if (q.type == TokenType::BitOrEqual)
        q.type = TokenType::BitOr;
    if (q.type == TokenType::BitXorEqual)
        q.type = TokenType::BitXor;
    if (q.type == TokenType::ShiftLeftEqual)
        q.type = TokenType::ShiftLeft;
    if (q.type == TokenType::ShiftRightEqual)
        q.type = TokenType::ShiftRight;
    return binary(a, q, b);
}
Value Interpreter::binary(const Value &a, const Token &o, const Value &b)
{
    if (auto ai = std::get_if<Value::InstancePtr>(&a.data))
    {
        const std::string cls = (*ai)->klass ? (*ai)->klass->name : "";
        if (cls == "Time" || cls == "Date")
        {
            auto ts = (*ai)->fields.find("timestamp");
            if (ts != (*ai)->fields.end())
            {
                long long left = integer(ts->second, o.lexeme);
                if (o.type == TokenType::Plus || o.type == TokenType::Minus)
                {
                    if (b.is_number())
                    {
                        long long delta = integer(b, o.lexeme) * (cls == "Date" ? 86400000LL : 1LL);
                        return call(member_get(a, o.type == TokenType::Plus ? "add" : "subtract"), {Value(delta)});
                    }
                    if (o.type == TokenType::Minus)
                    {
                        if (auto bi = std::get_if<Value::InstancePtr>(&b.data))
                        {
                            auto bt = (*bi)->fields.find("timestamp");
                            if (bt != (*bi)->fields.end() && (*bi)->klass->name == cls)
                                return Value(left - integer(bt->second, "-"));
                        }
                    }
                }
                if (o.type == TokenType::Spaceship)
                {
                    if (auto bi = std::get_if<Value::InstancePtr>(&b.data))
                    {
                        auto bt = (*bi)->fields.find("timestamp");
                        if (bt != (*bi)->fields.end() && (*bi)->klass->name == cls)
                        {
                            long long right = integer(bt->second, "<=>");
                            return Value(left < right ? -1 : left > right ? 1
                                                                          : 0);
                        }
                    }
                }
            }
        }
        if (cls == "Set")
        {
            if (o.type == TokenType::BitOr)
                return call(member_get(a, "union"), {b});
            if (o.type == TokenType::BitAnd)
                return call(member_get(a, "intersection"), {b});
            if (o.type == TokenType::BitXor)
                return call(member_get(a, "symmetric_difference"), {b});
            if (o.type == TokenType::Minus)
                return call(member_get(a, "difference"), {b});
        }
    }
    if (o.type == TokenType::Plus)
    {
        if (auto x = std::get_if<std::string>(&a.data))
            return Value(*x + b.to_string());
        if (auto x = std::get_if<Value::ArrayPtr>(&a.data))
        {
            auto r = **x;
            if (auto y = std::get_if<Value::ArrayPtr>(&b.data))
                r.insert(r.end(), (*y)->begin(), (*y)->end());
            else
                r.push_back(b);
            return Value(std::move(r));
        }
        if (a.is_number() && b.is_number())
        {
            double r = num(a, "+") + num(b, "+");
            if (std::holds_alternative<long long>(a.data) && std::holds_alternative<long long>(b.data))
                return Value((long long)r);
            return Value(r);
        }
    }
    if (o.type == TokenType::Minus || o.type == TokenType::Star || o.type == TokenType::Slash || o.type == TokenType::Power)
    {
        double x = num(a, o.lexeme), y = num(b, o.lexeme);
        if (o.type == TokenType::Slash && y == 0)
            throw std::runtime_error("ZeroDivisionError: division by zero");
        double r = o.type == TokenType::Minus ? x - y : o.type == TokenType::Star ? x * y
                                                    : o.type == TokenType::Slash  ? x / y
                                                                                  : std::pow(x, y);
        if (std::holds_alternative<long long>(a.data) && std::holds_alternative<long long>(b.data) && o.type != TokenType::Slash && o.type != TokenType::Power)
            return Value((long long)r);
        return Value(r);
    }
    if (o.type == TokenType::Percent)
    {
        long long y = integer(b, "%");
        if (y == 0)
            throw std::runtime_error("ZeroDivisionError: modulo by zero");
        return Value(integer(a, "%") % y);
    }
    if (o.type == TokenType::EqualEqual)
        return Value(equal(a, b));
    if (o.type == TokenType::BangEqual)
        return Value(!equal(a, b));
    if (o.type == TokenType::StrictEqual)
        return Value(a.type_name() == b.type_name() && equal(a, b));
    if (o.type == TokenType::StrictNotEqual)
        return Value(!(a.type_name() == b.type_name() && equal(a, b)));
    if (o.type == TokenType::Spaceship)
        throw std::runtime_error("TypeError: <=> requires comparable values");
    if (o.type == TokenType::Greater || o.type == TokenType::GreaterEqual || o.type == TokenType::Less || o.type == TokenType::LessEqual)
    {
        if (auto x = std::get_if<std::string>(&a.data))
        {
            auto y = std::get_if<std::string>(&b.data);
            if (!y)
                throw std::runtime_error("TypeError: incompatible comparison types");
            if (o.type == TokenType::Greater)
                return Value(*x > *y);
            if (o.type == TokenType::GreaterEqual)
                return Value(*x >= *y);
            if (o.type == TokenType::Less)
                return Value(*x < *y);
            return Value(*x <= *y);
        }
        double x = num(a, o.lexeme), y = num(b, o.lexeme);
        if (o.type == TokenType::Greater)
            return Value(x > y);
        if (o.type == TokenType::GreaterEqual)
            return Value(x >= y);
        if (o.type == TokenType::Less)
            return Value(x < y);
        return Value(x <= y);
    }
    if (o.type == TokenType::BitAnd)
        return Value(integer(a, "&") & integer(b, "&"));
    if (o.type == TokenType::BitOr)
        return Value(integer(a, "|") | integer(b, "|"));
    if (o.type == TokenType::BitXor)
        return Value(integer(a, "^") ^ integer(b, "^"));
    if (o.type == TokenType::ShiftLeft)
        return Value(integer(a, "<<") << integer(b, "<<"));
    if (o.type == TokenType::ShiftRight)
        return Value(integer(a, ">>") >> integer(b, ">>"));
    if (o.type == TokenType::In)
    {
        if (auto p = std::get_if<Value::ArrayPtr>(&b.data))
        {
            for (auto &v : **p)
                if (equal(a, v))
                    return Value(true);
            return Value(false);
        }
        if (auto p = std::get_if<Value::MapPtr>(&b.data))
            return Value(p->get()->count(a.to_string()) != 0);
        if (auto p = std::get_if<std::string>(&b.data))
            return Value(p->find(a.to_string()) != std::string::npos);
        throw std::runtime_error("TypeError: right operand of 'in' must be array, map, or string");
    }
    throw std::runtime_error("OperatorError: unsupported operator '" + o.lexeme + "'");
}
std::string Interpreter::interpolate(const std::string &s)
{
    std::string o;
    for (size_t i = 0; i < s.size();)
    {
        if (s[i] == '$')
        {
            size_t j = i + 1;
            while (j < s.size() && (std::isalnum((unsigned char)s[j]) || s[j] == '_' || s[j] == '.'))
                ++j;
            if (j > i + 1)
            {
                std::string k = s.substr(i + 1, j - i - 1);
                size_t d = k.find('.');
                try
                {
                    o += d == std::string::npos ? env_->get(k).to_string() : member_get(env_->get(k.substr(0, d)), k.substr(d + 1)).to_string();
                    i = j;
                    continue;
                }
                catch (...)
                {
                    throw std::runtime_error("NameError: cannot interpolate '$" + k + "'");
                }
            }
        }
        o += s[i++];
    }
    return o;
}
Value Interpreter::member_get(const Value &v, const std::string &n)
{
    if (auto i = std::get_if<Value::InstancePtr>(&v.data))
    {
        auto f = (*i)->fields.find(n);
        if (f != (*i)->fields.end())
            return f->second;
        for (auto k = (*i)->klass; k; k = k->parent)
        {
            auto m = k->methods.find(n);
            if (m != k->methods.end())
            {
                auto raw = std::get<Value::FunctionPtr>(m->second.data);
                auto bound = std::make_shared<Function>();
                *bound = *raw;
                bound->bound_self = *i;
                return Value(bound);
            }
        }
        throw std::runtime_error("NoMethodError: " + (*i)->klass->name + " has no member '" + n + "'");
    }
    if (auto c = std::get_if<Value::ClassPtr>(&v.data))
    {
        if (n == "new")
        {
            auto klass = *c;
            auto f = std::make_shared<Function>();
            f->name = klass->name + ".new";
            f->native = [klass, this](const std::vector<Value> &a)
            {auto ins=std::make_shared<Instance>();ins->klass=klass;Value init;for(auto k=klass;k;k=k->parent){auto it=k->methods.find("initialize");if(it!=k->methods.end()){init=it->second;break;}}if(!std::holds_alternative<Nil>(init.data)){auto raw=std::get<Value::FunctionPtr>(init.data);auto bound=std::make_shared<Function>();*bound=*raw;bound->bound_self=ins;call(Value(bound),a);}return Value(ins); };
            return Value(f);
        }
        for (auto k = *c; k; k = k->parent)
        {
            auto it = k->methods.find(n);
            if (it != k->methods.end())
                return it->second;
        }
    }
    if (std::holds_alternative<Value::ArrayPtr>(v.data))
    {
        auto f = [this, n, v](const std::vector<Value> &args)
        {auto p=std::get<Value::ArrayPtr>(v.data);if(n=="push"){for(auto&x:args)p->push_back(x);return Value((long long)p->size());}if(n=="pop"){if(p->empty())return Value{};Value x=p->back();p->pop_back();return x;}if(n=="shift"){if(p->empty())return Value{};Value x=p->front();p->erase(p->begin());return x;}if(n=="unshift"){p->insert(p->begin(),args.begin(),args.end());return Value((long long)p->size());}if(n=="insert"){need(args.size(),2,n);long long index=integer(args[0],n);if(index<0)index+=(long long)p->size();if(index<0)index=0;if(index>(long long)p->size())index=p->size();p->insert(p->begin()+index,args[1]);return Value(v);}if(n=="remove_at"){need(args.size(),1,n);long long index=integer(args[0],n);if(index<0)index+=(long long)p->size();if(index<0||index>=(long long)p->size())return Value{};Value removed=p->at(index);p->erase(p->begin()+index);return removed;}if(n=="clear"){p->clear();return Value{};}if(n=="first")return p->empty()?Value{}:p->front();if(n=="last")return p->empty()?Value{}:p->back();if(n=="contains"){need(args.size(),1,n);for(auto&x:*p)if(equal(x,args[0]))return Value(true);return Value(false);}if(n=="count"){need(args.size(),1,n);long long c=0;for(auto&x:*p)if(equal(x,args[0]))++c;return Value(c);}if(n=="index"){need(args.size(),1,n);for(size_t i=0;i<p->size();++i)if(equal(p->at(i),args[0]))return Value((long long)i);return Value{};}if(n=="join"){need(args.size(),1,n);std::string r,sep=std::get<std::string>(args[0].data);for(size_t i=0;i<p->size();++i){if(i)r+=sep;r+=p->at(i).to_string();}return Value(r);}if(n=="reverse"){std::reverse(p->begin(),p->end());return Value(v);}if(n=="length"||n=="size")return Value((long long)p->size());if(n=="each"){need(args.size(),1,n);for(auto&x:*p)call(args[0],{x});return Value(v);}if(n=="map"){need(args.size(),1,n);Array r;for(auto&x:*p)r.push_back(call(args[0],{x}));return Value(std::move(r));}if(n=="filter"){need(args.size(),1,n);Array r;for(auto&x:*p)if(call(args[0],{x}).is_truthy())r.push_back(x);return Value(std::move(r));}if(n=="any"){need(args.size(),1,n);for(auto&x:*p)if(call(args[0],{x}).is_truthy())return Value(true);return Value(false);}if(n=="all"){need(args.size(),1,n);for(auto&x:*p)if(!call(args[0],{x}).is_truthy())return Value(false);return Value(true);}throw std::runtime_error("NoMethodError: array has no method '"+n+"'"); };
        return Value(std::make_shared<Function>(Function{n, {}, nullptr, env_, nullptr, f}));
    }
    if (std::holds_alternative<std::string>(v.data))
    {
        auto f = [this, n, v](const std::vector<Value> &a) -> Value
        {auto s=std::get<std::string>(v.data);if(n=="upper"||n=="upcase"){std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return (char)std::toupper(c);});return Value(s);}if(n=="lower"||n=="downcase"){std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return (char)std::tolower(c);});return Value(s);}if(n=="strip"||n=="trim"){auto b=s.find_first_not_of(" \t\r\n"),e=s.find_last_not_of(" \t\r\n");return Value(b==std::string::npos?"":s.substr(b,e-b+1));}if(n=="contains"||n=="starts_with"||n=="ends_with"){need(a.size(),1,n);auto q=std::get<std::string>(a[0].data);if(n=="contains")return Value(s.find(q)!=std::string::npos);if(n=="starts_with")return Value(s.rfind(q,0)==0);return Value(q.size()<=s.size()&&s.compare(s.size()-q.size(),q.size(),q)==0);}if(n=="length"||n=="size")return Value((long long)s.size());if(n=="reverse"){std::reverse(s.begin(),s.end());return Value(s);}if(n=="repeat"){need(a.size(),1,n);long long k=integer(a[0],n);if(k<0)throw std::runtime_error("ValueError: repeat count cannot be negative");std::string r;for(long long i=0;i<k;++i)r+=s;return Value(r);}if(n=="to_int"){try{return Value(std::stoll(s));}catch(...){throw std::runtime_error("ValueError: invalid integer string");}}if(n=="to_float"){try{return Value(std::stod(s));}catch(...){throw std::runtime_error("ValueError: invalid float string");}}if(n=="slice"){if(a.size()<1||a.size()>2)throw std::runtime_error("ArgumentError: slice expects 1 or 2 arguments");long long start=integer(a[0],n);if(start<0)start+=(long long)s.size();if(start<0)start=0;if(start>(long long)s.size())start=s.size();long long end=(long long)s.size();if(a.size()==2){end=integer(a[1],n);if(end<0)end+=(long long)s.size();if(end<start)end=start;if(end>(long long)s.size())end=s.size();}return Value(s.substr((size_t)start,(size_t)(end-start)));}if(n=="char_at"){need(a.size(),1,n);long long index=integer(a[0],n);if(index<0)index+=(long long)s.size();if(index<0||index>=(long long)s.size())return Value{};return Value(std::string(1,s[(size_t)index]));}if(n=="split"){need(a.size(),1,n);auto sep=std::get<std::string>(a[0].data);if(sep.empty())throw std::runtime_error("ValueError: split separator cannot be empty");Array r;size_t pos=0;while(true){auto q=s.find(sep,pos);if(q==std::string::npos){r.emplace_back(s.substr(pos));break;}r.emplace_back(s.substr(pos,q-pos));pos=q+sep.size();}return Value(std::move(r));}if(n=="replace"){need(a.size(),2,n);auto from=std::get<std::string>(a[0].data),to=std::get<std::string>(a[1].data);size_t pos=0;while((pos=s.find(from,pos))!=std::string::npos){s.replace(pos,from.size(),to);pos+=to.size();}return Value(s);}throw std::runtime_error("NoMethodError: string has no method '"+n+"'"); };
        return Value(std::make_shared<Function>(Function{n, {}, nullptr, env_, nullptr, f}));
    }
    if (v.is_number())
    {
        auto f = [this, n, v](const std::vector<Value> &a) -> Value
        {double x=num(v,n);if(n=="abs")return Value(std::fabs(x));if(n=="floor")return Value(std::floor(x));if(n=="ceil")return Value(std::ceil(x));if(n=="round")return Value(std::round(x));if(n=="sqrt"){if(x<0)throw std::runtime_error("ValueError: sqrt domain error");return Value(std::sqrt(x));}if(n=="sin")return Value(std::sin(x));if(n=="cos")return Value(std::cos(x));if(n=="tan")return Value(std::tan(x));if(n=="log"){if(x<=0)throw std::runtime_error("ValueError: log domain error");return Value(std::log(x));}if(n=="to_int")return Value((long long)x);if(n=="to_string")return Value(v.to_string());if(n=="pow"){need(a.size(),1,n);return Value(std::pow(x,num(a[0],n)));}throw std::runtime_error("NoMethodError: number has no method '"+n+"'"); };
        return Value(std::make_shared<Function>(Function{n, {}, nullptr, env_, nullptr, f}));
    }
    if (auto m = std::get_if<Value::MapPtr>(&v.data))
    {
        auto mp = *m;
        if (n == "get" || n == "set" || n == "has" || n == "delete" || n == "keys" || n == "values" || n == "clear")
        {
            auto f = [mp, n](const std::vector<Value> &a) -> Value
            {if(n=="get"){need(a.size(),1,n);auto k=std::get<std::string>(a[0].data);auto i=mp->find(k);return i==mp->end()?Value{}:i->second;}if(n=="set"){need(a.size(),2,n);mp->insert_or_assign(std::get<std::string>(a[0].data),a[1]);return a[1];}if(n=="has"){need(a.size(),1,n);return Value(mp->count(std::get<std::string>(a[0].data))!=0);}if(n=="delete"){need(a.size(),1,n);return Value(mp->erase(std::get<std::string>(a[0].data))!=0);}if(n=="keys"){Array r;for(auto&[k,x]:*mp)r.emplace_back(k);return Value(std::move(r));}if(n=="values"){Array r;for(auto&[k,x]:*mp)r.push_back(x);return Value(std::move(r));}mp->clear();return Value{}; };
            return Value(std::make_shared<Function>(Function{n, {}, nullptr, env_, nullptr, f}));
        }
        auto i = mp->find(n);
        if (i != mp->end())
            return i->second;
        throw std::runtime_error("NoMethodError: map has no key or method '" + n + "'");
    }
    throw std::runtime_error("NoMethodError: member '" + n + "' not found on " + v.type_name());
}
void Interpreter::member_set(const Value &v, const std::string &n, Value x)
{
    if (auto i = std::get_if<Value::InstancePtr>(&v.data))
    {
        (*i)->fields[n] = std::move(x);
        return;
    }
    if (auto m = std::get_if<Value::MapPtr>(&v.data))
    {
        (*m)->insert_or_assign(n, std::move(x));
        return;
    }
    throw std::runtime_error("TypeError: only instances and maps have assignable members");
}
Value Interpreter::assign_target(const ExprPtr &t, const Token &o, const Value &v)
{
    if (auto x = std::dynamic_pointer_cast<Variable>(t))
    {
        Value nv = o.type == TokenType::Equal ? v : compound(env_->get(x->name), o, v);
        if (!env_->assign(x->name, nv))
            env_->define(x->name, nv);
        return nv;
    }
    if (auto x = std::dynamic_pointer_cast<Index>(t))
    {
        Value obj = evaluate(x->object);
        Value key = evaluate(x->index);
        if (auto p = std::get_if<Value::ArrayPtr>(&obj.data))
        {
            auto idx = (int)integer(key, "index");
            if (idx < 0)
                idx += (int)(*p)->size();
            if (idx < 0 || idx >= (int)(*p)->size())
                throw std::runtime_error("IndexError: array index out of range");
            (*p)->at(idx) = o.type == TokenType::Equal ? v : compound((*p)->at(idx), o, v);
            return (*p)->at(idx);
        }
        if (auto m = std::get_if<Value::MapPtr>(&obj.data))
        {
            if (!std::holds_alternative<std::string>(key.data))
                throw std::runtime_error("TypeError: map index must be a string");
            const std::string& name = std::get<std::string>(key.data);
            auto it = (*m)->find(name);
            Value current = it == (*m)->end() ? Value{} : it->second;
            Value nv = o.type == TokenType::Equal ? v : compound(current, o, v);
            (*m)->insert_or_assign(name, nv);
            return nv;
        }
        throw std::runtime_error("TypeError: indexed assignment requires an array or map");
    }
    if (auto x = std::dynamic_pointer_cast<Member>(t))
    {
        Value obj = evaluate(x->object);
        Value nv = o.type == TokenType::Equal ? v : compound(member_get(obj, x->name), o, v);
        member_set(obj, x->name, nv);
        return nv;
    }
    throw std::runtime_error("SyntaxError: left side of assignment is not assignable");
}
Value Interpreter::evaluate(const ExprPtr &e)
{
    if (auto x = std::dynamic_pointer_cast<Literal>(e))
    {
        if (std::holds_alternative<std::string>(x->value.data))
            return Value(interpolate(std::get<std::string>(x->value.data)));
        return x->value;
    }
    if (auto x = std::dynamic_pointer_cast<Variable>(e))
    {
        try
        {
            return env_->get(x->name);
        }
        catch (...)
        {
            auto it = builtins_.find(x->name);
            if (it != builtins_.end())
            {
                auto f = std::make_shared<Function>();
                f->name = x->name;
                f->native = it->second;
                return Value(f);
            }
            throw;
        }
    }
    if (auto x = std::dynamic_pointer_cast<ArrayExpr>(e))
    {
        Array a;
        for (auto &i : x->items)
            a.push_back(evaluate(i));
        return Value(std::move(a));
    }
    if (auto x = std::dynamic_pointer_cast<MapExpr>(e))
    {
        Map m;
        for (auto &[k, v] : x->items)
            m[k] = evaluate(v);
        return Value(std::move(m));
    }
    if (auto x = std::dynamic_pointer_cast<Index>(e))
    {
        Value o = evaluate(x->object);
        Value key = evaluate(x->index);
        long long i = key.is_number() ? integer(key, "index") : 0;
        if (auto a = std::get_if<Value::ArrayPtr>(&o.data))
        {
            if (i < 0)
                i += (long long)(*a)->size();
            if (i < 0 || i >= (long long)(*a)->size())
                throw std::runtime_error("IndexError: array index out of range");
            return (*a)->at((size_t)i);
        }
        if (auto m = std::get_if<Value::MapPtr>(&o.data))
        {
            if (!std::holds_alternative<std::string>(key.data))
                throw std::runtime_error("TypeError: map index must be a string");
            std::string k = std::get<std::string>(key.data);
            auto it = (*m)->find(k);
            return it == (*m)->end() ? Value{} : it->second;
        }
        if (auto s = std::get_if<std::string>(&o.data))
        {
            if (i < 0)
                i += s->size();
            if (i < 0 || i >= (long long)s->size())
                throw std::runtime_error("IndexError: string index out of range");
            return Value(std::string(1, (*s)[i]));
        }
        throw std::runtime_error("TypeError: indexing requires array, map, or string");
    }
    if (auto x = std::dynamic_pointer_cast<Member>(e))
        return member_get(evaluate(x->object), x->name);
    if (auto x = std::dynamic_pointer_cast<ShellExpr>(e))
    {
        FILE *p = LUCY_POPEN(x->command.c_str(), "r");
        if (!p)
            throw std::runtime_error("ShellError: cannot start command");
        std::string o;
        char b[256];
        while (fgets(b, sizeof b, p))
            o += b;
        int rc = LUCY_PCLOSE(p);
        while (!o.empty() && (o.back() == '\n' || o.back() == '\r'))
            o.pop_back();
        if (rc != 0)
            throw std::runtime_error("ShellError: command failed with status " + std::to_string(rc));
        return Value(o);
    }
    if (auto x = std::dynamic_pointer_cast<RangeExpr>(e))
    {
        long long a = integer(evaluate(x->a), "range"), b = integer(evaluate(x->b), "range");
        Array r;
        if (a <= b)
            for (long long i = a; i <= (x->inclusive ? b : b - 1); ++i)
                r.emplace_back(i);
        else
            for (long long i = a; i >= (x->inclusive ? b : b + 1); --i)
                r.emplace_back(i);
        return Value(std::move(r));
    }
    if (auto x = std::dynamic_pointer_cast<Unary>(e))
    {
        if (x->op.type == TokenType::Increment || x->op.type == TokenType::Decrement)
        {
            Value old = evaluate(x->right);
            Token q = x->op;
            q.type = x->op.type == TokenType::Increment ? TokenType::PlusEqual : TokenType::MinusEqual;
            Value nv = assign_target(x->right, q, Value(1));
            return x->postfix ? old : nv;
        }
        Value v = evaluate(x->right);
        if (x->op.type == TokenType::Not)
            return Value(!v.is_truthy());
        if (x->op.type == TokenType::Minus)
            return Value(-num(v, "-"));
        if (x->op.type == TokenType::Plus)
            return Value(num(v, "+"));
        if (x->op.type == TokenType::BitNot)
            return Value(~integer(v, "~"));
        throw std::runtime_error("TypeError: invalid unary operator '" + x->op.lexeme + "'");
    }
    if (auto x = std::dynamic_pointer_cast<Binary>(e))
    {
        if (x->op.type == TokenType::And)
        {
            auto a = evaluate(x->left);
            return Value(a.is_truthy() && evaluate(x->right).is_truthy());
        }
        if (x->op.type == TokenType::Or)
        {
            auto a = evaluate(x->left);
            return Value(a.is_truthy() || evaluate(x->right).is_truthy());
        }
        return binary(evaluate(x->left), x->op, evaluate(x->right));
    }
    if (auto x = std::dynamic_pointer_cast<Ternary>(e))
        return evaluate(x->c).is_truthy() ? evaluate(x->t) : evaluate(x->f);
    if (auto x = std::dynamic_pointer_cast<LambdaExpr>(e))
    {
        auto f = std::make_shared<Function>();
        f->name = "<lambda>";
        f->params = x->params;
        f->closure = env_;
        f->body = std::make_shared<Block>(std::vector<StmtPtr>{std::make_shared<ReturnStmt>(x->body)});
        return Value(f);
    }
    if (auto x = std::dynamic_pointer_cast<Call>(e))
    {
        Value f = evaluate(x->callee);
        std::vector<Value> a;
        std::vector<std::string> names;
        for (auto &i : x->args)
        {
            a.push_back(evaluate(i.value));
            names.push_back(i.name);
        }
        return call(f, a, names);
    }
    throw std::runtime_error("RuntimeError: invalid expression");
}
Value Interpreter::call(const Value &v, const std::vector<Value> &a, const std::vector<std::string> &names)
{
    auto p = std::get_if<Value::FunctionPtr>(&v.data);
    if (!p)
        throw std::runtime_error("TypeError: " + v.type_name() + " is not callable");
    auto f = *p;
    if (f->native)
        return f->native(a);
    std::vector<std::string> arg_names = names;
    if (arg_names.empty() && !a.empty())
        arg_names.assign(a.size(), "");
    if (arg_names.size() != a.size())
        throw std::runtime_error("RuntimeError: invalid call argument metadata");
    std::vector<Value> bound(f->params.size());
    std::vector<bool> supplied(f->params.size(), false);
    std::vector<Value> positional_values;
    size_t next_param = 0;
    bool named_seen = false;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (arg_names[i].empty())
        {
            if (named_seen)
                throw std::runtime_error("ArgumentError: positional argument cannot follow named argument");
            positional_values.push_back(a[i]);
            while (next_param < f->params.size() && supplied[next_param])
                ++next_param;
            if (next_param >= f->params.size())
                throw std::runtime_error("ArgumentError: function '" + f->name + "' received too many arguments");
            if (f->params[next_param].variadic)
                continue;
            bound[next_param] = a[i];
            supplied[next_param] = true;
            ++next_param;
        }
        else
        {
            named_seen = true;
            bool found = false;
            for (size_t j = 0; j < f->params.size(); ++j)
                if (f->params[j].name == arg_names[i])
                {
                    if (f->params[j].variadic)
                        throw std::runtime_error("ArgumentError: variadic parameter '" + arg_names[i] + "' must be passed positionally");
                    if (supplied[j])
                        throw std::runtime_error("ArgumentError: multiple values for argument '" + names[i] + "'");
                    bound[j] = a[i];
                    supplied[j] = true;
                    found = true;
                    break;
                }
            if (!found)
                throw std::runtime_error("ArgumentError: function '" + f->name + "' has no parameter named '" + arg_names[i] + "'");
        }
    }
    auto old = env_;
    env_ = make_environment(f->closure ? f->closure : globals_);
    if (f->bound_self)
        env_->define("self", Value(f->bound_self));
    try
    {
        size_t positional_index = 0;
        for (size_t i = 0; i < f->params.size(); ++i)
        {
            auto &param = f->params[i];
            if (param.variadic)
            {
                Array rest;
                while (positional_index < positional_values.size())
                    rest.push_back(positional_values[positional_index++]);
                env_->define(param.name, Value(std::move(rest)));
            }
            else if (supplied[i])
            {
                ++positional_index;
                env_->define(param.name, bound[i]);
            }
            else if (param.default_value)
            {
                env_->define(param.name, evaluate(param.default_value));
            }
            else
            {
                throw std::runtime_error("ArgumentError: missing required argument '" + param.name + "' in function '" + f->name + "'");
            }
        }
        execute(f->body);
    }
    catch (const ReturnSignal &r)
    {
        env_ = old;
        return r.value;
    }
    catch (...)
    {
        env_ = old;
        throw;
    }
    env_ = old;
    return Value{};
}
Value Interpreter::execute(const StmtPtr &s)
{
    if (auto x = std::dynamic_pointer_cast<ExprStmt>(s))
    {
        Value v = evaluate(x->expr);
        if (std::holds_alternative<Value::FunctionPtr>(v.data))
            return call(v, {});
        return v;
    }
    if (auto x = std::dynamic_pointer_cast<VarDecl>(s))
    {
        auto target = x->global ? globals_ : env_;
        target->define(x->name, x->value ? evaluate(x->value) : Value{}, x->constant);
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<Assign>(s))
        return assign_target(x->target, x->op, evaluate(x->value));
    if (auto x = std::dynamic_pointer_cast<Block>(s))
    {
        auto old = env_;
        env_ = make_environment(old);
        try
        {
            Value last;
            for (auto &i : x->statements)
                last = execute(i);
            env_ = old;
            return last;
        }
        catch (...)
        {
            env_ = old;
            throw;
        }
    }
    if (auto x = std::dynamic_pointer_cast<IfStmt>(s))
    {
        if (evaluate(x->condition).is_truthy())
            return execute(x->then_branch);
        for (auto &e : x->else_ifs)
            if (evaluate(e.first).is_truthy())
                return execute(e.second);
        return x->else_branch ? execute(x->else_branch) : Value{};
    }
    if (auto x = std::dynamic_pointer_cast<WhileStmt>(s))
    {
        size_t g = 0;
        while (evaluate(x->condition).is_truthy())
        {
            try
            {
                execute(x->body);
            }
            catch (BreakSignal &)
            {
                break;
            }
            catch (ContinueSignal &)
            {
            }
            if (++g > 10000000)
                throw std::runtime_error("LoopError: iteration limit exceeded");
        }
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<DoWhileStmt>(s))
    {
        size_t g = 0;
        do
        {
            try
            {
                execute(x->body);
            }
            catch (BreakSignal &)
            {
                break;
            }
            catch (ContinueSignal &)
            {
            }
            if (++g > 10000000)
                throw std::runtime_error("LoopError: iteration limit exceeded");
        } while (evaluate(x->condition).is_truthy());
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<SwitchStmt>(s))
    {
        Value target = evaluate(x->value);
        for (auto &c : x->cases)
        {
            if (equal(target, evaluate(c.first)))
            {
                try
                {
                    execute(c.second);
                }
                catch (BreakSignal &)
                {
                }
                return Value{};
            }
        }
        if (x->default_branch)
        {
            try
            {
                execute(x->default_branch);
            }
            catch (BreakSignal &)
            {
            }
        }
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<ForStmt>(s))
    {
        auto v = evaluate(x->iterable);
        auto a = std::get_if<Value::ArrayPtr>(&v.data);
        if (!a)
            throw std::runtime_error("TypeError: for/foreach expects an array or range");
        auto old = env_;
        env_ = make_environment(old);
        try
        {
            for (auto &item : **a)
            {
                if (env_->local(x->name))
                    env_->assign(x->name, item);
                else
                    env_->define(x->name, item);
                try
                {
                    execute(x->body);
                }
                catch (BreakSignal &)
                {
                    break;
                }
                catch (ContinueSignal &)
                {
                }
            }
            env_ = old;
        }
        catch (...)
        {
            env_ = old;
            throw;
        }
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<LoopStmt>(s))
    {
        size_t g = 0;
        while (true)
        {
            try
            {
                execute(x->body);
            }
            catch (BreakSignal &)
            {
                break;
            }
            catch (ContinueSignal &)
            {
            }
            if (++g > 10000000)
                throw std::runtime_error("LoopError: loop iteration limit exceeded");
        }
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<FunctionStmt>(s))
    {
        auto f = std::make_shared<Function>();
        f->name = x->name;
        f->params = x->params;
        f->body = x->body;
        f->closure = env_;
        env_->define(x->name, Value(f));
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<ClassStmt>(s))
    {
        auto c = std::make_shared<Class>();
        c->name = x->name;
        c->base = x->base;
        if (!x->base.empty())
        {
            Value b = env_->get(x->base);
            c->parent = std::get<Value::ClassPtr>(b.data);
        }
        for (auto &m : x->methods)
        {
            auto f = std::make_shared<Function>();
            f->name = m->name;
            f->params = m->params;
            f->body = m->body;
            f->closure = env_;
            c->methods[f->name] = Value(f);
        }
        env_->define(x->name, Value(c));
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<ReturnStmt>(s))
        throw ReturnSignal(x->value ? evaluate(x->value) : Value{});
    if (std::dynamic_pointer_cast<BreakStmt>(s))
        throw BreakSignal{};
    if (std::dynamic_pointer_cast<ContinueStmt>(s))
        throw ContinueSignal{};
    if (auto x = std::dynamic_pointer_cast<ImportStmt>(s))
    {
        import_module(*x);
        return Value{};
    }
    if (auto x = std::dynamic_pointer_cast<ThrowStmt>(s))
        throw std::runtime_error("Exception: " + evaluate(x->value).to_string());
    if (auto x = std::dynamic_pointer_cast<TryStmt>(s))
    {
        try
        {
            execute(x->body);
        }
        catch (const std::exception &e)
        {
            if (x->catch_body && (x->catch_type.empty() || x->catch_type == "Exception" || std::string(e.what()).find(x->catch_type + ":") != std::string::npos))
            {
                if (!x->catch_name.empty())
                    env_->define(x->catch_name, Value(std::string(e.what())));
                execute(x->catch_body);
            }
            else
            {
                if (x->finally_body)
                    execute(x->finally_body);
                throw;
            }
        }
        if (x->finally_body)
            execute(x->finally_body);
        return Value{};
    }
    throw std::runtime_error("RuntimeError: unknown statement");
}
void Interpreter::import_module(const ImportStmt &x)
{
    auto path = resolve_module(x.module);
    if (std::find(import_stack_.begin(), import_stack_.end(), path) != import_stack_.end())
        throw std::runtime_error("ImportError: circular import detected");
    import_stack_.push_back(path);
    try
    {
        Lexer l(read_file(path));
        Parser p(l.scan());
        auto prog = p.parse();
        auto old = env_;
        auto file = current_file_;
        auto mod = make_environment(globals_);
        env_ = mod;
        current_file_ = path;
        run(prog);
        // Ordinary imports expose a single module object (for example `import fs` -> `fs.read`).
        // Nothing is leaked into the caller scope; use `from module import name` for direct imports.
        // Keep the module environment alive so functions and classes can continue
        // resolving names defined by the module itself. Environments are owned by the
        // interpreter and cleared during interpreter destruction, which also breaks
        // closure/environment cycles safely.
        if (x.selective)
        {
            // `from module import name` exposes only the requested names.
            for (const auto &name : x.names)
            {
                old->define(name, mod->get(name));
            }
        }
        else
        {
            const std::string alias =
                x.alias.empty() ? fs::path(path).stem().string() : x.alias;

            auto object = std::make_shared<Instance>();
            auto klass = std::make_shared<Class>();
            klass->name = alias;
            object->klass = klass;
            for (auto &[name, entry] : mod->values())
            {
                if (std::holds_alternative<Value::FunctionPtr>(entry.value.data) &&
                    (name.empty() || name[0] != '_'))
                {
                    object->fields[name] = entry.value;
                }
            }
            old->define(alias, Value(object));
        }
        env_ = old;
        current_file_ = file;
        import_stack_.pop_back();
    }
    catch (...)
    {
        import_stack_.pop_back();
        throw;
    }
}
static fs::path executable_path()
{
#ifdef _WIN32
    char buffer[32768];
    DWORD n = GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    if (n == 0)
        return {};
    return fs::path(std::string(buffer, n));
#elif __APPLE__
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0)
        return {};
    return fs::path(buffer.c_str());
#elif __linux__
    std::error_code ec;
    auto path = fs::read_symlink("/proc/self/exe", ec);
    return ec ? fs::path{} : path;
#else
    return {};
#endif
}

std::string Interpreter::resolve_module(const std::string &m) const
{
    fs::path p = m;
    if (p.extension() != ".lucy")
        p += ".lucy";
    std::vector<fs::path> c;
    if (!current_file_.empty())
        c.push_back(fs::path(current_file_).parent_path() / p);
    if (const char *lp = std::getenv("LUCY_PATH"))
    {
        std::stringstream ss(lp);
        std::string q;
        char sep = ':';
#ifdef _WIN32
        sep = ';';
#endif
        while (std::getline(ss, q, sep))
            if (!q.empty())
                c.push_back(fs::path(q) / p);
    }
    c.push_back(fs::current_path() / "stdlib" / p);
    if (const char *home = std::getenv("LUCY_STDLIB"))
        c.push_back(fs::path(home) / p);

    // Prefer a stdlib next to the executable so a Lucy directory can be moved
    // anywhere without breaking imports. Also support the Unix install layout.
    const fs::path exe = executable_path();
    if (!exe.empty())
    {
        const fs::path exe_dir = exe.parent_path();
        c.push_back(exe_dir / "stdlib" / p);
        c.push_back(exe_dir.parent_path() / "share" / "lucy" / "stdlib" / p);
    }

#ifdef _WIN32
// Keep Windows free of Unix-specific filesystem fallbacks.
#else
    c.push_back(fs::path("/usr/local/share/lucy/stdlib") / p);
#endif

    for (auto &x : c)
        if (fs::exists(x))
            return fs::weakly_canonical(x).string();
    throw std::runtime_error("ImportError: module '" + m + "' not found");
}
std::vector<std::pair<std::string, std::string>> Interpreter::list_modules() const
{
    // همه‌ی مسیرهایی که Lucy برای import جستجو می‌کنه
    std::vector<fs::path> search_paths;

    // ۱. پوشه فایل فعلی
    if (!current_file_.empty())
        search_paths.push_back(fs::path(current_file_).parent_path());

    // ۲. LUCY_PATH
    if (const char *lp = std::getenv("LUCY_PATH"))
    {
        std::stringstream ss(lp);
        std::string q;
        char sep = ':';
#ifdef _WIN32
        sep = ';';
#endif
        while (std::getline(ss, q, sep))
            if (!q.empty())
                search_paths.push_back(fs::path(q));
    }

    // ۳. stdlib کنار پروژه
    search_paths.push_back(fs::current_path() / "stdlib");

    // ۴. LUCY_STDLIB
    if (const char *home = std::getenv("LUCY_STDLIB"))
        search_paths.push_back(fs::path(home));

    // ۵. کنار executable
    const fs::path exe = executable_path();
    if (!exe.empty())
    {
        const fs::path exe_dir = exe.parent_path();
        search_paths.push_back(exe_dir / "stdlib");
        search_paths.push_back(exe_dir.parent_path() / "share" / "lucy" / "stdlib");
    }

    // ۶. مسیر نصب یونیکس
#ifndef _WIN32
    search_paths.push_back(fs::path("/usr/local/share/lucy/stdlib"));
#endif

    // حالا همه فایل‌های .lucy رو پیدا کن
    std::vector<std::pair<std::string, std::string>> result;
    std::set<std::string> seen; // برای جلوگیری از تکرار

    for (const auto &base : search_paths)
    {
        if (!fs::is_directory(base))
            continue;

        for (const auto &entry : fs::directory_iterator(base))
        {
            if (!entry.is_regular_file())
                continue;
            auto path = entry.path();
            if (path.extension() != ".lucy")
                continue;

            std::string name = path.stem().string();

            // اگه قبلاً پیدا شده (توی مسیر با اولویت بالاتر)، رد کن
            if (seen.count(name))
                continue;
            seen.insert(name);

            result.emplace_back(name, path.lexically_normal().string());
        }
    }

    // مرتب‌سازی بر اساس اسم
    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b)
              { return a.first < b.first; });

    return result;
}
std::string Interpreter::read_file(const std::string &p) const
{
    std::ifstream f(p);
    if (!f)
        throw std::runtime_error("IOError: cannot read '" + p + "'");
    std::ostringstream s;
    s << f.rdbuf();
    return s.str();
}
Value Interpreter::run(const std::vector<StmtPtr> &s, bool echo)
{
    Value last;
    for (auto &i : s)
    {
        last = execute(i);
        if (echo && !std::holds_alternative<Nil>(last.data))
            std::cout << last.to_string() << '\n';
    }
    return last;
}
std::vector<std::string> Interpreter::completion_candidates(const std::string &input) const
{
    
    static const std::vector<std::string> keywords = {
        "if", "else", "while", "do", "for", "foreach", "loop", "switch", "case", "default", "function", "def", "lambda",
        "class", "return", "break", "continue", "end", "import", "from", "as",
        "const", "global", "in", "and", "or", "not", "new", "self", "super",
        "try", "catch", "finally", "throw", "true", "false", "nil",
        "modules", "modules_info" };

    static const std::vector<std::string> modules = {
    "collections", "csv", "datetime", "dir", "encoding", "file", "http", "io",
    "json", "math", "os", "path", "process", "random", "regex", "sqlite",
    "string", "sys", "time", "repl", "flow", "data", "result"
    };
    static const std::unordered_map<std::string, std::vector<std::string>> module_members = {
        {"collections", {"first", "last", "reverse", "contains", "count", "index", "compact", "unique", "flatten", "sum", "min", "max"}},
        {"csv", {"parse", "stringify"}},
        {"datetime", {"now", "from_timestamp", "format"}},
        {"dir", {"pwd", "chdir", "exists", "entries", "files", "dirs", "glob", "walk", "mkdir", "rmdir", "empty", "copy"}},
        {"encoding", {"base64_encode", "base64_decode", "hex_encode", "hex_decode", "url_encode", "url_decode"}},
        {"file", {"read", "write", "append", "read_lines", "write_lines", "exists", "size", "delete", "copy", "move", "touch"}},
        {"http", {"get", "post", "put", "patch", "delete", "head", "request", "client", "headers", "timeout", "connect_timeout", "proxy", "user_agent", "follow_redirects", "insecure", "connect", "bind", "listen", "accept", "recv", "send", "close", "resolve", "reverse"}},
        {"io", {"print", "write", "read", "ask"}},
        {"json", {"parse", "stringify"}},
        {"math", {"square", "cube", "clamp", "even", "odd", "abs", "sqrt", "pow", "sin", "cos", "tan", "floor", "ceil", "log", "min", "max", "factorial", "gcd", "lcm", "average"}},
        {"os", {"cwd", "env", "setenv", "unsetenv", "system", "pid", "cpu_count", "platform", "version", "home", "temp_dir", "command_exists"}},
        {"path", {"join", "absolute", "expand", "basename", "dirname", "extname", "stem"}},
        {"process", {"run", "capture"}},
        {"random", {"integer", "choice"}},
        {"regex", {"match", "search", "find_all", "replace"}},
        {"sqlite", {"open"}},
        {"string", {"capitalize", "reverse", "repeat"}},
        {"sys", {"version", "platform", "cwd", "env", "argv"}},
        {"time", {"now", "strptime", "format", "year", "month", "day", "hour", "minute", "second", "add", "subtract", "plus", "minus", "compare", "succ"}},
        {"repl", {"banner", "prompt", "commands", "topics", "help"}},
        {"flow", {"pipe", "tap", "branch", "repeat"}},
        {"data", {"pick", "omit", "merge", "values", "zip"}},
        {"result", {"ok", "err", "success", "unwrap", "message"}},
        {"date", {"parse", "strptime", "format", "add", "subtract", "next_day", "prev_day", "succ", "shift_months", "add_months", "subtract_months", "compare"}},
        {"digest", {"digest", "hexdigest", "base64digest", "file"}},
        {"socket", {"new", "connect", "bind", "listen", "accept", "recv", "send", "close"}},
        {"net_http", {"get", "put", "delete", "head", "patch", "post", "request", "start", "proxy"}},
        {"resolv", {"getaddress", "getname"}},
        {"fileutils", {"chmod", "chown", "ln", "link", "symlink"}},
        {"stringscanner", {"new", "scan", "scan_until", "skip", "skip_until", "check", "check_until", "match?", "matched", "matched_size", "pre_match", "post_match"}},
        {"set", {"new", "add", "delete", "include?", "member?", "each", "size", "length", "empty?", "clear", "map", "select", "reject", "merge", "subset?", "superset?", "intersect?", "union", "intersection", "difference", "symmetric_difference"}},
        {"yaml", {"load", "safe_load", "dump", "load_file"}},
        {"option_parser", {"new", "banner", "separator", "version", "program_name", "on", "parse", "parse!", "help", "summarize", "abort"}},
        {"logger", {"new", "debug", "info", "warn", "error", "fatal", "add", "log", "level", "set_level", "debug?", "info?", "warn?", "error?", "fatal?"}},
        {"timeout", {"timeout"}},
        {"benchmark", {"measure", "realtime"}},
        {"signal", {"trap", "list", "signame"}},
        {"process", {"run", "capture", "success", "output", "ppid", "spawn", "wait", "waitpid", "kill", "uid", "gid", "euid", "egid", "groups", "clock_gettime"}}};

    static const std::vector<std::string> array_members = {
        "push", "pop", "shift", "unshift", "insert", "remove_at", "clear", "first", "last", "contains",
        "count", "index", "join", "reverse", "length", "size", "each", "map",
        "filter", "any", "all"};

    static const std::vector<std::string> string_members = {
        "upper", "upcase", "lower", "downcase", "strip", "trim", "contains",
        "starts_with", "ends_with", "length", "size", "reverse", "repeat", "to_int",
        "to_float", "slice", "char_at", "split", "replace"};

    static const std::vector<std::string> map_members = {
        "get", "set", "has", "delete", "keys", "values", "length", "size", "clear"};

    std::size_t start = input.size();
    while (start > 0)
    {
        const char ch = input[start - 1];
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '.'))
        {
            break;
        }
        --start;
    }

    const std::string token = input.substr(start);
    const std::size_t dot = token.find('.');

    std::vector<std::string> result;
    if (dot != std::string::npos)
    {
        const std::string object_name = token.substr(0, dot);
        const std::string member_prefix = token.substr(dot + 1);

        auto module = module_members.find(object_name);
        if (module != module_members.end())
        {
            for (const auto &member : module->second)
            {
                if (member.compare(0, member_prefix.size(), member_prefix) == 0)
                {
                    result.push_back(member);
                }
            }
            return result;
        }

        try
        {
            Value object = env_->get(object_name);
            const auto add_member = [&](const std::string &name)
            {
                if (name.compare(0, member_prefix.size(), member_prefix) == 0)
                {
                    result.push_back(name);
                }
            };

            if (std::holds_alternative<Value::ArrayPtr>(object.data))
            {
                for (const auto &member : array_members)
                    add_member(member);
            }
            else if (std::holds_alternative<std::string>(object.data))
            {
                for (const auto &member : string_members)
                    add_member(member);
            }
            else if (std::holds_alternative<Value::MapPtr>(object.data))
            {
                for (const auto &member : map_members)
                    add_member(member);
            }
            else if (auto instance = std::get_if<Value::InstancePtr>(&object.data))
            {
                for (const auto &[name, value] : (*instance)->fields)
                    add_member(name);
                for (auto klass = (*instance)->klass; klass; klass = klass->parent)
                {
                    for (const auto &[name, value] : klass->methods)
                        add_member(name);
                }
            }
            else if (auto klass = std::get_if<Value::ClassPtr>(&object.data))
            {
                add_member("new");
                for (auto current = *klass; current; current = current->parent)
                {
                    for (const auto &[name, value] : current->methods)
                        add_member(name);
                }
            }
        }
        catch (...)
        {
            // Completion must never execute code or turn an unknown name into an error.
        }
        return result;
    }

    const std::string prefix = token;
    auto append_matching = [&result, &prefix](const auto &values)
    {
        for (const auto &value : values)
        {
            if (value.compare(0, prefix.size(), prefix) == 0)
            {
                result.push_back(value);
            }
        }
    };

    append_matching(keywords);
    append_matching(modules);
    for (const auto &[name, function] : builtins_)
    {
        if (name.compare(0, prefix.size(), prefix) == 0)
        {
            result.push_back(name);
        }
    }

    for (auto environment = env_; environment; environment = environment->parent())
    {
        for (const auto &[name, entry] : environment->values())
        {
            if (name.compare(0, prefix.size(), prefix) == 0)
            {
                result.push_back(name);
            }
        }
    }

    return result;
}

void Interpreter::repl()
{
    ReplLineEditor editor;

    auto eval_repl_library = [this](const std::string &source) -> Value
    {
        Lexer lexer(source);
        Parser parser(lexer.scan());
        return run(parser.parse(), false);
    };

    bool repl_library_loaded = false;
    try
    {
        eval_repl_library("import repl\nfrom repl import help\n");
        repl_library_loaded = true;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Warning: REPL library could not be loaded: " << error.what() << '\n';
    }

    if (repl_library_loaded)
    {
        try
        {
            Value banner = eval_repl_library("repl.banner()\n");
            std::cout << banner.to_string() << "\n\n";
        }
        catch (const std::exception &)
        {
            std::cout << "Lucy 1.0.1 Interactive REPL\n\n";
        }
    }
    else
    {
        std::cout << "Lucy 1.0.1 Interactive REPL\n\n";
    }

    std::string buffer;
    int block_depth = 0;

    auto update_block_depth = [&block_depth](const std::string &line)
    {
        std::string trimmed = line;
        while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())))
        {
            trimmed.pop_back();
        }

        std::istringstream stream(trimmed);
        std::string first;
        stream >> first;

        if (first == "if" || first == "while" || first == "for" || first == "foreach" ||
            first == "loop" || first == "switch" || first == "do" || first == "function" ||
            first == "def" || first == "class" || first == "try")
        {
            ++block_depth;
        }
        else if (first == "end")
        {
            block_depth = std::max(0, block_depth - 1);
        }
    };

    while (true)
    {
        std::string prompt = block_depth == 0 ? ">>> " : "... ";
        if (repl_library_loaded)
        {
            try
            {
                Value custom_prompt = eval_repl_library(
                    "repl.prompt(" + std::to_string(block_depth) + ")\n");
                prompt = custom_prompt.to_string();
            }
            catch (const std::exception &)
            {
                // Keep the native fallback prompt if the editable REPL library fails.
            }
        }
        std::string line;

        if (!editor.read_line(prompt, line, [this](const std::string &input)
                              { return completion_candidates(input); }))
        {
            break;
        }

        if (block_depth == 0 && line == ":quit")
            break;
        if (block_depth == 0 && line == ":exit")
            break;

        if (block_depth == 0 && line == ":help")
        {
            if (repl_library_loaded)
            {
                try
                {
                    eval_repl_library("repl.help()\n");
                }
                catch (const std::exception &error)
                {
                    std::cerr << error.what() << '\n';
                }
            }
            else
            {
                std::cout << "REPL help library is unavailable.\n";
            }
            continue;
        }

        if (block_depth == 0 && line == ":version")
        {
            if (repl_library_loaded)
            {
                try
                {
                    Value version = eval_repl_library("repl.version()\n");
                    std::cout << version.to_string() << '\n';
                }
                catch (const std::exception &)
                {
                    std::cout << "1.0.1\n";
                }
            }
            else
            {
                std::cout << "1.0.1\n";
            }
            continue;
        }

        if (block_depth == 0 && line == ":history")
        {
            // The line editor owns history; the command is intentionally lightweight.
            editor.print_history();
            continue;
        }

        if (block_depth == 0 && line == ":clear")
        {
            buffer.clear();
            block_depth = 0;
            std::cout << "\x1b[2J\x1b[H" << std::flush;
            continue;
        }

        buffer += line;
        buffer += '\n';
        update_block_depth(line);

        if (block_depth > 0)
        {
            continue;
        }

        try
        {
            Lexer lexer(buffer);
            Parser parser(lexer.scan());
            run(parser.parse(), true);
        }
        catch (const std::exception &error)
        {
            std::cerr << error.what() << '\n';
        }

        buffer.clear();
        block_depth = 0;
    }
}
