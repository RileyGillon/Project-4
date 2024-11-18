#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;

// Exception classes
template <typename DT>
class TreeException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Tree operation exception";
    }
};

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

// M-Tree class definition
template <typename DT>
class MTree {
protected:
    int M; // Maximum number of children per node (M+1 way split)
    vector<DT> values; // Values stored in the node (M-1 values)
    vector<MTree*> children; // Pointers to child MTrees (M+1 children)

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
    void merge_values(vector<DT>& result) const;
};

// Constructor
template <typename DT>
MTree<DT>::MTree(int M) : M(M) {
    if (M < 2) {
        throw invalid_argument("M must be at least 2");
    }
}

// Destructor
template <typename DT>
MTree<DT>::~MTree() {
    for (MTree* child : children) {
        delete child;
    }
}

// Check if node is a leaf
template <typename DT>
bool MTree<DT>::is_leaf() const {
    return children.empty();
}

// Insert value into tree
template <typename DT>
void MTree<DT>::insert(const DT& value) {
    // Check for duplicates
    if (find(value)) {
        throw duplicateInsertion();
    }

    // If leaf node, insert directly
    if (is_leaf()) {
        values.push_back(value);
        sort(values.begin(), values.end());
        
        // Split if necessary
        if (values.size() >= M) {
            split_node();
        }
    } else {
        // Find appropriate child and insert there
        MTree* child = find_child(value);
        if (child) {
            child->insert(value);
        } else {
            // Create new leaf child if needed
            MTree* new_child = new MTree(M);
            new_child->insert(value);
            children.push_back(new_child);
            sort(children.begin(), children.end(), 
                [](MTree* a, MTree* b) { 
                    return a->values.front() < b->values.front(); 
                });
        }
    }
}

// Split node when it exceeds capacity
template <typename DT>
void MTree<DT>::split_node() {
    if (values.size() < M) return;
    
    size_t mid = values.size() / 2;
    vector<DT> left_values(values.begin(), values.begin() + mid);
    vector<DT> right_values(values.begin() + mid, values.end());
    
    // Create new nodes
    MTree* left_child = new MTree(M);
    MTree* right_child = new MTree(M);
    
    left_child->values = left_values;
    right_child->values = right_values;
    
    // Update current node
    values = vector<DT>{left_values.back()};
    children = vector<MTree*>{left_child, right_child};
    
    // If there were existing children, distribute them
    if (!is_leaf()) {
        size_t split_point = (children.size() + 1) / 2;
        vector<MTree*> left_children(children.begin(), children.begin() + split_point);
        vector<MTree*> right_children(children.begin() + split_point, children.end());
        
        left_child->children = left_children;
        right_child->children = right_children;
    }
}

// Find appropriate child for a value
template <typename DT>
MTree<DT>* MTree<DT>::find_child(const DT& value) {
    if (children.empty()) return nullptr;
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (value < values[i]) {
            return children[i];
        }
    }
    return children.back();
}

// Search for a value in the tree
template <typename DT>
bool MTree<DT>::search(const DT& value) {
    if (find(value)) return true;
    
    if (!is_leaf()) {
        MTree* child = find_child(value);
        if (child) {
            return child->search(value);
        }
    }
    return false;
}

// Remove a value from the tree
template <typename DT>
void MTree<DT>::remove(const DT& value) {
    auto it = std::find(values.begin(), values.end(), value);
    if (it != values.end()) {
        values.erase(it);
    } else {
        if (!is_leaf()) {
            MTree* child = find_child(value);
            if (child) {
                child->remove(value);
                return;
            }
        }
        throw NotFoundException();
    }
}

// Build tree from input values
template <typename DT>
void MTree<DT>::buildTree(vector<DT>& input_values) {
    // Clear existing tree
    for (MTree* child : children) {
        delete child;
    }
    children.clear();
    values.clear();
    
    // Sort input values if not already sorted
    sort(input_values.begin(), input_values.end());
    
    // Remove duplicates
    input_values.erase(
        unique(input_values.begin(), input_values.end()),
        input_values.end()
    );
    
    // If small enough, just store in current node
    if (input_values.size() < M) {
        values = input_values;
        return;
    }
    
    // Calculate split points
    size_t divisor = input_values.size() / M;
    size_t remainder = input_values.size() % M;
    
    // Create child nodes
    for (size_t i = 0; i < M; ++i) {
        size_t start = i * divisor;
        size_t end = (i == M-1) ? input_values.size() : (i + 1) * divisor;
        
        if (i == M-1) end += remainder;
        
        vector<DT> child_values(
            input_values.begin() + start,
            input_values.begin() + end
        );
        
        MTree* child = new MTree(M);
        child->buildTree(child_values);
        children.push_back(child);
        
        if (i < M-1) {
            values.push_back(input_values[end-1]);
        }
    }
}

// Helper function to merge values from all nodes
template <typename DT>
void MTree<DT>::merge_values(vector<DT>& result) const {
    if (is_leaf()) {
        result.insert(result.end(), values.begin(), values.end());
    } else {
        for (MTree* child : children) {
            child->merge_values(result);
        }
    }
}

// Collect all values from the tree
template <typename DT>
vector<DT>& MTree<DT>::collect_values() {
    static vector<DT> result;
    result.clear();
    merge_values(result);
    sort(result.begin(), result.end());
    
    // Print values as required
    for (const DT& val : result) {
        cout << val << " ";
    }
    cout << endl;
    
    return result;
}

// Find a value in the current node
template <typename DT>
bool MTree<DT>::find(const DT& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

// Main function
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

    } catch (const TreeException<int>& ex) {
        cerr << "Caught TreeException: " << ex.what() << endl;
    } catch (const exception& ex) {
        cerr << "Caught exception: " << ex.what() << endl;
    }
    
    return 0;
}
