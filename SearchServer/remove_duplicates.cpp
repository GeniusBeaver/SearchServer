#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server){
    LOG_DURATION("RemoveDUP");
    map<vector<string>, vector<int>> doc;
 
    for (const int document_id : search_server) {
        vector<string> temp;
        auto str_and_freque = search_server.GetWordFrequencies(document_id);
        
        for (const auto& words : str_and_freque){
            temp.push_back(words.first);
        }
        
        doc[temp].push_back(document_id);
    }
    
    for (auto& i : doc){
        if (i.second.size() == 2){
            cout << "Found duplicate document id " << *i.second.rbegin() << endl;
            search_server.RemoveDocument(*i.second.rbegin());
        } else if (i.second.size() > 2){
            for (auto j = i.second.begin() + 1; j < i.second.end(); j++){
                cout << "Found duplicate document id " << *j << endl;
                search_server.RemoveDocument(*j);
            }
        }
        
    }
    
}