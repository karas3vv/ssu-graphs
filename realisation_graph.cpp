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

    void findCommonTarget(const string& u, const string& v) const;
    void printDegrees() const;

    Graph getReversed() const;

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
    if (findVertex(name) != -1) {
        cout << "Вершина \"" << name << "\" уже существует.\n";
        return;
    }
    adjList.push_back(Point(name));
    cout << "Вершина \"" << name << "\" успешно добавлена.\n";
}

// добавить ребро
void Graph::addEdge(const string& from, const string& to, int weight) {
    int i = findVertex(from);
    int j = findVertex(to);

    // проверяем существование вершин
    if (i == -1 && j == -1) {
        cout << "Вершины \"" << from << "\" и \"" << to << "\" не существуют. Ребро добавить невозможно.\n";
        return;
    } else if (i == -1) {
        cout << "Вершина \"" << from << "\" не существует. Ребро добавить невозможно.\n";
        return;
    } else if (j == -1) {
        cout << "Вершина \"" << to << "\" не существует. Ребро добавить невозможно.\n";
        return;
    }

    // проверяем, существует ли уже ребро
    auto& edges = adjList[i].adj;
    bool exists = any_of(edges.begin(), edges.end(), [&](const Edge& e) { return e.to == to; });

    if (exists) {
        cout << "Ребро \"" << from << " -> " << to << "\" уже существует. Добавление не выполнено.\n";
        return;
    }

    // добавляем ребро
    edges.push_back(Edge(to, weight));

    if (!directed && from != to) {
        adjList[j].adj.push_back(Edge(from, weight));
    }

    cout << "Ребро \"" << from << " -> " << to << "\" добавлено.\n";
}


// удалить вершину
void Graph::removePoint(const string& name) {
    int idx = findVertex(name);
    if (idx == -1) {
        cout << "Вершина \"" << name << "\" не существует.\n";
        return;
    }

    adjList.erase(adjList.begin() + idx);

    // удаляем все рёбра, ведущие к этой вершине
    for (auto& v : adjList) {
        v.adj.erase(remove_if(v.adj.begin(), v.adj.end(),
                              [&](Edge& e) { return e.to == name; }),
                    v.adj.end());
    }

    cout << "Вершина \"" << name << "\" удалена.\n";
}


// удалить ребро
void Graph::removeEdge(const string& from, const string& to) {
    int i = findVertex(from);
    int j = findVertex(to);

    // проверяем существование вершин
    if (i == -1 && j == -1) {
        cout << "Вершины \"" << from << "\" и \"" << to << "\" не существуют. Ребро удалить невозможно.\n";
        return;
    } else if (i == -1) {
        cout << "Вершина \"" << from << "\" не существует. Ребро удалить невозможно.\n";
        return;
    } else if (j == -1) {
        cout << "Вершина \"" << to << "\" не существует. Ребро удалить невозможно.\n";
        return;
    }

    auto& edgesFrom = adjList[i].adj;
    auto it = remove_if(edgesFrom.begin(), edgesFrom.end(), [&](Edge& e) { return e.to == to; });

    if (it == edgesFrom.end()) { // ребро не найдено
        cout << "Ребро \"" << from << " -> " << to << "\" не существует.\n";
    } else {
        edgesFrom.erase(it, edgesFrom.end());
        cout << "Ребро \"" << from << " -> " << to << "\" удалено.\n";
    }

    if (!directed) {
        auto& edgesTo = adjList[j].adj;
        edgesTo.erase(remove_if(edgesTo.begin(), edgesTo.end(), [&](Edge& e) { return e.to == from; }),
                      edgesTo.end());
    }
}

// найти общую вершину назначения для двух вершин-источников
void Graph::findCommonTarget(const string& u, const string& v) const {
    int idxU = findVertex(u);
    int idxV = findVertex(v);

    if (idxU == -1 && idxV == -1) {
        cout << "Вершины \"" << u << "\" и \"" << v << "\" не существуют.\n";
        return;
    } else if (idxU == -1) {
        cout << "Вершина \"" << u << "\" не существует.\n";
        return;
    } else if (idxV == -1) {
        cout << "Вершина \"" << v << "\" не существует.\n";
        return;
    }

    const auto& edgesU = adjList[idxU].adj;
    const auto& edgesV = adjList[idxV].adj;

    vector<string> common;
    for (const auto& e1 : edgesU) {
        for (const auto& e2 : edgesV) {
            if (e1.to == e2.to) {
                common.push_back(e1.to);
            }
        }
    }

    if (common.empty()) {
        cout << "Нет вершин, в которые идут дуги и из \"" << u << "\", и из \"" << v << "\".\n";
    } else {
        cout << "Вершины, в которые идут дуги из \"" << u << "\" и \"" << v << "\": ";
        for (const auto& name : common) {
            cout << name << " ";
        }
        cout << "\n";
    }
}

// сохранить граф в файл
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

// вывести степени вершин
void Graph::printDegrees() const {
    cout << "\nСтепени вершин:\n";

    for (const auto& v : adjList) {
        int outDeg = v.adj.size(); // исходящая степень
        int inDeg = 0;             // входящая степень

        // считаем входящие рёбра
        for (const auto& u : adjList) {
            for (const auto& e : u.adj) {
                if (e.to == v.adress) {
                    inDeg++;
                }
            }
        }

        if (directed) {
            cout << v.adress << ": входящая = " << inDeg 
                 << ", исходящая = " << outDeg << "\n";
        } else {
            // неориентированный граф: петля добавляет ещё 1
            int degree = outDeg;
            for (const auto& e : v.adj) {
                if (e.to == v.adress) degree++;
            }
            cout << v.adress << ": степень = " << degree << "\n";
        }
    }
}

Graph Graph::getReversed() const {
    if (!directed) {
        throw runtime_error("Операция обращённого графа применима только к ориентированным графам!");
    }

    Graph reversed(true); // создаём новый ориентированный граф

    // добавляем все вершины
    for (const auto& v : adjList) {
        reversed.addPoint(v.adress);
    }

    // добавляем рёбра в обратном направлении
    for (const auto& v : adjList) {
        for (const auto& e : v.adj) {
            reversed.addEdge(e.to, v.adress, e.weight);
        }
    }

    return reversed;
}


struct GraphRecord {
    string name;
    Graph* g;
};

int main() {
    vector<GraphRecord> graphs;
    Graph* current = nullptr;
    string currentName;
    int choice;

    do {
        cout << "\n=== Меню ===\n";
        cout << "1. Создать новый пустой граф\n";
        cout << "2. Загрузить граф из файла\n";
        cout << "3. Переключиться на другой граф\n";
        cout << "4. Добавить вершину\n";
        cout << "5. Добавить ребро\n";
        cout << "6. Показать список смежности текущего графа\n";
        cout << "7. Сохранить текущий граф в файл\n";
        cout << "8. Удалить вершину\n";
        cout << "9. Удалить ребро\n";
        cout << "10. Найти вершину, в которую ведут дуги из u и v\n";
        cout << "11. Вывести степени всех вершин\n";
        cout << "12. Построить обращённый орграф\n";
        cout << "0. Выход\n";
        cout << "Введите ваш выбор: ";
        cin >> choice;

        string name, from, to, fileName;
        bool directed;
        int weight;

        switch (choice) {
            case 1: {
                cout << "Введите имя нового графа: ";
                cin >> name;
                cout << "Ориентированный? (1 = да, 0 = нет): ";
                cin >> directed;
                Graph* g = new Graph(directed);
                graphs.push_back({name, g});
                current = g;
                currentName = name;
                cout << "Граф \"" << name << "\" создан и выбран как текущий.\n";
                break;
            }

            case 2: {
                cout << "Введите имя нового графа: ";
                cin >> name;
                cout << "Имя файла: ";
                cin >> fileName;
                cout << "Ориентированный? (1 = да, 0 = нет): ";
                cin >> directed;
                Graph* g = new Graph(fileName, directed);
                graphs.push_back({name, g});
                current = g;
                currentName = name;
                cout << "Граф \"" << name << "\" загружен из " << fileName << " и выбран как текущий.\n";
                break;
            }

            case 3: {  // переключение между графами
                if (graphs.empty()) {
                    cout << "Список графов пуст.\n";
                    break;
                }
                cout << "Доступные графы:\n";
                for (size_t i = 0; i < graphs.size(); i++) {
                    cout << i << ". " << graphs[i].name << (graphs[i].g == current ? " (текущий)" : "") << "\n";
                }
                int index;
                cout << "Введите номер графа для переключения: ";
                cin >> index;
                if (index >= 0 && index < (int)graphs.size()) {
                    current = graphs[index].g;
                    currentName = graphs[index].name;
                    cout << "Переключились на граф \"" << currentName << "\".\n";
                } else {
                    cout << "Неверный индекс.\n";
                }
                break;
            }

            case 4:
                if (!current) { cout << "Нет активного графа.\n"; break; }
                cout << "Введите имя вершины: ";
                cin >> from;
                current->addPoint(from);
                break;

            case 5:
                if (!current) { cout << "Нет активного графа.\n"; break; }
                cout << "Введите вершину-источник: ";
                cin >> from;
                cout << "Введите вершину-назначение: ";
                cin >> to;
                cout << "Введите вес ребра: ";
                cin >> weight;
                current->addEdge(from, to, weight);
                break;

            case 6:
                if (!current) { cout << "Нет активного графа.\n"; break; }
                cout << "Список смежности графа \"" << currentName << "\":\n";

                // сохранить в файл
                current->printAdjList("out_readable.txt");

                // вывести на экран
                for (const auto& v : current->adjList) {
                    cout << v.adress << ": ";
                    for (const auto& e : v.adj)
                        cout << "(" << e.to << "," << e.weight << ") ";
                    cout << "\n";
                }
                break;


            case 7:
                if (!current) { cout << "Нет активного графа.\n"; break; }
                current->saveToFile(currentName + "_export.txt");
                cout << "Граф \"" << currentName << "\" сохранён в файл " 
                     << currentName + "_export.txt" << "\n";
                break;

            case 8:  // удаление вершины
                if (!current) { cout << "Нет активного графа.\n"; break; }
                cout << "Введите вершину для удаления: ";
                cin >> from;
                current->removePoint(from);
                break;

            case 9:  // удаление ребра
                if (!current) { cout << "Нет активного графа.\n"; break; }
                cout << "Введите вершину-источник: ";
                cin >> from;
                cout << "Введите вершину-назначение: ";
                cin >> to;
                current->removeEdge(from, to);
                break;
            
            case 10: {
                if (!current) { cout << "Нет активного графа.\n"; break; }
                string u, v;
                cout << "Введите имя вершины u: ";
                cin >> u;
                cout << "Введите имя вершины v: ";
                cin >> v;
                current->findCommonTarget(u, v);
                break;
            }

            case 11:
                if (!current) { 
                    cout << "Нет активного графа.\n"; 
                    break; 
                }
                current->printDegrees();
                break;
            
            case 12: {
                if (!current) { 
                    cout << "Нет активного графа.\n"; 
                    break; 
                }
                try {
                    Graph reversed = current->getReversed();

                    // показываем пользователю
                    cout << "Обращённый граф создан. Его список смежности:\n";
                    for (const auto& v : reversed.adjList) {
                        cout << v.adress << ": ";
                        for (const auto& e : v.adj)
                            cout << "(" << e.to << "," << e.weight << ") ";
                        cout << "\n";
                    }

                    // при желании можно сохранить
                    reversed.saveToFile(currentName + "_reversed.txt");
                    cout << "Обращённый граф сохранён в файл: " 
                        << currentName + "_reversed.txt" << "\n";
                } 
                catch (const exception& e) {
                    cout << "Ошибка: " << e.what() << "\n";
                }
                break;
            }

            case 0:
                cout << "Выход...\n";
                break;

            default:
                cout << "Неверный выбор.\n";
        }

    } while (choice != 0);

    // очистка памяти
    for (auto& rec : graphs) {
        delete rec.g;
    }

    return 0;
}