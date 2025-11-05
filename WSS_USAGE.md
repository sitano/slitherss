# WebSocket Secure (WSS) Support

This document describes how to use the WSS (WebSocket Secure) feature in the slither server.

## Overview

The server now supports both regular WebSocket (ws://) and secure WebSocket (wss://) connections. By default, the server runs in ws:// mode, but you can enable wss:// mode by providing TLS certificates.

## Requirements

- OpenSSL development libraries
- Valid TLS/SSL certificate and private key files

## Installation

### Install Dependencies

On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y libboost-all-dev libssl-dev
```

On macOS (using Homebrew):
```bash
brew install boost openssl
```

### Build the Server

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

## Usage

### Running with WSS (Secure WebSocket)

To enable WSS support, use the `--tls` flag along with certificate and key paths:

```bash
./bin/slither_server --tls --cert /path/to/certificate.pem --key /path/to/private-key.pem
```

You can also specify a custom port:

```bash
./bin/slither_server --tls --cert /path/to/certificate.pem --key /path/to/private-key.pem --port 8443
```

### Running without WSS (Regular WebSocket)

To run the server without TLS (default behavior):

```bash
./bin/slither_server
```

Or with a custom port:

```bash
./bin/slither_server --port 8080
```

## Command-Line Options

- `--tls`: Enable TLS/SSL (WSS) support
- `--cert <path>`: Path to TLS certificate file (required when --tls is enabled)
- `--key <path>`: Path to TLS private key file (required when --tls is enabled)
- `--port, -p <port>`: Bind port (default: 8080)
- `--verbose, -v`: Set verbose output
- `--debug, -d`: Enable debug mode
- `--help, -h`: Print help message

## Generating Self-Signed Certificates (for testing)

For development and testing purposes, you can generate self-signed certificates:

```bash
# Generate private key
openssl genrsa -out server-key.pem 2048

# Generate certificate signing request
openssl req -new -key server-key.pem -out server.csr

# Generate self-signed certificate (valid for 365 days)
openssl x509 -req -days 365 -in server.csr -signkey server-key.pem -out server-cert.pem
```

Then run the server:

```bash
./bin/slither_server --tls --cert server-cert.pem --key server-key.pem
```

**Note:** Self-signed certificates will generate browser warnings. For production use, obtain a certificate from a trusted Certificate Authority (CA) like Let's Encrypt.

## Client Connection

### Connecting with WSS

When the server is running with `--tls`, clients should connect using the `wss://` protocol:

```javascript
const socket = new WebSocket('wss://your-server:8443');
```

### Connecting without WSS

When the server is running without `--tls`, clients should use the `ws://` protocol:

```javascript
const socket = new WebSocket('ws://your-server:8080');
```

## Security Recommendations

1. **Always use WSS in production**: Regular WebSocket connections (ws://) are unencrypted and vulnerable to eavesdropping.

2. **Use valid certificates**: Obtain certificates from a trusted CA for production deployments.

3. **Keep certificates updated**: Monitor certificate expiration dates and renew them before they expire.

4. **Secure private keys**: Protect your private key files with appropriate file permissions:
   ```bash
   chmod 600 /path/to/private-key.pem
   ```

5. **Use strong cipher suites**: The server is configured to disable SSLv2 and SSLv3, which have known vulnerabilities.

## Troubleshooting

### "error: --cert and --key are required when --tls is enabled"

Make sure you provide both `--cert` and `--key` parameters when using `--tls`:

```bash
./bin/slither_server --tls --cert certificate.pem --key private-key.pem
```

### "TLS initialization error: ..."

Check that:
- Certificate and key files exist at the specified paths
- Certificate and key files are readable by the server process
- Certificate and key files are in PEM format
- The private key matches the certificate

### Connection refused or timeout

Ensure:
- The server is running
- The port is not blocked by a firewall
- You're using the correct protocol (wss:// vs ws://)
- The hostname/IP address is correct

## Technical Details

The server uses:
- **WebSocket++ library**: For WebSocket protocol implementation
- **Boost.Asio**: For asynchronous I/O and SSL/TLS support
- **OpenSSL**: For TLS/SSL cryptographic operations
- **TLS version**: TLS 1.2 by default
- **Disabled protocols**: SSLv2, SSLv3 (known security vulnerabilities)

## Example

Complete example with all options:

```bash
# Generate test certificates
openssl genrsa -out test-key.pem 2048
openssl req -new -x509 -key test-key.pem -out test-cert.pem -days 365

# Run server with WSS on port 8443 with debug mode
./bin/slither_server --tls --cert test-cert.pem --key test-key.pem --port 8443 --debug --verbose

# Connect from browser console
const ws = new WebSocket('wss://localhost:8443');
ws.onopen = () => console.log('Connected!');
ws.onmessage = (event) => console.log('Message:', event.data);
```
