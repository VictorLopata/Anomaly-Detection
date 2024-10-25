import psycopg2
import matplotlib.pyplot as plt

# Configurazione della connessione al database
db_config = {
    'host': 'localhost',
    'port': '5432',
    'database': 'anomalyseeker',
    'user': 'admin',
    'password': 'admin'
}

def fetch_data(sensor_id):
    try:
        # Stabilire la connessione
        conn = psycopg2.connect(**db_config)
        cursor = conn.cursor()

        # Query per recuperare i dati, includendo is_anomaly
        query = """
            SELECT calculate_at, value, is_anomaly
            FROM Average
            WHERE sensor_id = %s
            ORDER BY calculate_at
        """
        cursor.execute(query, (sensor_id,))
        data = cursor.fetchall()

        # Chiudere la connessione
        cursor.close()
        conn.close()

        return data

    except Exception as e:
        print(f"Si è verificato un errore: {e}")
        return None

def plot_data(data, sensor_id):
    if not data:
        print("Nessun dato da visualizzare.")
        return

    # Separare i dati in tre liste
    calculate_at, value, is_anomaly = zip(*data)

    # Creare una lista di colori in base a is_anomaly
    colors = ['red' if anomaly else 'blue' for anomaly in is_anomaly]

    # Creare il grafico
    plt.figure(figsize=(10, 5))
    plt.scatter(calculate_at, value, c=colors)
    plt.title(f'Sensor ID {sensor_id}: Valore nel Tempo')
    plt.xlabel('Calculate At')
    plt.ylabel('Value')
    plt.grid(True)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    sensor_id = int(input("Inserisci sensor_id: "))
    data = fetch_data(sensor_id)
    plot_data(data, sensor_id)

