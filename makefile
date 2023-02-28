run: compile
	./main

runopt: compile
	./main -optimize
compile:
	g++ main.cpp -o main
	g++ producer.cpp -o producer
	g++ consumer.cpp -o consumer

clean:
	rm -f output_*.txt Adj_list.txt
	rm main consumer producer

stop:
	killall -9 ./consumer
	killall -9 ./producer