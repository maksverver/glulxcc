#!/usr/bin/env python

from random import randint

special = [ i&0xffffffff for i in [ 0, 1, 2, -1, -2, 2**31, 2**31 - 1, 2**31 - 2, 2**31 + 1, 2**31 + 2, 2**32 - 1, 2**32 - 2 ] ]
rand1   = [ randint(0,2**31-1) for _ in range(50) ]
rand2   = [ randint(1,2**31-1) for _ in range(50) ]
rand3   = [ randint(1,2**24-1) for _ in range(25) ]
rand4   = [ randint(1,2**16-1) for _ in range(25) ]

def test(a, b):
	print 'test(%#010xu, %#010xu, %#010xu, %#010xu);' % (a, b, a/b, a%b)

for a in special:
	for b in special:
		if b != 0: test(a,b)
for a in special:
	for b in rand2: test(a, b)
for a in rand1:
	for b in special:
		if b != 0: test(a, b)
for a,b in zip(rand1, rand2): test(a, b)
for a,b in zip(rand1, rand3): test(a, b)
for a,b in zip(rand1, rand4): test(a, b)
