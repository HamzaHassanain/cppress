#!/bin/bash

# Exit on any error
set -e

# Ensure we're in the project root directory
cd "$(dirname "$0")"

# ==================== HELP ====================
if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "Usage: ./scripts.sh [COMMAND] [OPTIONS]"
    echo ""
    echo "Build Commands:"
    echo "  clean              - Remove build directory"
    echo "  build [TYPE]       - Build the project (default) [debug|release|relwithdebinfo]"
    echo "  rebuild            - Clean and build in one step"
    echo "  configure [OPTS]   - Configure CMake with custom options"
    echo "  install [PREFIX]   - Install the project (default: /usr/local)"
    echo ""
    echo "Test Commands:"
    echo "  test [PATTERN]     - Run tests (optional: filter by pattern) or 'small-only' for small tests"
    echo "  test-lib LIB       - Test specific library (json|sockets|html|http|web)"
    echo "  test-verbose       - Run tests with verbose output"
    echo "  test-integration   - Run only integration tests"
    echo ""
    echo "Code Quality:"
    echo "  format             - Format all source files with clang-format"
    echo "  format-check       - Check formatting without modifying files"
    echo "  lint               - Run clang-tidy linter"
    echo "  sanitize [TYPE]    - Build with sanitizer (address|thread|undefined|memory|leak)"
    echo ""
    echo "Utility Commands:"
    echo "  run                - Run the main executable"
    echo "  libs               - List all available libraries"
    echo "  help               - Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./scripts.sh clean build release"
    echo "  ./scripts.sh test-lib json"
    echo "  ./scripts.sh build debug"
    echo "  ./scripts.sh configure -DBUILD_TESTS=OFF"
    echo "  ./scripts.sh install ~/.local"
    echo "  ./scripts.sh sanitize address"
    echo "  ./scripts.sh format"
    exit 0
fi


# ==================== CLEAN ====================
if [ "$1" = "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf build
    shift
fi


# ==================== CONFIGURE ====================
if [ "$1" = "configure" ]; then
    echo "Configuring CMake with custom options..."
    shift
    
    mkdir -p build
    
    echo "Running: cmake -S . -B build $@"
    cmake -S . -B build "$@"
    
    echo "Configuration completed!"
    exit 0
fi

# ==================== REBUILD ====================
if [ "$1" = "rebuild" ]; then
    echo "Cleaning and rebuilding..."
    rm -rf build
    shift
    # Continue to build with remaining args
    set -- "build" "$@"
fi

# ==================== NORMAL BUILD ====================
if [ "$1" = "build" ] || [ -z "$1" ]; then
    BUILD_TYPE=${2:-Release}
    
    # Normalize build type
    case ${BUILD_TYPE,,} in
        debug|d)
            BUILD_TYPE="Debug"
        ;;
        release|rel|r)
            BUILD_TYPE="Release"
        ;;
        relwithdebinfo|relwithdeb|rwd)
            BUILD_TYPE="RelWithDebInfo"
        ;;
        minsizerel|minsize)
            BUILD_TYPE="MinSizeRel"
        ;;
        *)
            # If second arg doesn't match build type, assume it's default Release
            if [ -n "$2" ]; then
                echo "Unknown build type: $2, using Release"
            fi
            BUILD_TYPE="Release"
        ;;
    esac
    
    echo "Building project (${BUILD_TYPE})..."
    
    mkdir -p build
    
    # Configure the project with CMake
    echo "Configuring CMake..."
    cmake -S . -B build -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
    
    # Build the project
    echo "Building project..."
    cd build
    make -j$(nproc)
    cd ..
    
    echo "✅ Build completed successfully! (${BUILD_TYPE})"
    shift
    [ -n "$2" ] && shift  # shift build type if provided
fi


# ==================== FORMAT ====================
if [ "$1" = "format" ]; then
    echo "Formatting code with clang-format (Google style)..."
    find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.ipp" \) \
    ! -path "./out/*" ! -path "./build/*" \
    -exec clang-format -i -style=file {} +
    echo "Formatting completed!"
    exit 0
fi

if [ "$1" = "format-check" ]; then
    echo "Checking code formatting..."
    NEEDS_FORMAT=$(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.ipp" \) \
        ! -path "./out/*" ! -path "./build/*" \
    -exec clang-format -style=file --dry-run --Werror {} + 2>&1 || true)
    
    if [ -n "$NEEDS_FORMAT" ]; then
        echo "❌ Some files need formatting:"
        echo "$NEEDS_FORMAT"
        echo ""
        echo "Run './scripts.sh format' to fix formatting issues."
        exit 1
    else
        echo "✅ All files are properly formatted!"
        exit 0
    fi
fi

# ==================== LINT ====================
if [ "$1" = "lint" ]; then
    echo "Running clang-tidy linter..."
    
    # Build with compile_commands.json first
    mkdir -p build
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    # Run clang-tidy on source files, header files, implementation files
    find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.ipp" \) \
    ! -path "./out/*" ! -path "./build/*" ! -path "./tests/*" \
    -exec clang-tidy -p build {} \;
    
    echo "Linting completed!"
    exit 0
fi

# ==================== SANITIZER BUILD ====================
if [ "$1" = "sanitize" ]; then
    SANITIZER_TYPE=${2:-address}
    echo "Building with $SANITIZER_TYPE sanitizer..."
    
    mkdir -p build
    cmake -S . -B build -DSANITIZER=$SANITIZER_TYPE
    cd build
    make -j$(nproc)
    cd ..
    
    echo "Build with sanitizer completed!"
    echo "Run tests with: cd build && ctest --output-on-failure"
    exit 0
fi

# ==================== INSTALL ====================
if [ "$1" = "install" ]; then
    INSTALL_PREFIX=${2:-/usr/local}
    
    echo "Installing to: $INSTALL_PREFIX"
    
    if [ ! -d "build" ]; then
        echo "Build directory not found. Building first..."
        mkdir -p build
        cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
        cd build
        make -j$(nproc)
        cd ..
    fi
    
    cd build
    
    # Check if we need sudo
    if [ "$INSTALL_PREFIX" = "/usr/local" ] || [ "$INSTALL_PREFIX" = "/usr" ]; then
        echo "System installation requires sudo..."
        sudo make install
    else
        make install
    fi
    
    cd ..
    
    echo "✅ Installation completed to: $INSTALL_PREFIX"
    echo ""
    echo "To use cppress in other projects:"
    if [ "$INSTALL_PREFIX" != "/usr/local" ] && [ "$INSTALL_PREFIX" != "/usr" ]; then
        echo "  cmake -Dcppress_DIR=$INSTALL_PREFIX/lib/cmake/cppress .."
    else
        echo "  find_package(cppress REQUIRED)"
    fi
    exit 0
fi

# ==================== LIST LIBRARIES ====================
if [ "$1" = "libs" ]; then
    echo "Available Libraries:"
    echo ""
    echo "  📦 json        - JSON parser library"
    echo "  📦 sockets     - Socket programming library"
    echo "  📦 html        - HTML builder library"
    echo "  📦 http        - HTTP server library"
    echo "  📦 web         - Web framework (combines all)"
    echo ""
    echo "Test individual library with: ./scripts.sh test-lib <name>"
    exit 0
fi

# ==================== TEST ====================
if [ "$1" = "test" ]; then
    echo "Running tests..."
    
    if [ ! -d "build" ]; then
        echo "Build directory not found. Building first..."
        cmake -S . -B build -DBUILD_TESTS=ON
        cd build
        make -j$(nproc)
        cd ..
    fi
    
    cd build
    if [ -n "$2" ]; then
        if [ "$2" = "small-only" ]; then
            ctest -R "SMALL_" --output-on-failure
        else
            ctest -R "$2" --output-on-failure
        fi
    else
        ctest --output-on-failure
    fi
    EXIT_CODE=$?
    cd ..
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo "✅ All tests passed!"
    else
        echo "❌ Some tests failed!"
    fi
    exit $EXIT_CODE
fi

# ==================== TEST LIBRARY ====================
if [ "$1" = "test-lib" ]; then
    if [ -z "$2" ]; then
        echo "❌ Error: Please specify a library to test"
        echo "Usage: ./scripts.sh test-lib <library>"
        echo "Available: json, sockets, html, http, web"
        exit 1
    fi
    
    LIB_NAME=$2
    echo "Testing library: $LIB_NAME"
    
    # Check if library directory exists
    if [ ! -d "libs/$LIB_NAME" ]; then
        echo "❌ Error: Library '$LIB_NAME' not found in libs/"
        exit 1
    fi
    
    # Build the library tests
    if [ ! -d "build" ]; then
        echo "Building project with tests..."
        cmake -S . -B build -DBUILD_TESTS=ON
        cd build
        make -j$(nproc)
        cd ..
    fi
    
    cd build
    # Run tests labeled with the library name
    ./libs/$LIB_NAME/tests/$LIB_NAME-tests
    
    EXIT_CODE=$?
    cd ..
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo "✅ All $LIB_NAME tests passed!"
    else
        echo "❌ Some $LIB_NAME tests failed!"
    fi
    exit $EXIT_CODE
fi

# ==================== TEST VERBOSE ====================
if [ "$1" = "test-verbose" ]; then
    echo "Running tests with verbose output..."
    
    if [ ! -d "build" ]; then
        echo "Building project with tests..."
        cmake -S . -B build -DBUILD_TESTS=ON
        cd build
        make -j$(nproc)
        cd ..
    fi
    
    cd build
    ctest --verbose --output-on-failure
    EXIT_CODE=$?
    cd ..
    exit $EXIT_CODE
fi

# ==================== TEST INTEGRATION ====================
if [ "$1" = "test-integration" ]; then
    echo "Running integration tests..."
    
    if [ ! -d "build" ]; then
        echo "Building project with tests..."
        cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_INTEGRATION_TESTS=ON
        cd build
        make -j$(nproc)
        cd ..
    fi
    
    cd build
    ctest -R "integration" --output-on-failure
    EXIT_CODE=$?
    cd ..
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo "✅ All integration tests passed!"
    else
        echo "❌ Some integration tests failed!"
    fi
    exit $EXIT_CODE
fi
