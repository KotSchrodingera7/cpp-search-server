#pragma once

#include <map>
#include <mutex>
#include <vector>

using namespace std::string_literals;


template <typename Key, typename Value>
struct BucketMap {
    std::map<Key, Value> map_;
    std::mutex mutex_map;
};


template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) :  current_map_(bucket_count) {
    };

    Access operator[](const Key& key) {
        int offset = ((uint64_t)key) % current_map_.size();

        struct BucketMap<Key, Value> &data = current_map_[offset];


        return { std::lock_guard(data.mutex_map), data.map_[key] };
    };
    void erase(const Key& key) {
        int offset = ((uint64_t)key) % current_map_.size();
        struct BucketMap<Key, Value> &data = current_map_[offset];

        std::lock_guard(data.mutex_map);

        data.map_.erase(key);
    }
    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for(auto &data: current_map_) {
            std::lock_guard<std::mutex> guard = std::lock_guard(data.mutex_map);
            result.insert(data.map_.begin(), data.map_.end());
        }

        return result;
    };

private:

    std::vector<struct BucketMap<Key, Value>> current_map_;
};