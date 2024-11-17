#include <iostream>
#include <vector>
using namespace std;

// Exception classes
class NotFoundException : public exception {
    const char* what() const throw() override {
        return "Value not found in tree";
    }
};

class DuplicateInsertionException : public exception {
    const char* what() const throw() override {
        return "Value already exists in tree";
    }
};

template <typename DT>
class MTree {
protected:
    const int M; // Maximum number of children per node (M+1 way split)
    vector<DT> values; // Values stored in the node (M-1 values)
    vector<MTree*> children; // Pointers to child MTrees (M+1 children)
    
    // Custom binary search implementation
    int binary_search_pos(const DT& value) const {
        int left = 0;
        int right = values.size() - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (values[mid] == value) return mid;
            if (values[mid] < value) left = mid + 1;
            else right = mid - 1;
        }
        return left;
    }

    // Custom find implementation
    bool binary_search_exists(const DT& value) const {
        int left = 0;
        int right = values.size() - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (values[mid] == value) return true;
            if (values[mid] < value) left = mid + 1;
            else right = mid - 1;
        }
        return false;
    }

    void validate_m() const {
        if (M < 2) {
            throw invalid_argument("M must be at least 2");
        }
    }

public:
    explicit MTree(int m) : M(m) {
        validate_m();
    }

    ~MTree() {
        for (auto* child : children) {
            delete child;
        }
    }

    bool is_leaf() const {
        return children.empty();
    }

    void insert(const DT& value) {
        if (search(value)) {
            throw DuplicateInsertionException();
        }

        if (is_leaf()) {
            const int pos = binary_search_pos(value);
            values.insert(values.begin() + pos, value);
            
            if (values.size() >= M) {
                split_node();
            }
        } else {
            find_child(value)->insert(value);
        }
    }

    void split_node() {
        if (!is_leaf()) return;

        vector<MTree*> new_children;
        vector<DT> new_values;
        
        const int values_per_child = values.size() / M;
        int extra_values = values.size() % M;
        int start_idx = 0;

        for (int i = 0; i < M; ++i) {
            const int child_size = values_per_child + (extra_values > 0 ? 1 : 0);
            if (extra_values > 0) --extra_values;

            auto* child = new MTree(M);
            
            // Copy values to child
            for (int j = 0; j < child_size; j++) {
                child->values.push_back(values[start_idx + j]);
            }
            
            new_children.push_back(child);
            
            if (i < M - 1 && start_idx + child_size < values.size()) {
                new_values.push_back(values[start_idx + child_size - 1]);
            }
            
            start_idx += child_size;
        }

        values = new_values;
        children = new_children;
    }

    MTree* find_child(const DT& value) const {
        if (is_leaf()) return nullptr;
        
        const int pos = binary_search_pos(value);
        return pos < values.size() && value > values[pos] 
            ? children[pos + 1] 
            : children[pos];
    }

    bool search(const DT& value) const {
        if (is_leaf()) {
            return binary_search_exists(value);
        }
        
        if (auto* child = find_child(value)) {
            return child->search(value);
        }
        return false;
    }

    void remove(const DT& value) {
        if (is_leaf()) {
            bool found = false;
            for (size_t i = 0; i < values.size(); ++i) {
                if (values[i] == value) {
                    values.erase(values.begin() + i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw NotFoundException();
            }
        } else {
            auto* child = find_child(value);
            if (!child) {
                throw NotFoundException();
            }
            child->remove(value);
        }
    }

    vector<DT> collect_values() const {
        vector<DT> all_values;
        
        if (is_leaf()) {
            all_values = values;
        } else {
            for (const auto* child : children) {
                auto child_values = child->collect_values();
                all_values.insert(all_values.end(), 
                                child_values.begin(), 
                                child_values.end());
            }
        }
        return all_values;
    }

    void buildTree(const vector<DT>& input_values) {
        // Clear existing tree
        for (auto* child : children) {
            delete child;
        }
        children.clear();
        values.clear();

        if (input_values.size() < M) {
            values = input_values;
            return;
        }

        const int values_per_child = input_values.size() / M;
        int extra_values = input_values.size() % M;
        int start_idx = 0;

        for (int i = 0; i < M; ++i) {
            const int child_size = values_per_child + (extra_values > 0 ? 1 : 0);
            if (extra_values > 0) --extra_values;

            vector<DT> child_values;
            for (int j = 0; j < child_size && start_idx < input_values.size(); ++j) {
                child_values.push_back(input_values[start_idx + j]);
            }
            
            auto* child = new MTree(M);
            child->buildTree(child_values);
            children.push_back(child);

            if (i < M - 1 && start_idx + child_size < input_values.size()) {
                values.push_back(input_values[start_idx + child_size - 1]);
            }
            
            start_idx += child_size;
        }
    }
};

int main() {
    int n, M, num_commands;
    char command;
    int value;
    
    cin >> n;  // First number (500) is ignored
    cin >> n;  // Actual size of the array
    
    vector<int> sorted_values(n);
    for (int i = 0; i < n; ++i) {
        cin >> sorted_values[i];
    }
    
    cin >> M;
    
    try {
        MTree<int>* tree = new MTree<int>(M);
        tree->buildTree(sorted_values);
        
        cin >> num_commands;
        
        for (int i = 0; i < num_commands; ++i) {
            cin >> command;
            
            switch (command) {
                case 'I': {
                    cin >> value;
                    try {
                        tree->insert(value);
                    } catch (const DuplicateInsertionException&) {
                        cout << "The value = " << value << " already in the tree." << endl;
                    }
                    break;
                }
                case 'R': {
                    cin >> value;
                    try {
                        tree->remove(value);
                        cout << "The value = " << value << " has been removed." << endl;
                    } catch (const NotFoundException&) {
                        cout << "The value = " << value << " not found." << endl;
                    }
                    break;
                }
                case 'F': {
                    cin >> value;
                    if (tree->search(value)) {
                        cout << "The element with value = " << value << " was found." << endl;
                    } else {
                        cout << "The element with value = " << value << " not found." << endl;
                    }
                    break;
                }
                case 'B': {
                    auto values = tree->collect_values();
                    tree->buildTree(values);
                    cout << "The tree has been rebuilt." << endl;
                    break;
                }
                default:
                    cout << "Invalid command!" << endl;
            }
        }

        // Print final list
        cout << "Final list:";
        auto final_values = tree->collect_values();
        int count = 0;
        for (const auto& val : final_values) {
            if (count % 10 == 0) cout << endl;
            cout << val << " ";
            count++;
        }
        cout << endl;

        delete tree;
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
