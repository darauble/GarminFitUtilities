#pragma once

#include <stdexcept>
#include <unordered_map>

namespace darauble::containers {

template <typename K, typename V>

class BiMap {
private:
    std::unordered_map<K, V> forwardMap;
    std::unordered_map<V, K> reverseMap;

public:
    BiMap() = default;

    BiMap(std::initializer_list<std::pair<const K, V>> initializerList) {
        for (const auto& pair : initializerList) {
            insert(pair.first, pair.second);
        }
    }

    void insert(const K key, const V value) {
        if (forwardMap.contains(key) || reverseMap.contains(value)) {
            throw std::invalid_argument("Duplicate key or value in BiMap");
        }
        forwardMap[key] = value;
        reverseMap[value] = key;
    }

    V atKey(K key) const {
        return forwardMap.at(key);
    }

    K atValue(V value) const {
        return reverseMap.at(value);
    }

    bool containsKey(K key) const {
        return forwardMap.contains(key);
    }

    bool containsValue(V value) const {
        return reverseMap.contains(value);
    }

    void eraseByKey(K key) {
        V value = forwardMap[key];
        forwardMap.erase(key);
        reverseMap.erase(value);
    }

    void eraseByValue(V value) {
        K key = reverseMap[value];
        reverseMap.erase(value);
        forwardMap.erase(key);
    }
};

} // namespace darauble
