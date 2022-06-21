#ifndef LSMTREE_CACHE_H
#define LSMTREE_CACHE_H

#include <cstdint>
#include <cstddef>
#include <list>
#include <string>
#include <map>

class CacheMem{
public:
    CacheMem(uint64_t key, uint64_t key2):_key(key),_key2(key2){}
    virtual void LoadBuf(char* buf, size_t bufsz)=0;

    friend class Cache;
private:
    size_t sz; // 缓存区大小
    uint64_t _key;
    uint64_t _key2;
};

class TableCacheMem: public CacheMem{
public:
    TableCacheMem(uint64_t key, uint64_t key2):CacheMem(key, key2){}
    void LoadBuf(char* buf, size_t bufsz);
    std::map<uint64_t,std::pair<size_t,size_t>> ib; // 缓存IndexBlock
};

class BlockCacheMem: public CacheMem{
public:
    BlockCacheMem(uint64_t key, uint64_t key2): CacheMem(key, key2){}
    void LoadBuf(char* buf, size_t bufsz);
    std::map<uint64_t,std::string> db; // 缓存DataBlock
};

// 采用LRU算法实现TableCache和BlockCache
class Cache{
public:
    explicit Cache(uint8_t cachetype);
    ~Cache();
    CacheMem* Get(uint64_t key, uint64_t key2);
    CacheMem* Put(uint64_t key, uint64_t key2, char* pointer, size_t size);
    void Evict(uint64_t key, uint64_t key2);

    static const uint8_t CACHE_TYPE_BLOCKCACHE=0;
    static const uint8_t CACHE_TYPE_TABLECACHE=1;
private:
    uint8_t cache_type;
    std::list<CacheMem*> LRUList;
    size_t total_size; // 总缓存大小
    size_t max_size; // 缓存大小上限
};

#endif //LSMTREE_CACHE_H
