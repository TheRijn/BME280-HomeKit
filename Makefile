
esp32:
	mos build --platform esp32 --local --verbose

clean:
	rm -rf build

.PHONY: clean

moreclean: clean
	rm -rf deps