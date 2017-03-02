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

expected_response = b'GET /echo HTTP/1.1\r\nHost: localhost:2020\r\nAccept-Encoding: gzip, deflate, compress\r\nAccept: */*\r\nUser-Agent: HTTPie/0.8.0\r\n\r\n'
actual_response = run(['http', 'localhost:2020/echo'], stdout=subprocess.PIPE)

if (actual_response.returncode != 0):
    print('FAILED: httpie encountered an error')

if (actual_response.stdout != expected_response):
    print('FAILED: httpie received a non-matching echo response')
    print('Expected response: \n%s' % expected_response)
    print('Completed request: \n%s' % actual_response.stdout.decode('UTF-8'))

print('SUCCESS: HTTPie request echo')

# Test proxy server 
print('DEBUG: Creating proxy server!')
proxy_server_process = subprocess.Popen(['./lightning', 'proxy_config'])

expected_proxy_response = b'<!DOCTYPE html><html><head></head><body><h1>Hello World!</h1></body></html>'
actual_proxy_response = run(['http', 'localhost:3030/reverse_proxy/static/index.html'], stdout=subprocess.PIPE)

if (actual_proxy_response.returncode != 0):
    print('FAILED: proxy server encountered an error')

if (actual_proxy_response.stdout != expected_proxy_response):
    print('FAILED: proxy server received a non-matching proxy response')
    print('Expected proxy response: \n%s' % expected_proxy_response)
    print('Completed proxy response: \n%s' % actual_proxy_response.stdout.decode('UTF-8'))

# Terminate the server
server_process.kill()
proxy_server_process.kill()


print("\nEND Lightning integration tests. All tests passed!")
