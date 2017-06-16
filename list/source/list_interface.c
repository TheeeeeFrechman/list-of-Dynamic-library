#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/list_interface.h"
//#include "./list_interface.h"

static void init_list(link_list *list);
static void memset_node_data(void *data, void *user_data);
static list_node *create_node(size_t data_size);

struct link_list *create_list(struct link_list *list, const size_t node_count, const size_t node_data_size)
{
    if (NULL != list)
        return NULL;

    list = (link_list *)malloc(sizeof(link_list));
    if (NULL == list)
        return NULL;
    memset(list, 0, sizeof(link_list));

    list->node_count = node_count;

    if (node_count > 0)
    {
        //init head node
        list->p_head = create_node(node_data_size);
        if (NULL == list->p_head)
            return NULL;

        list->p_tail = list->p_head;
        int i = 1;
        list_node *tmp_node = NULL;
        for (; i < node_count; i++)
        {
            if (NULL == (tmp_node = create_node(node_data_size)))
                return NULL;
            if (0 != insert_after(list, list->p_tail, tmp_node))
                return NULL;
            tmp_node = NULL;
        }
    }
    init_list(list);
    return list;
}

void init_list(struct link_list *list)
{
    if (0 != pthread_rwlock_init(&(list->list_lock), NULL))
    {
        printf("init list lock failed.\n");
        return;
    }
    list->traversal_list = traversal_list;
    list->memset_list_node_data = memset_list_node_data;
    list->free_list = free_list;
    list->insert_before = insert_before;
    list->insert_after = insert_after;
    list->delete_node = delete_node;
    list->search_node = search_node;
    list->pop_node = pop_node;
    list->push_new_node = push_new_node;
}

void traversal_list(struct link_list *list, traversal_func traversal_func_routine, void *user_data)
{
    if (NULL == list || NULL == traversal_func_routine)
        return;

    pthread_rwlock_wrlock(&(list->list_lock));
    list_node *tmp_head = list->p_head;
    list_node *tmp_node = tmp_head;
    while (NULL != tmp_node)
    {
        tmp_head = tmp_node->next;
        traversal_func_routine(tmp_node, user_data);
        tmp_node = tmp_head;
    }
    pthread_rwlock_unlock(&(list->list_lock));
}

void memset_node_data(void *data, void *user_data)
{
    if (NULL == data)
        return;

    list_node *clear_node = (list_node *)data;
    memset(clear_node->data, 0, clear_node->data_size);
}

void memset_list_node_data(struct link_list *list)
{
    traversal_list(list, memset_node_data, NULL);
}

void free_list_node(void *data, void *user_data)
{
    if (NULL == data)
        return;

    list_node *free_node = (list_node *)data;
    if (NULL != free_node->data)
    {
        free(free_node->data);
        free_node->data = NULL;
    }
    free_node->pre = free_node->next = NULL;
    free(free_node);
}

//free list indicated by list->p_head、and list->p_tail
//first: travesal list to free each node, 
//second: set list environment variable to 0
void free_list(struct link_list *list)
{
    if (NULL == list)
        return;

    traversal_list(list, free_list_node, NULL);
    pthread_rwlock_wrlock(&(list->list_lock));
    list->p_head = list->p_tail = NULL;
    list->node_count = 0;
    pthread_rwlock_unlock(&(list->list_lock));
}

list_node *create_node(size_t data_size)
{
    list_node *new_node = (list_node *)malloc(sizeof(list_node));
    if (NULL == new_node)
        return NULL;
    memset(new_node, 0 ,sizeof(list_node));
    if (data_size > 0)
    {
        if (NULL == (new_node->data = malloc(data_size)))
            return NULL;
        memset(new_node->data, 0, data_size);
        new_node->data_size = data_size;
    }
    return new_node;
}

/*
uint32_t forward_insert(list_node *head_node, list_node *insert_node)
{
    if (NULL == head_node || NULL == insert_node)
        return 1;

    insert_node->next = head_node->next;
    head_node->next = insert_node;
    if (NULL != head_node->next)
        head_node->next->pre = insert_node;
    insert_node->pre = head_node;

    return 0;
}

uint32_t tail_insert(list_node *tail_node, list_node *insert_node)
{
    if (NULL == tail_node || NULL == insert_node)
        return 1;

    tail_node->next = insert_node;
    insert_node->pre = tail_node;   
    return 0;
}
*/

uint32_t insert_before(struct link_list *list, list_node *current_node, list_node *insert_node)
{
    if (NULL == list || NULL == insert_node)
        return 1;

    pthread_rwlock_wrlock(&(list->list_lock));
    if (NULL == list->p_head || NULL == list->p_tail)        //list without node
        list->p_tail = list->p_head = insert_node;
    else
    {
        if (NULL == current_node)
        {
            pthread_rwlock_unlock(&(list->list_lock));
            return 1;
        }

        if (NULL == current_node->pre)  //insert before head node
        {
            insert_node->next = current_node;
            current_node->pre = insert_node;
            list->p_head = insert_node;
        }
        else
        {
            insert_node->next = current_node;
            insert_node->pre = current_node->pre;
            current_node->pre->next = insert_node;
            current_node->pre = insert_node;
        }
    }
    list->node_count++;
    pthread_rwlock_unlock(&(list->list_lock));
    return 0;
}

uint32_t insert_after(struct link_list *list, list_node *current_node, list_node *insert_node)
{
    if (NULL == list || NULL == insert_node)
        return 1;

    pthread_rwlock_wrlock(&(list->list_lock));
    if (NULL == list->p_head || NULL == list->p_tail)
        list->p_tail = list->p_head = insert_node;
    else
    {
        if (NULL == current_node)
        {
            pthread_rwlock_unlock(&(list->list_lock));
            return 1;
        }

        if (current_node->next == NULL)     //insert after tail head
        {
            current_node->next = insert_node;
            insert_node->pre = current_node;
            list->p_tail = insert_node;
        }
        else
        {
            insert_node->next = current_node->next;
            insert_node->pre = current_node;
            current_node->next->pre = insert_node;
            current_node->next = insert_node;
        }
    }
    list->node_count++;
    pthread_rwlock_unlock(&(list->list_lock));
    return 0;
}

void delete_node(struct link_list *list, list_node *del_node)
{
    if (NULL == list || NULL == del_node)
        return;

    pthread_rwlock_wrlock(&(list->list_lock));

    if (list->node_count == 0 || NULL == list->p_head)
    {
        pthread_rwlock_unlock(&(list->list_lock));
        return;
    }

    //del head node
    if (list->p_head == del_node)
    {
        list_node *new_head = list->p_head->next;
        if (NULL != new_head)
            new_head->pre = NULL;
        else
            list->p_tail = NULL;

        list->p_head = new_head;
    }
    else if (list->p_tail == del_node)
    {
        list->p_tail = del_node->pre;
        list->p_tail->next = NULL;
        del_node->pre = NULL;
    }
    else
    {
        del_node->pre->next = del_node->next;
        del_node->next->pre = del_node->pre;
        del_node->pre = del_node->next = NULL;
    }
    list->node_count--;
    pthread_rwlock_unlock(&(list->list_lock));
    return;
}

list_node *search_node(struct link_list *list, compare_func compare_func_routine, void *user_data)
{
    if (NULL == list || NULL == compare_func_routine)
        return NULL;

    pthread_rwlock_rdlock(&(list->list_lock));
    list_node *tmp_head = list->p_head;
    list_node *tmp_node = tmp_head;

    while (tmp_node != NULL)
    {
        tmp_head = tmp_node->next;
        if (0 == compare_func_routine(tmp_node, user_data))
        {
            pthread_rwlock_unlock(&(list->list_lock));
            return tmp_node;
        }
        tmp_node = tmp_head;
    }
    pthread_rwlock_unlock(&(list->list_lock));
    return NULL;
}

list_node *pop_node(struct link_list *list)
{
    if (NULL == list)
        return NULL;

    pthread_rwlock_wrlock(&(list->list_lock));
    if (NULL == list->p_head)
    {
        pthread_rwlock_unlock(&(list->list_lock));
        return NULL;
    }

    list_node *head_node = list->p_head;
    if (NULL == list->p_head->next)
        list->p_head = list->p_tail = NULL;
    else
        list->p_head = list->p_head->next;
    head_node->pre = head_node->next = NULL;
    pthread_rwlock_unlock(&(list->list_lock));
    return head_node;
}

uint32_t push_new_node(struct link_list *list, const void *node_data, const size_t node_data_size)
{
    if (NULL == list || NULL == node_data || node_data_size <= 0)
        return 1;

    list_node *new_node = create_node(node_data_size);
    if (NULL != new_node)
    {
        memcpy(new_node->data, node_data, node_data_size);
        return insert_after(list, list->p_tail, new_node);
    }
    else
        return 1;
}

/*void update_node(link_list *list, const new_node_data)
{

}*/
