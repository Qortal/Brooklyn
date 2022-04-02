from binascii import hexlify, unhexlify
from os import urandom

def read_key_from_file(file):
    f = open(file)
    n_str = f.readline()[:-1]
    e_str = f.readline()[:-1]
    p_str = f.readline()[:-1]
    q_str = f.readline()[:-1]
    f.close()
    e = int(e_str, 16)
    p = int(p_str, 16)
    q = int(q_str, 16)
    n = int(n_str, 16)
    if n != p * q:
        raise ValueError("wrong key", p, q, n)
    return (unhexlify(n_str), unhexlify(e_str), unhexlify(p_str), unhexlify(q_str), e, p, q, n)

# egcd and modinv are from wikibooks
# https://en.wikibooks.org/wiki/Algorithm_Implementation/Mathematics/Extended_Euclidean_algorithm

def egcd(a, b):
    if a == 0:
        return (b, 0, 1)
    else:
        g, y, x = egcd(b % a, a)
        return (g, x - (b // a) * y, y)

def modinv(a, m):
    g, x, y = egcd(a, m)
    if g != 1:
        raise Exception('modular inverse does not exist')
    else:
        return x % m

def pkcs1_pad_for_sign(digestinfo):
    byte_repr = b'\x00' + b'\x01' \
                + bytes.ljust(b'', 256 - 19 - 32 - 3, b'\xff') \
                + b'\x00' + digestinfo
    return int(hexlify(byte_repr), 16)

def compute_signature(key, digestinfo):
    e = key[4]
    p = key[5]
    q = key[6]
    n = key[7]
    p1 = p - 1
    q1 = q - 1
    h = p1 * q1
    d = modinv(e, h)
    dp = d % p1
    dq = d % q1
    qp = modinv(q, p)

    input = pkcs1_pad_for_sign(digestinfo)
    t1 = pow(input, dp, p)
    t2 = pow(input, dq, q)
    t = ((t1 - t2) * qp) % p
    sig = t2 + t * q
    return sig

def integer_to_bytes_256(i):
    s = hex(i)[2:]
    s = s.rstrip('L')
    if len(s) & 1:
        s = '0' + s
    return bytes.rjust(unhexlify(s), 256, b'\x00')

def get_raw_pubkey(key):
    return key[0]
