CC          := g++
CC_FLAGS    := -Wall -Wextra -pedantic -std=c++20

BUILD_DIR   := build
SOURCES      = $(wildcard UwU_src/*.cpp)
OBJECTS      = $(SOURCES:UwU_src/%.cpp=$(BUILD_DIR)/%.o)

.PHONY : all
all : uwu

uwu : $(OBJECTS) | $(BUILD_DIR)
	$(CC) $^ -o $@
	
ifneq ($(MAKECMDGOALS), clean)
-include $(OBJECTS:.o=.d)
endif

$(BUILD_DIR)/%.o : UwU_src/%.cpp | $(BUILD_DIR)
	$(CC) -c $(CC_FLAGS) -MP -MMD $< -o $@
	
$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)
	
.PHONY : clean
clean :
	rm -f uwu $(OBJECTS) $(OBJECTS:.o=.d)
