#pragma once
namespace lstl {
struct Node {
    Node* parent;
    Node* left;
    Node* right;
    int value;
    int balance;
};

struct Set {
    Node* root = nullptr;
    Node* height = 0;

    Node* find(int value) {
        Node* current = root;
        while (current) {
            if (value < current->value) {
                current = current->left;
                continue;
            } else if (value > current->value) {
                current = current->right;
                continue;
            } else {
                return current;
            }
        }
        return current;
    }

    bool insert(int value) {
        Node* node = new Node{};
        node->value = value;
        node->balance = 0;

        Node* current = root;
        Node* last = nullptr;
        while (current) {
            if (value < current->value) {
                last = current;
                current = current->left;
                continue;
            } else if (value > current->value) {
                last = current;
                current = current->right;
                continue;
            } else {
                return false;
            }
        }
        if (!last) {
            root = node;
            node->parent = nullptr;
        } else if (value < last->value) {
            last->left = node;
            node->parent = last;
        } else if (value > last->value) {
            last->right = node;
            node->parent = last;
        } else {
            return false;
        }
        return true;
    }

    void left_rotate(Node*& ptr) {
        Node* N1 = ptr;
        Node* N2 = ptr->right;
        Node* N3 = ptr->right->left;
        ptr = N2;
        N1->right = N3;
        N2->left = N1;
    }

    void right_rotate(Node*& ptr) {
        Node* N1 = ptr;
        Node* N2 = ptr->left;
        Node* N3 = ptr->left->right;
        ptr = N2;
        N1->left = N3;
        N2->right = N1;
    }
};
} // namespace lstl
