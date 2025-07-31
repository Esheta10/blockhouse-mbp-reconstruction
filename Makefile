TARGET = reconstruction
SRC = reconstruction.cpp

$(TARGET): $(SRC)
	g++ -std=c++17 -O2 -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o output_mbp.csv