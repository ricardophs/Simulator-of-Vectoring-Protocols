#!/bin/bash

# ASES=('Abilene')
# ASES=('AS1221')
# ASES=('BClique4' 'BClique8' 'BClique16' 'BClique32')
# ASES=('AS1221' 'AS1755' 'AS3967' 'AS3257' 'AS6461' 'AS1239')
# ASES=('AS1755' 'AS3967' 'AS3257' 'AS6461' 'AS1239')
# ASES=('AS1239')
# ASES=('Abilene' 'AS1221' 'AS1755' 'AS3967' 'AS3257' 'AS6461' 'AS1239')
# ASES=('Abilene' 'AS1221')
ASES=('Waxman80')

PROTOCOLS=('EIGRP' 'BGP')
# PROTOCOLS=('EIGRP' 'EIGRPFC')
# PROTOCOLS=('EIGRPFC')
# PROTOCOLS=('BGP' 'VP')
# PROTOCOLS=('BGP')
# TYPES=('SYNC' 'Quasi' 'ASYNC')
TYPES=('SYNC' 'Quasi')
# TYPES=('ASYNC')
# TYPES=('SYNC')
# TYPES=('Quasi' 'ASYNC')
# TYPES=('Quasi')
METRICS=('HC' 'WSP' 'SP' 'EM' 'SWP')
# METRICS=('EM' 'SWP')
# TESTS=('-adv' '-w' '-lf' '-lf2')
# TESTS=('-adv' '-w' '-lf')
TESTS=('-adv' '-lf' '-lf2')
# TESTS=('-lf' '-w' '-lf2')
# TESTS=('-lf')
# TESTS=('-adv')
# TESTS=('-lf2')


# bash createDirs.sh

echo "" > outWaxman.txt

for protocol in "${PROTOCOLS[@]}"; do
    echo "$protocol"
    for tp in "${TYPES[@]}"; do
        if [ $protocol == 'EIGRP' ] && [ $tp == 'Quasi' ]; then
            "${tp}" = 'ASYNC'
        fi
        # if [ $protocol == 'VP' ] && [ $tp != 'ASYNC' ]; then
        #     continue
        # fi
        echo -e "\t$tp"
        for as in "${ASES[@]}"; do
            echo -e "\t\t$as ---"
            for metric in "${METRICS[@]}"; do
                echo -e "\t\t\t$metric"
                for test in "${TESTS[@]}"; do
                    # if [[$protocol == 'VP' && $test != '-adv' && $test != '-lf']]; then
                    #     continue
                    # fi
                    # if [[ $protocol == 'BGP' && $tp == 'ASYNC' && $metric != 'HC' && $test != '-adv' && $test != '-lf2' ]]; then
                    #     continue
                    # fi
                    # if [[ $as == 'AS1755' && $metric != 'SWP' ]]; then
                    #     continue
                    # fi
                    echo -e "\t\t\t\t$test"
                    time ./main $as -p $protocol -m $metric -t $tp $test >> outWaxman.txt # > /dev/null
                    echo ""
                done
            done
        done
    done
    echo ""
done