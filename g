make
exit

./add-libs.rb Loopo.jucer
echo ====
sd Loopo.jucer
exit

ls -l ~/.stack/snapshots/x86_64-osx/cf793d5e58850485fdeea13086f3ea038bbe5323ca0ebfe2613e5228635402ab/8.6.5/lib/x86_64-osx-ghc-8.6.5/vector-0.12.0.3-LfvlcMFJAcY18uD1Y2O5Ig/libHSvector-0.12.0.3-LfvlcMFJAcY18uD1Y2O5Ig.a
rm -r unpack0
mkdir unpack0
cd unpack0
ar x ~/.stack/snapshots/x86_64-osx/cf793d5e58850485fdeea13086f3ea038bbe5323ca0ebfe2613e5228635402ab/8.6.5/lib/x86_64-osx-ghc-8.6.5/vector-0.12.0.3-LfvlcMFJAcY18uD1Y2O5Ig/libHSvector-0.12.0.3-LfvlcMFJAcY18uD1Y2O5Ig.a
cd ..
ls -l .stack-work/install/x86_64-osx/ff808cd888128b01de717e3769c1ec59758cf6244e29875e93e1206eaf726a29/8.6.5/lib/x86_64-osx-ghc-8.6.5/loopo-0.1.0.0-IIsHyREJmEr5rZYBnMM7nk/libHSloopo-0.1.0.0-IIsHyREJmEr5rZYBnMM7nk.a
rm -r unpack1
mkdir unpack1
cd unpack1
ar x ~/Loopo/.stack-work/install/x86_64-osx/ff808cd888128b01de717e3769c1ec59758cf6244e29875e93e1206eaf726a29/8.6.5/lib/x86_64-osx-ghc-8.6.5/loopo-0.1.0.0-IIsHyREJmEr5rZYBnMM7nk/libHSloopo-0.1.0.0-IIsHyREJmEr5rZYBnMM7nk.a
cd ..
ar cqs liball.a unpack*/*
exit

make
