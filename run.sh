#!/bin/bash
path_instances=$1
seed=$2
lowerboundid=$3
max_time=$4

echo Instancia,Nos_Explorados,Cortes_Realizados,Nos_Ignorados,Lower_Bound,Seed,N_Bins,Tempo_Nanosegundos,Tempo_Segundos,Tempo_Minutos

files=$(ls $path_instances)
for filename in $files
do
    ./bin/driver $path_instances$filename $seed $lowerboundid $max_time 6 > ./results/$filename.txt
    bins=$(sed '1!d' ./results/$filename.txt)
    nanosegundos=$(sed '2!d' ./results/$filename.txt)
    segundos=$(sed '3!d' ./results/$filename.txt)
    minutos=$(sed '4!d' ./results/$filename.txt)
    nos=$(sed '5!d' ./results/$filename.txt)
    cortes=$(sed '6!d' ./results/$filename.txt)
    ignorados=$(sed '7!d' ./results/$filename.txt)
    nos="${nos##* }"
    cortes="${cortes##* }"
    ignorados="${ignorados##* }"
    bins="${bins##* }"
    nanosegundos="${nanosegundos##* }"
    segundos="${segundos##* }"
    minutos="${minutos##* }"
    echo $filename,$nos,$cortes,$ignorados,$lowerboundid,$seed,$bins,$nanosegundos,$segundos,$minutos
done
