import socket
import time

HOST = 'localhost'
PORT = 65432
running = True

def send_bluetooth_message(arr):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(str(arr).encode())

def simulate_message():
    try:
        origin = int(input("Origin array index: "))
        destination = int(input("Destination array index: "))
        time_remaining = int(input("Time remaining (seconds): "))
        time_control = int(input("Time control (seconds): "))
        send_bluetooth_message([origin, destination, time_remaining, time_control])
        print(f"Sent: {[origin, destination, time_remaining, time_control]}")
        receive_bluetooth_response()
    except Exception as e:
        print(f"Error: {e}")

def simulate_game():
    moves = [
        [1, 3, 590, 600],  # White N: 2->4 
        [6, 4, 587, 600],  # Black N: 7->5 
        [3, 5, 580, 600],  # White N: 4->6 
        [7, 6, 577, 600],  # Black K: 8->7 
        [2, 3, 570, 600],  # White R: 2->4  
    ]
    for move in moves:
        send_bluetooth_message(move)
        print(f"Sent: {move}")
        receive_bluetooth_response()
        time.sleep(0.5)

def receive_bluetooth_response():
    host = 'localhost'
    port = 65433
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        conn, addr = s.accept()
        with conn:
            data = conn.recv(1024)
            if data:
                print(f"Response from main.py: {data.decode()}")

if __name__ == "__main__":
    while running:
        menu = input("Choose an option:\n1. Send move\n2. Quit\n3. Send premade game\n> ")
        if menu == '1':
            simulate_message()
        elif menu == '2':
            running = False
        elif menu == '3':
            simulate_game()
        else:
            print("Invalid option.")