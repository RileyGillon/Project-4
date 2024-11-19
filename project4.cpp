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

template <typename DT>
class duplicateInsertion : public exception {
    const char* what() const throw() override {
        return "Value already exists in tree";
    }
};

template <typename DT>
class NotFoundException : public exception {
     const char* what() const throw() override {
         return "Value not found in tree";
     }
};

template <typename DT>
class MTree {
protected:
    int M; // Maximum number of children per node (M+1 way split)
    vector<DT> values; // Values stored in the node (M-1 values)
    vector<MTree*> children; // Pointers to child MTrees (M+1 children)
    
    // Custom find implementation
    typename vector<DT>::iterator custom_find(typename vector<DT>::iterator first, 
                                            typename vector<DT>::iterator last, 
                                            const DT& value) {
        for (auto it = first; it != last; ++it) {
            if (*it == value) return it;
        }
        return last;
    }
    
    // Custom sort implementation (bubble sort)
    void custom_sort(vector<DT>& arr) {
        int n = arr.size();
        for (int i = 0; i < n-1; i++) {
            for (int j = 0; j < n-i-1; j++) {
                if (arr[j] > arr[j+1]) {
                    DT temp = arr[j];
                    arr[j] = arr[j+1];
                    arr[j+1] = temp;
                }
            }
        }
    }
    
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

    void getFinalValues() const {
        cout << "Final list: ";
        for (const DT& val : values) {
            cout << val << " ";
        }
        cout << endl;
    }
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
    if (!find(value)) {
        values.push_back(value);
        custom_sort(values);

        if (values.size() >= M) {
            split_node();
        }
    } else {
        throw duplicateInsertion<DT>();
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
    auto it = custom_find(values.begin(), values.end(), value);
    if (it != values.end()) {
        values.erase(it);
    } else {
        throw NotFoundException<DT>();
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
    for (const DT& val : values) {
        cout << val << " ";
    }
    return values;
}

template <typename DT>
bool MTree<DT>::find(const DT& value) {
    return custom_find(values.begin(), values.end(), value) != values.end();
}


int main() {
    try {
        int n;
        cin >> n;

        vector<int> input_values(n);

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
                   } catch (const duplicateInsertion<int>& ex) {
                       cout << "The value = " << value << " already in the tree." << endl;
                   }
                   break;
               case 'R':
                   try {
                       tree.remove(value);
                       cout << "The value = " << value << " has been removed." << endl;
                   } catch (const NotFoundException<int>& ex) {
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
//                   tree.collect_values();
                   cout << "The tree has been rebuilt." << endl;
                   break;
               default:
                   cout << "Invalid command" << endl;
           }
        }
        
        tree.getFinalValues();
        
    } catch (const TreeException<int>& ex) {
        cerr << "Caught TreeException: " << ex.what() << endl;
    } catch (const exception& ex) {
        cerr << "Caught exception: " << ex.what() << endl;
    }

    return 0;
}
