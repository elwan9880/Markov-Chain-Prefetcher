#!/bin/bash
#
#  A shell script for running cacheSim and calculating average AMAT improvement automatically.
#

make clean 
make

avg_amat=0
avg_amat_impr=0
files=`ls ./traces/ | sed "s/.trace//g"`

echo "------------------------------------------------------"
echo "traces		baseline AMAT	optimized AMAT"
echo "------------------------------------------------------"

for i in ${files}; do
	./cacheSim ./traces/${i}.trace
	mv ./traces/${i}.trace.out ./optimized/
	opt_amat=`cat ./optimized/${i}.trace.out | head -n 4 | tail -n 1`
	bl_amat=`cat ./baseline/${i}.trace.out | head -n 4 | tail -n 1`
	echo "${i}		${bl_amat}		${opt_amat}"
	avg_amat=`echo "${avg_amat} + ${opt_amat}" | bc`
	avg_amat_impr=`echo "${avg_amat_impr} + ${bl_amat} - ${opt_amat}" | bc`
done 

n_traces=`ls -1 ./traces/* | wc -l`
avg_amat=`echo "${avg_amat} / (${n_traces}.0)" | bc -l`
avg_amat_impr=`echo "${avg_amat_impr} / (${n_traces}.0)" | bc -l`

echo "------------------------------------------------------"
echo "Average AMAT: ${avg_amat}"
echo "Average AMAT Improvement: ${avg_amat_impr}"
echo "------------------------------------------------------"
