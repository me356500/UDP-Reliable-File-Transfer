
all:
	g++ -O3 -o receiver_select receiver_select.cpp
	g++ -std=c++17 -O3 -o sender_select sender_select.cpp -lstdc++fs
.PHONY: clean
clean:
	rm -rf receiver_select sender_select
