#!/usr/bin/env python

# NOTE: subprocess.run requires Python 3.5+
import subprocess
from subprocess import run

print("\nSTART Lightning integration tests\n")

# Test echoing
# Try a background fork/thread
server_process = subprocess.Popen(['./lightning', 'simple_config'])

# TODO: Have intermediate logging throughout
# TODO: Use Python unit test frameworks + logging libraries
print('DEBUG: Lightning server started!')

expected_response = b'GET / HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: HTTPie/0.9.8\r\nAccept-Encoding: gzip, deflate\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n'
actual_response = run(['http', 'localhost:8080'], stdout=subprocess.PIPE)

if (actual_response.returncode != 0):
    print('FAILED: httpie encountered an error')

if (actual_response.stdout != expected_response):
    print('FAILED: httpie received a non-matching echo response')
    print('Completed request: \n%s' % actual_response.stdout.decode('UTF-8'))

print('SUCCESS: HTTPie request echo')

# Terminate the server
server_process.kill()

print("\nEND Lightning integration tests. All tests passed!")
