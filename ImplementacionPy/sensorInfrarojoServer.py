# infrared_server.py
from flask import Flask, request, jsonify
import csv
import os
from datetime import datetime

app = Flask(__name__)
CSV_FILE = 'infrared_data.csv'

# Crear CSV con encabezados si no existe
if not os.path.exists(CSV_FILE):
    with open(CSV_FILE, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['timestamp', 'cny1', 'cny2', 'cny3', 'cny4', 'cny5', 'cny6'])

@app.route('/api/infrared', methods=['POST'])
def handle_data():
    data = request.get_json()
    
    # Registrar datos con marca de tiempo
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    record = [
        timestamp,
        data.get('cny1', 0),
        data.get('cny2', 0),
        data.get('cny3', 0),
        data.get('cny4', 0),
        data.get('cny5', 0),
        data.get('cny6', 0)
    ]
    
    # Escribir en CSV
    with open(CSV_FILE, 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(record)
    
    return jsonify({'status': 'success', 'received': data})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
