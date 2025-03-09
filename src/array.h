#ifndef _ARRAY_H_
#define _ARRAY_H_

struct Array {
    void *items;
    u32 count;
    u32 capacity;
    size_t itemSize;
    
    Allocator allocator;
};

#define InitArray(arr, type, setCapacity, backingAllocator) \
    do { \
        (arr).items = (backingAllocator).alloc(setCapacity * sizeof(type)); \
        (arr).count = 0; \
        (arr).capacity = setCapacity; \
        (arr).itemSize = sizeof(type); \
        (arr).allocator = (backingAllocator); \
    } while (0)

void* Next(Array* arr) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        void* newItems = arr->allocator.alloc(arr->capacity * arr->itemSize);
        memcpy(newItems, arr->items, arr->count * arr->itemSize);
        arr->allocator.free(arr->items);
        arr->items = newItems;
        
        printf("Resized array to %u capacity\n", arr->capacity);
    }

    void* item = (u8*)arr->items + arr->count * arr->itemSize;
    arr->count++;
    return item;
};

void* GetElement(Array* arr, u32 index) {
    if (index >= arr->count) {
        return 0;
    }
    return (u8*)arr->items + index * arr->itemSize;
}

#endif //_ARRAY_H_