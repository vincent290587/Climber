#!/bin/bash
cat Climber.log | grep 'DTY' > tmp1 ; sed 's/[^0-9 -]/ /g' tmp1 > tmp2 ; cat header.txt tmp2 > postpro.txt ; echo "Done"