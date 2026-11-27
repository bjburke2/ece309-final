// src/skills/skill_index.cpp — YOU IMPLEMENT. Spec §6.3.

#include "skills/skill_index.h"
#include "util/min_heap.h"
#include "util/text.h"

void SkillIndex::add(std::size_t doc_id, std::string_view text) {
    // TODO
    // Clue: use unique_tokens(text), not tokenize(text): a word that appears
    // twice in one document must post its doc_id only once.
    auto tokens = unique_tokens(text);
    for (const auto& word : tokens) {
        postings_[std::string(word)].push_back(doc_id);
    }
}

const std::vector<std::size_t>* SkillIndex::postings(std::string_view word) const {
    // TODO
    // Clue: the map's keys are std::string, so find(word) with a
    // string_view does not compile in C++17: use find(std::string(word)).
    // Return the address of the vector inside the map (&it->second).
    auto it = postings_.find(std::string(word));
    if (it == postings_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::size_t SkillIndex::word_count() const noexcept {
    // TODO
    return postings_.size();
}

std::vector<std::size_t> SkillIndex::search(std::string_view query, std::size_t k) const {
    // TODO: score documents using only the posting lists of the query's
    // words, then rank with top_k().
    // Clues:
    //  * Count with  std::unordered_map<std::size_t, std::size_t> score;
    //    walking the posting list of each word of unique_tokens(query)
    //    (a repeated query word counts once).
    //  * Put (score, doc_id) pairs in a vector and call top_k with a
    //    comparator meaning "a is WORSE than b": lower score is worse; with
    //    equal scores the LARGER doc_id is worse.
    //  * Don't copy the lambda in skill_index_linear.cpp: that one is a
    //    std::sort order ("a comes first"), the opposite direction.
    if (!k) {
        return {};
    }
 
    std::unordered_map<std::size_t, std::size_t> score;
    auto words = unique_tokens(query);
    for (const auto& word : words) {
        const std::vector<std::size_t>* list = postings(word);
        if (!list) {
            continue;
        }
        for (std::size_t doc_id : *list) {
            score[doc_id]++;
        }
    }
    std::vector<std::pair<std::size_t, std::size_t>> ranked;
    for (const auto& entry : score) {
        ranked.push_back({entry.second, entry.first});
    }

    auto worse = [](const std::pair<std::size_t, std::size_t>& a,
                    const std::pair<std::size_t, std::size_t>& b) {
        if (a.first != b.first) {
            return a.first < b.first;
        }
        return a.second > b.second;
    };
    auto best = top_k(ranked, k, worse);
 
    std::vector<std::size_t> result;
    for (const auto& p : best) {
        result.push_back(p.second);
    }
    return result;
}
