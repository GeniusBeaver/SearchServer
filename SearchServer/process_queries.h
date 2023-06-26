#pragma once
#include "search_server.h"
#include <functional>
#include <execution>
#include <algorithm>
#include <vector>
#include <list>
#include "document.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries); 

std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);