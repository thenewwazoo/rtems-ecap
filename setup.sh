#!/bin/bash

mkdir -p lib
git -C lib clone https://github.com/thenewwazoo/markov-localizer.git
cp Makefile.libmarkov lib/markov-localizer/c99_fp/Makefile
