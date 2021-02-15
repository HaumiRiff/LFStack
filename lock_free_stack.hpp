#pragma once
#include <atomic>
#include <memory>
#include <optional>

template<typename T>
class lock_free_stack {
private:
    struct node {
        T data;
        node *next;

        explicit node(T &data_) :
                data(data_) {}
    };
    std::atomic<node*> head;
    std::atomic<node*> delete_list;

    void deletion(node* n){
        while(n){
            node* next = n->next;
            delete n;
            n = next;
        }
    }
    void link_to_delete_list(node* first, node* last) {
        last->next = delete_list;
        while (!delete_list.compare_exchange_weak(last->next, first));
    }



public:
    lock_free_stack(const lock_free_stack &) = delete;
    lock_free_stack(lock_free_stack &&) = delete;

    lock_free_stack(): head(nullptr),
        delete_list(nullptr)
    {}

    ~lock_free_stack() {
        deletion(head);
        deletion(delete_list);
    }

    bool empty() {
        return (!head);
    }

    void push(T&& data){

        node* const new_node = new node(data);
        new_node->next = head.load(std::memory_order_relaxed);
        while(!head.compare_exchange_weak(new_node->next,new_node,
                std::memory_order_release,std::memory_order_relaxed));
    }

    std::optional<T> pop() {

        node* old_head = head.load(std::memory_order_relaxed);
        while(old_head && !head.compare_exchange_weak(old_head,old_head->next,
                std::memory_order_acquire,std::memory_order_relaxed));

        std::optional<T> result;

        if(old_head) {
            result.emplace(old_head->data);
            link_to_delete_list(old_head, old_head);
            return result;
        }
        else return std::nullopt;

    }
};
