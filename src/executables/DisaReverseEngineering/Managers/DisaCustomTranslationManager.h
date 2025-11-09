#pragma once

#include <Disa/PG_DisaIdentifier.h>
#include <unordered_map>
#include <list>

struct PG_DisaModLoadInfo;
struct PG_DisaModInfo;


template <typename T, size_t MaxSize = 128>
struct BoundedQueueMap {
    // Types
    using KeyType = std::string;
    using ValueType = T;

    // Main data
    std::unordered_map<KeyType, std::pair<ValueType, typename std::list<KeyType>::iterator>> map;
    std::list<KeyType> order;

    // Insert or update a value
    void insert(const KeyType& key, const ValueType& value) {
        auto it = map.find(key);
        if (it != map.end()) {
            // Key already exists — update value, move to back
            order.erase(it->second.second);
            order.push_back(key);
            it->second = { value, std::prev(order.end()) };
        }
        else {
            // New key — insert
            if (map.size() >= MaxSize) {
                // Remove oldest
                const auto& oldest = order.front();
                map.erase(oldest);
                order.pop_front();
            }
            order.push_back(key);
            map[key] = { value, std::prev(order.end()) };
        }
    }

    ValueType& emplace(const KeyType& key) {
        auto it = map.find(key);
        if (it != map.end()) {
            // Key already exists — update value, move to back
            order.erase(it->second.second);
            order.push_back(key);
            it->second = { {}, std::prev(order.end()) };
            return it->second.first;
        }

        if (map.size() >= MaxSize) {
            // Remove oldest
            const auto& oldest = order.front();
            map.erase(oldest);
            order.pop_front();
        }
        order.push_back(key);
        auto& v = map[key];
        v = { {}, std::prev(order.end()) };
        return v.first;
    }

    // Access by key (throws if not found)
    ValueType* at(const KeyType& key) {
        auto it = map.find(key);
        if (it == map.end()) return nullptr;
        return &it->second.first;
    }

    // Check existence
    bool contains(const KeyType& key) const {
        return map.find(key) != map.end();
    }

    // Current size
    size_t size() const {
        return map.size();
    }

    // Remove key
    bool erase(const KeyType& key) {
        auto it = map.find(key);
        if (it == map.end()) return false;
        order.erase(it->second.second);
        map.erase(it);
        return true;
    }

    // Clear all
    void clear() {
        map.clear();
        order.clear();
    }
};

class DisaCustomTranslationManager{
public:
	void test();
	bool translateMark(char*& shiftJis);
	static bool isTranslationMarked(char* shiftJis);
	std::string& getUtf8FromMark(char* shiftJis);

	char* translate(char* shiftJis);

	static DisaCustomTranslationManager* get();

protected:
	std::unordered_map<std::string, std::string> m_translations; //MD5 hash, translation utf8
	unsigned short m_currentMarkSingle = 0x01; //single byte replacement
	unsigned short m_currentMark = 0x8600; 
	unsigned short m_last = 0; 
	using TranslationIt = std::unordered_map<std::string, std::string>::iterator;
	struct TraslationInfo {
		unsigned short originalLength;
		unsigned short newLength;
		TranslationIt it;
		std::vector<char> copy;
	};
	std::unordered_map<unsigned short, TraslationInfo> m_markers; //marker, [originalSize, taranslation]
	std::string m_stringBuffer;

    BoundedQueueMap<TraslationInfo> m_translationsMemory;
	unsigned short m_currSwiftJis = 0x8500;
	std::unordered_map<uint32_t, unsigned short> m_utf8ToSwiftJis;
	std::unordered_map<unsigned short, uint32_t> m_swiftJisToUtf8;
	unsigned short m_translationMemIndex = 0;
};