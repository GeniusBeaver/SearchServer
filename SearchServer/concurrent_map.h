#pragma once
#include <map>
#include <mutex>
#include <vector>
#include <cstdlib>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
    std::lock_guard<std::mutex> guard;
    Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
    :
    mtx_(bucket_count),
    map_(bucket_count),
    bucket_count_(bucket_count)
    {
    }

    Access operator[](const Key& key){
    auto index = static_cast<uint64_t>(key) % bucket_count_;
    std::map<Key, Value>& bucket = map_[index];
        return {std::lock_guard(mtx_[index]), bucket[key]};
    }
    
    void Erase(const Key& key){
        auto index = static_cast<uint64_t>(key) % bucket_count_;
        std::lock_guard guard(mtx_[index]);
        map_[index].erase(key);
    }

    std::map<Key, Value> BuildOrdinaryMap(){
        std::map<Key, Value> result;
        for (int i = 0; i < bucket_count_; ++i){
            std::lock_guard guard_(mtx_[i]);
    result.insert(map_[i].begin(), map_[i].end());
        }
        return result;
    }

private:
    std::vector<std::mutex> mtx_;
    std::vector<std::map<Key, Value>> map_;
    //std::mutex mtx_;
    int bucket_count_;
};