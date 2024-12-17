import serial
import time
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

# Configuración del puerto serial
serial_port = "COM3"
baud_rate = 9600  # Asegúrate de que coincida con la configuración de tu Arduino
ser = serial.Serial(serial_port, baud_rate)
time.sleep(2)  # Tiempo para que el puerto serial se estabilice

# Configuración de la gráfica 3D
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Función para actualizar la orientación
def update_orientation(roll, pitch, yaw):
    # Crear un cubo para representar el dron
    l = 1.0  # Longitud del cubo
    x = np.array([-l, l, l, -l, -l, l, l, -l]) / 2
    y = np.array([-l, -l, l, l, -l, -l, l, l]) / 2
    z = np.array([-l, -l, -l, -l, l, l, l, l]) / 2
    
    # Rotación basada en roll, pitch, yaw
    roll_rad = np.radians(roll)
    pitch_rad = np.radians(pitch)
    yaw_rad = np.radians(yaw)
    
    # Matrices de rotación
    R_x = np.array([
        [1, 0, 0],
        [0, np.cos(roll_rad), -np.sin(roll_rad)],
        [0, np.sin(roll_rad), np.cos(roll_rad)],
    ])
    
    R_y = np.array([
        [np.cos(pitch_rad), 0, np.sin(pitch_rad)],
        [0, 1, 0],
        [-np.sin(pitch_rad), 0, np.cos(pitch_rad)],
    ])
    
    R_z = np.array([
        [np.cos(yaw_rad), -np.sin(yaw_rad), 0],
        [np.sin(yaw_rad), np.cos(yaw_rad), 0],
        [0, 0, 1],
    ])
    
    # Rotación completa
    R = R_z @ R_y @ R_x
    coords = np.dot(R, np.vstack([x, y, z]))
    
    # Limpiar y redibujar el cubo
    ax.cla()
    ax.plot_wireframe(
        coords[0].reshape(2, 4),
        coords[1].reshape(2, 4),
        coords[2].reshape(2, 4),
    )
    ax.set_xlim(-1, 1)
    ax.set_ylim(-1, 1)
    ax.set_zlim(-1, 1)
    ax.set_title(f"Roll: {roll}°, Pitch: {pitch}°, Yaw: {yaw}°")
    plt.pause(0.01)

# Leer datos del serial y actualizar la gráfica
try:
    while True:
        
        line = ser.readline().decode('latin-1').strip()

        if line:
            # Parsear los datos del serial
            try:
                # Dividir la línea en base al nuevo formato
                parts = line.split(";")
                roll = int(parts[0].split(":")[1])
                pitch = int(parts[1].split(":")[1])
                yaw = int(parts[2].split(":")[1])
                
                # Actualizar orientación
                update_orientation(roll, pitch, yaw)
            except (ValueError, IndexError):
                print(f"Error parsing line: {line}")
except KeyboardInterrupt:
    print("Finalizando...")
finally:
    ser.close()
    plt.close()
