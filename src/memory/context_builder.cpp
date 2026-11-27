// src/memory/context_builder.cpp — YOU IMPLEMENT. Spec §4.4.

#include "memory/context_builder.h"
#include "util/min_heap.h"

ContextPlan select_context(const std::vector<ContextItem>& items,
                           std::size_t budget, std::size_t keep_recent) {
    // TODO: total the tokens; if they fit, keep everything. Otherwise push
    // every unprotected item onto a MinHeap keyed on (drop_rank(role), index)
    // and pop/drop until the total fits or the heap is empty.
    //
    // Clues:
    //  * Heap entries:  MinHeap<std::pair<int, std::size_t>> heap;
    //    std::pair compares .first, then .second, which is exactly
    //    "lowest rank first, then oldest first".
    //  * "The last keep_recent items" start at index items.size() - keep_recent,
    //    but these are unsigned: if keep_recent > items.size() that
    //    subtraction wraps around to a huge number. Handle that case first.
    //  * A vector<bool> keep(items.size(), true) makes the final pass easy:
    //    collect the indices still marked true, in ascending order.
    //  * total_tokens is the sum of the KEPT items; over_budget is
    //    total_tokens > budget.
    std::size_t total = 0;
    for (const ContextItem& item : items) {
        total += item.tokens;
    }
    std::vector<bool> keep(items.size(), true);
    if (total > budget) {
        std::size_t recent_start = 0;
        if (keep_recent < items.size()) {
            recent_start = items.size() - keep_recent;
        }
        MinHeap<std::pair<int, std::size_t>> heap;
        for (std::size_t i = 0; i < items.size(); i++) {
            if ((items[i].role == Role::System) || (items[i].pinned) || (i >= recent_start)) {
                continue;
            }
            heap.push({drop_rank(items[i].role), i});
        }
        while ((total > budget) && !heap.empty()) {
            std::size_t index = heap.pop().second;
            keep[index] = false;
            total -= items[index].tokens;
        }
    }
    ContextPlan plan;
    for (std::size_t i = 0; i < items.size(); i++) {
        if (keep[i]) {
            plan.keep.push_back(i);
        }
    }
    plan.total_tokens = total;
    plan.over_budget = total > budget;
    return plan;
}
