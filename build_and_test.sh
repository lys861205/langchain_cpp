#!/bin/bash

# Build and test script for LangChain C++ API Models

echo "Building LangChain C++ with API Model support..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DBUILD_API_MODELS=ON

# Build the project
make -j4

if [ $? -eq 0 ]; then
    echo "Build successful!"

    # Run the API model example
    echo "Running API Model Example..."
    if [ -f "api_model_example" ]; then
        ./api_model_example
    else
        echo "API Model Example not found"
    fi

    # Run the comprehensive API test
    echo "Running Comprehensive API Test..."
    if [ -f "comprehensive_api_test" ]; then
        ./comprehensive_api_test
    else
        echo "Comprehensive API Test not found"
    fi

else
    echo "Build failed!"
    exit 1
fi