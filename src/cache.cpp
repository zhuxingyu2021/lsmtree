# include "cache.h"
# include "sstable.h"
# include "option.h"

// 从文件读取的内存缓冲区中加载IndexBlock
void TableCacheMem::LoadBuf(char *buf, size_t bufsz) {
    SSTable::LoadIndexBlockFromBuf(buf,bufsz,ib);
}

// 从文件读取的内存缓冲区中加载DataBlock
void BlockCacheMem::LoadBuf(char *buf, size_t bufsz) {
    SSTable::LoadDataBlockFromBuf(buf,bufsz,db);
}

Cache::Cache(uint8_t cachetype):cache_type(cachetype) {
    if(cache_type==CACHE_TYPE_BLOCKCACHE) max_size=Option::BLOCKCACHE_SIZE;
    else if(cache_type==CACHE_TYPE_TABLECACHE) max_size=Option::TABLECACHE_SIZE;
    else return;
}

Cache::~Cache(){
    for(auto item:LRUList){
        if(cache_type==CACHE_TYPE_BLOCKCACHE) delete (BlockCacheMem*)item;
        else if(cache_type==CACHE_TYPE_TABLECACHE) delete (TableCacheMem*)item;
        else return;
    }
}

// 查找key对应的缓存，如果找到，返回缓存地址，否则返回nullptr
CacheMem* Cache::Get(uint64_t key, uint64_t key2){
    auto iter = LRUList.begin();
    for(;iter!=LRUList.end();iter++){
        if(((*iter)->_key==key) && ((*iter)->_key2==key2)){
            break;
        }
    }
    if(((*iter)->_key!=key) || ((*iter)->_key2!=key2)){ // 未找到
        return nullptr;
    }

    // 若找到指定的缓存，根据LRU算法，需要把缓存插入到链表表头
    auto mem = *iter;
    LRUList.erase(iter);
    LRUList.insert(LRUList.begin(), mem);
    return mem;
}

// 把pointer指向的内存单元标记为缓存，并返回缓存地址
CacheMem* Cache::Put(uint64_t key, uint64_t key2, char* pointer, size_t size){
    CacheMem* cm = nullptr;
    if(cache_type==CACHE_TYPE_BLOCKCACHE) cm = new BlockCacheMem(key, key2);
    else if(cache_type==CACHE_TYPE_TABLECACHE) cm = new TableCacheMem(key, key2);
    else return nullptr;
    cm->LoadBuf(pointer, size);
    cm->sz = size;

    // LRU淘汰链表最后的元素
    while(total_size+cm->sz>max_size){
        auto last = LRUList.end();
        last--;
        total_size -= (*last)->sz;
        if(cache_type==CACHE_TYPE_BLOCKCACHE) delete (BlockCacheMem*)(*last);
        else if(cache_type==CACHE_TYPE_TABLECACHE) delete (TableCacheMem*)(*last);
        LRUList.erase(last);
    }

    // 把当前元素插入到链表表头
    total_size += cm->sz;
    LRUList.insert(LRUList.begin(), cm);
    return cm;
}

// 从缓存中删除指定项
void Cache::Evict(uint64_t key, uint64_t key2) {
    // 查找指定项
    auto iter = LRUList.begin();
    for(;iter!=LRUList.end();iter++){
        if(((*iter)->_key==key) && ((*iter)->_key2==key2)){
            break;
        }
    }
    if(((*iter)->_key!=key) || ((*iter)->_key2!=key2)) return;

    // 删除指定项
    total_size -= (*iter)->sz;
    if(cache_type==CACHE_TYPE_BLOCKCACHE) delete (BlockCacheMem*)(*iter);
    else if(cache_type==CACHE_TYPE_TABLECACHE) delete (TableCacheMem*)(*iter);
    LRUList.erase(iter);
}
