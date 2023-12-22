make all
rm gateway.log
rm data.csv
port=5678
clients=40
echo -e "starting gateway "
valgrind --leak-check=summary ./sensor_gateway $port $clients &
sleep 3
echo -e 'starting sensor nodes'
./sensor_node 15 1 127.0.0.1 $port &

./sensor_node 21 3 127.0.0.1 $port &

./sensor_node 37 2 127.0.0.1 $port &

./sensor_node 132 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 1 3 127.0.0.1 $port &

sleep 1

./sensor_node 15 1 127.0.0.1 $port &

./sensor_node 21 3 127.0.0.1 $port &

./sensor_node 37 2 127.0.0.1 $port &

./sensor_node 132 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 1 3 127.0.0.1 $port &

sleep 1

./sensor_node 15 1 127.0.0.1 $port &

./sensor_node 21 3 127.0.0.1 $port &

./sensor_node 37 2 127.0.0.1 $port &

./sensor_node 132 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 1 3 127.0.0.1 $port &

sleep 1

./sensor_node 15 1 127.0.0.1 $port &

./sensor_node 21 3 127.0.0.1 $port &

./sensor_node 37 2 127.0.0.1 $port &

./sensor_node 132 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 1 3 127.0.0.1 $port &

./sensor_node 15 1 127.0.0.1 $port &

./sensor_node 21 3 127.0.0.1 $port &

./sensor_node 37 2 127.0.0.1 $port &

./sensor_node 132 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 142 3 127.0.0.1 $port &

./sensor_node 1 3 127.0.0.1 $port &

./sensor_node 15 1 127.0.0.1 $port &

./sensor_node 21 3 127.0.0.1 $port &

./sensor_node 37 2 127.0.0.1 $port &

./sensor_node 132 3 127.0.0.1 $port &

sleep 11
killall sensor_node

echo "Results for regular nodes"
cat gateway.log | grep parsed | wc -l && cat gateway.log | grep insertion | wc -l && cat data.csv | wc -l

./sensor_node 2 8 127.0.0.1 $port 

cat gateway.log | grep parsed | wc -l && cat gateway.log | grep insertion | wc -l && cat gateway.log | grep timed | wc -l && cat gateway.log | grep invalid | wc -l && cat data.csv | wc -l 

sleep 3
killall sensor_gateway
