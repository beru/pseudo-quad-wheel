/* Copyright (c) 2010 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:

Retrieved from: http://en.literateprograms.org/Red-black_tree_(C)?oldid=16016
*/

#ifndef _RBTREE_H_
#define _RBTREE_H_ 1

enum rbtree_node_color { RED, BLACK };

typedef struct rbtree_node_t {
    void* key;
    void* value;
    struct rbtree_node_t* left;
    struct rbtree_node_t* right;
    struct rbtree_node_t* parent;
    enum rbtree_node_color color;
} *rbtree_node;

typedef struct rbtree_t {
    rbtree_node root;
} *rbtree;


typedef int (*rb_compare_func)(void* left, void* right);
typedef void (*rb_vfree_func)(void*ps,void *key, void* value);
typedef void (*rb_kfree_func)(void*ps,void* data);

typedef void (*rb_lookup_helper)(void *key, void* userdata);
typedef int (*rb_insert_helper)(void *key);

typedef void (*rb_vreplace_func)(void*ps,void *to, void* from);
typedef int (*rb_walk_callback)(void*ps, void *key, void *value, void *userdata);

void rbtree_module_init(void *ps,
			rb_compare_func compare, 
						rb_kfree_func kfreefunc,
						rb_vfree_func vfreefunc,
						rb_vreplace_func vreplacefunc,
						rb_lookup_helper lookuphelper,
						rb_insert_helper inserthelper);
rbtree rbtree_create(void*);
void* rbtree_lookup(rbtree t, void* key, void *userdata);
int rbtree_insert(void*,rbtree t, void* key, void* value);
void rbtree_delete(void*,rbtree t, void* key);
int rbtree_walk(void*,rbtree t, void *userdata, rb_walk_callback callback);
void rbtree_destroy(void*,rbtree t);

#endif

