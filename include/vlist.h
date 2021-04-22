#ifndef VLIST
#define VLIST

#include <stddef.h>

#define __VLIST_NODE_STRUCT_TYPE struct{void*prev;void*next;}
#define VLISTNODE __VLIST_NODE_STRUCT_TYPE ;

#define VLIST_ERROR_INVALID_INDEX -1
#define VLIST_ERROR_MALLOC_FAIL -2

typedef struct vlist* vlist;

// return non-zero to break
typedef int VLIST_RUNNABLE_FUNC_TYPE(vlist this, long i);

typedef void* VLIST_GET_FUNC_TYPE(vlist this, long index);
typedef const void* VLIST_GET_CONST_FUNC_TYPE(vlist this, long index);
typedef int VLIST_ADD_FUNC_TYPE(vlist this, const void* node);
typedef void VLIST_QUICK_ADD_FUNC_TYPE(vlist this, void* node);
typedef int VLIST_INSERT_FUNC_TYPE(vlist this, long index, const void* node);
typedef int VLIST_QUICK_INSERT_FUNC_TYPE(vlist this, long index, void* node);
typedef int VLIST_REMOVE_FUNC_TYPE(vlist this, long index);
typedef void VLIST_FOREACH_FUNC_TYPE(vlist this, VLIST_RUNNABLE_FUNC_TYPE* run);

struct vlist
{
    void* current;
    long current_idx;
    long size;
    size_t node_size;

    VLIST_GET_FUNC_TYPE* get;
    VLIST_GET_CONST_FUNC_TYPE* get_const;
    VLIST_ADD_FUNC_TYPE* add;
    VLIST_QUICK_ADD_FUNC_TYPE* quick_add;
    VLIST_INSERT_FUNC_TYPE* insert;
    VLIST_QUICK_INSERT_FUNC_TYPE* quick_insert;
    VLIST_REMOVE_FUNC_TYPE* remove;
    VLIST_FOREACH_FUNC_TYPE* foreach;
};

vlist make_vlist(size_t node_size);
void delete_vlist(vlist vlist_, vlist* vlist_ptr);

void* vlist_get(vlist this, long index);
const void* vlist_get_const(vlist this, long index);
int vlist_add(vlist this, const void* node);
void vlist_quick_add(vlist this, void* node);
int vlist_insert(vlist this, long index, const void* node);
int vlist_quick_insert(vlist this, long index, void* node);
int vlist_remove(vlist this, long index);
void vlist_foreach(vlist this, VLIST_RUNNABLE_FUNC_TYPE* run);

#endif // VLIST