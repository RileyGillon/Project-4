#include <iostream>
#include <vector>
using namespace std;

template <typename DT>
class TreeException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Tree operation exception";
    }
};

// Custom find function to replace std::find
template<typename Container, typename T>
typename Container::iterator custom_find(Container& c, const T& value) {
    auto it = c.begin();
    while (it != c.end()) {
        if (*it == value) return it;
        ++it;
    }
    return c.end();
}

// Custom sort function
template<typename Container>
void custom_sort(Container& c) {
    for (size_t i = 0; i < c.size(); i++) {
        for (size_t j = i + 1; j < c.size(); j++) {
            if (c[j] < c[i]) {
                auto temp = c[i];
                c[i] = c[j];
                c[j] = temp;
            }
        }
    }
}

template <typename DT>
class MTree {
protected:
    int M;
    vector<DT> values;
    vector<MTree*> children;
    
public:
    MTree(int M);
    ~MTree();
    bool is_leaf() const;
    void insert(const DT& value);
    void split_node();
    MTree* find_child(const DT& value);
    bool search(const DT& value);
    void remove(const DT& value);
    void buildTree(vector<DT>& input_values);
    vector<DT>& collect_values();
    bool find(const DT& value);
};

template <typename DT>
MTree<DT>::MTree(int M) : M(M) {
    if (M < 2) {
        throw invalid_argument("M must be at least 2");
    }
}

template <typename DT>
MTree<DT>::~MTree() {
    for (MTree* child : children) {
        delete child;
    }
}

template <typename DT>
bool MTree<DT>::is_leaf() const {
    return children.empty();
}

template <typename DT>
void MTree<DT>::insert(const DT& value) {
    values.push_back(value);
    custom_sort(values);
    
    if (values.size() >= M) {
        split_node();
    }
}

template <typename DT>
void MTree<DT>::split_node() {
    if (values.size() < M) return;
}

template <typename DT>
MTree<DT>* MTree<DT>::find_child(const DT& value) {
    for (size_t i = 0; i < values.size(); ++i) {
        if (value < values[i]) {
            return children.empty() ? nullptr : children[i];
        }
    }
    return children.empty() ? nullptr : children.back();
}

template <typename DT>
bool MTree<DT>::search(const DT& value) {
    if (find(value)) {
        return true;
    }

    if (!is_leaf()) {
        MTree* child = find_child(value);
        if (child) {
            return child->search(value);
        }
    }
    return false;
}

template <typename DT>
void MTree<DT>::remove(const DT& value) {
    auto it = custom_find(values, value);
    if (it != values.end()) {
        values.erase(it);
    } else {
        throw TreeException<DT>();
    }
}

template <typename DT>
void MTree<DT>::buildTree(vector<DT>& input_values) {
    for (const DT& value : input_values) {
        insert(value);
    }
}

template <typename DT>
vector<DT>& MTree<DT>::collect_values() {
    for (int val : values) {
        cout << val << " ";
    }
    return values;
}

template <typename DT>
bool MTree<DT>::find(const DT& value) {
    return custom_find(values, value) != values.end();
}

class duplicateInsertion : public exception {
    const char* what() const throw() override {
        return "Value already exists in tree";
    }
};

class NotFoundException : public exception {
     const char* what() const throw() override {
         return "Value not found in tree";
     }
};

int main() {
    try {
        int n;
        cin >> n;

        vector<int> input_values(n);
//        cout << "Enter " << n << " integers:" << endl;

        for (int i = 0; i < n; ++i) {
            cin >> input_values[i];
        }

        int M;
        cin >> M;
        MTree<int> tree(M);
        tree.buildTree(input_values);

        int numCommands;
        cin >> numCommands;
        cin.ignore();

        string line;
        while (numCommands-- > 0 && getline(cin, line)) {
            if (line.empty()) continue;
            char command = line[0];
            int value = 0;
            if (command != 'B' && line.length() > 2) {
                for (size_t j = 2; j < line.length(); ++j) {
                    if (isdigit(line[j])) {
                        value = value * 10 + (line[j] - '0');
                    }
                }
            }

           switch (command) {
               case 'I':
                   try {
                       tree.insert(value);
                       cout << value << " has been added to the tree." << endl;
                   } catch (const duplicateInsertion&) {
                       cout << "The value = " << value << " is already in the tree." << endl;
                   }
               break;
               case 'R':
                   try {
                       tree.remove(value);
                       cout << "The value = " << value << " has been removed." << endl;
                   } catch (const NotFoundException&) {
                       cout << "The value = " << value << " not found." << endl;
                   }
               break;
               case 'F':
                   if (tree.search(value)) {
                       cout << "The element with value = " << value << " was found." << endl;
                   } else {
                       cout << "The element with value = " << value << " not found." << endl;
                   }
               break;
               case 'B':
                   tree.collect_values();
               break;
               default:
                   cout << "Unknown Command" << endl;
           }
        }
        auto values = tree.collect_values();

    } catch (const TreeException<int>& ex) {
        cerr << "Caught TreeException: " << ex.what() << endl;
    } catch (const exception& ex) {
        cerr << "Caught exception: " << ex.what() << endl;
    }
    
    return 0;
}
