#!/bin/bash
echo "------------- make de tpc --------------"
make

for rep in `ls ./tests`
	do
		echo "---------- execution des tests dans ../$rep --------- "
		for fichier in `ls ./tests/$rep`;do

			v=$fichier
			v2=${v::-4}
			v2+=".asm"
			#echo $v2
			echo "---------- tests dans ../$rep/$fichier --------- "
			./bin/tpcc -t -s $v2 < ./tests/$rep/$fichier
			let "res = $?"
			echo "\$? = " $res
			if [ $res = 0 ]
			then
				echo "Programme correct !"

			else
				echo "Le programme n'a pas pu compiler"
			fi
	done
done