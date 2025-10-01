#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;

struct Edge {
    string to;   // адрес вершины назначения
    int weight;  // вес ребра
    Edge(string t, int w = 1) : to(t), weight(w) {}
};

struct Point {
    string adress;          // имя вершины
    vector<Edge> adj;       // список смежных вершин (ребер)

    Point(string adr = "") : adress(adr) {}
};

class Graph {
private:
    bool directed;   
public:    
    vector<Point> adjList;

    // конструкторы
    Graph(bool dir = false) : directed(dir) {}               
    Graph(const string& filePath, bool dir = false);         
    Graph(const Graph& other);                               

    void addPoint(const string& name);
    void addEdge(const string& from, const string& to, int weight = 1);
    void removePoint(const string& name);
    void removeEdge(const string& from, const string& to);
    void printAdjList(const string& filePath) const;
    void saveToFile(const string& filePath) const;
    int findVertex(const string& name) const;
};

// реализация

Graph::Graph(const string& filePath, bool dir) : directed(dir) {
    ifstream fin(filePath);
    if (!fin.is_open()) throw runtime_error("Не удалось открыть файл");

    string from, to;
    int w;
    while (fin >> from >> to >> w) {
        addPoint(from);
        addPoint(to);
        addEdge(from, to, w);
    }
}

Graph::Graph(const Graph& other) : directed(other.directed), adjList(other.adjList) {}

int Graph::findVertex(const string& name) const {
    for (int i = 0; i < (int)adjList.size(); i++)
        if (adjList[i].adress == name) return i;
    return -1;
}

// добавить вершину
void Graph::addPoint(const string& name) {
    if (findVertex(name) == -1) adjList.push_back(Point(name));
}

// добавить ребро
void Graph::addEdge(const string& from, const string& to, int weight) {
    int i = findVertex(from);
    int j = findVertex(to);
    if (i == -1 || j == -1) throw runtime_error("Одна из вершин не найдена");

    adjList[i].adj.push_back(Edge(to, weight));

    if (!directed && from != to) {
        adjList[j].adj.push_back(Edge(from, weight));
    }
}

// удалить вершину
void Graph::removePoint(const string& name) {
    int idx = findVertex(name);
    if (idx == -1) return;

    adjList.erase(adjList.begin() + idx);
    // удаляем все рёбра, ведущие к этой вершине
    for (auto& v : adjList) {
        v.adj.erase(remove_if(v.adj.begin(), v.adj.end(),
                              [&](Edge& e) { return e.to == name; }),
                    v.adj.end());
    }
}

// удалить ребро
void Graph::removeEdge(const string& from, const string& to) {
    int i = findVertex(from);
    if (i == -1) return;
    adjList[i].adj.erase(remove_if(adjList[i].adj.begin(), adjList[i].adj.end(),
                                   [&](Edge& e) { return e.to == to; }),
                         adjList[i].adj.end());
    if (!directed) {
        int j = findVertex(to);
        if (j != -1) {
            adjList[j].adj.erase(remove_if(adjList[j].adj.begin(), adjList[j].adj.end(),
                                           [&](Edge& e) { return e.to == from; }),
                                 adjList[j].adj.end());
        }
    }
}

void Graph::saveToFile(const string& filePath) const {
    ofstream fout(filePath);
    if (!fout.is_open()) throw runtime_error("Не удалось открыть файл");

    for (const auto& v : adjList) {
        for (const auto& e : v.adj) {
            if (directed) {
                // для ориентированного графа сохраняем всё
                fout << v.adress << " " << e.to << " " << e.weight << "\n";
            } else {
                // для неориентированного графа:
                // записываем ребро, если from < to или это петля (from == to)
                if (v.adress < e.to || v.adress == e.to) {
                    fout << v.adress << " " << e.to << " " << e.weight << "\n";
                }
            }
        }
    }
}

// вывести список смежности в файл
void Graph::printAdjList(const string& filePath) const {
    ofstream fout(filePath);
    if (!fout.is_open()) throw runtime_error("Cannot open file.");
    for (const auto& v : adjList) {
        fout << v.adress << ": ";
        for (const auto& e : v.adj)
            fout << "(" << e.to << "," << e.weight << ") ";
        fout << "\n";
    }
}



int main() {
    int choice;
    Graph* g = nullptr;  // указатель на текущий граф

    do {
        cout << "\n=== Меню ===\n";
        cout << "1. Создать пустой граф\n";
        cout << "2. Загрузить граф из файла (через конструктор)\n";
        cout << "3. Добавить вершину\n";
        cout << "4. Добавить ребро\n";
        cout << "5. Удалить вершину\n";
        cout << "6. Удалить ребро\n";
        cout << "7. Показать список смежности\n";
        cout << "8. Сохранить граф в файл для конструктора\n";
        cout << "0. Выход\n";
        cout << "Введите ваш выбор: ";
        cin >> choice;

        string from, to, fileName;
        int weight;
        bool directed;

        switch (choice) {
            case 1:
                cout << "Создать ориентированный граф? (1 = да, 0 = нет): ";
                cin >> directed;
                delete g;  // удаляем старый граф, если был
                g = new Graph(directed);
                cout << "Пустой граф создан.\n";
                break;

            case 2: // Загрузка графа из файла через конструктор
                cout << "Введите имя файла: ";
                cin >> fileName;
                cout << "Ориентированный граф? (1 = да, 0 = нет): ";
                cin >> directed;
                delete g;  
                try {
                    g = new Graph(fileName, directed);
                    cout << "Граф успешно загружен из файла.\n";
                } catch (exception& e) {
                    cerr << "Ошибка: " << e.what() << endl;
                    g = nullptr;
                }
                break;

            case 3:
                if (!g) { cout << "Сначала создайте граф!\n"; break; }
                cout << "Введите имя вершины: ";
                cin >> from;
                g->addPoint(from);
                cout << "Вершина добавлена.\n";
                break;

            case 4:
                if (!g) { cout << "Сначала создайте граф!\n"; break; }
                cout << "Введите вершину-источник: ";
                cin >> from;
                cout << "Введите вершину-назначение: ";
                cin >> to;
                cout << "Введите вес ребра: ";
                cin >> weight;
                try {
                    g->addEdge(from, to, weight);
                    cout << "Ребро добавлено.\n";
                } catch (exception& e) {
                    cerr << "Ошибка: " << e.what() << endl;
                }
                break;

            case 5:
                if (!g) { cout << "Сначала создайте граф!\n"; break; }
                cout << "Введите вершину, которую нужно удалить: ";
                cin >> from;
                g->removePoint(from);
                cout << "Вершина удалена.\n";
                break;

            case 6:
                if (!g) { cout << "Сначала создайте граф!\n"; break; }
                cout << "Введите вершину-источник: ";
                cin >> from;
                cout << "Введите вершину-назначение: ";
                cin >> to;
                g->removeEdge(from, to);
                cout << "Ребро удалено.\n";
                break;

            case 7:
                if (!g) { cout << "Сначала создайте граф!\n"; break; }
                g->printAdjList("out_readable.txt");
                cout << "Список смежности сохранён в файл out_readable.txt и выведен на экран:\n";
                for (const auto& v : g->adjList) {
                    cout << v.adress << ": ";
                    for (const auto& e : v.adj)
                        cout << "(" << e.to << "," << e.weight << ") ";
                    cout << "\n";
                }
                break;

            case 8:
                if (!g) { cout << "Сначала создайте граф!\n"; break; }
                g->saveToFile("out_for_constructor.txt");
                cout << "Граф сохранён в формате для конструктора (out_for_constructor.txt).\n";
                break;

            case 0:
                cout << "Выход из программы.\n";
                break;

            default:
                cout << "Неверный выбор. Попробуйте снова.\n";
        }

    } while (choice != 0);

    delete g;

    return 0;
}
