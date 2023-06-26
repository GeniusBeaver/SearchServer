#pragma once
#include "log_duration.h"
#include "request_queue.h"


std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(SearchServer& test,const std::string& raw_query, int document_id);

std::vector<Document> FindTopDocuments(SearchServer& test, const std::string& raw_query);

void AddDocument(SearchServer& test, int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings);
