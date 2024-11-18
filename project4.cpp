#include <iostream>
#include <vector>
using namespace std;

// Custom exceptions
class duplicateInsertion : public std::runtime_error {
public:
    duplicateInsertion() : runtime_error("Duplicate value") {}
};

class NotFoundException : public std::runtime_error {
public:
    NotFoundException() : runtime_error("Value not found") {}
};

template <typename DT>
class MTree {
protected:
    int M;                     // Maximum number of children per node
    vector<DT> values;         // Values stored in the node (M-1 values)
    vector<MTree*> children;   // Pointers to child MTrees (M+1 children)

    // Helper function to sort vector without using algorithm header
    void sortVector(vector<DT>& vec) {
        for (size_t i = 0; i < vec.size(); i++) {
            for (size_t j = i + 1; j < vec.size(); j++) {
                if (vec[j] < vec[i]) {
                    DT temp = vec[i];
                    vec[i] = vec[j];
                    vec[j] = temp;
                }
            }
        }
    }

    // Helper function for binary search without using algorithm header
    bool binarySearchHelper(const vector<DT>& vec, const DT& value) {
        int left = 0;
        int right = vec.size() - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (vec[mid] == value) return true;
            if (vec[mid] < value) left = mid + 1;
            else right = mid - 1;
        }
        return false;
    }

    // Helper function to find insert position
    size_t findInsertPosition(const vector<DT>& vec, const DT& value) {
        size_t pos = 0;
        while (pos < vec.size() && vec[pos] < value) {
            pos++;
        }
        return pos;
    }

public:
    MTree(int M) : M(M) {}

    ~MTree() {
        for (auto child : children) {
            delete child;
        }
    }

    bool is_leaf() const {
        return children.empty();
    }

    void insert(const DT& value) {
        if (is_leaf()) {
            // Check for duplicates
            for (const DT& v : values) {
                if (v == value) {
                    throw duplicateInsertion();
                }
            }

            // Insert the value in sorted order
            size_t pos = findInsertPosition(values, value);
            values.insert(values.begin() + pos, value);

            // Split if necessary
            if (values.size() >= M) {
                split_node();
            }
        } else {
            MTree* child = find_child(value);
            child->insert(value);
        }
    }

    void split_node() {
        if (is_leaf()) {
            // Create new children for the current node
            int partitionSize = values.size() / M;
            int remainder = values.size() % M;
            int currentPos = 0;

            for (int i = 0; i < M; ++i) {
                MTree* newChild = new MTree(M);
                int endPos = currentPos + partitionSize + (i < remainder ? 1 : 0);
                
                // Copy values to the new child
                newChild->values.insert(
                    newChild->values.end(),
                    values.begin() + currentPos,
                    values.begin() + endPos
                );
                
                children.push_back(newChild);
                currentPos = endPos;
            }

            // Keep the split values in the current node
            vector<DT> splitValues;
            for (int i = 1; i < M; ++i) {
                splitValues.push_back(children[i]->values.front());
            }
            values = splitValues;
        }
    }

    MTree* find_child(const DT& value) {
        if (is_leaf() || children.empty()) {
            return this;
        }

        for (size_t i = 0; i < values.size(); ++i) {
            if (value < values[i]) {
                return children[i];
            }
        }
        return children.back();
    }

    bool find(DT& value) {
        if (is_leaf()) {
            return binarySearchHelper(values, value);
        }
        MTree* child = find_child(value);
        return child->find(value);
    }

    void remove(const DT& value) {
        if (is_leaf()) {
            size_t pos = findInsertPosition(values, value);
            if (pos >= values.size() || values[pos] != value) {
                throw NotFoundException();
            }
            values.erase(values.begin() + pos);
        } else {
            MTree* child = find_child(value);
            child->remove(value);
        }
    }

    vector<DT>& collect_values() {
        static vector<DT> result;
        result.clear();
        
        if (is_leaf()) {
            result = values;
        } else {
            for (auto child : children) {
                vector<DT> childValues = child->collect_values();
                result.insert(result.end(), childValues.begin(), childValues.end());
            }
        }
        return result;
    }

    void buildTree(vector<DT>& input_values) {
        // Clear existing tree
        for (auto child : children) {
            delete child;
        }
        children.clear();
        values.clear();

        // Sort input values
        sortVector(input_values);

        if (input_values.size() < M) {
            // If we have fewer values than M, store them in this leaf node
            values = input_values;
        } else {
            // Split values into M parts
            int partitionSize = input_values.size() / M;
            int remainder = input_values.size() % M;
            int currentPos = 0;

            // Create child nodes
            for (int i = 0; i < M; ++i) {
                MTree* newChild = new MTree(M);
                int endPos = currentPos + partitionSize + (i < remainder ? 1 : 0);
                
                vector<DT> childValues(
                    input_values.begin() + currentPos,
                    input_values.begin() + endPos
                );
                newChild->buildTree(childValues);
                
                children.push_back(newChild);
                if (i > 0) {
                    values.push_back(input_values[currentPos]);
                }
                currentPos = endPos;
            }
        }
    }
};

int main() {
    int n;  // number of numbers in the initial sorted array
    int M;
    int numCommands;
    char command;
    int value;

    // Read n
    cin >> n;
    
    vector<int> mySortedValues(n);
    // Read n numbers into the vector
    for (int i = 0; i < n; i++) {
        cin >> mySortedValues[i];
    }

    // Get the M value
    cin >> M;
    
    MTree<int>* myTree = new MTree<int>(M);
    
    // Build the tree
    myTree->buildTree(mySortedValues);
    
    cin >> numCommands; // Read the number of commands
    
    for (int i = 0; i < numCommands; i++) {
        cin >> command; // Read the command type
        
        switch (command) {
            case 'I': {  // Insert
                cin >> value;
                try {
                    myTree->insert(value);
                    cout << "The value = " << value << " has been inserted." << endl;
                } catch (const duplicateInsertion& e) {
                    cout << "The value = " << value << " already in the tree." << endl;
                }
                break;
            }
            case 'R': {  // Remove
                cin >> value;
                try {
                    myTree->remove(value);
                    cout << "The value = " << value << " has been removed." << endl;
                } catch (const NotFoundException& e) {
                    cout << "The value = " << value << " not found." << endl;
                }
                break;
            }
            case 'F': {  // Find
                cin >> value;
                if (myTree->find(value)) {
                    cout << "The element with value = " << value << " was found." << endl;
                } else {
                    cout << "The element with value = " << value << " not found." << endl;
                }
                break;
            }
            case 'B': {  // rebuild tree
                vector<int> myValues = myTree->collect_values();
                myTree->buildTree(myValues);
                cout << "The tree has been rebuilt." << endl;
                break;
            }
            default:
                cout << "Invalid command!" << endl;
        }
    }

    delete myTree;
    return 0;
}
