#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <experimental/coroutine>
#include <experimental/generator>

using namespace std;

template <typename T>
struct BinaryTree;

template <typename T>
struct Node {
    T value = T();
    Node<T>* left = nullptr;
    Node<T>* right = nullptr;
    Node<T>* parent = nullptr;
    BinaryTree<T>* tree = nullptr;
    
    explicit Node(const T& value) : value(value) {}
    
    Node(const T& value, Node<T>* const left, Node<T>* const right)
    : value(value), left(left), right(right) {
        this->left->tree = this->right->tree = tree;
        this->left->parent = this->right->parent = this;
    }
    
    void set_tree(BinaryTree<T>* t) {
        tree = t;
        if (left) left->set_tree(t);
        if (right) right->set_tree(t);
    }
    
    ~Node() {
        if (left) delete left;
        if (right) delete right;
    }
};

template <typename T>
struct BinaryTree {
    Node<T>* root = nullptr;
    
    explicit BinaryTree(Node<T>* const root) : root(root) { root->set_tree(this); }
    
    ~BinaryTree() {
        if (root) delete root;
    }
    
    template <typename U>
    struct BinaryTreeIterator {
        Node<U>* current;
        
        explicit BinaryTreeIterator(Node<U>* current) : current(current) {}
        
        bool operator!=(const BinaryTreeIterator<U>& other) {
            return current != other.current;
        }
        
        // no recusion!
        BinaryTreeIterator<U>& operator++() {
            //finish the current node and its left sub-tree
            if (current->right) { //traverse its right sub-tree
                current = current->right;
                while (current->left) current = current->left;
            } else { //find the closest right parent
                Node<T>* p = current->parent;
                while (p && current == p->right) {
                    current = p;
                    p = p->parent;
                }
                current = p;
            }
            return *this;
        }
        
        Node<U>& operator*() { return *current; }
    };
    typedef BinaryTreeIterator<T> iterator;
    
    iterator end() { return iterator{nullptr}; }
    
    iterator begin() {
        Node<T>* n = root;
        
        if (n)
            while (n->left) n = n->left;
        return iterator{n};
    }
    
    experiemental::generator<Node<T>*> post_order(){
        return  post_order_impl(root);
    }
    
private:
    experiemental::generator<Node<T>*> post_order_impl(Node<T>* node){
        if (node){
            for (auto x: post_order_impl(node->left))
                co_yield x;
            for (auto y: post_order_impl(node->right))
                co_yield y;
            co_yield node;
        }
    }
};

int main() {
    // in order traversal
    //           me
    //          /  \
    //    mother    father
    //        /      \
    //      m'm       m'f
    BinaryTree<string> family{
        new Node<string>{"me",
            new Node<string>{"mother", new Node<string>{"mother's mother"},
                new Node<string>{"mother's father"}},
            new Node<string>{"father"}}};
    
    // in order traversal
    for (auto it = family.begin(); it != family.end(); ++it) {
        cout << (*it).value << endl;
    }
    
    // post-order traversal
    for(auto it: family.post_order())
        cout << it->value << endl;
    
    return 0;
}
