#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

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
    const int M;
    vector<DT> values;
    vector<MTree*> children;

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

public:
    explicit MTree(int m) : M(m) {
        if (M < 2) {
            throw invalid_argument("M must be at least 2");
        }
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
            int pos = binary_search_pos(value);
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
        
        int values_per_child = values.size() / M;
        int extra_values = values.size() % M;
        int start_idx = 0;

        for (int i = 0; i < M; ++i) {
            int child_size = values_per_child + (extra_values > 0 ? 1 : 0);
            if (extra_values > 0) --extra_values;

            auto* child = new MTree(M);
            
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
        
        int pos = binary_search_pos(value);
        return children[pos < children.size() ? pos : children.size() - 1];
    }

    bool search(const DT& value) const {
        for (const auto& v : values) {
            if (v == value) return true;
        }
        
        if (!is_leaf()) {
            MTree* child = find_child(value);
            if (child) return child->search(value);
        }
        
        return false;
    }

    void remove(const DT& value) {
        if (is_leaf()) {
            for (size_t i = 0; i < values.size(); ++i) {
                if (values[i] == value) {
                    values.erase(values.begin() + i);
                    return;
                }
            }
            throw NotFoundException();
        } else {
            MTree* child = find_child(value);
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
                all_values.insert(all_values.end(), child_values.begin(), child_values.end());
            }
        }
        return all_values;
    }

    void buildTree(const vector<DT>& input_values) {
        for (auto* child : children) {
            delete child;
        }
        children.clear();
        values.clear();

        if (input_values.empty()) {
            return;
        }

        if (input_values.size() < M) {
            values = input_values;
            return;
        }

        int values_per_child = input_values.size() / M;
        int extra_values = input_values.size() % M;
        int start_idx = 0;

        for (int i = 0; i < M; ++i) {
            int child_size = values_per_child + (extra_values > 0 ? 1 : 0);
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
    
    cin >> n;
    cin >> n;
    
    vector<int> sorted_values(n);
    for (int i = 0; i < n; ++i) {
        cin >> sorted_values[i];
    }
    
    cin >> M;
    cin >> num_commands;
    cin.ignore();
    
    try {
        MTree<int>* tree = new MTree<int>(M);
        tree->buildTree(sorted_values);
        
        string line;
        while (num_commands-- > 0 && getline(cin, line)) {
            istringstream iss(line);
            char command;
            int value;
            
            iss >> command;
            if (command != 'B') {
                if (!(iss >> value)) continue;
            }
            
            switch (command) {
                case 'I':
                    try {
                        tree->insert(value);
                    } catch (const DuplicateInsertionException&) {
                        cout << "The value = " << value << " already in the tree." << endl;
                    }
                    break;
                case 'R':
                    try {
                        tree->remove(value);
                        cout << "The value = " << value << " has been removed." << endl;
                    } catch (const NotFoundException&) {
                        cout << "The value = " << value << " not found." << endl;
                    }
                    break;
                case 'F':
                    if (tree->search(value)) {
                        cout << "The element with value = " << value << " was found." << endl;
                    } else {
                        cout << "The element with value = " << value << " not found." << endl;
                    }
                    break;
                case 'B': {
                    auto values = tree->collect_values();
                    tree->buildTree(values);
                    cout << "The tree has been rebuilt." << endl;
                    break;
                }
            }
        }

        cout << "Final list:" << endl;
        auto final_values = tree->collect_values();
        for (size_t i = 0; i < final_values.size(); ++i) {
            cout << final_values[i];
            if ((i + 1) % 10 == 0 || i == final_values.size() - 1) {
                cout << endl;
            } else {
                cout << " ";
            }
        }

        delete tree;
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
