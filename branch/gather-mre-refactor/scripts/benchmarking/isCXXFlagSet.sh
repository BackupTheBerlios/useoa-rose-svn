function isCXXFlagSet {
    cat $1 | grep "CXXFLAGS"
}

isCXXFlagSet $1 $2
return $?
