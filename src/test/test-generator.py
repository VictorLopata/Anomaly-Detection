import csv
import time
import redis

FILE_CSV = './csvfiles/file1.csv'
N_SENSOR = 10
current_time = None
s_data = {}


# Connessione a redis
redis_client = redis.Redis(host='localhost', port=6379, db=0)


def send(sensor_data: dict):
    print("Mando i dati:")
    for sId, v in sensor_data.items():
        print(f'Value = {v}         Sensor = {sId}')
        stream = f'stream#{sId}'
        data  = {
            'Value': v
        }
        redis_client.xadd(stream, data)


with open(FILE_CSV, mode='r') as csvfile:
    csv_reader = csv.reader(csvfile, delimiter=',')

    # Skip the first row
    next(csv_reader)
    skip = False
    for row in csv_reader:
        sample_time = int(row[0].strip())
        sensor_id = int(row[1].strip())
        value = row[2].strip()
        if sensor_id == N_SENSOR:
            skip = True
        # Controllo se value è definito
        if value != 'NULL':
            value = float(value)
        else:
            continue

        if current_time is None:
            current_time = sample_time

        if sample_time == current_time:
            if skip:
                continue
            s_data[sensor_id] = value
        else:
            # Aspetta 1 secondo
            # TODO: Casuali
            time.sleep(1)

            send(s_data)

            current_time = sample_time
            s_data = {sensor_id: value}
            skip = False
    if s_data:
        send(s_data)







