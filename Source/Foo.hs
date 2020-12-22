module Foo (foo) where

import Data.Word
import Foreign.Ptr
import Foreign.Storable

foreign export ccall foo :: Word32 -> Word32 -> IO Word32

foo :: Word32 -> Word32 -> IO Word32
foo x y = return $ x * y + 1

foreign export ccall hs_frobb :: Ptr Float -> IO ()

hs_frobb ptr = do
  f <- peek ptr
  pokeElemOff ptr 0 0.7 -- (f * 2 + 0.3)
  pokeElemOff ptr 1 0.8 -- (f * 3 + 0.2)
