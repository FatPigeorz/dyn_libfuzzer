
#!/bin/sh
LIBFUZZER_SRC_DIR=$(dirname $0)
for f in $LIBFUZZER_SRC_DIR/*.cpp; do
  clang++ -O2 -fPIC -std=c++14 $f -c &
done
wait
rm -f libFuzzer.a
ar r libFuzzer.a Fuzzer*.o
rm -f Fuzzer*.o
