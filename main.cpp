/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 产生了循环引用，a 指向 b，b 指向 a，引用计数无法清零
    // std::shared_ptr<Node> next;
    std::unique_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    // std::weak_ptr<Node> prev;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    Node(int value) : value(value) {}  // 有什么可以改进的？

    void insert(int value) {
        auto node = std::make_unique<Node>(value);
        node->prev = prev;
        prev->next->prev = node.get();
        node->next = std::move(prev->next);
        prev->next = std::move(node);
    }

    void erase() {
        if (next)   // 先处理裸指针
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
            // prev->next = next;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
        // 产生了循环引用
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        head = std::make_unique<Node>(other.head->value);
        auto tail = head.get(); // tail 用于追踪 this
        auto p = other.front()->next.get();   // p 用于追踪 other
        while(p != nullptr) {
            auto node = std::make_unique<Node>(p->value);
            node->prev = tail;
            tail->next = std::move(node);

            tail = tail->next.get();
            p = p->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 当需要调用此函数时，编译器会构造一个临时对象，随后调用移动赋值函数

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next); // = head->next;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if (head)
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

// 使用 const 引用避免传参时拷贝
void print(const List& lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]    // ok

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]     // ok

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};
    // a 和 b 一共 11 次析构
    // valgrind: no leaks are possible

    return 0;
}
