// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.

#include "memory/memory_index.h"

MemoryIndex::~MemoryIndex() {
    // TODO: free every node (post-order: both children before the node).
    destroy(root_);
}

void MemoryIndex::destroy(Node* n) {
    if (!n) {
        return;
    }
    destroy(n->left);
    destroy(n->right);
    delete n;
}

void MemoryIndex::update_height(Node* n) {
    std::size_t l = height_of(n->left);
    std::size_t r = height_of(n->right);
    if (l > r){
        n->height = l + 1;
    }
    else{
        n->height = r + 1;
    }
}


int MemoryIndex::balance_factor(const Node* n) {
    return static_cast<int>(height_of(n->left)) - static_cast<int>(height_of(n->right));
}
MemoryIndex::Node* MemoryIndex::rotate_left(Node* n) {
    Node* r = n->right;
    n->right = r->left;
    r->left = n;
    update_height(n);  
    update_height(r);
    return r;
}
MemoryIndex::Node* MemoryIndex::rotate_right(Node* n) {
    Node* l = n->left;
    n->left = l->right;
    l->right = n;
    update_height(n);
    update_height(l);
    return l;
}

MemoryIndex::Node* MemoryIndex::rebalance(Node* n) {
    update_height(n);
    int bf = balance_factor(n);

    if (bf > 1) {                                   // left-heavy
        if (balance_factor(n->left) < 0) {          // LR 
            n->left = rotate_left(n->left);
        }
        return rotate_right(n);                     // LL 
    }
    if (bf < -1) {                                  // right-heavy
        if (balance_factor(n->right) > 0) {         // RL
            n->right = rotate_right(n->right);
        }
        return rotate_left(n);                      // RR
    }
    return n;
}


MemoryIndex::Node* MemoryIndex::insert_node(Node* n, MemoryRecord&& rec, bool* inserted) {
    if (!n) {
        Node* node = new Node;
        node->rec = std::move(rec);
        *inserted = true;
        return node;
    }
    if (rec.id < n->rec.id) {
        n->left = insert_node(n->left, std::move(rec), inserted);
    } else if (rec.id > n->rec.id) {
        n->right = insert_node(n->right, std::move(rec), inserted);
    } else {
        return n;   
    }
    if (!*inserted) {
        return n;   
    }
    return rebalance(n);
}

bool MemoryIndex::insert(MemoryRecord rec) {
    // TODO: walk down from root_ (smaller ids left, larger right). Equal id:
    // return false and change nothing. Otherwise attach a new Node where the
    // walk fell off the tree, ++count_, return true.
    // Clue: memory ids arrive as 1, 2, 3, ..., so the tree is one long right
    // spine. A loop handles 5,000 nodes easily; so does recursion.
    bool inserted = false;
    root_ = insert_node(root_, std::move(rec), &inserted);
    if (inserted) {
        count_++;
    }
    return inserted;
}

const MemoryRecord* MemoryIndex::find(std::uint64_t id) const {
    // TODO
    const Node* n = root_;
    while (n) {
        if (id == n->rec.id) {
            return &n->rec;
        }
        if (id < n->rec.id) {
            n = n->left;
        } else {
            n = n->right;
        }
    }
    return nullptr;
}

bool MemoryIndex::remove(std::uint64_t id) {
    // TODO: leaf, one child, two children (use the in-order successor or
    // predecessor). Spec Appendix C draws all three.
    // Clue: the successor (leftmost node of the right subtree) has no left
    // child but MAY have a right child: that child takes its place.
    // Clue: --count_ exactly once per successful remove. Counting inside
    // the helper AND here double-counts the two-children case.
    // Clue: when the removed node is the root, root_ itself must change.
    bool removed = false;
    root_ = remove_node(root_, id, &removed);
    if (removed) {
        count_--;
    }

    return removed;
}

MemoryIndex::Node* MemoryIndex::remove_node(Node* n, std::uint64_t id, bool* removed) {
    if (!n) {
        return nullptr;
    }

    if (id < n->rec.id) {
        n->left = remove_node(n->left, id, removed);
    } else if (id > n->rec.id) {
        n->right = remove_node(n->right, id, removed);
    } else {
        if (!(n->left)) {
            Node* child = n->right;
            delete n;
            *removed = true;
            return child;
        }
        if (!(n->right)) {
            Node* child = n->left;
            delete n;
            *removed = true;
            return child;
        }

        Node* successor = n->right;
        while (successor->left) {
            successor = successor->left;
        }
        std::uint64_t succ_id = successor->rec.id;
        n->rec = std::move(successor->rec);
        n->right = remove_node(n->right, succ_id, removed);
    }

    if (!*removed) {
        return n;
    }
    return rebalance(n);
}

std::size_t MemoryIndex::size() const noexcept { return count_; }

std::size_t MemoryIndex::height() const {
    // TODO: empty tree = 0, one node = 1, otherwise 1 + the taller child.
    return height_of(root_);
}

std::size_t MemoryIndex::height_of(const Node* n) {
    if (n)
        return n->height;
    else
        return 0;
}

void MemoryIndex::for_each_in_order(const std::function<void(const MemoryRecord&)>& fn) const {
    // TODO: left subtree, then fn(node->rec), then right subtree.
    in_order(root_, fn);
}

void MemoryIndex::in_order(const Node* n, const std::function<void(const MemoryRecord&)>& fn) {
    if (!n) {
        return;
    }
    in_order(n->left, fn);
    fn(n->rec);
    in_order(n->right,fn);
}

void MemoryIndex::clear() {
    // TODO
    // Clue: after freeing the nodes, set root_ = nullptr and count_ = 0, or
    // the destructor frees them a second time.
    destroy(root_);
    root_ = nullptr;
    count_ = 0;
}
