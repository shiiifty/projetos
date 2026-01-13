# nuruomino.py: Template para implementação do projeto de Inteligência Artificial 2024/2025.
# Devem alterar as classes e funções neste ficheiro de acordo com as instruções do enunciado.
# Além das funções e classes sugeridas, podem acrescentar outras que considerem pertinentes.

from sys import stdin
from search import Problem
from search import Node
from search import astar_search

valid_shapes = {
    'L': [
        frozenset({(0, 0), (1, 0), (2, 0), (2, 1)}),
        frozenset({(0, 0), (0, 1), (0, 2), (1, 0)}),
        frozenset({(0, 1), (1, 1), (2, 1), (2, 0)}),
        frozenset({(1, 0), (1, 1), (1, 2), (0, 2)}),
        frozenset({(0, 0), (1, 0), (2, 0), (0, 1)}),
        frozenset({(0, 0), (0, 1), (0, 2), (1, 2)}),
        frozenset({(0, 0), (0, 1), (1, 1), (2, 1)}),
        frozenset({(0, 0), (1, 0), (1, 1), (1, 2)}),
    ],
    'I': [
        frozenset({(0, 0), (1, 0), (2, 0), (3, 0)}),
        frozenset({(0, 0), (0, 1), (0, 2), (0, 3)}),
    ],
    'T': [
        frozenset({(0, 0), (0, 1), (0, 2), (1, 1)}),
        frozenset({(0, 1), (1, 0), (1, 1), (2, 1)}),
        frozenset({(1, 0), (1, 1), (1, 2), (0, 1)}),
        frozenset({(0, 0), (1, 0), (2, 0), (1, 1)}),
    ]
}

class NuruominoState:
    state_id = 0

    def __init__(self, board):
        self.board = board
        self.id = NuruominoState.state_id
        NuruominoState.state_id += 1

    def __lt__(self, other):
        """ Este método é utilizado em caso de empate na gestão da lista
        de abertos nas procuras inshapedas. """
        return self.id < other.id
    
    def __eq__(self, other):
        if not isinstance(other, NuruominoState):
            return False
        return self.board.matrix == other.board.matrix

    def __hash__(self):
        return hash(tuple(tuple(row) for row in self.board.matrix))

class Board:
    """Representação interna de um board do Puzzle Nuruomino."""
    def __init__(self, matrix, size_list, copy, region_cells, subregions_cache=None):
        self.matrix = matrix
        self.size_list = size_list
        self.copy = copy
        self.region_cells = region_cells if region_cells is not None else []
        self.subregions_cache = subregions_cache if subregions_cache is not None else {}
    
    def calculate_adjacents(self, row: int, col: int, diagonals=True) -> list:
        directions = [(-1, -1), (-1, 0), (-1, 1),
                    (0, -1),           (0, 1),
                    (1, -1),  (1, 0),  (1, 1)] if diagonals else \
                    [(-1, 0), (0, -1), (0, 1), (1, 0)]

        adjacents = []
        for dx, dy in directions:
            r, c = row + dx, col + dy
            if 0 <= r < len(self.matrix) and 0 <= c < len(self.matrix[0]):
                adjacents.append((r, c))
        return adjacents


    def adjacent_regions(self, region:int) -> list:
        """Devolve uma lista das regiões que fazem fronteira com a região enviada no argumento."""
        adjacent_regions = []
        for row in range(len(self.matrix)-1):
            for column in range(len(self.matrix[row])-1):
                if self.matrix[row][column] == region:
                    adjacents = board.calculate_adjacents(row, column)
                    for r, c in adjacents:
                        if board.matrix[r][c] not in adjacent_regions and board.matrix[r][c] != region and isinstance(board.matrix[r][c], int):
                            adjacent_regions.append(board.matrix[r][c])

        return adjacent_regions
    
    def adjacent_positions(self, row:int, col:int) -> list:
        """Devolve as posições adjacentes à região, em todas as direções, incluindo diagonais."""
        region = self.matrix[row][col]
        adjacent_region_positions = []
        for row in range(len(self.matrix)-1):
            for column in range(len(self.matrix[row])-1):
                if self.matrix[row][column] == region:
                    adjacents = board.calculate_adjacents(row, column)
                    for r, c in adjacents:
                        if (r,c) not in adjacent_region_positions and board.matrix[r][c] != region:
                            adjacent_region_positions.append((r,c))

        return adjacent_region_positions

    def adjacent_values(self, row:int, col:int) -> list:
        """Devolve os values das cells adjacentes à região, em todas as direções, incluindo diagonais."""
        adjacent_region_positions = self.adjacent_positions(row, col)
        adjacent_region_values = []
        for r,c in adjacent_region_positions:
            if self.matrix[r][c] not in adjacent_region_values:
                adjacent_region_values.append(self.matrix[r][c])
        
        return adjacent_region_values
    
    def get_region_letter(self, region: str) -> list:
        region_list = []
        for row in range(len(self.matrix)):
            for column in range(len(self.matrix[row])):
                if self.matrix[row][column] == region:
                    region_list.append((row, column))
        
        return region_list
    
    def normalize_coords(self, coords) :
        base_x = min(x for x, y in coords)
        base_y = min(y for x, y in coords)
        return frozenset((x - base_x, y - base_y) for x, y in coords)
    
    def identify_form(self, region_list: list) -> str | None:
        coords_norm = self.normalize_coords(region_list)

        for letter, shapes in valid_shapes.items():
            if coords_norm in shapes:
                return letter
    
        return 'S'

    def is_L(self, region_list: list) -> bool:
        norm = self.normalize_coords(region_list)
        return self.identify_form(norm) == 'L'

    def is_I(self, region_list: list) -> bool:
        norm = self.normalize_coords(region_list)
        return self.identify_form(norm) == 'I'
    
    def is_T(self, region_list: list) -> bool:
        norm = self.normalize_coords(region_list)
        return self.identify_form(norm) == 'T'

    
    def is_square(self, region_list: list) -> bool:
        cells = set(region_list)
        for x, y in cells:
            if ((x+1, y) in cells and
                (x, y+1) in cells and
                (x+1, y+1) in cells):
                return True 

        return False 

    def is_S(self, region_list: list) -> bool:
        norm = self.normalize_coords(region_list)
        return self.identify_form(norm) == 'S' and not self.is_square(norm)
        
    def illegal_square(self, region_list: list):
        max_row = len(self.matrix)
        max_col = len(self.matrix[0])
        cases = ["L", "I", "T", "S"]

        for row, col in region_list:
            square_offsets = [
                [(0, 0), (0, 1), (1, 0), (1, 1)],
                [(0, 0), (0, -1), (1, 0), (1, -1)],
                [(0, 0), (0, 1), (-1, 0), (-1, 1)],
                [(0, 0), (0, -1), (-1, 0), (-1, -1)],
            ]
            
            for offsets in square_offsets:
                positions = [(row + dr, col + dc) for dr, dc in offsets]

                if all(0 <= r < max_row and 0 <= c < max_col for r, c in positions):
                    values = [self.matrix[r][c] for r, c in positions]
                    string_count = sum((val in cases) for val in values)
                    int_indices = [i for i, val in enumerate(values) if isinstance(val, int)]

                    if string_count == 3 and len(int_indices) == 1:
                        r, c = positions[int_indices[0]]
                        self.matrix[r][c] = "x"

    def update_size_list(self) -> list:
        for i in range(len(self.size_list)):
            self.size_list[i] = 0
        for row in self.matrix:
            for column in row:
                if isinstance(column, int):
                    self.size_list[column-1] += 1
                    
        return self.size_list
                
    def update_region_cells(self):
        max_region = max([cell for row in self.matrix for cell in row if isinstance(cell, int)], default=0)
        self.region_cells = [[] for _ in range(max_region)]
        self.size_list = [0 for _ in range(max_region)]

        for i in range(len(self.matrix)):
            for j in range(len(self.matrix[0])):
                val = self.matrix[i][j]
                if isinstance(val, int):
                    self.region_cells[val - 1].append((i, j))
                    self.size_list[val - 1] += 1

    def forms_square_fake(self, coords, letter):
        temp = self.clone()
        letters = {'L', 'T', 'I', 'S'}
        rows = len(temp.matrix)
        cols = len(temp.matrix[0])

        for x, y in coords:
            temp.matrix[x][y] = letter

        for x, y in coords:
            quadrants = [
                [(x, y), (x+1, y), (x, y+1), (x+1, y+1)],
                [(x-1, y), (x, y), (x-1, y+1), (x, y+1)],
                [(x, y-1), (x+1, y-1), (x, y), (x+1, y)],
                [(x-1, y-1), (x, y-1), (x-1, y), (x, y)],
            ]

            for quad in quadrants:
                if all(0 <= r < rows and 0 <= c < cols for r, c in quad):
                    values = [temp.matrix[r][c] for r, c in quad]
                    if sum(1 for v in values if v in letters) == 4:
                        return True

        return False
    
    def solve_size_4(self):
        valid = True
        shapes = {
            "L": self.is_L,
            "I": self.is_I,
            "T": self.is_T,
            "S": self.is_S,
        }

        while valid:
            valid = False

            for n, region_list in enumerate(self.region_cells):
                if self.size_list[n] != 4:
                    continue

                for letter, check_func in shapes.items():
                    if check_func(region_list) and not self.forms_square_fake(region_list, letter):
                        for x, y in region_list:
                            self.matrix[x][y] = letter

                        self.illegal_square(region_list)
                        valid = True
                        self.update_region_cells()
                        break 

    
    def find_subregions_size_four(self, region: int, board) -> list:
        cache_key = (region, tuple(sorted(board.region_cells[region - 1])))
        if cache_key in board.subregions_cache:
            return board.subregions_cache[cache_key]
                
        region_cells = set(board.region_cells[region - 1])
        result = {}

        def expand(group):
            if len(group) == 4:
                sorted_group = tuple(sorted(group))
                if sorted_group in result:
                    return

                if self.is_L(group):
                    result[sorted_group] = 'L'
                elif self.is_I(group):
                    result[sorted_group] = 'I'
                elif self.is_T(group):
                    result[sorted_group] = 'T'
                elif self.is_S(group):
                    result[sorted_group] = 'S'
                return
        
            for cell in group:
                x, y = cell
                for dx, dy in [(-1,0), (1,0), (0,-1), (0,1)]:
                    neighbor = (x + dx, y + dy)
                    if neighbor in region_cells and neighbor not in group:
                        expand(group + [neighbor])

        for cell in region_cells:
            expand([cell])

        subregions = [(shape, list(group)) for group, shape in result.items()]

        self.subregions_cache[cache_key] = subregions

        return subregions
    
    def connected_shaped_areas(self, board) -> bool:
        letters = {'L', 'I', 'T', 'S'}
        visited = set()

        pieces = [(r, c) for r in range(len(board.matrix))
                        for c in range(len(board.matrix[0]))
                        if board.matrix[r][c] in letters]

        if not pieces:
            return True 

        queue = [pieces[0]]
        while queue:
            r, c = queue.pop()
            if (r, c) in visited:
                continue
            visited.add((r, c))
            for dr, dc in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                nr, nc = r + dr, c + dc
                if (0 <= nr < len(board.matrix) and
                    0 <= nc < len(board.matrix[0]) and
                    board.matrix[nr][nc] in letters and
                    (nr, nc) not in visited):
                    queue.append((nr, nc))

        return len(visited) == len(pieces)

    
    def clone(self):
        return Board(
            [row[:] for row in self.matrix],
            self.size_list[:],
            [row[:] for row in self.copy],
            [cells[:] for cells in self.region_cells],
            self.subregions_cache.copy()
        )

    def print_board(board):
        prev_len = len(board[0]) if board else 0  

        for i, row in enumerate(board):
            current_len = len(row)

            if i != 0 and current_len != prev_len:
                print()
            
            print('\t'.join(str(cell) for cell in row))
            prev_len = current_len


    @staticmethod
    def parse_instance():
        """Lê o test do standard input (stdin) que é passado como argumento
        e retorna uma instância da classe Board.

        Por exemplo:
            $ python3 pipe.py < test-01.txt

            > from sys import stdin
            > line = stdin.readline().split()
        """
        copy = []
        matrix = []
        size_list = []
        max_region = 0

        for line in stdin:
            values = line.strip().split("\t")
            row = []
            for v in values:
                v_int = int(v)
                row.append(v_int)
                if v_int > max_region:
                    max_region = v_int
            matrix.append(row)
            copy.append(row[:])

        size_list = [0 for _ in range(max_region)]
        region_cells = [[] for _ in range(max_region)]

        for i, row in enumerate(matrix):
            for j, val in enumerate(row):
                size_list[val-1] += 1
                region_cells[val-1].append((i, j))

        return Board(matrix, size_list, copy, region_cells)

    # TODO: outros metodos da classe Board

class Nuruomino(Problem):
    def __init__(self, board: Board):
        """O construtor especifica o estado inicial."""
        self.board = board
        self.initial = NuruominoState(board)

    def forms_square(self, state: NuruominoState, coords: list, letter: str) -> bool:
        temp_board = state.board.clone()
        rows = len(temp_board.matrix)
        cols = len(temp_board.matrix[0])
        letters = {'L', 'T', 'I', 'S'}

        for x, y in coords:
            temp_board.matrix[x][y] = letter

        for x, y in coords:
            quadrants = [
                [(x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1)],  
                [(x - 1, y), (x, y), (x - 1, y + 1), (x, y + 1)],  
                [(x, y - 1), (x + 1, y - 1), (x, y), (x + 1, y)],  
                [(x - 1, y - 1), (x, y - 1), (x - 1, y), (x, y)]   
            ]

            for quad in quadrants:
                if all(0 <= r < rows and 0 <= c < cols for r, c in quad):
                    values = [temp_board.matrix[r][c] for r, c in quad]
                    if sum(1 for v in values if v in letters) == 4:
                        return True

        return False
    
    def actions(self, state: NuruominoState) -> list:
        region_sizes = [
            (i + 1, size)
            for i, size in enumerate(state.board.size_list)
            if size >= 4 and state.board.find_subregions_size_four(i + 1, state.board)
        ]

        if not region_sizes:
            return []

        sorted_regions = sorted(
        region_sizes,
        key=lambda x: (
                x[1], 
                len(state.board.find_subregions_size_four(x[0], state.board)) 
            )
        )

        min_region_id, _ = sorted_regions[0]

        actions = []
        sub_regions = state.board.find_subregions_size_four(min_region_id, state.board)

        for shape, sub_region in sub_regions:
            forbidden = set()
            adjacentes_cache = {}

            for row, col in sub_region:
                if (row, col) not in adjacentes_cache:
                    adjacentes_cache[(row, col)] = state.board.calculate_adjacents(row, col, False)

                for x, y in adjacentes_cache[(row, col)]:
                    val = state.board.matrix[x][y]
                    if val in {'L', 'I', 'T', 'S'}:
                        forbidden.add(val)

            if shape in forbidden:
                continue

            if self.forms_square(state, sub_region, shape):
                continue

            actions.append((min_region_id, shape, _, sub_region))

        return actions

    def result(self, state: NuruominoState, action) -> NuruominoState:
        """Retorna o estado resultante de executar a 'action' sobre
        'state' passado como argumento. A ação a executar deve ser uma
        das presentes na lista obtida pela execução de
        self.actions(state)."""
        region_id, letter, _, coords = action

        new_board = state.board.clone()
        new_board.update_region_cells()

        for row, col in coords:
            new_board.matrix[row][col] = letter
        
        new_board.illegal_square(coords)

        for row, col in new_board.region_cells[region_id - 1]:
            if (row, col) not in coords:
                new_board.matrix[row][col] = 'x'

        new_board.subregions_cache = {
            k: v for k, v in new_board.subregions_cache.items()
            if k[0] != region_id
        }

        new_board.update_region_cells()

        if any(size == 4 for size in new_board.size_list):
            new_board.solve_size_4()

        new_state = NuruominoState(new_board)

        return new_state
        
    def goal_test(self, state: NuruominoState) -> bool:
        """Retorna True se e só se o estado passado como argumento é
        um estado objetivo. Deve verificar se todas as posições do board
        estão preenchidas de acordo com as regras do problema."""

        if any(size != 0 for size in state.board.size_list):
            return False
        
        if not state.board.connected_shaped_areas(state.board):
            return False
                  
        return True

    def h(self, node: Node):
        """Função heuristica utilizada para a procura A*."""

        board = node.state.board
        h = 0
        letters = {'L', 'I', 'T', 'S'}

        for region_id, size in enumerate(board.size_list, start=1):
            if size < 4:
                continue
            h += (size + 3) // 4

            sobra = size % 4
            if sobra > 0:
                h += 0.25 * sobra

            for row, col in board.region_cells[region_id - 1]:
                for dx, dy in [(-1,0), (1,0), (0,-1), (0,1)]:
                    r, c = row + dx, col + dy
                    if 0 <= r < len(board.matrix) and 0 <= c < len(board.matrix[0]):
                        if board.matrix[r][c] in letters:
                            h += 0.4

            subregions = board.subregions_cache.get(region_id)

            if subregions is None:
                subregions = board.find_subregions_size_four(region_id, node.state.board)

            tamanho_sub = len(subregions)
            
            if tamanho_sub == 0:
                h += 100

            elif tamanho_sub > 6:
                h += (tamanho_sub - 6) * 0.3

            shape_count = {"L": 0, "I": 0, "T": 0, "S": 0}
            for shape, _ in subregions:
                shape_count[shape] += 1

            for shape, count in shape_count.items():
                if count > 3:
                    h += 0.2 * (count - 3)

            for row, col in board.region_cells[region_id - 1]:
                free = 0
                for dx, dy in [(-1,0), (1,0), (0,-1), (0,1)]:
                    r, c = row + dx, col + dy
                    if 0 <= r < len(board.matrix) and 0 <= c < len(board.matrix[0]):
                        if board.matrix[r][c] == region_id:
                            free += 1
                if free == 0:
                    h += 0.7

            for row, col in board.region_cells[region_id - 1]:
                for dx, dy in [(-1,0), (1,0), (0,-1), (0,1)]:
                    r, c = row + dx, col + dy
                    if 0 <= r < len(board.matrix) and 0 <= c < len(board.matrix[0]):
                        if board.matrix[r][c] in letters:
                            if board.matrix[r][c] == board.matrix[row][col]:
                                h += 0.6

        h += sum((s - 8) * 0.5 for s in board.size_list if s > 8)

        return h

if __name__ == "__main__":
    board = Board.parse_instance()
    board.solve_size_4()

    problem = Nuruomino(board)
    solution_node = astar_search(problem)

    if solution_node:
        for row in range(len(solution_node.state.board.matrix)):
            for col in range(len(solution_node.state.board.matrix[row])):
                if solution_node.state.board.matrix[row][col] == 'x':
                    solution_node.state.board.matrix[row][col] = board.copy[row][col]
        Board.print_board(solution_node.state.board.matrix)