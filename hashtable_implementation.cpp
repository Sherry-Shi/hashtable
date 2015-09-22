下面的代码实现，可以说还是算很标准的hashmap的实现，hashmap里面的hash function需要自己实现，就像priority_queue里面的comparator, hash_function也是作为一个template 参数进行定义的，需要用户自己去实现。

下面的hashmap实现， 包含：
1. getEntry, putEntry, remove, size, isEmpty()等api
2. 对hashmap经常可能出现的conflict进行了处理，用的是 separate chaining来做的；
3. 处理了当hashtable的存储空间超过了load_factor的时候，需要进行resize的需求
4. 采用了 mutex, C++的 thread 和 mutex类， 对里面的单元操作进行了lock, unlock.

从下面的实现可以看出来：
hashtable: average的getEntry/find/putEntry的时间复杂度都是O(1), key--> hash_key-->table[hash_key]
           但是如果有冲突并且冲突比较多， worst case 时间复杂度O(n), 需要遍历Linklist来寻找相应的位置。


/* the basic knowledge of datastructure and basic object-oriented class implementation */

//hashmap --> hashtable的实现 : using array of linklists with max size to implement hashtable
//use chaining to solve the conflict

template <typename K, typename V>
class HashEntry {
private:
    K key;
    V value;
    HashEntry* next;
public:
    HashEntry(K key, V value, HashEntry* next) {
        this->key = key;
        this->value = value;
        this->next = next;
    }
    
    void setKey(K key) {
        this->key = key;
    }
    
    void setValue(V value) {
        this->value = value;
    }
    
    void setNext(HashEntry* next) {
        this->next = next;
    }
    
    K getKey() const{
        return key;
    }
    
    V getValue() const{
        return value;
    }
    
    HashEntry* getNext() const {
        return next;
    }
};

const int TABLE_SIZE = 128;

//Default hash function class
template <typename K>
struct KeyHash {
    unsigned long operator() (const K& key) const {
        return reinterpret_cast<unsigned long>(key) % TABLE_SIZE;
    }
};

template <typename K, typename V, typename F = KeyHash<K>>

class HashTable {
private:
    HashEntry<K,V>** table;
    int size;
    float loadFactor;
    int Capacity;
    F hashFunc;
    mutex mtx;
public:
    HashTable( float loadFactor) {
        this->Capacity = TABLE_SIZE;
        this->loadFactor = loadFactor;
        table = new HashEntry<K,V>*[Capacity];
        for (int i = 0; i < Capacity; i++) {
            table[i] = NULL;
        }
    }
    
    ~HashTable() {
        for (int i = 0;i < Capacity; i++) {
            if (table[i] != NULL) {
                HashEntry<K,V>* tmp = table[i];
                HashEntry<K,V>* d = tmp;
                while (tmp) {
                    d = tmp;
                    tmp = tmp->getNext();
                    delete d;
                }
            }
            table[i] = NULL;
        }
        delete[] table;
    }
    
    int getSize() {
        return size;
    }
    bool isEmpty() {
        return size == 0;
    }
    void clear() {
        for (int i = 0; i < size; i++) {
            table[i] = NULL;
        }
    }
    
    bool getValue(K key, V &value) {
        mtx.lock();
        unsigned long hashValue = hashFunc(key);
        HashEntry<K,V>* entry = table[hashValue];
        while (entry) {
            if (entry->getKey() == key) {
                value =  entry->getValue();
                return true;
            }
            entry = entry->getNext();
        }
        mtx.unlock();
        return false;

    }
    void putEntry(K key, V value) {
        mtx.lock();
        unsigned long hashValue = hashFunc(key);
        HashEntry<K, V>* entry = table[hashValue];
        HashEntry<K, V>* pre = NULL;
        
        while (entry) {
            if (entry->getKey() == key) {
                entry->setValue(value);
                return;
            }
            pre = entry;
            entry = entry->getNext();
        }
        HashEntry<K, V> * newEntry = new HashEntry<K, V>(key, value, NULL);
        if (pre != NULL) {
            pre->setNext(newEntry);
        } else {
            //insert as first bucket element
            table[hashValue] = newEntry;
        }
        size++;
        mtx.unlock();
    }
    
    void remove(K key) {
        mtx.lock();
        unsigned long hashValue = hashFunc(key);
        HashEntry<K, V>* entry = table[hashValue];
        HashEntry<K, V>* pre = NULL;
        
        if (entry == NULL) {
            return;
        }
        while (entry) {
            if (entry->getKey() == key) {
                break;
            }
            pre = entry;
            entry = entry->getNext();
        }
        if (pre == NULL) {
            table[hashValue] = entry->getNext();
        } else {
            pre->setNext(entry->getNext());
        }
        size--;
        delete entry;
        mtx.unlock();
    }
    bool needRehash() {
        return (size + 0.0) / TABLE_SIZE >= loadFactor;
    }
    void rehashing() {
        mtx.lock();
        //create a new array with double size
        //maintain the newly created array
        HashEntry<K,V>** tmp = table;
        table = new HashEntry<K, V>[TABLE_SIZE*2];
        //traverse all the nodes in the current array, and move them to the new array using putEntry
        for (int i = 0; i < TABLE_SIZE*2; i++) {
            while (tmp[i] != NULL) {
                HashEntry<K, V> cur = tmp[i];
                tmp[i] = cur->getNext();
                unsigned long hashValue = hashFunc(cur->getKey());
                cur->setNext(table[hashValue]);
                table[hashValue] = cur;
            }
        }
        mtx.unlock();
    }
};


struct MyKeyHash {
    unsigned long operator()(const int& k) const
    {
        return k % 10;
    }
};


int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    int a[7] = {1,2,4,6,7,8,9};
    vector<int> input(a, a+7);
 // vector<int> rst = getUnion()

    
    HashTable<int, string, MyKeyHash> hmap(0.7);
    hmap.putEntry(1, "val1");
    hmap.putEntry(2, "val2");
    hmap.putEntry(3, "val3");
    
    string value;
    hmap.getValue(2, value);
    cout << value << endl;
    bool res = hmap.getValue(3, value);
    if (res)
        cout << value << endl;
    hmap.remove(3);
    res = hmap.getValue(3, value);
    if (res)  
        cout << value << endl;
    return 0;
}

