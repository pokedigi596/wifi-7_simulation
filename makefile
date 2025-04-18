all:
	g++ -std=c++20 -I ./ns-3-dev/build/include -I ./ns-3-dev/src -I ./ns-3-dev/ -o my_sim main.cpp -L ./ns-3-dev/build/lib \
	-lns3-dev-core-default -lns3-dev-wifi-default -lns3-dev-internet-default -lns3-dev-mobility-default -lns3-dev-applications-default -lns3-dev-network-default
clean:
	rm -rf my_sim