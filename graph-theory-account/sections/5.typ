= Обходы графа II
== Условие
Проверить, является ли граф деревом, или лесом, или не является ни тем, ни другим.

== Код (фрагменты кода)
```cpp
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
    }
}
```

== Краткое описание алгоритма
Данный код содержит набор алгоритмов для анализа графов и их классификации. Основные функции:
=== Что делает
1. Базовые операции

    + vertexCount() - количество вершин

    + edgeCount() - количество ребер (для неориентированных делит на 2)

2. Поиск циклов

    + hasCycleUndir() - проверяет циклы в неориентированном графе (DFS с родителем)

    + hasCycleDir() - проверяет циклы в ориентированном графе (DFS с раскраской)

3. Анализ связности

    + countComponents() - подсчет компонент связности (как неориентированный граф)

    + dfsUndir() - DFS для обхода компонент

4. Классификация графов
Для неориентированных графов:

    + isTreeUndirected() - проверяет, является ли деревом (связный + n-1 ребер + ацикличный)

    + isForestUndirected() - проверяет, является ли лесом (ацикличный)

Для ориентированных графов:

    isArborescence() - ориентированное дерево с корнем:

        + Нет циклов

        + Один корень (входящая степень = 0)

        + Все остальные: входящая степень = 1

        + Корень достижим до всех вершин

    isDirectedForest() - ориентированный лес:

        + Нет циклов

        + Входящая степень ≤ 1 для всех вершин

5. Основная классификация
classify() возвращает тип графа:

    + "Tree" - неориентированное дерево

    + "Forest" - неориентированный лес

    + "DirectedArborescence" - ориентированное дерево

    + "DirectedForest" - ориентированный лес

    + "Other" - другой тип


== Примеры входных и выходных данных
=== Входные данные
```
A B 1
A C 1
B D 1
B E 1
C F 1
```

=== Выходные данные
```
Тип графа: DirectedArborescence
```