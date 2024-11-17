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

    bool binary_search(const DT& value) const {
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

    int find_position(const DT& value) const {
        int pos = 0;
        while (pos < values.size() && values[pos] < value) pos++;
        return pos;
    }

public:
    explicit MTree(int m) : M(m) {
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

    bool find(const DT& value) const {
        if (binary_search(value)) return true;
        
        if (!is_leaf()) {
            int pos = find_position(value);
            if (pos < children.size()) {
                return children[pos]->find(value);
            }
        }
        return false;
    }

    void insert(const DT& value) {
        if (find(value)) {
            throw duplicateInsertion();
        }

        if (is_leaf()) {
            int pos = find_position(value);
            values.insert(values.begin() + pos, value);
            
            if (values.size() >= M) {
                split_node();
            }
        } else {
            int pos = find_position(value);
            if (pos == children.size()) pos--;
            children[pos]->insert(value);
        }
    }

    void remove(const DT& value) {
        if (!find(value)) {
            throw NotFoundException();
        }

        if (is_leaf()) {
            int pos = find_position(value);
            if (pos < values.size() && values[pos] == value) {
                values.erase(values.begin() + pos);
            } else {
                throw NotFoundException();
            }
        } else {
            int pos = find_position(value);
            if (pos == children.size()) pos--;
            children[pos]->remove(value);
        }
    }

    void split_node() {
        if (!is_leaf()) return;

        vector<MTree*> new_children;
        vector<DT> new_values;
        
        int values_per_child = (values.size() + M - 1) / M;
        int start = 0;

        for (int i = 0; i < M && start < values.size(); ++i) {
            auto* child = new MTree(M);
            int end = min(start + values_per_child, (int)values.size());
            
            for (int j = start; j < end; ++j) {
                child->values.push_back(values[j]);
            }
            new_children.push_back(child);
            
            if (i < M - 1 && end < values.size()) {
                new_values.push_back(values[end - 1]);
            }
            start = end;
        }

        values = new_values;
        children = new_children;
    }

    vector<DT> collect_values() const {
        vector<DT> result;
        
        if (is_leaf()) {
            result = values;
        } else {
            for (const auto* child : children) {
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
        int start = 0;

        for (int i = 0; i < M && start < input_values.size(); ++i) {
            vector<DT> child_values;
            int end = min(start + values_per_child, (int)input_values.size());
            
            for (int j = start; j < end; ++j) {
                child_values.push_back(input_values[j]);
            }
            
            auto* child = new MTree(M);
            child->buildTree(child_values);
            children.push_back(child);
            
            if (i < M - 1 && end < input_values.size()) {
                values.push_back(input_values[end - 1]);
            }
            start = end;
        }
    }
};

int main() {
    int n;
    cin >> n >> n;
    
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
                    auto values = tree->collect_values();
                    tree->buildTree(values);
                    cout << "The tree has been rebuilt." << endl;
                    break;
            }
        }

        auto final_values = tree->collect_values();
        if (!final_values.empty()) {
            cout << "Final list: ";
            for (size_t i = 0; i < final_values.size(); ++i) {
                cout << final_values[i];
                if ((i + 1) % 20 == 0 && i != final_values.size() - 1) {
                    cout << "\n";
                } else if (i != final_values.size() - 1) {
                    cout << " ";
                }
            }
            cout << endl;
        }

        delete tree;
        
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
