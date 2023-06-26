#pragma once
#include <deque>
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        SwitchTime();
        QueryResult newn;
        newn.time = 0;
        newn.count_doc = server_.FindTopDocuments(raw_query, document_predicate).size();
        requests_.push_front(newn);
        return server_.FindTopDocuments(raw_query, document_predicate);
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        int time;
        int count_doc;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& server_;
    
    void SwitchTime();
}; 


    