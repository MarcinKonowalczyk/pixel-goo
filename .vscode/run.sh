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

# Debug print
echo "EXTENSION : ${EXTENSION}"
echo "FILENAME : ${FILENAME}"
echo "DIR : ${DIR}"
echo "ROOT : ${ROOT}"

# Do some other stuff here. In this case run the makefile if
# you're currently editing the makefile
# if [ "$FILENAME" = "makefile" ]; then
#     echo "About to run the makefile!";
#     make;
# fi

make --directory="$ROOT/build/"

# exit 0

OUT=$?
if [ $OUT == 0 ]; then
    echo "bash: Running goddamnit..."
    "$ROOT/build/goddamnit";
else
    echo "bash: Compilation failed";
    exit $OUT
fi


# make;