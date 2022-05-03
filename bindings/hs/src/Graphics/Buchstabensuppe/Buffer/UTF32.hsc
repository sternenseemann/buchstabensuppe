{-# LANGUAGE RecordWildCards #-}
{-# LANGUAGE Unsafe #-}

{-|
Module: Graphics.Buchstabensuppe.Buffer.UTF32
Description: Bindings to buchstabensuppe's UTF32 buffer functions
-}
module Graphics.Buchstabensuppe.Buffer.UTF32
  ( -- * Constructing Buffers
    newBuffer
  , Buffer
    -- * Converting to Buffers
  , fromString
  , fromText
  , fromUtf8
    -- * Querying Buffer Info
  , getCapacity
  , getLength
    -- * Extending Buffers
  , append
  , appendSingle
  , append'
  , appendSingle'
  ) where

#include <buchstabensuppe.h>
#include <stdbool.h>

import Codec.Binary.UTF8.Light ( c2w )
import Control.Monad ( when )
import Data.Word ( Word32 () )
import qualified Data.ByteString as BS
import qualified Data.ByteString.Unsafe as BS
import qualified Data.Text as T
import qualified Data.Text.Encoding as T

import Foreign.C.Error ( throwErrno, throwErrnoIf_
                       , getErrno, eOK
                       , resetErrno
                       )
import Foreign.C.Types ( CSize (..), CBool (..), CChar (..) )
import Foreign.ForeignPtr ( ForeignPtr (), newForeignPtr, withForeignPtr )
import Foreign.Marshal.Alloc ( malloc )
import Foreign.Marshal.Array ( withArrayLen )
import Foreign.Ptr ( Ptr (), FunPtr () )
import Foreign.Storable ( Storable (..) )

-- High-Level Haskell interface

-- | Wrapper around a @bs_utf32_buffer_t@, allocated entirely on the heap.
newtype Buffer = Buffer { unBuffer :: ForeignPtr BufferRaw }

getCapacity :: Buffer -> IO CSize
getCapacity (Buffer buf) = withForeignPtr buf $ fmap bufferRawCapacity . peek

getLength :: Buffer -> IO CSize
getLength (Buffer buf) = withForeignPtr buf $ fmap bufferRawLength . peek

makeBuffer :: Ptr BufferRaw -> IO Buffer
makeBuffer bufStruct =
  Buffer <$> newForeignPtr p_bsw_utf32_buffer_free bufStruct

-- | Create a new 'Buffer' of the specified size. If the specified capacity
--   were to run out, it would be extended automatically.
--   Note that this action never fails. If it fails to allocate the requested
--   memory, a buffer with capacity 0 will be returned.
newBuffer
  :: CSize
  -- ^ Initial storage capacity in number of elements.
  -> IO Buffer
newBuffer initialSize = do
  bufStruct <- malloc
  c_bsw_utf32_buffer_new initialSize bufStruct
  makeBuffer bufStruct

appendSingle :: Buffer -> Char -> IO ()
appendSingle buf = appendSingle' buf . c2w

appendSingle' :: Buffer -> Word32 -> IO ()
appendSingle' (Buffer buf) c = withForeignPtr buf
  $ \raw ->
      throwErrnoIf_
        (not . fromCBool)
        "Graphics.Buchstabensuppe.Buffer.UTF32.appendSingle'"
        $ c_bs_utf32_buffer_append_single c raw

append :: Buffer -> String -> IO ()
append buf = append' buf . map c2w

append' :: Buffer -> [Word32] -> IO ()
append' (Buffer buf) cs = withForeignPtr buf
  $ \raw -> withArrayLen cs
      $ \len arr ->
          throwErrnoIf_
            (not . fromCBool)
            "Graphics.Buchstabensuppe.Buffer.UTF32.append'"
            -- TODO: integer size?
            $ c_bs_utf32_buffer_append arr (fromIntegral len) raw

fromUtf8 :: BS.ByteString -> IO Buffer
fromUtf8 bs = BS.unsafeUseAsCStringLen bs
  $ \(charPtr, len) -> do
      bufStruct <- malloc

      resetErrno -- clear errno before invoking because it won't indicate errors
      -- TODO: integer size?
      c_bsw_decode_utf8 charPtr (fromIntegral len) bufStruct

      errno <- getErrno
      when (errno /= eOK)
        $ throwErrno "Graphics.Buchstabensuppe.Buffer.UTF32.fromUtf8"

      makeBuffer bufStruct

-- TODO: with text 2.0 this should be cheap (could be cheaper ofc),
-- for earlier versions there's maybe a better option
fromText :: T.Text -> IO Buffer
fromText = fromUtf8 . T.encodeUtf8

fromString :: [Char] -> IO Buffer
fromString cs = do
  buf <- newBuffer 0
  buf `append` cs
  pure buf

-- Utils and Types for interfacing with the C code

fromCBool :: CBool -> Bool
fromCBool b = b /= #{const false}

data BufferRaw
  = BufferRaw
  { bufferRawBuffer :: Ptr Word32
  , bufferRawCapacity :: CSize
  , bufferRawLength :: CSize
  }

instance Storable BufferRaw where
  alignment _ = #{alignment bs_utf32_buffer_t}
  sizeOf _ = #{size bs_utf32_buffer_t}
  peek ptr = do
    bufferRawBuffer <- #{peek bs_utf32_buffer_t, bs_utf32_buffer} ptr
    bufferRawCapacity <- #{peek bs_utf32_buffer_t, bs_utf32_buffer_cap} ptr
    bufferRawLength <- #{peek bs_utf32_buffer_t, bs_utf32_buffer_len} ptr
    pure $ BufferRaw {..}
  poke ptr BufferRaw {..} = do
    #{poke bs_utf32_buffer_t, bs_utf32_buffer} ptr bufferRawBuffer
    #{poke bs_utf32_buffer_t, bs_utf32_buffer_cap} ptr bufferRawCapacity
    #{poke bs_utf32_buffer_t, bs_utf32_buffer_len} ptr bufferRawLength

-- FFI

-- Wrapper functions from cbits because Haskell FFI doesn't support returning structs.
foreign import ccall "bsw_utf32_buffer_new"
  c_bsw_utf32_buffer_new :: CSize -> Ptr BufferRaw -> IO ()

foreign import ccall "bsw_decode_utf8"
  c_bsw_decode_utf8 :: Ptr CChar -> CSize -> Ptr BufferRaw -> IO ()

foreign import ccall "&bsw_utf32_buffer_free"
  p_bsw_utf32_buffer_free :: FunPtr (Ptr BufferRaw -> IO ())

-- Direct bindings to buchstabensuppe
foreign import ccall "bs_utf32_buffer_append_single"
  c_bs_utf32_buffer_append_single :: Word32 -> Ptr BufferRaw -> IO CBool

foreign import ccall "bs_utf32_buffer_append"
  c_bs_utf32_buffer_append :: Ptr Word32 -> CSize -> Ptr BufferRaw -> IO CBool
