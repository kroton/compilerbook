#!/bin/bash

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 6 "1+10/2;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 0 "-3+3;"
try 0 "-(3+5)+8;"
try 15 "-3*-5;"
try 15 "-(1+2)*-(2+3);"
try 1 "2==2;"
try 0 "2==3;"
try 1 "1+2+3==1*2*3;"
try 1 "(1==1)+(1==2);"
try 2 "(1==1)+(2==2);"
try 1 "2!=3;"
try 0 "2!=2;"
try 1 "1<2;"
try 0 "2<1;"
try 1 "(1+1)<(1+2);"
try 1 "2<=3;"
try 1 "2<=2;"
try 0 "3<=2;"
try 1 "3>2;"
try 0 "2>3;"
try 1 "3>=2;"
try 1 "2>=2;"
try 0 "2>=3;"
try 1 "3>2+2==0;"
try 0 "3>1+1==0;"
try 1 "3>1+1==1;"
try 8 "a=2;b=3;c=a*(1+b);c;"
try 20 "a=2;b=3;c=a*(1+(a*b));a+b+c+1;"
try 2 "return 2;"
try 5 "return 5; return 8;"
try 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
try 6 "foo = 1; bar = 2 + 3; return foo + bar;"

echo OK