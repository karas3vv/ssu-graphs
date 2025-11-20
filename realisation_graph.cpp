#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <queue>
#include <unordered_map>
#include <set>
#include <climits>

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

    void kruskalMST() const;

    void verticesAllDistances() const;
    void bellmanFord(const string& start);
    void floydPeriphery(const string& start, int N) const;

    int edmondsKarp(const string& sourceName, const string& sinkName) const;

    // вспомогательные: подсчёт числа вершин и рёбер 
    // (для неориентированного учитываем каждое неориентир. ребро 1 раз)
    int vertexCount() const {
        return (int)adjList.size();
    }

    int edgeCount() const {
        int cnt = 0;
        for (const auto& v : adjList) cnt += (int)v.adj.size();
        if (!directed) cnt /= 2; // в неориентированном случае рёбра хранятся дважды
        return cnt;
    }

    // DFS для подсчёта компонент (рассматриваем граф как неориентированный)
    void dfsUndir(int v, vector<char>& used) const {
        used[v] = 1;
        for (const auto& e : adjList[v].adj) {
            int to = findVertex(e.to);
            if (to != -1 && !used[to]) dfsUndir(to, used);
        }
    }

    // проверка на циклы в неориентированном графе (DFS с родителем)
    bool hasCycleUndirUtil(int v, int parent, vector<char>& used) const {
        used[v] = 1;
        for (const auto& e : adjList[v].adj) {
            int to = findVertex(e.to);
            if (to == -1) continue;
            if (!used[to]) {
                if (hasCycleUndirUtil(to, v, used)) return true;
            } else if (to != parent) {
                // нашли обратное посещённое ребро (и не родитель) -> цикл
                return true;
            }
        }
        return false;
    }

    bool hasCycleUndir() const {
        int n = vertexCount();
        vector<char> used(n, 0);
        for (int i = 0; i < n; ++i) {
            if (!used[i]) {
                if (hasCycleUndirUtil(i, -1, used)) return true;
            }
        }
        return false;
    }

    // проверка на циклы в ориентированном графе 
    // (DFS с раскраской: 0=white,1=gray,2=black)
    bool hasCycleDirUtil(int v, vector<int>& color) const {
        color[v] = 1; // gray
        for (const auto& e : adjList[v].adj) {
            int to = findVertex(e.to);
            if (to == -1) continue;
            if (color[to] == 0) {
                if (hasCycleDirUtil(to, color)) return true;
            } else if (color[to] == 1) {
                // нашли обратную (серую) вершину -> цикл
                return true;
            }
        }
        color[v] = 2; // black
        return false;
    }

    bool hasCycleDir() const {
        int n = vertexCount();
        vector<int> color(n, 0);
        for (int i = 0; i < n; ++i) {
            if (color[i] == 0) {
                if (hasCycleDirUtil(i, color)) return true;
            }
        }
        return false;
    }

    // подсчёт компонент (через неориентированный просмотр)
    int countComponents() const {
        int n = vertexCount();
        vector<char> used(n, 0);
        int comps = 0;
        for (int i = 0; i < n; ++i) {
            if (!used[i]) {
                ++comps;
                dfsUndir(i, used);
            }
        }
        return comps;
    }

    // подсчёт входных степеней (для ориентированного графа)
    vector<int> indegrees() const {
        int n = vertexCount();
        vector<int> indeg(n, 0);
        for (int i = 0; i < n; ++i) {
            for (const auto& e : adjList[i].adj) {
                int to = findVertex(e.to);
                if (to != -1) indeg[to]++;
            }
        }
        return indeg;
    }

    // проверка: неориентированный лес (acyclic)
    bool isForestUndirected() const {
        // лес = ацикличный неориентированный граф
        return !hasCycleUndir();
    }

    bool isTreeUndirected() const {
        if (directed) return false;
        int n = vertexCount();
        if (n == 0) return false; // пустой граф — трактуем как не-дерево (по задаче можно считать особым случаем)
        // дерево <=> связный и edges == n-1 (и ацикличный)
        int edges = edgeCount();
        if (edges != n - 1) return false;
        int comps = countComponents();
        return comps == 1 && !hasCycleUndir();
    }

    // ориентированная арборесценция (ориент. дерево с корнем)
    bool isArborescence() const {
        if (!directed) return false;
        int n = vertexCount();
        if (n == 0) return false;
        // 1) нет ориентированных циклов
        if (hasCycleDir()) return false;
        // 2) ровно один корень (indegree == 0), все остальные indeg == 1
        auto indeg = indegrees();
        int rootCount = 0;
        for (int d : indeg) {
            if (d == 0) ++rootCount;
            else if (d != 1) return false;
        }
        if (rootCount != 1) return false;
        // 3) корень должен быть способен достичь все вершины (проверим достижимость из найденного корня)
        int root = -1;
        for (int i = 0; i < n; ++i) if (indeg[i] == 0) { root = i; break; }
        // BFS/DFS по ориентированным рёбрам
        vector<char> used(n, 0);
        // простой стек DFS:
        vector<int> st; st.push_back(root); used[root] = 1;
        while (!st.empty()) {
            int v = st.back(); st.pop_back();
            for (const auto& e : adjList[v].adj) {
                int to = findVertex(e.to);
                if (to != -1 && !used[to]) {
                    used[to] = 1;
                    st.push_back(to);
                }
            }
        }
        for (int i = 0; i < n; ++i) if (!used[i]) return false;
        return true;
    }

    // ориентированный лес арборесценций: нет ориентированных циклов и indeg <= 1 for all vertices
    bool isDirectedForest() const {
        if (!directed) return false;
        if (hasCycleDir()) return false;
        auto indeg = indegrees();
        for (int d : indeg) if (d > 1) return false;
        return true;
    }

    // основная классификация: возвращает 
    // "Tree", "Forest", "DirectedArborescence", "DirectedForest" или "Other"
    string classify() const {
        if (!directed) {
            if (isTreeUndirected()) return "Tree";
            if (isForestUndirected()) return "Forest";
            return "Other";
        } else {
            if (isArborescence()) return "DirectedArborescence";
            if (isDirectedForest()) return "DirectedForest";
            return "Other";
        }
    }

    vector<string> verticesWithinK(int k) const {
    vector<string> result;
    int n = vertexCount();

    for (int i = 0; i < n; ++i) {
        vector<int> dist(n, -1); // -1 = не достигнута
        queue<int> q;
        dist[i] = 0;
        q.push(i);

        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (const auto& e : adjList[v].adj) {
                int to = findVertex(e.to);
                if (to != -1 && dist[to] == -1) {
                    dist[to] = dist[v] + 1;
                    q.push(to);
                }
            }
        }

        // проверяем, все расстояния ≤ k
        bool ok = true;
        for (int d : dist) {
            if (d == -1 || d > k) {
                ok = false;
                break;
            }
        }

        if (ok) result.push_back(adjList[i].adress);
    }

    return result;
    }
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

void Graph::kruskalMST() const {
    // Алгоритм Краскала работает только для неориентированных графов
    if (directed) {
        cout << "Kruskal: граф ориентированный — алгоритм применим только к неориентированным графам.\n";
        return;
    }

    // вспомогательная запись ребра: индексы вершин + вес
    struct ERec { int u, v, w; };

    int n = (int)adjList.size();
    if (n == 0) {
        cout << "Граф пустой.\n";
        return;
    }

    // 1) Собираем все рёбра (для неориентированного — только один раз: i < j)
    vector<ERec> edges;
    for (int i = 0; i < n; ++i) {
        for (const auto& e : adjList[i].adj) {
            int j = findVertex(e.to);
            if (j == -1) continue; // защита на случай неконсистентности
            if (i < j) { // добавляем только один экземпляр ребра для неориентированного графа
                edges.push_back({i, j, e.weight});
            }
        }
    }

    if (edges.empty()) {
        cout << "В графе нет рёбер.\n";
        return;
    }

    // 2) Сортируем рёбра по весу
    sort(edges.begin(), edges.end(), [](const ERec& a, const ERec& b) {
        return a.w < b.w;
    });

    // 3) DSU (Union-Find) по индексам 0..n-1
    struct DSU {
        vector<int> p, r;
        DSU(int n=0) { p.resize(n); r.assign(n,0); for (int i=0;i<n;++i) p[i]=i; }
        int find(int a) { return p[a]==a ? a : p[a]=find(p[a]); }
        bool unite(int a, int b) {
            a = find(a); b = find(b);
            if (a==b) return false;
            if (r[a] < r[b]) swap(a,b);
            p[b] = a;
            if (r[a]==r[b]) ++r[a];
            return true;
        }
    } dsu(n);

    // 4) Построим MST в новом графе mst
    Graph mst(false); // неориентированный
    for (const auto& pt : adjList) mst.addPoint(pt.adress); // добавим все вершины в MST

    int totalWeight = 0;
    vector<ERec> mstEdges;

    cout << "\n--- Алгоритм Краскала ---\n";
    for (const auto& er : edges) {
        if (dsu.find(er.u) != dsu.find(er.v)) {
            dsu.unite(er.u, er.v);
            mst.addEdge(adjList[er.u].adress, adjList[er.v].adress, er.w);
            mstEdges.push_back(er);
            totalWeight += er.w;
            cout << "Добавлено ребро: " << adjList[er.u].adress 
                 << " - " << adjList[er.v].adress 
                 << " (вес = " << er.w << ")\n";
        }
    }

    cout << "Суммарный вес минимального остова: " << totalWeight << "\n";

    // Сохраним результат в файл
    try {
        mst.saveToFile("mst_output.txt");
        cout << "MST сохранён в mst_output.txt\n";
    } catch (const exception& ex) {
        cout << "Не удалось сохранить MST в файл: " << ex.what() << "\n";
    }
}

void Graph::verticesAllDistances() const {
    if (adjList.empty()) {
        cout << "Граф пуст.\n";
        return;
    }

    string startName;
    cout << "Введите начальную вершину: ";
    cin >> startName;

    int s = findVertex(startName);
    if (s == -1) {
        cout << "Вершина \"" << startName << "\" не найдена.\n";
        return;
    }

    const int INF = INT_MAX / 4;
    int n = vertexCount();
    vector<int> dist(n, INF);
    vector<int> parent(n, -1); // опционально: чтобы восстановить пути
    dist[s] = 0;

    // min-куча: (dist, vertex_index)
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    pq.push({0, s});

    while (!pq.empty()) {
        auto [d, v] = pq.top(); pq.pop();
        if (d != dist[v]) continue; // устаревшая запись в куче

        // проходим все вершины-соседи v (используем adjList)
        for (const auto& e : adjList[v].adj) {
            int to = findVertex(e.to);
            if (to == -1) continue; // защита от неконсистентности
            int w = e.weight;
            if (dist[v] != INF && dist[v] + w < dist[to]) {
                dist[to] = dist[v] + w;
                parent[to] = v;
                pq.push({dist[to], to});
            }
        }
    }

    // вывод расстояний
    cout << "\nКратчайшие расстояния от вершины " << startName << ":\n";
    for (int i = 0; i < n; ++i) {
        cout << adjList[i].adress << " : ";
        if (dist[i] == INF) cout << "недостижима\n";
        else cout << dist[i] << "\n";
    }

    // если тебе нужно проверить условие "все расстояния <= N",
    // можно спросить N и проверить:
    char ask;
    cout << "\nПроверить, что все расстояния ≤ N? (y/n): ";
    cin >> ask;
    if (ask == 'y' || ask == 'Y') {
        int N;
        cout << "Введите N: ";
        cin >> N;
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            if (i == s) continue;
            if (dist[i] == INF || dist[i] > N) { ok = false; break; }
        }
        if (ok) cout << "Все расстояния от " << startName << " до остальных ≤ " << N << "\n";
        else cout << "Не все расстояния ≤ " << N << "\n";
    }
}

void Graph::bellmanFord(const string& start) {
    int n = vertexCount();
    int startIndex = findVertex(start);

    if (startIndex == -1) {
        cout << "Вершина " << start << " не найдена.\n";
        return;
    }

    const long long INF = LLONG_MAX / 4;
    vector<long long> dist(n, INF);
    dist[startIndex] = 0;

    // Алгоритм Беллмана–Форда
    for (int i = 0; i < n - 1; ++i) {
        bool updated = false;
        for (int u = 0; u < n; ++u) {
            if (dist[u] == INF) continue;
            for (const auto& e : adjList[u].adj) {
                int v = findVertex(e.to);
                if (v == -1) continue;
                long long w = e.weight;
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    updated = true;
                }
            }
        }
        if (!updated) break; // оптимизация
    }

    // Проверка на отрицательные циклы (опционально)
    for (int u = 0; u < n; ++u) {
        if (dist[u] == INF) continue;
        for (const auto& e : adjList[u].adj) {
            int v = findVertex(e.to);
            if (v == -1) continue;
            if (dist[u] + e.weight < dist[v]) {
                cout << "Граф содержит цикл отрицательного веса!\n";
                return;
            }
        }
    }

    // Вывод результатов
    cout << "Кратчайшие расстояния от вершины " << start << ":\n";
    for (int i = 0; i < n; ++i) {
        cout << adjList[i].adress << " : ";
        if (dist[i] == INF)
            cout << "недостижима\n";
        else
            cout << dist[i] << "\n";
    }
}

void Graph::floydPeriphery(const string& start, int N) const {
    int n = adjList.size();
    if (n == 0) {
        cout << "Граф пуст.\n";
        return;
    }

    // Сопоставляем вершины индексам
    unordered_map<string, int> idx;
    for (int i = 0; i < n; ++i)
        idx[adjList[i].adress] = i;

    if (!idx.count(start)) {
        cout << "Вершина " << start << " не найдена в графе.\n";
        return;
    }

    // Инициализация матрицы расстояний
    vector<vector<int>> dist(n, vector<int>(n, INT_MAX / 2));

    for (int i = 0; i < n; ++i)
        dist[i][i] = 0;

    for (int i = 0; i < n; ++i)
        for (const auto& e : adjList[i].adj)
            dist[i][idx[e.to]] = min(dist[i][idx[e.to]], e.weight);

    // Алгоритм Флойда–Уоршелла
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][j] > dist[i][k] + dist[k][j])
                    dist[i][j] = dist[i][k] + dist[k][j];

    // Определяем N-периферию
    int s = idx[start];
    vector<string> periphery;
    for (int i = 0; i < n; ++i)
        if (dist[s][i] > N && dist[s][i] < INT_MAX / 2)
            periphery.push_back(adjList[i].adress);

    // Вывод результата
    cout << "N-периферия вершины " << start << " (N = " << N << "): ";
    if (periphery.empty())
        cout << "пусто\n";
    else {
        for (const auto& v : periphery)
            cout << v << " ";
        cout << "\n";
    }
}

int Graph::edmondsKarp(const string& sourceName, const string& sinkName) const {
    int n = vertexCount();
    int s = findVertex(sourceName);
    int t = findVertex(sinkName);
    if (s == -1 || t == -1) {
        cout << "Ошибка: источник или сток не найдены.\n";
        return 0;
    }

    // Матрица пропускных способностей
    vector<vector<int>> capacity(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (const auto& e : adjList[i].adj) {
            int j = findVertex(e.to);
            if (j != -1)
                capacity[i][j] += e.weight; // если несколько рёбер — суммируем
        }
    }

    vector<vector<int>> flow(n, vector<int>(n, 0));
    int maxFlow = 0;

    while (true) {
        // BFS для поиска пути с остаточной ёмкостью
        vector<int> parent(n, -1);
        queue<int> q;
        q.push(s);
        parent[s] = s;

        while (!q.empty() && parent[t] == -1) {
            int u = q.front();
            q.pop();
            for (int v = 0; v < n; ++v) {
                if (parent[v] == -1 && capacity[u][v] - flow[u][v] > 0) {
                    parent[v] = u;
                    q.push(v);
                }
            }
        }

        // если пути нет — всё, закончили
        if (parent[t] == -1)
            break;

        // находим минимальную пропускную способность по пути
        int increment = INT_MAX;
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            increment = min(increment, capacity[u][v] - flow[u][v]);
        }

        // обновляем потоки вдоль пути
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            flow[u][v] += increment;
            flow[v][u] -= increment; // обратное ребро
        }

        maxFlow += increment;
    }

    cout << "Максимальный поток из " << sourceName << " в " << sinkName << " = " << maxFlow << "\n";
    return maxFlow;
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
        cout << "13. Классифицировать текущий граф\n";
        cout << "14. Найти вершины, до всех остальных достижимые за ≤ k шагов\n";
        cout << "15. Построить минимальный остров (Краскал)\n";
        cout << "16. Найти вершины, из которых все минимальные пути до остальных ≤ N (Дейкстра)\n";
        cout << "17. Найти кратчайшие пути из заданной вершины (Беллман–Форд)\n";
        cout << "18. Определить N-периферию для заданной вершины (Флойд–Уоршелл)\n";
        cout << "19. Найти максимальный поток (Эдмондс–Карп)\n";
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
                    cout << "Обращённый граф создан. Его список смежности:\n";
                    for (const auto& v : reversed.adjList) {
                        cout << v.adress << ": ";
                        for (const auto& e : v.adj)
                            cout << "(" << e.to << "," << e.weight << ") ";
                        cout << "\n";
                    }
                    reversed.saveToFile(currentName + "_reversed.txt");
                    cout << "Обращённый граф сохранён в файл: " 
                        << currentName + "_reversed.txt" << "\n";
                } 
                catch (const exception& e) {
                    cout << "Ошибка: " << e.what() << "\n";
                }
                break;
            }

            case 13:
                if (!current) {
                    cout << "Нет активного графа.\n";
                    break;
                }
                cout << "Тип графа: " << current->classify() << "\n";
                break;
            
            case 14: {
                if (!current) { 
                    cout << "Нет активного графа.\n"; 
                    break; 
                }
                int k;
                cout << "Введите k: ";
                cin >> k;
                auto vertices = current->verticesWithinK(k);
                cout << "Вершины, из которых все другие достижимы за ≤ " << k << " шагов: ";
                for (const auto& name : vertices) cout << name << " ";
                cout << "\n";
                break;
            }

            case 15:
                if (!current) { cout << "Нет активного графа.\n"; break; }
                current->kruskalMST();
                break;
            
            case 16:
                if (!current) { cout << "Нет активного графа.\n"; break; }
                current->verticesAllDistances();
                break;

            case 17: {
                if (!current) { cout << "Нет активного графа.\n"; break; }
                string start;
                cout << "Введите имя начальной вершины: ";
                cin >> start;
                current->bellmanFord(start);
                break;
            }

            case 18: {
                if (!current) {
                    cout << "Нет активного графа.\n";
                    break;
                }
                string start;
                int N;
                cout << "Введите вершину: ";
                cin >> start;
                cout << "Введите N: ";
                cin >> N;
                current->floydPeriphery(start, N);
                break;
            }

            case 19: {
                if (!current) { cout << "Нет активного графа.\n"; break; }
                string src, sink;
                cout << "Введите имя источника: ";
                cin >> src;
                cout << "Введите имя стока: ";
                cin >> sink;
                current->edmondsKarp(src, sink);
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