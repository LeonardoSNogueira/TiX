import pygame
import os
import tkinter as tk
from tkinter import filedialog
import threading
import socket
import re

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
TIME_CONTROL = 600

# --- Font loading ---
pygame.font.init()
FONT = pygame.font.Font('../assets/fonts/DelaGothicOne-Regular.ttf',30)
pygame.display.set_caption('conecTiX')

# --- Button Class ---
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
def draw_strip_pieces(surface, piece_order, board_rect, piece_images):
    for i, piece_key in enumerate(piece_order):
        if piece_key:
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

def save_moves(moves_list):
    import sys
    root = tk.Tk()
    root.withdraw()  # Hide the main window
    file_path = filedialog.asksaveasfilename(
        defaultextension=".txt",
        filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
        initialdir=os.path.abspath("../games"),
        title="Save Game As"
    )
    if not file_path:
        return  # User cancelled
    # Determine result
    result_str = None
    # Use the global piece_order to check for checkmate/draw
    global piece_order
    if is_checkmate('b', piece_order):
        result_str = '1-0'
    elif is_checkmate('w', piece_order):
        result_str = '0-1'
    elif is_stalemate('w', piece_order) or is_stalemate('b', piece_order) or is_insufficient_material(piece_order):
        result_str = '1/2-1/2'
    else:
        print("Game not finished. Who won? Enter result:")
        print("1-0 for white, 0-1 for black, 1/2-1/2 for draw")
        result_str = input("Result: ").strip()
        if result_str not in ['1-0', '0-1', '1/2-1/2']:
            print("Invalid result. Saving without result.")
            result_str = None
    with open(file_path, "w") as file:
        for turn_num, turn_moves in enumerate(moves_list):
            pgn_line = f"{turn_num + 1}. "
            if len(turn_moves) > 0:  # White's move
                pgn_line += turn_moves[0]
            if len(turn_moves) > 1:  # Black's move
                pgn_line += f" {turn_moves[1]}"
            file.write(pgn_line + "\n")
        if result_str:
            file.write(result_str + "\n")

def reset_board():
    return PIECE_START_ORDER.copy(), 0, [], None, {}

# --- Bluetooth Comms ---
def bluetooth_listener(callback):
    """
    Listens for incoming bluetooth (TCP) messages from the ESP32 (or simul.py).
    Calls the provided callback with the received array as arguments.
    """
    def listen():
        host = 'localhost'
        port = 65432  
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((host, port))
            s.listen()
            print(f"Listening for ESP32/simul.py on {host}:{port}...")
            while True:
                conn, addr = s.accept()
                with conn:
                    data = conn.recv(1024)
                    if data:
                        try:
                            arr = eval(data.decode())  # e.g., "[1, 3, 300, 600]"
                            if isinstance(arr, list):
                                callback(*arr)
                        except Exception as e:
                            print(f"Error decoding bluetooth message: {e}")
    thread = threading.Thread(target=listen, daemon=True)
    thread.start()

# --- Main Game Loop ---
# Global game state for access in callback
piece_order = None
moves = None
white_clock = None
black_clock = None
current_turn = 0  # 0 for white, 1 for black

def send_bluetooth_response(response_arr):
    host = 'localhost'
    port = 65433  # Use a different port for responses
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            s.sendall(str(response_arr).encode())
    except Exception as e:
        print(f"Error sending bluetooth response: {e}")

def handle_bluetooth_message(origin, destination, time_remaining, time_control):
    global piece_order, moves, white_clock, black_clock, current_turn, TIME_CONTROL
    TIME_CONTROL = time_control
    piece = piece_order[origin]
    if piece is None:
        print(f"No piece at origin {origin}")
        send_bluetooth_response([0, 0])
        return
    # Enforce turn order
    expected_color = 'w' if current_turn == 0 else 'b'
    if piece[0] != expected_color:
        print(f"It's not {piece[0]}'s turn!")
        send_bluetooth_response([0, 0])
        return
    # Check if move is legal
    if not legal_move(piece, origin, destination, piece_order):
        print(f"Illegal move: {piece} from {origin} to {destination}")
        send_bluetooth_response([0, 0])
        return
    captured = piece_order[destination]
    piece_order[destination] = piece
    piece_order[origin] = None
    move_str = notation(piece, destination, 'b' if piece[0] == 'w' else 'w', captured is not None, piece_order)
    move_with_time = f"{move_str} {int(time_remaining)}"
    if len(moves) == 0 or len(moves[-1]) == 2:
        moves.append([move_with_time])
    else:
        moves[-1].append(move_with_time)
    if piece[0] == 'w':
        white_clock.time_left = time_remaining
        current_turn = 1
    else:
        black_clock.time_left = time_remaining
        current_turn = 0
    # Check for win/draw
    winner = 0
    if is_checkmate('b', piece_order):
        winner = 1  # White wins
    elif is_checkmate('w', piece_order):
        winner = 2  # Black wins
    elif is_stalemate('w', piece_order) or is_stalemate('b', piece_order) or is_insufficient_material(piece_order):
        winner = 3  # Draw
    send_bluetooth_response([1, winner])
    print(f"Bluetooth move: {origin}->{destination}, {piece}, time: {time_remaining}")
    print(moves)

# --- Scene Management ---
current_scene = "main"  # can be 'main' or 'analysis'
analysis_moves = []
analysis_times = []
analysis_index = 0
analysis_piece_order = None
analysis_white_clock = None
analysis_black_clock = None
analysis_result = None

def parse_game_file(file_path):
    moves = []
    times = []
    result = None
    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line in ['1-0', '0-1', '1/2-1/2']:
                result = line
                continue
            if line[0].isdigit() is False:
                continue
            # Example: 1. N4 596 N5 598
            parts = line.split()
            if len(parts) >= 3:
                # White move, white time, black move, black time
                if len(parts) >= 5:
                    moves.append((parts[1], parts[3]))
                    times.append((int(parts[2]), int(parts[4])))
                else:
                    moves.append((parts[1],))
                    times.append((int(parts[2]),))
    return moves, times, result

def set_analysis_state(index):
    global analysis_piece_order, analysis_white_clock, analysis_black_clock
    analysis_piece_order, _, _, _, _ = reset_board()
    analysis_white_clock = ChessClock(TIME_CONTROL)
    analysis_black_clock = ChessClock(TIME_CONTROL)
    if index < 0:
        return
    for i in range(index+1):
        turn = i % 2
        move_pair = analysis_moves[i//2] if turn == 0 else analysis_moves[i//2]
        time_pair = analysis_times[i//2] if turn == 0 else analysis_times[i//2]
        if turn == 0:
            move_str = move_pair[0]
            time_val = time_pair[0]
            match = re.search(r'(\d+)', move_str)
            if not match:
                continue
            dest = int(match.group(1)) - 1
            for idx, piece in enumerate(analysis_piece_order):
                if piece and piece[0] == 'w' and piece[1] == move_str[0]:
                    analysis_piece_order[dest] = piece
                    analysis_piece_order[idx] = None
                    break
            analysis_white_clock.time_left = time_val
        else:
            if len(move_pair) > 1:
                move_str = move_pair[1]
                time_val = time_pair[1]
                match = re.search(r'(\d+)', move_str)
                if not match:
                    continue
                dest = int(match.group(1)) - 1
                for idx, piece in enumerate(analysis_piece_order):
                    if piece and piece[0] == 'b' and piece[1] == move_str[0]:
                        analysis_piece_order[dest] = piece
                        analysis_piece_order[idx] = None
                        break
                analysis_black_clock.time_left = time_val

def draw_analysis_scene(window, board, board_rect, logo_img, logo_rect, piece_images, FONT):
    # Move everything down
    top_offset = 60
    left_offset = 100
    board_rect_left = board_rect.copy()
    board_rect_left.left = left_offset
    board_rect_left.top += top_offset
    # Draw logo centered above the board
    logo_rect_left = logo_rect.copy()
    logo_rect_left.centerx = board_rect_left.centerx
    logo_rect_left.bottom = board_rect_left.top - 10
    window.blit(logo_img, logo_rect_left)
    window.blit(board, board_rect_left)
    draw_strip_pieces(window, analysis_piece_order, board_rect_left, piece_images)
    # Draw clocks below board
    clock_width, clock_height = 200, 50
    clock_y_offset = board_rect_left.bottom + 20
    white_clock_rect = pygame.Rect(board_rect_left.left, clock_y_offset, clock_width, clock_height)
    black_clock_rect = pygame.Rect(board_rect_left.right - clock_width, clock_y_offset, clock_width, clock_height)
    pygame.draw.rect(window, (160, 160, 160), white_clock_rect, border_radius=8)
    white_time_surface = FONT.render(analysis_white_clock.get_time_str(), True, (80, 80, 80))
    window.blit(white_time_surface, (white_clock_rect.centerx - white_time_surface.get_width() // 2, white_clock_rect.centery - white_time_surface.get_height() // 2))
    pygame.draw.rect(window, (80, 80, 80), black_clock_rect, border_radius=8)
    pygame.draw.rect(window, (160, 160, 160), black_clock_rect, border_radius=8, width=2)
    black_time_surface = FONT.render(analysis_black_clock.get_time_str(), True, (160, 160, 160))
    window.blit(black_time_surface, (black_clock_rect.centerx - black_time_surface.get_width() // 2, black_clock_rect.centery - black_time_surface.get_height() // 2))
    # Draw move list background on the right
    move_list_x = board_rect_left.right + 60
    move_list_y = board_rect_left.top
    move_list_width = 260
    move_list_height = max(360, len(analysis_moves) * 32 + 60)
    pygame.draw.rect(window, (44,43,41), (move_list_x - 20, move_list_y - 20, move_list_width, move_list_height), border_radius=16)
    # Draw move list (moves only)
    move_font = pygame.font.Font('../assets/fonts/DelaGothicOne-Regular.ttf', 24)
    for i, move_pair in enumerate(analysis_moves):
        move_str = f"{i+1}. {move_pair[0]}"
        if len(move_pair) > 1:
            move_str += f" {move_pair[1]}"
        move_surface = move_font.render(move_str, True, (220, 220, 220))
        window.blit(move_surface, (move_list_x, move_list_y + i * 32))
    # Draw result at the end if present
    if analysis_result:
        result_font = pygame.font.Font('../assets/fonts/DelaGothicOne-Regular.ttf', 28)
        result_surface = result_font.render(analysis_result, True, (149, 171, 129))
        result_y = move_list_y + len(analysis_moves) * 32 + 16
        window.blit(result_surface, (move_list_x, result_y))
    # Draw navigation buttons below clocks, centered
    nav_button_width, nav_button_height = 120, 50
    nav_y = black_clock_rect.bottom + 30
    total_width = nav_button_width * 2 + 40
    nav_x = board_rect_left.centerx - total_width // 2
    prev_button_rect = pygame.Rect(nav_x, nav_y, nav_button_width, nav_button_height)
    next_button_rect = pygame.Rect(nav_x + nav_button_width + 40, nav_y, nav_button_width, nav_button_height)
    prev_button = Button(prev_button_rect, "Prev", FONT, (44,43,41), (149,171,129))
    next_button = Button(next_button_rect, "Next", FONT, (44,43,41), (149,171,129))
    prev_button.draw(window)
    next_button.draw(window)
    return prev_button, next_button

def main():
    global TIME_CONTROL, piece_order, moves, white_clock, black_clock, current_turn
    global current_scene, analysis_moves, analysis_times, analysis_index, analysis_piece_order, analysis_white_clock, analysis_black_clock, analysis_result
    pygame.init()
    # Initialize game state
    piece_order, _, moves, _, _ = reset_board()
    white_clock = ChessClock(TIME_CONTROL)
    black_clock = ChessClock(TIME_CONTROL)
    current_turn = 0
    bluetooth_listener(handle_bluetooth_message)

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
    save_button = Button(
        rect=((WINDOW_WIDTH) // 2 - 120, button_y + 2 * (button_height + 15), button_width+50, button_height+50),
        text="Save",
        font=FONT,
        bg_color=  (44,43,41),
        text_color=(149,171,129),
        border_color=None,
        border_width=2
    )
    new_game_button = Button(
        rect=((WINDOW_WIDTH) // 2 - 360, button_y + 2 * (button_height + 15), button_width+50, button_height+50),
        text="New Game",
        font=FONT,
        bg_color=  (44,43,41),
        text_color=(149,171,129),
        border_color=None,
        border_width=2
    )
    load_button = Button(
        rect=((WINDOW_WIDTH) // 2 + 120, button_y + 2 * (button_height + 15), button_width+50, button_height+50),
        text="Load Game",
        font=FONT,
        bg_color=  (44,43,41),
        text_color=(149,171,129),
        border_color=None,
        border_width=2
    )
    prev_button = None
    next_button = None
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if current_scene == "main":
                    if event.key == pygame.K_s: 
                        save_moves(moves)
                    elif event.key == pygame.K_r:
                        piece_order, _, moves, _, _ = reset_board()
                        white_clock = ChessClock(TIME_CONTROL)
                        black_clock = ChessClock(TIME_CONTROL)
                        current_turn = 0
                    elif event.key == pygame.K_ESCAPE:
                        running = False
                elif current_scene == "analysis":
                    if event.key == pygame.K_ESCAPE:
                        current_scene = "main"
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                mouse_pos = pygame.mouse.get_pos()
                if current_scene == "main":
                    if save_button.is_clicked(mouse_pos):
                        save_moves(moves)
                    elif new_game_button.is_clicked(mouse_pos):
                        if moves and len(moves) > 0:
                            save_moves(moves)
                        piece_order, _, moves, _, _ = reset_board()
                        white_clock = ChessClock(TIME_CONTROL)
                        black_clock = ChessClock(TIME_CONTROL)
                        current_turn = 0
                    elif load_button.is_clicked(mouse_pos):
                        # Load game file and switch to analysis scene
                        root = tk.Tk()
                        root.withdraw()
                        file_path = filedialog.askopenfilename(
                            defaultextension=".txt",
                            filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
                            initialdir=os.path.abspath("../games"),
                            title="Load Game File"
                        )
                        if file_path:
                            analysis_moves, analysis_times, analysis_result = parse_game_file(file_path)
                            analysis_index = -1
                            set_analysis_state(analysis_index)
                            current_scene = "analysis"
                elif current_scene == "analysis":
                    if prev_button and prev_button.is_clicked(mouse_pos):
                        if analysis_index > -1:
                            analysis_index -= 1
                            set_analysis_state(analysis_index)
                    elif next_button and next_button.is_clicked(mouse_pos):
                        if analysis_index < len(analysis_moves)*2-1:
                            analysis_index += 1
                            set_analysis_state(analysis_index)
        window.fill(BACKGROUND_COLOR)
        if current_scene == "main":
            window.blit(board, board_rect)
            window.blit(logo_img, logo_rect)
            draw_strip_pieces(window, piece_order, board_rect, piece_images)
            # --- Draw clocks (dynamic) ---
            clock_width, clock_height = 200, 50
            clock_y_offset = board_rect.bottom + 20
            white_clock_rect = pygame.Rect(board_rect.left, clock_y_offset, clock_width, clock_height)
            black_clock_rect = pygame.Rect(board_rect.right - clock_width, clock_y_offset, clock_width, clock_height)
            pygame.draw.rect(window, (160, 160, 160), white_clock_rect, border_radius=8)
            white_time_surface = FONT.render(white_clock.get_time_str(), True, (80, 80, 80))
            window.blit(white_time_surface, (white_clock_rect.centerx - white_time_surface.get_width() // 2, white_clock_rect.centery - white_time_surface.get_height() // 2))
            pygame.draw.rect(window, (80, 80, 80), black_clock_rect, border_radius=8)
            pygame.draw.rect(window, (160, 160, 160), black_clock_rect, border_radius=8, width=2) 
            black_time_surface = FONT.render(black_clock.get_time_str(), True, (160, 160, 160))
            window.blit(black_time_surface, (black_clock_rect.centerx - black_time_surface.get_width() // 2, black_clock_rect.centery - black_time_surface.get_height() // 2))
            # --- Draw Buttons ---
            start_stop_button.text = "Start"
            start_stop_button.draw(window)
            save_button.draw(window)
            new_game_button.draw(window)
            load_button.draw(window)
        elif current_scene == "analysis":
            prev_button, next_button = draw_analysis_scene(window, board, board_rect, logo_img, logo_rect, piece_images, FONT)
        pygame.display.update()
    pygame.quit()

if __name__ == "__main__":
    main()