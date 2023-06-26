#include "search_server.h"
#include "document.h"
#include "string_processing.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <numeric>

using namespace std;



SearchServer::SearchServer(std::string_view stop_words_text)
    : SearchServer(
        SplitIntoWords(stop_words_text))
{
}

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(
        SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }
    
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
    dec_.push_back(string(document));
    const auto words = SplitIntoWordsNoStop(dec_.back());
    
    const double inv_word_count = 1.0 / words.size();
    for (string_view word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        words_freqs[document_id][word] += inv_word_count;
    }
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) { return document_status == status;});
    }

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&, string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) { return document_status == status;});
    }

vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&, string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&, string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(execution::par, raw_query, [status](int document_id, DocumentStatus document_status, int rating) { return document_status == status;});
    }

vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&, string_view raw_query) const {
    return FindTopDocuments(execution::par, raw_query, DocumentStatus::ACTUAL);
}

vector<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}

vector<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

int SearchServer::GetDocumentCount() const {
    return words_freqs.size();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
    const auto query = ParseQuery(true, raw_query);
    std::vector<std::string_view> matched_words;
    
    for (std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            return { matched_words, documents_.at(document_id).status };
        }
    }
    
    for (std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    
    return { matched_words, documents_.at(document_id).status };
}

    tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, string_view raw_query, int document_id) const{
        return MatchDocument(raw_query, document_id);
    }

    tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, string_view raw_query, int document_id) const{
        const auto query = ParseQuery(false, raw_query);
        vector<string_view> matched_words;
        
        if(any_of(execution::par, query.minus_words.begin(), query.minus_words.end(), [&](auto word){
            return (word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id));
        }) == true) return {matched_words, documents_.at(document_id).status};
        
        matched_words.resize(query.plus_words.size());
        auto it1 = copy_if(execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), [&](auto word){
            return (word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id));
		});
        
        matched_words.erase(it1, matched_words.end());
        sort(execution::par, matched_words.begin(), matched_words.end());
        auto it = unique(execution::par, matched_words.begin(), matched_words.end());
        matched_words.erase(it, matched_words.end());
        return {matched_words, documents_.at(document_id).status};
    }

bool SearchServer::IsStopWord(const string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const string_view word) {
   return none_of(word.begin(), word.end(), [](char c) {return c >= '\0' && c < ' ';});
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const{
    const static map<string_view, double> words_freque;
    if (find(document_ids_.begin(), document_ids_.end(), document_id) == document_ids_.end()){
        return words_freque;
    }
        
    return words_freqs.at(document_id);
    }

void SearchServer::RemoveDocument(int document_id) {
    documents_.erase(document_id);
    words_freqs.erase(document_id);

    for (auto word : word_to_document_freqs_) {
        bool is_document_present = word.second.count(document_id);
        if (is_document_present) {
            word.second.erase(document_id);
        }
    }
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id){
    return RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    auto words_it = words_freqs.find(document_id);
    std::vector<std::string_view> erase_id;
    
    if (words_it != words_freqs.end()) {
        erase_id.resize(words_it->second.size());
        std::transform(std::execution::par,
            words_it->second.begin(),
            words_it->second.end(),
            erase_id.begin(), [](auto word) {
                return word.first;
        });
     
    
    for_each(std::execution::par, erase_id.begin(), erase_id.end(), [&](auto temp){
        word_to_document_freqs_.at(temp).erase(document_id);
    });
    }
     
    words_freqs.erase(document_id);
    documents_.erase(document_id);
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view& text) const {
    vector<string_view> words;
    
    for (string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("Word "s + string(word) + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    if (text.size() == 0) {
        throw invalid_argument("Query word is empty"s);
    }
    
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw invalid_argument("Query word "s + string(text) + " is invalid");
    }

    return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(bool tf, std::string_view text) const{
    SearchServer::Query result;
    for (const string_view word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            } else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }
    
    if(tf){
        sort(result.minus_words.begin(), result.minus_words.end());
        sort(result.plus_words.begin(), result.plus_words.end());
    
        auto minus_it = unique(result.minus_words.begin(), result.minus_words.end());
        auto plus_it = unique(result.plus_words.begin(), result.plus_words.end());
    
        result.minus_words.erase(minus_it, result.minus_words.end());
        result.plus_words.erase(plus_it, result.plus_words.end());
    }
    return result;
}
   
double SearchServer::ComputeWordInverseDocumentFreq(const string_view word) const {
    return log(std::distance(document_ids_.begin(), document_ids_.end()) * 1.0 / word_to_document_freqs_.at(word).size());
}