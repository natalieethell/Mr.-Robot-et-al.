# Based on example Makefile:
# https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
MAKEOPTS = "-j 2"

CXX = g++
SRC_FLAGS = -std=c++0x -I nginx-configparser/ -Wall -Wextra -Werror
LDFLAGS = -lpthread -lboost_system

PARSER_PATH = ./nginx-configparser/
GTEST_DIR = nginx-configparser/googletest/googletest
GTEST_FLAGS = -isystem ${GTEST_DIR}/include

# TODO: Split out dependency handling so that we can avoid rebuilding
.PHONY: $(TARGET) $(TESTS) all test clean

TARGET = lightning
TESTS = lightning_server_test
SRC = $(PARSER_PATH)config_parser.cc lightning_main.cc lightning_server.cc server_config.cc request_handlers.cc

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(SRC_FLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

# TODO: Generalize for multiple unit tests
# TODO: These assume compilation on LINUX, so we need to add an OS check
$(TESTS):
	# Build and link GTest
	$(CXX) $(SRC_FLAGS) $(GTEST_FLAGS) -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	# Build our own tests, using GTest
	# TODO: Figure out how to name multiple CC and produce corresponding object for each
	$(CXX) $(SRC_FLAGS) $(GTEST_FLAGS) $(TESTS).cc ${GTEST_DIR}/src/gtest_main.cc libgtest.a $(LDFLAGS) -o $(TESTS)

# TODO: Check if lighting
test: $(TARGET) $(TESTS)
	./$(TESTS)
	./$(TARGET) simple_config &
	python3 lightning_integration_test.py

clean:
	$(RM) $(TARGET) $(TESTS) *.o *.a
