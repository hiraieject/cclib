#!/opt/pstest/bin/python3

import time

for i in range(5):
    print(f'test command: {i}', flush=True)
    time.sleep(1)

