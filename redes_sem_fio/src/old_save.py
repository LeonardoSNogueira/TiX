import pygame
import random

# --- Configuration ---
WINDOW_WIDTH = 1600
WINDOW_HEIGHT = 900
BOARD_SIZE = (900, 132)
SQUARE_SIZE = BOARD_SIZE[0] // 8
BACKGROUND_COLOR = (75, 72, 71)
PIECE_TYPES = ['N', 'R', 'K']
COLORS = ['w', 'b']
PIECE_START_ORDER = [
    'wK', 'wN', 'wR', None, None, 'bR', 'bN', 'bK'
]
TIME_CONTROL =  600

# --- Font loading ---
pygame.font.init()
FONT = pygame.font.Font('../assets/fonts/DelaGothicOne-Regular.ttf',30)
pygame.display.set_caption('conecTiX')

# --- Chess Clock Class ---
class ChessClock:
    def __init__(self, start_seconds):
        self.time_left = start_seconds
        self.running = False
        self.last_update = pygame.time.get_ticks()

    def start(self):
        self.running = True
        self.last_update = pygame.time.get_ticks()

    def stop(self):
        self.running = False

    def update(self):
        if self.running:
            now = pygame.time.get_ticks()
            elapsed = (now - self.last_update) / 1000  # seconds
            self.last_update = now
            self.time_left = max(0, self.time_left - elapsed)

    def get_time_str(self):
        minutes = int(self.time_left) // 60
        seconds = int(self.time_left) % 60
        return f"{minutes:01}:{seconds:02}"

# --- Asset Loading ---
def load_image(path, size=None, flip_x=False):
    image = pygame.image.load(path)
    if size:
        image = pygame.transform.scale(image, size)
    if flip_x:
        image = pygame.transform.flip(image, True, False)
    return image

def load_assets():
    board_img = load_image("../assets/images/board.png", BOARD_SIZE)
    logo_img = load_image("../assets/images/logo.png")
    icon_img = load_image("../assets/images/pawn.png")
    pygame.display.set_icon(icon_img)
    piece_imgs = {}
    for color in COLORS:
        for piece in PIECE_TYPES:
            key = color + piece
            path = f"../assets/images/pieces/{key}.png"
            flip_x = (key == 'wN')
            piece_imgs[key] = load_image(path, (SQUARE_SIZE, SQUARE_SIZE), flip_x=flip_x)
    return board_img, logo_img, piece_imgs

# --- Drawing Functions ---
def draw_strip_pieces(surface, piece_order, board_rect, piece_images, dragging=None, drag_offset=(0,0), mouse_pos=(0,0)):
    for i, piece_key in enumerate(piece_order):
        if piece_key:
            if dragging and dragging['index'] == i:
                x, y = mouse_pos[0] - drag_offset[0], mouse_pos[1] - drag_offset[1]
                surface.blit(piece_images[piece_key], (x, y))
            else:
                x = board_rect.left + i * SQUARE_SIZE
                y = board_rect.top + 10
                surface.blit(piece_images[piece_key], (x, y))

def get_piece_at_pos(pos, board_rect):
    mx, my = pos
    if not board_rect.collidepoint(mx, my):
        return None
    rel_x = mx - board_rect.left
    rel_y = my - board_rect.top
    if rel_y < 0 or rel_y > BOARD_SIZE[1]:
        return None
    col = rel_x // SQUARE_SIZE
    if col < 0 or col >= 8:
        return None
    return int(col)

# --- Game Logic Functions ---
def movement_rules(piece, origin, destination, piece_order):
    if piece[1] == 'K':
        return origin != destination and abs(destination - origin) == 1
    if piece[1] == 'R':
        if origin == destination:
            return False
        step = 1 if destination > origin else -1
        for i in range(origin + step, destination, step):
            if piece_order[i] is not None:
                return False
        return True
    if piece[1] == 'N':
        return origin != destination and abs(destination - origin) == 2
    return False

def find_king(color, piece_order):
    king = f"{color}K"
    for i, piece in enumerate(piece_order):
        if piece == king:
            return i
    return None

def is_king_attacked(king_pos, color, piece_order):
    enemy_color = 'b' if color == 'w' else 'w'
    for i, piece in enumerate(piece_order):
        if piece and piece[0] == enemy_color:
            original = piece_order[king_pos]
            piece_order[king_pos] = None
            can_attack = False
            if movement_rules(piece, i, king_pos, piece_order):
                if original is None or original[0] != enemy_color:
                    can_attack = True
            piece_order[king_pos] = original
            if can_attack:
                return True
    return False

def legal_move(piece, origin, destination, piece_order):
    if not movement_rules(piece, origin, destination, piece_order):
        return False
    if piece_order[destination] is not None and piece_order[destination][0] == piece[0]:
        return False
    captured = piece_order[destination]
    piece_order[destination] = piece
    piece_order[origin] = None
    color = piece[0]
    king_pos = find_king(color, piece_order)
    in_check = is_king_attacked(king_pos, color, piece_order)
    piece_order[origin] = piece
    piece_order[destination] = captured
    return not in_check

def has_legal_moves(color, piece_order):
    for origin, piece in enumerate(piece_order):
        if piece and piece[0] == color:
            for destination in range(8):
                if origin != destination and legal_move(piece, origin, destination, piece_order):
                    return True
    return False

def is_checkmate(color, piece_order):
    king_pos = find_king(color, piece_order)
    return is_king_attacked(king_pos, color, piece_order) and not has_legal_moves(color, piece_order)

def is_stalemate(color, piece_order):
    king_pos = find_king(color, piece_order)
    return not is_king_attacked(king_pos, color, piece_order) and not has_legal_moves(color, piece_order)

def is_insufficient_material(piece_order):
    pieces = [p for p in piece_order if p is not None]
    return len(pieces) == 2 and 'wK' in pieces and 'bK' in pieces

def board_state_tuple(piece_order):
    return tuple(piece_order)

def notation(piece_moved, destination, enemy_color, is_actual_capture, piece_order):
    piece_symbol = piece_moved[1]
    dest_str = str(destination + 1)
    checkmate = is_checkmate(enemy_color, piece_order)
    check = is_king_attacked(find_king(enemy_color, piece_order), enemy_color, piece_order)

    move_str = f"{piece_symbol}{'x' if is_actual_capture else ''}{dest_str}"

    if checkmate:
        move_str += "#"
    elif check:
        move_str += "+"

    return move_str

def save_moves(moves_list, result_string):
    input_name = "game"
    i = random.randint(0, 1000000)
    with open(f"../games/{input_name}_{i}.txt", "w") as file:
        for turn_num, turn_moves in enumerate(moves_list):
            pgn_line = f"{turn_num + 1}. "
            if len(turn_moves) > 0: # White's move
                pgn_line += turn_moves[0]
            if len(turn_moves) > 1: # Black's move
                pgn_line += f" {turn_moves[1]}"
            file.write(pgn_line + "\n")
        if result_string:
            file.write(f"Result: {result_string}\n")

def reset_board():
    return PIECE_START_ORDER.copy(), 0, [], None, {}

# --- Reusable Button Class ---
class Button:
    def __init__(self, rect, text, font, bg_color, text_color, border_color=None, border_radius=8, border_width=0):
        self.rect = pygame.Rect(rect)
        self.text = text
        self.font = font
        self.bg_color = bg_color
        self.text_color = text_color
        self.border_color = border_color
        self.border_radius = border_radius
        self.border_width = border_width

    def draw(self, surface):
        pygame.draw.rect(surface, self.bg_color, self.rect, border_radius=self.border_radius)
        if self.border_color and self.border_width > 0:
            pygame.draw.rect(surface, self.border_color, self.rect, border_radius=self.border_radius, width=self.border_width)
        text_surf = self.font.render(self.text, True, self.text_color)
        surface.blit(
            text_surf,
            (self.rect.centerx - text_surf.get_width() // 2, self.rect.centery - text_surf.get_height() // 2)
        )

    def is_clicked(self, mouse_pos):
        return self.rect.collidepoint(mouse_pos)

# --- Main Game Loop ---
def main():
    global TIME_CONTROL
    pygame.init()
    window = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
    # --- Board Loading ---
    board_img, logo_img, piece_images = load_assets()
    board = pygame.Surface(BOARD_SIZE, pygame.SRCALPHA)
    pygame.draw.rect(board, (255, 255, 255, 255), board.get_rect(), border_radius=10)
    board.blit(board_img, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)
    board_rect = board.get_rect()
    board_rect.center = (WINDOW_WIDTH // 2, WINDOW_HEIGHT // 2)
    # --- Logo loading ---
    logo_rect = logo_img.get_rect()
    logo_rect.center = (WINDOW_WIDTH // 2, WINDOW_HEIGHT // 5)

    # --- Chess Clocks ---
    white_clock = ChessClock(TIME_CONTROL)  # 10 minutes
    black_clock = ChessClock(TIME_CONTROL)
    active_color = 'w'

    move_start_time = pygame.time.get_ticks()

    piece_order, turn, moves, dragging, board_states = reset_board()
    running = True

    # --- Button setup ---
    button_width, button_height = 180, 40
    button_x = (WINDOW_WIDTH - button_width) // 2
    button_y = board_rect.bottom + 20 + 50 + 20  # below clocks
    start_stop_button = Button(
        rect=(button_x, button_y, button_width, button_height),
        text="Start",
        font=FONT,
        bg_color=(120, 120, 120),
        text_color=(30, 30, 30),
        border_color=(80, 80, 80),
        border_width=2
    )
    reset_button = Button(
        rect=((WINDOW_WIDTH) // 2 - 120, button_y + 2 * (button_height + 15), button_width+50, button_height+50),
        text="Reset",
        font=FONT,
        bg_color= 	(44,43,41),
        text_color=(149,171,129),
        border_color=None,
        border_width=2
    )
    new_game_button = Button(
        rect=((WINDOW_WIDTH) // 2 - 360, button_y + 2 * (button_height + 15), button_width+50, button_height+50),
        text="New Game",
        font=FONT,
        bg_color= 	(44,43,41),
        text_color=(149,171,129),
        border_color=None,
        border_width=2
    )
    load_button = Button(
        rect=((WINDOW_WIDTH) // 2 + 120, button_y + 2 * (button_height + 15), button_width+50, button_height+50),
        text="Load Game",
        font=FONT,
        bg_color= 	(44,43,41),
        text_color=(149,171,129),
        border_color=None,
        border_width=2
    )
    while running:
        # --- Update clocks ---
        if active_color == 'w':
            white_clock.update()
        else:
            black_clock.update()

        # --- Check for timeout ---
        if white_clock.time_left <= 0:
            print("Black wins on time!")
            save_moves(moves, "0-1")
            running = False
            move_start_time = pygame.time.get_ticks()
        if black_clock.time_left <= 0:
            print("White wins on time!")
            save_moves(moves, "1-0")
            running = False
            move_start_time = pygame.time.get_ticks()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                mouse_pos = pygame.mouse.get_pos()
                if start_stop_button.is_clicked(mouse_pos):
                    if white_clock.running or black_clock.running:
                        white_clock.stop()
                        black_clock.stop()
                    else:
                        if active_color == 'w':
                            white_clock.start()
                        else:
                            black_clock.start()
                elif reset_button.is_clicked(mouse_pos):
                    piece_order, turn, moves, dragging, board_states = reset_board()
                    white_clock = ChessClock(TIME_CONTROL)
                    black_clock = ChessClock(TIME_CONTROL)
                    active_color = 'w'
                
                    move_start_time = pygame.time.get_ticks()

                
                else:
                    idx = get_piece_at_pos(mouse_pos, board_rect)
                    if idx is not None and piece_order[idx] is not None:
                        piece_x = board_rect.left + idx * SQUARE_SIZE
                        piece_y = board_rect.top + 10
                        offset_x = mouse_pos[0] - piece_x
                        offset_y = mouse_pos[1] - piece_y
                        dragging = {'index': idx, 'offset': (offset_x, offset_y)}
            elif event.type == pygame.MOUSEBUTTONUP and event.button == 1:
                if dragging is not None:
                    mouse_pos = pygame.mouse.get_pos()
                    idx_to = get_piece_at_pos(mouse_pos, board_rect)
                    idx_from = dragging['index']
                    dragging = None
                    if idx_to is not None and idx_to != idx_from:
                        current_player_color = 'w' if turn % 2 == 0 else 'b'
                        moving_piece = piece_order[idx_from]
                        if moving_piece and moving_piece[0] == current_player_color:
                            if legal_move(moving_piece, idx_from, idx_to, piece_order):
                                was_capture = piece_order[idx_to] is not None and piece_order[idx_to][0] != moving_piece[0]
                                if piece_order[idx_to] is not None and piece_order[idx_to][0] == moving_piece[0]:
                                    continue
                                piece_order[idx_to] = moving_piece
                                piece_order[idx_from] = None
                                enemy_color = 'b' if current_player_color == 'w' else 'w'
                                move_notation_str = notation(moving_piece, idx_to, enemy_color, was_capture, piece_order)
                                # --- Append time spent on move ---
                                move_end_time = pygame.time.get_ticks()
                                time_spent = (move_end_time - move_start_time) / 1000
                                move_notation_str += f' ({time_spent:.1f}s)'

                                # --- PGN-like notation appending ---
                                if turn % 2 == 0:  # White's move
                                    moves.append([move_notation_str])
                                else:  # Black's move
                                    if moves:  # Ensure there's a turn entry for black's move
                                        moves[-1].append(move_notation_str)
                                    else: # Fallback for unexpected case (black moves first?)
                                        moves.append(['', move_notation_str])

                                move_start_time = pygame.time.get_ticks()
                                # --- Switch clocks after move ---
                                if active_color == 'w':
                                    white_clock.stop()
                                    black_clock.start()
                                    active_color = 'b'
                                else:
                                    black_clock.stop()
                                    white_clock.start()
                                    active_color = 'w'

                                if is_checkmate(enemy_color, piece_order):
                                    print(f"Checkmate! {'White' if enemy_color == 'b' else 'Black'} wins.")
                                    save_moves(moves, "1-0" if enemy_color == 'b' else "0-1")
                                    
                                elif is_stalemate(enemy_color, piece_order):
                                    print("Stalemate! It's a draw.")
                                    save_moves(moves, "1/2-1/2")
                                   
                                state = board_state_tuple(piece_order)
                                board_states[state] = board_states.get(state, 0) + 1
                                if board_states[state] == 3:
                                    print("Draw by threefold repetition!")
                                    save_moves(moves, "1/2-1/2")
                                    
                                elif is_insufficient_material(piece_order):
                                    print("Draw by insufficient material!")
                                    save_moves(moves, "1/2-1/2")
                                   
                                if running:
                                    turn += 1
                                    print(moves)
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_s:
                    save_moves(moves, "")
                elif event.key == pygame.K_r:
                    piece_order, turn, moves, dragging, board_states = reset_board()
                    white_clock = ChessClock(TIME_CONTROL)
                    black_clock = ChessClock(TIME_CONTROL)
                    active_color = 'w'
                    white_clock.start()
                    move_start_time = pygame.time.get_ticks()
                elif event.key == pygame.K_ESCAPE:
                    running = False

        window.fill(BACKGROUND_COLOR)
        window.blit(board, board_rect)
        window.blit(logo_img, logo_rect)
        mouse_pos = pygame.mouse.get_pos()
        if dragging is not None:
            draw_strip_pieces(window, piece_order, board_rect, piece_images, dragging=dragging, drag_offset=dragging['offset'], mouse_pos=mouse_pos)
        else:
            draw_strip_pieces(window, piece_order, board_rect, piece_images)

        # --- Draw clocks ---
        clock_width, clock_height = 200, 50
        clock_y_offset = board_rect.bottom + 20
        white_clock_rect = pygame.Rect(board_rect.left, clock_y_offset, clock_width, clock_height)
        black_clock_rect = pygame.Rect(board_rect.right - clock_width, clock_y_offset, clock_width, clock_height)
        # White clock: gray background, dark text
        pygame.draw.rect(window, (160, 160, 160), white_clock_rect, border_radius=8)
        white_time_surface = FONT.render(white_clock.get_time_str(), True, (80, 80, 80))
        window.blit(white_time_surface, (white_clock_rect.centerx - white_time_surface.get_width() // 2, white_clock_rect.centery - white_time_surface.get_height() // 2))
        # Black clock: dark background, gray text
        pygame.draw.rect(window, (80, 80, 80), black_clock_rect, border_radius=8)
        pygame.draw.rect(window, (160, 160, 160), black_clock_rect, border_radius=8, width=2)  # white border
        black_time_surface = FONT.render(black_clock.get_time_str(), True, (160, 160, 160))
        window.blit(black_time_surface, (black_clock_rect.centerx - black_time_surface.get_width() // 2, black_clock_rect.centery - black_time_surface.get_height() // 2))

        # --- Draw Buttons ---
        start_stop_button.text = "Stop" if (white_clock.running or black_clock.running) else "Start"
        start_stop_button.draw(window)
        reset_button.draw(window)
        new_game_button.draw(window)
        load_button.draw(window)
        pygame.display.update()
    pygame.quit()

if __name__ == "__main__":
    main()