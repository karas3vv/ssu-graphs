= Максимальный поток V
== Условие
Решить задачу на нахождение максимального потока любым алгоритмом. Подготовить примеры, демонстрирующие работу алгоритма в разных случаях.

== Код (фрагменты кода)
```
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
```

== Краткое описание алгоритма
Находит максимальный поток от истока (source) к стоку (sink) в ориентированном графе, где рёбра имеют пропускные способности.
=== Что делает
1. Инициализация

Находит вершины истока и стока

Создает матрицу пропускных способностей из весов рёбер

Инициализирует матрицу потоков нулями

2. Основной цикл

Пока существует увеличивающий путь:
3. Поиск пути (BFS)

Ищет путь от истока к стоку в остаточной сети

Остаточная пропускная способность: capacity[u][v] - flow[u][v]

Запоминает путь через массив parent

4. Вычисление минимального потока

Находит минимальную остаточную способность вдоль найденного пути

Это максимальный поток, который можно пропустить по этому пути

5. Обновление потоков

Увеличивает поток по прямым рёбрам пути

Уменьшает поток по обратным рёбрам (для возможности отмены)

6. Накопление результата

Добавляет найденный поток к общему максимальному потоку

== Примеры входных и выходных данных
=== Входные данные
```
S A 10
S C 10
A B 4
A C 2
C D 15
B T 10
D B 6
D T 10
```
=== Выходные данные
```
=== Edmond-Karp algorithm - Max Flow ===
Введите имя источника: S
Введите имя стока: T
Максимальный поток из S в T = 16
```