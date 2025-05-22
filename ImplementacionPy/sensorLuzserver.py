# server.py
from flask import Flask, request, jsonify
import csv
import os

app = Flask(__name__)
CSV_FILE = 'datos_ldr.csv'

# Crear archivo CSV con encabezados si no existe
if not os.path.isfile(CSV_FILE):
    with open(CSV_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['ldr1', 'ldr2'])

@app.route('/api/endpoint', methods=['POST'])
def recibir_datos():
    data = request.get_json()
    ldr1 = data.get('ldr1')
    ldr2 = data.get('ldr2')
    with open(CSV_FILE, mode='a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([ldr1, ldr2])
    return jsonify({'status': 'ok', 'received': data})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
