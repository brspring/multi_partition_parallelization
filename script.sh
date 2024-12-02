#!/bin/bash

for num in {1..8}; do
    echo "Executando ./mpp $num"
    ./mpp $num
done