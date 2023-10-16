#!/bin/bash

generate_random_number() {
    echo $((RANDOM % 10))
}

# IP do cliente
IP_ADDRESS="127.0.0.1"
taxa=$1

if ! [[ "$taxa" =~ ^[0-9]+([.][0-9]+)?$ ]] || (( $(echo "$taxa <= 0" | bc -l) )); then
    echo "Por favor, forneça uma taxa válida (número positivo)."
    exit 1
fi

# Operações disponíveis: add, subtract, multiply, divide
operations=("add" "subtract" "multiply" "divide")

# Loop infinito para realizar chamadas ao cliente
while true
do

random_operation=${operations[$((RANDOM % ${#operations[@]}))]}

num1=$(generate_random_number)
num2=$(generate_random_number)

./client $IP_ADDRESS $random_operation $num1 $num2

sleep "$(echo "scale=4; 1/$taxa" | bc -l)"
done