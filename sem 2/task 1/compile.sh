if [[ ! -d bin ]]; then
    mkdir bin
fi

fpc -obin/$1 -Sa $1.pas && ./bin/$1
