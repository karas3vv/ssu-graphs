import tkinter as tk
from tkinter import ttk
import math
import random
import heapq


class GraphVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("Уточка против Фермеров")
        self.root.geometry("1200x800")

        # Структура графа
        self.nodes = {}      # {node_id: (x, y)}
        self.edges = []      # [(node1, node2, weight)]

        # Игровые объекты
        self.duck_pos = None
        self.farmers_pos = []     # [(node_id, farmer_id)]
        self.lake_node = None
        self.game_moves = 0
        self.game_running = False
        self.game_paused = False
        self.max_moves = 10

        # Цвета
        self.colors = {
            'default': '#3498db',
            'path': '#2ecc71',
            'background': '#f8f9fa',
            'text': '#2c3e50'
        }
        self.game_colors = {
            'duck': '#FFD700',
            'farmer': '#8B4513',
            'lake': '#4169E1',
            'danger': '#FF4500',
            'swamp': '#94a3b8'   # болота по рёбрам
        }

        # ---------- LAYOUT ----------
        main_frame = tk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Левая панель
        control_frame = ttk.Frame(main_frame)
        control_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)
        self.control_frame = control_frame

        # Правая часть: сверху canvas, снизу лог
        right_frame = tk.Frame(main_frame)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.canvas = tk.Canvas(right_frame, bg=self.colors['background'])
        self.canvas.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        info_frame = tk.Frame(right_frame)
        info_frame.pack(side=tk.BOTTOM, fill=tk.X)

        self.info_text = tk.Text(
            info_frame,
            height=10,
            width=90,
            font=('Arial', 10),
            bg='#f5f5f5',
            relief=tk.SUNKEN,
            wrap=tk.WORD
        )
        self.info_text.pack(side=tk.LEFT, fill=tk.X, expand=True, pady=(5, 5))

        scrollbar = tk.Scrollbar(info_frame, orient=tk.VERTICAL, command=self.info_text.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.info_text.config(yscrollcommand=scrollbar.set)

        # ---------- Кнопки / статус ----------
        self.create_controls()
        self.bind_events()
        self.update_info()
        self.add_info("🦆 Нажми «Новая игра», чтобы начать.")

    def create_controls(self):
        control_frame = self.control_frame

        title_label = ttk.Label(
            control_frame,
            text="Уточка vs Фермеры",
            font=('Arial', 16, 'bold')
        )
        title_label.pack(pady=10)

        game_frame = ttk.LabelFrame(control_frame, text="🎮 Игра", padding=10)
        game_frame.pack(fill=tk.X, pady=5)

        ttk.Button(
            game_frame, text="🦆 Новая игра",
            command=self.new_game
        ).pack(fill=tk.X, pady=2)

        ttk.Button(
            game_frame, text="⏸ Пауза",
            command=self.toggle_pause
        ).pack(fill=tk.X, pady=2)

        ttk.Button(
            game_frame, text="🔄 Ход фермеров",
            command=self.next_turn
        ).pack(fill=tk.X, pady=2)

        self.game_status_label = ttk.Label(
            game_frame,
            text="Ходов: 0/10 | Статус: Готов",
            font=('Arial', 10, 'bold')
        )
        self.game_status_label.pack(pady=5)

        stats_frame = ttk.LabelFrame(control_frame, text="📊 Статистика", padding=10)
        stats_frame.pack(fill=tk.X, pady=5)
        self.stats_label = ttk.Label(stats_frame, text="Вершин: 0, Рёбер: 0")
        self.stats_label.pack()

    # ---------- ИГРА ----------

    def new_game(self):
        """Новая игра: случайный граф 10–10 вершин, усложнённые правила."""
        self.clear_game_state()

        n_nodes = 10
        self.clear_graph()

        # Вершины
        # Вершины (без наложений, с минимальным расстоянием)
        min_dist = 80  # минимальное расстояние между центрами вершин

        def is_far_enough(new_x, new_y):
            for (old_x, old_y) in self.nodes.values():
                if math.hypot(new_x - old_x, new_y - old_y) < min_dist:
                    return False
            return True

        for i in range(n_nodes):
            node_id = f"V{i + 1}"
            # подбираем позицию до тех пор, пока не найдём свободную
            for _ in range(100):  # защита от бесконечного цикла
                x = random.randint(80, 720)
                y = random.randint(80, 520)
                if is_far_enough(x, y):
                    self.nodes[node_id] = (x, y)
                    break
            else:
                # если за 100 попыток не нашли, просто ставим без проверки
                self.nodes[node_id] = (x, y)

        nodes_list = list(self.nodes.keys())

        # Утка в V1, даём ей минимум 3 соседа
        self.duck_pos = "V1"
        base_neighbors = ["V2", "V3"]
        if n_nodes >= 5:
            base_neighbors.append("V5")
        for nb in base_neighbors:
            if nb in self.nodes and not self.edge_exists("V1", nb):
                w = random.randint(1, 5)
                self.edges.append(("V1", nb, w))

        # Остальная связность (цепочка)
        for i in range(1, n_nodes - 1):
            n1 = nodes_list[i]
            n2 = nodes_list[(i + 1) % n_nodes]
            if not self.edge_exists(n1, n2):
                w = random.randint(1, 6)
                self.edges.append((n1, n2, w))

        # Дополнительные рёбра, часть — болота
        avoid = set(["V1"] + base_neighbors)
        extra_edges = n_nodes * 2
        for _ in range(extra_edges):
            node1 = random.choice(nodes_list)
            node2 = random.choice(nodes_list)
            if node1 == node2:
                continue
            if node1 in avoid and node2 in avoid:
                continue
            if not self.edge_exists(node1, node2):
                # 30% рёбер — болото (вес 10–18)
                if random.random() < 0.3:
                    w = random.randint(10, 18)
                else:
                    w = random.randint(2, 8)
                self.edges.append((node1, node2, w))

        # Озеро — последняя вершина
        self.lake_node = f"V{n_nodes}"

        # Два фермера далеко от утки
        far_nodes = [v for v in nodes_list if v not in avoid]
        if len(far_nodes) >= 2:
            f1 = random.choice(far_nodes)
            far_nodes.remove(f1)
            f2 = random.choice(far_nodes)
        else:
            f1, f2 = "V6" if n_nodes >= 6 else "V2", "V7" if n_nodes >= 7 else "V3"
        self.farmers_pos = [(f1, 1), (f2, 2)]

        self.game_moves = 0
        self.game_running = True
        self.game_paused = False

        self.draw_game_graph()
        self.update_game_status()
        self.add_info(f"🦆 Новая игра: вершин {n_nodes}. Озеро: {self.lake_node}. Утка: V1.")
        self.add_info(f"👨 Фермеры стартуют из {self.farmers_pos[0][0]} и {self.farmers_pos[1][0]}.")
        self.log_possible_moves()

    def clear_game_state(self):
        self.duck_pos = None
        self.farmers_pos = []
        self.lake_node = None
        self.game_moves = 0
        self.game_running = False
        self.game_paused = False

    def toggle_pause(self):
        if not self.game_running:
            return
        self.game_paused = not self.game_paused
        self.add_info("⏸ Пауза" if self.game_paused else "▶ Продолжение")
        self.update_game_status()

    def next_turn(self):
        if self.game_running and not self.game_paused:
            self.make_full_turn()

    def make_full_turn(self):
        if not self.game_running or self.game_paused:
            return

        if self.duck_pos == self.lake_node:
            self.end_game("win", f"🦆 Уточка добралась до озера {self.lake_node} за {self.game_moves} ходов!")
            return

        # Появление третьего фермера после 8 хода
        if self.game_moves >= 8 and len(self.farmers_pos) < 3:
            # Вставляем третьего фермера в случайную далёкую вершину
            candidates = [v for v in self.nodes.keys()
                          if v not in [p for p, _ in self.farmers_pos] + [self.duck_pos, self.lake_node]]
            if candidates:
                spawn = random.choice(candidates)
                self.farmers_pos.append((spawn, 3))
                self.add_info(f"⚠ Появился третий фермер в {spawn}!")

        self.move_farmers()

        # Проверка поимки
        for farmer_node, _ in self.farmers_pos:
            if farmer_node == self.duck_pos:
                self.end_game("lose", f"👨 Фермер поймал уточку на {farmer_node} за {self.game_moves} ходов!")
                return

        if self.game_moves >= self.max_moves:
            self.end_game("draw", f"😴 Уточка устала бегать... Ничья.")
            return

        self.update_game_status()
        self.draw_game_graph()
        self.log_possible_moves()

    def move_duck(self, new_pos):
        self.duck_pos = new_pos
        self.game_moves += 1
        self.add_info(f"🦆 Уточка переместилась в {new_pos}")

    def move_farmers(self):
        """Фермеры всегда делают один шаг по A* к уточке."""
        for i, (farmer_node, farmer_id) in enumerate(self.farmers_pos):
            current = farmer_node

            # Строим путь A* от текущего фермера к утке
            path = self.a_star(current, self.duck_pos)

            # Если путь есть и длина больше 1, делаем один шаг вперёд
            if path and len(path) > 1:
                current = path[1]

            # Обновляем позицию фермера
            self.farmers_pos[i] = (current, farmer_id)
            self.add_info(f"👨‍🌾 Фермер {farmer_id} двигается в {current}")


    def end_game(self, result, message):
        self.game_running = False
        self.add_info(message)
        if result == "win":
            self.game_status_label.config(text="🎉 ПОБЕДА! 🦆", foreground="green")
        elif result == "lose":
            self.game_status_label.config(text="💀 ПОРАЖЕНИЕ! 👨", foreground="red")
        else:
            self.game_status_label.config(text="😴 НИЧЬЯ", foreground="gray")

    def update_game_status(self):
        status = "Игра" if self.game_running and not self.game_paused else \
                 "Пауза" if self.game_paused else "Готов"
        color = "green" if self.game_running and not self.game_paused else "gray"
        self.game_status_label.config(
            text=f"Ходов: {self.game_moves}/{self.max_moves} | Статус: {status}",
            foreground=color
        )
        self.update_info()

    # ---------- ЛОГ ВОЗМОЖНЫХ ХОДОВ ----------

    def log_possible_moves(self):
        """Вывести в лог все доступные соседние ходы утки с пометками."""
        if not self.duck_pos:
            return
        neighbors = self.get_neighbors(self.duck_pos)
        if not neighbors:
            self.add_info("❌ Уточке больше некуда ходить.")
            return

        parts = []
        for nb in neighbors:
            tags = []

            # Опасность: фермер в этой вершине или соседней
            danger = any(
                nb == f_node or nb in self.get_neighbors(f_node)
                for f_node, _ in self.farmers_pos
            )
            if danger:
                tags.append("опасно")

            # Ближе ли к озеру по эвристике
            if self.lake_node:
                closer = self.heuristic(nb, self.lake_node) < self.heuristic(self.duck_pos, self.lake_node)
                if closer:
                    tags.append("к озеру ближе")

            # Болото: есть ребро с большим весом
            w = self.get_edge_weight(self.duck_pos, nb)
            if w >= 10:
                tags.append("болото")

            if not tags:
                tags.append("нейтрально")

            parts.append(f"{nb} ({', '.join(tags)})")

        self.add_info("🔍 Возможные ходы: " + "; ".join(parts))

    # ---------- СОБЫТИЯ / ОТРИСОВКА ----------

    def bind_events(self):
        self.canvas.bind("<Button-1>", self.on_canvas_click)

    def on_canvas_click(self, event):
        if not self.game_running or self.game_paused:
            return

        x, y = event.x, event.y
        clicked_node = self.find_node_at(x, y)
        if not clicked_node or clicked_node == self.duck_pos:
            return

        neighbors = self.get_neighbors(self.duck_pos)
        if clicked_node in neighbors:
            self.move_duck(clicked_node)
            self.root.after(600, self.make_full_turn)

    def draw_game_graph(self):
        node_colors = {}

        if self.lake_node:
            node_colors[self.lake_node] = self.game_colors['lake']

        for farmer_node, _ in self.farmers_pos:
            for nb in self.get_neighbors(farmer_node):
                if nb not in node_colors and nb not in (self.duck_pos, self.lake_node):
                    node_colors[nb] = self.game_colors['danger']

        self.draw_graph(node_colors=node_colors)
        self.draw_game_objects()

    def draw_graph(self, highlighted_nodes=None, highlighted_edges=None, node_colors=None):
        if highlighted_nodes is None:
            highlighted_nodes = set()
        if highlighted_edges is None:
            highlighted_edges = set()
        if node_colors is None:
            node_colors = {}

        self.canvas.delete("all")

        for node1, node2, weight in self.edges:
            x1, y1 = self.nodes[node1]
            x2, y2 = self.nodes[node2]

            # болота — толстые серо-синие линии
            if weight >= 10:
                edge_color = self.game_colors['swamp']
                width = 4
            else:
                edge_color = self.colors['path'] if (node1, node2, weight) in highlighted_edges else 'gray'
                width = 2

            self.canvas.create_line(x1, y1, x2, y2, width=width, fill=edge_color, arrow=tk.LAST)

            mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
            self.canvas.create_text(
                mid_x, mid_y, text=str(weight),
                fill=self.colors['text'], font=('Arial', 9, 'bold')
            )

        for node_id, (x, y) in self.nodes.items():
            color = node_colors.get(node_id, self.colors['default'])
            self.canvas.create_oval(
                x - 15, y - 15, x + 15, y + 15,
                fill=color, outline='black', width=2
            )
            self.canvas.create_text(
                x, y, text=node_id,
                fill='white', font=('Arial', 10, 'bold')
            )

    def draw_game_objects(self):
        if self.duck_pos:
            x, y = self.nodes[self.duck_pos]
            self.canvas.create_oval(
                x - 18, y - 18, x + 18, y + 18,
                fill=self.game_colors['duck'], outline='orange', width=3
            )
            self.canvas.create_text(x, y, text="🦆", font=('Arial', 14, 'bold'))

        for farmer_node, farmer_id in self.farmers_pos:
            x, y = self.nodes[farmer_node]
            self.canvas.create_oval(
                x - 16, y - 16, x + 16, y + 16,
                fill=self.game_colors['farmer'], outline='brown', width=2
            )
            self.canvas.create_text(
                x, y, text=f"👨‍🌾{farmer_id}",
                font=('Arial', 9, 'bold')
            )

        if self.lake_node:
            x, y = self.nodes[self.lake_node]
            self.canvas.create_text(x, y - 22, text="🌊", font=('Arial', 14, 'bold'))

    # ---------- АЛГОРИТМЫ / ВСПОМОГАТЕЛЬНОЕ ----------

    def get_neighbors(self, node):
        neighbors = set()
        for n1, n2, _ in self.edges:
            if n1 == node:
                neighbors.add(n2)
            elif n2 == node:
                neighbors.add(n1)
        return list(neighbors)

    def get_edge_weight(self, node1, node2):
        for n1, n2, w in self.edges:
            if (n1 == node1 and n2 == node2) or (n1 == node2 and n2 == node1):
                return w
        return 1

    def a_star(self, start, goal):
        if start == goal:
            return [start]

        open_set = [(0, start)]
        came_from = {}
        g_score = {node: float('inf') for node in self.nodes}
        g_score[start] = 0
        f_score = {node: float('inf') for node in self.nodes}
        f_score[start] = self.heuristic(start, goal)

        while open_set:
            current = heapq.heappop(open_set)[1]
            if current == goal:
                path = []
                while current in came_from:
                    path.append(current)
                    current = came_from[current]
                path.append(start)
                return path[::-1]

            for nb in self.get_neighbors(current):
                tentative_g = g_score[current] + self.get_edge_weight(current, nb)
                if tentative_g < g_score[nb]:
                    came_from[nb] = current
                    g_score[nb] = tentative_g
                    f_score[nb] = tentative_g + self.heuristic(nb, goal)
                    heapq.heappush(open_set, (f_score[nb], nb))

        return None

    def heuristic(self, node1, node2):
        x1, y1 = self.nodes[node1]
        x2, y2 = self.nodes[node2]
        return math.hypot(x1 - x2, y1 - y2)

    def edge_exists(self, n1, n2):
        for a, b, _ in self.edges:
            if (a == n1 and b == n2) or (a == n2 and b == n1):
                return True
        return False

    def find_node_at(self, x, y, radius=20):
        for node_id, (nx, ny) in self.nodes.items():
            if math.hypot(nx - x, ny - y) <= radius:
                return node_id
        return None

    def clear_graph(self):
        self.nodes.clear()
        self.edges.clear()
        self.canvas.delete("all")
        self.update_info()

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
