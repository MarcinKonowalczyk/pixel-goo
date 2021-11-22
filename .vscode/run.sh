# https://gist.github.com/MarcinKonowalczyk/709e93f08e9d72f8092acd5b8d34c81f
# Example .run.sh
echo "Hello from run script! ^_^"

# Extension, filename and directory parts of the file which triggered this
EXTENSION="${1##*.}"
FILENAME=$(basename -- "$1")
DIR="${1%/*}/"

# The direcotry of the file folder from which this script is running
# https://stackoverflow.com/a/246128/2531987
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ROOT="${ROOT%/*}/"

# VERBOSE=true
VERBOSE=false
if $VERBOSE; then
    echo "EXTENSION : ${EXTENSION}"
    echo "FILENAME : ${FILENAME}"
    echo "DIR : ${DIR}"
    echo "ROOT : ${ROOT}"
fi

if [ "$FILENAME" == "perlin_test.cpp" ]; then
    (
        cd "$DIR"
        g++ -I "../lib/glm/" -Wall perlin_test.cpp -o perlin_test && ./perlin_test
    )
else
    TARGET="goo"
    make --directory="$ROOT/build/"

    OUT=$?
    if [ $OUT == 0 ]; then
        echo "bash: Running $TARGET..."
        (
            cd "$ROOT/build/";
            "$ROOT/build/$TARGET";
        )
    else
        echo "bash: Compilation failed";
        exit $OUT
    fi
fi