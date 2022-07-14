# compile.sh
#
# MIT License
#
# Copyright (C) 2022 Stefanos "Steven" Tsakiris
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#!/bin/sh

DEBUG=false                                           # whether to keep asm files
                                                      #
compileFile(){                                        #
    gcc -x c -S -std=c89 -Os -D_POSIX_C_SOURCE=199309L -lxcb -lxcb-randr -o "$asm/$1.s" "source/$1.c"                           -Waddress -Warray-bounds=1 -Wbool-compare -Wbool-operation -Wchar-subscripts -Wduplicate-decl-specifier -Wformat -Wformat-overflow -Wformat-truncation -Wint-in-bool-context -Wimplicit -Wimplicit-int -Wimplicit-function-declaration -Winit-self -Wlogical-not-parentheses -Wmain -Wmaybe-uninitialized -Wmemset-elt-size -Wmemset-transposed-args -Wmisleading-indentation -Wmissing-attributes -Wmissing-braces -Wmultistatement-macros -Wnarrowing -Wnonnull -Wnonnull-compare -Wopenmp-simd -Wparentheses -Wpointer-sign -Wrestrict -Wreturn-type -Wsequence-point -Wsign-compare -Wsizeof-pointer-div -Wsizeof-pointer-memaccess -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wvolatile-register-var -Wclobbered -Wcast-function-type -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough=3 -Wmissing-field-initializers -Wmissing-parameter-type -Wold-style-declaration -Woverride-init -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value -Wunused-parameter -Wunused-but-set-parameter -Wpedantic
    files="$files $asm/$1.s"                          # add compiled file to list
}                                                     #
                                                      #
asm="output/asm"                                      # path to asm files
files=""                                              # list of files that form the executable
cd $(dirname "$0")                                    # open location of this script
if [ ! -f compile.sh ]; then                          # if this script is not found in the location provided
    printf "could not find compile script location\n" #
    exit 1                                            #
fi                                                    #
rm -r output > /dev/null 2>&1                         # remove directory
mkdir output                                          # create output directory
mkdir "$asm"                                          # create asm directory
if [ ! -d "$asm" ]; then                              # if directory is not in location
    printf "could not create file structure\n"        #
    exit 1                                            #
fi                                                    #
compileFile "mine"                                    # compile individual file
gcc $files -o output/mine                             # create executable
if ! "$DEBUG"; then                                   # if debug mode
    rm -r "$asm"                                      # remove asm files
fi                                                    #
exit 0                                                #
