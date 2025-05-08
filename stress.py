# stress_test.py
import requests
import threading

def worker():
    for _ in range(100):
        try:
            requests.get("http://localhost:8000/index.html")
        except:
            pass

threads = []
for _ in range(50):  # 50 concurrent workers
    t = threading.Thread(target=worker)
    t.start()
    threads.append(t)

for t in threads:
    t.join()
