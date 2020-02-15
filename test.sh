#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp test.o tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# try 0 0
# try 42 42
# try 21 "5+20-4;"
# try 41 " 12 + 34 - 5 ;"
# try 47 "5+6*7;"
# try 15 "5*(9-6);"
# try 4 "(3+5)/2;"
# try 10 "-10+20;"
# try 20 "-4*(-5);"
# try 3 "+5-2;"

# try 0 '0==1;'
# try 1 '42==42;'
# try 1 '0!=1;'
# try 0 '42!=42;'

# try 1 '0<1;'
# try 0 '1<1;'
# try 0 '2<1;'
# try 1 '0<=1;'
# try 1 '1<=1;'
# try 0 '2<=1;'

# try 1 '1>0;'
# try 0 '1>1;'
# try 0 '1>2;'
# try 1 '1>=0;'
# try 1 '1>=1;'
# try 0 '1>=2;'

try 4 'a=1; a+3;'
try 5 'a=2; z=3; a+z;'
try 8 'aho=2; z1a=3; aho=5; aho+z1a;'
try 8 'aho=2; z1a=3; aho=5; return aho+z1a;'
try 5 'aho=2; z_a=3; return aho+z_a; aho=5;'

try 7 'a = 3; b = 4; if (a != b) return a + b;'
try 1 'a = 4; b = 3; if (a == b) return a + b; else return a - b;'
try 7 'a = 4; b = 3; if (a != b) return a + b; else return a - b;'
try 4 'a = 4; b = 3; if (a != b) if (a > b) return a; else return b; else return a - b;'
try 5 'a = 5; b = 0; while (a != b) b = b + 1; return b;'
try 10 'a = 5; for(i = 0; i < 5; i = i + 1) a = a + 1; return a;'
try 10 'a = 5; for(; a < 10 ;) a = a + 1; return a;'
try 4 'a = 4; b = 3; if (a == b) return a + b; else if (a > b) return a; else return b;'
try 2 'a = 4; b = 5; if (a == b) { return a + b; } else { a = 1; b = 1; return a + b; }'
try 9 'a = 4; b = 5; if (a != b) { return a + b; } else { a = 1; b = 1; return a + b; }'
try 0 'foo();'

echo OK
