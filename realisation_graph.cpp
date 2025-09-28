#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>

using namespace std;

// ====== Структура узла ======
struct Node {
    string name; // имя вершины

    // сравнение двух вершин (по имени)
    bool operator==(const Node& other) const {
        return name == other.name;
    }
};

// ====== Структура ребра ====== 
struct Edge {
    Node* from;   // вершина-источник
    Node* to;     // вершина-назначение
    float weight; // вес (0, если граф невзвешенный)

    Edge(Node* f, Node* t, float w) : from(f), to(t), weight(w) {}
};

// ====== Хэш-функция для unordered_map ======
struct NodeHash {
    size_t operator()(const Node& n) const {
        return hash<string>()(n.name); // считаем хэш по имени вершины
    }
};

// ====== Класс Graph ====== 
class Graph {
private:
    vector<Node> nodes; // список всех вершин
    unordered_map<Node, vector<Edge>, NodeHash> adjacencyList; // список смежности
    bool isOrient;   // true = ориентированный граф
    bool isWeighted; // true = взвешенный граф

public:
    // конструктор
    Graph(bool orient, bool weighted)
        : isOrient(orient), isWeighted(weighted) {}

    // Создание графа из файла
    static Graph fromFile(const string& path) {
        ifstream file(path);
        if (!file.is_open()) {
            throw runtime_error("Не удалось открыть файл " + path);
        }

        // читаем тип графа
        int orientFlag, weightedFlag;
        file >> orientFlag >> weightedFlag;
        Graph g(orientFlag, weightedFlag);

        // читаем количество вершин
        int N;
        file >> N;

        // читаем вершины
        for (int i = 0; i < N; i++) {
            string nodeName;
            file >> nodeName;
            g.addNode(Node{nodeName});
        }

        // читаем рёбра (пока не закончится файл)
        string from, to;
        float w = 0;
        while (file >> from >> to) {
            if (g.isWeighted) file >> w; // вес читаем только если граф взвешенный
            Node* v = g.findNodePtr(from);
            Node* u = g.findNodePtr(to);
            if (v && u) g.addEdge(v, u, w);
        }
        return g;
    }

    // Создание графа через консоль
    static Graph fromConsole() {
        int orientFlag, weightedFlag;

        cout << "Ориентированный? (1 - да, 0 - нет): ";
        cin >> orientFlag;

        cout << "Взвешенный? (1 - да, 0 - нет): ";
        cin >> weightedFlag;

        Graph g(orientFlag, weightedFlag);

        // вводим вершины
        int N;
        cout << "Введите количество вершин: ";
        cin >> N;

        cout << "Введите имена вершин:\n";
        for (int i = 0; i < N; i++) {
            string name;
            cin >> name;
            g.addNode(Node{name});
        }

        // вводим рёбра
        int M;
        cout << "Введите количество рёбер: ";
        cin >> M;

        cout << "Введите рёбра (формат: from to [weight]):\n";
        for (int i = 0; i < M; i++) {
            string from, to;
            float w = 0;
            cin >> from >> to;
            if (weightedFlag) cin >> w; // вес только если граф взвешенный
            Node* v = g.findNodePtr(from);
            Node* u = g.findNodePtr(to);
            if (v && u) g.addEdge(v, u, w);
        }
        return g;
    }

    // Добавление вершины
    void addNode(const Node& n) {
        if (contains(n)) return;    // если вершина уже есть — не добавляем
        nodes.push_back(n);         // добавляем в список
        adjacencyList[n] = {};      // создаём пустой список смежности
    }

    // Поиск вершины по имени
    Node* findNodePtr(const string& name) {
        for (auto& node : nodes) {
            if (node.name == name) return &node;
        }
        return nullptr;
    }

    // Проверка наличия вершины
    bool contains(const Node& n) const {
        return find(nodes.begin(), nodes.end(), n) != nodes.end();
    }

    // Добавление ребра
    void addEdge(Node* v, Node* w, float weight = 0) {
        float realWeight = isWeighted ? weight : 0;
        adjacencyList[*v].push_back(Edge(v, w, realWeight)); // v → w
        if (!isOrient) adjacencyList[*w].push_back(Edge(w, v, realWeight)); // если неориентированный → обратное ребро
    }

    // Вывод графа
    void printGraph() const {
        for (const auto& node : nodes) {
            cout << node.name << ": ";
            auto it = adjacencyList.find(node);
            if (it != adjacencyList.end()) {
                for (const auto& edge : it->second) {
                    cout << "(" << edge.to->name
                         << ", w=" << edge.weight << ") ";
                }
            }
            cout << "\n";
        }
    }
};



int main() {
    try {
        int choice;
        cout << "Выберите способ создания графа:\n";
        cout << "1 - из файла\n";
        cout << "2 - ввод с консоли\n";
        cin >> choice;

        // создаём граф
        Graph g = (choice == 1)
                  ? Graph::fromFile("graph.txt")
                  : Graph::fromConsole();

        // ======= здесь надо сделать какое-то интерактивное меню =======
        while (true) {
            cout << "\nМеню:\n";
            cout << "1 - Добавить вершину\n";
            cout << "2 - Добавить ребро\n";
            cout << "3 - Удалить вершину\n";
            cout << "4 - Удалить ребро\n";
            cout << "5 - Показать граф\n";
            cout << "0 - Выход\n";
            int action;
            cin >> action;

            if (action == 0) break; // выход из цикла

            switch (action) {
                case 1: { // добавить вершину
                    string name;
                    cout << "Имя вершины: ";
                    cin >> name;
                    g.addNode(Node{name});
                    break;
                }
                case 2: { // добавить ребро
                    string from, to;
                    float weight = 0;
                    cout << "Ребро (from to weight): ";
                    cin >> from >> to;
                    if (g.isWeighted) cin >> weight;
                    Node* f = g.findNodePtr(from);
                    Node* t = g.findNodePtr(to);
                    if (f && t) g.addEdge(f, t, weight);
                    break;
                }
                case 3: { // удалить вершину
                    string name;
                    cout << "Имя вершины для удаления: ";
                    cin >> name;
                    deleteNode(g, name);
                    break;
                }
                case 4: { // удалить ребро
                    string from, to;
                    cout << "Ребро для удаления (from to): ";
                    cin >> from >> to;
                    deleteEdge(g, from, to);
                    break;
                }
                case 5: // показать граф
                    g.printGraph();
                    break;
                default:
                    cout << "Некорректный выбор\n";
            }
        }
        // ======= конец меню =======
    } catch (exception& e) {
        cerr << e.what() << "\n";
    }
    return 0;
}
