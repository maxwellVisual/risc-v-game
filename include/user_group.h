#pragma once

#include "risc-v.h"
#include "privilege.h"

#define PRIVILEGED_GROUP_ROOT_ID ((privilege_id_t)0)

typedef struct{
    privilege_id_t id;
    uint64_t member_count;
    size_t member_size;
    privilege_id_t* members;
    RV_Env* env;
} privileged_group_t;

typedef map_t privileged_group_map_t;

extern int privileged_group_init(privileged_group_t* group, privilege_id_t id);
extern void privileged_group_destroy(privileged_group_t* a);

extern int privileged_groups_init(privileged_group_map_t* map);
extern void privileged_groups_deinit(privileged_group_map_t* map);
extern privileged_group_t* privileged_groups_get(privileged_group_map_t* map, privilege_id_t id);
extern privileged_group_t* privileged_groups_add_user(privileged_group_map_t* map, privilege_id_t user_id);
extern void privileged_groups_remove(privileged_group_map_t* map, privilege_id_t id);
