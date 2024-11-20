#!/bin/sh
# Ignoring savm16, savm13, goto, usrloc, savm7, savm6, savm5, jmpoff, ise70, savrk, myloc, stack locations
egrep -v '06007:|06013:|06014:|06016:|06017:|06020:|06021:|06022:|06037:|06041:|01014:|2....:' 
