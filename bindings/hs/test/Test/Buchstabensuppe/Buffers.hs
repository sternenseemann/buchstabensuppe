module Test.Buchstabensuppe.Buffers (buffers) where

import qualified Graphics.Buchstabensuppe.Buffer.UTF32 as Buf

import Control.Exception (catch, throwIO)
import Control.Monad (when)
import Control.Monad.IO.Class
import qualified Data.ByteString as BS
import qualified Data.Text as T
import qualified Data.Text.Encoding as T
import Foreign.C.Error (errnoToIOError, eINVAL)
import Test.Tasty
import Test.Tasty.QuickCheck
import Test.QuickCheck.Monadic

-- Note these properties all assume all allocations will succeed.
buffers :: TestTree
buffers = testGroup "Test buchstabensuppe buffer bindings"
  [ testProperty "Buffer capacity and length are correct after creation" prop_bufferCreate
  , testProperty "Buffer capacity and length are correct after appendSingle" prop_bufferAppendSingle
  , testProperty "Buffer capacity and length are correct after append" prop_bufferAppend
  , testProperty "Appending an empty string to a buffer does nothing" prop_bufferAppendEmpty
  , testProperty "Decoding an UTF-8 ByteString returns the correct length" prop_bufferDecodeUtf8
  , testProperty "Decoding a non-UTF-8 ByteString fails" prop_bufferDecodeUtf8Failures
  , testProperty "Converting from [Char] returns the correct length" prop_bufferFromString
  , testProperty "Converting from Text returns the correct length" prop_bufferFromText
  ]

prop_bufferCreate :: Property
prop_bufferCreate = monadicIO $ do
  size <- pick $ choose (0, 16384)
  buf <- liftIO $ Buf.newBuffer size
  cap <- liftIO $ Buf.getCapacity buf
  len <- liftIO $ Buf.getLength buf
  monitorBuffer buf
  pure $ len == 0 && size == cap

prop_bufferAppendSingle :: Property
prop_bufferAppendSingle = monadicIO $ do
  buf <- arbitraryBuffer

  len <- liftIO $ Buf.getLength buf
  cap <- liftIO $ Buf.getCapacity buf

  monitorBuffer buf

  char <- pick arbitrary
  liftIO $ buf `Buf.appendSingle` char

  newCap <- liftIO $ Buf.getCapacity buf
  (&& (len <= cap || newCap == (cap + 1))) <$> expectedLength buf (len + 1)

prop_bufferAppendEmpty :: Property
prop_bufferAppendEmpty = monadicIO $ do
  buf <- arbitraryBuffer

  len <- liftIO $ Buf.getLength buf

  liftIO $ buf `Buf.append` ""

  expectedLength buf len

prop_bufferAppend :: Property
prop_bufferAppend = monadicIO $ do
  buf <- arbitraryBuffer
  extraString <- pick $ resize 30 $ listOf arbitrary

  len <- liftIO $ Buf.getLength buf

  liftIO $ buf `Buf.append` extraString

  expectedLength buf $ len + fromIntegral (length extraString)

prop_bufferDecodeUtf8 :: Property
prop_bufferDecodeUtf8 = monadicIO $ do
  text <- fmap T.pack $ pick $ resize 1024 $ listOf arbitrary
  buf <- liftIO $ Buf.fromUtf8 $ T.encodeUtf8 text

  expectedLength buf (T.length text)

prop_bufferDecodeUtf8Failures :: Property
prop_bufferDecodeUtf8Failures = monadicIO $ do
  bs <- fmap BS.pack $ pick $ resize 2048 $ listOf arbitrary

  case T.decodeUtf8' bs of
    Right _ -> pure True -- we ignore random valid UTF-8 strings here
    Left _ ->
      liftIO $ (Buf.fromUtf8 bs >> pure False) `catch` \e ->
        if e == errnoToIOError "Graphics.Buchstabensuppe.Buffer.UTF32.fromUtf8" eINVAL Nothing Nothing
        then pure True
        else throwIO e -- rethrowing allows us to see the unexpected exception

prop_bufferFromString :: Property
prop_bufferFromString = monadicIO $ do
  str <- pick $ resize 100 $ listOf arbitrary
  buf <- liftIO $ Buf.fromString str
  expectedLength buf (length str)

prop_bufferFromText :: Property
prop_bufferFromText = monadicIO $ do
  text <- fmap T.pack $ pick $ resize 2048 $ listOf arbitrary
  buf <- liftIO $ Buf.fromText text
  expectedLength buf (T.length text)

expectedLength :: Integral a => Buf.Buffer -> a -> PropertyM IO Bool
expectedLength buf len = do
  monitorBuffer buf
  actual <- liftIO $ Buf.getLength buf
  cap <- liftIO $ Buf.getCapacity buf
  pure $ actual == fromIntegral len && cap >= fromIntegral len

arbitraryBuffer :: PropertyM IO Buf.Buffer
arbitraryBuffer = do
  initialSize <- pick $ choose (0, 16)
  buf <- liftIO $ Buf.newBuffer initialSize

  addInitialContents <- pick arbitrary
  when addInitialContents $ do
    str <- pick $ resize 20 $ listOf arbitrary
    liftIO $ buf `Buf.append` str

  pure buf

monitorBuffer :: Buf.Buffer -> PropertyM IO ()
monitorBuffer buf = do
  len <- liftIO $ Buf.getLength buf
  cap <- liftIO $ Buf.getCapacity buf
  monitor $ counterexample
    $ "Capacity: " ++ show cap ++ "; Length: " ++ show len
