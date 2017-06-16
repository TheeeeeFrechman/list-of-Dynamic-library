#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#define Public
#define Private

typedef uint32_t (* compare_func)(void *data, void *user_data);
typedef void (* traversal_func)(void *data, void *user_data);

/**
 * typedef list node structure
 */
typedef struct list_node
{
    struct list_node *pre;
    struct list_node *next;
    void *data;
    size_t data_size;
}list_node;

/**
 * typedef list enviroment varibale structure
 * includes list head and tail pointer, head node pointer
 */
typedef struct link_list
{
    list_node *p_head;
    list_node *p_tail;
    size_t node_count;              //num of the list node, standby
    size_t used_node_count;         //num of the used list node, standby
    pthread_rwlock_t list_lock;

    Public void (* traversal_list)(struct link_list *list, traversal_func traversal_func_routine, void *user_data);
    Public void (* memset_list_node_data)(struct link_list *list);
    Public void (* free_list)(struct link_list *list);
    Public uint32_t (* insert_before)(struct link_list *list, list_node *current_node, list_node *insert_node);
    Public uint32_t (* insert_after)(struct link_list *list, list_node *current_node, list_node *insert_node);
    Public void (* delete_node)(struct link_list *list, list_node *del_node);
    Public list_node* (* search_node)(struct link_list *list, compare_func compare_func_routine, void *user_data);
    Public list_node* (* pop_node)(struct link_list *list);
    Public uint32_t (* push_new_node)(struct link_list *list, const void *node_data, const size_t node_data_size);
}link_list;

/**
 * @brief      Creates list enviroment variable and list.
 *
 * @param      list            The list enviroment variable pointer
 * @param[in]  node_count      The node count, can be 0
 * @param[in]  node_data_size  The node data size, indicate the data's size, can not be 0
 *
 * @return     list enviroment varibale pointer
 */
struct link_list *create_list(struct link_list *list, const size_t node_count, const size_t node_data_size);

/**
 * @brief      traversal list to do something
 *
 * @param      list                    The list
 * @param[in]  traversal_func_routine  The traversal function routine
 * @param      user_data               The user data pass to the func(param2)
 */
void traversal_list(struct link_list *list, traversal_func traversal_func_routine, void *user_data);

/**
 * @brief      memset each node's data of the list
 *
 * @param      list  The list
 */
void memset_list_node_data(struct link_list *list);

/**
 * @brief      free list node
 *
 * @param      data       The data, transform to list node *
 * @param      user_data  The user data
 */
void free_list_node(void *data, void *user_data);

/**
 * @brief      free list pointed by list->head_node, and memset list
 *
 * @param      list  The list
 */
void free_list(struct link_list *list);

/**
 * @brief      insert a node before current node
 *
 * @param      list The list
 * @param      current_node  The current node
 * @param      insert_node   The insert node
 *
 * @return     list or current_node or insert_node is NULL return 1 indicates failure, success return 0
 */
uint32_t insert_before(struct link_list *list, list_node *current_node, list_node *insert_node);

/**
 * @brief      insert a node after current node
 *
 * @param      list  The list
 * @param      current_node  The current node
 * @param      insert_node   The insert node
 *
 * @return     list or current_node or insert_node is NULL return 1 indicates failure, success return 0
 */
uint32_t insert_after(struct link_list *list, list_node *current_node, list_node *insert_node);

/**
 * @brief      delete a node from list
 *
 * @param[in]  list  The list
 * @param      del_node   The delete node
 */
void delete_node(struct link_list *list, list_node *del_node);

/**
 * @brief      search a node from list
 *
 * @param      list          The list
 * @param[in]  comp_func_routine  The compare function routine
 * @param      user_data          The user data
 *
 * @return     if list or list->head_node is NULL, or compare_func_routine is NULL, return NULL indicates failure
 *                else excute compare_func_routine to find node, if found return the node's addr, else return NULL
 */
list_node *search_node(struct link_list *list, compare_func compare_func_routine, void *user_data);

/**
 * @brief      pop a node
 *
 * @param      list  The list
 *
 * @return     if head node is not null, return head node, and p_head move to next node or be null
 *                else return null
 */
list_node *pop_node(struct link_list *list);

/**
 * @brief      create a new node and init with node data and data size when node data is not NULL and data size > 0
 *
 * @param      list         The list
 * @param      insert_node  The node data
 * @param      node_data_size The node data size
 *
 * @return     success return 0, failed return 1
 */
uint32_t push_new_node(struct link_list *list, const void *node_data, const size_t node_data_size);
