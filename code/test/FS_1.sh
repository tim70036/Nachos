../build.linux/nachos -f
../build.linux/nachos -mkdir /t0
../build.linux/nachos -mkdir /t1
../build.linux/nachos -mkdir /t2
../build.linux/nachos -cp num_100.txt /t2/f1
../build.linux/nachos -mkdir /t0/aa
../build.linux/nachos -mkdir /t0/bb
../build.linux/nachos -mkdir /t0/cc
../build.linux/nachos -lr /
echo "========================================="
../build.linux/nachos -r /t2/f1
../build.linux/nachos -r /t0/aa
../build.linux/nachos -r /t0/bb
../build.linux/nachos -r /t0/cc
../build.linux/nachos -lr /
echo "==================Delete All except t0 , t1 ,t2 ======================="
