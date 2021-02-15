#include "lock_free_stack.hpp"
#include <cassert>
#include <future>
#include <iostream>


class Test{
    
public:

    //Самые простые тесты на базовую функциональность
    static void test_lock_free_stack_push(){
        lock_free_stack<int> stack;
        stack.push(20);
        assert(!stack.empty());
    }
    static void test_lock_free_stack_pop(){
        lock_free_stack<int> stack;
        stack.push(20);
        stack.push(40);
        auto num = stack.pop();
        assert(num.value() == 40);
        num = stack.pop();
        assert(num.value() == 20);
    }
    static void test_lock_free_stack_empty_pop(){
        lock_free_stack<std::string> stack;
        auto num = stack.pop();
        assert(!num.has_value());
    }


    //Проверка на нескольких потоках
    static void test_lock_free_stack_push_pop(){
        lock_free_stack<int> stack;

        std::promise<void> push_ready, pop_ready, start;
        std::shared_future<void> ready(start.get_future());

        auto push_done = std::async(std::launch::async,
                               [&stack, ready, &push_ready]() {
                                           push_ready.set_value();
                                           ready.wait();
                                           stack.push(1);
                            }
        );
        auto pop_done = std::async(std::launch::async,
                            [&stack,ready,&pop_ready](){
                                pop_ready.set_value();
                                ready.wait();
                                return stack.pop();
                            }
        );
        push_ready.get_future().wait();
        pop_ready.get_future().wait();

        start.set_value();
        push_done.get();
        auto popped_out = pop_done.get();

        assert((!popped_out.has_value() && !stack.empty())
                    || (popped_out.value() == 1 && stack.empty()) );
    }


    static void test_lock_free_stack_double_push(){
        lock_free_stack<int> stack;

        std::promise<void> push_ready_1, push_ready_2, start;
        std::shared_future<void> ready(start.get_future());

        auto push_done_1 = std::async(std::launch::async,
                               [&stack, ready, &push_ready_1]() {
                                   push_ready_1.set_value();
                                   ready.wait();
                                   stack.push(1);
                               }
        );
        auto push_done_2 = std::async(std::launch::async,
                              [&stack,ready,&push_ready_2](){
                                  push_ready_2.set_value();
                                  ready.wait();
                                  stack.push(2);
                              }
        );
        push_ready_1.get_future().wait();
        push_ready_2.get_future().wait();

        start.set_value();
        push_done_1.get();
        push_done_2.get();

        auto first = stack.pop(), second = stack.pop();
        assert(stack.empty() &&
            (first == 1 && second == 2 || first == 2 && second == 1));
    }


    static void test_lock_free_stack_double_pop(){
        lock_free_stack<int> stack;
        stack.push(1);
        stack.push(2);

        std::promise<void> start, pop_ready_1, pop_ready_2;
        std::shared_future<void> ready(start.get_future());

        auto pop_done_1 = std::async(std::launch::async,
                                 [&stack,ready,&pop_ready_1](){
                                     pop_ready_1.set_value();
                                     ready.wait();
                                     return stack.pop();
                                 }
        );
        auto pop_done_2 = std::async(std::launch::async,
                                 [&stack,ready,&pop_ready_2](){
                                     pop_ready_2.set_value();
                                     ready.wait();
                                     return stack.pop();
                                 }
        );
        pop_ready_1.get_future().wait();
        pop_ready_2.get_future().wait();

        start.set_value();
        auto p1 = pop_done_1.get(), p2 = pop_done_2.get();
        assert(stack.empty() && (p1 == 1 && p2 == 2 || p1 == 2 && p2 == 1));
    }
};

int main(){
        Test::test_lock_free_stack_push();
        Test::test_lock_free_stack_pop();
        Test::test_lock_free_stack_empty_pop();

        Test::test_lock_free_stack_push_pop();
        Test::test_lock_free_stack_double_push();
        Test::test_lock_free_stack_double_pop();

        std::cout << "All tests were passed!";
}