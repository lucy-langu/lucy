#include "lucy/runtime.hpp"
#include "type_system.hpp"
#include <algorithm>
#include <limits>
#include <vector>
namespace lucy {
void Environment::define(const std::string &n, Value v, bool c, const std::string &type) { values_[n] = {std::move(v), c, type}; }
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
        if (!i->second.type_name.empty())
            require_type(i->second.type_name, v, "variable '" + n + "'");
        i->second.value = std::move(v);
        return true;
    }
    return parent_ && parent_->assign(n, std::move(v));
}
bool Environment::type_check(const std::string &name, const Value &value) const
{
    auto i = values_.find(name);
    if (i != values_.end()) return i->second.type_name.empty() || type_matches(i->second.type_name, value);
    return parent_ ? parent_->type_check(name, value) : true;
}
static std::size_t lucy_edit_distance(const std::string& left, const std::string& right)
{
    std::vector<std::size_t> previous(right.size() + 1), current(right.size() + 1);
    for (std::size_t j = 0; j <= right.size(); ++j) previous[j] = j;
    for (std::size_t i = 1; i <= left.size(); ++i) {
        current[0] = i;
        for (std::size_t j = 1; j <= right.size(); ++j) {
            const std::size_t substitution = previous[j - 1] + (left[i - 1] == right[j - 1] ? 0 : 1);
            current[j] = std::min({previous[j] + 1, current[j - 1] + 1, substitution});
        }
        previous.swap(current);
    }
    return previous[right.size()];
}

Value Environment::get(const std::string &n) const
{
    auto i = values_.find(n);
    if (i != values_.end())
        return i->second.value;
    if (parent_)
        return parent_->get(n);

    // Suggest a visible name for common misspellings without changing lookup semantics.
    std::string best;
    std::size_t best_distance = std::numeric_limits<std::size_t>::max();
    for (const Environment* scope = this; scope; scope = scope->parent_.get()) {
        for (const auto& entry : scope->values_) {
            const auto distance = lucy_edit_distance(n, entry.first);
            if (distance < best_distance) {
                best_distance = distance;
                best = entry.first;
            }
        }
    }
    const std::size_t suggestion_limit = n.size() < 5 ? 1 : 2;
    if (!best.empty() && best_distance <= suggestion_limit)
        throw std::runtime_error("NameError: undefined variable '" + n + "'. Did you mean '" + best + "'?");
    throw std::runtime_error("NameError: undefined variable '" + n + "'");
}
std::shared_ptr<Environment> Environment::root()
{
    auto r = shared_from_this();
    while (r->parent_)
        r = r->parent_;
    return r;
}

} // namespace lucy
