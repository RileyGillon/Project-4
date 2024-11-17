#include <iostream>
#include <vector>
using namespace std;

class NotFoundException : public exception {
    const char* what() const throw() override {
        return "Value not found in tree";
    }
};

class duplicateInsertion : public exception {
    const char* what() const throw() override {
        return "Value already exists in tree";
    }
};

template <typename DT>
class MTree {
protected:
    int M;
    vector<DT> values;
    vector<MTree*> children;

public:
    MTree(int m) : M(m) {
        if (m < 2) {
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
        if (find(value)) {
            throw duplicateInsertion();
        }

        if (is_leaf()) {
            int pos = 0;
            while (pos < values.size() && values[pos] < value) {
                pos++;
            }
            values.insert(values.begin() + pos, value);
            
            if (values.size() >= M) {
                split_node();
            }
        } else {
            find_child(value)->insert(value);
        }

        cout << "The value = " << value << " has been inserted." << endl;
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
            for (int j = 0; j < child_size && start_idx < values.size(); ++j) {
                child->values.push_back(values[start_idx++]);
            }
            new_children.push_back(child);
            
            if (i < M - 1 && start_idx < values.size()) {
                new_values.push_back(values[start_idx - 1]);
            }
        }

        values = new_values;
        children = new_children;
    }

    MTree* find_child(const DT& value) const {
        for (size_t i = 0; i < values.size(); ++i) {
            if (value <= values[i]) {
                return children[i];
            }
        }
        return children.back();
    }

    bool find(const DT& value) const {
        for (const auto& v : values) {
            if (v == value) return true;
        }
        
        if (!is_leaf()) {
            MTree* child = find_child(value);
            if (child) return child->find(value);
        }
        
        return false;
    }

    void remove(const DT& value) {
        if (!find(value)) {
            throw NotFoundException();
        }
        
        if (is_leaf()) {
            for (size_t i = 0; i < values.size(); ++i) {
                if (values[i] == value) {
                    values.erase(values.begin() + i);
                    break;
                }
            }
        } else {
            MTree* child = find_child(value);
            if (child) {
                child->remove(value);
            }
        }
    }

    vector<DT> collect_values() const {
        vector<DT> result;
        
        if (is_leaf()) {
            result = values;
        } else {
            for (auto* child : children) {
                auto child_values = child->collect_values();
                result.insert(result.end(), child_values.begin(), child_values.end());
            }
        }
        
        return result;
    }

    void buildTree(vector<DT>& input_values) {
        for (auto* child : children) {
            delete child;
        }
        children.clear();
        values.clear();

        if (input_values.empty()) return;

        if (input_values.size() < M) {
            values = input_values;
            return;
        }

        int values_per_child = (input_values.size() + M - 1) / M;
        int start_idx = 0;

        for (int i = 0; i < M && start_idx < input_values.size(); ++i) {
            vector<DT> child_values;
            int child_size = min(values_per_child, (int)input_values.size() - start_idx);
            
            for (int j = 0; j < child_size; ++j) {
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
    int n;
    cin >> n;  // Ignore first number
    cin >> n;  // Read actual array size
    
    vector<int> initial_values(n);
    for (int i = 0; i < n; ++i) {
        cin >> initial_values[i];
    }
    
    int M;
    cin >> M;
    
    try {
        MTree<int>* tree = new MTree<int>(M);
        tree->buildTree(initial_values);
        
        int numCommands;
        cin >> numCommands;
        cin.ignore();
        
        string line;
        while (numCommands-- > 0) {
            getline(cin, line);
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
                        tree->insert(value);
                    } catch (const duplicateInsertion&) {
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
                    if (tree->find(value)) {
                        cout << "The element with value = " << value << " was found." << endl;
                    } else {
                        cout << "The element with value = " << value << " not found." << endl;
                    }
                    break;
                    
                case 'B':
                    {
                        auto values = tree->collect_values();
                        tree->buildTree(values);
                        cout << "The tree has been rebuilt." << endl;
                    }
                    break;
            }
        }

        vector<int> final_values = tree->collect_values();
        if (!final_values.empty()) {
            cout << "Final list:";
            for (size_t i = 0; i < final_values.size(); ++i) {
                if (i % 20 == 0) cout << " ";
                cout << final_values[i];
                if (i == final_values.size() - 1) {
                    cout << endl;
                } else if ((i + 1) % 20 == 0) {
                    cout << "\n";
                } else {
                    cout << " ";
                }
            }
        }

        delete tree;
        
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
