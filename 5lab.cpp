#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <map>
#include <iomanip>
#include <cmath>
using namespace std;

// Структура задачи
struct Task {
    int id;
    double base_priority;
    int duration;
    int time_created;
    double current_priority;
    int start_time;
    int end_time;
};

// Узел AVL дерева
struct AVLNode {
    int key;
    Task* task;
    AVLNode* left;
    AVLNode* right;
    int height;
    AVLNode(int k, Task* t) : key(k), task(t), left(nullptr), right(nullptr), height(1) {}
};

// AVL дерево для истории задач
class AVLTree {
private:
    AVLNode* root;

    int height(AVLNode* node) {
        return node ? node->height : 0;
    }

    int balanceFactor(AVLNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    void updateHeight(AVLNode* node) {
        if (node) {
            node->height = 1 + max(height(node->left), height(node->right));
        }
    }

    AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    AVLNode* balance(AVLNode* node) {
        if (!node) return nullptr;

        updateHeight(node);
        int bf = balanceFactor(node);

        if (bf > 1) {
            if (balanceFactor(node->left) < 0) {
                node->left = rotateLeft(node->left);
            }
            return rotateRight(node);
        }
        if (bf < -1) {
            if (balanceFactor(node->right) > 0) {
                node->right = rotateRight(node->right);
            }
            return rotateLeft(node);
        }
        return node;
    }

    AVLNode* insert(AVLNode* node, int key, Task* task) {
        if (!node) return new AVLNode(key, task);

        if (key < node->key) {
            node->left = insert(node->left, key, task);
        }
        else if (key > node->key) {
            node->right = insert(node->right, key, task);
        }
        else {
            return node; // Ключи уникальны
        }

        return balance(node);
    }

    AVLNode* minValueNode(AVLNode* node) {
        AVLNode* current = node;
        while (current && current->left) {
            current = current->left;
        }
        return current;
    }

    AVLNode* deleteNode(AVLNode* root, int key) {
        if (!root) return root;

        if (key < root->key) {
            root->left = deleteNode(root->left, key);
        }
        else if (key > root->key) {
            root->right = deleteNode(root->right, key);
        }
        else {
            if (!root->left || !root->right) {
                AVLNode* temp = root->left ? root->left : root->right;
                if (!temp) {
                    temp = root;
                    root = nullptr;
                }
                else {
                    *root = *temp;
                }
                delete temp;
            }
            else {
                AVLNode* temp = minValueNode(root->right);
                root->key = temp->key;
                root->task = temp->task;
                root->right = deleteNode(root->right, temp->key);
            }
        }

        if (!root) return root;
        return balance(root);
    }

    Task* search(AVLNode* root, int key) {
        if (!root) return nullptr;
        if (key == root->key) return root->task;
        if (key < root->key) return search(root->left, key);
        return search(root->right, key);
    }

    void clear(AVLNode* node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

public:
    AVLTree() : root(nullptr) {}
    ~AVLTree() { clear(root); }

    void insert(int key, Task* task) {
        root = insert(root, key, task);
    }

    void remove(int key) {
        root = deleteNode(root, key);
    }

    Task* search(int key) {
        return search(root, key);
    }
};

// Двоичная куча для управления задачами
class MaxHeap {
private:
    vector<Task*> heap;
    int current_time;
    double aging_factor;

    void updatePriorities() {
        for (Task* task : heap) {
            task->current_priority = task->base_priority +
                (current_time - task->time_created) * aging_factor;
        }
    }

    void heapifyDown(int index) {
        int size = heap.size();
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < size && heap[left]->current_priority > heap[largest]->current_priority) {
            largest = left;
        }
        if (right < size && heap[right]->current_priority > heap[largest]->current_priority) {
            largest = right;
        }

        if (largest != index) {
            swap(heap[index], heap[largest]);
            heapifyDown(largest);
        }
    }

    void buildHeap() {
        for (int i = heap.size() / 2 - 1; i >= 0; i--) {
            heapifyDown(i);
        }
    }

public:
    MaxHeap() : aging_factor(0.1), current_time(0) {}

    void setCurrentTime(int time) {
        current_time = time;
    }

    void addTask(Task* task) {
        heap.push_back(task);
    }

    bool isEmpty() const {
        return heap.empty();
    }

    Task* extractMax() {
        if (heap.empty()) return nullptr;

        updatePriorities();
        buildHeap();

        Task* maxTask = heap[0];
        heap[0] = heap.back();
        heap.pop_back();

        if (!heap.empty()) {
            heapifyDown(0);
        }

        return maxTask;
    }
};

// Генератор случайных задач
vector<Task> generateTasks(int n) {
    vector<Task> tasks;
    for (int i = 0; i < n; i++) {
        Task t;
        t.id = i;
        t.base_priority = rand() % 100 + 1; // Приоритет 1-100
        t.duration = rand() % 10 + 1;       // Длительность 1-10
        t.time_created = 0;                 // Все задачи создаются в момент времени 0
        tasks.push_back(t);
    }
    return tasks;
}

// Визуализация ленты времени
void visualizeTimeline(const vector<Task*>& tasks, int total_time, int processor_id) {
    if (tasks.empty()) {
        cout << "Processor " << processor_id << ": No tasks executed." << endl;
        return;
    }

    const int timeline_width = 80;
    double scale = static_cast<double>(timeline_width) / total_time;
    vector<string> bands(3, string(timeline_width, ' '));

    for (Task* task : tasks) {
        int start_pos = round(task->start_time * scale);
        int end_pos = round(task->end_time * scale);
        if (end_pos > timeline_width) end_pos = timeline_width;
        if (start_pos >= timeline_width) continue;

        // Верхняя граница
        for (int i = start_pos; i < end_pos; i++) {
            bands[0][i] = (i == start_pos) ? '[' : '-';
            if (i == end_pos - 1) bands[0][i] = ']';
        }

        // Тело задачи
        string task_id = "T" + to_string(task->id);
        int id_pos = start_pos + 1;
        for (int i = 0; i < task_id.size() && id_pos + i < end_pos - 1; i++) {
            bands[1][id_pos + i] = task_id[i];
        }

        // Нижняя граница
        for (int i = start_pos; i < end_pos; i++) {
            bands[2][i] = (i == start_pos || i == end_pos - 1) ? '|' : '-';
        }
    }

    cout << "Processor " << processor_id << " timeline:" << endl;
    for (const string& band : bands) {
        cout << band << endl;
    }

    // Метки времени
    cout << "0";
    for (int i = 1; i <= 10; i++) {
        int pos = round(i * total_time / 10.0 * scale);
        if (pos < timeline_width) {
            cout << setw(pos) << i * total_time / 10;
        }
    }
    cout << endl << endl;
}

int main() {
    srand(time(nullptr));
    int num_tasks = 20;
    int num_processors = 3;

    // Генерация задач и инициализация AVL дерева
    vector<Task> tasks = generateTasks(num_tasks);
    AVLTree historyTree;
    for (Task& t : tasks) {
        historyTree.insert(t.id, &t);
    }

    // Распределение задач по процессорам
    vector<MaxHeap> processors(num_processors);
    vector<vector<Task*>> completed_tasks(num_processors);

    for (Task& t : tasks) {
        int processor_id = rand() % num_processors;
        processors[processor_id].addTask(&t);
    }

    // Выполнение задач на процессорах
    for (int i = 0; i < num_processors; i++) {
        int current_time = 0;
        processors[i].setCurrentTime(current_time);

        while (!processors[i].isEmpty()) {
            Task* task = processors[i].extractMax();
            if (!task) continue;

            task->start_time = current_time;
            current_time += task->duration;
            task->end_time = current_time;
            completed_tasks[i].push_back(task);
        }

        // Визуализация ленты времени для процессора
        int total_time = current_time;
        visualizeTimeline(completed_tasks[i], total_time, i);
    }

    // Поиск задачи по ID
    int search_id;
    cout << "Enter task ID to search (-1 to exit): ";
    while (cin >> search_id && search_id != -1) {
        Task* task = historyTree.search(search_id);
        if (task) {
            cout << "Task " << task->id << ": "
                << "Priority=" << task->base_priority << ", "
                << "Duration=" << task->duration << ", "
                << "Created at=" << task->time_created << ", "
                << "Executed=[" << task->start_time << "-" << task->end_time << "]\n";
        }
        else {
            cout << "Task not found.\n";
        }
        cout << "Enter task ID to search (-1 to exit): ";
    }

    return 0;
}
