// src/policy/policy_chain.cpp — YOU IMPLEMENT. Spec §5.3.

#include "policy/policy_chain.h"

#include <utility>

bool PolicyChain::add(std::unique_ptr<Policy> child) {
    // TODO
    // Clue: a unique_ptr cannot be copied. Check for nullptr first, then
    // children_.push_back(std::move(child)). Plain push_back(child) gives a
    // long error mentioning "use of deleted function unique_ptr(const unique_ptr&)".
    if (child == nullptr) {
        return false;
    }
    else{
        children_.push_back(std::move(child));
        return true;
    }
}
    

std::size_t PolicyChain::size() const noexcept { return children_.size(); }

Decision PolicyChain::evaluate(const CommandRequest& req) {
    // TODO
    // Clues:
    //  * Handle the empty chain before the loop (Deny, policy "chain").
    //  * Loop with  for (const auto& child : children_)  (the & matters:
    //    unique_ptr can't be copied) and call child->evaluate(req).
    //  * Don't keep a "most restrictive so far" maximum: that returns the
    //    LAST Ask. Instead: return a Deny the moment you see one; remember
    //    only the FIRST Ask; after the loop return that Ask if there was one.
    //  * The enum is nested: write Decision::Kind::Deny, not Deny or
    //    Decision::Deny.
    if (children_.empty()) {
        return Decision::deny("the chain has no policies", "chain");
    }
    bool yn = false;
    Decision first_ask = Decision::allow("no confirmation needed", "chain");
    for (const auto& child : children_) {
        Decision d = child->evaluate(req);
        if (d.kind == Decision::Kind::Deny) {
            return d;
        }
        if (d.kind == Decision::Kind::Ask && !yn) {
            first_ask = d;
            yn = true;
        }
    }
    if (yn) {
        return first_ask;
    }
    else {
        return Decision::allow("all policies allowed command", "chain");
    }
    
}
