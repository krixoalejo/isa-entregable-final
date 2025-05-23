# server.py
from flask import Flask, request, jsonify
import csv
import os
from datetime import datetime

app = Flask(__name__)
CSV_FILE = 'traffic_data.csv'

HEADERS = [
    'timestamp', 'cambios', 'modo_noche',
    'ldr1', 'ldr2', 'co2',
    'boton1', 'boton2',
    'cny1', 'cny2', 'cny3', 'cny4', 'cny5', 'cny6'
]

# Variables globales para el modo
modo_variable = None
modo_valor = None

if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(HEADERS)

@app.route('/api/data', methods=['POST'])
def handle_data():
    print("Headers:", dict(request.headers))
    #print("Raw body:", request.data.decode('utf-8'))  # Mostrar texto crudo
    
    try:
        data = request.get_json(force=True)  # Forzar parseo JSON
        #print("Datos recibidos:", data)
    except Exception as e:
        print("Error al parsear JSON:", e)
        data = None
    
    # Aquí sigue el procesamiento normal, por ejemplo:
    if data is None:
        return jsonify({'status': 'error', 'message': 'JSON inválido o ausente'}), 400
    
    # Procesar data normalmente...    
    record = [
        datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        data.get('cambios', 0),
        data.get('modo_noche', False),
        data.get('ldr1', 0),
        data.get('ldr2', 0),
        data.get('co2', 0),
        data.get('boton1', 0),
        data.get('boton2', 0),
        data.get('cny1', 0),
        data.get('cny2', 0),
        data.get('cny3', 0),
        data.get('cny4', 0),
        data.get('cny5', 0),
        data.get('cny6', 0)
    ]
    
    with open(CSV_FILE, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(record)
    
    return jsonify({'status': 'success'})

@app.route('/get-mode', methods=['GET'])
def get_mode():
    if modo_variable is None or modo_valor is None:
        return ('', 204)
    return jsonify({"variable": modo_variable, "valor": modo_valor})

@app.route('/set-mode', methods=['POST'])
def set_mode():
    global modo_variable, modo_valor
    data = request.get_json()
    modo_variable = data.get("variable")
    modo_valor = data.get("valor")
    return jsonify({"status": "ok"})

@app.route('/ack-mode', methods=['POST'])
def ack_mode():
    global modo_variable, modo_valor
    modo_variable = None
    modo_valor = None
    return jsonify({"status": "cleared"})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
