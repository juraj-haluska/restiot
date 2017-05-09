#!/bin/bash

FILE=${1}
NEW="${FILE%.*}.h"
touch ${NEW}

echo "const char ${2} [] =" > ${NEW}

while IFS='' read -r line || [[ -n "$line" ]]; do
    echo "    \"$line\\n\"" >> ${NEW}
done < "${FILE}"

echo "$(cat ${NEW});" > ${NEW}
