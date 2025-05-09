# stress_test_with_stats.py
import requests
import threading

success = 0
fail = 0
lock = threading.Lock()

def worker():
    global success, fail
    for _ in range(1000):
        try:
            r = requests.get("http://localhost:8000/empty.html", timeout=5)
            if r.status_code == 200:
                with lock:
                    success += 1
            else:
                with lock:
                    fail += 1
        except:
            with lock:
                fail += 1

threads = []
for _ in range(50):  # 50 concurrent workers
    t = threading.Thread(target=worker)
    t.start()
    threads.append(t)

for t in threads:
    t.join()

total = success + fail
availability = (success / total) * 100 if total else 0
print(f"Total Requests: {total}")
print(f"Success: {success}")
print(f"Fail: {fail}")
print(f"Availability: {availability:.2f}%")
