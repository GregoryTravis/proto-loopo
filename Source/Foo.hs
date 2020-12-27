module Foo (foo) where

import Data.Word
import qualified Data.Vector.Storable.Mutable as MV
import Foreign.C.Types
import Foreign.Ptr
import Foreign.ForeignPtr
import Foreign.Storable
import System.IO (stdout, stderr, hSetBuffering, BufferMode(..))

foreign export ccall foo :: Word32 -> Word32 -> IO Word32

foo :: Word32 -> Word32 -> IO Word32
foo x y = return $ x * y + 1

foreign export ccall hs_frobb :: Ptr Float -> CInt -> IO ()

-- pointer pokey
--hs_frobb :: Ptr Float -> CInt -> IO ()
_hs_frobb ptr len = do
  f0 <- peekElemOff ptr 0
  fn <- peekElemOff ptr last
  pokeElemOff ptr 0 (g f0)
  pokeElemOff ptr last (g fn)
  where g x = x * 2 + 0.003
        last :: Int
        last = (fromIntegral len) - 1

-- veccy
hs_frobb ptr len = do
  fptr <- newForeignPtr_ ptr
  let mv = MV.unsafeFromForeignPtr fptr 0 (fromIntegral len)
  f0 <- MV.read mv 0
  fn <- MV.read mv last
  MV.write mv 0 (g f0)
  MV.write mv last (g fn)
  where g x = x * 3 + 0.003
        last :: Int
        last = (fromIntegral len) - 1

data CMidi = CMidi CBool CInt deriving Show

instance Storable CMidi where
  sizeOf _ = 8
  alignment = sizeOf
  peek ptr = do
    b <- peekByteOff ptr 0
    nn <- peekByteOff ptr 4
    return $ CMidi b nn
  poke ptr (CMidi isOn nn) = do
    pokeByteOff ptr 0 isOn
    pokeByteOff ptr 4 nn

foreign export ccall hs_frobb_midi :: Ptr CMidi -> CInt -> IO ()

hs_frobb_midi ptr len = do
  hSetBuffering stdout NoBuffering
  hSetBuffering stderr NoBuffering
  --putStrLn ("LEN " ++ show len)
  fptr <- newForeignPtr_ ptr
  let mv = MV.unsafeFromForeignPtr fptr 0 (fromIntegral len)
  if len == 0
    then return () -- putStrLn "Empty"
    else do
      old@(CMidi isOn noteNumber) <- MV.read mv 0
      let nu = CMidi isOn (noteNumber + 1) 
      MV.write mv 0 nu
      putStrLn ("wrote " ++ show old ++ " " ++ show nu)
      return ()
