# make update-proj
# exit

#(cabal configure && cabal build) 2>&1 | tee out
(make) 2>&1 | tee out
make_result=${PIPESTATUS[0]}
echo make_result $make_result
if [ $make_result -ne 0 ]
then
  L out
fi
