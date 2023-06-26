#include "process_queries.h"

using namespace std;

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries){
    vector<vector<Document>> result(queries.size());
    transform(std::execution::par, queries.begin(), queries.end(), result.begin(), [&search_server](const auto& temp){return search_server.FindTopDocuments(temp);});
    return result;
}

list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries){
    list<Document> results;
    for(const auto& vec_doc : ProcessQueries(search_server, queries)){
        for(const auto& doc : vec_doc){
            results.push_back(doc);
        }
    }
    return results;
}