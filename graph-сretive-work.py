import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog
import math
import random
import json
from collections import deque


class GraphVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("Визуализация алгоритмов графов")
        self.root.geometry("1620x960")

        # Структура графа
        self.nodes = {}  # {node_id: (x, y)}
        self.edges = []  # [(node1, node2, weight)]
        self.node_counter = 1
        self.selected_node = None
        self.edge_start = None

        # Цвета для визуализации
        self.colors = {
            'default': '#0366d6',     # gitHub-синий
            'visited': '#d73a49',     # красный акцент (ошибка)
            'path': '#28a745',        # зеленый акцент (успех)
            'start': '#f6f8fa',       # светлый фон для стартового выделения
            'end': '#6f42c1',         # фиолетовый, как в GitHub Pull request
            'mst': '#ffd33d',         # жёлтый акцент (внимание)
            'background': '#f6f8fa',  # очень светлый серый (фон)
            'text': '#24292e',        # тёмно-серый текст
            'edge': '#d1d5da'         # светлый серый для рёбер
        }

        # общий правый фрейм под канву и лог
        right_frame = tk.Frame(self.root)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=10, pady=10)

        # поле для рисования (слева)
        self.canvas = tk.Canvas(right_frame, bg=self.colors['background'],
                                width=900, height=700)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # информационное поле (справа от canvas)
        log_frame = tk.Frame(right_frame)
        log_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=(10, 0))

        self.info_text = tk.Text(log_frame, height=25, width=41, font=('Arial', 11))
        self.info_text.pack(fill=tk.BOTH, expand=True)

        # флаги паузы для анимаций
        self.is_paused = False
        self.is_running = False

        self.create_controls()
        self.bind_events()
        self.update_info()

    def create_controls(self):
        # основная панель управления
        control_frame = ttk.Frame(self.root)
        control_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

        # заголовок
        title_label = ttk.Label(control_frame, text="Графовый визуализатор",
                                font=('Arial', 14, 'bold'))
        title_label.pack(pady=10)

        # режимы работы
        mode_frame = ttk.LabelFrame(control_frame, text="Режимы работы", padding=10)
        mode_frame.pack(fill=tk.X, pady=5)

        self.mode_var = tk.StringVar(value="add_node")

        ttk.Radiobutton(mode_frame, text="Добавить вершину",
                        variable=self.mode_var, value="add_node").pack(anchor=tk.W)
        ttk.Radiobutton(mode_frame, text="Добавить ребро",
                        variable=self.mode_var, value="add_edge").pack(anchor=tk.W)
        ttk.Radiobutton(mode_frame, text="Перемещать",
                        variable=self.mode_var, value="move").pack(anchor=tk.W)
        ttk.Radiobutton(mode_frame, text="Удалить",
                        variable=self.mode_var, value="delete").pack(anchor=tk.W)

        # алгоритмы
        algo_frame = ttk.LabelFrame(control_frame, text="Алгоритмы", padding=10)
        algo_frame.pack(fill=tk.X, pady=5)

        ttk.Button(algo_frame, text="Обход в ширину (BFS)",
                   command=self.bfs_visualization).pack(fill=tk.X, pady=2)
        ttk.Button(algo_frame, text="Обход в глубину (DFS)",
                   command=self.dfs_visualization).pack(fill=tk.X, pady=2)
        ttk.Button(algo_frame, text="Поиск в глубину (рекурсия)",
                   command=self.dfs_recursive_visualization).pack(fill=tk.X, pady=2)
        ttk.Button(algo_frame, text="Алгоритм Прима (MST)",
                   command=self.prim_visualization).pack(fill=tk.X, pady=2)
        ttk.Button(algo_frame, text="Поиск компонент связности",
                   command=self.find_components).pack(fill=tk.X, pady=2)

        # управление графом
        graph_frame = ttk.LabelFrame(control_frame, text="Управление графом", padding=10)
        graph_frame.pack(fill=tk.X, pady=5)

        ttk.Button(graph_frame, text="Очистить граф",
                   command=self.clear_graph).pack(fill=tk.X, pady=2)
        ttk.Button(graph_frame, text="Случайный граф",
                   command=self.generate_random_graph).pack(fill=tk.X, pady=2)
        ttk.Button(graph_frame, text="Добавить вес ребрам",
                   command=self.add_weights).pack(fill=tk.X, pady=2)

        # кнопки управления анимацией
        anim_frame = ttk.LabelFrame(control_frame, text="Управление анимацией", padding=10)
        anim_frame.pack(fill=tk.X, pady=5)

        ttk.Button(anim_frame, text="Пауза",
                   command=self.pause_animation).pack(fill=tk.X, pady=2)
        ttk.Button(anim_frame, text="Возобновить",
                   command=self.resume_animation).pack(fill=tk.X, pady=2)

        # кнопки экспорта/импорта
        io_frame = ttk.LabelFrame(control_frame, text="Импорт/Экспорт", padding=10)
        io_frame.pack(fill=tk.X, pady=5)

        ttk.Button(io_frame, text="Сохранить граф (JSON)", 
                   command=self.export_graph).pack(fill=tk.X, pady=2)
        ttk.Button(io_frame, text="Загрузить граф (JSON)", 
                   command=self.import_graph).pack(fill=tk.X, pady=2)

        # статистика
        stats_frame = ttk.LabelFrame(control_frame, text="Статистика", padding=10)
        stats_frame.pack(fill=tk.X, pady=5)

        self.stats_label = ttk.Label(stats_frame, text="Вершин: 0, Рёбер: 0")
        self.stats_label.pack()

    def bind_events(self):
        self.canvas.bind("<Button-1>", self.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.on_canvas_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_canvas_release)

    def on_canvas_click(self, event):
        mode = self.mode_var.get()
        x, y = event.x, event.y

        if mode == "add_node":
            node_id = f"V{self.node_counter}"
            self.nodes[node_id] = (x, y)
            self.node_counter += 1
            self.draw_graph()

        elif mode == "add_edge":
            clicked_node = self.find_node_at(x, y)
            if clicked_node:
                if self.edge_start is None:
                    self.edge_start = clicked_node
                    self.highlight_node(clicked_node, self.colors['start'])
                else:
                    if self.edge_start != clicked_node:
                        weight = simpledialog.askinteger("Вес ребра", "Введите вес ребра:", initialvalue=1)
                        if weight is not None:
                            # Проверяем, нет ли уже такого ребра
                            edge_exists = False
                            for edge in self.edges:
                                if (edge[0] == self.edge_start and edge[1] == clicked_node) or \
                                        (edge[0] == clicked_node and edge[1] == self.edge_start):
                                    edge_exists = True
                                    break

                            if not edge_exists:
                                self.edges.append((self.edge_start, clicked_node, weight))
                    self.edge_start = None
                    self.draw_graph()

        elif mode == "move":
            clicked_node = self.find_node_at(x, y)
            if clicked_node:
                self.selected_node = clicked_node

        elif mode == "delete":
            clicked_node = self.find_node_at(x, y)
            if clicked_node:
                # Удаляем вершину и связанные рёбра
                del self.nodes[clicked_node]
                self.edges = [edge for edge in self.edges
                              if edge[0] != clicked_node and edge[1] != clicked_node]
                self.draw_graph()

        self.update_info()

    def on_canvas_drag(self, event):
        if self.mode_var.get() == "move" and self.selected_node:
            self.nodes[self.selected_node] = (event.x, event.y)
            self.draw_graph()

    def on_canvas_release(self, event):
        self.selected_node = None

    def find_node_at(self, x, y, radius=20):
        for node_id, (node_x, node_y) in self.nodes.items():
            distance = math.sqrt((node_x - x) ** 2 + (node_y - y) ** 2)
            if distance <= radius:
                return node_id
        return None

    def draw_graph(self, highlighted_nodes=None, highlighted_edges=None, node_colors=None):
        if highlighted_nodes is None:
            highlighted_nodes = set()
        if highlighted_edges is None:
            highlighted_edges = set()
        if node_colors is None:
            node_colors = {}

        self.canvas.delete("all")

        # рисуем рёбра
        for edge in self.edges:
            node1, node2, weight = edge
            x1, y1 = self.nodes[node1]
            x2, y2 = self.nodes[node2]

            is_highlighted = (node1, node2, weight) in highlighted_edges or (node2, node1, weight) in highlighted_edges
            edge_color = self.colors['mst'] if is_highlighted else self.colors['edge']
            line_width = 3 if is_highlighted else 2

            # рисуем линию
            self.canvas.create_line(x1, y1, x2, y2, width=line_width,
                                    fill=edge_color, arrow=tk.LAST)

            # Подпись веса
            mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
            self.canvas.create_text(mid_x, mid_y, text=str(weight),
                                    fill=self.colors['text'], font=('Arial', 10, 'bold'))

        # рисуем вершины
        for node_id, (x, y) in self.nodes.items():
            color = node_colors.get(node_id, self.colors['default'])
            if node_id in highlighted_nodes:
                color = self.colors['visited']

            # круг вершины
            self.canvas.create_oval(x - 15, y - 15, x + 15, y + 15, fill=color,
                                    outline='black', width=2)

            # подпись вершины
            self.canvas.create_text(x, y, text=node_id, fill='white',
                                    font=('Arial', 10, 'bold'))

    def highlight_node(self, node_id, color=None):
        if color is None:
            color = self.colors['visited']
        self.draw_graph(highlighted_nodes={node_id})
        self.canvas.update()

    def get_neighbors(self, node):
        neighbors = set()
        for edge in self.edges:
            if edge[0] == node:
                neighbors.add(edge[1])
            elif edge[1] == node:
                neighbors.add(edge[0])
        return list(neighbors)

    def get_edge_weight(self, node1, node2):
        for edge in self.edges:
            if (edge[0] == node1 and edge[1] == node2) or \
                    (edge[0] == node2 and edge[1] == node1):
                return edge[2]
        return 0

    def is_connected(self):
        if not self.nodes:
            return True

        visited = set()
        stack = [next(iter(self.nodes))]

        while stack:
            current = stack.pop()
            if current not in visited:
                visited.add(current)
                stack.extend(self.get_neighbors(current))

        return len(visited) == len(self.nodes)

    def ask_start_node(self):
        return self.ask_node("Выберите начальную вершину:")

    def ask_node(self, message):
        nodes = list(self.nodes.keys())
        if not nodes:
            return None

        selected = simpledialog.askstring("Выбор вершины",
                                          f"{message}\nДоступные вершины: {', '.join(nodes)}",
                                          initialvalue=nodes[0])
        return selected if selected in nodes else None

    def pause_animation(self):
        if self.is_running:
            self.is_paused = True
            self.add_info("Анимация поставлена на паузу")

    def resume_animation(self):
        if self.is_paused:
            self.is_paused = False
            self.add_info("Анимация возобновлена")

    def bfs_visualization(self):
        if not self.nodes:
            messagebox.showwarning("Предупреждение", "Граф пуст!")
            return

        start_node = self.ask_start_node()
        if not start_node:
            return

        visited = set()
        queue = deque([start_node])
        visited.add(start_node)

        self.is_running = True
        self.is_paused = False

        def bfs_step():
            if self.is_paused:
                self.root.after(200, bfs_step)
                return

            if queue:
                current = queue.popleft()
                self.draw_graph(highlighted_nodes=visited)
                self.add_info(f"Посещаем вершину: {current}")

                for neighbor in self.get_neighbors(current):
                    if neighbor not in visited:
                        visited.add(neighbor)
                        queue.append(neighbor)
                        self.add_info(f"Добавляем в очередь: {neighbor}")

                self.root.after(1500, bfs_step)  # Задержка 1.5 секунды
            else:
                self.is_running = False
                self.add_info("BFS завершен! Посещенные вершины: " + ", ".join(sorted(visited)))
                self.draw_graph()

        self.add_info(f"Начинаем BFS из вершины: {start_node}")
        bfs_step()

    def dfs_visualization(self):
        if not self.nodes:
            messagebox.showwarning("Предупреждение", "Граф пуст!")
            return

        start_node = self.ask_start_node()
        if not start_node:
            return

        visited = set()
        stack = [start_node]

        self.is_running = True
        self.is_paused = False

        def dfs_step():
            if self.is_paused:
                self.root.after(200, dfs_step)
                return

            if stack:
                current = stack.pop()

                if current not in visited:
                    visited.add(current)
                    self.draw_graph(highlighted_nodes=visited)
                    self.add_info(f"Посещаем вершину: {current}")

                    neighbors = self.get_neighbors(current)
                    for neighbor in reversed(neighbors):
                        if neighbor not in visited:
                            stack.append(neighbor)

                self.root.after(1500, dfs_step)
            else:
                self.is_running = False
                self.add_info("DFS завершен! Посещенные вершины: " + ", ".join(sorted(visited)))
                self.draw_graph()

        self.add_info(f"Начинаем DFS из вершины: {start_node}")
        dfs_step()

    def dfs_recursive_visualization(self):
        if not self.nodes:
            messagebox.showwarning("Предупреждение", "Граф пуст!")
            return

        start_node = self.ask_start_node()
        if not start_node:
            return

        visited = set()
        traversal_order = []

        self.is_running = True
        self.is_paused = False

        def dfs_recursive(current, parent=None):
            if self.is_paused:
                return True

            if current not in visited:
                visited.add(current)
                traversal_order.append(current)

                self.draw_graph(highlighted_nodes=visited)
                self.add_info(f"Рекурсивно посещаем: {current}")
                self.canvas.update()
                self.root.after(2000)

                neighbors = self.get_neighbors(current)
                for neighbor in neighbors:
                    if neighbor not in visited:
                        if not self.is_paused:
                            return dfs_recursive(neighbor, current)
                        else:
                            while self.is_paused:
                                self.root.after(100)
            return False

        def start_dfs():
            self.add_info(f"Начинаем рекурсивный DFS из вершины: {start_node}")
            has_more = dfs_recursive(start_node)
            if not has_more:
                self.is_running = False
                self.add_info("Рекурсивный DFS завершен! Порядок: " + " → ".join(traversal_order))
                self.draw_graph()

        start_dfs()

    def prim_visualization(self):
        if not self.nodes:
            messagebox.showwarning("Предупреждение", "Граф пуст!")
            return

        if not self.is_connected():
            messagebox.showerror("Ошибка", "Граф должен быть связным для построения MST!")
            return

        start_node = self.ask_start_node()
        if not start_node:
            return

        mst_edges = set()
        visited = {start_node}
        available_edges = []

        self.is_running = True
        self.is_paused = False

        def prim_step():
            if self.is_paused:
                self.root.after(200, prim_step)
                return

            # Добавляем все рёбра из посещённых вершин в доступные
            for node in visited:
                for neighbor in self.get_neighbors(node):
                    if neighbor not in visited:
                        weight = self.get_edge_weight(node, neighbor)
                        available_edges.append((weight, node, neighbor))

            # Удаляем дубликаты
            unique_edges = []
            seen = set()
            for edge in available_edges:
                key = (min(edge[1], edge[2]), max(edge[1], edge[2]))
                if key not in seen:
                    seen.add(key)
                    unique_edges.append(edge)
            available_edges = unique_edges

            if available_edges and len(visited) < len(self.nodes):
                available_edges.sort()
                min_weight, from_node, to_node = available_edges.pop(0)

                if to_node not in visited:
                    mst_edges.add((from_node, to_node, min_weight))
                    visited.add(to_node)

                    self.draw_graph(highlighted_nodes=visited, highlighted_edges=mst_edges)
                    self.add_info(f"Добавляем ребро: {from_node}-{to_node} (вес: {min_weight})")

                    self.root.after(2000, prim_step)
                else:
                    self.root.after(100, prim_step)
            else:
                self.is_running = False
                total_weight = sum(edge[2] for edge in mst_edges)
                self.add_info(f"Алгоритм Прима завершен! Общий вес MST: {total_weight}")
                self.draw_graph(highlighted_edges=mst_edges)

        self.add_info("Запускаем алгоритм Прима для построения MST...")
        prim_step()

    def get_node_colors(self, components, colors):
        node_colors = {}
        for i, component in enumerate(components):
            color = colors[i % len(colors)]
            for node in component:
                node_colors[node] = color
        return node_colors

    def find_components(self):
        if not self.nodes:
            messagebox.showwarning("Предупреждение", "Граф пуст!")
            return

        self.is_running = True
        self.is_paused = False

        visited = set()
        components = []
        colors = ['#e74c3c', '#3498db', '#2ecc71', '#f39c12', '#9b59b6']
        node_list = list(self.nodes.keys())

        def process_node(index):
            if not self.is_running:
                return
            if self.is_paused:
                self.root.after(200, lambda: process_node(index))
                return
            if index >= len(node_list):
                self.draw_graph(node_colors=self.get_node_colors(components, colors))
                self.add_info(f"Обнаружено {len(components)} компонент(а) связности.")
                self.is_running = False
                return

            node = node_list[index]
            if node not in visited:
                component = set()
                stack = [node]
                while stack:
                    current = stack.pop()
                    if current not in visited:
                        visited.add(current)
                        component.add(current)
                        stack.extend(self.get_neighbors(current))
                components.append(component)
                self.draw_graph(node_colors=self.get_node_colors(components, colors))
                self.add_info(f"Обнаружена компонента: {', '.join(sorted(component))}")

            self.root.after(1000, lambda: process_node(index + 1))

        process_node(0)

    def clear_graph(self):
        self.nodes.clear()
        self.edges.clear()
        self.node_counter = 1
        self.draw_graph()
        self.update_info()
        self.add_info("Граф очищен")

    def generate_random_graph(self):
        n_nodes = simpledialog.askinteger("Случайный граф", "Количество вершин:", initialvalue=6)
        if n_nodes:
            self.clear_graph()

            # создаём вершины в случайных позициях
            for i in range(n_nodes):
                node_id = f"V{i + 1}"
                x = random.randint(50, 700)
                y = random.randint(50, 550)
                self.nodes[node_id] = (x, y)

            # создаём рёбра для обеспечения связности
            nodes_list = list(self.nodes.keys())
            for i in range(len(nodes_list) - 1):
                weight = random.randint(1, 10)
                self.edges.append((nodes_list[i], nodes_list[i + 1], weight))

            # добавляем случайные рёбра
            for _ in range(n_nodes):
                node1 = random.choice(nodes_list)
                node2 = random.choice(nodes_list)
                if node1 != node2:
                    weight = random.randint(1, 10)
                    # проверка на, нет ли уже такого ребра
                    edge_exists = any(
                        (e[0] == node1 and e[1] == node2) or
                        (e[0] == node2 and e[1] == node1)
                        for e in self.edges
                    )
                    if not edge_exists:
                        self.edges.append((node1, node2, weight))

            self.draw_graph()
            self.update_info()
            self.add_info(f"Сгенерирован случайный граф с {n_nodes} вершинами")

    def add_weights(self):
        for i in range(len(self.edges)):
            node1, node2, old_weight = self.edges[i]
            new_weight = simpledialog.askinteger("Вес ребра",
                                                 f"Введите вес для ребра {node1}-{node2}:",
                                                 initialvalue=old_weight)
            if new_weight is not None:
                self.edges[i] = (node1, node2, new_weight)

        self.draw_graph()
        self.add_info("Веса рёбер обновлены")

    def export_graph(self):
        file_path = filedialog.asksaveasfilename(defaultextension=".json",
                                                 filetypes=[("JSON files", "*.json")],
                                                 title="Сохранить граф")
        if not file_path:
            return

        data = {
            'nodes': [{'id': node_id, 'x': x, 'y': y} for node_id, (x, y) in self.nodes.items()],
            'edges': [{'from': e[0], 'to': e[1], 'weight': e[2]} for e in self.edges]
        }

        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, ensure_ascii=False, indent=4)
            self.add_info(f"Граф сохранён в файл: {file_path}")
        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось сохранить граф: {e}")

    def import_graph(self):
        file_path = filedialog.askopenfilename(defaultextension=".json",
                                               filetypes=[("JSON files", "*.json")],
                                               title="Загрузить граф")
        if not file_path:
            return

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                data = json.load(f)

            self.nodes.clear()
            self.edges.clear()

            for node in data.get('nodes', []):
                self.nodes[node['id']] = (node['x'], node['y'])

            for edge in data.get('edges', []):
                self.edges.append((edge['from'], edge['to'], edge['weight']))

            # Находим максимальный номер вершины для продолжения нумерации
            max_num = 0
            for node_id in self.nodes.keys():
                if node_id.startswith('V'):
                    try:
                        num = int(node_id[1:])
                        if num > max_num:
                            max_num = num
                    except ValueError:
                        pass
            self.node_counter = max_num + 1

            self.draw_graph()
            self.update_info()
            self.add_info(f"Граф загружен из файла: {file_path}")

        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось загрузить граф: {e}")

    def update_info(self):
        self.stats_label.config(text=f"Вершин: {len(self.nodes)}, Рёбер: {len(self.edges)}")

    def add_info(self, message):
        self.info_text.insert(tk.END, f"\n{message}")
        self.info_text.see(tk.END)
        self.info_text.update()


if __name__ == "__main__":
    root = tk.Tk()
    app = GraphVisualizer(root)
    root.mainloop()