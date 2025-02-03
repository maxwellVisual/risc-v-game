#include "user_group.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "arrays.h"

int privileged_group_init(privileged_group_t* group, privilege_id_t id){
    group->id = id;
    group->member_count = 0;
    group->member_size = 0;
    group->members = NULL;
    return 0;
}
int privileged_group_add_member(privileged_group_t* group, privilege_id_t id){
    array_t tmp = {
        .capacity = group->member_size, 
        .count = group->member_count,
        .size = sizeof(privilege_id_t),
        .data = (void*)group->members
    };

    int ret = array_push(&tmp, (void*)id);
    group->member_size = tmp.capacity;
    group->member_count = tmp.count;
    group->members = tmp.data;
    return ret;
}
static int privileged_group_compare(privileged_group_t* a, privileged_group_t* b){
    return a->id - b->id;
}
static uint32_t privileged_group_hash(void* a){
    return (uint32_t)(privilege_id_t)a;
}
void privileged_group_destroy(privileged_group_t* a){
    free(a->members);
}

int privileged_groups_init(privileged_group_map_t* map){
    return map_init(map, (comparator_func_t*)privileged_group_compare, (hash_func_t*)privileged_group_hash);
}
void privileged_groups_deinit(privileged_group_map_t* map){
    map_destroy(map);
}
privileged_group_t* privileged_groups_get(privileged_group_map_t* map, privilege_id_t id){
    return (privileged_group_t*)map_get(map, (void*)id);
}
privileged_group_t* privileged_groups_add_user(privileged_group_map_t* map, privilege_id_t user_id){
    privileged_group_t* group = (privileged_group_t*)malloc(sizeof(privileged_group_t));
    group->id = user_id;
    group->member_count = 0;
    group->members = NULL;
    group->member_size = 0;
    return (privileged_group_t*)map_set(map, (void*)user_id, group);
}
void privileged_groups_remove(privileged_group_map_t* map, privilege_id_t id){
    privileged_group_t* group = (privileged_group_t*)map_remove(map, (void*)id, NULL);
    if(group != NULL){
        privileged_group_destroy(group);
        free(group);
    }
}