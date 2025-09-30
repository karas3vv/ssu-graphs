#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>

using namespace std;

// структуры

// структура вершины
struct Node {
    string name;

    // сравнение вершин по имени
    bool operator==(const Node& other) const {
        return name == other.name;
    }
};

// структура ребра
struct Edge {
    Node* from;
    Node* to;
    float weight;
};

// хэш-функция для Node (чтобы использовать в unordered_map)
struct NodeHash {
    size_t operator()(const Node& n) const {
        return hash<string>()(n.name);
    }
};

// класс Graph
class Graph {
public:
    vector<Node> nodes;                                      // список вершин
    unordered_map<Node, vector<Edge>, NodeHash> adjacencyList; // список смежности
    bool isOrient;    // ориентированный
    bool isWeighted;  // взвешенный

    // конструктор по умолчанию
    Graph(bool orient = false, bool weighted = false)
        : isOrient(orient), isWeighted(weighted) {}

    // поиск вершины по имени
    Node* findNodePtr(const string& name) {
        for (auto& n : nodes) {
            if (n.name == name) return &n;
        }
        return nullptr;
    }

    // добавление вершины
    void addNode(const Node& n) {
        if (findNodePtr(n.name)) {
            cout << "Вершина " << n.name << " уже существует.\n";
            return;
        }
        nodes.push_back(n);
        adjacencyList[n] = {};
        cout << "Вершина " << n.name << " добавлена.\n";
    }

    // добавление ребра
    void addEdge(Node* from, Node* to, float weight = 0) {
        if (!from || !to) {
            cout << "Ошибка: одна из вершин не найдена.\n";
            return;
        }

        float w = isWeighted ? weight : 0;
        adjacencyList[*from].push_back({from, to, w});

        // если граф неориентированный — добавить обратное ребро
        if (!isOrient) {
            adjacencyList[*to].push_back({to, from, w});
        }

        cout << "Ребро " << from->name << " -> " << to->name
             << " добавлено (вес " << w << ").\n";
    }

    // удаление вершины
    void deleteNode(const string& name) {
        Node* n = findNodePtr(name);
        if (!n) {
            cout << "Вершина не найдена.\n";
            return;
        }

        // удалить вершину из списка
        nodes.erase(remove(nodes.begin(), nodes.end(), *n), nodes.end());

        // удалить список смежности вершины
        adjacencyList.erase(*n);

        // удалить все рёбра, ведущие к этой вершине
        for (auto& [node, edges] : adjacencyList) {
            edges.erase(remove_if(edges.begin(), edges.end(),
                [n](Edge& e) { return e.to == n; }), edges.end());
        }

        cout << "Вершина " << name << " удалена.\n";
    }

    // удаление ребра
    void deleteEdge(const string& fromName, const string& toName) {
        Node* fromNode = findNodePtr(fromName);
        Node* toNode = findNodePtr(toName);

        if (!fromNode || !toNode) {
            cout << "Одна из вершин не найдена.\n";
            return;
        }

        // удалить ребро from → to
        auto& edgesFrom = adjacencyList[*fromNode];
        edgesFrom.erase(remove_if(edgesFrom.begin(), edgesFrom.end(),
            [toNode](Edge& e) { return e.to == toNode; }), edgesFrom.end());

        // если граф неориентированный, удалить обратное ребро
        if (!isOrient) {
            auto& edgesTo = adjacencyList[*toNode];
            edgesTo.erase(remove_if(edgesTo.begin(), edgesTo.end(),
                [fromNode](Edge& e) { return e.to == fromNode; }), edgesTo.end());
        }

        cout << "Ребро " << fromName << " -> " << toName << " удалено.\n";
    }

    // вывод графа (список смежности)
    void printGraph() const {
        cout << "\n===== Граф =====\n";
        cout << "Ориентированный: " << (isOrient ? "да" : "нет") << "\n";
        cout << "Взвешенный: " << (isWeighted ? "да" : "нет") << "\n";
        cout << "Вершины: ";
        for (auto& n : nodes) cout << n.name << " ";
        cout << "\n\nСписок смежности:\n";
        for (const auto& [node, edges] : adjacencyList) {
            cout << node.name << ": ";
            for (const auto& e : edges) {
                cout << e.to->name;
                if (isWeighted) cout << "(" << e.weight << ")";
                cout << " ";
            }
            cout << "\n";
        }
        cout << "================\n";
    }

    // создание графа через консоль
    static Graph fromConsole() {
        char orientChoice, weightChoice;
        cout << "Ориентированный граф? (y/n): ";
        cin >> orientChoice;
        cout << "Взвешенный граф? (y/n): ";
        cin >> weightChoice;

        Graph g(orientChoice == 'y', weightChoice == 'y');

        int n;
        cout << "Введите количество вершин: ";
        cin >> n;

        for (int i = 0; i < n; i++) {
            string name;
            cout << "Имя вершины " << i + 1 << ": ";
            cin >> name;
            g.addNode({name});
        }

        int m;
        cout << "Введите количество рёбер: ";
        cin >> m;
        cout << "Формат ребра: from to [weight]\n";

        for (int i = 0; i < m; i++) {
            string from, to;
            float w = 0;
            cin >> from >> to;
            if (g.isWeighted) cin >> w;
            g.addEdge(g.findNodePtr(from), g.findNodePtr(to), w);
        }

        return g;
    }

    // создание графа из файла
    static Graph fromFile(const string& path) {
        ifstream file(path);
        if (!file.is_open()) throw runtime_error("Не удалось открыть файл");

        char orientChoice, weightChoice;
        file >> orientChoice >> weightChoice;

        Graph g(orientChoice == 'y', weightChoice == 'y');

        int n;
        file >> n;
        for (int i = 0; i < n; i++) {
            string name;
            file >> name;
            g.addNode({name});
        }

        int m;
        file >> m;
        for (int i = 0; i < m; i++) {
            string from, to;
            float w = 0;
            file >> from >> to;
            if (g.isWeighted) file >> w;
            g.addEdge(g.findNodePtr(from), g.findNodePtr(to), w);
        }

        return g;
    }
};



int main() {
    setlocale(LC_ALL, "ru");

    try {
        int createChoice;
        cout << "Создать граф:\n1 - из файла\n2 - из консоли\n";
        cin >> createChoice;

        Graph g = (createChoice == 1)
            ? Graph::fromFile("graph1.txt") // имя файла
            : Graph::fromConsole();

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

            if (action == 0) break;

            switch (action) {
                case 1: {
                    string name;
                    cout << "Имя вершины: ";
                    cin >> name;
                    g.addNode({name});
                    break;
                }
                case 2: {
                    string from, to;
                    float weight = 0;
                    cout << "Ребро (from to [weight]): ";
                    cin >> from >> to;
                    if (g.isWeighted) cin >> weight;
                    g.addEdge(g.findNodePtr(from), g.findNodePtr(to), weight);
                    break;
                }
                case 3: {
                    string name;
                    cout << "Имя вершины для удаления: ";
                    cin >> name;
                    g.deleteNode(name);
                    break;
                }
                case 4: {
                    string from, to;
                    cout << "Ребро для удаления (from to): ";
                    cin >> from >> to;
                    g.deleteEdge(from, to);
                    break;
                }
                case 5:
                    g.printGraph();
                    break;
                default:
                    cout << "Некорректный выбор\n";
            }
        }

    } catch (exception& e) {
        cerr << "Ошибка: " << e.what() << "\n";
    }

    return 0;
}
