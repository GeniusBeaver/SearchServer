#include "request_queue.h"
using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server): server_(search_server) {
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    SwitchTime();
    QueryResult newn;
    newn.time = 0;
    newn.count_doc = server_.FindTopDocuments(raw_query, status).size();
    requests_.push_front(newn);
    
    return server_.FindTopDocuments(raw_query, status);
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    SwitchTime();
    QueryResult newn;
    newn.time = 0;
    newn.count_doc = server_.FindTopDocuments(raw_query).size();
    requests_.push_front(newn);
    
    return server_.FindTopDocuments(raw_query);
}

int RequestQueue::GetNoResultRequests() const {
    return std::count_if(requests_.begin(), requests_.end(), [](const auto& i){return (i.count_doc == 0 && i.time < min_in_day_);});
}
    
void RequestQueue::SwitchTime(){
    for (auto& timmings : requests_){
        timmings.time++;
    }
}