#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <stack>
#include <cmath>
#include <chrono>
#include <functional>

using namespace std;
using Board = uint64_t;
using StateDepth = pair<Board, int>;

const int N = 4;
const Board GOAL_BOARD = 1311768467463790320;
const int MAX_DEPTH = 80;
const int ITERATIONS = 1;

/// <summary>
/// Создает доску из массива плиток
/// </summary>
Board encode_board(const int tiles[16]) {
    Board code = 0;
    for (int i = 0; i < 16; ++i) {
        code <<= 4;          // освободить 4 бита
        code |= (tiles[i] & 0xF); // записать плитку
    }
    return code;
}

/// <summary>
/// Декодирование плитки i из 64-битного числа
/// </summary>
int get_tile(Board board, int i) {
    return (board >> ((15 - i) * 4)) & 0xF;
}

void print_board(Board code) {
    for (int i = 15; i >= 0; --i) {
        int tile = (code >> (i * 4)) & 0xF;
        cout << tile << " ";
        if (i % 4 == 0) cout << "\n";
    }
    cout << endl;
}

/// <summary>
/// Проверка, решаема ли доска
/// </summary>
bool is_solvable(Board  board) {
    int sum_inversions = 0;
    int e = -1;

    for (int i = 0; i < N * N; i++) {
        
        int tile_i = get_tile(board, i);
        if (tile_i == 0) {
            e = i / N + 1;
            continue;
        }

        int n_i = 0;
        for (int j = i + 1; j < N * N; j++) {
            int tile_j = get_tile(board, j);
            if (tile_j != 0 && tile_i > tile_j) {
                n_i++;
            }
        }

        sum_inversions += n_i;
    }
    return (sum_inversions + e) % 2 == 0;
}


int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    return c - 'A' + 10;
}

/// <summary>
/// Создание доски из строки в hex-формате
/// </summary>
Board create_board_from_string(const string& str) {
    int arr[16];
    for (int i = 0; i < 16; ++i) {
        arr[i] = hex_to_int(str[i]);
    }
    return encode_board(arr);
}

/// <summary>
/// Ищет индекс пустой ячейки
/// </summary>
int find_zero_index(Board board) {
    for (int i = 0; i < N * N; ++i) {
        if (get_tile(board, i) == 0) {
            return i;
        }
    }
    return -1;
}

/// <summary>
/// Обменивает значения плиток на позициях index1 и index2 в Board
/// </summary>
Board swap_tiles(Board board, int index1, int index2) {
    int tile1 = get_tile(board, index1);
    int tile2 = get_tile(board, index2);


    uint64_t mask1 = (uint64_t)0xF << ((15 - index1) * 4);
    uint64_t mask2 = (uint64_t)0xF << ((15 - index2) * 4);

    board &= ~(mask1 | mask2);

    board |= ((uint64_t)tile2 & 0xF) << ((15 - index1) * 4);
    board |= ((uint64_t)tile1 & 0xF) << ((15 - index2) * 4);

    return board;
}


/// <summary>
/// Возвращает все соседние доски, полученные путем перемещения пустой плитки
/// </summary>
vector<Board> get_neighbors(Board current_board) {
    vector<Board> neighbors;
    int zero_idx = find_zero_index(current_board);

    int r = zero_idx / N;
    int c = zero_idx % N; 

    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    for (int i = 0; i < 4; ++i) {
        int next_r = r + dr[i];
        int next_c = c + dc[i];

        // Проверяем, находится ли новая позиция в пределах доски 4x4
        if (next_r >= 0 && next_r < N && next_c >= 0 && next_c < N) {

            int next_idx = next_r * N + next_c;

            Board next_board = swap_tiles(current_board, zero_idx, next_idx);
            neighbors.push_back(next_board);
        }
    }

    return neighbors;
}


void print_path(Board start, Board goal, const unordered_map<Board, Board>& parent_map) {
    vector<Board> path;
    Board current = goal;

    while (current != start) {
        path.push_back(current);
        current = parent_map.at(current);
    }
    path.push_back(start);
    reverse(path.begin(), path.end());

    cout << "\nНайдено решение за " << path.size() - 1 << " ходов.\n";
    for (size_t i = 0; i < path.size(); ++i) {
        cout << "--- Ход " << i << " ---\n";
        print_board(path[i]);
    }
}


bool BFS(Board start_board, Board& solution_end_node, unordered_map<Board, Board>& parent_map) {
    parent_map.clear();

    if (!is_solvable(start_board)) {
        return false;
    }
    
    queue<Board> q;
    parent_map[start_board] = start_board;
	q.push(start_board);
    while (!q.empty()) {
		Board current = q.front();
		q.pop();

        if (current == GOAL_BOARD) {
            solution_end_node = current;
            return true;
        }
        
        for (Board next_board : get_neighbors(current)) {
            if (parent_map.find(next_board) == parent_map.end()) {
                parent_map[next_board] = current;
                q.push(next_board);
            }
        }
    }

    return false;
}

void DFS(Board start_board) {
    if (!is_solvable(start_board)) {
        cout << "Доска неразрешима. Невозможно найти решение." << endl;
        return;
    }

    unordered_map<Board, Board> parent_map;
    stack<StateDepth> s;

    s.push({ start_board, 0 });
    parent_map[start_board] = start_board;

    while (!s.empty()) {
        Board current_board = s.top().first;
        int current_depth = s.top().second;

        s.pop();
        if (current_board == GOAL_BOARD) {
            cout << "Решение найдено!" << endl;
            print_path(start_board, current_board, parent_map);
            return;
        }

        if (current_depth >= MAX_DEPTH) {
            continue;
        }

        for (Board next_board : get_neighbors(current_board)) {
            if (parent_map.find(next_board) == parent_map.end()) {
                parent_map[next_board] = current_board;
                s.push({ next_board, current_depth + 1 });
            }
        }
	}

    cout << "Решение не найдено." << endl;
}


bool DLS(Board start_board, int max_depth, Board& solution_end_node, unordered_map<Board, Board>& parent_map) {
    parent_map.clear();

    stack<StateDepth> s;

    s.push({ start_board, 0 });
    parent_map[start_board] = start_board;

    while (!s.empty()) {
        Board current_board = s.top().first;
        int current_depth = s.top().second;
        s.pop();

        if (current_board == GOAL_BOARD) {
            solution_end_node = current_board;
            return true;
        }

        if (current_depth >= max_depth) {
            continue;
        }

        for (Board next_board : get_neighbors(current_board)) {
            if (parent_map.find(next_board) == parent_map.end()) {
                parent_map[next_board] = current_board;
                s.push({ next_board, current_depth + 1 });
            }
        }
    }
    return false;
}

bool IDS(Board start_board, int absolute_max_depth, Board& solution_end_node, unordered_map<Board, Board>& parent_map) {
    if (!is_solvable(start_board)) {
        return false;
    }

    cout << "--- Запуск IDS до максимальной глубины: " << absolute_max_depth << " ---\n";

    parent_map.clear();

    for (int depth = 0; depth <= absolute_max_depth; ++depth) {
        cout << "Проверка глубины: " << depth << endl;

        if (DLS(start_board, depth, solution_end_node, parent_map)) {
            return true;
        }
    }

    return false;
}


/// <summary>
/// Манхэттенское расстояние
/// </summary>
int manhattan_distance(Board board) {
    int distance = 0;
    for (int i = 0; i < 16; ++i) {
        int tile = get_tile(board, i);
        if (tile == 0) continue;
        int target_row = (tile - 1) / N;
        int target_col = (tile - 1) % N;
        int current_row = i / N;
        int current_col = i % N;
        distance += abs(target_row - current_row) + abs(target_col - current_col);
    }
    return distance;
}

struct Node {
	Board board;
	int g; // стоимость от начального узла
	int h; // эвристическая стоимость до цели
	int f; // Общая стоимость

    Node(Board b, int g_cost, int h_cost)
        : board(b), g(g_cost), h(h_cost), f(g_cost + h_cost) {
    }

    bool operator>(const Node& other) const {
        if (f == other.f) {
            return h > other.h;
        }
        return f > other.f;
    }
};

bool A_star(Board start_board, Board& solution_end_node, unordered_map<Board, Board>& parent_map) {
    if (!is_solvable(start_board)) {
        cout << "Доска неразрешима. Невозможно найти решение." << endl;
        return false;
    }

    parent_map.clear();

    priority_queue<Node, vector<Node>, greater<Node>> open_list;
    unordered_map<Board, int> g_scores;

    int h_start = manhattan_distance(start_board);
    Node start_node(start_board, 0, h_start);

    open_list.push(start_node);
    g_scores[start_board] = 0;
    parent_map[start_board] = start_board;

    while (!open_list.empty()) {
        Node current_node = open_list.top();
        open_list.pop();
        Board current_board = current_node.board;

        if (current_board == GOAL_BOARD) {
            solution_end_node = current_board;
            return true;
        }

        if (current_node.g > g_scores.at(current_board)) {
            continue;
        }

        for (Board next_board : get_neighbors(current_board)) {
            int new_g = current_node.g + 1;

            if (g_scores.find(next_board) == g_scores.end() || new_g < g_scores.at(next_board)) {
                int h_next = manhattan_distance(next_board);
                Node next_node(next_board, new_g, h_next);

                g_scores[next_board] = new_g;
                parent_map[next_board] = current_board;

                open_list.push(next_node);
            }
        }
    }

    return false;
}

int next_threshold;

bool IDAS_DFSL(Board board, int g, int threshold, Board& solution_end_node, unordered_map<Board, Board>& parent_map) {

    int h = manhattan_distance(board);
    int f = g + h;

    if (f > threshold) {
        if (f < next_threshold) {
            next_threshold = f;
        }
        return false;
    }

    if (board == GOAL_BOARD) {
        solution_end_node = board;
        return true;
    }

    for (Board next_board : get_neighbors(board)) {
        if (parent_map.find(next_board) == parent_map.end()) {
            parent_map[next_board] = board;
            if (IDAS_DFSL(next_board, g + 1, threshold, solution_end_node, parent_map)) {
                return true;
            }
            parent_map.erase(next_board);
        }
    }

    return false;
}

bool IDA_star(Board start_board, int max_f_limit, Board& solution_end_node, unordered_map<Board, Board>& parent_map) {
    if (!is_solvable(start_board)) {
        return false;
    }

    int threshold = manhattan_distance(start_board);

    parent_map.clear();
    parent_map[start_board] = start_board;

    while (threshold <= max_f_limit) {
        next_threshold = MAX_DEPTH * 2;

        if (IDAS_DFSL(start_board, 0, threshold, solution_end_node, parent_map)) {
            return true;
        }

        if (next_threshold == MAX_DEPTH * 2) {
            return false;
        }

        threshold = next_threshold;
    }

    return false;
}


int main() {
    setlocale(LC_ALL, "ru");
    while (true) {
        string s;
        cout << "Введите начальную позицию: ";
        cin >> s;
        cout << endl;

        Board test_board = create_board_from_string(s);

        Board solution_end_node;
        unordered_map<Board, Board> solution_parent_map;

        auto start = chrono::high_resolution_clock::now();

        for (int iteration = 0; iteration < ITERATIONS; iteration++) {

            Board temp_end;
            unordered_map<Board, Board> temp_map;

            if (iteration == ITERATIONS - 1) {
                A_star(test_board, solution_end_node, solution_parent_map);
            }
            else {
                A_star(test_board, temp_end, temp_map);
            }
        }

        auto end = chrono::high_resolution_clock::now();

        auto total_duration_us = chrono::duration_cast<chrono::microseconds>(end - start);
        double total_duration_ms = (double)total_duration_us.count() / 1000.0;
        double time_avg_ms = total_duration_ms / ITERATIONS;



        if (solution_parent_map.empty() || solution_parent_map.find(solution_end_node) == solution_parent_map.end()) {
            if (solution_end_node == GOAL_BOARD) {
                cout << "Цель достигнута! Путь оптимален. Демонстрация пути:\n";
                print_path(test_board, solution_end_node, solution_parent_map);
            }
            else {
                cout << "Решение не найдено." << endl;
            }
        }
        else {
            cout << "Цель достигнута! Путь оптимален. Демонстрация пути:\n";
            print_path(test_board, solution_end_node, solution_parent_map);
        }

        cout << "\nСреднее время за итерацию: " << time_avg_ms << " мс.\n";
    }

    return 0;
}