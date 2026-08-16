CXX = g++
CXXFLAGS = -std=c++17 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Static build flags (self-contained, zero Homebrew dependency)
STATIC_DEPS = $(CURDIR)/deps/sfml
STATIC_CXXFLAGS = -std=c++17 -DSFML_STATIC -I$(STATIC_DEPS)/include
STATIC_LDFLAGS = -Wl,-force_load,$(STATIC_DEPS)/lib/libsfml-window-s.a \
                 -L$(STATIC_DEPS)/lib \
                 -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s \
                 -lfreetype -lFLAC -lvorbisenc -lvorbisfile -lvorbis -logg \
                 -framework Foundation -framework AppKit -framework IOKit \
                 -framework Carbon -framework AudioToolbox -framework CoreAudio -framework OpenGL

TARGET = test
STATIC_TARGET = test_static
APP_BUNDLE = MTG.app
SRC = test.cc

.PHONY: all static bundle run run-static run-bundle clean

all: $(TARGET)

# Dynamic build
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Static build (single portable standalone binary)
static: $(STATIC_TARGET)

$(STATIC_TARGET): $(SRC)
	$(CXX) $(STATIC_CXXFLAGS) $(SRC) -o $(STATIC_TARGET) $(STATIC_LDFLAGS)

# Standalone macOS .app Bundle
bundle: $(STATIC_TARGET)
	@echo "Creating macOS App Bundle: $(APP_BUNDLE)..."
	@mkdir -p $(APP_BUNDLE)/Contents/MacOS
	@mkdir -p $(APP_BUNDLE)/Contents/Resources
	@cp $(STATIC_TARGET) $(APP_BUNDLE)/Contents/MacOS/MTG
	@chmod +x $(APP_BUNDLE)/Contents/MacOS/MTG
	@printf '%s\n' \
		'<?xml version="1.0" encoding="UTF-8"?>' \
		'<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' \
		'<plist version="1.0">' \
		'<dict>' \
		'    <key>CFBundleExecutable</key>' \
		'    <string>MTG</string>' \
		'    <key>CFBundleIdentifier</key>' \
		'    <string>com.mtg.app</string>' \
		'    <key>CFBundleName</key>' \
		'    <string>MTG</string>' \
		'    <key>CFBundlePackageType</key>' \
		'    <string>APPL</string>' \
		'    <key>CFBundleShortVersionString</key>' \
		'    <string>1.0</string>' \
		'    <key>LSMinimumSystemVersion</key>' \
		'    <string>11.0</string>' \
		'    <key>NSHighResolutionCapable</key>' \
		'    <true/>' \
		'</dict>' \
		'</plist>' > $(APP_BUNDLE)/Contents/Info.plist
	@echo "App Bundle created successfully at $(CURDIR)/$(APP_BUNDLE)"

run: $(TARGET)
	./$(TARGET)

run-static: $(STATIC_TARGET)
	./$(STATIC_TARGET)

run-bundle: bundle
	open $(APP_BUNDLE)

clean:
	rm -rf $(TARGET) $(STATIC_TARGET) $(APP_BUNDLE)
