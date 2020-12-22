module Foo (foo) where

import Data.Word
import Foreign.C.Types
import Foreign.Ptr
import Foreign.Storable

foreign export ccall foo :: Word32 -> Word32 -> IO Word32

foo :: Word32 -> Word32 -> IO Word32
foo x y = return $ x * y + 1

foreign export ccall hs_frobb :: Ptr Float -> CInt -> IO ()

-- pointer pokey
--hs_frobb :: Ptr Float -> CInt -> IO ()
hs_frobb ptr len = do
  f0 <- peekElemOff ptr 0
  fn <- peekElemOff ptr last
  pokeElemOff ptr 0 (g f0)
  pokeElemOff ptr last (g fn)
  where g x = x * 2 + 0.003
        last :: Int
        last = (fromIntegral len) - 1

-- veccy
-- hs_frobb ptr len = do
