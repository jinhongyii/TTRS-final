/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    > class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */

    private:
        class RB_Tree {
        public:
            struct node {
                node *ch[2], *fa, *pre, *nxt;
                value_type v;
                bool red;
                int size;
                node(const value_type &_v, node *nil) : v(_v), ch{nil, nil}, fa(nil), pre(nil), nxt(nil), red(true), size(1) {}
                node(const Key &key, node *nil): v(key, T()), ch{nil, nil}, fa(nil), pre(nil), nxt(nil), red(true), size(1) {}
                node(const node &u, node *nil) : v(u.v), red(u.red), size(u.size){
                    ch[0] = ch[1] = fa = pre = nxt = nil;
                }
                void setc(node *r, int c) {
                    ch[c] = r; r->fa = this;
                }
                int pl() {return fa->ch[1] == this;}
                node *brother() {
                    return fa->ch[pl() ^ 1];
                }
                void swap_except_v(node *r) {
                    node *t;
                    int c1 = pl(), c2 = r->pl();
                    t = r->ch[0]; r->ch[0] = ch[0]; ch[0] = t;
                    t = r->ch[1]; r->ch[1] = ch[1]; ch[1] = t;
                    t = r->fa; r->fa = fa; fa = t;
                    t = r->pre; r->pre = pre; pre = t;
                    t = r->nxt; r->nxt = nxt; nxt = t;
                    bool rr = r->red;
                    r->red = red; red = rr;

                    if (fa == this) fa = r;
                    if (ch[0] == this) ch[0] = r;
                    if (ch[1] == this) ch[1] = r;
                    if (pre == this) pre = r;
                    if (nxt == this) nxt = r;
                    if (r->fa == r) r->fa = this;
                    if (r->ch[0] == r) r->ch[0] = this;
                    if (r->ch[1] == r) r->ch[1] = this;
                    if (r->pre == r) r->pre = this;
                    if (r->nxt == r) r->nxt = this;

                    pre->nxt = this;
                    nxt->pre = this;
                    fa->ch[c2] = this;
                    ch[0]->fa = this;
                    ch[1]->fa = this;

                    r->pre->nxt = r;
                    r->nxt->pre = r;
                    r->fa->ch[c1] = r;
                    r->ch[0]->fa = r;
                    r->ch[1]->fa = r;
                }
            };

            node *root, *head, *nil;

            int size;

            node *get_head() const {
                node *r = root;
                if (r == nil) return nil;
                while (r->ch[0] != nil) r = r->ch[0];
                return r;
            }

            const node *get_head2() const {
                const node *r = root;
                if (r == nil) return nil;
                while (r->ch[0] != nil) r = r->ch[0];
                return r;
            }

            node *get_tail() const {
                node *r = root;
                if (r == nil) return nil;
                while (r->ch[1] != nil) r = r->ch[1];
                return r;
            }

            node *get_pre(node *r) {
                if (r->ch[0] != nil) {
                    node *t = r->ch[0];
                    while (t->ch[1] != nil) t = t->ch[1];
                    return t;
                } else {
                    while (r != nil && r->pl() != 1) r = r->fa;
                    return r->fa;
                }
            }

            node *get_nxt(node *r) {
                if (r->ch[1] != nil) {
                    node *t = r->ch[1];
                    while (t->ch[0] != nil) t = t->ch[0];
                    return t;
                } else {
                    while (r != nil && r->pl() != 0) r = r->fa;
                    return r->fa;
                }
            }

            void release(node *r) {
                if (r == nil) return;
                release(r->ch[0]);
                release(r->ch[1]);
                delete r;
            }

            RB_Tree() {
                nil = (node *) operator new(sizeof(node));
                nil->pre = nil->nxt = nil->ch[0] = nil->ch[1] = nil->fa = nil;
                nil->red = false;
                nil->size = 0;
                head = root = nil;
                size = 0;
            }

            ~RB_Tree() {
                release(root);
                operator delete(nil);
            }

            node *newtree(node *r, node *left, node *right) {
                if (r->size == 0) return nil;
                node *t = new node(*r);
                t->pre = left;
                t->nxt = right;
                if (left != nil) left->nxt = t;
                if (right != nil) right->pre = t;
                t->setc(newtree(r->ch[0], left, t), 0);
                t->setc(newtree(r->ch[1], t, right), 1);
                return t;
            }

            void rotate(node *r) {
                node *f = r->fa;
                int c = r->pl();
                if (f == root) root = r, r->fa = nil;
                else f->fa->setc(r, f->pl());
                f->setc(r->ch[c ^ 1], c);
                r->setc(f, c ^ 1);
            }

            //the initial color of r must be red.
            void insert_fix(node *r) {
                while (r->fa->red) {
                    node *uncle = r->fa->brother();
                    if (uncle->red) {
                        uncle->red = false;
                        r->fa->red = false;
                        r = r->fa->fa;
                        r->red = true;
                    } else {
                        int c = r->fa->pl();
                        if (r->pl() != c) rotate(r);
                        else r = r->fa;
                        r->red = false;
                        r->fa->red = true;
                        rotate(r);
                        break;
                    }
                }
                root->red = false;
            }

            void delete_fix(node *r) {
                if (r == root) return;
                node *f = r->fa;
                node *b = r->brother();
                int c = r->pl();
                if (!b->red && !b->ch[0]->red && !b->ch[1]->red) {
                    if (f->red) {
                        f->red = false;
                        b->red = true;
                    } else {
                        b->red = true;
                        delete_fix(f);
                    }
                } else {
                    if (b->red) {
                        rotate(b);
                        b->red = false;
                        f->red = true;
                        delete_fix(r);
                    } else {
                        if (b->ch[c ^ 1]->red) {
                            rotate(b);
                            b->red = f->red;
                            f->red = false;
                            b->ch[c ^ 1]->red = false;
                        } else {
                            node *t = b->ch[c];
                            rotate(t);
                            t->red = false;
                            t->ch[c ^ 1]->red = true;
                            delete_fix(r);
                        }
                    }
                }
            }

            void remove(node *r) {
                if (r->ch[0] != nil && r->ch[1] != nil) {
                    node *t = r->nxt;
                    r->swap_except_v(t);
                    if (root == r) root = t;
                    remove(r);
                } else if  (r->ch[0] == nil && r->ch[1] == nil) {
                    node *p = r->pre, *q = r->nxt;
                    if (p != nil) p->nxt = q;
                    if (q != nil) q->pre = p;
                    if (r->red) {
                        if (r == root) root = nil;
                        else r->fa->ch[r->pl()] = nil;
                        delete r;
                        --size;
                    } else {
                        delete_fix(r);
                        if (r == root) root = nil;
                        else r->fa->ch[r->pl()] = nil;
                        delete r;
                        --size;
                    }
                } else {
                    node *p = r->pre, *q = r->nxt;
                    if (p != nil) p->nxt = q;
                    if (q != nil) q->pre = p;
                    if (r->ch[0] == nil)
                        r->swap_except_v(r->ch[1]);
                    else
                        r->swap_except_v(r->ch[0]);
                    if (root == r) root = r->fa;
                    remove(r);
                }
            }

            node *find(const Key &key) const {
                node *r = root;
                auto cmp = Compare();
                while (r != nil) {
                    if (!cmp(r->v.first, key) && !cmp(key, r->v.first)) return r;
                    else if (cmp(key, r->v.first)) r = r->ch[0];
                    else r = r->ch[1];
                }
                if (r == nil) return nullptr;
            }

            pair<node *, bool> insert(const value_type &V) {
                node *r = root;
                auto cmp = Compare();
                if (r == nil) {
                    root = new node(V, nil);
                    ++size;
                    root->red = false;
                    return pair<node *, bool>(root, true);
                }
                while (r != nil) {
                    if (!cmp(r->v.first, V.first) && !cmp(V.first, r->v.first)) return pair<node *, bool>(r, false);
                    else if (cmp(V.first, r->v.first)) {
                        if (r->ch[0] != nil)
                            r = r->ch[0];
                        else {
                            node *q = r, *p = r->pre;
                            r->setc(new node(V, nil), 0);
                            ++size;
                            r = r->ch[0];
                            r->pre = p; r->nxt = q;
                            if (p != nil) p->nxt = r;
                            if (q != nil) q->pre = r;
                            insert_fix(r);
                            return pair<node *, bool>(r, true);
                        }
                    } else {
                        if (r->ch[1] != nil)
                            r = r->ch[1];
                        else {
                            node *p = r, *q = r->nxt;
                            r->setc(new node(V, nil), 1);
                            ++size;
                            r = r->ch[1];
                            r->pre = p; r->nxt = q;
                            if (p != nil) p->nxt = r;
                            if (q != nil) q->pre = r;
                            insert_fix(r);
                            return pair<node *, bool>(r, true);
                        }
                    }
                }
            }

            node *find_insert(const Key &key) {
                node *r = root;
                auto cmp = Compare();
                if (r == nil) {
                    root = new node(key, nil);
                    ++size;
                    root->red = false;
                    return root;
                }
                while (r != nil) {
                    if (!cmp(r->v.first, key) && !cmp(key, r->v.first)) return r;
                    else if (cmp(key, r->v.first)) {
                        if (r->ch[0] != nil)
                            r = r->ch[0];
                        else {
                            node *q = r, *p = r->pre;
                            r->setc(new node(key, nil), 0);
                            ++size;
                            r = r->ch[0];
                            r->pre = p; r->nxt = q;
                            if (p != nil) p->nxt = r;
                            if (q != nil) q->pre = r;
                            insert_fix(r);
                            return r;
                        }
                    } else {
                        if (r->ch[1] != nil)
                            r = r->ch[1];
                        else {
                            node *p = r, *q = r->nxt;
                            r->setc(new node(key, nil), 1);
                            ++size;
                            r = r->ch[1];
                            r->pre = p; r->nxt = q;
                            if (p != nil) p->nxt = r;
                            if (q != nil) q->pre = r;
                            insert_fix(r);
                            return r;
                        }
                    }
                }
            }

            void dfs_c(node *r) {
                if (r == nil) return;
                dfs_c(r->ch[0]);
                dfs_c(r->ch[1]);
                if (r->red) ++cnt;
            }
            int cnt;
            int count_red() {
                cnt = 0;
                dfs_c(root);
                return cnt;
            }
        };


    public:
        class const_iterator;
        class iterator {
        private:
            /**
             * TODO add data members
             *   just add whatever you want.
             */

            typename RB_Tree::node *p;
            RB_Tree *RB;

            friend class const_interator;
            friend class map;

        public:
            iterator() : p(nullptr), RB(nullptr) {}
            iterator(const iterator &other) : p(other.p), RB(other.RB) {}
            iterator(typename RB_Tree::node *r, RB_Tree *rb) : p(r), RB(rb) {}
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, just return the answer.
             * as well as operator-
             */
            /**
             * TODO iter++
             */
            iterator operator++(int) {
                iterator a(*this);
                if (p == nullptr) throw index_out_of_bound();
                p = p->nxt;
                if (p == RB->nil) p = nullptr;
                return a;
            }
            /**
             * TODO ++iter
             */
            iterator & operator++() {
                if (p == nullptr) throw index_out_of_bound();
                p = p->nxt;
                if (p == RB->nil) p = nullptr;
                return *this;
            }
            /**
             * TODO iter--
             */
            iterator operator--(int) {
                iterator a(*this);
                if (p == nullptr) p = RB->get_tail();
                else p = p->pre;
                if (p == RB->nil) {
                    p = nullptr;
                    throw index_out_of_bound();
                }
                return a;
            }
            /**
             * TODO --iter
             */
            iterator & operator--() {
                if (p == nullptr) p = RB->get_tail();
                else p = p->pre;
                if (p == RB->nil) {
                    p = nullptr;
                    throw index_out_of_bound();
                }
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type & operator*() const {
                return p->v;
            }
            bool operator==(const iterator &rhs) const {
                return p == rhs.p && RB == rhs.RB;
            }
            bool operator==(const const_iterator &rhs) const {
                return p == rhs.p && RB == rhs.RB;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return p != rhs.p || RB != rhs.RB;
            }
            bool operator!=(const const_iterator &rhs) const {
                return p != rhs.p || RB != rhs.RB;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type* operator->() const noexcept {
                return &p->v;
            }
        };
        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            // data members.
            const typename RB_Tree::node *p;
            const RB_Tree *RB;

            friend class iterator;
            friend class map;

        public:
            const_iterator() : p(nullptr), RB(nullptr) {}
            const_iterator(const const_iterator &other) : p(other.p), RB(other.RB) {}
            const_iterator(const iterator &other) : p(other.p), RB(other.RB) {}
            const_iterator(typename RB_Tree::node *r, const RB_Tree *rb) : p(r), RB(rb) {}
            // And other methods in iterator.
            // And other methods in iterator.
            // And other methods in iterator.

            const_iterator operator++(int) {
                const_iterator a(*this);
                if (p == nullptr) throw index_out_of_bound();
                p = p->nxt;
                if (p == RB->nil) p = nullptr;
                return a;
            }
            /**
             * TODO ++iter
             */
            const_iterator & operator++() {
                if (p == nullptr) throw index_out_of_bound();
                p = p->nxt;
                if (p == RB->nil) p = nullptr;
                return *this;
            }
            /**
             * TODO iter--
             */
            const_iterator operator--(int) {
                const_iterator a(*this);
                if (p == nullptr) p = RB->get_tail();
                else p = p->pre;
                if (p == RB->nil) {
                    p = nullptr;
                    throw index_out_of_bound();
                }
                return a;
            }
            /**
             * TODO --iter
             */
            const_iterator & operator--() {
                if (p == nullptr) p = RB->get_tail();
                else p = p->pre;
                if (p == RB->nil) {
                    p = nullptr;
                    throw index_out_of_bound();
                }
                return *this;
            }

            const value_type & operator*() const {
                return p->v;
            }
            bool operator==(const iterator &rhs) const {
                return p == rhs.p && RB == rhs.RB;
            }
            bool operator==(const const_iterator &rhs) const {
                return p == rhs.p && RB == rhs.RB;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return p != rhs.p || RB != rhs.RB;
            }
            bool operator!=(const const_iterator &rhs) const {
                return p != rhs.p || RB != rhs.RB;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            const value_type* operator->() const noexcept {
                return &p->v;
            }
        };
        /**
         * TODO two constructors
         */

    private:
        RB_Tree TREE;

    public:
        map() {
        }
        map(const map &other) {
            TREE.root = TREE.newtree(other.TREE.root, TREE.nil, TREE.nil);
            TREE.size = other.size();
        }
        /**
         * TODO assignment operator
         */
        map & operator=(const map &other) {
            if (this == &other) return *this;
            TREE.release(TREE.root);
            TREE.root = TREE.newtree(other.TREE.root, TREE.nil, TREE.nil);
            TREE.size = other.size();
            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map() {}
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T & at(const Key &key) {
            auto *r = TREE.find(key);
            if (r == nullptr) throw index_out_of_bound();
            return r->v.second;
        }
        const T & at(const Key &key) const {
            auto *r = TREE.find(key);
            if (r == nullptr) throw index_out_of_bound();
            return r->v.second;
        }
        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T & operator[](const Key &key) {
            auto *r = TREE.find_insert(key);
            return r->v.second;
        }
        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T & operator[](const Key &key) const {
            auto *r = TREE.find(key);
            if (r == nullptr) throw index_out_of_bound();
            return r->v.second;
        }
        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            typename RB_Tree::node *p = TREE.get_head();
            if (p == TREE.nil) p = nullptr;
            return iterator(p, &TREE);
        }
        const_iterator cbegin() const {
            typename RB_Tree::node *p = TREE.get_head();
            if (p == TREE.nil) p = nullptr;
            return const_iterator(p, (const RB_Tree*) &TREE);
        }
        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            return iterator(nullptr, &TREE);
        }
        const_iterator cend() const {
            return const_iterator(nullptr, (const RB_Tree*) &TREE);
        }
        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return TREE.size == 0;
        }
        /**
         * returns the number of elements.
         */
        size_t size() const {
            return TREE.size;
        }
        /**
         * clears the contents
         */
        void clear() {
            TREE.release(TREE.root);
            TREE.size = 0;
            TREE.root = TREE.head = TREE.nil;
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            pair<typename RB_Tree::node *, bool> p = TREE.insert(value);
            return pair<iterator, bool>(iterator(p.first, &TREE), p.second);
        }
        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos.p == nullptr) throw index_out_of_bound();
            if (pos.RB != &TREE) throw invalid_iterator();
            TREE.remove(pos.p);
        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const {
            if (TREE.find(key) != nullptr) return 1;
            else return 0;
        }
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) {
            typename RB_Tree::node *p = TREE.find(key);
            return iterator(p, &TREE);
        }
        const_iterator find(const Key &key) const {
            typename RB_Tree::node *p = TREE.find(key);
            return const_iterator(p, &TREE);
        }

        int count_red() {
            return TREE.count_red();
        }
    };

}

#endif