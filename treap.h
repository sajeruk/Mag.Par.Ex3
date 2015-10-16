#pragma once

#include "cstdlib"

// by http://e-maxx.ru/algo/treap

class TTreap {
public:
    TTreap() : Root(nullptr) {}

    void Insert(size_t key) {
        insert(Root, new TItem(key, rand()));
    }

    void Remove(size_t key) {
        erase(Root, key);
    }

    size_t NumOfVertexGreater(size_t key) {
        return NumOfVertexGreaterHelper(Root, key, 0);
    }

    ~TTreap() {
        ClearHelper(Root);
    }

private:
    struct TItem {
        size_t key, prior;
        TItem * l;
        TItem * r;
        size_t cnt;
        TItem() { }
        TItem (size_t key, size_t prior) : key(key), prior(prior), l(NULL), r(NULL), cnt(1) { }
    };

    typedef TItem * pitem;

private:
    pitem Root;

private:
    void ClearHelper(pitem root) {
        if (!root)
            return;
        ClearHelper(root->l);
        ClearHelper(root->r);
        delete root;
    }

    size_t NumOfVertexGreaterHelper(pitem root, size_t key, size_t acc) {
        if (key == root->key) {
            return acc + cnt(root->r);
        }
        if (key < root->key) {
            return NumOfVertexGreaterHelper(root->l, key, acc + 1 + cnt(root->r));
        } else { // key > root
            return NumOfVertexGreaterHelper(root->r, key, acc);
        }
    }

    size_t cnt (pitem t) {
        return t ? t->cnt : 0;
    }

    void upd_cnt (pitem t) {
        if (t)
            t->cnt = 1 + cnt(t->l) + cnt (t->r);
    }

    void split (pitem t, size_t key, pitem & l, pitem & r) {
        if (!t)
            l = r = NULL;
        else if (key < t->key)
            split (t->l, key, l, t->l),  r = t;
        else
            split (t->r, key, t->r, r),  l = t;

        upd_cnt(t);
    }

    void insert (pitem & t, pitem it) {
        if (!t)
            t = it;
        else if (it->prior > t->prior)
            split (t, it->key, it->l, it->r),  t = it;
        else
            insert (it->key < t->key ? t->l : t->r, it);

        upd_cnt(t);
    }

    void merge (pitem & t, pitem l, pitem r) {
        if (!l || !r)
            t = l ? l : r;
        else if (l->prior > r->prior)
            merge (l->r, l->r, r),  t = l;
        else
            merge (r->l, l, r->l),  t = r;

        upd_cnt(t);
    }

    void erase (pitem & t, size_t key) {
        if (t->key == key) {
            auto ll = t->l;
            auto rr = t->r;
            delete t;
            merge (t, ll, rr);
        }
        else
            erase (key < t->key ? t->l : t->r, key);

        upd_cnt(t);
    }
};
