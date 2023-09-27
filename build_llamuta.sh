
#!/bin/sh
LIBFUZZER_SRC_DIR=$(dirname $0)
for f in $LIBFUZZER_SRC_DIR/*.cpp; do
  clang++ -O2 -fPIC -std=c++11 $f -c &
done
wait
ar r libFuzzer.a *.o 
