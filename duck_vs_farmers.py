import tkinter as tk
from tkinter import ttk, filedialog
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
        self.add_info("🦆 Нажми «Новая игра» или «Загрузить граф», чтобы начать.")

    def create_controls(self):
        control_frame = self.control_frame

        title_label = ttk.Label(
            control_frame,
            text="Уточка vs Фермеры",
            font=('Arial', 16, 'bold')
        )
        title_label.pack(pady=10)

        # Граф
        graph_frame = ttk.LabelFrame(control_frame, text="📂 Граф", padding=10)
        graph_frame.pack(fill=tk.X, pady=5)
        
        ttk.Button(
            graph_frame, 
            text="📁 Загрузить граф из файла", 
            command=self.load_graph_from_file
        ).pack(fill=tk.X, pady=2)

        ttk.Button(
            graph_frame, 
            text="🎲 Новая игра", 
            command=self.new_game
        ).pack(fill=tk.X, pady=2)

        # Игра
        game_frame = ttk.LabelFrame(control_frame, text="🎮 Игра", padding=10)
        game_frame.pack(fill=tk.X, pady=5)

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

        # Статистика
        stats_frame = ttk.LabelFrame(control_frame, text="📊 Статистика", padding=10)
        stats_frame.pack(fill=tk.X, pady=5)
        self.stats_label = ttk.Label(stats_frame, text="Вершин: 0, Рёбер: 0")
        self.stats_label.pack()

    # ---------- ЗАГРУЗКА ГРАФА ----------
    def load_graph_from_file(self):
        path = filedialog.askopenfilename(
            title="Выбери файл графа",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        if not path:
            return

        self.clear_game_state()
        self.clear_graph()

        nodes = {}
        edges = []
        mode = None
        
        try:
            with open(path, "r", encoding="utf-8") as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()
                    if not line or line.startswith("#"):
                        continue
                    
                    if line.lower() == "nodes:":
                        mode = "nodes"
                        continue
                    if line.lower() == "edges:":
                        mode = "edges"
                        continue

                    if mode == "nodes":
                        parts = line.split()
                        if len(parts) != 3:
                            self.add_info(f"❌ Ошибка в строке {line_num}: неверный формат узла")
                            return
                        nid, xs, ys = parts
                        nodes[nid] = (int(xs), int(ys))
                    elif mode == "edges":
                        parts = line.split()
                        if len(parts) != 3:
                            self.add_info(f"❌ Ошибка в строке {line_num}: неверный формат ребра")
                            return
                        n1, n2, ws = parts
                        edges.append((n1, n2, int(ws)))

            if not nodes or not edges:
                self.add_info("❌ Файл графа пустой или некорректный.")
                return

            self.nodes = nodes
            self.edges = edges
            
            # Утка в первой вершине, озеро в последней
            node_list = list(self.nodes.keys())
            self.duck_pos = node_list[0]
            self.lake_node = node_list[-1]

            # Фермеры в случайных вершинах (не утка и не озеро)
            candidates = [v for v in node_list if v not in (self.duck_pos, self.lake_node)]
            random.shuffle(candidates)
            if len(candidates) >= 2:
                self.farmers_pos = [(candidates[0], 1), (candidates[1], 2)]
            else:
                self.farmers_pos = [(self.duck_pos, 1), (self.lake_node, 2)]

            self.game_moves = 0
            self.game_running = True
            self.game_paused = False
            
            if self.has_safe_start():
                self.draw_game_graph()
                self.update_game_status()
                self.add_info(f"📂 Загружен граф: {len(nodes)} вершин, {len(edges)} рёбер.")
                self.add_info(f"🦆 Утка: {self.duck_pos}, 🌊 озеро: {self.lake_node}")
                self.log_possible_moves()
            else:
                self.add_info("⚠ Стартовая позиция слишком опасная. Попробуй другую карту.")
                self.clear_game_state()
                
        except Exception as e:
            self.add_info(f"❌ Ошибка при чтении файла: {e}")

    # ---------- ИГРА ----------
    def new_game(self):
        """Новая игра с ограниченными попытками регенерации."""
        self.clear_game_state()
        attempts = 0
        while attempts < 5:
            self.generate_valid_graph()
            if self.has_safe_start():
                break
            attempts += 1
            self.clear_game_state()
        
        if not self.has_safe_start():
            self.add_info("⚠ Не удалось создать безопасный уровень. Используй 'Загрузить граф'.")
            return
            
        self.game_moves = 0
        self.game_running = True
        self.game_paused = False
        
        self.draw_game_graph()
        self.update_game_status()
        self.add_info(f"🦆 Новая игра: {len(self.nodes)} вершин. Озеро: {self.lake_node}.")
        self.add_info(f"👨 Фермеры: {', '.join(f[0] for f in self.farmers_pos)}")
        self.log_possible_moves()

    def generate_valid_graph(self):
        """Генерация ДЕРЕВА с ограничениями."""
        self.clear_graph()
        n_nodes = 10
        min_dist = 80

        def is_far_enough(new_x, new_y):
            for (old_x, old_y) in self.nodes.values():
                if math.hypot(new_x - old_x, new_y - old_y) < min_dist:
                    return False
            return True

        # Вершины
        for i in range(n_nodes):
            node_id = f"V{i + 1}"
            attempts = 0
            while attempts < 100:
                x = random.randint(80, 720)
                y = random.randint(80, 520)
                if is_far_enough(x, y):
                    self.nodes[node_id] = (x, y)
                    break
                attempts += 1
            else:
                self.nodes[node_id] = (x, y)

        nodes_list = list(self.nodes.keys())

        # Утка V1 с 3 соседями
        self.duck_pos = "V1"
        base_neighbors = ["V2", "V3", "V5"]
        for nb in base_neighbors:
            if nb in self.nodes:
                w = random.randint(1, 5)
                self.edges.append(("V1", nb, w))

        # Главная цепочка V2→V4→V6→V8→V10 (озеро)
        main_chain = ["V2", "V4", "V6", "V8", "V10"]
        for i in range(len(main_chain)-1):
            if not self.edge_exists(main_chain[i], main_chain[i+1]):
                w = random.randint(3, 8)
                self.edges.append((main_chain[i], main_chain[i+1], w))

        # Параллельная ветка V3→V7→V9
        side_chain = ["V3", "V7", "V9"]
        for i in range(len(side_chain)-1):
            if not self.edge_exists(side_chain[i], side_chain[i+1]):
                w = random.randint(3, 8)
                self.edges.append((side_chain[i], side_chain[i+1], w))

        # Соединение веток (V5 как хаб)
        cross_edges = [("V5", "V4", 4), ("V5", "V7", 5), ("V9", "V10", 6)]
        for n1, n2, w in cross_edges:
            if n1 in self.nodes and n2 in self.nodes and not self.edge_exists(n1, n2):
                self.edges.append((n1, n2, w))

        # Озеро в конце
        self.lake_node = "V10"

        # Дополнительные рёбра с ограничениями
        max_edges = n_nodes * 2  # Максимум 20 рёбер
        extra_attempts = 25
        swamp_count = 0
        max_swamps = 4

        for _ in range(extra_attempts):
            if len(self.edges) >= max_edges:
                break
            node1 = random.choice(nodes_list)
            node2 = random.choice(nodes_list)
            if (node1 == node2 or self.edge_exists(node1, node2) or 
                self.degree(node1) >= 4 or self.degree(node2) >= 4):
                continue
                
            if swamp_count < max_swamps and random.random() < 0.3:
                w = random.randint(10, 15)
                swamp_count += 1
            else:
                w = random.randint(2, 8)
            self.edges.append((node1, node2, w))

        # Фермеры ДАЛЕКО от утки и озера
        avoid = set(["V1", "V10"] + base_neighbors)
        far_nodes = [v for v in nodes_list if v not in avoid]
        random.shuffle(far_nodes)
        self.farmers_pos = [(far_nodes[0], 1), (far_nodes[1], 2)]

    def degree(self, node):
        """Степень вершины."""
        return sum(1 for a, b, _ in self.edges if a == node or b == node)

    def has_safe_start(self):
        """Есть ли хотя бы один безопасный ход для утки?"""
        if not self.duck_pos:
            return False
        neighbors = self.get_neighbors(self.duck_pos)
        if not neighbors:
            return False
        
        for nb in neighbors:
            danger = any(
                nb == f_node or nb in self.get_neighbors(f_node)
                for f_node, _ in self.farmers_pos
            )
            if not danger:
                return True
        return False

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
        """Фермеры двигаются по A* к уточке."""
        for i, (farmer_node, farmer_id) in enumerate(self.farmers_pos):
            path = self.a_star(farmer_node, self.duck_pos)
            if path and len(path) > 1:
                new_pos = path[1]
            else:
                new_pos = farmer_node  # Стоим на месте если нет пути
                
            self.farmers_pos[i] = (new_pos, farmer_id)
            self.add_info(f"👨‍🌾 Фермер {farmer_id} → {new_pos}")

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

    # ---------- ЛОГ ----------
    def log_possible_moves(self):
        if not self.duck_pos:
            return
        neighbors = self.get_neighbors(self.duck_pos)
        if not neighbors:
            self.add_info("❌ Уточке больше некуда ходить.")
            return

        parts = []
        for nb in neighbors:
            tags = []

            # Опасность
            danger = any(
                nb == f_node or nb in self.get_neighbors(f_node)
                for f_node, _ in self.farmers_pos
            )
            if danger:
                tags.append("⚠ опасно")

            # Ближе к озеру
            if self.lake_node:
                closer = self.heuristic(nb, self.lake_node) < self.heuristic(self.duck_pos, self.lake_node)
                if closer:
                    tags.append("➡️ ближе")

            # Болото
            w = self.get_edge_weight(self.duck_pos, nb)
            if w >= 10:
                tags.append("🐸 болото")

            if not tags:
                tags.append("✅ безопасно")

            parts.append(f"{nb} ({', '.join(tags)})")

        self.add_info("🔍 Возможные ходы: " + "; ".join(parts))

    # ---------- СОБЫТИЯ ----------
    def bind_events(self):
        self.canvas.bind("<Button-1>", self.on_canvas_click)

    def on_canvas_click(self, event):
        if not self.game_running or self.game_paused:
            return

        clicked_node = self.find_node_at(event.x, event.y)
        if not clicked_node or clicked_node == self.duck_pos:
            return

        neighbors = self.get_neighbors(self.duck_pos)
        if clicked_node in neighbors:
            self.move_duck(clicked_node)
            self.root.after(600, self.make_full_turn)

    # ---------- ОТРИСОВКА ----------
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
        if highlighted_nodes is None: highlighted_nodes = set()
        if highlighted_edges is None: highlighted_edges = set()
        if node_colors is None: node_colors = {}

        self.canvas.delete("all")

        for node1, node2, weight in self.edges:
            x1, y1 = self.nodes[node1]
            x2, y2 = self.nodes[node2]

            if weight >= 10:
                edge_color = self.game_colors['swamp']
                width = 4
            else:
                edge_color = self.colors['path'] if (node1, node2, weight) in highlighted_edges else 'gray'
                width = 2

            self.canvas.create_line(x1, y1, x2, y2, width=width, fill=edge_color, arrow=tk.LAST)

            mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
            self.canvas.create_text(mid_x, mid_y, text=str(weight), fill=self.colors['text'], font=('Arial', 9, 'bold'))

        for node_id, (x, y) in self.nodes.items():
            color = node_colors.get(node_id, self.colors['default'])
            self.canvas.create_oval(x-15, y-15, x+15, y+15, fill=color, outline='black', width=2)
            self.canvas.create_text(x, y, text=node_id, fill='white', font=('Arial', 10, 'bold'))

    def draw_game_objects(self):
        if self.duck_pos:
            x, y = self.nodes[self.duck_pos]
            self.canvas.create_oval(x-18, y-18, x+18, y+18, fill=self.game_colors['duck'], outline='orange', width=3)
            self.canvas.create_text(x, y, text="🦆", font=('Arial', 14, 'bold'))

        for farmer_node, farmer_id in self.farmers_pos:
            x, y = self.nodes[farmer_node]
            self.canvas.create_oval(x-16, y-16, x+16, y+16, fill=self.game_colors['farmer'], outline='brown', width=2)
            self.canvas.create_text(x, y, text=f"👨‍🌾{farmer_id}", font=('Arial', 9, 'bold'))

        if self.lake_node:
            x, y = self.nodes[self.lake_node]
            self.canvas.create_text(x, y-22, text="🌊", font=('Arial', 14, 'bold'))

    # ---------- АЛГОРИТМЫ ----------
    def get_neighbors(self, node):
        neighbors = set()
        for n1, n2, _ in self.edges:
            if n1 == node: neighbors.add(n2)
            elif n2 == node: neighbors.add(n1)
        return list(neighbors)

    def get_edge_weight(self, node1, node2):
        for n1, n2, w in self.edges:
            if (n1 == node1 and n2 == node2) or (n1 == node2 and n2 == node1):
                return w
        return 1

    def a_star(self, start, goal):
        if start == goal: return [start]

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
