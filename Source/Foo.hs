module Foo (foo) where

import Data.Word

foreign export ccall foo :: Word32 -> Word32 -> IO Word32

foo :: Word32 -> Word32 -> IO Word32
foo x y = return $ x * y + 1
