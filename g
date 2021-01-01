#(cabal build) 2>&1 | tee out
#exit

(make) 2>&1 | tee out
make_result=${PIPESTATUS[0]}
echo make_result $make_result
if [ $make_result -ne 0 ]
then
  L out
fi
