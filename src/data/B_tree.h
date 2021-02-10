#ifndef B_TREE_H
#define B_TREE_H

#include<iostream>
#include<cstdlib>
#include<queue>

	template<typename T>
	struct Node {
		int n;
		struct Node<T>** child
		T* key;    
		bool leaf;  
		Node(int num = 0):n(num),child(nullptr),key(nullptr),leaf(true){};
		~Node() { 
     n = 0;
     child = nullptr;
     key = nullptr; }
	};

template<typename T>
class BTree {
public:
    BTree(int m = 2):t(m),node_num(0){root = new Node<T>;}
	  ~BTree() { PostOrder_Delete(root); }
	  Node<T>* B_Tree_Search(const T& elem) { return search(root, elem); }
    void B_Tree_Insert(const T& k) { Insert(root, k); }
	  void B_Tree_Delete(const T &k);					    
	  void Print();										
    int size()const { return node_num; }                   
private:
	  void PostOrder_Delete(Node<T>*& cur);
	  Node<T>* search(Node<T>* x, const T& k);
    void B_Tree_Split_Child(Node<T>* x, int i);
    void Insert(Node<T>* &r, const T& k);
	  void B_Tree_Insert_NonFull(Node<T>* x, const T& k); 
	  void Merge_Node(Node<T>*& x, int i, Node<T>*& y, Node<T>*& z);  
	  T Search_Predecessor(Node<T>* y);
	  T Search_Successor(Node<T>* z);   
	  void Shift_To_Left_Child(Node<T>*& x, int i, Node<T>*& y, Node<T>*& z);
	  void Shift_To_Right_Child(Node<T>*& x, int i, Node<T>*& y, Node<T>*& z);
	  void B_Tree_Delete_NoNone(Node<T>* x, const T &k);
	  Node<T>* root;
	  int t;             
    int node_num;       
};

template<typename T>
Node<T>* BTree<T>::search(Node<T>* x, const T& k) {
	int i = 0;
	while (i<x->n && k>x->key[i]) {
		++i;
	}
	if (i < x->n&&k == x->key[i]) {   
		return x;
	}
	else if (x->leaf) {        
		return nullptr;
	}
	else {
		return search(x->child[i], k);   
	}
}

template<typename T>
void BTree<T>::B_Tree_Split_Child(Node<T>* x, int i) { 
	Node<T>* y = x->child[i];
	Node<T>*L, *R;   
	L = new Node<T>;
	R = new Node<T>;
	L->n = R->n = t - 1;
	L->key = new T[t - 1];
	R->key = new T[t - 1];
	L->leaf = R->leaf = x->child[i]->leaf;
	for (int j = 0;j <= t - 2;++j) {
		L->key[j] = y->key[j];
		R->key[j] = y->key[j + t];
	}
	if (!y->leaf) {
		L->child = new Node<T>*[t];
		R->child = new Node<T>*[t];
		for (int j = 0;j < t;++j) {
			L->child[j] = y->child[j];
			R->child[j] = y->child[j + t];
		}
	}
	x->key = (T*)realloc(x->key, (x->n + 1) * sizeof(T));
	x->child = (Node<T>**)realloc(x->child, (x->n + 2) * sizeof(Node<T>*));
	for (int j = x->n - 1;j >= i;--j) { 
		x->key[j + 1] = x->key[j];
	}
	x->key[i] = y->key[t - 1];
	for (int j = x->n;j >= i + 1;--j) {
		x->child[j + 1] = x->child[j];
	}
	x->child[i] = L;
	x->child[i + 1] = R;
	x->n++;
	delete[] y->child;
	delete[] y->key;
	delete y;
}

template<typename T>
void BTree<T>::Insert(Node<T>* &r, const T& k) {
	if (B_Tree_Search(k)) {
		return;
	}
	else if (r != nullptr && r->n == 2 * t - 1) {
		Node<T>* s = new Node<T>;
		s->leaf = false;
		s->child = new Node<T>*[1];
		s->child[0] = r;
		r = s;
		s->n = 0;
		B_Tree_Split_Child(s, 0);
		B_Tree_Insert_NonFull(s, k);
	}
	else {
		B_Tree_Insert_NonFull(r, k);
	}
    node_num++;  
}

template<typename T>
void BTree<T>::B_Tree_Insert_NonFull(Node<T>* x, const T& k) {
	int i = x->n - 1;
	if (x->leaf) {
		x->key = (T*)realloc(x->key, (x->n + 1) * sizeof(T));
		while (i >= 0 && k < x->key[i]) {
			x->key[i + 1] = x->key[i];
			--i;
		}
		x->key[i + 1] = k;
		++x->n;
	}
	else {
		while (i >= 0 && k < x->key[i]) {
			--i;
		}
		++i;
		if (x->child[i]->n == 2 * t - 1) {
			B_Tree_Split_Child(x, i);
			if (k > x->key[i]) {
				++i;
			}
		}
		B_Tree_Insert_NonFull(x->child[i], k);  
	}
}

template<typename T>
void BTree<T>::Merge_Node(Node<T>*& x, int i, Node<T>*& y, Node<T>*& z) {
	y->key = (T*)realloc(y->key, (2 * t - 1) * sizeof(T));
	for (int j = 0;j < t - 1;++j) {
		y->key[t + j] = z->key[j];
	}
	if (!y->leaf) {
		y->child = (Node<T>**)realloc(y->child, 2 * t * sizeof(Node<T>**));
		for (int j = 0;j < t;++j) {
			y->child[t + j] = z->child[j];
		}
	}
	y->key[t - 1] = x->key[i];
	y->n = 2 * t - 1;
	for (int j = i;j < x->n - 1;++j) {
		x->key[j] = x->key[j + 1];
	}
	x->key = (T*)realloc(x->key, (x->n - 1) * sizeof(T));
	for (int j = i + 1;j <= x->n - 1;++j) {
		x->child[j] = x->child[j + 1];
	}
	x->child = (Node<T>**)realloc(x->child, (x->n) * sizeof(Node<T>**));
	x->child[i] = y;
	--x->n;
	delete[] z->key;
	delete[] z->child;
	delete z;
	z = nullptr;
}

template<typename T>
T BTree<T>::Search_Predecessor(Node<T>* y) {
	Node<T>* x = y;
	while (!x->leaf) {
		x = x->child[x->n];
	}
	return x->key[x->n - 1];
}

template<typename T>
T BTree<T>::Search_Successor(Node<T>* z) {
	Node<T>* x = z;
	while (!x->leaf) {
		x = x->child[0];
	}
	return x->key[0];
}

template<typename T>
void BTree<T>::Shift_To_Left_Child(Node<T>*& x, int i, Node<T>*& y, Node<T>*& z) {
	if (y == nullptr) {  
		y = new Node<T>;
	}
	++y->n;
	y->key = (T*)realloc(y->key, y->n * sizeof(T));
	y->key[y->n - 1] = x->key[i];
	x->key[i] = z->key[0];
	for (int j = 0;j < z->n - 1;++j) {
		z->key[j] = z->key[j + 1];
	}
	if (!z->leaf) {
		y->child = (Node<T>**)realloc(y->child, (y->n + 1) * sizeof(Node<T>*));
		y->child[y->n] = z->child[0];
		for (int j = 0;j < z->n;++j) {
			z->child[j] = z->child[j + 1];
		}
		z->child = (Node<T>**)realloc(z->child, z->n * sizeof(Node<T>*));
	}
	--z->n;
	z->key = (T*)realloc(z->key, z->n * sizeof(T));
}

template<typename T>
void BTree<T>::Shift_To_Right_Child(Node<T>*& x, int i, Node<T>*& y, Node<T>*& z) {
	if (z == nullptr) { 
		z = new Node<T>;
	}
	++z->n;
	z->key = (T*)realloc(z->key, z->n * sizeof(T));
	for (int j = z->n - 1;j > 0;--j) {
		z->key[j] = z->key[j - 1];
	}
	z->key[0] = x->key[i];
	x->key[i] = y->key[y->n - 1];
	if (!z->leaf) {
		z->child = (Node<T>**)realloc(z->child, (z->n + 1) * sizeof(Node<T>*));
		for (int j = 0;j < z->n;++j) {
			z->child[j + 1] = z->child[j];
		}
		z->child[0] = y->child[y->n];
		y->child = (Node<T>**)realloc(y->child, y->n * sizeof(Node<T>*));
	}
	--y->n;
	y->key = (T*)realloc(y->key, y->n * sizeof(T));
}

template<typename T>
void BTree<T>::B_Tree_Delete_NoNone(Node<T>* x, const T &k) {
	if (x->leaf) {  
		int i = 0;
		while (i<x->n && k>x->key[i]) {
			++i;
		}
		for (int j = i + 1;j < x->n;++j) {
			x->key[j - 1] = x->key[j];
		}
		--x->n;
		x->key = (T*)realloc(x->key, x->n * sizeof(T));
	}
	else {
		int i = 0;
		while (i<x->n&&k>x->key[i]) {
			++i;
		}
		Node<T> *y = x->child[i], *z = nullptr;
		if (i < x->n) {
			z = x->child[i + 1];
		}
		if (k == x->key[i]) { 
			if (y->n > t - 1) { 
				T k1 = Search_Predecessor(y);
				B_Tree_Delete_NoNone(y, k1);
				x->key[i] = k1;
			}
			else if (z->n > t - 1) { 
				T k1 = Search_Successor(z);
				B_Tree_Delete_NoNone(z, k1);
				x->key[i] = k1;
			}
			else {  
				Merge_Node(x, i, y, z);
				B_Tree_Delete_NoNone(y, k);
			}
		}
		else { 
			Node<T>* p = nullptr;
			if (i > 0) {
				p = x->child[i - 1];
			}
			if (y->n == t - 1) {
				if (i > 0 && p->n > t - 1) { 
					Shift_To_Right_Child(x, i - 1, p, y);
				}
                else if (i<x->n&&z->n>t - 1) { 
					Shift_To_Left_Child(x, i, y, z);
				}
				else if (i > 0) {
					Merge_Node(x, i - 1, p, y);
					y = p;
				}
				else {         
					Merge_Node(x, i, y, z);      
				}
			}
			B_Tree_Delete_NoNone(y, k);
		}
	}
}
 
template<typename T>
void BTree<T>::B_Tree_Delete(const T &k) {
	Node<T>* r = root;
	if (root->n == 1 && root->child == nullptr) {
		delete[] root->key;
		delete root;
		root = nullptr;
	}
	else if (root->n == 1){  
		Node<T> *y = root->child[0], *z = root->child[1];
		if (y->n == z->n&&y->n == t - 1) {
			Merge_Node(root, 0, y, z);
			root = y;
			delete[] r->child;
			delete[] r->key;
			delete r;
			B_Tree_Delete_NoNone(y, k);
		}
		else {
			B_Tree_Delete_NoNone(root, k);
		}
	}
	else {
		B_Tree_Delete_NoNone(r, k);
	}
    --node_num;
}

template<typename T>
void BTree<T>::Print() {
  Node<T> *last = root;
	Node<T>* p = root;
    std::queue<BT::Node<T>*>Q;
	if (p) {
		Q.push(p);
	}
	while (!Q.empty()) {
		Node<T>* tmp = Q.front();
		Q.pop();
		if (!tmp->leaf) {
            std::cout << "|";
		}
        std::cout << "(";
		for (int i = 0;i < tmp->n;++i) {
            std::cout << tmp->key[i];
			if (tmp->child && i != tmp->n - 1) {
                std::cout << "|";
			}
		}
        std::cout << ")";
		if (!tmp->leaf) {   
            std::cout << "|";
			for (int i = 0;i < tmp->n + 1;++i) {
				Q.push(tmp->child[i]);
			}
		}
		if (!last->leaf && tmp == last) {
            std::cout << std::endl;
			last = last->child[last->n];
		}
	}
    std::cout << std::endl;
}

template<typename T>
void BTree<T>::PostOrder_Delete(Node<T>*& cur) {
    if (cur->leaf) {
		delete[] cur->key;
		delete cur;
		cur = nullptr;
	}
    else { 
		  for (int i = 0;i < cur->n + 1;++i) {
			    PostOrder_Delete(cur->child[i]);
		}
		delete[] cur->key;
		delete cur;
		cur = nullptr;
	}
}

#endif