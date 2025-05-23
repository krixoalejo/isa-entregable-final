# traffic_server.py
from flask import Flask, request, jsonify
import csv
import os
from datetime import datetime

app = Flask(__name__)
CSV_FILE = 'traffic_data.csv'

# Crear CSV si no existe
if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([
            'timestamp', 'vehiculos1', 'vehiculos2', 'co2',
            'boton1', 'boton2', 'mensaje', 'dia'
        ])

@app.route('/api/data', methods=['POST'])
def handle_data():
    data = request.get_json()
    
    record = [
        datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        data.get('vehiculos1', 0),
        data.get('vehiculos2', 0),
        data.get('co2', 0),
        data.get('boton1', False),
        data.get('boton2', False),
        data.get('mensaje', ''),
        data.get('dia', True)
    ]
    
    with open(CSV_FILE, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(record)
    
    return jsonify({'status': 'success'})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
