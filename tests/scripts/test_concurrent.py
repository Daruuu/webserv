#!/usr/bin/env python3
"""
Test concurrent requests and connections to stress-test the server
"""
import socket
import threading
import time
import sys

HOST = 'localhost'
PORT = 8080
NUM_CONCURRENT = 20

results = {
    'success': 0,
    'failed': 0,
    'timeout': 0,
    'disconnect': 0
}
lock = threading.Lock()

def send_request(req_id, disconnect=False, delay=0):
    """Send a single HTTP request"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((HOST, PORT))
        
        request = f"GET /?req={req_id} HTTP/1.1\r\nHost: {HOST}\r\n\r\n"
        sock.sendall(request.encode())
        
        if delay > 0:
            time.sleep(delay)
        
        if disconnect:
            # Simulate abrupt disconnect
            sock.close()
            with lock:
                results['disconnect'] += 1
            return
        
        # Read response
        response = b""
        while True:
            chunk = sock.recv(1024)
            if not chunk:
                break
            response += chunk
            if b"\r\n\r\n" in response or b"\n\n" in response:
                break
        
        sock.close()
        
        if b"HTTP/1.1 200" in response or b"HTTP/1.0 200" in response:
            with lock:
                results['success'] += 1
        else:
            with lock:
                results['failed'] += 1
                
    except socket.timeout:
        with lock:
            results['timeout'] += 1
    except Exception as e:
        with lock:
            results['failed'] += 1
        print(f"Request {req_id} error: {e}")

def test_concurrent_normal():
    """Test normal concurrent requests"""
    print(f"\n=== Test 1: {NUM_CONCURRENT} concurrent normal requests ===")
    threads = []
    for i in range(NUM_CONCURRENT):
        t = threading.Thread(target=send_request, args=(i,))
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()
    
    print(f"Success: {results['success']}")
    print(f"Failed: {results['failed']}")
    print(f"Timeout: {results['timeout']}")

def test_concurrent_disconnect():
    """Test concurrent disconnects"""
    print(f"\n=== Test 2: {NUM_CONCURRENT//2} requests with abrupt disconnects ===")
    
    # Reset results
    for key in results:
        results[key] = 0
    
    threads = []
    for i in range(NUM_CONCURRENT // 2):
        # Half disconnect, half complete
        disconnect = (i % 2 == 0)
        t = threading.Thread(target=send_request, args=(i, disconnect))
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()
    
    print(f"Success: {results['success']}")
    print(f"Disconnects: {results['disconnect']}")
    print(f"Failed: {results['failed']}")

def test_slow_clients():
    """Test slow clients that delay between connect and request"""
    print(f"\n=== Test 3: Slow clients (connect but delay) ===")
    
    # Reset results
    for key in results:
        results[key] = 0
    
    threads = []
    for i in range(10):
        t = threading.Thread(target=send_request, args=(i, False, 0.5))
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()
    
    print(f"Success: {results['success']}")
    print(f"Timeout: {results['timeout']}")

def test_rapid_connect_disconnect():
    """Rapid connect/disconnect cycles"""
    print(f"\n=== Test 4: Rapid connect/disconnect cycles ===")
    
    count = 0
    for i in range(50):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((HOST, PORT))
            sock.close()
            count += 1
        except Exception as e:
            print(f"Error on iteration {i}: {e}")
    
    print(f"Successfully completed {count}/50 rapid connects")

if __name__ == "__main__":
    print("=== WebServ Concurrent Request Tests ===")
    print(f"Target: {HOST}:{PORT}")
    
    try:
        test_concurrent_normal()
        time.sleep(2)
        
        test_concurrent_disconnect()
        time.sleep(2)
        
        test_slow_clients()
        time.sleep(2)
        
        test_rapid_connect_disconnect()
        
        print("\n=== All tests completed ===")
        print("Check server logs for proper handling and no crashes")
        
    except KeyboardInterrupt:
        print("\nTests interrupted")
        sys.exit(1)
