= Обходы графа II
== Условие
Вывести все вершины, длины кратчайших (по числу дуг) путей от которых до всех остальных не превосходят k.

== Код (фрагменты кода)
```cpp
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
```

== Краткое описание алгоритма
Данный алгоритм находит вершины, из которых все другие вершины достижимы на расстоянии ≤ K.
=== Что делает
Для каждой вершины графа:

    + Запускает BFS для вычисления кратчайших расстояний до всех других вершин

    + Проверяет, что все вершины достижимы и расстояние до них не превышает K

    + Добавляет вершину в результат, если условие выполняется
== Примеры входных и выходных данных
=== Входные данные
```
A B 4
A C 2
B C 5
B D 10
C E 3
E D 4
D F 11
E F 5
F G 7
G H 1
H I 6
I J 2
C J 9
```

=== Выходные данные
```
Введите k: 3
Вершины, из которых все другие достижимы за ≤ 3 шагов: C E F J 
```