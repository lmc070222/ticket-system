/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <cstddef>
#include <functional>
#include <memory>

#include "exceptions.hpp"
#include "utility.hpp"

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key> >
class map {
   public:
    /**
     * the internal type of data.
     * it should have a default constructor, a copy constructor.
     * You can use sjtu::map as value_type by typedef.
     */
    typedef pair<const Key, T> value_type;
    struct Node {
        Node* lson = nullptr;
        Node* rson = nullptr;
        Node* parent = nullptr;
        value_type kv;
        int height = 1;
        Node(const value_type& val) : kv(val) {
        }
        Node(){};
    };
    /**
     * see BidirectionalIterator at CppReference for help.
     *
     * if there is anything wrong throw invalid_iterator.
     *     like it = map.begin(); --it;
     *       or it = map.end(); ++end();
     */
    class const_iterator;
    class iterator {
       private:
        /**
         * TODO add data members
         *   just add whatever you want.
         */
       public:
        Node* cur = nullptr;
        Node* header = nullptr;
        iterator(Node* cur_, Node* hea) {
            cur = cur_;
            header = hea;
        }
        iterator() {
            cur = nullptr;
            header = nullptr;
            // TODO
        }

        iterator(const iterator& other) {
            cur = other.cur;
            header = other.header;
            // TODO
        }

        /**
         * TODO iter++
         */
        iterator operator++(int) {
            if (this->cur == header) throw invalid_iterator();
            iterator new_;
            new_ = *this;
            if (cur->rson != nullptr) {
                Node* p = cur->rson;
                while (p->lson != nullptr) p = p->lson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->lson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header;
            }
            return new_;
        }

        /**
         * TODO ++iter
         */
        iterator& operator++() {
            if (this->cur == header) throw invalid_iterator();
            if (cur->rson != nullptr) {
                Node* p = cur->rson;
                while (p->lson != nullptr) p = p->lson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->lson != p)
                    p = p->parent;
                if (p->parent != nullptr)
                    this->cur = p->parent;
                else
                    this->cur = header;
            }
            return *this;
        }

        /**
         * TODO iter--
         */
        iterator operator--(int) {
            if (this->cur == header->lson) throw invalid_iterator();
            if (cur == header) {
                iterator new_;
                new_ = *this;
                cur = header->rson;
                return new_;
            }
            iterator new_;
            new_ = *this;
            if (cur->lson != nullptr) {
                Node* p = cur->lson;
                while (p->rson != nullptr) p = p->rson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->rson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header->lson;
            } 
            return new_;
        }

        /**
         * TODO --iter
         */
        iterator& operator--() {
            if (this->cur == header->lson) throw invalid_iterator();
            if (cur == header) {
                cur = header->rson;
                return *this;
            }
            if (cur->lson != nullptr) {
                Node* p = cur->lson;
                while (p->rson != nullptr) p = p->rson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->rson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header->lson;
            }
            return *this;
        }

        /**
         * a operator to check whether two iterators are same (pointing to the
         * same memory).
         */
        value_type& operator*() const {
            return cur->kv;
        }

        bool operator==(const iterator& rhs) const {
            return cur == rhs.cur;
        }

        bool operator==(const const_iterator& rhs) const {
            return cur == rhs.cur;
        };

        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator& rhs) const {
            return !((*this) == rhs);
        }

        bool operator!=(const const_iterator& rhs) const {
            return !((*this) == rhs);
        }

        /**
         * for the support of it->first.
         * See
         * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
         * for help.
         */
        value_type* operator->() const noexcept {
            return &(cur->kv);
        }
    };
    class const_iterator {
        // it should has similar member method as iterator.
        //  and it should be able to construct from an iterator.
       private:
        // data members.
       public:
        Node* cur = nullptr;
        Node* header = nullptr;
        const_iterator(Node* cur_, Node* hea) {
            cur = cur_;
            header = hea;
        }
        const_iterator() {
            cur = nullptr;
            header = nullptr;
            // TODO
        }

        const_iterator(const const_iterator& other) {
            cur = other.cur;
            header = other.header;
            // TODO
        }

        const_iterator(const iterator& other) {
            cur = other.cur;
            header = other.header;
            // TODO
        }
        // And other methods in iterator.
        // And other methods in iterator.
        // And other methods in iterator.
        const_iterator operator++(int) {
            if (this->cur == header) throw invalid_iterator();
            const_iterator new_;
            new_ = *this;
            if (cur->rson != nullptr) {
                Node* p = cur->rson;
                while (p->lson != nullptr) p = p->lson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->lson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header;
            }
            return new_;
        }
        const_iterator& operator++() {
            if (this->cur == header) throw invalid_iterator();
            if (cur->rson != nullptr) {
                Node* p = cur->rson;
                while (p->lson != nullptr) p = p->lson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->lson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header;
            }
            return *this;
        }
        const_iterator operator--(int) {
            if (this->cur == header->lson) throw invalid_iterator();
            if (cur == header) {
                const_iterator new_ = *this;
                cur = header->rson;
                return new_;
            }
            const_iterator new_;
            new_ = *this;
            if (cur->lson != nullptr) {
                Node* p = cur->lson;
                while (p->rson != nullptr) p = p->rson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->rson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header->lson;
            }
            return new_;
        }
        const_iterator& operator--() {
            if (this->cur == header->lson) throw invalid_iterator();
            if (cur == header) {
                cur = header->rson;
                return *this;
            }
            if (cur->lson != nullptr) {
                Node* p = cur->lson;
                while (p->rson != nullptr) p = p->rson;
                this->cur = p;
            } else {
                Node* p = cur;
                while (p->parent != header and p->parent->rson != p)
                    p = p->parent;
                if (p->parent != header)
                    this->cur = p->parent;
                else
                    this->cur = header->lson;
            }
            return *this;
        }
        value_type& operator*() const {
            return cur->kv;
        }
        bool operator==(const iterator& rhs) const {
            return cur == rhs.cur;
        }
        bool operator==(const const_iterator& rhs) const {
            return cur == rhs.cur;
        };

        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator& rhs) const {
            return !((*this) == rhs);
        }

        bool operator!=(const const_iterator& rhs) const {
            return !((*this) == rhs);
        }
        value_type* operator->() const noexcept {
            return &(cur->kv);
        }
    };

    /**
     * TODO two constructors
     */
    Node* header = nullptr;
    Node* root;
    int size_ = 0;
    map() {
        root = nullptr;
        std::allocator<Node> alloc;
        header = alloc.allocate(1);
        header->parent = root;
        header->lson = header;
        header->rson = header;
    }
    Node* copyTree(Node* otherNode, Node* newParent) {
        if (otherNode == nullptr) return nullptr;
        Node* newNode = new Node(otherNode->kv);
        newNode->height = otherNode->height;
        newNode->parent = newParent;
        newNode->lson = copyTree(otherNode->lson, newNode);
        newNode->rson = copyTree(otherNode->rson, newNode);
        return newNode;
    }
    Node* findmin(Node* x) {
        if (x == nullptr) return header;
        while (x->lson != nullptr) x = x->lson;
        return x;
    }
    Node* findmax(Node* x) {
        if (x == nullptr) return header;
        while (x->rson != nullptr) x = x->rson;
        return x;
    }
    map(const map& other) {
        std::allocator<Node> alloc;
        header = alloc.allocate(1);
        header->parent = nullptr;
        header->lson = header;
        header->rson = header;
        size_ = 0;
        if (other.root != nullptr) {
            root = copyTree(other.header->parent, header);
            header->parent = root;
            root->parent = header;
            header->lson = findmin(root);
            header->rson = findmax(root);
            size_ = other.size_;
        } else
            root = nullptr;
    }

    /**
     * TODO assignment operator
     */
    map& operator=(const map& other) {
        if (this == &other) return *this;
        clear();
        if (other.root != nullptr) {
            root = copyTree(other.header->parent, header);
            header->parent = root;
            root->parent = header;
            header->lson = findmin(root);
            header->rson = findmax(root);
            size_ = other.size_;
        } else
            root = nullptr;
        return *this;
    }

    /**
     * TODO Destructors
     */
    ~map() {
        clear();
        std::allocator<Node> alloc;
        alloc.deallocate(header, 1);
    }

    /**
     * TODO
     * access specified element with bounds checking
     * Returns a reference to the mapped value of the element with key
     * equivalent to key. If no such element exists, an exception of type
     * `index_out_of_bound'
     */
    Node* findNode(Node* t, const Key& key) const {
        Compare comp;
        while (t != nullptr) {
            if (comp(key, t->kv.first)) {
                t = t->lson;
            } else if (comp(t->kv.first, key)) {
                t = t->rson;
            } else {
                return t;
            }
        }
        return nullptr;
    }
    T& at(const Key& key) {
        Compare com;
        Node* tmp = findNode(root, key);
        if (tmp == nullptr)
            throw index_out_of_bound();
        else
            return tmp->kv.second;
    }

    const T& at(const Key& key) const {
        Compare com;
        Node* tmp = findNode(root, key);
        if (tmp == nullptr)
            throw index_out_of_bound();
        else
            return tmp->kv.second;
    }

    /**
     * TODO
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to
     * key, performing an insertion if such key does not already exist.
     */
    T& operator[](const Key& key) {
        Compare com;
        Node* tmp = findNode(root, key);
        if (tmp == nullptr) {
            auto res = insert({key, T()});
            return res.first->second;
        } else
            return tmp->kv.second;
    }

    /**
     * behave like at() throw index_out_of_bound if such key does not exist.
     */
    const T& operator[](const Key& key) const {
        Compare com;
        Node* tmp = findNode(root, key);
        if (tmp == nullptr) {
            throw index_out_of_bound();
        } else
            return tmp->kv.second;
    }

    /**
     * return a iterator to the beginning
     */
    iterator begin() {
        iterator x(header->lson, header);
        return x;
    }

    const_iterator cbegin() const {
        iterator x(header->lson, header);
        return x;
    }

    /**
     * return a iterator to the end
     * in fact, it returns past-the-end.
     */
    iterator end() {
        iterator x(header, header);
        return x;
    }

    const_iterator cend() const {
        iterator x(header, header);
        return x;
    }

    /**
     * checks whether the container is empty
     * return true if empty, otherwise false.
     */
    bool empty() const {
        return (size_ == 0);
    }

    /**
     * returns the number of elements.
     */
    size_t size() const {
        return size_;
    }

    /**
     * clears the contents
     */
    void deletetree(Node* p) {
        if (p == nullptr) return;
        deletetree(p->lson);
        deletetree(p->rson);
        delete p;
    }
    void clear() {
        deletetree(root);
        root = nullptr;
        header->parent = root;
        header->lson = header;
        header->rson = header;
        size_ = 0;
    }

    /**
     * insert an element.
     * return a pair, the first of the pair is
     *   the iterator to the new element (or the element that prevented the
     * insertion), the second one is true if insert successfully, or false.
     */
    pair<iterator, bool> insert(const value_type& value) {
        if (root == nullptr) {
            root = new Node(value);
            root->parent = header;
            header->parent = root;
            header->lson = root;
            header->rson = root;
            root->height = 1;
            header->parent = root;
            size_++;
            return pair<iterator, bool>{iterator(root, header), true};
        }
        Compare comp;
        Node* p = this->root;
        Node* parent;
        while (p != nullptr) {
            parent = p;
            if (comp(p->kv.first, value.first)) {
                p = p->rson;
            } else if (comp(value.first, p->kv.first)) {
                p = p->lson;
            } else
                return pair<iterator, bool>{iterator(p, header), false};
        }
        size_++;
        Node* new_node = new Node(value);
        new_node->parent = parent;
        if (comp(value.first, parent->kv.first))
            parent->lson = new_node;
        else if (comp(parent->kv.first, value.first))
            parent->rson = new_node;
        if (comp(value.first, header->lson->kv.first)) header->lson = new_node;
        if (comp(header->rson->kv.first, value.first)) header->rson = new_node;
        p = parent;
        while (p != nullptr and p != header) {
            updateheight(p);
            int bf = getbalance(p);
            if (bf == 2) {
                if (getbalance(p->lson) >= 0)
                    p = rotateRight(p);
                else {
                    p->lson = rotateLeft(p->lson);
                    p = rotateRight(p);
                }
                break;
            }
            if (bf == -2) {
                if (getbalance(p->rson) <= 0) {
                    p = rotateLeft(p);
                } else {
                    p->rson = rotateRight(p->rson);
                    p = rotateLeft(p);
                }
                break;
            }
            p = p->parent;
        }
        header->parent = root;
        return pair<iterator, bool>{iterator(new_node, header), true};
    }

    /**
     * erase the element at pos.
     *
     * throw if pos pointed to a bad element (pos == this->end() || pos points
     * an element out of this)
     */
    void updateheight(Node* x) {
        if (x->lson == nullptr and x->rson == nullptr)
            x->height = 1;
        else if (x->lson == nullptr and x->rson != nullptr)
            x->height = x->rson->height + 1;
        else if (x->lson != nullptr and x->rson == nullptr)
            x->height = x->lson->height + 1;
        else
            x->height = std::max(x->lson->height, x->rson->height) + 1;
    }
    int getbalance(Node* p) {
        int height1 = (p->lson == nullptr) ? 0 : p->lson->height;
        int height2 = (p->rson == nullptr) ? 0 : p->rson->height;
        return height1 - height2;
    }
    Node* rotateRight(Node* p) {
        Node* child = p->lson;
        Node* parent = p->parent;
        p->lson = child->rson;
        if (child->rson) child->rson->parent = p;
        child->parent = parent;
        if (parent == header) {
            root = child;
            header->parent = child;
        } else {
            if (parent->lson == p)
                parent->lson = child;
            else
                parent->rson = child;
        }
        child->rson = p;
        p->parent = child;
        updateheight(p);
        updateheight(child);
        return child;
    }
    Node* rotateLeft(Node* p) {
        Node* child = p->rson;
        Node* parent = p->parent;
        p->rson = child->lson;
        if (child->lson) child->lson->parent = p;
        child->parent = parent;
        if (parent == header) {
            root = child;
            header->parent = child;
        } else {
            if (parent->rson == p)
                parent->rson = child;
            else
                parent->lson = child;
        }
        child->lson = p;
        p->parent = child;
        updateheight(p);
        updateheight(child);
        return child;
    }
void erase(iterator pos) {
        if (pos == end() or pos.header != header) throw invalid_iterator();
        size_--;
        value_type vt = *pos;
        Node* tmp = pos.cur;
        Node* parent = tmp->parent;
        Compare comp;
        if (tmp->lson == nullptr and tmp->rson == nullptr) {
            if (parent->lson == tmp) parent->lson = nullptr;
            if (parent->rson == tmp) parent->rson = nullptr;
            if (parent == header) root = nullptr;
            Node* p = parent;
            while (p != nullptr and p != header) {
                updateheight(p);
                int bf = getbalance(p);
                if (bf == 2) {
                    if (getbalance(p->lson) >= 0)
                        p = rotateRight(p);
                    else {
                        p->lson = rotateLeft(p->lson);
                        p = rotateRight(p);
                    }
                }
                if (bf == -2) {
                    if (getbalance(p->rson) <= 0) {
                        p = rotateLeft(p);
                    } else {
                        p->rson = rotateRight(p->rson);
                        p = rotateLeft(p);
                    }
                }
                p = p->parent;
            }
            delete tmp;
        } else if (tmp->lson == nullptr) {
            if (parent->lson == tmp) parent->lson = tmp->rson;
            if (parent->rson == tmp) parent->rson = tmp->rson;
            tmp->rson->parent = parent;
            if (parent == header) {
                root = tmp->rson;
                tmp->rson->parent = header;
            }
            Node* p = parent;
            while (p != nullptr and p != header) {
                updateheight(p);
                int bf = getbalance(p);
                if (bf == 2) {
                    if (getbalance(p->lson) >= 0)
                        p = rotateRight(p);
                    else {
                        p->lson = rotateLeft(p->lson);
                        p = rotateRight(p);
                    }
                }
                if (bf == -2) {
                    if (getbalance(p->rson) <= 0) {
                        p = rotateLeft(p);
                    } else {
                        p->rson = rotateRight(p->rson);
                        p = rotateLeft(p);
                    }
                }
                p = p->parent;
            }
            delete tmp;
        } else if (tmp->rson == nullptr) {
            if (parent->lson == tmp) parent->lson = tmp->lson;
            if (parent->rson == tmp) parent->rson = tmp->lson;
            tmp->lson->parent = parent;
            if (parent == header) {
                root = tmp->lson;
                tmp->lson->parent = header;
            }
            Node* p = parent;
            while (p != nullptr and p != header) {
                updateheight(p);
                int bf = getbalance(p);
                if (bf == 2) {
                    if (getbalance(p->lson) >= 0)
                        p = rotateRight(p);
                    else {
                        p->lson = rotateLeft(p->lson);
                        p = rotateRight(p);
                    }
                }
                if (bf == -2) {
                    if (getbalance(p->rson) <= 0) {
                        p = rotateLeft(p);
                    } else {
                        p->rson = rotateRight(p->rson);
                        p = rotateLeft(p);
                    }
                }
                p = p->parent;
            }
            delete tmp;
        } else {
            ++pos;
            iterator x = pos;
            --pos;
            Node* cur = x.cur;
            Node* cur_parent = cur->parent;
            if (cur == tmp->rson) {
                cur->parent = parent;
                if (tmp == parent->lson) parent->lson = cur;
                cur->lson = tmp->lson;
                cur->height = tmp->height;
                tmp->lson->parent = cur;
                if (tmp == parent->rson) parent->rson = cur;
                if (tmp == root) root = cur;
                Node* p = cur;
                while (p != nullptr and p != header) {
                    updateheight(p);
                    int bf = getbalance(p);
                    if (bf == 2) {
                        if (getbalance(p->lson) >= 0)
                            p = rotateRight(p);
                        else {
                            p->lson = rotateLeft(p->lson);
                            p = rotateRight(p);
                        }
                    }
                    if (bf == -2) {
                        if (getbalance(p->rson) <= 0) {
                            p = rotateLeft(p);
                        } else {
                            p->rson = rotateRight(p->rson);
                            p = rotateLeft(p);
                        }
                    }
                    p = p->parent;
                }
                delete tmp;
            } else if (cur->rson != nullptr) {
                if (cur_parent->lson == cur)
                    cur->parent->lson = cur->rson;
                else if (cur_parent->rson == cur)
                    cur->parent->rson = cur->rson;
                cur->rson->parent = cur->parent;
                cur->parent = parent;
                if (tmp == parent->lson) parent->lson = cur;
                cur->lson = tmp->lson;
                cur->rson = tmp->rson;
                cur->height = tmp->height;
                tmp->lson->parent = cur;
                tmp->rson->parent = cur;
                if (tmp == parent->rson) parent->rson = cur;
                if (tmp == root) root = cur;
                Node* p = cur_parent;
                while (p != nullptr and p != header) {
                    updateheight(p);
                    int bf = getbalance(p);
                    if (bf == 2) {
                        if (getbalance(p->lson) >= 0)
                            p = rotateRight(p);
                        else {
                            p->lson = rotateLeft(p->lson);
                            p = rotateRight(p);
                        }
                    }
                    if (bf == -2) {
                        if (getbalance(p->rson) <= 0) {
                            p = rotateLeft(p);
                        } else {
                            p->rson = rotateRight(p->rson);
                            p = rotateLeft(p);
                        }
                    }
                    p = p->parent;
                }
                delete tmp;
            } else if (cur->rson == nullptr) {
                if (cur_parent->lson == cur)
                    cur->parent->lson = nullptr;
                else if (cur_parent->rson == cur)
                    cur->parent->rson = nullptr;
                cur->parent = parent;
                if (tmp == parent->lson) parent->lson = cur;
                if (tmp == parent->rson) parent->rson = cur;
                cur->lson = tmp->lson;
                cur->rson = tmp->rson;
                if (tmp->lson != nullptr) cur->lson->parent = cur;
                if (tmp->rson != nullptr) cur->rson->parent = cur;
                cur->height = tmp->height;
                if (tmp == root) root = cur;
                Node* p = cur_parent;
                while (p != nullptr and p != header) {
                    updateheight(p);
                    int bf = getbalance(p);
                    if (bf == 2) {
                        if (getbalance(p->lson) >= 0)
                            p = rotateRight(p);
                        else {
                            p->lson = rotateLeft(p->lson);
                            p = rotateRight(p);
                        }
                    }
                    if (bf == -2) {
                        if (getbalance(p->rson) <= 0) {
                            p = rotateLeft(p);
                        } else {
                            p->rson = rotateRight(p->rson);
                            p = rotateLeft(p);
                        }
                    }
                    p = p->parent;
                }
                delete tmp;
            }
        }
        if (size_ == 0) {
            root = nullptr;
            header->parent = root;
            header->lson = header;
            header->rson = header;
        } else {
            header->parent = root;
            header->lson = findmin(root);
            header->rson = findmax(root);
        }
    }

    /**
     * Returns the number of elements with key
     *   that compares equivalent to the specified argument,
     *   which is either 1 or 0
     *     since this container does not allow duplicates.
     * The default method of check the equivalence is !(a < b || b > a)
     */
    size_t count(const Key& key) const {
        if (findNode(root, key) == nullptr) return 0;
        return 1;
    }

    /**
     * Finds an element with key equivalent to key.
     * key value of the element to search for.
     * Iterator to an element with key equivalent to key.
     *   If no such element is found, past-the-end (see end()) iterator is
     * returned.
     */
    iterator find(const Key& key) {
        if (findNode(root, key) == nullptr) return end();
        return iterator(findNode(root, key), header);
    }

    const_iterator find(const Key& key) const {
        if (findNode(root, key) == nullptr) return cend();
        return const_iterator(findNode(root, key), header);
    }
};

}  // namespace sjtu

#endif
