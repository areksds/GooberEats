#ifndef EXPANDABLE_HASHMAP
#define EXPANDABLE_HASHMAP

#include <vector>
#include <list>
#include <utility>

// ExpandableHashMap.h

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    // Functions
    unsigned int bucketNumber(const KeyType& key) const;
    void rehash();
    
    // Data members
    int m_buckets = 8;
    int m_size = 0;
    double maximumLoadFactor;
    std::vector<std::list<std::pair<KeyType,ValueType>>> m_map = std::vector<std::list<std::pair<KeyType,ValueType>>>(8);
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor) : maximumLoadFactor(maximumLoadFactor) {
   
    if (maximumLoadFactor < 0)
        maximumLoadFactor = 0.5;
    
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    for (auto i = m_map.begin(); i != m_map.end(); i++)
    {
        for (auto it = i->begin(); it != i->end(); it++)
        {
            i->erase(it);
        }
        m_map.erase(i);
    }
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    ExpandableHashMap* temp = new ExpandableHashMap(maximumLoadFactor);
    m_map.swap(*temp);
    delete temp;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_size;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    ValueType* ptr = find(key);
    if (ptr != nullptr)
        *ptr = value;
    else
    {
        m_size++;
        int bucket = bucketNumber(key);
        m_map[bucket].push_back(std::pair<KeyType, ValueType>(key,value));
        if ((static_cast<double>(m_size) / m_buckets) > maximumLoadFactor)
            rehash();
    }
    
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    int bucket = bucketNumber(key);
    for (auto it = m_map[bucket].begin(); it != m_map[bucket].end(); it++)
    {
       if (it->first == key)
           return &(it->second);
    }
    return nullptr;
}

// PRIVATE FUNCTIONS

template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::bucketNumber(const KeyType& key) const
{
    unsigned int hasher(const KeyType& key);
    unsigned int h = hasher(key);
    return h % m_buckets;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::rehash()
{
    std::vector<std::list<std::pair<KeyType,ValueType>>> tempmap = std::vector<std::list<std::pair<KeyType,ValueType>>>(m_buckets*2);
    for (int i = 0; i < m_map.size(); i++)
    {
        for (auto it = m_map[i].begin(); it != m_map[i].end(); it++)
        {
            unsigned int hasher(const KeyType& key);
            unsigned int h = hasher(it->first);
            tempmap[h % (m_buckets*2)].push_back(*it);
        }
    }
    m_buckets *= 2;
    m_map.swap(tempmap);
}

#endif

