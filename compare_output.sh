NOTGREP_REGEX="Barry////Adam///|"
GREP_REGEX="(Barry)|(Adam)"

make
NOTGREP_OUTPUT=$(./not-grep "$NOTGREP_REGEX" test_file.txt)
GREP_OUTPUT=$(grep -E "$GREP_REGEX" test_file.txt)

if [[ "$NOTGREP_OUTPUT" == "NFA generation failed; exiting." ]]; then
    echo "NFA generation failed; exiting."
    exit 1
fi

if [[ "$NOTGREP_OUTPUT" == "$GREP_OUTPUT" ]]; then
  echo "Outputs match"
else
  echo "Outputs don't match"
fi