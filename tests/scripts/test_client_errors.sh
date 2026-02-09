#!/usr/bin/env bash
# Test client disconnections and error scenarios

BLUE='\033[0;34m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${BLUE}=== Testing Client Disconnections & Error Handling ===${NC}\n"

# Test 1: Malformed request
echo -e "${BLUE}Test 1: Malformed request (no HTTP version)${NC}"
timeout 2 bash -c 'printf "GET /index.html\r\nHost: localhost\r\n\r\n" | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should handle malformed request${NC}\n"
sleep 1

# Test 2: Incomplete headers
echo -e "${BLUE}Test 2: Disconnect during header transmission${NC}"
timeout 2 bash -c 'printf "GET /index.html HTTP/1.1\r\nHost: localhost\r\n" | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should cleanup disconnected client${NC}\n"
sleep 1

# Test 3: POST with complete body
echo -e "${BLUE}Test 3: POST with complete body${NC}"
timeout 2 bash -c 'printf "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello" | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should handle POST with body${NC}\n"
sleep 1

# Test 4: Multiple rapid connections
echo -e "${BLUE}Test 4: Multiple rapid connect/disconnect${NC}"
for i in {1..5}; do
    timeout 1 bash -c 'printf "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080' &
done
wait
echo -e "${GREEN}✓ Server should handle multiple simultaneous requests${NC}\n"
sleep 1

# Test 5: Invalid HTTP method
echo -e "${BLUE}Test 5: Invalid HTTP method${NC}"
timeout 2 bash -c 'printf "INVALID /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should return error for invalid method${NC}\n"
sleep 1

# Test 6: Request line too long
echo -e "${BLUE}Test 6: Extremely long request URI${NC}"
LONG_URI=$(printf 'A%.0s' {1..1000})
timeout 2 bash -c "printf 'GET /${LONG_URI} HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc localhost 8080" || true
echo -e "${GREEN}✓ Server should handle or reject oversized URI${NC}\n"
sleep 1

# Test 7: Malformed header
echo -e "${BLUE}Test 7: Malformed header (no value)${NC}"
timeout 2 bash -c 'printf "GET / HTTP/1.1\r\nHost:\r\n\r\n" | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should handle malformed headers${NC}\n"
sleep 1

# Test 8: Multiple Host headers
echo -e "${BLUE}Test 8: Multiple Host headers${NC}"
timeout 2 bash -c 'printf "GET / HTTP/1.1\r\nHost: localhost\r\nHost: evil.com\r\n\r\n" | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should handle duplicate headers${NC}\n"
sleep 1

# Test 9: POST with larger body
echo -e "${BLUE}Test 9: POST with larger body${NC}"
timeout 2 bash -c '(printf "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1024\r\n\r\n"; dd if=/dev/zero bs=1024 count=1 2>/dev/null) | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should handle POST with body${NC}\n"
sleep 1

# Test 10: Keep-alive multiple requests
echo -e "${BLUE}Test 10: Keep-alive then abrupt disconnect${NC}"
timeout 3 bash -c '(printf "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\nGET /about HTTP/1.1\r\nHost: localhost\r\n\r\n") | nc localhost 8080' || true
echo -e "${GREEN}✓ Server should handle keep-alive disconnect${NC}\n"
sleep 1

echo -e "${GREEN}=== All disconnect/error tests completed ===${NC}"
