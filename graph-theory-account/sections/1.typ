= Минимальные требования для класса Граф
== Условие
Для решения всех задач курса необходимо создать класс (или иерархию классов - на усмотрение разработчика), содержащий:

1. Структуру для хранения списка смежности графа (не работать с графом через матрицы смежности, если в некоторых алгоритмах удобнее использовать список ребер - реализовать метод, создающий список рёбер на основе списка смежности);

2. Конструкторы (не менее 3-х):
  - конструктор по умолчанию, создающий пустой граф
  - конструктор, заполняющий данные графа из файла
  - конструктор-копию (аккуратно, не все сразу делают именно копию)
  - специфические конструкторы для удобства тестирования

3. Методы:

  - добавляющие вершину,
  - добавляющие ребро (дугу),
  - удаляющие вершину,
  - удаляющие ребро (дугу),
  - выводящие список смежности в файл (в том числе в пригодном для чтения конструктором формате).
  - Не выполняйте некорректные операции, сообщайте об ошибках.

4. Должны поддерживаться как ориентированные, так и неориентированные графы. Заранее предусмотрите возможность добавления меток и\или весов для дуг. Поддержка мультиграфа не требуется.

5. Добавьте минималистичный консольный интерфейс пользователя (не смешивая его с реализацией!), позволяющий добавлять и удалять вершины и рёбра (дуги) и просматривать текущий список смежности графа.

6. Сгенерируйте не менее 4 входных файлов с разными типами графов (балансируйте на комбинации ориентированность-взвешенность) для тестирования класса в этом и последующих заданиях. Графы должны содержать не менее 7-10 вершин, в том числе петли и изолированные вершины.



Замечание:

В зависимости от выбранного способа хранения графа могут появиться дополнительные трудности при удалении-добавлении, например, необходимость переименования вершин, если граф хранится списком $($например, vector C++, List C#$)$. Этого можно избежать, если хранить в списке пару (имя вершины, список смежных вершин), или хранить в другой структуре (например, Dictionary C#$,$ map в С++, при этом список смежности вершины может также храниться в виде словаря с ключами - смежными вершинами и значениями - весами соответствующих ребер). Идеально, если в качестве вершины реализуется обобщенный тип (generic), но достаточно использовать строковый тип или свой класс.

== код (фрагменты кода)

```cpp
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

struct GraphRecord {
    string name;
    Graph* g;
};
```

//== краткое описание алгоритма

//== примеры входных и выходных данных